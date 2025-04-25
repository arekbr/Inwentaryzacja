/**
 * @file DatabaseConfigDialog.h
 * @brief Deklaracja klasy DatabaseConfigDialog do konfiguracji połączenia z bazą danych.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Klasa DatabaseConfigDialog reprezentuje okno dialogowe umożliwiające użytkownikowi wybór typu
 * bazy danych (SQLite lub MySQL) oraz wprowadzenie parametrów połączenia, takich jak ścieżka do pliku
 * SQLite lub dane dostępowe MySQL (host, nazwa bazy, użytkownik, hasło, port). Klasa przechowuje
 * wprowadzone dane i udostępnia je poprzez metody dostępowe.
 */

#ifndef DATABASECONFIGDIALOG_H
#define DATABASECONFIGDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class DatabaseConfigDialog;
}

/**
 * @class DatabaseConfigDialog
 * @brief Okno dialogowe do konfiguracji połączenia z bazą danych.
 *
 * Klasa DatabaseConfigDialog pozwala użytkownikowi wybrać typ bazy danych (SQLite lub MySQL)
 * i skonfigurować parametry połączenia. Po zatwierdzeniu danych przez użytkownika, parametry
 * są dostępne poprzez metody dostępowe. Obsługuje dynamiczne przełączanie interfejsu w zależności
 * od wybranego typu bazy danych.
 */
class DatabaseConfigDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy DatabaseConfigDialog.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * Inicjalizuje okno dialogowe i ustawia interfejs użytkownika.
     */
    explicit DatabaseConfigDialog(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy DatabaseConfigDialog.
     *
     * Zwalnia zasoby interfejsu użytkownika.
     */
    ~DatabaseConfigDialog();

    /**
     * @brief Zwraca wybrany typ bazy danych.
     * @return QString zawierający typ bazy danych (np. "SQLite" lub "MySQL").
     */
    QString selectedDatabaseType() const;

    /**
     * @brief Zwraca ścieżkę do pliku bazy SQLite.
     * @return QString zawierający ścieżkę do pliku SQLite.
     */
    QString sqliteFilePath() const;

    /**
     * @brief Zwraca adres hosta dla bazy MySQL.
     * @return QString zawierający adres hosta MySQL.
     */
    QString mysqlHost() const;

    /**
     * @brief Zwraca nazwę bazy danych MySQL.
     * @return QString zawierający nazwę bazy danych MySQL.
     */
    QString mysqlDatabase() const;

    /**
     * @brief Zwraca nazwę użytkownika dla bazy MySQL.
     * @return QString zawierający nazwę użytkownika MySQL.
     */
    QString mysqlUser() const;

    /**
     * @brief Zwraca hasło użytkownika dla bazy MySQL.
     * @return QString zawierający hasło użytkownika MySQL.
     */
    QString mysqlPassword() const;

    /**
     * @brief Zwraca numer portu dla bazy MySQL.
     * @return int zawierający numer portu MySQL.
     */
    int mysqlPort() const;

public slots:
    /**
     * @brief Zatwierdza wprowadzone dane i zamyka okno dialogowe.
     *
     * Przesłania metodę accept() klasy QDialog, zapisując wprowadzone parametry
     * i zamykając okno dialogowe.
     */
    void accept() override;

private slots:
    /**
     * @brief Obsługuje zmianę typu bazy danych w combo boxie.
     * @param index Indeks wybranego typu bazy danych.
     *
     * Aktualizuje widoczność pól w interfejsie użytkownika w zależności od wybranego
     * typu bazy danych (SQLite lub MySQL).
     */
    void onDatabaseTypeChanged(int index);

private:
    /// Wskaźnik na obiekt interfejsu użytkownika.
    Ui::DatabaseConfigDialog *ui;
};

#endif // DATABASECONFIGDIALOG_H
