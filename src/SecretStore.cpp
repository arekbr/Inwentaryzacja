#include "SecretStore.h"

#include <QEventLoop>
#include <QSettings>
#include <QStandardPaths>

#include <qtkeychain/keychain.h>

namespace {

QString keychainService()
{
    // Testy (QStandardPaths w trybie testowym) nie mogą ruszać prawdziwych wpisów usera.
    return QStandardPaths::isTestModeEnabled() ? QStringLiteral("Inwentaryzacja-test")
                                               : QStringLiteral("Inwentaryzacja");
}

QSettings appSettings()
{
    return QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                         + QStringLiteral("/inwentaryzacja.ini"),
                     QSettings::IniFormat);
}

/// QtKeychain działa asynchronicznie, a wołający (okno ustawień, PreviewDialog) potrzebują
/// wyniku od razu — czekamy na lokalnej pętli zdarzeń. Bez limitu czasu: porzucone zadanie
/// blokowałoby kolejkę QtKeychain (jedno zadanie naraz), a macOS może czekać na decyzję usera.
QKeychain::Error runKeychainJob(QKeychain::Job &job, const QString &account, QString *errorMessage)
{
    job.setAutoDelete(false);
    job.setInsecureFallback(false);  // nigdy jawnego tekstu zamiast sejfu
    job.setKey(account);
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();  // start() kolejkuje doStart — finished nie przyjdzie przed exec()
    loop.exec();
    if (job.error() != QKeychain::NoError && job.error() != QKeychain::EntryNotFound && errorMessage)
        *errorMessage = job.errorString();
    return job.error();
}

QString readKeychain(const QString &account, QString *errorMessage)
{
    QKeychain::ReadPasswordJob job(keychainService());
    return runKeychainJob(job, account, errorMessage) == QKeychain::NoError ? job.textData() : QString();
}

bool writeKeychain(const QString &account, const QString &value, QString *errorMessage)
{
    QKeychain::WritePasswordJob job(keychainService());
    job.setTextData(value);
    return runKeychainJob(job, account, errorMessage) == QKeychain::NoError;
}

bool deleteKeychain(const QString &account, QString *errorMessage)
{
    QKeychain::DeletePasswordJob job(keychainService());
    const QKeychain::Error error = runKeychainJob(job, account, errorMessage);
    return error == QKeychain::NoError || error == QKeychain::EntryNotFound;
}

void removeLegacyIniValue(const QString &legacyIniKey)
{
    QSettings s = appSettings();
    if (s.contains(legacyIniKey))  // bez contains() remove() przepisywałby ini przy każdym odczycie
        s.remove(legacyIniKey);
}

}  // namespace

namespace SecretStore
{

ReadResult read(const QString &account, const QString &legacyIniKey)
{
    ReadResult result;
    const QString legacyValue = appSettings().value(legacyIniKey).toString();
    if (!legacyValue.isEmpty())
    {
        if (writeKeychain(account, legacyValue, &result.keychainError)
            && readKeychain(account, &result.keychainError) == legacyValue)
            removeLegacyIniValue(legacyIniKey);
        else
            result.stillInIni = true;
        result.value = legacyValue;  // apka działa dalej jak przed migracją, nawet gdy sejf odmówił
        return result;
    }

    removeLegacyIniValue(legacyIniKey);  // pusty wpis zostawiony przez starszą wersję
    result.value = readKeychain(account, &result.keychainError);
    return result;
}

bool write(const QString &account, const QString &legacyIniKey, const QString &value,
           QString *keychainError)
{
    const bool ok = value.isEmpty() ? deleteKeychain(account, keychainError)
                                    : writeKeychain(account, value, keychainError);
    // Stara wartość z ini jest nieaktualna niezależnie od wyniku: zostawiona po nieudanym zapisie
    // wróciłaby po cichu przy następnym starcie (migracja) zamiast nowej, wpisanej przez usera.
    removeLegacyIniValue(legacyIniKey);
    return ok;
}

}  // namespace SecretStore
