#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Ustawienie połączenia z bazą SQLite
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/arekbr/inwentaryzacja/muzeum.db");
    if (!db.open()) {
        QMessageBox::critical(this, tr("Błąd bazy danych"),
                              tr("Nie udało się otworzyć bazy danych:\n%1").arg(db.lastError().text()));
        return;
    }
    // SELECt * from typ;

    ui->New_item_type->addItem("test",1);
    // Podłączenie przycisku "Zapisz" do slotu onSaveClicked
    connect(ui->New_item_PushButton_OK, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
}

MainWindow::~MainWindow() {
    if (db.isOpen())
        db.close();
    delete ui;
}

void MainWindow::onSaveClicked() {
    // Pobranie danych z pól formularza
    QString name = ui->New_item_name->text();
    QString serialNumber = ui->New_item_serialNumber->text();
    QString partNumber = ui->New_item_partNumber->text();
    QString revision = ui->New_item_revision->text();
    int productionYear = ui->New_item_ProductionDate->date().year();
    int value = ui->New_item_value->text().toInt();
    QString description = ui->New_item_description->toPlainText();

    // Przykładowe przypisanie identyfikatorów z QComboBox (indeks + 1 jako id)
    int type_id = ui->New_item_type->currentIndex() + 1;
    int vendor_id = ui->New_item_vendor->currentIndex() + 1;
    int model_id = ui->New_item_model->currentIndex() + 1;
    int status_id = ui->New_item_status->currentIndex() + 1;
    int storage_place_id = ui->New_item_storagePlace->currentIndex() + 1;

    // Przygotowanie zapytania INSERT
    QSqlQuery query;
    query.prepare("INSERT INTO eksponaty (name, type_id, vendor_id, model_id, serial_number, part_number, revision, production_year, status_id, storage_place_id, description, value) "
                  "VALUES (:name, :type_id, :vendor_id, :model_id, :serial_number, :part_number, :revision, :production_year, :status_id, :storage_place_id, :description, :value)");
    query.bindValue(":name", name);
    query.bindValue(":type_id", type_id);
    query.bindValue(":vendor_id", vendor_id);
    query.bindValue(":model_id", model_id);
    query.bindValue(":serial_number", serialNumber);
    query.bindValue(":part_number", partNumber);
    query.bindValue(":revision", revision);
    query.bindValue(":production_year", productionYear);
    query.bindValue(":status_id", status_id);
    query.bindValue(":storage_place_id", storage_place_id);
    query.bindValue(":description", description);
    query.bindValue(":value", value);

    if (!query.exec()) {

        qDebug() << "Błąd zapisu:" << query.lastError().text();
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zapisać rekordu:\n%1").arg(query.lastError().text()));
    } else {
        QMessageBox::information(this, tr("Sukces"), tr("Rekord został dodany do bazy danych."));
    }
}
