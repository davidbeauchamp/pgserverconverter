#include "converter.h"

#include <QSettings>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

converter::converter(QString pInputFile, QString pDatabase)
{
    _inputFile  = pInputFile;
    _pgDatabase = pDatabase;
}

void converter::start() {
    QStringList groups;
    QList<Server> servers;

    int serverCount = 0;

    console() << " [*] Starting" << endl;
    QFile *file;
#ifndef Q_OS_WIN
    file = new QFile(_inputFile);
    if (!file->exists()) {
        console() << " [!] ERROR: Input file " << _inputFile << " does not exist" << endl;
        exit(-1);
    }
#endif
    file = new QFile(_pgDatabase);
    if (!file->exists()) {
        console() << " [!] ERROR: Database file " << _pgDatabase << " does not exist" << endl;
        exit(-1);
    }
    else {
        if (_db.isOpen())
            _db.close();
        _db = QSqlDatabase::addDatabase("QSQLITE");
        _db.setDatabaseName(_pgDatabase);

        if (!_db.open()) {
            console() << " [!] ERROR: Cannot open SQLite database"  << _pgDatabase << endl;
            exit(-1);
        }
    }

#ifdef Q_OS_WIN
        QSettings settings("HKEY_CURRENT_USER\\Software\\pgAdmin III", QSettings::NativeFormat);
#else
        QSettings settings(_inputFile, QSettings::IniFormat);
#endif

    QStringList allkeys = settings.allKeys(); // I don't know why I have to call this to make it read the file..

    serverCount = settings.value("Servers/Count").toString().toInt();

    for (int i = 1; i <= serverCount; i += 1) {
        settings.beginGroup("Servers/" + QString::number(i));
        if (!groups.contains(settings.value("Group").toString())) {
            groups.append(settings.value("Group").toString());
        }

        Server server;
        server.userid      = 1;
        server.group       = settings.value("Group").toString();
        server.host        = settings.value("Server").toString();
        server.port        = settings.value("Port").toInt();
        server.maintdb     = settings.value("Database").toString();
        server.user        = settings.value("Username").toString();
        server.role        = settings.value("Rolename").toString();
        int sslmode = settings.value("SSL").toInt();
        switch (sslmode) {
            case 1:
                server.sslmode = "require";
                break;
            case 3:
                server.sslmode = "allow";
                break;
            case 4:
                server.sslmode = "disable";
                break;
            case 5:
                server.sslmode = "verify-ca";
                break;
            case 6:
                server.sslmode = "verify-full";
                break;
            case -1:
            case 2:
            default:
                server.sslmode = "prefer";
                break;
        }
        server.name        = settings.value("Description").toString();
        server.discoveryid = settings.value("DiscoveryID").toString();
        servers.append((server));
        settings.endGroup();
    }
    console() << " [*] Importing groups" << endl;
    // need to make sure the groups exist first
    foreach (QString group, groups) {
        console() << " [*] Importing group " << group << " if it does not exist" ;
        QSqlQuery grpqry;
        grpqry.prepare("INSERT INTO servergroup (user_id, name) "
                       "SELECT :user_id, :name1 "
                       " WHERE NOT EXISTS (SELECT name FROM servergroup WHERE name = :name2) ");
        grpqry.bindValue(":user_id", 1);
        grpqry.bindValue(":name1", group);
        grpqry.bindValue(":name2", group);

        if (!grpqry.exec()) {
            console() << "Error importing group: " << group << "-" << grpqry.lastError().text() << endl;
            exit(-1);
        }
        else if (grpqry.numRowsAffected() > 0)
            console() << " ... it doesn't. Record imported." << endl;
        else
            console() << " ... it does. Record skipped." << endl;
    }
    console() << " [*] Done importing groups"  << endl;
    console() << " [*] Importing servers"  << endl;
    foreach (Server s,servers) {
        console() << " [*] Importing server " << s.name << " as long as the host, port, and user combo is unique";
        QSqlQuery sqry;
        sqry.prepare("INSERT INTO server (user_id, servergroup_id, name, host, port, "
                     "                    maintenance_db, username, role, ssl_mode, comment, discovery_id) "
                     "SELECT :user_id, (SELECT id FROM servergroup WHERE servergroup.name = :groupname), :name, :host, :port, "
                     "       :maintenance_db, :username, :role, :ssl_mode, :comment, :discovery_id "
                     " WHERE NOT EXISTS (SELECT server.name FROM server WHERE server.host = :serverhost AND server.port = :serverport AND server.username = :serveruser);");

        sqry.bindValue(":user_id", 1);
        sqry.bindValue(":groupname",      s.group);
        sqry.bindValue(":name",           s.name);
        sqry.bindValue(":host",           s.host);
        sqry.bindValue(":port",           s.port);
        sqry.bindValue(":maintenance_db", s.maintdb);
        sqry.bindValue(":username",       s.user);
        sqry.bindValue(":role",           s.role);
        sqry.bindValue(":ssl_mode",       s.sslmode);
        sqry.bindValue(":comment",        s.comment);
        sqry.bindValue(":discovery_id",   s.discoveryid);
        sqry.bindValue(":serverhost",     s.host);
        sqry.bindValue(":serverport",     s.port);
        sqry.bindValue(":serveruser",     s.user);

        if (!sqry.exec()) {
            console() << "Error importing server:"  << s.name << "-" << sqry.lastError().text() << endl;
            exit(-1);
        }
        else if (sqry.numRowsAffected() > 0)
            console() << "... it is. Record imported." << endl;
        else
            console() << "... it isn't. Record skipped." << endl;
    }
    console() << " [*] Done importing servers" << endl;
    console() << " [*] Done" << endl;
    _db.close();
    exit(0);
}

