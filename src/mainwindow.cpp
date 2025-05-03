/**
 * @file mainwindow.cpp
 * @brief Implementacja klasy MainWindow do zarządzania formularzem eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.2.2
 * @date 2025-05-03
 *
 * @section Overview
 * Plik zawiera implementację metod klasy MainWindow, odpowiedzialnej za formularz do dodawania,
 * edycji i klonowania eksponatów w aplikacji inwentaryzacyjnej. Klasa obsługuje ładowanie danych
 * do combo boxów, zarządzanie zdjęciami (w bazie i buforze), zapisywanie rekordów w bazie danych
 * MySQL oraz otwieranie dialogów słownikowych. Implementacja obejmuje również automatyczne
 * dodawanie nowych wartości do słowników po ich wpisaniu w combo boxach.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje interfejs, połączenie z bazą danych, combo boxy i sloty.
 * 2. **Destruktor** – zwalnia zasoby.
 * 3. **Metody publiczne** – ładowanie danych do combo boxów, ustawianie trybów, dostęp do combo boxów.
 * 4. **Sloty prywatne** – obsługują akcje użytkownika (zapis, anulowanie, zdjęcia, słowniki).
 * 5. **Metody prywatne** – wczytywanie rekordów i zdjęć.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QSqlQuery, QFileDialog, QMessageBox, QGraphicsScene, QCompleter, itp.
 * - **Nagłówki aplikacji**: mainwindow.h, photoitem.h, types.h, models.h, vendors.h, status.h, storage.h.
 * - **Interfejs użytkownika**: ui_mainwindow.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa obsługuje tylko MySQL w tej implementacji, choć aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 * - Automatyczne dodawanie wartości do słowników wymaga wybrania producenta dla modeli.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

// Inne nagłówki
#include <QCompleter>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include "models.h"
#include "photoitem.h"
#include "status.h"
#include "storage.h"
#include "types.h"
#include "vendors.h"

/**
 * @brief Konstruktor klasy MainWindow.
 * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
 *
 * @section ConstructorOverview
 * Inicjalizuje interfejs użytkownika, ustanawia połączenie z bazą danych, ładuje dane do combo boxów,
 * podłącza sygnały i sloty dla przycisków oraz konfiguruje automatyczne dodawanie nowych wartości
 * do słowników. Ustawia początkowy stan formularza (brak edycji, brak wybranego zdjęcia).
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_editMode(false)
    , m_recordId(QString())
    , m_selectedPhotoIndex(-1)
{
    ui->setupUi(this);

    // Pobranie istniejącego połączenia z bazy danych
    db = QSqlDatabase::database("default_connection");
    if (!db.isOpen()) {
        QMessageBox::critical(this,
                              tr("Błąd bazy danych"),
                              tr("Brak otwartego połączenia z bazą danych (default_connection)."));
        return;
    }

    // Ładowanie danych do ComboBoxów
    loadComboBoxData("types", ui->New_item_type);
    loadComboBoxData("vendors", ui->New_item_vendor);
    loadComboBoxData("models", ui->New_item_model);
    loadComboBoxData("statuses", ui->New_item_status);
    loadComboBoxData("storage_places", ui->New_item_storagePlace);

    // Podpinanie sygnałów i slotów przycisków
    connect(ui->New_item_PushButton_OK, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(ui->New_item_PushButton_Cancel,
            &QPushButton::clicked,
            this,
            &MainWindow::onCancelClicked);
    connect(ui->New_item_addPhoto, &QPushButton::clicked, this, &MainWindow::onAddPhotoClicked);
    connect(ui->New_item_removePhoto,
            &QPushButton::clicked,
            this,
            &MainWindow::onRemovePhotoClicked);

    // Słowniki
    connect(ui->New_item_addType, &QPushButton::clicked, this, &MainWindow::onAddTypeClicked);
    connect(ui->New_item_addVendor, &QPushButton::clicked, this, &MainWindow::onAddVendorClicked);
    connect(ui->New_item_addModel, &QPushButton::clicked, this, &MainWindow::onAddModelClicked);
    connect(ui->New_item_addStatus, &QPushButton::clicked, this, &MainWindow::onAddStatusClicked);
    connect(ui->New_item_addStoragePlace,
            &QPushButton::clicked,
            this,
            &MainWindow::onAddStoragePlaceClicked);

    // Automatyczne dodawanie nowych wartości do słowników
    auto autoInsert = [this](QComboBox *comboBox, const QString &tableName) {
        comboBox->setEditable(true);
        connect(comboBox->lineEdit(), &QLineEdit::editingFinished, this, [this, comboBox, tableName]() {
            QString text = comboBox->currentText().trimmed();
            if (text.isEmpty()
                || comboBox->findText(text, Qt::MatchFixedString | Qt::MatchCaseSensitive) != -1)
                return;

            QSqlQuery q(db);
            QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

            if (tableName == "models") {
                int vendorIndex = ui->New_item_vendor->currentIndex();
                QVariant vendorData = ui->New_item_vendor->itemData(vendorIndex);

                if (!vendorData.isValid() || vendorData.toString().isEmpty()) {
                    QMessageBox::warning(
                        this,
                        tr("Błąd"),
                        tr("Nie można dodać modelu – najpierw wybierz lub zapisz producenta."));
                    return;
                }

                QString vendorId = vendorData.toString();
                q.prepare(
                    "INSERT INTO models (id, name, vendor_id) VALUES (:id, :name, :vendor_id)");
                q.bindValue(":vendor_id", vendorId);
            } else {
                q.prepare(QString("INSERT INTO %1 (id, name) VALUES (:id, :name)").arg(tableName));
            }

            q.bindValue(":id", uuid);
            q.bindValue(":name", text);

            if (!q.exec()) {
                QMessageBox::warning(this,
                                     tr("Błąd"),
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

    // Ustawienia wstępne
    ui->graphicsView->setTransform(QTransform());
}

/**
 * @brief Destruktor klasy MainWindow.
 *
 * @section DestructorOverview
 * Zamyka połączenie z bazą danych, jeśli jest otwarte, i zwalnia zasoby interfejsu użytkownika.
 */
