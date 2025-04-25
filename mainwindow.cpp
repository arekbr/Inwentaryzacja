#include "mainwindow.h"
#include "ui_mainwindow.h"

// Inne ewentualne include:
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QInputDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QSettings>
#include <QUuid>   // <-- do generowania UUID, jeśli potrzebne
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include <QMessageBox>
#include <QSqlQuery>
#include <QCompleter>

#include "photoitem.h"
#include "types.h"
#include "models.h"
#include "vendors.h"
#include "status.h"
#include "storage.h"

///////////////////////
// Konstruktor / Destruktor
///////////////////////

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_editMode(false),
    m_recordId(QString()),
    m_selectedPhotoIndex(-1)
{
    ui->setupUi(this);

    // Pobranie już istniejącego połączenia (np. SQLite czy MySQL) zdefiniowanego w "default_connection"
    db = QSqlDatabase::database("default_connection");
    if (!db.isOpen()) {
        QMessageBox::critical(this, tr("Błąd bazy danych"),
                              tr("Brak otwartego połączenia z bazą danych (default_connection)."));
        return;
    }

    // Ładowanie danych do ComboBoxów
    loadComboBoxData("types",           ui->New_item_type);
    loadComboBoxData("vendors",         ui->New_item_vendor);
    loadComboBoxData("models",          ui->New_item_model);
    loadComboBoxData("statuses",        ui->New_item_status);
    loadComboBoxData("storage_places",  ui->New_item_storagePlace);

    // Podpinamy sygnały / sloty przycisków
    connect(ui->New_item_PushButton_OK,     &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(ui->New_item_PushButton_Cancel, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(ui->New_item_addPhoto,          &QPushButton::clicked, this, &MainWindow::onAddPhotoClicked);
    connect(ui->New_item_removePhoto,       &QPushButton::clicked, this, &MainWindow::onRemovePhotoClicked);

    // Słowniki
    connect(ui->New_item_addType,           &QPushButton::clicked, this, &MainWindow::onAddTypeClicked);
    connect(ui->New_item_addVendor,         &QPushButton::clicked, this, &MainWindow::onAddVendorClicked);
    connect(ui->New_item_addModel,          &QPushButton::clicked, this, &MainWindow::onAddModelClicked);
    connect(ui->New_item_addStatus,         &QPushButton::clicked, this, &MainWindow::onAddStatusClicked);
    connect(ui->New_item_addStoragePlace,   &QPushButton::clicked, this, &MainWindow::onAddStoragePlaceClicked);

    // Automatyczne dodawanie nowego wpisu do bazy jeśli użytkownik wpisał coś nowego w ComboBoxie
    auto autoInsert = [this](QComboBox *comboBox, const QString &tableName) {
        comboBox->setEditable(true);
        connect(comboBox->lineEdit(), &QLineEdit::editingFinished, this, [this, comboBox, tableName]() {
            QString text = comboBox->currentText().trimmed();
            if (text.isEmpty() || comboBox->findText(text, Qt::MatchFixedString | Qt::MatchCaseSensitive) != -1)
                return;

            QSqlQuery q(db);
            QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

            if (tableName == "models") {
                int vendorIndex = ui->New_item_vendor->currentIndex();
                QVariant vendorData = ui->New_item_vendor->itemData(vendorIndex);

                if (!vendorData.isValid() || vendorData.toString().isEmpty()) {
                    QMessageBox::warning(this, tr("Błąd"),
                                         tr("Nie można dodać modelu – najpierw wybierz lub zapisz producenta."));
                    return;
                }

                QString vendorId = vendorData.toString();
                q.prepare("INSERT INTO models (id, name, vendor_id) VALUES (:id, :name, :vendor_id)");
                q.bindValue(":vendor_id", vendorId);
            } else {
                q.prepare(QString("INSERT INTO %1 (id, name) VALUES (:id, :name)").arg(tableName));
            }

            q.bindValue(":id", uuid);
            q.bindValue(":name", text);

            if (!q.exec()) {
                QMessageBox::warning(this, tr("Błąd"),
                                     tr("Nie udało się dodać nowej wartości do '%1':\n%2")
                                         .arg(tableName, q.lastError().text()));
                return;
            }

            loadComboBoxData(tableName, comboBox);
            comboBox->setCurrentIndex(comboBox->findText(text));
        });
    };



    autoInsert(ui->New_item_type, "types");
    autoInsert(ui->New_item_vendor, "vendors");
    autoInsert(ui->New_item_model, "models");
    autoInsert(ui->New_item_status, "statuses");
    autoInsert(ui->New_item_storagePlace, "storage_places");




    // Ustawienia wstępne // === [KONIEC konstruktora MainWindow] ===

    ui->graphicsView->setTransform(QTransform());
    // === [KONIEC konstruktora MainWindow] ===

}

MainWindow::~MainWindow()
{
    if (db.isOpen()) {
        db.close();
    }
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QComboBox *combo = qobject_cast<QComboBox*>(obj);
        if (combo && combo->view()) {
            combo->showPopup();
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


///////////////////////
// Metody getNewItemXxxComboBox()
///////////////////////

// Te definicje są kluczowe, by linker nie zgłaszał "undefined reference"

QComboBox* MainWindow::getNewItemVendorComboBox() const
{
    return ui->New_item_vendor;
}

QComboBox* MainWindow::getNewItemModelComboBox() const
{
    return ui->New_item_model;
}

QComboBox* MainWindow::getNewItemStoragePlaceComboBox() const
{
    return ui->New_item_storagePlace;
}

QComboBox* MainWindow::getNewItemTypeComboBox() const
{
    return ui->New_item_type;
}

QComboBox* MainWindow::getNewItemStatusComboBox() const
{
    return ui->New_item_status;
}

///////////////////////
// Metoda ładowania ComboBoxa
///////////////////////
void MainWindow::loadComboBoxData(const QString &tableName, QComboBox *comboBox)
{
    comboBox->clear();
    QSqlQuery query(db);
    if (query.exec(QString("SELECT id, name FROM %1").arg(tableName))) {
        while (query.next()) {
            const QString itemName = query.value("name").toString();
            const QString itemId   = query.value("id").toString();
            comboBox->addItem(itemName, itemId);
        }
    } else {
        qDebug() << "Błąd w loadComboBoxData dla" << tableName << ":" << query.lastError().text();
    }
}

///////////////////////
// setEditMode
///////////////////////
void MainWindow::setEditMode(bool edit, const QString &recordId)
{
    m_editMode = edit;
    m_recordId = recordId;

    if (m_editMode && !m_recordId.isEmpty()) {
        loadRecord(m_recordId);
    } else {
        // Czyszczenie pól
        ui->New_item_name->clear();
        ui->New_item_serialNumber->clear();
        ui->New_item_partNumber->clear();
        ui->New_item_revision->clear();
        ui->New_item_value->clear();
        ui->New_item_description->clear();
        ui->New_item_ProductionDate->setDate(QDate::currentDate());

        // ComboBoxy czyścimy / resetujemy
        QList<QComboBox*> combos = {
            ui->New_item_type,
            ui->New_item_vendor,
            ui->New_item_model,
            ui->New_item_status,
            ui->New_item_storagePlace
        };
        for (QComboBox *c : combos) {
            c->setEditable(true);           // Pozwala wpisywać nowe dane
            c->clearEditText();            // Czyści pole (ale nie listę!)
            c->setCurrentIndex(-1);        // Nie wybiera nic domyślnie
            c->completer()->setCompletionMode(QCompleter::PopupCompletion); // Pokazuje sugestie
        }


        // Czyszczenie sceny zdjęć
        ui->graphicsView->setScene(nullptr);
        m_selectedPhotoIndex = -1;
        m_photoBuffer.clear();
    }
    // >>> Ustaw domyślne "brak" dla status i miejsce przechowywania
    auto setDefaultIfAvailable = [](QComboBox *comboBox, const QString &targetName) {
        int index = comboBox->findText(targetName, Qt::MatchFixedString | Qt::MatchCaseSensitive);
        if (index != -1) {
            comboBox->setCurrentIndex(index);
        }
    };

    setDefaultIfAvailable(ui->New_item_status, "brak");
    setDefaultIfAvailable(ui->New_item_storagePlace, "brak");
    // <<< koniec sekcji

}

///////////////////////
// setCloneMode na maku ok
///////////////////////
void MainWindow::setCloneMode(const QString &recordId)
{
    // Ładujemy istniejący rekord, ale docelowo to będzie nowy (kopiowany)
    m_editMode = false;          // to będzie nowy rekord
    loadRecord(recordId);        // skopiuj dane tekstowe
    m_recordId.clear();          // brak ID = tryb „nowy”

    // >>> NOWE – wyczyść zdjęcia <<<
    ui->graphicsView->setScene(nullptr);
    m_selectedPhotoIndex = -1;
    m_photoBuffer.clear();
    m_photoPathsBuffer.clear();
}

///////////////////////
// Ładowanie rekordu
///////////////////////
void MainWindow::loadRecord(const QString &recordId)
{
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT name, serial_number, part_number, revision,
               production_year, status_id, type_id, vendor_id,
               model_id, storage_place_id, description, value
        FROM eksponaty
        WHERE id = :id
    )");
    query.bindValue(":id", recordId);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, tr("Błąd"),
                             tr("Nie znaleziono rekordu o ID %1").arg(recordId));
        return;
    }

    ui->New_item_name->setText(query.value("name").toString());
    ui->New_item_serialNumber->setText(query.value("serial_number").toString());
    ui->New_item_partNumber->setText(query.value("part_number").toString());
    ui->New_item_revision->setText(query.value("revision").toString());
    ui->New_item_value->setText(query.value("value").toString());
    ui->New_item_description->setPlainText(query.value("description").toString());

    // Rok => QDate
    int prodYear = query.value("production_year").toInt();
    ui->New_item_ProductionDate->setDate(QDate(prodYear, 1, 1));

    // Ustawiamy ComboBoxy wg ID
    ui->New_item_type->setCurrentIndex(
        ui->New_item_type->findData(query.value("type_id").toString()));
    ui->New_item_vendor->setCurrentIndex(
        ui->New_item_vendor->findData(query.value("vendor_id").toString()));
    ui->New_item_model->setCurrentIndex(
        ui->New_item_model->findData(query.value("model_id").toString()));
    ui->New_item_status->setCurrentIndex(
        ui->New_item_status->findData(query.value("status_id").toString()));
    ui->New_item_storagePlace->setCurrentIndex(
        ui->New_item_storagePlace->findData(query.value("storage_place_id").toString()));

    // Załaduj zdjęcia
    loadPhotos(recordId);
}

///////////////////////
// Ładowanie zdjęć
///////////////////////
void MainWindow::loadPhotos(const QString &recordId)
{
    QSqlQuery query(db);
    query.prepare("SELECT id, photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", recordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęć:" << query.lastError().text();
        ui->graphicsView->setScene(nullptr);
        return;
    }

    // Tworzymy scenę z miniaturami
    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbSize=80, spacing=5;
    int x=5, y=5, idx=0;

    while (query.next()) {
        QByteArray imgData = query.value("photo").toByteArray();
        QPixmap pix;
        if (!pix.loadFromData(imgData)) {
            qDebug() << "Nie można załadować BLOB zdjęcia";
            continue;
        }
        QPixmap scaled = pix.scaled(thumbSize, thumbSize,
                                    Qt::KeepAspectRatio, Qt::SmoothTransformation);

        PhotoItem *item = new PhotoItem();
        item->setPixmap(scaled);
        item->setData(0, query.value("id").toString()); // ID zdjęcia
        item->setData(1, idx);

        // sygnał clicked => onPhotoClicked
        connect(item, &PhotoItem::clicked, this, [this, item]() {
            onPhotoClicked(item);
        });

        item->setPos(x,y);
        scene->addItem(item);

        x += (scaled.width()+spacing);
        if (x + thumbSize > ui->graphicsView->width()-10) {
            x=5;
            y += (scaled.height()+spacing);
        }
        idx++;
    }

    if (!scene->items().isEmpty()) {
        scene->setSceneRect(0,0, ui->graphicsView->width()-10, y+thumbSize+5);
        ui->graphicsView->setScene(scene);
        ui->graphicsView->resetTransform();

        qreal scaleFactor = qMin(
            (ui->graphicsView->width()-10.0)/scene->width(),
            (ui->graphicsView->height()-10.0)/scene->height()
            );
        if (scaleFactor<1.0) scaleFactor=1.0;
        ui->graphicsView->scale(scaleFactor, scaleFactor);
    }
    else {
        ui->graphicsView->setScene(nullptr);
        delete scene;
    }
}

///////////////////////
// Bufor zdjęć (gdy nowy rekord nie zapisany w bazie)
///////////////////////
void MainWindow::loadPhotosFromBuffer()
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbSize=80, spacing=5;
    int x=5,y=5;

    for (int i=0; i<m_photoBuffer.size(); i++){
        QPixmap pix;
        if (!pix.loadFromData(m_photoBuffer[i])) {
            qDebug() << "Błąd loadFromData w buforze zdjęć";
            continue;
        }
        QPixmap scaled = pix.scaled(thumbSize, thumbSize,
                                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QGraphicsPixmapItem *it = scene->addPixmap(scaled);
        it->setPos(x,y);
        x += (scaled.width()+spacing);
        if (x + thumbSize > ui->graphicsView->width()-10) {
            x=5;
            y += (scaled.height()+spacing);
        }
    }
    ui->graphicsView->setScene(scene);
}



///////////////////////
// Obsługa zapisu
///////////////////////
void MainWindow::onSaveClicked()
{
    QSqlQuery q(db);

    if (!m_editMode) {
        // Nowy rekord -> generujemy UUID
        m_recordId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        q.prepare(R"(
            INSERT INTO eksponaty
            (id, name, serial_number, part_number, revision, production_year,
             status_id, type_id, vendor_id, model_id, storage_place_id,
             description, value)
            VALUES
            (:id, :name, :serial_number, :part_number, :revision, :production_year,
             :status_id, :type_id, :vendor_id, :model_id, :storage_place_id,
             :description, :value)
        )");
        q.bindValue(":id", m_recordId);
    }
    else {
        // Edycja
        q.prepare(R"(
            UPDATE eksponaty
            SET name=:name,
                serial_number=:serial_number,
                part_number=:part_number,
                revision=:revision,
                production_year=:production_year,
                status_id=:status_id,
                type_id=:type_id,
                vendor_id=:vendor_id,
                model_id=:model_id,
                storage_place_id=:storage_place_id,
                description=:description,
                value=:value
            WHERE id=:id
        )");
        q.bindValue(":id", m_recordId);
    }

    q.bindValue(":name", ui->New_item_name->text());
    q.bindValue(":serial_number", ui->New_item_serialNumber->text());
    q.bindValue(":part_number", ui->New_item_partNumber->text());
    q.bindValue(":revision", ui->New_item_revision->text());
    q.bindValue(":production_year", ui->New_item_ProductionDate->date().year());

    // Klucze obce
    QString statusId = ui->New_item_status->currentData().toString();
    if (statusId.isEmpty()) statusId="unknown_status_uuid";

    QString typeId = ui->New_item_type->currentData().toString();
    if (typeId.isEmpty()) typeId="unknown_type_uuid";

    QString vendorId = ui->New_item_vendor->currentData().toString();
    if (vendorId.isEmpty()) vendorId="unknown_vendor_uuid";

    QString modelId = ui->New_item_model->currentData().toString();
    if (modelId.isEmpty()) modelId="unknown_model_uuid";

    QString storageId = ui->New_item_storagePlace->currentData().toString();
    if (storageId.isEmpty()) storageId="unknown_storage_uuid";

    q.bindValue(":status_id",        statusId);
    q.bindValue(":type_id",          typeId);
    q.bindValue(":vendor_id",        vendorId);
    q.bindValue(":model_id",         modelId);
    q.bindValue(":storage_place_id", storageId);

    q.bindValue(":description", ui->New_item_description->toPlainText());
    q.bindValue(":value",
                ui->New_item_value->text().isEmpty()
                    ? 0
                    : ui->New_item_value->text().toInt());

    if (!q.exec()) {
        QMessageBox::critical(this, tr("Błąd"),
                              tr("Nie udało się zapisać:\n%1").arg(q.lastError().text()));
        return;
    }

    // jeśli to nowy rekord, zapisujemy zdjęcia z bufora i przenosimy pliki
    if (!m_editMode) {
        for (int i = 0; i < m_photoBuffer.size(); ++i) {
            QByteArray ba = m_photoBuffer[i];
            QString pid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QSqlQuery q2(db);
            q2.prepare(R"(
                INSERT INTO photos (id, eksponat_id, photo)
                VALUES (:id, :exid, :photo)
            )");
            q2.bindValue(":id",    pid);
            q2.bindValue(":exid",  m_recordId);
            q2.bindValue(":photo", ba);
            if (q2.exec()) {
                // teraz przenieś plik z bufora ścieżek
                const QString orig = m_photoPathsBuffer.at(i);
                QFileInfo fi(orig);
                QString doneDir = fi.absolutePath()
                                  + QDir::separator()
                                  + QStringLiteral("gotowe");
                QDir().mkpath(doneDir);
                QString dst = doneDir
                              + QDir::separator()
                              + fi.fileName();
                QFile::rename(orig, dst);
            }
        }
        m_photoBuffer.clear();
        m_photoPathsBuffer.clear();
    }

    emit recordSaved(m_recordId);
    QMessageBox::information(this, tr("Sukces"), tr("Rekord zapisany pomyślnie."));
    close();
}

///////////////////////
// Anuluj
///////////////////////
void MainWindow::onCancelClicked()
{
    close();
}

///////////////////////
// Dodawanie zdjęć
///////////////////////
void MainWindow::onAddPhotoClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Wybierz zdjęcia"), QString(),
        tr("Images (*.jpg *.jpeg *.png)"));
    if (files.isEmpty()) return;

    for (const auto &fn : std::as_const(files)) {
        QFile f(fn);
        if (!f.open(QIODevice::ReadOnly)) {
            qDebug() << "Nie można otworzyć:" << fn;
            continue;
        }
        QByteArray data = f.readAll();
        f.close();


        if (m_recordId.isEmpty()) {
            m_photoBuffer.append(data);
            m_photoPathsBuffer.append(fn);
        } else {
            // Rekord już w bazie
            QString photoId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QSqlQuery q(db);
            q.prepare(R"(
                INSERT INTO photos (id, eksponat_id, photo)
                VALUES (:id, :exid, :photo)
            )");
            q.bindValue(":id",    photoId);
            q.bindValue(":exid",  m_recordId);
            q.bindValue(":photo", data);
            if (q.exec()) {
                // przenieś plik
                QFileInfo fi(fn);
                // Pełna ścieżka do katalogu „gotowe”
                QString doneDir = fi.absolutePath()
                                  + QDir::separator()
                                  + QStringLiteral("gotowe");
                // tworzy katalog (i wszystkie potrzebne nadrzędne), jeśli nie istnieje
                if (!QDir().mkpath(doneDir)) {
                    QMessageBox::warning(this, tr("Uwaga"),
                                         tr("Nie udało się utworzyć katalogu:\n%1").arg(doneDir));
                }
                QString dst = doneDir
                              + QDir::separator()
                              + fi.fileName();
                if (!QFile::rename(fn, dst)) {
                    QMessageBox::warning(this, tr("Uwaga"),
                                         tr("Nie można przenieść %1 do %2").arg(fn, dst));
                }
            } else {
                QMessageBox::critical(this, tr("Błąd"),
                                      tr("Nie można zapisać zdjęcia:\n%1")
                                          .arg(q.lastError().text()));
            }
        }
    }

    // Odświeżamy podgląd
    if (m_recordId.isEmpty())
        loadPhotosFromBuffer();
    else
        loadPhotos(m_recordId);
    this->raise();
    this->activateWindow();

}



