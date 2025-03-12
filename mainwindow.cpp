#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "photoitem.h"
#include "types.h"
#include "models.h"
#include "vendors.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <qglobal.h>

///////////////////////
// Metody dostępowe //
///////////////////////

QComboBox* MainWindow::getNewItemTypeComboBox() const
{
    return ui->New_item_type;
}

QComboBox* MainWindow::getNewItemModelComboBox() const
{
    return ui->New_item_model;
}

QComboBox* MainWindow::getNewItemVendorComboBox() const
{
    return ui->New_item_vendor;
}

///////////////////////
// Konstruktor, destruktor, inicjalizacja //
///////////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_editMode(false),
    m_recordId(-1),
    m_selectedPhotoIndex(-1)
{
    ui->setupUi(this);

    // Ustawienie połączenia z bazą SQLite
    if (!QSqlDatabase::contains("default_connection")) {
        db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");

#ifdef Q_OS_LINUX
        db.setDatabaseName("/home/arekbr/inwentaryzacja/muzeum.db");
#elif defined(Q_OS_MAC)
        db.setDatabaseName("/Users/Arek/inwentaryzacja/muzeum.db");
#else
#error "System operacyjny nie jest obsługiwany"
#endif
        if (!db.open()) {
            QMessageBox::critical(this, tr("Błąd bazy danych"),
                                  tr("Nie udało się otworzyć bazy:\n%1").arg(db.lastError().text()));
            return;
        }
    } else {
        db = QSqlDatabase::database("default_connection");
    }

    // Ładowanie danych do ComboBoxów przy użyciu funkcji loadComboBoxData
    loadComboBoxData("types", ui->New_item_type);
    loadComboBoxData("vendors", ui->New_item_vendor);
    loadComboBoxData("models", ui->New_item_model);
    loadComboBoxData("statuses", ui->New_item_status);
    loadComboBoxData("storage_places", ui->New_item_storagePlace);

    // Podłączenie przycisków
    connect(ui->New_item_PushButton_OK, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(ui->New_item_PushButton_Cancel, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(ui->New_item_addPhoto, &QPushButton::clicked, this, &MainWindow::onAddPhotoClicked);
    connect(ui->New_item_removePhoto, &QPushButton::clicked, this, &MainWindow::onRemovePhotoClicked);
    connect(ui->New_item_addType, &QPushButton::clicked, this, &MainWindow::onAddTypeClicked);
    connect(ui->New_item_addVendor, &QPushButton::clicked, this, &MainWindow::onAddVendorClicked);
    connect(ui->New_item_addModel, &QPushButton::clicked, this, &MainWindow::onAddModelClicked);

    ui->graphicsView->setTransform(QTransform());
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
    delete ui;
}

///////////////////////
// Funkcje pomocnicze //
///////////////////////

void MainWindow::loadComboBoxData(const QString &tableName, QComboBox *comboBox)
{
    comboBox->clear();
    QSqlQuery query(db);
    if (query.exec(QString("SELECT id, name FROM %1").arg(tableName))) {
        while (query.next()) {
            comboBox->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    } else {
        qDebug() << "Błąd ładowania danych dla" << tableName << ":" << query.lastError().text();
    }
}

///////////////////////
// Tryb edycji/dodawania //
///////////////////////

void MainWindow::setEditMode(bool edit, int recordId)
{
    m_editMode = edit;
    m_recordId = recordId;
    if (m_editMode && m_recordId != -1) {
        loadRecord(m_recordId);
    } else {
        // Tryb dodawania – czyszczenie pól
        ui->New_item_name->clear();
        ui->New_item_serialNumber->clear();
        ui->New_item_partNumber->clear();
        ui->New_item_revision->clear();
        ui->New_item_value->clear();
        ui->New_item_description->clear();
        ui->New_item_ProductionDate->setDate(QDate::currentDate());

        QList<QComboBox*> combos = {
            ui->New_item_type,
            ui->New_item_vendor,
            ui->New_item_model,
            ui->New_item_status,
            ui->New_item_storagePlace
        };
        for (QComboBox* combo : combos) {
            // Czyszczenie – ustawienie indeksu na -1
            combo->setEditable(true);
            combo->clearEditText();
            combo->setCurrentIndex(-1);
            combo->setEditable(false);
        }
        ui->graphicsView->setScene(nullptr);
        m_selectedPhotoIndex = -1;
    }
}

///////////////////////
// Ładowanie rekordu i zdjęć //
///////////////////////

void MainWindow::loadRecord(int recordId)
{
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT name, serial_number, part_number, revision, production_year,
               status_id, type_id, vendor_id, model_id, storage_place_id,
               description, value, image_path
        FROM eksponaty
        WHERE id = :id
    )");
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

        // Ustawienie ComboBoxów przy użyciu findData()
        ui->New_item_type->setCurrentIndex(ui->New_item_type->findData(query.value("type_id")));
        ui->New_item_vendor->setCurrentIndex(ui->New_item_vendor->findData(query.value("vendor_id")));
        ui->New_item_model->setCurrentIndex(ui->New_item_model->findData(query.value("model_id")));
        ui->New_item_status->setCurrentIndex(ui->New_item_status->findData(query.value("status_id")));
        ui->New_item_storagePlace->setCurrentIndex(ui->New_item_storagePlace->findData(query.value("storage_place_id")));

        loadPhotos(recordId);
    } else {
        QMessageBox::warning(this, tr("Błąd"), tr("Nie znaleziono rekordu o id %1").arg(recordId));
    }
}

void MainWindow::loadPhotos(int recordId)
{
    QSqlQuery query(db);
    query.prepare("SELECT id, photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", recordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęć:" << query.lastError().text();
        ui->graphicsView->setScene(nullptr);
        return;
    }
    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbnailSize = 80;
    const int spacing = 5;
    int x = 5, y = 5, index = 0;
    while (query.next()) {
        QByteArray imageData = query.value("photo").toByteArray();
        QPixmap pixmap;
        if (!pixmap.loadFromData(imageData)) {
            qDebug() << "Nie można załadować zdjęcia BLOB dla rekordu" << recordId;
            continue;
        }
        QPixmap scaled = pixmap.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        PhotoItem *item = new PhotoItem();
        item->setPixmap(scaled);
        item->setData(0, query.value("id").toInt());
        item->setData(1, index);
        connect(item, &PhotoItem::clicked, this, [this, item]() { onPhotoClicked(item); });
        item->setPos(x, y);
        scene->addItem(item);
        x += thumbnailSize + spacing;
        if (x + thumbnailSize > ui->graphicsView->width() - 10) {
            x = 5;
            y += thumbnailSize + spacing;
        }
        ++index;
    }
    if (!scene->items().isEmpty()) {
        scene->setSceneRect(0, 0, ui->graphicsView->width() - 10, y + thumbnailSize + 5);
        ui->graphicsView->setScene(scene);
        ui->graphicsView->resetTransform();
        qreal scaleFactor = qMin((ui->graphicsView->width() - 10.0) / scene->width(),
                                 (ui->graphicsView->height() - 10.0) / scene->height());
        if (scaleFactor < 1.0) scaleFactor = 1.0;
        ui->graphicsView->scale(scaleFactor, scaleFactor);
        if (m_selectedPhotoIndex == -1) {
            for (QGraphicsItem *itm : scene->items()) {
                if (PhotoItem *photoItem = dynamic_cast<PhotoItem*>(itm))
                    photoItem->setSelected(false);
            }
        } else {
            QList<QGraphicsItem*> items = scene->items();
            if (m_selectedPhotoIndex >= 0 && m_selectedPhotoIndex < items.size()) {
                if (PhotoItem *photoItem = dynamic_cast<PhotoItem*>(items[m_selectedPhotoIndex]))
                    photoItem->setSelected(true);
            }
        }
    } else {
        ui->graphicsView->setScene(nullptr);
        delete scene;
    }
}

///////////////////////
// Obsługa zapisu, anulowania, zdjęć //
///////////////////////

void MainWindow::onSaveClicked()
{
    QSqlQuery query(db);
    int newRecordId = -1;
    if (!m_editMode) {
        query.prepare(R"(
            INSERT INTO eksponaty
            (name, serial_number, part_number, revision, production_year, status_id,
             type_id, vendor_id, model_id, storage_place_id, description, value, image_path)
            VALUES
            (:name, :serial_number, :part_number, :revision, :production_year, :status_id,
             :type_id, :vendor_id, :model_id, :storage_place_id, :description, :value, :image_path)
        )");
    } else {
        query.prepare(R"(
            UPDATE eksponaty
            SET name = :name, serial_number = :serial_number, part_number = :part_number, revision = :revision,
                production_year = :production_year, status_id = :status_id, type_id = :type_id,
                vendor_id = :vendor_id, model_id = :model_id, storage_place_id = :storage_place_id,
                description = :description, value = :value, image_path = :image_path
            WHERE id = :id
        )");
        query.bindValue(":id", m_recordId);
    }

    query.bindValue(":name", ui->New_item_name->text());
    query.bindValue(":serial_number", ui->New_item_serialNumber->text());
    query.bindValue(":part_number", ui->New_item_partNumber->text());
    query.bindValue(":revision", ui->New_item_revision->text());
    query.bindValue(":production_year", ui->New_item_ProductionDate->date().year());
    query.bindValue(":status_id", ui->New_item_status->currentData().isValid() ? ui->New_item_status->currentData() : 1);
    query.bindValue(":type_id", ui->New_item_type->currentData().isValid() ? ui->New_item_type->currentData() : 1);
    query.bindValue(":vendor_id", ui->New_item_vendor->currentData().isValid() ? ui->New_item_vendor->currentData() : 1);
    query.bindValue(":model_id", ui->New_item_model->currentData().isValid() ? ui->New_item_model->currentData() : 1);
    query.bindValue(":storage_place_id", ui->New_item_storagePlace->currentData().isValid() ? ui->New_item_storagePlace->currentData() : 1);
    query.bindValue(":description", ui->New_item_description->toPlainText());
    query.bindValue(":value", ui->New_item_value->text().isEmpty() ? 0 : ui->New_item_value->text().toInt());
    query.bindValue(":image_path", "");

    if (!query.exec()) {
        qDebug() << "Błąd zapisu:" << query.lastError().text();
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zapisać rekordu:\n%1")
                                                    .arg(query.lastError().text()));
        return;
    }

    if (!m_editMode) {
        newRecordId = query.lastInsertId().toInt();
        m_recordId = newRecordId;
    } else {
        newRecordId = m_recordId;
    }

    emit recordSaved(newRecordId);
    QMessageBox::information(this, tr("Sukces"), tr("Operacja zapisu zakończona powodzeniem."));
    close();
}

