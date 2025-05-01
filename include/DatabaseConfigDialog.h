/**
 * @file DatabaseConfigDialog.h
 * @brief Deklaracja klasy DatabaseConfigDialog do konfiguracji połączenia z bazą danych oraz wyboru skórki graficznej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.2.0
 * @date 2025-05-01
 *
 * Klasa DatabaseConfigDialog reprezentuje okno dialogowe umożliwiające użytkownikowi wybór typu
 * bazy danych (SQLite lub MySQL), wprowadzenie parametrów połączenia (ścieżka do pliku SQLite
 * lub dane dostępowe MySQL) oraz wybór skórki graficznej aplikacji (Amiga, ZX Spectrum, Standard).
 * Klasa przechowuje wprowadzone dane, obsługuje zmianę stylów i czcionek, a także zapisuje ustawienia
 * w QSettings dla trwałości między sesjami aplikacji.
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
 * @brief Okno dialogowe do konfiguracji połączenia z bazą danych i wyboru skórki graficznej.
 *
 * Klasa DatabaseConfigDialog pozwala użytkownikowi skonfigurować parametry połączenia z bazą danych
 * oraz wybrać skórkę graficzną aplikacji. Po zatwierdzeniu danych przez użytkownika, parametry są
 * zapisywane w QSettings i dostępne poprzez metody dostępowe. Obsługuje dynamiczne przełączanie
 * interfejsu w zależności od wybranego typu bazy danych oraz zmianę stylów i czcionek aplikacji.
 */
class DatabaseConfigDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy DatabaseConfigDialog.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * Inicjalizuje okno dialogowe, ustawia interfejs użytkownika, ładuje dostępne skórki
     * i konfiguruje początkowe ustawienia na podstawie QSettings.
     */
    explicit DatabaseConfigDialog(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy DatabaseConfigDialog.
     *
     * Zwalnia zasoby interfejsu użytkownika oraz odrejestrowuje załadowane czcionki.
     */
    ~DatabaseConfigDialog();

    /**
     * @brief Zwraca wybrany typ bazy danych.
     * @return QString zawierający typ bazy danych (np. "SQLite3" lub "MySQL").
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
     * połączenia z bazą danych oraz wybraną skórkę graficzną w QSettings.
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

    /**
     * @brief Obsługuje zmianę skórki graficznej.
     * @param skin Nazwa wybranej skórki (np. "Amiga", "ZX Spectrum", "Standard").
     *
     * Ładuje odpowiedni plik QSS, ustawia odpowiednią czcionkę i aktualizuje interfejs aplikacji.
     */
    void onSkinChanged(const QString &skin);

private:
    /**
     * @brief Ładuje arkusz stylów dla wybranej skórki.
     * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
     *
     * Wczytuje plik QSS z zasobów Qt i stosuje go do aplikacji za pomocą qApp->setStyleSheet.
     */
    void loadStyleSheet(const QString &skin);

    /**
     * @brief Ładuje czcionkę dla wybranej skórki.
     * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
     *
     * Ładuje odpowiednią czcionkę z zasobów Qt (topaz.ttf dla Amigi, zxspectrum.ttf dla ZX Spectrum
     * lub domyślną czcionkę systemową dla Standard) i ustawia ją dla aplikacji.
     */
    void loadFont(const QString &skin);

    /// Wskaźnik na obiekt interfejsu użytkownika.
    Ui::DatabaseConfigDialog *ui;

    /// ID czcionki Topaz załadowanej z zasobów.
    int m_topazFontId;

    /// ID czcionki ZX Spectrum załadowanej z zasobów.
    int m_zxFontId;

    /// ID czcionki ZX Spectrum załadowanej z zasobów.
    int m_atari8bitFontId;
};

#endif // DATABASECONFIGDIALOG_H
