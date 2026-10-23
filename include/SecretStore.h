#ifndef SECRETSTORE_H
#define SECRETSTORE_H

#include <QString>

/// Sekrety aplikacji w systemowym sejfie (QtKeychain: Pęk kluczy macOS / Menedżer poświadczeń
/// Windows / Secret Service Linux), usługa `Inwentaryzacja` — w testach `Inwentaryzacja-test`.
/// Starsze wersje trzymały sekrety jawnym tekstem w inwentaryzacja.ini; read() przenosi je do sejfu.
///
/// Wywołania czekają na sejf na lokalnej pętli zdarzeń (macOS może przy tym pytać usera o zgodę).
namespace SecretStore
{
struct ReadResult
{
    QString value;
    QString keychainError;    ///< niepusty = sejf odmówił albo jest niedostępny
    bool stillInIni = false;  ///< migracja się nie udała — wartość nadal leży jawnym tekstem w ini
};

/// Wartość z sejfu pod kontem `account`. Gdy w ini leży stara wartość pod `legacyIniKey`: zapis do
/// sejfu, odczyt zwrotny i dopiero wtedy usunięcie z ini — awaria sejfu nie gubi sekretu.
ReadResult read(const QString &account, const QString &legacyIniKey);

/// Zapis w sejfie, pusta wartość usuwa wpis. Nigdy nie pisze do ini. Starą wartość z ini usuwa
/// zawsze — także po nieudanym zapisie, bo inaczej przy następnym starcie migracja przywróciłaby ją
/// po cichu zamiast nowej.
bool write(const QString &account, const QString &legacyIniKey, const QString &value,
           QString *keychainError = nullptr);
}  // namespace SecretStore

#endif // SECRETSTORE_H
