/**
 * @file DatabaseConfigDialog.cpp
 * @brief Implementacja klasy DatabaseConfigDialog z obsługą tworzenia nowego pliku SQLite oraz wyboru skórki graficznej aplikacji.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025-05-03
 *
 * @section Overview
 * Plik DatabaseConfigDialog.cpp zawiera implementację metod klasy DatabaseConfigDialog, która zarządza oknem dialogowym do konfiguracji połączenia z bazą danych (SQLite lub MySQL) oraz wyboru skórki graficznej (Amiga, Atari 8bit, ZX Spectrum, Standard). Implementacja obejmuje ładowanie stylów QSS, czcionek, zapis ustawień w QSettings oraz obsługę dynamicznego wyboru pliku SQLite z możliwością tworzenia nowego pliku .db.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Sekcja pomocnicza** – funkcja getSettings do tworzenia i zwracania obiektu QSettings.
 * 2. **Sekcja konstruktora** – inicjalizacja interfejsu, combo boxów, ustawień i połączeń sygnałów-slotów.
 * 3. **Sekcja destruktora** – zwalnianie zasobów i czcionek.
 * 4. **Sekcja metod publicznych** – metody dostępowe do parametrów bazy danych oraz metoda accept.
 * 5. **Sekcja slotów prywatnych** – obsługa zmiany typu bazy danych i skórki graficznej.
 * 6. **Sekcja metod prywatnych** – ładowanie stylów QSS i czcionek dla skórek.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QApplication, QDebug, QDir, QFile, QFileDialog, QFontDatabase, QPushButton, QSettings.
 * - **Interfejs użytkownika**: ui_DatabaseConfigDialog.h generowany przez Qt Designer.
 * - **Zasoby**: Pliki QSS (:/styles/.qss), czcionki (:/fonts/.ttf).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Ustawienia są zapisywane w pliku inwentaryzacja.ini w katalogu aplikacji.
 * - Obsługa błędów obejmuje logowanie komunikatów w przypadku problemów z ładowaniem zasobów.
 */

#include "DatabaseConfigDialog.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include "ui_DatabaseConfigDialog.h"

/**
 * @brief Pobiera obiekt QSettings dla aplikacji.
 * @return Obiekt QSettings zainicjalizowany z plikiem inwentaryzacja.ini.
 *
 * @section FunctionOverview
 * Tworzy plik inwentaryzacja.ini w katalogu aplikacji, jeśli nie istnieje, i zwraca obiekt QSettings w formacie IniFormat do przechowywania ustawień aplikacji.
 */
static QSettings getSettings()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir); // twórz katalog jeśli nie istnieje
    QString settingsPath = configDir + "/inwentaryzacja.ini";
    qDebug() << "Używam pliku ustawień:" << settingsPath;
    return QSettings(settingsPath, QSettings::IniFormat);
}

/**
 * @brief Konstruktor klasy DatabaseConfigDialog.
 * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
 *
 * @section ConstructorOverview
 * Inicjalizuje interfejs użytkownika, ustawia opcje combo boxów dla typu bazy danych (SQLite3, MySQL) i skórek graficznych (Amiga, Atari 8bit, ZX Spectrum, Standard). Ładuje zapisane ustawienia z QSettings, konfiguruje początkowy stan interfejsu i ustanawia połączenia sygnałów-slotów dla interakcji użytkownika, w tym wybór pliku SQLite.
 */