///////////////////////
// Usuwanie zdjęć
///////////////////////
void MainWindow::onRemovePhotoClicked()
{
    if (m_selectedPhotoIndex == -1) {
        QMessageBox::warning(this, tr("Błąd"), tr("Najpierw wybierz zdjęcie do usunięcia."));
        return;
    }
    QGraphicsScene *scene = ui->graphicsView->scene();
    if (!scene) {
        QMessageBox::warning(this, tr("Błąd"), tr("Brak zdjęć w widoku."));
        return;
    }
    QList<QGraphicsItem*> items = scene->items();
    if (m_selectedPhotoIndex < 0 || m_selectedPhotoIndex >= items.size()) {
        return;
    }
    PhotoItem *selItem = dynamic_cast<PhotoItem*>(items[m_selectedPhotoIndex]);
    if (!selItem) return;

    QString photoId = selItem->data(0).toString();
    auto ans = QMessageBox::question(this, tr("Potwierdzenie"),
                                     tr("Czy usunąć zdjęcie?"),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ans==QMessageBox::Yes) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM photos WHERE id=:id");
        q.bindValue(":id", photoId);
        if (!q.exec()) {
            QMessageBox::critical(this, tr("Błąd"),
                                  tr("Nie można usunąć zdjęcia:\n%1").arg(q.lastError().text()));
        } else {
            loadPhotos(m_recordId);
        }
    }
    m_selectedPhotoIndex=-1;
}