void MainWindow::onCancelClicked()
{
    close();
}

void MainWindow::onAddPhotoClicked()
{
    if (m_recordId == -1) {
        QMessageBox::warning(this, tr("Błąd"), tr("Najpierw zapisz rekord, aby dodać zdjęcie."));
        return;
    }
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Wybierz zdjęcia"),
                                                          QString(),
                                                          tr("Images (*.jpg *.jpeg *.png)"));
    if (fileNames.isEmpty()) return;

    QSqlQuery query(db);
    for (const QString &fileName : fileNames) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Nie można otworzyć pliku:" << fileName;
            continue;
        }
        QByteArray imageData = file.readAll();
        file.close();

        query.prepare("INSERT INTO photos (eksponat_id, photo) VALUES (:eksponat_id, :photo)");
        query.bindValue(":eksponat_id", m_recordId);
        query.bindValue(":photo", imageData);
        if (!query.exec()) {
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zapisać zdjęcia:\n%1")
                                                        .arg(query.lastError().text()));
        } else {
            qDebug() << "Zdjęcie zapisane dla rekordu" << m_recordId;
        }
    }
    loadPhotos(m_recordId);
}

void MainWindow::onRemovePhotoClicked()
{
    if (m_selectedPhotoIndex == -1) {
        QMessageBox::warning(this, tr("Błąd"), tr("Najpierw wybierz zdjęcie do usunięcia."));
        return;
    }
    QGraphicsScene *scene = ui->graphicsView->scene();
    if (!scene) {
        QMessageBox::warning(this, tr("Błąd"), tr("Brak zdjęć do usunięcia."));
        return;
    }
    QList<QGraphicsItem*> items = scene->items();
    if (m_selectedPhotoIndex >= 0 && m_selectedPhotoIndex < items.size()) {
        PhotoItem *selectedItem = dynamic_cast<PhotoItem*>(items[m_selectedPhotoIndex]);
        if (selectedItem) {
            int photoId = selectedItem->data(0).toInt();
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Potwierdzenie"),
                                          tr("Czy na pewno chcesz usunąć wybrane zdjęcie?"),
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QSqlQuery query(db);
                query.prepare("DELETE FROM photos WHERE id = :id");
                query.bindValue(":id", photoId);
                if (!query.exec()) {
                    QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć zdjęcia:\n%1")
                                                                .arg(query.lastError().text()));
                } else {
                    loadPhotos(m_recordId);
                }
            }
        }
    }
    m_selectedPhotoIndex = -1;
}