MainWindow::~MainWindow()
{
    if (db.isOpen()) {
        db.close();
    }
    delete ui;
}

/**
 * @brief Obsługuje zdarzenia filtrowania dla obiektów interfejsu.
 * @param obj Obiekt, dla którego przetwarzane jest zdarzenie.
 * @param event Wskaźnik na zdarzenie.
 * @return true, jeśli zdarzenie zostało obsłużone; false w przeciwnym razie.
 *
 * @section MethodOverview
 * Otwiera listę combo boxa po kliknięciu myszą, a następnie przekazuje zdarzenie
 * do QMainWindow dla dalszego przetwarzania.
 */
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QComboBox *combo = qobject_cast<QComboBox *>(obj);
        if (combo && combo->view()) {
            combo->showPopup();
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

/**
 * @brief Zwraca wskaźnik na combo box dla producentów.
 * @return Wskaźnik na QComboBox dla producentów.
 */
QComboBox *MainWindow::getNewItemVendorComboBox() const
{
    return ui->New_item_vendor;
}

/**
 * @brief Zwraca wskaźnik na combo box dla modeli.
 * @return Wskaźnik na QComboBox dla modeli.
 */
QComboBox *MainWindow::getNewItemModelComboBox() const
{
    return ui->New_item_model;
}

/**
 * @brief Zwraca wskaźnik na combo box dla miejsc przechowywania.
 * @return Wskaźnik na QComboBox dla miejsc przechowywania.
 */
QComboBox *MainWindow::getNewItemStoragePlaceComboBox() const
{
    return ui->New_item_storagePlace;
}

/**
 * @brief Zwraca wskaźnik na combo box dla typów.
 * @return Wskaźnik na QComboBox dla typów.
 */
QComboBox *MainWindow::getNewItemTypeComboBox() const
{
    return ui->New_item_type;
}

/**
 * @brief Zwraca wskaźnik na combo box dla statusów.
 * @return Wskaźnik na QComboBox dla statusów.
 */
QComboBox *MainWindow::getNewItemStatusComboBox() const
{
    return ui->New_item_status;
}

/**
 * @brief Ładuje dane z tabeli do combo boxa.
 * @param tableName Nazwa tabeli w bazie danych (np. "types", "vendors").
 * @param comboBox Wskaźnik na QComboBox, do którego ładowane są dane.
 *
 * @section MethodOverview
 * Pobiera unikalne wartości z kolumn "id" i "name" podanej tabeli i wypełnia nimi combo box,
 * ustawiając ID jako dane użytkownika dla każdej pozycji. Wyświetla błędy w konsoli debugowania.
 */
void MainWindow::loadComboBoxData(const QString &tableName, QComboBox *comboBox)
{
    comboBox->clear();
    QSqlQuery query(db);
    if (query.exec(QString("SELECT id, name FROM %1").arg(tableName))) {
        while (query.next()) {
            const QString itemName = query.value("name").toString();
            const QString itemId = query.value("id").toString();
            comboBox->addItem(itemName, itemId);
        }
    } else {
        qDebug() << "Błąd w loadComboBoxData dla" << tableName << ":" << query.lastError().text();
    }
}

/**
 * @brief Ustawia tryb edycji lub dodawania rekordu.
 * @param edit true dla trybu edycji, false dla trybu dodawania.
 * @param recordId ID rekordu do edycji (puste dla nowego rekordu).
 *
 * @section MethodOverview
 * Konfiguruje formularz w trybie edycji istniejącego rekordu lub dodawania nowego,
 * czyszcząc pola, ustawiając domyślne wartości dla statusu i miejsca przechowywania,
 * oraz włączając autouzupełnianie dla combo boxów.
 */
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
        QList<QComboBox *> combos = {ui->New_item_type,
                                     ui->New_item_vendor,
                                     ui->New_item_model,
                                     ui->New_item_status,
                                     ui->New_item_storagePlace};
        for (QComboBox *c : combos) {
            c->setEditable(true);
            c->clearEditText();
            c->setCurrentIndex(-1);
            c->completer()->setCompletionMode(QCompleter::PopupCompletion);
        }

        // Czyszczenie sceny zdjęć
        ui->graphicsView->setScene(nullptr);
        m_selectedPhotoIndex = -1;
        m_photoBuffer.clear();
    }
    // Ustaw domyślne "brak" dla statusu i miejsca przechowywania
    auto setDefaultIfAvailable = [](QComboBox *comboBox, const QString &targetName) {
        int index = comboBox->findText(targetName, Qt::MatchFixedString | Qt::MatchCaseSensitive);
        if (index != -1) {
            comboBox->setCurrentIndex(index);
        }
    };

    setDefaultIfAvailable(ui->New_item_status, "brak");
    setDefaultIfAvailable(ui->New_item_storagePlace, "brak");
}

