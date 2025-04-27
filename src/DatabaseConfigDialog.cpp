/**
 * @file DatabaseConfigDialog.cpp
 * @brief Implementacja klasy DatabaseConfigDialog z obsługą tworzenia nowego pliku SQLite
 *        oraz selekcję pliku bazy przy pomocy getSaveFileName(), umożliwiającą wskazanie
 *        nieistniejącego jeszcze pliku i jego automatyczne utworzenie.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-27
 *
 * Plik zawiera implementację metod klasy DatabaseConfigDialog, odpowiedzialnej za konfigurację
 * parametrów połączenia z bazą danych (SQLite lub MySQL). Rozszerzono funkcję wyboru pliku SQLite,
 * by umożliwić wskazanie nowej ścieżki (getSaveFileName) i automatyczne utworzenie pliku .db.
 */

#include "DatabaseConfigDialog.h"
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#include "ui_DatabaseConfigDialog.h"

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

DatabaseConfigDialog::DatabaseConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseConfigDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DatabaseConfigDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    ui->dbTypeComboBox->clear();
    ui->dbTypeComboBox->addItem("SQLite3");
    ui->dbTypeComboBox->addItem("MySQL");
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->dbTypeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DatabaseConfigDialog::onDatabaseTypeChanged);

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

    // Możliwość wyboru nowego pliku .db: użycie getSaveFileName
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
                    // utworzenie nowego pliku bazy
                    if (file.open(QIODevice::WriteOnly)) {
                        file.close();
                    }
                }
                ui->sqlitePathLineEdit->setText(filePath);
            }
        }
    });
}

DatabaseConfigDialog::~DatabaseConfigDialog()
{
    delete ui;
}

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

    QDialog::accept();
}

QString DatabaseConfigDialog::selectedDatabaseType() const
{
    return ui->dbTypeComboBox->currentText();
}

QString DatabaseConfigDialog::sqliteFilePath() const
{
    return ui->sqlitePathLineEdit->text();
}

QString DatabaseConfigDialog::mysqlHost() const
{
    return ui->hostLineEdit->text();
}

QString DatabaseConfigDialog::mysqlDatabase() const
{
    return ui->databaseLineEdit->text();
}

QString DatabaseConfigDialog::mysqlUser() const
{
    return ui->userLineEdit->text();
}

QString DatabaseConfigDialog::mysqlPassword() const
{
    return ui->passwordLineEdit->text();
}

int DatabaseConfigDialog::mysqlPort() const
{
    return ui->portSpinBox->value();
}

void DatabaseConfigDialog::onDatabaseTypeChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}