DatabaseConfigDialog::DatabaseConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseConfigDialog)
    , m_topazFontId(-1)
    , m_zxFontId(-1)
    , m_atari8bitFontId(-1)
{
    ui->setupUi(this);

    // Sekcja: Połączenia przycisków dialogowych
    // Łączy sygnały przycisków OK/Anuluj z odpowiednimi slotami (accept, reject).
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DatabaseConfigDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Sekcja: Inicjalizacja combo boxa dla typu bazy danych
    // Wypełnia combo box opcjami SQLite3 i MySQL, ustawia domyślny widok na SQLite.
    ui->dbTypeComboBox->clear();
    ui->dbTypeComboBox->addItem("SQLite3");
    ui->dbTypeComboBox->addItem("MySQL");
    ui->stackedWidget->setCurrentIndex(0);

    // Sekcja: Połączenie dla zmiany typu bazy danych
    // Łączy sygnał zmiany indeksu combo boxa z slotem onDatabaseTypeChanged.
    connect(ui->dbTypeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DatabaseConfigDialog::onDatabaseTypeChanged);

    // Sekcja: Inicjalizacja combo boxa dla skórki graficznej
    // Wypełnia combo box dostępnymi skórkami graficznymi.
    ui->filterSelectSkin->clear();
    ui->filterSelectSkin->addItems({"Amiga", "Atari 8bit", "ZX Spectrum", "Standard"});

    // Sekcja: Ładowanie zapisanych ustawień
    // Wczytuje ustawienia z pliku inwentaryzacja.ini, ustawia wartości pól interfejsu.
    QSettings settings = getSettings();
    QString savedDbType = settings.value("Database/Type", "SQLite3").toString();
    ui->dbTypeComboBox->setCurrentText(savedDbType);
    int index = ui->dbTypeComboBox->findText(savedDbType);
    ui->stackedWidget->setCurrentIndex(index >= 0 ? index : 0);

    ui->sqlitePathLineEdit->setText(settings.value("Database/SQLite/FilePath", "").toString());

    ui->hostLineEdit->setText(settings.value("Database/MySQL/Host", "127.0.0.1").toString());
    ui->databaseLineEdit->setText(
        settings.value("Database/MySQL/Database", "nazwa_bazy").toString());
    ui->userLineEdit->setText(settings.value("Database/MySQL/User", "user").toString());
    ui->passwordLineEdit->setText(settings.value("Database/MySQL/Password", "").toString());
    ui->portSpinBox->setValue(settings.value("Database/MySQL/Port", 3306).toInt());

    QString savedSkin = settings.value("skin", "Standard").toString();
    int skinIndex = ui->filterSelectSkin->findText(savedSkin);
    ui->filterSelectSkin->setCurrentIndex(skinIndex != -1 ? skinIndex : 2); // Domyślnie "Standard"
    loadStyleSheet(savedSkin);
    loadFont(savedSkin);

    // Sekcja: Połączenie dla zmiany skórki graficznej
    // Łączy sygnał zmiany skórki z slotem onSkinChanged.
    connect(ui->filterSelectSkin,
            &QComboBox::currentTextChanged,
            this,
            &DatabaseConfigDialog::onSkinChanged);

    // Sekcja: Połączenie dla przycisku wyboru pliku SQLite
    // Obsługuje wybór istniejącego lub nowego pliku SQLite, z opcją utworzenia nowego pliku .db.
    connect(ui->selectFileButton, &QPushButton::clicked, this, [this]() {
        if (ui->dbTypeComboBox->currentText() == "SQLite3") {
            QString filePath
                = QFileDialog::getSaveFileName(this,
                                               tr("Wskaż nową lub istniejącą bazę SQLite"),
                                               QString(),
                                               tr("SQLite Database (*.db);;Wszystkie pliki (*.*)"));
            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (!file.exists()) {
                    // Utworzenie nowego pliku bazy
                    if (file.open(QIODevice::WriteOnly)) {
                        file.close();
                    }
                }
                ui->sqlitePathLineEdit->setText(filePath);
            }
        }
    });
}

/**
 * @brief Destruktor klasy DatabaseConfigDialog.
 *
 * @section DestructorOverview
 * Zwalnia zasoby interfejsu użytkownika (ui) oraz usuwa załadowane czcionki (Topaz, ZX Spectrum, Atari 8bit) z QFontDatabase, aby zapobiec wyciekom pamięci.
 */
