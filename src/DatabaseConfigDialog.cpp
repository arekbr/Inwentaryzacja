/**
 * @file DatabaseConfigDialog.cpp
 * @brief Implementacja klasy DatabaseConfigDialog z obsługą tworzenia nowego pliku SQLite
 *        oraz wyboru skórki graficznej aplikacji.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.2.0
 * @date 2025-05-01
 *
 * Plik zawiera implementację metod klasy DatabaseConfigDialog, odpowiedzialnej za konfigurację
 * parametrów połączenia z bazą danych (SQLite lub MySQL) oraz wybór skórki graficznej aplikacji
 * (Amiga, ZX Spectrum, Standard). Rozszerzono funkcję wyboru pliku SQLite, by umożliwić wskazanie
 * nowej ścieżki za pomocą getSaveFileName i automatyczne utworzenie pliku .db. Dodano obsługę
 * zmiany skórki graficznej, w tym ładowanie stylów QSS i czcionek z zasobów Qt, z zapisem wyboru
 * w QSettings dla trwałości między sesjami aplikacji.
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
#include "ui_DatabaseConfigDialog.h"

/**
 * @brief Pobiera obiekt QSettings dla aplikacji.
 * @return Obiekt QSettings zainicjalizowany z plikiem inwentaryzacja.ini.
 *
 * Tworzy plik konfiguracyjny, jeśli nie istnieje, w katalogu aplikacji i zwraca obiekt QSettings
 * w formacie IniFormat.
 */
static QSettings getSettings()
{
    QString settingsPath = QCoreApplication::applicationDirPath() + "/inwentaryzacja.ini";
    QFile settingsFile(settingsPath);
    if (!settingsFile.exists()) {
        QFile file(settingsPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
        }
    }
    return QSettings(settingsPath, QSettings::IniFormat);
}

/**
 * @brief Konstruktor klasy DatabaseConfigDialog.
 * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
 *
 * Inicjalizuje interfejs użytkownika, ustawia opcje combo boxów dla typu bazy danych
 * i skórki graficznej, ładuje zapisane ustawienia z QSettings i konfiguruje połączenia
 * sygnałów i slotów dla interakcji użytkownika.
 */
