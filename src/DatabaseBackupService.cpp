#include "DatabaseBackupService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <zlib.h>

namespace {

QString trBackup(const char *text)
{
    return QObject::tr(text);
}

bool writeProcessOutputToGzip(QProcess &process, gzFile gzipFile, QString *errorMessage)
{
    const QByteArray stdoutData = process.readAllStandardOutput();
    if (stdoutData.isEmpty())
        return true;

    const int written = gzwrite(gzipFile, stdoutData.constData(), static_cast<unsigned int>(stdoutData.size()));
    if (written == 0)
    {
        int errNo = Z_OK;
        const char *gzipError = gzerror(gzipFile, &errNo);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się zapisać backupu do pliku gzip.")
                            + QStringLiteral("\n")
                            + QString::fromUtf8(gzipError ? gzipError : "");
        return false;
    }

    return true;
}

} // namespace

DatabaseBackupService::DatabaseBackupService(QSqlDatabase database)
    : m_database(database)
{
}

bool DatabaseBackupService::backupToGzipFile(const QString &outputPath, QString *errorMessage) const
{
    MySqlConnectionInfo connectionInfo;
    if (!extractConnectionInfo(&connectionInfo, errorMessage))
        return false;

    const QString dumpExecutable = findDumpExecutable();
    if (dumpExecutable.isEmpty())
    {
        if (errorMessage)
            *errorMessage = trBackup("Nie znaleziono programu mysqldump ani mariadb-dump w systemie.");
        return false;
    }

    const QFileInfo outputInfo(outputPath);
    if (!QDir().mkpath(outputInfo.absolutePath()))
    {
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się przygotować katalogu docelowego dla backupu.");
        return false;
    }

    const QString tempOutputPath = outputPath + QStringLiteral(".tmp");
    QFile::remove(tempOutputPath);

    gzFile gzipFile = gzopen(QFile::encodeName(tempOutputPath).constData(), "wb9");
    if (!gzipFile)
    {
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się otworzyć pliku docelowego backupu.");
        return false;
    }

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("MYSQL_PWD"), connectionInfo.password);
    process.setProcessEnvironment(environment);
    process.setProgram(dumpExecutable);
    process.setArguments(buildDumpArguments(connectionInfo));
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();

    if (!process.waitForStarted())
    {
        gzclose(gzipFile);
        QFile::remove(tempOutputPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się uruchomić procesu backupu mysqldump.")
                            + QStringLiteral("\n")
                            + process.errorString();
        return false;
    }

    QByteArray stderrBuffer;
    while (process.state() != QProcess::NotRunning)
    {
        process.waitForFinished(100);
        stderrBuffer += process.readAllStandardError();
        if (!writeProcessOutputToGzip(process, gzipFile, errorMessage))
        {
            process.kill();
            process.waitForFinished();
            gzclose(gzipFile);
            QFile::remove(tempOutputPath);
            return false;
        }
    }

    stderrBuffer += process.readAllStandardError();
    if (!writeProcessOutputToGzip(process, gzipFile, errorMessage))
    {
        gzclose(gzipFile);
        QFile::remove(tempOutputPath);
        return false;
    }

    if (gzclose(gzipFile) != Z_OK)
    {
        QFile::remove(tempOutputPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się domknąć pliku backupu gzip.");
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
    {
        QFile::remove(tempOutputPath);
        if (errorMessage)
            *errorMessage = trBackup("Proces mysqldump zakończył się błędem.")
                            + QStringLiteral("\n")
                            + QString::fromUtf8(stderrBuffer.trimmed());
        return false;
    }

    QFile::remove(outputPath);
    if (!QFile::rename(tempOutputPath, outputPath))
    {
        QFile::remove(tempOutputPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się zapisać końcowego pliku backupu.");
        return false;
    }

    if (errorMessage)
        errorMessage->clear();
    return true;
}

QStringList DatabaseBackupService::buildDumpArguments(const MySqlConnectionInfo &connectionInfo)
{
    QStringList arguments;
    arguments << QStringLiteral("--single-transaction")
              << QStringLiteral("--quick")
              << QStringLiteral("--hex-blob")
              << QStringLiteral("--routines")
              << QStringLiteral("--events")
              << QStringLiteral("--triggers")
              << QStringLiteral("--skip-comments");

    if (!connectionInfo.host.isEmpty())
        arguments << QStringLiteral("--host=%1").arg(connectionInfo.host);
    if (connectionInfo.port > 0)
        arguments << QStringLiteral("--port=%1").arg(connectionInfo.port);
    if (!connectionInfo.user.isEmpty())
        arguments << QStringLiteral("--user=%1").arg(connectionInfo.user);

    arguments << connectionInfo.database;
    return arguments;
}

QString DatabaseBackupService::findDumpExecutable()
{
    const QString mariaDbDump = QStandardPaths::findExecutable(QStringLiteral("mariadb-dump"));
    if (!mariaDbDump.isEmpty())
        return mariaDbDump;

    return QStandardPaths::findExecutable(QStringLiteral("mysqldump"));
}

bool DatabaseBackupService::extractConnectionInfo(MySqlConnectionInfo *connectionInfo,
                                                  QString *errorMessage) const
{
    if (!connectionInfo)
        return false;

    if (!m_database.isOpen())
    {
        if (errorMessage)
            *errorMessage = trBackup("Połączenie z bazą danych jest zamknięte.");
        return false;
    }

    if (!m_database.driverName().contains(QStringLiteral("QMYSQL"), Qt::CaseInsensitive))
    {
        if (errorMessage)
            *errorMessage = trBackup("Backup sql.gz jest dostępny tylko dla połączeń MySQL/MariaDB.");
        return false;
    }

    connectionInfo->host = m_database.hostName();
    connectionInfo->database = m_database.databaseName();
    connectionInfo->user = m_database.userName();
    connectionInfo->password = m_database.password();
    connectionInfo->port = m_database.port();

    if (connectionInfo->database.isEmpty())
    {
        if (errorMessage)
            *errorMessage = trBackup("Brakuje nazwy bazy danych do wykonania backupu.");
        return false;
    }

    if (connectionInfo->user.isEmpty())
    {
        if (errorMessage)
            *errorMessage = trBackup("Brakuje nazwy użytkownika bazy danych do wykonania backupu.");
        return false;
    }

    return true;
}
