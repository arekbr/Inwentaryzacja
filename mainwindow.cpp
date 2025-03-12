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
#include "photoitem.h"

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
    connect(ui->New_item_removePhoto, &QPushButton::clicked, this, &MainWindow::onRemovePhotoClicked);

    // Wyłącz automatyczne skalowanie widoku
    ui->graphicsView->setTransform(QTransform());
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

        // W trybie dodawania czyścimy widok zdjęć
        ui->graphicsView->setScene(nullptr);
        m_selectedPhotoIndex = -1;
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

        // Załaduj zdjęcia do graphicsView
        loadPhotos(recordId);
    } else {
        QMessageBox::warning(this, tr("Błąd"), tr("Nie znaleziono rekordu o id %1").arg(recordId));
    }
}

void MainWindow::loadPhotos(int recordId)
{
    // Pobierz wszystkie zdjęcia z tabeli photos dla danego rekordu
    QSqlQuery query(db);
    query.prepare("SELECT id, photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", recordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęć:" << query.lastError().text();
        ui->graphicsView->setScene(nullptr);
        return;
    }

    // Utwórz nową scenę graficzną
    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbnailSize = 80; // Rozmiar miniatur (kwadrat 80x80 pikseli)
    int x = 5; // Początkowa pozycja X z marginesem
    int y = 5; // Początkowa pozycja Y z marginesem
    const int spacing = 5; // Odstęp między miniaturami

    int index = 0;
    while (query.next()) {
        QByteArray imageData = query.value("photo").toByteArray();
        QPixmap pixmap;
        if (!pixmap.loadFromData(imageData)) {
            qDebug() << "Nie można załadować zdjęcia z danych BLOB dla rekordu" << recordId;
            continue;
        }

        // Skaluj obraz do rozmiaru miniatury, zachowując proporcje
        QPixmap scaledPixmap = pixmap.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Utwórz niestandardowy PhotoItem
        PhotoItem *item = new PhotoItem();
        item->setPixmap(scaledPixmap);
        item->setData(0, query.value("id").toInt()); // Przechowujemy ID zdjęcia
        item->setData(1, index); // Przechowujemy indeks w siatce
        connect(item, &PhotoItem::clicked, this, [this, item]() { onPhotoClicked(item); });
        item->setPos(x, y);
        scene->addItem(item);

        // Aktualizuj pozycję dla następnej miniatury
        x += thumbnailSize + spacing;
        if (x + thumbnailSize > ui->graphicsView->width() - 10) {
            x = 5;
            y += thumbnailSize + spacing;
        }
        index++;
    }

    // Ustaw scenę w widżecie
    if (!scene->items().isEmpty()) {
        // Ustaw rozmiar sceny na podstawie liczby miniatur
        scene->setSceneRect(0, 0, ui->graphicsView->width() - 10, y + thumbnailSize + 5);
        ui->graphicsView->setScene(scene);

        // Resetuj transformację widoku, aby uniknąć przeskalowania
        ui->graphicsView->resetTransform();

        // Ustaw minimalny zoom, aby miniatury nie były zbyt małe
        qreal scaleFactor = qMin(
            (ui->graphicsView->width() - 10.0) / scene->width(),
            (ui->graphicsView->height() - 10.0) / scene->height()
            );
        if (scaleFactor < 1.0) {
            scaleFactor = 1.0; // Nie zmniejszamy miniatur poniżej ich naturalnego rozmiaru
        }
        ui->graphicsView->scale(scaleFactor, scaleFactor);

        // Odznacz wszystkie zdjęcia po załadowaniu (jeśli nie było wcześniej zaznaczenia)
        if (m_selectedPhotoIndex == -1) {
            QList<QGraphicsItem*> items = scene->items();
            for (QGraphicsItem *item : items) {
                if (PhotoItem *photoItem = dynamic_cast<PhotoItem*>(item)) {
                    photoItem->setSelected(false);
                }
            }
        } else {
            // Przywróć poprzednie zaznaczenie, jeśli istnieje
            QList<QGraphicsItem*> items = scene->items();
            if (m_selectedPhotoIndex >= 0 && m_selectedPhotoIndex < items.size()) {
                if (PhotoItem *photoItem = dynamic_cast<PhotoItem*>(items[m_selectedPhotoIndex])) {
                    photoItem->setSelected(true);
                }
            }
        }
    } else {
        ui->graphicsView->setScene(nullptr);
        delete scene;
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

    emit recordSaved(newRecordId); // Emitujemy sygnał z ID zapisanej pozycji
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

    // Odśwież widok zdjęć w formularzu
    loadPhotos(m_recordId);
}

void MainWindow::onRemovePhotoClicked()
{
    if (m_selectedPhotoIndex == -1) {
        QMessageBox::warning(this, tr("Błąd"), tr("Najpierw wybierz zdjęcie do usunięcia."));
        return;
    }

    // Pobierz ID wybranego zdjęcia z sceny
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

            // Potwierdzenie usunięcia
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Potwierdzenie"),
                                          tr("Czy na pewno chcesz usunąć wybrane zdjęcie?"),
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QSqlQuery query(db);
                query.prepare("DELETE FROM photos WHERE id = :id");
                query.bindValue(":id", photoId);
                if (!query.exec()) {
                    qDebug() << "Błąd usuwania zdjęcia:" << query.lastError().text();
                    QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć zdjęcia:\n%1")
                                                                .arg(query.lastError().text()));
                } else {
                    qDebug() << "Zdjęcie o id" << photoId << "zostało usunięte.";
                    loadPhotos(m_recordId); // Odśwież siatkę
                }
            }
        }
    }

    m_selectedPhotoIndex = -1; // Resetuj wybrany indeks po operacji
}

void MainWindow::onPhotoClicked(PhotoItem *item)
{
    QGraphicsScene *scene = ui->graphicsView->scene();
    if (!scene) return;

    QList<QGraphicsItem*> items = scene->items();
    for (int i = 0; i < items.size(); ++i) {
        if (PhotoItem *photoItem = dynamic_cast<PhotoItem*>(items[i])) {
            photoItem->setSelected(items[i] == item);
            if (items[i] == item) {
                m_selectedPhotoIndex = i;
            }
        }
    }
    qDebug() << "Wybrane zdjęcie o indeksie:" << m_selectedPhotoIndex;
}
