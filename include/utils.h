/**
 * @file utils.h
 * @brief Deklaracja funkcji pomocniczych wykorzystywanych w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Plik zawiera deklarację funkcji `setupDatabase`, służącej do inicjalizacji
 * połączenia z bazą danych (np. SQLite, MySQL) z wykorzystaniem parametrów takich jak typ,
 * źródło danych, host, port i dane uwierzytelniające.
 *
 * Funkcja ta może być wywoływana na etapie uruchamiania aplikacji w celu przygotowania
 * globalnego połączenia `QSqlDatabase` o nazwie "default_connection".
 */

#ifndef UTILS_H
#define UTILS_H

#include <QString>

/**
 * @brief Inicjalizuje połączenie z bazą danych na podstawie podanych parametrów.
 *
 * Funkcja tworzy i rejestruje globalne połączenie `QSqlDatabase` pod nazwą „default_connection”.
 * Obsługuje różne types baz danych (np. QSQLITE, QMYSQL) i umożliwia konfigurację połączenia
 * zarówno lokalnego (plik), jak i zdalnego (host, port, login, hasło).
 *
 * @param dbType Typ bazy danych (np. "QSQLITE", "QMYSQL").
 * @param dbSource Nazwa pliku bazy danych lub nazwa zdalnej bazy (w zależności od typu).
 * @param host (opcjonalnie) Adres hosta dla połączeń zdalnych (np. "localhost").
 * @param user (opcjonalnie) Nazwa użytkownika dla połączeń wymagających logowania.
 * @param password (opcjonalnie) Hasło użytkownika.
 * @param port (opcjonalnie) Numer portu (dla MySQL domyślnie 3306).
 * @return true jeśli połączenie zostało pomyślnie nawiązane, false w przypadku błędu.
 */
bool setupDatabase(const QString &dbType,
                   const QString &dbSource,
                   const QString &host = QString(),
                   const QString &user = QString(),
                   const QString &password = QString(),
                   int port = 0);

#endif // UTILS_H