DatabaseConfigDialog::DatabaseConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseConfigDialog)
    , m_topazFontId(-1)
    , m_zxFontId(-1)
{
    ui->setupUi(this);

    // Połączenia dla przycisków dialogowych
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DatabaseConfigDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Inicjalizacja combo boxa dla typu bazy danych
    ui->dbTypeComboBox->clear();
    ui->dbTypeComboBox->addItem("SQLite3");
    ui->dbTypeComboBox->addItem("MySQL");
    ui->stackedWidget->setCurrentIndex(0);

    // Połączenie dla zmiany typu bazy danych
    connect(ui->dbTypeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DatabaseConfigDialog::onDatabaseTypeChanged);

    // Inicjalizacja combo boxa dla skórki graficznej
    ui->filterSelectSkin->clear();
    ui->filterSelectSkin->addItems({"Amiga", "ZX Spectrum", "Standard"});

    // Ładowanie zapisanych ustawień
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

    // Połączenie dla zmiany skórki graficznej
    connect(ui->filterSelectSkin,
            &QComboBox::currentTextChanged,
            this,
            &DatabaseConfigDialog::onSkinChanged);

    // Połączenie dla przycisku wyboru pliku SQLite
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
 * Zwalnia zasoby interfejsu użytkownika i usuwa załadowane czcionki z QFontDatabase.
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
 * Zapisuje parametry połączenia z bazą danych oraz wybraną skórkę graficzną w QSettings
 * i wywołuje metodę accept klasy QDialog.
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
 */
QString DatabaseConfigDialog::selectedDatabaseType() const
{
    return ui->dbTypeComboBox->currentText();
}

/**
 * @brief Zwraca ścieżkę do pliku bazy SQLite.
 * @return QString zawierający ścieżkę do pliku SQLite.
 */
QString DatabaseConfigDialog::sqliteFilePath() const
{
    return ui->sqlitePathLineEdit->text();
}

/**
 * @brief Zwraca adres hosta dla bazy MySQL.
 * @return QString zawierający adres hosta MySQL.
 */
QString DatabaseConfigDialog::mysqlHost() const
{
    return ui->hostLineEdit->text();
}

/**
 * @brief Zwraca nazwę bazy danych MySQL.
 * @return QString zawierający nazwę bazy danych MySQL.
 */
QString DatabaseConfigDialog::mysqlDatabase() const
{
    return ui->databaseLineEdit->text();
}

/**
 * @brief Zwraca nazwę użytkownika dla bazy MySQL.
 * @return QString zawierający nazwę użytkownika MySQL.
 */
QString DatabaseConfigDialog::mysqlUser() const
{
    return ui->userLineEdit->text();
}

/**
 * @brief Zwraca hasło użytkownika dla bazy MySQL.
 * @return QString zawierający hasło użytkownika MySQL.
 */
QString DatabaseConfigDialog::mysqlPassword() const
{
    return ui->passwordLineEdit->text();
}

/**
 * @brief Zwraca numer portu dla bazy MySQL.
 * @return int zawierający numer portu MySQL.
 */
int DatabaseConfigDialog::mysqlPort() const
{
    return ui->portSpinBox->value();
}

/**
 * @brief Obsługuje zmianę typu bazy danych w combo boxie.
 * @param index Indeks wybranego typu bazy danych.
 *
 * Przełącza stronę w QStackedWidget w zależności od wybranego typu bazy danych
 * (SQLite lub MySQL).
 */
void DatabaseConfigDialog::onDatabaseTypeChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

/**
 * @brief Obsługuje zmianę skórki graficznej.
 * @param skin Nazwa wybranej skórki (np. "Amiga", "ZX Spectrum", "Standard").
 *
 * Ładuje arkusz stylów i czcionkę dla wybranej skórki, aktualizuje interfejs aplikacji
 * i zapisuje wybór w QSettings.
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
 * Mapuje nazwę skórki na odpowiedni plik QSS w zasobach Qt i ustawia styl aplikacji
 * za pomocą qApp->setStyleSheet.
 */
void DatabaseConfigDialog::loadStyleSheet(const QString &skin)
{
    QString qssPath;
    if (skin == "Amiga") {
        qssPath = ":/styles/amiga.qss";
    } else if (skin == "ZX Spectrum") {
        qssPath = ":/styles/zxspectrum.qss";
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
 * Mapuje nazwę skórki na odpowiednią czcionkę: Amiga -> topaz.ttf, ZX Spectrum -> zxspectrum.ttf,
 * Standard -> domyślna czcionka systemowa. Ładuje czcionkę z zasobów Qt za pomocą QFontDatabase
 * i ustawia ją dla aplikacji za pomocą qApp->setFont.
 */
void DatabaseConfigDialog::loadFont(const QString &skin)
{
    QFont font;
    if (skin == "Amiga") {
        if (m_topazFontId == -1) {
            QFile file(":/images/topaz.ttf");
            if (!file.exists()) {
                qWarning() << "Plik topaz.ttf nie istnieje w zasobach!";
                font = QFont(); // Fallback na domyślną czcionkę systemową
            } else {
                m_topazFontId = QFontDatabase::addApplicationFont(":/images/topaz.ttf");
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
            QFile file(":/images/zxspectrum.ttf");
            if (!file.exists()) {
                qWarning() << "Plik zxspectrum.ttf nie istnieje w zasobach!";
                font = QFont(); // Fallback na domyślną czcionkę systemową
            } else {
                m_zxFontId = QFontDatabase::addApplicationFont(":/images/zxspectrum.ttf");
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
    } else {
        // Standard: użyj domyślnej czcionki systemowej
        font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        font.setPointSize(12); // Spójność z innymi skórkami
        qDebug() << "Ustawiono domyślną czcionkę systemową dla skórki Standard, rodzina:"
                 << font.family();
    }

    // Ustaw czcionkę dla aplikacji
    qApp->setFont(font);

    // Reset palety, aby wymusić aktualizację czcionki
    qApp->setPalette(QApplication::palette());

    // Wymuś aktualizację stylów, aby zapewnić propagację czcionki
    qApp->setStyleSheet(qApp->styleSheet());
    qDebug() << "Ustawiono czcionkę dla aplikacji:" << font.family()
             << ", rozmiar:" << font.pointSize();
}
