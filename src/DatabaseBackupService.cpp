#include "DatabaseBackupService.h"

#include <QDeadlineTimer>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>

#include <chrono>
#include <zlib.h>

namespace {

QString trBackup(const char *text)
{
    return QObject::tr(text);
}

// E-4 (audit 2026-04-26): atomowe rename z zachowaniem starego backupu jako .old.
// Jeśli rename tmp → target padnie cross-FS lub przy permissions, stary backup
// nie znika (rollback z .old). Po sukcesie .old jest usuwany.
// Stara wersja: QFile::remove(target) + rename(tmp, target) — jeśli rename padnie,
// user traci OBA pliki (stary i nowy).
bool atomicReplaceWithBackup(const QString &tempPath, const QString &targetPath, QString *errorMessage)
{
    const QString oldPath = targetPath + QStringLiteral(".old");
    QFile::remove(oldPath);  // czysty start dla .old (z poprzednich runów)

    bool hadOldBackup = false;
    if (QFile::exists(targetPath))
    {
        if (!QFile::rename(targetPath, oldPath))
        {
            if (errorMessage)
                *errorMessage = QObject::tr("Nie udało się zarchiwizować starego backupu jako .old.");
            return false;
        }
        hadOldBackup = true;
    }

    if (!QFile::rename(tempPath, targetPath))
    {
        // Rollback: przywróć .old jako target
        if (hadOldBackup)
            QFile::rename(oldPath, targetPath);
        QFile::remove(tempPath);
        if (errorMessage)
            *errorMessage = QObject::tr("Nie udało się zapisać finalnego pliku backupu pod docelową nazwą.");
        return false;
    }

    // Sukces — usuwamy .old (już niepotrzebny)
    if (hadOldBackup)
        QFile::remove(oldPath);
    return true;
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

    // E-5 (audit 2026-04-26): dispatch po driver name. SQLite (default backend)
    // dostal teraz natywny backup przez VACUUM INTO + gzip — wczesniej dawal
    // blad "tylko MySQL". MySQL/MariaDB nadal przez mysqldump.
    const QString driverName = m_database.driverName();
    if (driverName == QStringLiteral("QSQLITE"))
    {
        return backupSqliteToGzipFile(m_database.databaseName(),
                                      outputPath,
                                      errorMessage,
                                      result,
                                      progressCallback,
                                      statusCallback);
    }

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

    // E-3 (audit 2026-04-26): zamiast MYSQL_PWD env (leak przez /proc/<pid>/environ
    // + memory dump parent procesu) — uzyj --defaults-extra-file ze zwyklym tmp
    // plikiem chmod 600. Plik zniknie automatycznie po wyjsciu z funkcji
    // (QTemporaryFile setAutoRemove default true).
    QTemporaryFile defaultsFile(QDir::tempPath() + QStringLiteral("/mysqldump-XXXXXX.cnf"));
    defaultsFile.setAutoRemove(true);
    if (!defaultsFile.open())
    {
        gzclose(gzipFile);
        QFile::remove(tempOutputPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się utworzyć tymczasowego pliku konfiguracji "
                                     "dla mysqldump.")
                            + QStringLiteral("\n") + defaultsFile.errorString();
        return false;
    }
    // chmod 600 — tylko owner read/write, plik z haslem nie ma byc world-readable
    if (!defaultsFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner))
    {
        gzclose(gzipFile);
        QFile::remove(tempOutputPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się ustawić uprawnień 0600 na pliku konfiguracji.");
        return false;
    }
    {
        QTextStream cnf(&defaultsFile);
        cnf << "[client]\n";
        if (!connectionInfo.user.isEmpty())
            cnf << "user=" << connectionInfo.user << "\n";
        if (!connectionInfo.password.isEmpty())
            cnf << "password=" << connectionInfo.password << "\n";
        cnf.flush();
    }
    defaultsFile.close();  // flush handle — proces dziecko bedzie czytac przez path

    QProcess process;
    process.setProgram(dumpExecutable);
    process.setArguments(buildDumpArguments(connectionInfo, defaultsFile.fileName()));
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
    // E-2 (audit 2026-04-26): timeouts żeby mysqldump hang nie blokowal apki:
    // - hardDeadline: globalny limit (default 30 min) — backup gigantycznej bazy
    //   moze trwac dlugo, ale 30 min to absolutny rozsadny gorny prog
    // - idleTimeoutMs: jeśli przez X sekund mysqldump nie wyemituje ani bajtu
    //   na stdout — uznajemy za hang (zablokowana tabela, network stall, etc.)
    QDeadlineTimer hardDeadline(std::chrono::minutes(30));
    constexpr int idleTimeoutMs = 30 * 1000;
    QElapsedTimer sinceLastBytes;
    sinceLastBytes.start();

    while (process.state() != QProcess::NotRunning)
    {
        if (hardDeadline.hasExpired())
        {
            process.kill();
            process.waitForFinished(5000);
            gzclose(gzipFile);
            QFile::remove(tempOutputPath);
            if (errorMessage)
                *errorMessage = trBackup("Backup przerwany — przekroczono globalny limit czasu (30 min).")
                                + (stderrBuffer.isEmpty() ? QString()
                                   : QStringLiteral("\nmysqldump stderr:\n")
                                     + QString::fromUtf8(stderrBuffer.left(2000)));
            return false;
        }
        if (sinceLastBytes.elapsed() > idleTimeoutMs)
        {
            process.kill();
            process.waitForFinished(5000);
            gzclose(gzipFile);
            QFile::remove(tempOutputPath);
            if (errorMessage)
                *errorMessage = trBackup("Backup przerwany — mysqldump nie wysłał danych przez 30 s "
                                         "(zablokowana tabela lub network stall?).")
                                + (stderrBuffer.isEmpty() ? QString()
                                   : QStringLiteral("\nmysqldump stderr:\n")
                                     + QString::fromUtf8(stderrBuffer.left(2000)));
            return false;
        }

        process.waitForFinished(100);
        stderrBuffer += process.readAllStandardError();
        const QByteArray stdoutData = process.readAllStandardOutput();
        if (!stdoutData.isEmpty())
        {
            sinceLastBytes.restart();  // E-2: reset idle timer na nowych bytes
            const int written = gzwrite(gzipFile,
                                        stdoutData.constData(),
                                        static_cast<unsigned int>(stdoutData.size()));
            // E-1 (audit 2026-04-26): partial-write też jest błędem, nie tylko 0.
            // gzwrite zwraca liczbe bajtów zapisanych — jeśli < requested,
            // bufor zlib padl miedzy callami. Stary `== 0` przepuszczalby.
            if (written != static_cast<int>(stdoutData.size()))
            {
                int errNo = Z_OK;
                const char *gzipError = gzerror(gzipFile, &errNo);
                if (errorMessage)
                    *errorMessage = trBackup("Nie udało się zapisać backupu do pliku gzip.")
                                    + QStringLiteral("\n")
                                    + QString::fromUtf8(gzipError ? gzipError : "")
                                    // E-1: stderr z mysqldump zawiera prawdziwą przyczynę
                                    // (np. 'access denied for user X'). Bez tego operator
                                    // dostawal generic 'nie udalo sie zapisac'.
                                    + (stderrBuffer.isEmpty() ? QString()
                                       : QStringLiteral("\nmysqldump stderr:\n")
                                         + QString::fromUtf8(stderrBuffer.left(2000)));
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
        // E-1: jak wyżej — partial write = błąd
        if (written != static_cast<int>(remainingStdout.size()))
        {
            int errNo = Z_OK;
            const char *gzipError = gzerror(gzipFile, &errNo);
            gzclose(gzipFile);
            QFile::remove(tempOutputPath);
            if (errorMessage)
                *errorMessage = trBackup("Nie udało się zapisać backupu do pliku gzip.")
                                + QStringLiteral("\n")
                                + QString::fromUtf8(gzipError ? gzipError : "")
                                + (stderrBuffer.isEmpty() ? QString()
                                   : QStringLiteral("\nmysqldump stderr:\n")
                                     + QString::fromUtf8(stderrBuffer.left(2000)));
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

    // E-4: atomic replace + .old rotation (zachowuje stary backup gdy nowy padnie)
    if (!atomicReplaceWithBackup(tempOutputPath, outputPath, errorMessage))
        return false;

    if (statusCallback)
        statusCallback(trBackup("Trwa sprawdzanie integralności archiwum SQL.gz..."));
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

bool DatabaseBackupService::backupSqliteToGzipFile(const QString &sourceDatabasePath,
                                                    const QString &outputPath,
                                                    QString *errorMessage,
                                                    BackupResult *result,
                                                    const std::function<void(qint64)> &progressCallback,
                                                    const std::function<void(const QString &)> &statusCallback)
{
    if (result)
        *result = BackupResult{};

    if (sourceDatabasePath.isEmpty())
    {
        if (errorMessage)
            *errorMessage = trBackup("Backup SQLite niemożliwy — brak ścieżki do pliku bazy.");
        return false;
    }
    // E-5: ':memory:' i specjalne nazwy — nie da się zrobić backup tego co nie jest plikiem
    if (sourceDatabasePath == QStringLiteral(":memory:")
        || sourceDatabasePath.startsWith(QStringLiteral("file::memory:"))
        || sourceDatabasePath.startsWith(QStringLiteral("file:")))
    {
        if (errorMessage)
            *errorMessage = trBackup("Backup bazy SQLite in-memory nie jest możliwy.");
        return false;
    }
    if (!QFile::exists(sourceDatabasePath))
    {
        if (errorMessage)
            *errorMessage = trBackup("Plik bazy SQLite nie istnieje: ") + sourceDatabasePath;
        return false;
    }

    const QFileInfo outputInfo(outputPath);
    if (!QDir().mkpath(outputInfo.absolutePath()))
    {
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się przygotować katalogu docelowego dla backupu.");
        return false;
    }

    if (statusCallback)
        statusCallback(trBackup("Trwa tworzenie backupu SQLite (VACUUM INTO)..."));

    // E-5 krok 1: VACUUM INTO do tymczasowego pliku .db (uncompressed).
    // VACUUM INTO jest atomic, bezpieczny podczas zapisu, plik wynikowy
    // to standardowy SQLite .db (compact — bez fragmentacji). Wymaga SQLite >= 3.27 (2019).
    QTemporaryFile vacuumTarget(QDir::tempPath() + QStringLiteral("/inwentaryzacja-vacuum-XXXXXX.db"));
    vacuumTarget.setAutoRemove(true);
    if (!vacuumTarget.open())
    {
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się utworzyć pliku tymczasowego dla VACUUM INTO.")
                            + QStringLiteral("\n") + vacuumTarget.errorString();
        return false;
    }
    const QString vacuumPath = vacuumTarget.fileName();
    vacuumTarget.close();
    // VACUUM INTO wymaga że plik docelowy NIE istnieje. QTemporaryFile::open
    // tworzy plik — kasujemy zanim VACUUM go zapisze.
    QFile::remove(vacuumPath);

    {
        // Otwieramy osobne connection do source (read-only by default for VACUUM source)
        // żeby nie kolidowac z aktywnym m_database (który moze być w transakcji).
        const QString connName = QStringLiteral("backup-sqlite-vacuum-")
                                 + QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
        {
            QSqlDatabase backupDb = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
            backupDb.setDatabaseName(sourceDatabasePath);
            if (!backupDb.open())
            {
                const QString err = backupDb.lastError().text();
                QSqlDatabase::removeDatabase(connName);
                if (errorMessage)
                    *errorMessage = trBackup("Nie udało się otworzyć bazy SQLite do backupu.")
                                    + QStringLiteral("\n") + err;
                return false;
            }

            QSqlQuery vacuumQuery(backupDb);
            // Single quote escape (basic) — path raczej nie zawiera ', ale dla bezpieczenstwa.
            QString escapedPath = vacuumPath;
            escapedPath.replace(QStringLiteral("'"), QStringLiteral("''"));
            const QString sql = QStringLiteral("VACUUM INTO '%1'").arg(escapedPath);
            if (!vacuumQuery.exec(sql))
            {
                const QString err = vacuumQuery.lastError().text();
                backupDb.close();
                QSqlDatabase::removeDatabase(connName);
                if (errorMessage)
                    *errorMessage = trBackup("Polecenie VACUUM INTO nie powiodło się.")
                                    + QStringLiteral("\n") + err;
                return false;
            }
            backupDb.close();
        }
        QSqlDatabase::removeDatabase(connName);
    }

    // E-5 krok 2: gzip plik VACUUM target → outputPath
    if (statusCallback)
        statusCallback(trBackup("Trwa kompresja gzip..."));

    const QString tempOutputPath = outputPath + QStringLiteral(".tmp");
    QFile::remove(tempOutputPath);

    gzFile gzipFile = gzopen(QFile::encodeName(tempOutputPath).constData(), "wb9");
    if (!gzipFile)
    {
        QFile::remove(vacuumPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się otworzyć pliku docelowego backupu.");
        return false;
    }

    QFile vacuumFile(vacuumPath);
    if (!vacuumFile.open(QIODevice::ReadOnly))
    {
        gzclose(gzipFile);
        QFile::remove(tempOutputPath);
        QFile::remove(vacuumPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się odczytać tymczasowego pliku VACUUM.");
        return false;
    }

    qint64 totalWrittenBytes = 0;
    constexpr int chunkSize = 64 * 1024;
    QByteArray chunk;
    chunk.resize(chunkSize);
    while (!vacuumFile.atEnd())
    {
        const qint64 readBytes = vacuumFile.read(chunk.data(), chunkSize);
        if (readBytes <= 0)
            break;
        const int written = gzwrite(gzipFile, chunk.constData(), static_cast<unsigned int>(readBytes));
        if (written != static_cast<int>(readBytes))
        {
            int errNo = Z_OK;
            const char *gzipError = gzerror(gzipFile, &errNo);
            gzclose(gzipFile);
            vacuumFile.close();
            QFile::remove(tempOutputPath);
            QFile::remove(vacuumPath);
            if (errorMessage)
                *errorMessage = trBackup("Nie udało się zapisać backupu SQLite do pliku gzip.")
                                + QStringLiteral("\n")
                                + QString::fromUtf8(gzipError ? gzipError : "");
            return false;
        }
        totalWrittenBytes += readBytes;
        if (progressCallback)
            progressCallback(totalWrittenBytes);
    }
    vacuumFile.close();
    QFile::remove(vacuumPath);  // QTemporaryFile autoRemove + manual cleanup defensive

    if (gzclose(gzipFile) != Z_OK)
    {
        QFile::remove(tempOutputPath);
        if (errorMessage)
            *errorMessage = trBackup("Nie udało się zamknąć pliku gzip backupu (możliwa korupcja).");
        return false;
    }

    if (!verifyGzipFile(tempOutputPath, errorMessage))
    {
        QFile::remove(tempOutputPath);
        return false;
    }

    // E-4: atomic replace + .old rotation
    if (!atomicReplaceWithBackup(tempOutputPath, outputPath, errorMessage))
        return false;

    if (result)
    {
        const QFileInfo outputFileInfo(outputPath);
        result->compressedBytes = outputFileInfo.size();
        result->uncompressedBytes = totalWrittenBytes;
        result->gzipVerified = true;
    }

    if (statusCallback)
        statusCallback(trBackup("Backup SQLite zakończony pomyślnie."));

    if (errorMessage)
        errorMessage->clear();
    return true;
}

QStringList DatabaseBackupService::buildDumpArguments(const MySqlConnectionInfo &connectionInfo,
                                                       const QString &defaultsExtraFile)
{
    QStringList arguments;
    // E-3: --defaults-extra-file MUSI byc PIERWSZYM argumentem (mysql convention)
    if (!defaultsExtraFile.isEmpty())
        arguments << QStringLiteral("--defaults-extra-file=%1").arg(defaultsExtraFile);

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
    // E-3: --user= tylko gdy nie ma defaults-extra-file (tam user juz jest)
    if (!connectionInfo.user.isEmpty() && defaultsExtraFile.isEmpty())
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
