#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QDate>
#include <QFileDialog>
#include <QFile>
#include <QBuffer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_editMode(false),
    m_recordId(-1)
{
    ui->setupUi(this);

    // Ustawienie połączenia z bazą SQLite
    if (!QSqlDatabase::contains("default_connection")) {
        db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");
        db.setDatabaseName("/home/arekbr/inwentaryzacja/muzeum.db");
        if (!db.open()) {
            QMessageBox::critical(this, tr("Błąd bazy danych"),
                                  tr("Nie udało się otworzyć bazy danych:\n%1").arg(db.lastError().text()));
            return;
        }
    } else {
        db = QSqlDatabase::database("default_connection");
    }

    // Dynamiczne ładowanie danych do QComboBoxów
    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM typy")) {
        while (query.next()) {
            ui->New_item_type->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    if (query.exec("SELECT id, name FROM vendors")) {
        while (query.next()) {
            ui->New_item_vendor->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    if (query.exec("SELECT id, name FROM models")) {
        while (query.next()) {
            ui->New_item_model->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    if (query.exec("SELECT id, name FROM status")) {
        while (query.next()) {
            ui->New_item_status->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    if (query.exec("SELECT id, name FROM storage_places")) {
        while (query.next()) {
            ui->New_item_storagePlace->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }

    // Podłączenie przycisków
    connect(ui->New_item_PushButton_OK, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(ui->New_item_PushButton_Cancel, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(ui->New_item_addPhoto, &QPushButton::clicked, this, &MainWindow::onAddPhotoClicked);
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
    delete ui;
}

void MainWindow::setEditMode(bool edit, int recordId)
{
    m_editMode = edit;
    m_recordId = recordId;
    if (m_editMode && m_recordId != -1) {
        loadRecord(m_recordId);
    } else {
        // Tryb dodawania – czyścimy wszystkie pola
        ui->New_item_name->clear();
        ui->New_item_serialNumber->clear();
        ui->New_item_partNumber->clear();
        ui->New_item_revision->clear();
        ui->New_item_value->clear();
        ui->New_item_description->clear();
        ui->New_item_ProductionDate->setDate(QDate::currentDate());

        // Lista QComboBoxów, które chcemy wyczyścić
        QList<QComboBox*> comboBoxes = {
            ui->New_item_type,
            ui->New_item_vendor,
            ui->New_item_model,
            ui->New_item_status,
            ui->New_item_storagePlace
        };

        // Dla każdego QComboBox ustawiamy tymczasowo edytowalność, aby móc wyczyścić tekst,
        // ustawiamy indeks na -1 (co oznacza brak zaznaczenia), a następnie przywracamy nieedytowalność.
        for (QComboBox* combo : comboBoxes) {
            combo->setEditable(true);
            combo->clearEditText();
            combo->setCurrentIndex(-1);
            combo->setEditable(false);
        }
    }
}

void MainWindow::loadRecord(int recordId)
{
    QSqlQuery query(db);
    query.prepare("SELECT name, serial_number, part_number, revision, production_year, status_id, type_id, vendor_id, model_id, storage_place_id, description, value, image_path "
                  "FROM eksponaty WHERE id = :id");
    query.bindValue(":id", recordId);
    if (query.exec() && query.next()) {
        ui->New_item_name->setText(query.value("name").toString());
        ui->New_item_serialNumber->setText(query.value("serial_number").toString());
        ui->New_item_partNumber->setText(query.value("part_number").toString());
        ui->New_item_revision->setText(query.value("revision").toString());
        ui->New_item_value->setText(query.value("value").toString());
        ui->New_item_description->setPlainText(query.value("description").toString());

        int prodYear = query.value("production_year").toInt();
        ui->New_item_ProductionDate->setDate(QDate(prodYear, 1, 1));

        // Użycie findData() dla ComboBoxów
        int type_id = query.value("type_id").toInt();
        int vendor_id = query.value("vendor_id").toInt();
        int model_id = query.value("model_id").toInt();
        int status_id = query.value("status_id").toInt();
        int storage_place_id = query.value("storage_place_id").toInt();

        int typeIndex = ui->New_item_type->findData(type_id);
        if (typeIndex != -1)
            ui->New_item_type->setCurrentIndex(typeIndex);

        int vendorIndex = ui->New_item_vendor->findData(vendor_id);
        if (vendorIndex != -1)
            ui->New_item_vendor->setCurrentIndex(vendorIndex);

        int modelIndex = ui->New_item_model->findData(model_id);
        if (modelIndex != -1)
            ui->New_item_model->setCurrentIndex(modelIndex);

        int statusIndex = ui->New_item_status->findData(status_id);
        if (statusIndex != -1)
            ui->New_item_status->setCurrentIndex(statusIndex);

        int storagePlaceIndex = ui->New_item_storagePlace->findData(storage_place_id);
        if (storagePlaceIndex != -1)
            ui->New_item_storagePlace->setCurrentIndex(storagePlaceIndex);

        // Załaduj obraz z tabeli photos (jeśli istnieje)
        QSqlQuery photoQuery(db);
        photoQuery.prepare("SELECT photo FROM photos WHERE eksponat_id = :id LIMIT 1");
        photoQuery.bindValue(":id", recordId);
        if (photoQuery.exec() && photoQuery.next()) {
            QByteArray imageData = photoQuery.value("photo").toByteArray();
            QPixmap pixmap;
            pixmap.loadFromData(imageData);
            if (!pixmap.isNull()) {
                QGraphicsScene *scene = new QGraphicsScene(this);
                scene->addPixmap(pixmap);
                ui->graphicsView->setScene(scene);
            }
        } else {
            ui->graphicsView->setScene(nullptr);
        }
    } else {
        QMessageBox::warning(this, tr("Błąd"), tr("Nie znaleziono rekordu o id %1").arg(recordId));
    }
}

void MainWindow::onSaveClicked()
{
    QSqlQuery query(db);
    int newRecordId = -1;
    if (!m_editMode) {
        // Dodawanie nowego rekordu
        query.prepare("INSERT INTO eksponaty (name, serial_number, part_number, revision, production_year, status_id, type_id, vendor_id, model_id, storage_place_id, description, value, image_path) "
                      "VALUES (:name, :serial_number, :part_number, :revision, :production_year, :status_id, :type_id, :vendor_id, :model_id, :storage_place_id, :description, :value, :image_path)");
    } else {
        // Aktualizacja istniejącego rekordu
        query.prepare("UPDATE eksponaty SET name = :name, serial_number = :serial_number, part_number = :part_number, revision = :revision, "
                      "production_year = :production_year, status_id = :status_id, type_id = :type_id, vendor_id = :vendor_id, model_id = :model_id, "
                      "storage_place_id = :storage_place_id, description = :description, value = :value, image_path = :image_path "
                      "WHERE id = :id");
        query.bindValue(":id", m_recordId);
    }

    // Wiązanie wszystkich pól z formularza z użyciem nowoczesnego API QVariant
    query.bindValue(":name", ui->New_item_name->text().isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : ui->New_item_name->text());
    query.bindValue(":serial_number", ui->New_item_serialNumber->text().isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : ui->New_item_serialNumber->text());
    query.bindValue(":part_number", ui->New_item_partNumber->text().isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : ui->New_item_partNumber->text());
    query.bindValue(":revision", ui->New_item_revision->text().isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : ui->New_item_revision->text());
    query.bindValue(":production_year", ui->New_item_ProductionDate->date().year());
    query.bindValue(":status_id", ui->New_item_status->currentData().isValid() ? ui->New_item_status->currentData() : 1);
    query.bindValue(":type_id", ui->New_item_type->currentData().isValid() ? ui->New_item_type->currentData() : 1);
    query.bindValue(":vendor_id", ui->New_item_vendor->currentData().isValid() ? ui->New_item_vendor->currentData() : 1);
    query.bindValue(":model_id", ui->New_item_model->currentData().isValid() ? ui->New_item_model->currentData() : 1);
    query.bindValue(":storage_place_id", ui->New_item_storagePlace->currentData().isValid() ? ui->New_item_storagePlace->currentData() : 1);
    query.bindValue(":description", ui->New_item_description->toPlainText().isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : ui->New_item_description->toPlainText());
    query.bindValue(":value", ui->New_item_value->text().isEmpty() ? 0 : ui->New_item_value->text().toInt());
    query.bindValue(":image_path", ""); // Pole image_path nie będzie używane, pozostawiam puste

    if (!query.exec()) {
        qDebug() << "Błąd zapisu:" << query.lastError().text();
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zapisać rekordu:\n%1")
                                                    .arg(query.lastError().text()));
        return;
    }

    // Pobierz ID nowo dodanego rekordu (dla trybu dodawania)
    if (!m_editMode) {
        newRecordId = query.lastInsertId().toInt();
        m_recordId = newRecordId; // Ustawiamy m_recordId na nowo dodany rekord
    } else {
        newRecordId = m_recordId; // W trybie edycji używamy istniejącego ID
    }

    emit recordSaved(); // Emitujemy sygnał po udanym zapisie
    QMessageBox::information(this, tr("Sukces"), tr("Operacja zapisu zakończona powodzeniem."));
    close(); // Zamykamy okno formularza po zapisie
}

void MainWindow::onCancelClicked()
{
    // Zamykamy okno bez zapisywania zmian
    close();
}

void MainWindow::onAddPhotoClicked()
{
    // Sprawdź, czy rekord istnieje (musi być zapisany przed dodaniem zdjęcia)
    if (m_recordId == -1) {
        QMessageBox::warning(this, tr("Błąd"), tr("Najpierw zapisz rekord, aby dodać zdjęcie."));
        return;
    }

    // Otwórz dialog wyboru plików (tylko .jpg i .png)
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Wybierz zdjęcia"),
                                                          QString(),
                                                          tr("Images (*.jpg *.jpeg *.png)"));
    if (fileNames.isEmpty()) {
        return; // Użytkownik anulował wybór
    }

    QSqlQuery query(db);
    // Użycie indeksów zamiast range-based for, aby uniknąć ostrzeżenia clazy
    for (int i = 0; i < fileNames.size(); ++i) {
        const QString &fileName = fileNames.at(i);
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Nie można otworzyć pliku:" << fileName;
            continue;
        }

        QByteArray imageData = file.readAll();
        file.close();

        // Wstaw zdjęcie do tabeli photos
        query.prepare("INSERT INTO photos (eksponat_id, photo) VALUES (:eksponat_id, :photo)");
        query.bindValue(":eksponat_id", m_recordId);
        query.bindValue(":photo", imageData);

        if (!query.exec()) {
            qDebug() << "Błąd zapisu zdjęcia:" << query.lastError().text();
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zapisać zdjęcia:\n%1")
                                                        .arg(query.lastError().text()));
        } else {
            qDebug() << "Zdjęcie zapisane dla rekordu" << m_recordId;
        }
    }

    // Odśwież widok zdjęcia w formularzu (pobierz pierwsze zdjęcie)
    loadRecord(m_recordId);
}