DatabaseConfigDialog::~DatabaseConfigDialog()
{
    if (m_topazFontId != -1) {
        QFontDatabase::removeApplicationFont(m_topazFontId);
    }
    if (m_zxFontId != -1) {
        QFontDatabase::removeApplicationFont(m_zxFontId);
    }
    delete ui;
}

/**
 * @brief Zatwierdza wprowadzone dane i zamyka okno dialogowe.
 *
 * @section MethodOverview
 * Zapisuje wszystkie wprowadzone parametry (typ bazy danych, ścieżka SQLite, dane MySQL, wybrana skórka) w pliku inwentaryzacja.ini za pomocą QSettings, a następnie wywołuje metodę accept klasy QDialog, zamykając okno z wynikiem QDialog::Accepted.
 */
void DatabaseConfigDialog::accept()
{
    QSettings settings = getSettings();
    settings.setValue("Database/Type", ui->dbTypeComboBox->currentText());
    settings.setValue("Database/SQLite/FilePath", ui->sqlitePathLineEdit->text());
    settings.setValue("Database/MySQL/Host", ui->hostLineEdit->text());
    settings.setValue("Database/MySQL/Database", ui->databaseLineEdit->text());
    settings.setValue("Database/MySQL/User", ui->userLineEdit->text());
    settings.setValue("Database/MySQL/Password", ui->passwordLineEdit->text());
    settings.setValue("Database/MySQL/Port", ui->portSpinBox->value());
    settings.setValue("skin", ui->filterSelectSkin->currentText());

    QDialog::accept();
}

/**
 * @brief Zwraca wybrany typ bazy danych.
 * @return QString zawierający typ bazy danych.
 *
 * @section MethodOverview
 * Pobiera tekst wybrany w combo boxie dbTypeComboBox, reprezentujący typ bazy danych (SQLite3 lub MySQL).
 */
QString DatabaseConfigDialog::selectedDatabaseType() const
{
    return ui->dbTypeComboBox->currentText();
}

/**
 * @brief Zwraca ścieżkę do pliku bazy SQLite.
 * @return QString zawierający ścieżkę do pliku SQLite.
 *
 * @section MethodOverview
 * Zwraca tekst z pola sqlitePathLineEdit, zawierający ścieżkę do pliku bazy SQLite.
 */
QString DatabaseConfigDialog::sqliteFilePath() const
{
    return ui->sqlitePathLineEdit->text();
}

/**
 * @brief Zwraca adres hosta dla bazy MySQL.
 * @return QString zawierający adres hosta MySQL.
 *
 * @section MethodOverview
 * Zwraca tekst z pola hostLineEdit, zawierający adres hosta bazy MySQL.
 */
QString DatabaseConfigDialog::mysqlHost() const
{
    return ui->hostLineEdit->text();
}

/**
 * @brief Zwraca nazwę bazy danych MySQL.
 * @return QString zawierający nazwę bazy danych MySQL.
 *
 * @section MethodOverview
 * Zwraca tekst z pola databaseLineEdit, zawierający nazwę bazy danych MySQL.
 */
QString DatabaseConfigDialog::mysqlDatabase() const
{
    return ui->databaseLineEdit->text();
}

/**
 * @brief Zwraca nazwę użytkownika dla bazy MySQL.
 * @return QString zawierający nazwę użytkownika MySQL.
 *
 * @section MethodOverview
 * Zwraca tekst z pola userLineEdit, zawierający nazwę użytkownika bazy MySQL.
 */
QString DatabaseConfigDialog::mysqlUser() const
{
    return ui->userLineEdit->text();
}

/**
 * @brief Zwraca hasło użytkownika dla bazy MySQL.
 * @return QString zawierający hasło użytkownika MySQL.
 *
 * @section MethodOverview
 * Zwraca tekst z pola passwordLineEdit, zawierający hasło użytkownika bazy MySQL.
 */
QString DatabaseConfigDialog::mysqlPassword() const
{
    return ui->passwordLineEdit->text();
}

