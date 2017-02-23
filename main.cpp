#include <QCoreApplication>
#include <QCommandLineParser>

#include <QStandardPaths>
#include <QDir>
#include "converter.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("pgserverconverter");
    app.setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Convert saved pgadmin3 servers to pgadmin4 db entries");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption inputFile(QStringList() << "i" << "input",
    #ifdef Q_OS_LINUX
    QCoreApplication::translate("main", "pgadmin3 config file. Defaults to ~/.pgadmin3"),
    #endif
    #ifdef Q_OS_MAC
    QCoreApplication::translate("main", "pgadmin3 config file. Defaults to ~/Library/Preferences/pgadmin3 Preferences"),
    #endif
    #ifdef Q_OS_WIN
    QCoreApplication::translate("main", "pgadmin3 config file. Defaults to the windows registry"),
    #endif
    QCoreApplication::translate("main", "inputfile"));
    parser.addOption(inputFile);

    QCommandLineOption database(QStringList() << "d" << "database",
    #ifdef Q_OS_WIN
    QCoreApplication::translate("main", "Path to pgAdmin4's SQLite database. Defaults to ~/Application Data/Roaming/pgadmin/pgadmin4.db"),
    #else
    QCoreApplication::translate("main", "Path to pgAdmin4's SQLite database. Defaults to ~/.pgadmin/pgadmin4.db"),
    #endif
    QCoreApplication::translate("main", "database"));
    parser.addOption(database);

    parser.process(app);

    QString _input;
    QString _db;

    if (parser.isSet("input")) {
        _input = parser.value("input");
    }
    else {
        #ifdef Q_OS_LINUX
        _input = QDir::homePath() + "/.pgadmin3";
        #endif
        #ifdef Q_OS_MAC
        _input = QDir::homePath() + "/Library/Preferences/pgadmin3 Preferences";
        #endif
        #ifdef Q_OS_WIN
        _input = "Windows Registry";
        #endif
    }

    if (parser.isSet("database")) {
        _db = parser.value("database");
    }
    else {
        #ifdef Q_OS_WIN
        _db = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] + "/../pgadmin/pgadmin4.db";
        #else
        _db = QDir::homePath() + "/.pgadmin/pgadmin4.db";
        #endif
    }

    converter c(_input, _db);
    c.start();
    return app.exec();
}
