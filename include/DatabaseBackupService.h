#ifndef DATABASEBACKUPSERVICE_H
#define DATABASEBACKUPSERVICE_H

#include <QSqlDatabase>
#include <QString>
#include <QStringList>

struct MySqlConnectionInfo
{
    QString host;
    QString database;
    QString user;
    QString password;
    int port = 3306;
};

class DatabaseBackupService
{
public:
    explicit DatabaseBackupService(QSqlDatabase database = QSqlDatabase::database("default_connection"));

    bool backupToGzipFile(const QString &outputPath, QString *errorMessage) const;

    static QStringList buildDumpArguments(const MySqlConnectionInfo &connectionInfo);
    static QString findDumpExecutable();

private:
    bool extractConnectionInfo(MySqlConnectionInfo *connectionInfo, QString *errorMessage) const;

    QSqlDatabase m_database;
};

#endif // DATABASEBACKUPSERVICE_H