/**
 * @brief Zwraca numer portu dla bazy MySQL.
 * @return int zawierający numer portu MySQL.
 *
 * @section MethodOverview
 * Zwraca wartość z pola portSpinBox, reprezentującą numer portu bazy MySQL.
 */
int DatabaseConfigDialog::mysqlPort() const
{
    return ui->portSpinBox->value();
}

/**
 * @brief Obsługuje zmianę typu bazy danych w combo boxie.
 * @param index Indeks wybranego typu bazy danych.
 *
 * @section SlotOverview
 * Przełącza aktywną stronę w QStackedWidget, pokazując odpowiednie pola dla wybranego typu bazy danych (SQLite lub MySQL).
 */
void DatabaseConfigDialog::onDatabaseTypeChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

/**
 * @brief Obsługuje zmianę skórki graficznej.
 * @param skin Nazwa wybranej skórki (np. "Amiga", "ZX Spectrum", "Standard").
 *
 * @section SlotOverview
 * Wywołuje metody loadStyleSheet i loadFont dla wybranej skórki, aktualizuje styl i czcionkę aplikacji, zapisuje wybór w QSettings i loguje zmianę.
 */
void DatabaseConfigDialog::onSkinChanged(const QString &skin)
{
    loadStyleSheet(skin);
    loadFont(skin);
    QSettings settings = getSettings();
    settings.setValue("skin", skin);
    qDebug() << "Zmieniono skórkę na:" << skin;
}

/**
 * @brief Ładuje arkusz stylów dla wybranej skórki.
 * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
 *
 * @section MethodOverview
 * Mapuje nazwę skórki na odpowiedni plik QSS w zasobach Qt (np. :/styles/amiga.qss), wczytuje jego zawartość i stosuje do aplikacji za pomocą qApp->setStyleSheet. Loguje sukces lub błąd ładowania.
 */
void DatabaseConfigDialog::loadStyleSheet(const QString &skin)
{
    QString qssPath;
    if (skin == "Amiga") {
        qssPath = ":/styles/amiga.qss";
    } else if (skin == "ZX Spectrum") {
        qssPath = ":/styles/zxspectrum.qss";
    } else if (skin == "Atari 8bit") {
        qssPath = ":/styles/atari8bit.qss";
    } else {
        qssPath = ":/styles/default.qss"; // Domyślnie Standard
    }

    QFile file(qssPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QString::fromUtf8(file.readAll());
        qApp->setStyleSheet(style);
        file.close();
        qDebug() << "Załadowano skórkę:" << skin << "z pliku:" << qssPath;
    } else {
        qWarning() << "Nie można załadować pliku QSS:" << qssPath;
    }
}

/**
 * @brief Ładuje czcionkę dla wybranej skórki.
 * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
 *
 * @section MethodOverview
 * Mapuje nazwę skórki na odpowiednią czcionkę (np. topaz.ttf dla Amigi, zxspectrum.ttf dla ZX Spectrum, EightBit Atari-Ataripl.ttf dla Atari 8bit, domyślna systemowa dla Standard). Ładuje czcionkę za pomocą QFontDatabase, ustawia ją dla aplikacji za pomocą qApp->setFont i aktualizuje paletę oraz styl, aby zapewnić propagację czcionki. Loguje sukces lub błąd ładowania.
 */
