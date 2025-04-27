/**
 * @file DatabaseConfigDialog.cpp
 * @brief Implementacja klasy DatabaseConfigDialog do konfiguracji połączenia z bazą danych.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera implementację metod klasy DatabaseConfigDialog, odpowiedzialnej za konfigurację
 * parametrów połączenia z bazą danych (SQLite lub MySQL). Klasa obsługuje wybór typu bazy danych,
 * wprowadzanie parametrów połączenia, zapisywanie ustawień w QSettings oraz dynamiczne przełączanie
 * interfejsu w zależności od wybranego typu bazy danych.
 */

#include "DatabaseConfigDialog.h"
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#include "ui_DatabaseConfigDialog.h"

/**
 * @brief Funkcja pomocnicza zwracająca obiekt QSettings.
 *
 * Tworzy lub otwiera plik ustawień "inwentaryzacja.ini" w katalogu aplikacji
 * i zwraca obiekt QSettings skonfigurowany do pracy w formacie INI.
 *
 * Jeśli plik "inwentaryzacja.ini" nie istnieje, funkcja automatycznie go tworzy
 * jako pusty plik tekstowy. Dzięki temu możliwe jest bezpieczne zapisywanie i
 * odczytywanie ustawień aplikacji bez potrzeby sprawdzania istnienia pliku w innych miejscach kodu.
 *
 * @return QSettings Obiekt gotowy do odczytu i zapisu ustawień aplikacji.
 */
static QSettings getSettings()
{
    QString settingsPath = QCoreApplication::applicationDirPath() + "/inwentaryzacja.ini";

    // Upewniamy się, że plik istnieje
    QFile settingsFile(settingsPath);
    if (!settingsFile.exists()) {
        // Jeśli nie istnieje, tworzymy pusty plik
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
 * Inicjalizuje interfejs użytkownika, ustawia dostępne typy baz danych (SQLite3, MySQL),
 * konfiguruje przełączanie między stronami ustawień za pomocą QStackedWidget oraz wczytuje
 * zapisane ustawienia z QSettings. Podłącza sygnały i sloty dla przycisków i combo boxa.
 */
DatabaseConfigDialog::DatabaseConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseConfigDialog)
{
    ui->setupUi(this);

    // Połączenie przycisków OK i Cancel z metodami accept() i reject()
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DatabaseConfigDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Wyczyść comboBox, aby usunąć ewentualne domyślne pozycje
    ui->dbTypeComboBox->clear();
    // Ustawienie dostępnych typów baz danych
    ui->dbTypeComboBox->addItem("SQLite3");
    ui->dbTypeComboBox->addItem("MySQL");

    // Ustaw domyślną stronę konfiguracji – indeks 0 (SQLite3)
    ui->stackedWidget->setCurrentIndex(0);

    // Podłączenie zmiany typu bazy danych
    connect(ui->dbTypeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DatabaseConfigDialog::onDatabaseTypeChanged);

    // Wczytujemy zapisane ustawienia za pomocą QSettings
    QSettings settings = getSettings();
    QString savedDbType = settings.value("Database/Type", "SQLite3").toString();
    ui->dbTypeComboBox->setCurrentText(savedDbType);

    int index = ui->dbTypeComboBox->findText(savedDbType);
    ui->stackedWidget->setCurrentIndex(index >= 0 ? index : 0);

    // Ustawienia dla SQLite
    ui->sqlitePathLineEdit->setText(settings.value("Database/SQLite/FilePath", "").toString());

    // Ustawienia dla MySQL
    ui->hostLineEdit->setText(settings.value("Database/MySQL/Host", "127.0.0.1").toString());
    ui->databaseLineEdit->setText(
        settings.value("Database/MySQL/Database", "nazwa_bazy").toString());
    ui->userLineEdit->setText(settings.value("Database/MySQL/User", "user").toString());
    ui->passwordLineEdit->setText(settings.value("Database/MySQL/Password", "").toString());
    ui->portSpinBox->setValue(settings.value("Database/MySQL/Port", 3306).toInt());

    // Podpięcie przycisku wyboru pliku dla SQLite
    connect(ui->selectFileButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this,
                                                        tr("Wybierz plik bazy danych"),
                                                        QString(),
                                                        tr("SQLite Database (*.db)"));
        if (!filePath.isEmpty())
            ui->sqlitePathLineEdit->setText(filePath);
    });
}

/**
 * @brief Destruktor klasy DatabaseConfigDialog.
 *
 * Zwalnia zasoby interfejsu użytkownika.
 */
DatabaseConfigDialog::~DatabaseConfigDialog()
{
    delete ui;
}

/**
 * @brief Zatwierdza wprowadzone dane i zamyka okno dialogowe.
 *
 * Zapisuje wszystkie parametry połączenia (typ bazy danych, ścieżka SQLite, dane MySQL)
 * w QSettings i wywołuje domyślną metodę accept() klasy QDialog, zamykając okno.
 */
void DatabaseConfigDialog::accept()
{
    // Zapisanie ustawień w QSettings
    QSettings settings = getSettings();
    settings.setValue("Database/Type", ui->dbTypeComboBox->currentText());
    settings.setValue("Database/SQLite/FilePath", ui->sqlitePathLineEdit->text());
    settings.setValue("Database/MySQL/Host", ui->hostLineEdit->text());
    settings.setValue("Database/MySQL/Database", ui->databaseLineEdit->text());
    settings.setValue("Database/MySQL/User", ui->userLineEdit->text());
    settings.setValue("Database/MySQL/Password", ui->passwordLineEdit->text());
    settings.setValue("Database/MySQL/Port", ui->portSpinBox->value());

    // Wywołujemy domyślne działanie akceptacji
    QDialog::accept();
}

/**
 * @brief Zwraca wybrany typ bazy danych.
 * @return QString zawierający typ bazy danych (np. "SQLite3" lub "MySQL").
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
 * @param index Indeks wybranego typu bazy danych (0 dla SQLite3, 1 dla MySQL).
 *
 * Przełącza stronę w QStackedWidget, wyświetlając odpowiednie pola konfiguracyjne
 * dla wybranego typu bazy danych.
 */
void DatabaseConfigDialog::onDatabaseTypeChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}
