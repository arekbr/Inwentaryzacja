/**
 * @file utils.cpp
 * @brief Implementacja funkcji pomocniczej `setupDatabase` odpowiedzialnej za konfigurację połączenia z bazą danych w aplikacji inwentaryzacyjnej.
 * @version \projectnumber
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera implementację funkcji `setupDatabase`, która konfiguruje i otwiera połączenie
 * z bazą danych typu SQLite lub MySQL, zapisując je pod nazwą "default_connection". Funkcja
 * obsługuje parametry dla połączeń lokalnych (SQLite) i zdalnych (MySQL), a dla SQLite
 * automatycznie tworzy schemat bazy danych i wypełnia go przykładowymi danymi słownikowymi.
 *
 * @section Structure
 * Kod zawiera:
 * 1. **Implementację funkcji `setupDatabase`** – konfiguruje połączenie, otwiera bazę i tworzy schemat dla SQLite.
 * 2. **Tworzenie schematu SQLite** – definiuje tabele i dane początkowe.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas `QMessageBox`, `QSqlDatabase`, `QSqlError`, `QSqlQuery`, `QUuid`.
 * - **Nagłówki aplikacji**: Brak (tylko Qt).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Funkcja jest kluczowa dla inicjalizacji bazy danych w aplikacji, wywoływana np. przez `DatabaseConfigDialog`.
 * - Schemat SQLite obejmuje tabele: `eksponaty`, `types`, `vendors`, `models`, `statuses`, `storage_places`, `photos`.
 * - Przykładowe dane słownikowe obejmują typowe wartości dla retro sprzętu komputerowego.
 */

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

/**
 * @brief Inicjalizuje połączenie z bazą danych, zapisując je pod nazwą "default_connection".
 *
 * @section FunctionOverview
 * Funkcja konfiguruje połączenie z bazą danych SQLite (`QSQLITE`) lub MySQL (`QMYSQL`).
 * Dla SQLite `dbSource` to ścieżka do pliku bazy, dla MySQL to nazwa bazy danych.
 * Jeśli połączenie "default_connection" istnieje, jest usuwane i tworzone od nowa.
 * Dla SQLite automatycznie tworzy schemat bazy (tabele i dane początkowe) jeśli nie istnieje.
 *
 * @param dbType Typ bazy danych ("SQLite3" lub "MySQL").
 * @param dbSource Dla SQLite – ścieżka do pliku; dla MySQL – nazwa bazy danych.
 * @param host Adres hosta dla MySQL (np. "localhost", opcjonalny).
 * @param user Nazwa użytkownika dla MySQL (opcjonalny).
 * @param password Hasło użytkownika dla MySQL (opcjonalny).
 * @param port Numer portu dla MySQL (domyślnie 3306, opcjonalny).
 * @return `true` jeśli połączenie zostało nawiązane pomyślnie; `false` w przypadku błędu.
 */