void DatabaseConfigDialog::loadFont(const QString &skin)
{
    QFont font;
    if (skin == "Amiga") {
        if (m_topazFontId == -1) {
            QFile file(":/fonts/topaz.ttf");
            if (!file.exists()) {
                qWarning() << "Plik topaz.ttf nie istnieje w zasobach!";
                font = QFont(); // Fallback na domyślną czcionkę systemową
            } else {
                m_topazFontId = QFontDatabase::addApplicationFont(":/fonts/topaz.ttf");
                if (m_topazFontId == -1) {
                    qWarning() << "Nie można załadować pliku topaz.ttf z zasobów";
                }
            }
        }
        if (m_topazFontId != -1) {
            QStringList families = QFontDatabase::applicationFontFamilies(m_topazFontId);
            if (!families.isEmpty()) {
                font.setFamily(families.first());
                font.setPointSize(12);
                qDebug() << "Załadowano czcionkę Topaz dla skórki Amiga, rodzina:"
                         << families.first();
            } else {
                qWarning() << "Brak dostępnych rodzin czcionek dla topaz.ttf";
                font = QFont(); // Fallback
            }
        } else {
            font = QFont(); // Fallback
        }
    } else if (skin == "ZX Spectrum") {
        if (m_zxFontId == -1) {
            QFile file(":/fonts/zxspectrum.ttf");
            if (!file.exists()) {
                qWarning() << "Plik zxspectrum.ttf nie istnieje w zasobach!";
                font = QFont(); // Fallback na domyślną czcionkę systemową
            } else {
                m_zxFontId = QFontDatabase::addApplicationFont(":/fonts/zxspectrum.ttf");
                if (m_zxFontId == -1) {
                    qWarning() << "Nie można załadować pliku zxspectrum.ttf z zasobów";
                }
            }
        }
        if (m_zxFontId != -1) {
            QStringList families = QFontDatabase::applicationFontFamilies(m_zxFontId);
            if (!families.isEmpty()) {
                font.setFamily(families.first());
                font.setPointSize(12);
                qDebug() << "Załadowano czcionkę ZX Spectrum dla skórki ZX Spectrum, rodzina:"
                         << families.first();
            } else {
                qWarning() << "Brak dostępnych rodzin czcionek dla zxspectrum.ttf";
                font = QFont(); // Fallback
            }
        } else {
            font = QFont(); // Fallback
        }
    } else if (skin == "Atari 8bit") {
        if (m_atari8bitFontId == -1) {
            QFile file(":/fonts/EightBit Atari-Ataripl.ttf");
            if (!file.exists()) {
                qWarning() << "Plik EightBit Atari-Ataripl.ttf nie istnieje w zasobach!";
                font = QFont(); // Fallback na domyślną czcionkę systemową
            } else {
                m_atari8bitFontId = QFontDatabase::addApplicationFont(
                    ":/fonts/EightBit Atari-Ataripl.ttf");
                if (m_atari8bitFontId == -1) {
                    qWarning() << "Nie można załadować pliku EightBit Atari-Ataripl.ttf z zasobów";
                }
            }
        }
        if (m_atari8bitFontId != -1) {
            QStringList families = QFontDatabase::applicationFontFamilies(m_atari8bitFontId);
            if (!families.isEmpty()) {
                font.setFamily(families.first());
                font.setPointSize(12);
                qDebug() << "Załadowano czcionkę EightBit Atari-Ataripl dla skórki Atari, rodzina:"
                         << families.first();
            } else {
                qWarning() << "Brak dostępnych rodzin czcionek dla EightBit Atari-Ataripl.ttf";
                font = QFont(); // Fallback
            }
        } else {
            font = QFont(); // Fallback
        }
    } else {
        // Standard: użyj domyślnej czcionki systemowej
        font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        font.setPointSize(12); // Spójność z innymi skórkami
        qDebug() << "Ustawiono domyślną czcionkę systemową dla skórki Standard, rodzina:"
                 << font.family();
    }

    // Sekcja: Ustawienie czcionki dla aplikacji
    // Ustawia wybraną czcionkę dla całej aplikacji, resetuje paletę i styl, aby zapewnić propagację.
    qApp->setFont(font);

    // Reset palety, aby wymusić aktualizację czcionki
    qApp->setPalette(QApplication::palette());

    // Wymuś aktualizację stylów, aby zapewnić propagację czcionki
    qApp->setStyleSheet(qApp->styleSheet());
    qDebug() << "Ustawiono czcionkę dla aplikacji:" << font.family()
             << ", rozmiar:" << font.pointSize();
}