void MainWindow::onPhotoClicked(PhotoItem *item)
{
    QGraphicsScene *scene = ui->graphicsView->scene();
    if (!scene) return;
    QList<QGraphicsItem*> items = scene->items();
    for (int i = 0; i < items.size(); ++i) {
        if (PhotoItem *photoItem = dynamic_cast<PhotoItem*>(items[i])) {
            photoItem->setSelected(items[i] == item);
            if (items[i] == item)
                m_selectedPhotoIndex = i;
        }
    }
    qDebug() << "Wybrane zdjęcie o indeksie:" << m_selectedPhotoIndex;
}

///////////////////////
// Dialogi słowników //
///////////////////////

void MainWindow::onAddTypeClicked()
{
    typy *typeDialog = new typy(this);
    typeDialog->setMainWindow(this);
    typeDialog->exec();
    delete typeDialog;
}

void MainWindow::onAddVendorClicked()
{
    vendors *vendorDialog = new vendors(this);
    vendorDialog->setMainWindow(this);
    vendorDialog->exec();
    delete vendorDialog;
}

void MainWindow::onAddModelClicked()
{
    models *modelDialog = new models(this);
    modelDialog->setMainWindow(this);
    int vendorId = ui->New_item_vendor->currentData().isValid() ? ui->New_item_vendor->currentData().toInt() : -1;
    if (vendorId != -1) {
        modelDialog->setVendorId(vendorId);
    }
    modelDialog->exec();
    delete modelDialog;
}