/**
 * @brief Ustawia tryb klonowania rekordu.
 * @param recordId ID rekordu do sklonowania.
 *
 * @section MethodOverview
 * Wypełnia formularz danymi istniejącego rekordu, przygotowując go do zapisu jako nowy rekord,
 * z wyczyszczeniem zdjęć, ID rekordu i buforów zdjęć.
 */
void MainWindow::setCloneMode(const QString &recordId)
{
    m_editMode = false;
    loadRecord(recordId);
    m_recordId.clear();
    ui->graphicsView->setScene(nullptr);
    m_selectedPhotoIndex = -1;
    m_photoBuffer.clear();
    m_photoPathsBuffer.clear();
}

/**
 * @brief Wczytuje rekord o podanym ID do formularza.
 * @param recordId ID rekordu do wczytania.
 *
 * @section MethodOverview
 * Pobiera dane rekordu z tabeli eksponaty, wypełnia pola formularza (nazwa, numer seryjny,
 * rok produkcji, itd.) i ładuje powiązane zdjęcia. Wyświetla ostrzeżenie, jeśli rekord nie istnieje.
 */
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
        QMessageBox::warning(this, tr("Błąd"), tr("Nie znaleziono rekordu o ID %1").arg(recordId));
        return;
    }

    ui->New_item_name->setText(query.value("name").toString());
    ui->New_item_serialNumber->setText(query.value("serial_number").toString());
    ui->New_item_partNumber->setText(query.value("part_number").toString());
    ui->New_item_revision->setText(query.value("revision").toString());
    ui->New_item_value->setText(query.value("value").toString());
    ui->New_item_description->setPlainText(query.value("description").toString());

    int prodYear = query.value("production_year").toInt();
    ui->New_item_ProductionDate->setDate(QDate(prodYear, 1, 1));

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

    loadPhotos(recordId);
}

