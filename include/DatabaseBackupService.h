#ifndef DATABASEBACKUPSERVICE_H
#define DATABASEBACKUPSERVICE_H

#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <functional>

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
    struct BackupResult
    {
        qint64 compressedBytes = 0;
        qint64 uncompressedBytes = 0;
        bool gzipVerified = false;
    };

    explicit DatabaseBackupService(QSqlDatabase database = QSqlDatabase::database("default_connection"));

    bool connectionInfo(MySqlConnectionInfo *connectionInfo, QString *errorMessage) const;

    bool backupToGzipFile(const QString &outputPath,
                          QString *errorMessage,
                          BackupResult *result = nullptr,
                          const std::function<void(qint64)> &progressCallback = {},
                          const std::function<void(const QString &)> &statusCallback = {}) const;

    static bool backupToGzipFile(const MySqlConnectionInfo &connectionInfo,
                                 const QString &outputPath,
                                 QString *errorMessage,
                                 BackupResult *result = nullptr,
                                 const std::function<void(qint64)> &progressCallback = {},
                                 const std::function<void(const QString &)> &statusCallback = {});

    static QStringList buildDumpArguments(const MySqlConnectionInfo &connectionInfo);
    static QString findDumpExecutable();

private:
    bool extractConnectionInfo(MySqlConnectionInfo *connectionInfo, QString *errorMessage) const;

    QSqlDatabase m_database;
};

#endif // DATABASEBACKUPSERVICE_H
