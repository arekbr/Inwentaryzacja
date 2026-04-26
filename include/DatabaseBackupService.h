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

/// O-5 (audit 2026-04-26): identyczny lifetime contract jak ItemRepository —
/// `m_database` jest QSqlDatabase HANDLE, nie owner. NIE wywołuj
/// `QSqlDatabase::removeDatabase(name)` dopóki ten obiekt żyje.
/// Dla SQLite backup używa się wewnętrznie OSOBNEGO connection
/// ("backup-sqlite-vacuum-<tid>") żeby nie kolidować z aktywnym m_database.
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

    /// E-5 (audit 2026-04-26): SQLite native backup przez VACUUM INTO + gzip.
    /// VACUUM INTO jest atomic — bezpieczny nawet podczas zapisu (write lock
    /// na czas backupu). Output: standardowy SQLite .db spakowany gzip.
    /// @param sourceDatabasePath ścieżka do źródłowej bazy SQLite (z `m_database.databaseName()`)
    /// @param outputPath docelowa ścieżka `.sql.gz` (lub `.db.gz` — uniwersalne)
    static bool backupSqliteToGzipFile(const QString &sourceDatabasePath,
                                       const QString &outputPath,
                                       QString *errorMessage,
                                       BackupResult *result = nullptr,
                                       const std::function<void(qint64)> &progressCallback = {},
                                       const std::function<void(const QString &)> &statusCallback = {});

    /// E-3 (audit 2026-04-26): jesli defaultsExtraFile niepusta, zostanie dodana
    /// jako pierwszy argument `--defaults-extra-file=<path>` i `--user=` zostanie
    /// pominiete (user/password sa w defaults file). Bez tego mysqldump dziala
    /// po staremu (--user= w args, hasla wymaga z env MYSQL_PWD).
    static QStringList buildDumpArguments(const MySqlConnectionInfo &connectionInfo,
                                          const QString &defaultsExtraFile = QString());
    static QString findDumpExecutable();

private:
    bool extractConnectionInfo(MySqlConnectionInfo *connectionInfo, QString *errorMessage) const;

    QSqlDatabase m_database;
};

#endif // DATABASEBACKUPSERVICE_H