/**
 * @brief Wczytuje zdjęcia powiązane z danym rekordem.
 * @param recordId ID rekordu, którego zdjęcia są wczytywane.
 *
 * @section MethodOverview
 * Pobiera zdjęcia z tabeli photos, tworzy miniatury o rozmiarze 80x80 pikseli
 * i wyświetla je w QGraphicsView, z możliwością wyboru zdjęcia.
 */
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

    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbSize = 80, spacing = 5;
    int x = 5, y = 5, idx = 0;

    while (query.next()) {
        QByteArray imgData = query.value("photo").toByteArray();
        QPixmap pix;
        if (!pix.loadFromData(imgData)) {
            qDebug() << "Nie można załadować BLOB zdjęcia";
            continue;
        }
        QPixmap scaled = pix.scaled(thumbSize,
                                    thumbSize,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

        PhotoItem *item = new PhotoItem();
        item->setPixmap(scaled);
        item->setData(0, query.value("id").toString());
        item->setData(1, idx);

        connect(item, &PhotoItem::clicked, this, [this, item]() { onPhotoClicked(item); });

        item->setPos(x, y);
        scene->addItem(item);

        x += (scaled.width() + spacing);
        if (x + thumbSize > ui->graphicsView->width() - 10) {
            x = 5;
            y += (scaled.height() + spacing);
        }
        idx++;
    }

    if (!scene->items().isEmpty()) {
        scene->setSceneRect(0, 0, ui->graphicsView->width() - 10, y + thumbSize + 5);
        ui->graphicsView->setScene(scene);
        ui->graphicsView->resetTransform();

        qreal scaleFactor = qMin((ui->graphicsView->width() - 10.0) / scene->width(),
                                 (ui->graphicsView->height() - 10.0) / scene->height());
        if (scaleFactor < 1.0)
            scaleFactor = 1.0;
        ui->graphicsView->scale(scaleFactor, scaleFactor);
    } else {
        ui->graphicsView->setScene(nullptr);
        delete scene;
    }
}

/**
 * @brief Wczytuje zdjęcia z bufora pamięci.
 *
 * @section MethodOverview
 * Wyświetla miniatury zdjęć przechowywanych w buforze (dla rekordów jeszcze niezapisanych)
 * w QGraphicsView, z rozmiarem miniatur 80x80 pikseli.
 */
void MainWindow::loadPhotosFromBuffer()
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbSize = 80, spacing = 5;
    int x = 5, y = 5;

    for (int i = 0; i < m_photoBuffer.size(); i++) {
        QPixmap pix;
        if (!pix.loadFromData(m_photoBuffer[i])) {
            qDebug() << "Błąd loadFromData w buforze zdjęć";
            continue;
        }
        QPixmap scaled = pix.scaled(thumbSize,
                                    thumbSize,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
        QGraphicsPixmapItem *it = scene->addPixmap(scaled);
        it->setPos(x, y);
        x += (scaled.width() + spacing);
        if (x + thumbSize > ui->graphicsView->width() - 10) {
            x = 5;
            y += (scaled.height() + spacing);
        }
    }
    ui->graphicsView->setScene(scene);
}

/**
 * @brief Zapisuje dane z formularza do bazy danych.
 *
 * @section MethodOverview
 * Waliduje dane, zapisuje rekord w tabeli eksponaty (INSERT dla nowych, UPDATE dla edycji),
 * przenosi zdjęcia z bufora do bazy danych i katalogu "gotowe", emituje sygnał recordSaved
 * i zamyka okno po sukcesie.
 */