bool setupDatabase(const QString &dbType,
                   const QString &dbSource,
                   const QString &host,
                   const QString &user,
                   const QString &password,
                   int port)
{
    // Usunięcie istniejącego połączenia o nazwie "default_connection"
    QSqlDatabase::removeDatabase("default_connection");
    QSqlDatabase db = QSqlDatabase::addDatabase(dbType.compare("MySQL", Qt::CaseInsensitive) == 0
                                                    ? "QMYSQL"
                                                    : "QSQLITE",
                                                "default_connection");

    // Konfiguracja parametrów połączenia
    if (dbType.compare("MySQL", Qt::CaseInsensitive) == 0) {
        db.setHostName(host);
        db.setDatabaseName(dbSource);
        db.setUserName(user);
        db.setPassword(password);
        db.setPort(port);
    } else {
        // SQLite
        db.setDatabaseName(dbSource);
    }

    // Próba otwarcia połączenia
    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Błąd połączenia"), db.lastError().text());
        return false;
    }

    // --- AUTOMATYCZNE TWORZENIE SCHEMATU DLA SQLITE ---
    if (dbType.startsWith("SQLite", Qt::CaseInsensitive)) {
        QStringList needed
            = {"eksponaty", "types", "vendors", "models", "statuses", "storage_places", "photos"};
        QStringList existing = db.tables();
        bool missing = false;
        for (const QString &t : needed) {
            if (!existing.contains(t, Qt::CaseInsensitive)) {
                missing = true;
                break;
            }
        }
        if (missing) {
            QSqlQuery q(db);
            // Tabele
            q.exec(R"(
                CREATE TABLE IF NOT EXISTS eksponaty (
                  id TEXT PRIMARY KEY,
                  name TEXT NOT NULL,
                  type_id TEXT NOT NULL,
                  vendor_id TEXT NOT NULL,
                  model_id TEXT NOT NULL,
                  serial_number TEXT,
                  part_number TEXT,
                  revision TEXT,
                  production_year INTEGER,
                  status_id TEXT NOT NULL,
                  storage_place_id TEXT NOT NULL,
                  description TEXT,
                  value INTEGER
                )
            )");
            q.exec(R"(
                CREATE TABLE IF NOT EXISTS types (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )");
            q.exec(R"(
                CREATE TABLE IF NOT EXISTS vendors (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )");
            q.exec(R"(
                CREATE TABLE IF NOT EXISTS models (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL,
                  vendor_id TEXT NOT NULL
                )
            )");
            q.exec(R"(
                CREATE TABLE IF NOT EXISTS statuses (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )");
            q.exec(R"(
                CREATE TABLE IF NOT EXISTS storage_places (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )");
            q.exec(R"(
                CREATE TABLE IF NOT EXISTS photos (
                  id TEXT PRIMARY KEY,
                  eksponat_id TEXT NOT NULL,
                  photo BLOB NOT NULL
                )
            )");

            // Przykładowe dane słownikowe
            auto genId = []() { return QUuid::createUuid().toString(QUuid::WithoutBraces); };
            // types
            QString t1 = genId(), t2 = genId(), t3 = genId();
            q.exec(QString("INSERT OR IGNORE INTO types(id,name) VALUES('%1','Komputer')").arg(t1));
            q.exec(QString("INSERT OR IGNORE INTO types(id,name) VALUES('%1','Monitor')").arg(t2));
            q.exec(QString("INSERT OR IGNORE INTO types(id,name) VALUES('%1','Kabel')").arg(t3));
            // vendors
            QString v1 = genId(), v2 = genId(), v3 = genId();
            q.exec(QString("INSERT OR IGNORE INTO vendors(id,name) VALUES('%1','Atari')").arg(v1));
            q.exec(
                QString("INSERT OR IGNORE INTO vendors(id,name) VALUES('%1','Commodore')").arg(v2));
            q.exec(
                QString("INSERT OR IGNORE INTO vendors(id,name) VALUES('%1','Sinclair')").arg(v3));
            // models
            QString m1 = genId(), m2 = genId(), m3 = genId();
            q.exec(QString("INSERT OR IGNORE INTO models(id,name,vendor_id) "
                           "VALUES('%1','Atari 800XL','%2')")
                       .arg(m1, v1));
            q.exec(QString("INSERT OR IGNORE INTO models(id,name,vendor_id) "
                           "VALUES('%1','Amiga 500','%2')")
                       .arg(m2, v2));
            q.exec(QString("INSERT OR IGNORE INTO models(id,name,vendor_id) "
                           "VALUES('%1','ZX Spectrum','%2')")
                       .arg(m3, v3));
            // statuses
            QString s1 = genId(), s2 = genId(), s3 = genId();
            q.exec(
                QString("INSERT OR IGNORE INTO statuses(id,name) VALUES('%1','Sprawny')").arg(s1));
            q.exec(QString("INSERT OR IGNORE INTO statuses(id,name) VALUES('%1','Uszkodzony')")
                       .arg(s2));
            q.exec(QString("INSERT OR IGNORE INTO statuses(id,name) VALUES('%1','W naprawie')")
                       .arg(s3));
            // storage_places
            QString sp1 = genId(), sp2 = genId();
            q.exec(QString("INSERT OR IGNORE INTO storage_places(id,name) "
                           "VALUES('%1','Magazyn 1')")
                       .arg(sp1));
            q.exec(QString("INSERT OR IGNORE INTO storage_places(id,name) "
                           "VALUES('%1','Półka B3')")
                       .arg(sp2));
        }
    }

    return true;
}
