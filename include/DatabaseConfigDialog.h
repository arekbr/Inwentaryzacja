/**
 * @file DatabaseConfigDialog.h
 * @brief Deklaracja klasy DatabaseConfigDialog do konfiguracji połączenia z bazą danych oraz wyboru skórki graficznej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025- istituti-03
 *
 * @section Overview
 * Plik DatabaseConfigDialog.h definiuje klasę DatabaseConfigDialog, która reprezentuje okno dialogowe służące do konfiguracji parametrów połączenia z bazą danych (SQLite lub MySQL) oraz wyboru skórki graficznej aplikacji (Amiga, Atari 8bit, ZX Spectrum, Standard). Klasa umożliwia zapis ustawień w QSettings, dynamiczną zmianę stylów i czcionek oraz zapewnia metody dostępowe do parametrów połączenia z bazą danych.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Definicję klasy DatabaseConfigDialog** – dziedziczy po QDialog, zawiera publiczne metody dostępowe, sloty oraz prywatne metody i zmienne.
 * 2. **Deklaracje metod publicznych** – konstruktor, destruktor oraz metody zwracające parametry bazy danych.
 * 3. **Deklaracje slotów** – obsługa zatwierdzania danych i zmiany typu bazy danych lub skórki graficznej.
 * 4. **Deklaracje metod prywatnych** – ładowanie arkuszy stylów i czcionek dla wybranych skórek.
 * 5. **Zmienne prywatne** – wskaźnik na interfejs użytkownika oraz identyfikatory załadowanych czcionek.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QDialog, QString oraz mechanizmów sygnałów i slotów.
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_DatabaseConfigDialog.h).
 * - **Zasoby**: Pliki QSS dla stylów (amiga.qss, atari8bit.qss, zxspectrum.qss, default.qss) oraz czcionki (topaz.ttf, zxspectrum.ttf, EightBit Atari-Ataripl.ttf).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dokumentacja i komentarze dodano bez ingerencji w oryginalny kod.
 * - Klasa obsługuje trwałe przechowywanie ustawień za pomocą QSettings.
 * - Interfejs użytkownika dynamicznie dostosowuje się do wybranego typu bazy danych (SQLite/MySQL) za pomocą QStackedWidget.
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
 * @section ClassOverview
 * Klasa DatabaseConfigDialog dziedziczy po QDialog i dostarcza interfejs graficzny do wprowadzania parametrów połączenia z bazą danych oraz wyboru skórki graficznej. Obsługuje dwa typy baz danych (SQLite i MySQL) oraz cztery skórki graficzne (Amiga, Atari 8bit, ZX Spectrum, Standard). Po zatwierdzeniu ustawienia są zapisywane w QSettings, a metody dostępowe pozwalają na pobranie parametrów w innych częściach aplikacji.
 *
 * @section Responsibilities
 * - Wyświetlanie i zarządzanie oknem dialogowym z polami do konfiguracji bazy danych.
 * - Dynamiczne przełączanie interfejsu w zależności od typu bazy danych.
 * - Ładowanie i stosowanie arkuszy stylów (QSS) oraz czcionek dla wybranych skórek.
 * - Trwałe przechowywanie ustawień w pliku inwentaryzacja.ini.
 */
class DatabaseConfigDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy DatabaseConfigDialog.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * @section ConstructorOverview
     * Inicjalizuje okno dialogowe, konfiguruje interfejs użytkownika (ui), ładuje dostępne skórki graficzne, ustawia opcje combo boxów i wczytuje zapisane ustawienia z QSettings. Ustawia połączenia sygnałów i slotów dla dynamicznej interakcji z użytkownikiem.
     */
    explicit DatabaseConfigDialog(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy DatabaseConfigDialog.
     *
     * @section DestructorOverview
     * Zwalnia zasoby interfejsu użytkownika (ui) oraz usuwa załadowane czcionki z QFontDatabase, aby zapobiec wyciekom pamięci.
     */
    ~DatabaseConfigDialog();

    /**
     * @brief Zwraca wybrany typ bazy danych.
     * @return QString zawierający typ bazy danych (np. "SQLite3" lub "MySQL").
     *
     * @section MethodOverview
     * Pobiera aktualnie wybrany typ bazy danych z combo boxa dbTypeComboBox.
     */
    QString selectedDatabaseType() const;

    /**
     * @brief Zwraca ścieżkę do pliku bazy SQLite.
     * @return QString zawierający ścieżkę do pliku SQLite.
     *
     * @section MethodOverview
     * Zwraca ścieżkę do pliku bazy SQLite wprowadzoną w polu sqlitePathLineEdit.
     */
    QString sqliteFilePath() const;

    /**
     * @brief Zwraca adres hosta dla bazy MySQL.
     * @return QString zawierający adres hosta MySQL.
     *
     * @section MethodOverview
     * Zwraca adres hosta MySQL wprowadzony w polu hostLineEdit.
     */
    QString mysqlHost() const;

    /**
     * @brief Zwraca nazwę bazy danych MySQL.
     * @return QString zawierający nazwę bazy danych MySQL.
     *
     * @section MethodOverview
     * Zwraca nazwę bazy danych MySQL wprowadzoną w polu databaseLineEdit.
     */
    QString mysqlDatabase() const;

    /**
     * @brief Zwraca nazwę użytkownika dla bazy MySQL.
     * @return QString zawierający nazwę użytkownika MySQL.
     *
     * @section MethodOverview
     * Zwraca nazwę użytkownika MySQL wprowadzoną w polu userLineEdit.
     */
    QString mysqlUser() const;

    /**
     * @brief Zwraca hasło użytkownika dla bazy MySQL.
     * @return QString zawierający hasło użytkownika MySQL.
     *
     * @section MethodOverview
     * Zwraca hasło użytkownika MySQL wprowadzone w polu passwordLineEdit.
     */
    QString mysqlPassword() const;

    /**
     * @brief Zwraca numer portu dla bazy MySQL.
     * @return int zawierający numer portu MySQL.
     *
     * @section MethodOverview
     * Zwraca numer portu MySQL wprowadzony w polu portSpinBox.
     */
    int mysqlPort() const;

public slots:
    /**
     * @brief Zatwierdza wprowadzone dane i zamyka okno dialogowe.
     *
     * @section SlotOverview
     * Przesłania metodę accept() klasy QDialog. Zapisuje wszystkie wprowadzone parametry (typ bazy danych, ścieżka SQLite, dane MySQL, wybrana skórka) w QSettings i zamyka okno dialogowe z wynikiem QDialog::Accepted.
     */
    void accept() override;

private slots:
    /**
     * @brief Obsługuje zmianę typu bazy danych w combo boxie.
     * @param index Indeks wybranego typu bazy danych.
     *
     * @section SlotOverview
     * Aktualizuje widoczność pól w interfejsie użytkownika (QStackedWidget) w zależności od wybranego typu bazy danych (SQLite lub MySQL).
     */
    void onDatabaseTypeChanged(int index);

    /**
     * @brief Obsługuje zmianę skórki graficznej.
     * @param skin Nazwa wybranej skórki (np. "Amiga", "ZX Spectrum", "Standard").
     *
     * @section SlotOverview
     * Wywołuje metody loadStyleSheet i loadFont dla wybranej skórki, aktualizuje interfejs aplikacji i zapisuje wybór w QSettings.
     */
    void onSkinChanged(const QString &skin);

private:
    /**
     * @brief Ładuje arkusz stylów dla wybranej skórki.
     * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
     *
     * @section MethodOverview
     * Wczytuje odpowiedni plik QSS z zasobów Qt (np. :/styles/amiga.qss) i stosuje go do aplikacji za pomocą qApp->setStyleSheet. W przypadku błędu loguje ostrzeżenie.
     */
    void loadStyleSheet(const QString &skin);

    /**
     * @brief Ładuje czcionkę dla wybranej skórki.
     * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
     *
     * @section MethodOverview
     * Ładuje odpowiednią czcionkę z zasobów Qt (np. topaz.ttf dla Amigi) za pomocą QFontDatabase i ustawia ją dla aplikacji za pomocą qApp->setFont. W przypadku Standard używa domyślnej czcionki systemowej.
     */
    void loadFont(const QString &skin);

    /// Wskaźnik na obiekt interfejsu użytkownika.
    Ui::DatabaseConfigDialog *ui;

    /// ID czcionki Topaz załadowanej z zasobów.
    int m_topazFontId;

    /// ID czcionki ZX Spectrum załadowanej z zasobów.
    int m_zxFontId;

    /// ID czcionki Atari 8bit załadowanej z zasobów.
    int m_atari8bitFontId;
};

#endif // DATABASECONFIGDIALOG_H
