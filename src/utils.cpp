/**
 * @file utils.cpp
 * @brief Implementacja funkcji pomocniczej `setupDatabase` odpowiedzialnej za konfigurację połączenia z bazą danych w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Plik zawiera implementację funkcji `setupDatabase`, która konfiguruje
 * połączenie z bazą danych typu SQLite lub MySQL, zapisując je pod stałą nazwą
 * „default_connection”. Obsługuje parametry połączenia lokalnego i zdalnego.
 */

#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>

/**
 * @brief Inicjalizuje połączenie z bazą danych, zapisując je pod nazwą "default_connection".
 *
 * Obsługiwane typy:
 * - `"SQLite3"`: korzysta z drivera `QSQLITE`; `dbSource` to ścieżka do pliku bazy.
 * - `"MySQL"`: korzysta z drivera `QMYSQL`; `dbSource` to nazwa bazy danych;
 *    opcjonalnie podaje się `host`, `user`, `password` oraz `port`.
 *
 * Jeśli połączenie o nazwie `default_connection` już istnieje, zostaje ponownie użyte.
 * W przeciwnym razie zostaje utworzone nowe połączenie z odpowiednim konfiguratorem.
 *
 * @param dbType Typ bazy danych ("SQLite3" lub "MySQL").
 * @param dbSource Dla SQLite – ścieżka do pliku; dla MySQL – nazwa bazy danych.
 * @param host (opcjonalnie) Adres hosta dla MySQL (np. "localhost").
 * @param user (opcjonalnie) Nazwa użytkownika.
 * @param password (opcjonalnie) Hasło użytkownika.
 * @param port (opcjonalnie) Numer portu (dla MySQL; domyślnie 3306).
 * @return `true`, jeśli połączenie zostało nawiązane pomyślnie; w przeciwnym razie `false`.
 */
bool setupDatabase(const QString &dbType,
                   const QString &dbSource,
                   const QString &host,
                   const QString &user,
                   const QString &password,
                   int port)
{
    QSqlDatabase db;

    // Sprawdzamy, czy połączenie już istnieje
    if (QSqlDatabase::contains("default_connection"))
        db = QSqlDatabase::database("default_connection");
    else {
        if(dbType == "SQLite3") {
            db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");
            db.setDatabaseName(dbSource);
        } else if(dbType == "MySQL") {
            db = QSqlDatabase::addDatabase("QMYSQL", "default_connection");
            db.setHostName(host);
            db.setDatabaseName(dbSource);
            db.setUserName(user);
            db.setPassword(password);
            if(port != 0)
                db.setPort(port);
        } else {
            QMessageBox::critical(nullptr, QObject::tr("Błąd konfiguracji"),
                                  QObject::tr("Nieznany typ bazy danych: %1").arg(dbType));
            return false;
        }
    }

    // Próba otwarcia połączenia
    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Błąd połączenia"),
                              db.lastError().text());
        return false;
    }

    return true;
}
