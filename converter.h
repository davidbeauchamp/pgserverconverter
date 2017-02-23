#ifndef CONVERTER_H
#define CONVERTER_H

#include <QString>
#include <QTextStream>
#include <QCoreApplication>
#include <QSqlDatabase>

struct Server {
    int  userid;
    QString  group;
    QString  name;
    QString  host;
    int      port;
    QString  maintdb;
    QString  user;
    QString  role;
    QString  sslmode;
    QString  comment;
    QString  discoveryid;
};

class converter
{
    public:
        converter(QString pInputFile, QString pDatabase);
        inline QTextStream& console()
        {
            static QTextStream ts(stdout);
            return ts;
        }

    public slots:
        void start();

    private:
        QString _inputFile;
        QString _pgDatabase;
        QSqlDatabase _db;
};

#endif // CONVERTER_H
