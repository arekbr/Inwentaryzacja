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

bool verifyGzipFile(const QString &path, QString *errorMessage)
{
    gzFile gzipFile = gzopen(QFile::encodeName(path).constData(), "rb");
    if (!gzipFile)
    {
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się ponownie otworzyć backupu do weryfikacji.");
        return false;
    }

    char buffer[64 * 1024];
    int readBytes = 0;
    do
    {
        readBytes = gzread(gzipFile, buffer, sizeof(buffer));
        if (readBytes < 0)
        {
            int errNo = Z_OK;
            const char *gzipError = gzerror(gzipFile, &errNo);
            gzclose(gzipFile);
            if (errorMessage)
                *errorMessage = trBackup("Plik backupu gzip nie przeszedł weryfikacji integralności.")
                                + QStringLiteral("\n")
                                + QString::fromUtf8(gzipError ? gzipError : "");
            return false;
        }
    } while (readBytes > 0);

    gzclose(gzipFile);
    return true;
}

} // namespace

DatabaseBackupService::DatabaseBackupService(QSqlDatabase database)
    : m_database(database)
{
}

bool DatabaseBackupService::connectionInfo(MySqlConnectionInfo *connectionInfo,
                                           QString *errorMessage) const
{
    return extractConnectionInfo(connectionInfo, errorMessage);
}

bool DatabaseBackupService::backupToGzipFile(const QString &outputPath,
                                             QString *errorMessage,
                                             BackupResult *result,
                                             const std::function<void(qint64)> &progressCallback,
                                             const std::function<void(const QString &)> &statusCallback) const
{
    if (result)
        *result = BackupResult{};

    MySqlConnectionInfo connectionInfo;
    if (!extractConnectionInfo(&connectionInfo, errorMessage))
        return false;

    return backupToGzipFile(connectionInfo,
                            outputPath,
                            errorMessage,
                            result,
                            progressCallback,
                            statusCallback);
}

bool DatabaseBackupService::backupToGzipFile(const MySqlConnectionInfo &connectionInfo,
                                             const QString &outputPath,
                                             QString *errorMessage,
                                             BackupResult *result,
                                             const std::function<void(qint64)> &progressCallback,
                                             const std::function<void(const QString &)> &statusCallback)
{
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
    if (statusCallback)
        statusCallback(trBackup("Trwa tworzenie backupu SQL.gz..."));
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
    qint64 totalWrittenBytes = 0;
    while (process.state() != QProcess::NotRunning)
    {
        process.waitForFinished(100);
        stderrBuffer += process.readAllStandardError();
        const QByteArray stdoutData = process.readAllStandardOutput();
        if (!stdoutData.isEmpty())
        {
            const int written = gzwrite(gzipFile,
                                        stdoutData.constData(),
                                        static_cast<unsigned int>(stdoutData.size()));
            if (written == 0)
            {
                int errNo = Z_OK;
                const char *gzipError = gzerror(gzipFile, &errNo);
                if (errorMessage)
                    *errorMessage = trBackup("Nie udało się zapisać backupu do pliku gzip.")
                                    + QStringLiteral("\n")
                                    + QString::fromUtf8(gzipError ? gzipError : "");
                process.kill();
                process.waitForFinished();
                gzclose(gzipFile);
                QFile::remove(tempOutputPath);
                return false;
            }

            totalWrittenBytes += stdoutData.size();
            if (progressCallback)
                progressCallback(totalWrittenBytes);
        }
    }

    stderrBuffer += process.readAllStandardError();
    const QByteArray remainingStdout = process.readAllStandardOutput();
    if (!remainingStdout.isEmpty())
    {
        const int written = gzwrite(gzipFile,
                                    remainingStdout.constData(),
                                    static_cast<unsigned int>(remainingStdout.size()));
        if (written == 0)
        {
            int errNo = Z_OK;
            const char *gzipError = gzerror(gzipFile, &errNo);
            gzclose(gzipFile);
            QFile::remove(tempOutputPath);
            if (errorMessage)
                *errorMessage = trBackup("Nie udało się zapisać backupu do pliku gzip.")
                                + QStringLiteral("\n")
                                + QString::fromUtf8(gzipError ? gzipError : "");
            return false;
        }

        totalWrittenBytes += remainingStdout.size();
        if (progressCallback)
            progressCallback(totalWrittenBytes);
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

    if (statusCallback)
        statusCallback(trBackup("Trwa weryfikacja backupu SQL.gz..."));
    QString verificationError;
    if (!verifyGzipFile(outputPath, &verificationError))
    {
        QFile::remove(outputPath);
        if (errorMessage)
            *errorMessage = verificationError;
        return false;
    }

    if (result)
    {
        const QFileInfo outputFileInfo(outputPath);
        result->compressedBytes = outputFileInfo.size();
        result->uncompressedBytes = totalWrittenBytes;
        result->gzipVerified = true;
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