void MainWindow::onSaveClicked()
{
    QSqlQuery q(db);

    if (!m_editMode) {
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
    } else {
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

    QString statusId = ui->New_item_status->currentData().toString();
    if (statusId.isEmpty())
        statusId = "unknown_status_uuid";

    QString typeId = ui->New_item_type->currentData().toString();
    if (typeId.isEmpty())
        typeId = "unknown_type_uuid";

    QString vendorId = ui->New_item_vendor->currentData().toString();
    if (vendorId.isEmpty())
        vendorId = "unknown_vendor_uuid";

    QString modelId = ui->New_item_model->currentData().toString();
    if (modelId.isEmpty())
        modelId = "unknown_model_uuid";

    QString storageId = ui->New_item_storagePlace->currentData().toString();
    if (storageId.isEmpty())
        storageId = "unknown_storage_uuid";

    q.bindValue(":status_id", statusId);
    q.bindValue(":type_id", typeId);
    q.bindValue(":vendor_id", vendorId);
    q.bindValue(":model_id", modelId);
    q.bindValue(":storage_place_id", storageId);

    q.bindValue(":description", ui->New_item_description->toPlainText());
    q.bindValue(":value",
                ui->New_item_value->text().isEmpty() ? 0 : ui->New_item_value->text().toInt());

    if (!q.exec()) {
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Nie udało się zapisać:\n%1").arg(q.lastError().text()));
        return;
    }

    if (!m_editMode) {
        for (int i = 0; i < m_photoBuffer.size(); ++i) {
            QByteArray ba = m_photoBuffer[i];
            QString pid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QSqlQuery q2(db);
            q2.prepare(R"(
                INSERT INTO photos (id, eksponat_id, photo)
                VALUES (:id, :exid, :photo)
            )");
            q2.bindValue(":id", pid);
            q2.bindValue(":exid", m_recordId);
            q2.bindValue(":photo", ba);
            if (q2.exec()) {
                const QString orig = m_photoPathsBuffer.at(i);
                QFileInfo fi(orig);
                QString doneDir = fi.absolutePath() + QDir::separator() + QStringLiteral("gotowe");
                QDir().mkpath(doneDir);
                QString dst = doneDir + QDir::separator() + fi.fileName();
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

/**
 * @brief Anuluje edycję/dodawanie i zamyka okno.
 *
 * @section MethodOverview
 * Zamyka okno formularza bez zapisywania jakichkolwiek zmian.
 */
void MainWindow::onCancelClicked()
{
    close();
}

/**
 * @brief Dodaje nowe zdjęcia do rekordu.
 *
 * @section MethodOverview
 * Otwiera okno wyboru plików, ładuje zdjęcia do bufora (dla nowych rekordów) lub bazy danych
 * (dla edycji), przenosi pliki do katalogu "gotowe" i odświeża podgląd miniaturek w QGraphicsView.
 */
void MainWindow::onAddPhotoClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Wybierz zdjęcia"),
                                                      QString(),
                                                      tr("Images (*.jpg *.jpeg *.png)"));
    if (files.isEmpty())
        return;

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
            QString photoId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QSqlQuery q(db);
            q.prepare(R"(
                INSERT INTO photos (id, eksponat_id, photo)
                VALUES (:id, :exid, :photo)
            )");
            q.bindValue(":id", photoId);
            q.bindValue(":exid", m_recordId);
            q.bindValue(":photo", data);
            if (q.exec()) {
                QFileInfo fi(fn);
                QString doneDir = fi.absolutePath() + QDir::separator() + QStringLiteral("gotowe");
                if (!QDir().mkpath(doneDir)) {
                    QMessageBox::warning(this,
                                         tr("Uwaga"),
                                         tr("Nie udało się utworzyć katalogu:\n%1").arg(doneDir));
                }
                QString dst = doneDir + QDir::separator() + fi.fileName();
                if (!QFile::rename(fn, dst)) {
                    QMessageBox::warning(this,
                                         tr("Uwaga"),
                                         tr("Nie można przenieść %1 do %2").arg(fn, dst));
                }
            } else {
                QMessageBox::critical(this,
                                      tr("Błąd"),
                                      tr("Nie można zapisać zdjęcia:\n%1").arg(q.lastError().text()));
            }
        }
    }

    if (m_recordId.isEmpty())
        loadPhotosFromBuffer();
    else
        loadPhotos(m_recordId);
    this->raise();
    this->activateWindow();
}

/**
 * @brief Usuwa wybrane zdjęcie z rekordu.
 *
 * @section MethodOverview
 * Usuwa zdjęcie z bazy danych po potwierdzeniu użytkownika, wykonując zapytanie DELETE
 * i odświeżając podgląd miniaturek w QGraphicsView.
 */
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
    QList<QGraphicsItem *> items = scene->items();
    if (m_selectedPhotoIndex < 0 || m_selectedPhotoIndex >= items.size()) {
        return;
    }
    PhotoItem *selItem = dynamic_cast<PhotoItem *>(items[m_selectedPhotoIndex]);
    if (!selItem)
        return;

    QString photoId = selItem->data(0).toString();
    auto ans = QMessageBox::question(this,
                                     tr("Potwierdzenie"),
                                     tr("Czy usunąć zdjęcie?"),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::Yes) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM photos WHERE id=:id");
        q.bindValue(":id", photoId);
        if (!q.exec()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie można usunąć zdjęcia:\n%1").arg(q.lastError().text()));
        } else {
            loadPhotos(m_recordId);
        }
    }
    m_selectedPhotoIndex = -1;
}

