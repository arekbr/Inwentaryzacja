#include "DatabaseConfigDialog.h"
#include "ui_DatabaseConfigDialog.h"
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

DatabaseConfigDialog::DatabaseConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseConfigDialog)
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

    // Wczytujemy zapisane ustawienia za pomocą QSettings
    QSettings settings("MyCompany", "MyApp");  // Zmień nazwę według swoich potrzeb
    QString savedDbType = settings.value("Database/Type", "SQLite3").toString();
    ui->dbTypeComboBox->setCurrentText(savedDbType);

    int index = ui->dbTypeComboBox->findText(savedDbType);
    ui->stackedWidget->setCurrentIndex(index >= 0 ? index : 0);

    // Ustawienia dla SQLite
    ui->sqlitePathLineEdit->setText(settings.value("Database/SQLite/FilePath", "").toString());

    // Ustawienia dla MySQL
    ui->hostLineEdit->setText(settings.value("Database/MySQL/Host", "127.0.0.1").toString());
    ui->databaseLineEdit->setText(settings.value("Database/MySQL/Database", "nazwa_bazy").toString());
    ui->userLineEdit->setText(settings.value("Database/MySQL/User", "user").toString());
    ui->passwordLineEdit->setText(settings.value("Database/MySQL/Password", "").toString());
    ui->portSpinBox->setValue(settings.value("Database/MySQL/Port", 3306).toInt());

    // Podpięcie przycisku wyboru pliku dla SQLite
    connect(ui->selectFileButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Wybierz plik bazy danych"),
                                                        QString(), tr("SQLite Database (*.db)"));
        if (!filePath.isEmpty())
            ui->sqlitePathLineEdit->setText(filePath);
    });

}

DatabaseConfigDialog::~DatabaseConfigDialog()
{
    delete ui;
}

void DatabaseConfigDialog::accept()
{
    // Zapisanie ustawień w QSettings
    QSettings settings("MyCompany", "MyApp");
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

void DatabaseConfigDialog::on_dbTypeComboBox_currentIndexChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}
