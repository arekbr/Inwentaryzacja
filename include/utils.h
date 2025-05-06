/**
 * @file utils.h
 * @brief Deklaracja funkcji pomocniczych wykorzystywanych w aplikacji inwentaryzacyjnej.
 * @version \projectnumber
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera deklarację funkcji `setupDatabase`, odpowiedzialnej za inicjalizację
 * połączenia z bazą danych (SQLite lub MySQL) z wykorzystaniem parametrów takich jak
 * typ bazy, źródło danych, host, port i dane uwierzytelniające. Funkcja jest kluczowym
 * elementem konfiguracji bazy danych w aplikacji inwentaryzacyjnej, ustanawiając globalne
 * połączenie o nazwie "default_connection", używane przez inne komponenty aplikacji.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację funkcji `setupDatabase`** – konfiguruje i otwiera połączenie z bazą danych.
 * 2. **Zależności** – minimalne, obejmuje tylko `QString` z Qt.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klasy `QString` dla parametrów funkcji.
 * - **Nagłówki Qt**: Pośrednio zależne od `QSqlDatabase` (w `utils.cpp`).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Funkcja `setupDatabase` jest wykorzystywana na etapie uruchamiania aplikacji, np. w `DatabaseConfigDialog`.
 * - Obsługuje zarówno SQLite (lokalny plik bazy), jak i MySQL (zdalne połączenie).
 */

#ifndef UTILS_H
#define UTILS_H

#include <QString>

/**
 * @brief Inicjalizuje połączenie z bazą danych na podstawie podanych parametrów.
 *
 * @section FunctionOverview
 * Funkcja tworzy i rejestruje globalne połączenie `QSqlDatabase` pod nazwą "default_connection".
 * Obsługuje różne typy baz danych (SQLite poprzez `QSQLITE`, MySQL poprzez `QMYSQL`) i umożliwia
 * konfigurację połączenia lokalnego (plik SQLite) lub zdalnego (MySQL z hostem, portem, użytkownikiem).
 * W przypadku SQLite automatycznie tworzy schemat bazy danych i wypełnia przykładowymi danymi.
 *
 * @param dbType Typ bazy danych ("SQLite3" lub "MySQL").
 * @param dbSource Ścieżka do pliku bazy (SQLite) lub nazwa bazy danych (MySQL).
 * @param host Adres hosta dla połączeń MySQL (np. "localhost", opcjonalny).
 * @param user Nazwa użytkownika dla MySQL (opcjonalny).
 * @param password Hasło użytkownika dla MySQL (opcjonalny).
 * @param port Numer portu dla MySQL (domyślnie 3306, opcjonalny).
 * @return true jeśli połączenie zostało pomyślnie nawiązane, false w przypadku błędu.
 */
bool setupDatabase(const QString &dbType,
                   const QString &dbSource,
                   const QString &host = QString(),
                   const QString &user = QString(),
                   const QString &password = QString(),
                   int port = 0);

#endif // UTILS_H