/**
 * @brief Obsługuje kliknięcie w miniaturę zdjęcia.
 * @param item Wskaźnik na obiekt PhotoItem reprezentujący miniaturę.
 *
 * @section MethodOverview
 * Ustawia wybraną miniaturę jako aktywną, aktualizuje indeks wybranego zdjęcia
 * i odznacza pozostałe miniatury w QGraphicsView.
 */
void MainWindow::onPhotoClicked(PhotoItem *item)
{
    QGraphicsScene *scene = ui->graphicsView->scene();
    if (!scene)
        return;

    QList<QGraphicsItem *> its = scene->items();
    for (int i = 0; i < its.size(); i++) {
        PhotoItem *pit = dynamic_cast<PhotoItem *>(its[i]);
        if (pit) {
            pit->setSelected(its[i] == item);
            if (its[i] == item) {
                m_selectedPhotoIndex = i;
            }
        }
    }
    qDebug() << "Wybrane zdjęcie o indeksie:" << m_selectedPhotoIndex;
}

/**
 * @brief Otwiera okno dodawania nowego typu.
 *
 * @section MethodOverview
 * Wyświetla okno dialogowe (klasa types) do dodania nowego typu eksponatu
 * do tabeli types, ustawia referencję do MainWindow i aktywuje okno główne po zamknięciu.
 */
void MainWindow::onAddTypeClicked()
{
    types dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    this->raise();
    this->activateWindow();
}

/**
 * @brief Otwiera okno dodawania nowego producenta.
 *
 * @section MethodOverview
 * Wyświetla okno dialogowe (klasa vendors) do dodania nowego producenta
 * do tabeli vendors, ustawia referencję do MainWindow i aktywuje okno główne po zamknięciu.
 */
void MainWindow::onAddVendorClicked()
{
    vendors dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    this->raise();
    this->activateWindow();
}

/**
 * @brief Otwiera okno dodawania nowego modelu.
 *
 * @section MethodOverview
 * Wyświetla okno dialogowe (klasa models) do dodania nowego modelu
 * do tabeli models, ustawia referencję do MainWindow i aktywuje okno główne po zamknięciu.
 */
void MainWindow::onAddModelClicked()
{
    models dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    this->raise();
    this->activateWindow();
}

/**
 * @brief Otwiera okno dodawania nowego statusu.
 *
 * @section MethodOverview
 * Wyświetla okno dialogowe (klasa status) do dodania nowego statusu
 * do tabeli statuses, ustawia referencję do MainWindow i aktywuje okno główne po zamknięciu.
 */
void MainWindow::onAddStatusClicked()
{
    status dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    this->raise();
    this->activateWindow();
}

/**
 * @brief Otwiera okno dodawania nowego miejsca przechowywania.
 *
 * @section MethodOverview
 * Wyświetla okno dialogowe (klasa storage) do dodania nowego miejsca przechowywania
 * do tabeli storage_places, ustawia referencję do MainWindow i aktywuje okno główne po zamknięciu.
 */
void MainWindow::onAddStoragePlaceClicked()
{
    storage dlg(this);
    dlg.setMainWindow(this);
    dlg.exec();
    this->raise();
    this->activateWindow();
}