///////////////////////
// Kliknięcie zdjęcia w miniaturce
///////////////////////
void MainWindow::onPhotoClicked(PhotoItem *item)
{
    QGraphicsScene *scene = ui->graphicsView->scene();
    if (!scene) return;

    QList<QGraphicsItem*> its = scene->items();
    for (int i=0; i<its.size(); i++){
        PhotoItem *pit = dynamic_cast<PhotoItem*>(its[i]);
        if (pit) {
            pit->setSelected(its[i]==item);
            if (its[i]==item) {
                m_selectedPhotoIndex = i;
            }
        }
    }
    qDebug() << "Wybrane zdjęcie o indeksie:" << m_selectedPhotoIndex;
}

///////////////////////
// Dialogi słowników
///////////////////////
void MainWindow::onAddTypeClicked()
{
    typy dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    // Dodajemy te dwie linijki:
    this->raise();
    this->activateWindow();
}

void MainWindow::onAddVendorClicked()
{
    vendors dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    // Dodajemy te dwie linijki:
    this->raise();
    this->activateWindow();
}

void MainWindow::onAddModelClicked()
{
    models dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    // Dodajemy te dwie linijki:
    this->raise();
    this->activateWindow();
}

void MainWindow::onAddStatusClicked()
{
    status dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    // Dodajemy te dwie linijki:
    this->raise();
    this->activateWindow();
}

void MainWindow::onAddStoragePlaceClicked()
{
    storage dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    // Dodajemy te dwie linijki:
    this->raise();
    this->activateWindow();
}
