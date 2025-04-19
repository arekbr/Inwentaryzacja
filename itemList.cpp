#include "itemList.h"
#include "ItemFilterProxyModel.h"
#include "fullscreenphotoviewer.h"
#include "mainwindow.h"
#include "photoitem.h"
#include "ui_itemList.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QSqlRelationalTableModel>
#include <QUuid>
#include <QtMath>
#include <functional>

//=============================================================================
// Konstruktor i Destruktor
//=============================================================================

itemList::itemList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::itemList)
    , m_sourceModel(nullptr)
    , m_proxyModel(nullptr)
    , filterTypeComboBox(nullptr)
    , filterVendorComboBox(nullptr)
    , filterModelComboBox(nullptr)
    , filterStatusComboBox(nullptr)
    , filterStorageComboBox(nullptr)
    , m_currentRecordId()
    , m_previewWindow(nullptr)
{
    ui->setupUi(this);

    // Inicjalizacja pól QComboBox
    filterTypeComboBox = ui->filterTypeComboBox;
    filterVendorComboBox = ui->filterVendorComboBox;
    filterModelComboBox = ui->filterModelComboBox;
    filterStatusComboBox = ui->filterStatusComboBox;
    filterStorageComboBox = ui->filterStorageComboBox;

    // Połączenie z bazą danych
    QSqlDatabase db;
    if (!QSqlDatabase::contains("default_connection")) {
        db = QSqlDatabase::addDatabase("QMYSQL", "default_connection");
        db.setHostName("localhost"); // Zastąp swoim adresem serwera
        db.setDatabaseName("retro_komputery");
        db.setUserName("user");
        db.setPassword("password");
        if (!db.open()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie można połączyć z bazą danych:\n%1")
                                      .arg(db.lastError().text()));
            qApp->quit();
            return;
        }
        if (!verifyDatabaseSchema(db)) {
            createDatabaseSchema(db);
            insertSampleData(db);
        }
    } else {
        db = QSqlDatabase::database("default_connection");
        if (!db.isOpen()) {
            QMessageBox::critical(this, tr("Błąd"), tr("Połączenie z bazą danych zamknięte."));
            qApp->quit();
            return;
        }
    }

    // Model źródłowy
    m_sourceModel = new QSqlRelationalTableModel(this, db);
    m_sourceModel->setTable("eksponaty");
    m_sourceModel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    m_sourceModel->setRelation(2, QSqlRelation("types", "id", "name"));
    m_sourceModel->setRelation(3, QSqlRelation("vendors", "id", "name"));
    m_sourceModel->setRelation(4, QSqlRelation("models", "id", "name"));
    m_sourceModel->setRelation(9, QSqlRelation("statuses", "id", "name"));
    m_sourceModel->setRelation(10, QSqlRelation("storage_places", "id", "name"));
    m_sourceModel->select();

    // Model proxy
    m_proxyModel = new ItemFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_sourceModel);

    // Konfiguracja widoku tabeli
    ui->itemList_tableView->setModel(m_proxyModel);
    ui->itemList_tableView->setItemDelegate(new QSqlRelationalDelegate(ui->itemList_tableView));
    ui->itemList_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemList_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemList_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->itemList_tableView->hideColumn(0); // Ukryj kolumnę UUID
    ui->itemList_tableView->resizeColumnsToContents();

    // Połączenia przycisków
    connect(ui->itemList_pushButton_new, &QPushButton::clicked, this, &itemList::onNewButtonClicked);
    connect(ui->itemList_pushButton_edit,
            &QPushButton::clicked,
            this,
            &itemList::onEditButtonClicked);
    connect(ui->itemList_pushButton_clone,
            &QPushButton::clicked,
            this,
            &itemList::onCloneButtonClicked);
    connect(ui->itemList_pushButton_delete,
            &QPushButton::clicked,
            this,
            &itemList::onDeleteButtonClicked);
    connect(ui->itemList_pushButton_end, &QPushButton::clicked, this, &itemList::onEndButtonClicked);
    connect(ui->itemList_pushButton_about, &QPushButton::clicked, this, &itemList::onAboutClicked);

    connect(ui->itemList_tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &itemList::onTableViewSelectionChanged);

    // Inicjalizacja filtrów
    initFilters(db);
}

itemList::~itemList()
{
    delete ui;
}

//=============================================================================
// Inicjalizacja filtrów
//=============================================================================

void itemList::initFilters(QSqlDatabase &db)
{
    auto initFilter =
        [&](QComboBox *cb, const QString &table, std::function<void(const QString &)> setter) {
            cb->blockSignals(true);
            cb->clear();
            cb->addItem(tr("Wszystkie"));
            QSqlQuery q(db);
            q.exec(QString("SELECT name FROM %1 ORDER BY name").arg(table));
            while (q.next())
                cb->addItem(q.value(0).toString());
            cb->blockSignals(false);
            connect(cb, &QComboBox::currentTextChanged, this, [setter](const QString &txt) {
                setter(txt == tr("Wszystkie") ? QString() : txt);
            });
        };
    initFilter(filterTypeComboBox, "types", [this](const QString &v) {
        m_proxyModel->setTypeFilter(v);
    });
    initFilter(filterVendorComboBox, "vendors", [this](const QString &v) {
        m_proxyModel->setVendorFilter(v);
    });
    initFilter(filterModelComboBox, "models", [this](const QString &v) {
        m_proxyModel->setModelFilter(v);
    });
    initFilter(filterStatusComboBox, "statuses", [this](const QString &v) {
        m_proxyModel->setStatusFilter(v);
    });
    initFilter(filterStorageComboBox, "storage_places", [this](const QString &v) {
        m_proxyModel->setStorageFilter(v);
    });
}

//=============================================================================
// Odświeżanie filtrów
//=============================================================================

void itemList::refreshFilters()
{
    QSqlDatabase db = QSqlDatabase::database("default_connection");
    if (!db.isOpen()) {
        qDebug()
            << "itemList: Błąd - połączenie z bazą danych zamknięte podczas odświeżania filtrów.";
        return;
    }

    // Zachowaj aktualnie wybrane wartości filtrów
    QString currentType = filterTypeComboBox->currentText();
    QString currentVendor = filterVendorComboBox->currentText();
    QString currentModel = filterModelComboBox->currentText();
    QString currentStatus = filterStatusComboBox->currentText();
    QString currentStorage = filterStorageComboBox->currentText();

    // Odśwież filtry
    initFilters(db);

    // Przywróć wybrane wartości, jeśli nadal istnieją
    auto restoreFilter = [](QComboBox *cb, const QString &value) {
        if (!value.isEmpty() && value != tr("Wszystkie")) {
            int index = cb->findText(value);
            if (index != -1) {
                cb->setCurrentIndex(index);
            }
        }
    };
    restoreFilter(filterTypeComboBox, currentType);
    restoreFilter(filterVendorComboBox, currentVendor);
    restoreFilter(filterModelComboBox, currentModel);
    restoreFilter(filterStatusComboBox, currentStatus);
    restoreFilter(filterStorageComboBox, currentStorage);
}

//=============================================================================
// Obsługa zaznaczenia w tabeli
//=============================================================================

void itemList::onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    if (selected.indexes().isEmpty()) {
        ui->itemList_graphicsView->setScene(nullptr);
        m_currentRecordId.clear();
        return;
    }

    QModelIndex proxyIndex = selected.indexes().first();
    QModelIndex srcIndex = m_proxyModel->mapToSource(proxyIndex);
    m_currentRecordId = m_sourceModel->data(m_sourceModel->index(srcIndex.row(), 0)).toString();

    // Wczytaj zdjęcia
    QSqlQuery query(QSqlDatabase::database("default_connection"));
    query.prepare("SELECT photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", m_currentRecordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęć (MySQL):" << query.lastError().text();
        ui->itemList_graphicsView->setScene(nullptr);
        return;
    }

    int viewWidth = ui->itemList_graphicsView->viewport()->width() - 10;
    int viewHeight = ui->itemList_graphicsView->viewport()->height() - 10;

    QList<QPixmap> pixmaps;
    while (query.next()) {
        QByteArray imageData = query.value(0).toByteArray();
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            pixmaps.append(pixmap);
        } else {
            qDebug() << "Nie można załadować BLOB (MySQL).";
        }
    }

    if (pixmaps.isEmpty()) {
        ui->itemList_graphicsView->setScene(nullptr);
        return;
    }

    QGraphicsScene *scene = new QGraphicsScene(this);
    const int spacing = 5;
    int photoCount = pixmaps.size();

    int cols = qMax(1, qMin(qCeil(qSqrt(photoCount)), viewWidth / 100));
    int rows = (photoCount + cols - 1) / cols;
    int maxThumbnailWidth = (viewWidth - (cols - 1) * spacing) / cols;
    int maxThumbnailHeight = (viewHeight - (rows - 1) * spacing) / rows;

    int x = 5, y = 5;
    for (int i = 0; i < photoCount; i++) {
        QPixmap original = pixmaps[i];
        QPixmap scaled = original.scaled(maxThumbnailWidth,
                                         maxThumbnailHeight,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
        PhotoItem *item = new PhotoItem();
        item->setPixmap(scaled);
        item->setData(0, QVariant(original));
        item->setPos(x, y);
        scene->addItem(item);

        connect(item, &PhotoItem::hovered, this, &itemList::onPhotoHovered);
        connect(item, &PhotoItem::unhovered, this, &itemList::onPhotoUnhovered);
        connect(item, &PhotoItem::clicked, this, &itemList::onPhotoClicked);

        x += scaled.width() + spacing;
        if ((i + 1) % cols == 0) {
            x = 5;
            y += scaled.height() + spacing;
        }
    }

    int totalWidth = cols * maxThumbnailWidth + (cols - 1) * spacing + 10;
    int totalHeight = rows * maxThumbnailHeight + (rows - 1) * spacing + 10;
    scene->setSceneRect(0, 0, totalWidth, totalHeight);
    ui->itemList_graphicsView->setScene(scene);
    ui->itemList_graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

//=============================================================================
// Obsługa przycisków
//=============================================================================

void itemList::onNewButtonClicked()
{
    MainWindow *w = new MainWindow(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setEditMode(false, QString());
    connect(w, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    w->show();
}

void itemList::onEditButtonClicked()
{
    auto *sel = ui->itemList_tableView->selectionModel();
    if (!sel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do edycji."));
        return;
    }
    QModelIndex proxyIdx = sel->selectedRows().first();
    QModelIndex srcIdx = m_proxyModel->mapToSource(proxyIdx);
    QString id = m_sourceModel->data(m_sourceModel->index(srcIdx.row(), 0)).toString();

    MainWindow *w = new MainWindow(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setEditMode(true, id);
    connect(w, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    w->show();
}

void itemList::onCloneButtonClicked()
{
    auto *sel = ui->itemList_tableView->selectionModel();
    if (!sel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do klonowania."));
        return;
    }
    QModelIndex proxyIdx = sel->selectedRows().first();
    QModelIndex srcIdx = m_proxyModel->mapToSource(proxyIdx);
    QString id = m_sourceModel->data(m_sourceModel->index(srcIdx.row(), 0)).toString();

    MainWindow *w = new MainWindow(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setCloneMode(id);
    connect(w, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    w->show();
}

void itemList::onDeleteButtonClicked()
{
    auto *sel = ui->itemList_tableView->selectionModel();
    if (!sel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do usunięcia."));
        return;
    }
    QModelIndex proxyIdx = sel->selectedRows().first();
    QModelIndex srcIdx = m_proxyModel->mapToSource(proxyIdx);
    QString id = m_sourceModel->data(m_sourceModel->index(srcIdx.row(), 0)).toString();

    if (QMessageBox::question(this,
                              tr("Potwierdzenie"),
                              tr("Czy na pewno chcesz usunąć rekord %1?").arg(id),
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        QSqlQuery q(QSqlDatabase::database("default_connection"));
        q.prepare("DELETE FROM eksponaty WHERE id = :id");
        q.bindValue(":id", id);
        if (!q.exec()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie udało się usunąć:\n%1").arg(q.lastError().text()));
        } else {
            m_sourceModel->select();
            QMessageBox::information(this, tr("Sukces"), tr("Rekord usunięty."));
        }
    }
}

void itemList::onEndButtonClicked()
{
    qApp->quit();
}

void itemList::onAboutClicked()
{
    const QString html
        = tr("<h3>%1</h3>"
             "<p>%2</p>"
             "<p><b>Autor:</b> %3</p>"
             "<p><b>Wersja:</b> %4</p>")
              .arg(QCoreApplication::applicationName(),
                   QStringLiteral("Program do inwentaryzacji retro komputerów"),
                   QStringLiteral(
                       "Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK"),
                   QCoreApplication::applicationVersion());

    QMessageBox::about(this, tr("O programie"), html);
}

//=============================================================================
// Obsługa interakcji ze zdjęciami
//=============================================================================

void itemList::onPhotoHovered(PhotoItem *item)
{
    if (m_previewWindow) {
        m_previewWindow->close();
        m_previewWindow = nullptr;
    }

    QPixmap originalPixmap = item->data(0).value<QPixmap>();
    if (originalPixmap.isNull())
        return;

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    QRect screenGeometry = screen->availableGeometry();
    int screenCenterX = screenGeometry.center().x();
    int screenCenterY = screenGeometry.center().y();

    int maxWidth = screenGeometry.width() * 0.8;
    int maxHeight = screenGeometry.height() * 0.8;
    QPixmap scaled = originalPixmap.scaled(maxWidth,
                                           maxHeight,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

    m_previewWindow = new QWidget(nullptr,
                                  Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    m_previewWindow->setAttribute(Qt::WA_TranslucentBackground);
    m_previewWindow->setStyleSheet("background-color: rgba(0, 0, 0, 200); border-radius: 10px;");

    QLabel *imageLabel = new QLabel(m_previewWindow);
    imageLabel->setStyleSheet("background: transparent;");
    imageLabel->setPixmap(scaled);

    m_previewWindow->setFixedSize(scaled.width() + 20, scaled.height() + 20);
    imageLabel->setGeometry(10, 10, scaled.width(), scaled.height());

    int windowX = screenCenterX - (m_previewWindow->width() / 2);
    int windowY = screenCenterY - (m_previewWindow->height() / 2);
    m_previewWindow->move(windowX, windowY);

    m_previewWindow->raise();
    m_previewWindow->activateWindow();
    m_previewWindow->show();
}

void itemList::onPhotoUnhovered(PhotoItem *item)
{
    Q_UNUSED(item);
    if (m_previewWindow) {
        m_previewWindow->close();
        m_previewWindow = nullptr;
    }
}

void itemList::onPhotoClicked(PhotoItem *item)
{
    QPixmap original = item->data(0).value<QPixmap>();
    if (original.isNull())
        return;

    FullScreenPhotoViewer *viewer = new FullScreenPhotoViewer(original, this);
    viewer->show();
}

//=============================================================================
// Zapis rekordu i odświeżanie
//=============================================================================

void itemList::onRecordSaved(const QString &recordId)
{
    refreshList(recordId);
}

void itemList::refreshList(const QString &recordId)
{
    m_sourceModel->select();
    ui->itemList_tableView->resizeColumnsToContents();

    // Odśwież filtry
    refreshFilters();

    if (!recordId.isEmpty()) {
        for (int row = 0; row < m_sourceModel->rowCount(); ++row) {
            QModelIndex srcIdx = m_sourceModel->index(row, 0);
            if (m_sourceModel->data(srcIdx).toString() == recordId) {
                QModelIndex proxyIdx = m_proxyModel->mapFromSource(srcIdx);
                ui->itemList_tableView->selectionModel()->select(proxyIdx,
                                                                 QItemSelectionModel::ClearAndSelect
                                                                     | QItemSelectionModel::Rows);
                m_currentRecordId = recordId;
                break;
            }
        }
    }
}

//=============================================================================
// Schemat bazy danych i dane przykładowe
//=============================================================================

bool itemList::verifyDatabaseSchema(QSqlDatabase &db)
{
    QStringList tables
        = {"eksponaty", "types", "vendors", "models", "statuses", "storage_places", "photos"};
    for (const QString &t : tables) {
        if (!db.tables().contains(t, Qt::CaseInsensitive))
            return false;
    }
    return true;
}

void itemList::createDatabaseSchema(QSqlDatabase &db)
{
    qDebug() << "itemList: Tworzenie schematu (MySQL)";
    QSqlQuery query(db);

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS eksponaty (
            id VARCHAR(36) PRIMARY KEY,
            name TEXT NOT NULL,
            type_id VARCHAR(36) NOT NULL,
            vendor_id VARCHAR(36) NOT NULL,
            model_id VARCHAR(36) NOT NULL,
            serial_number TEXT,
            part_number TEXT,
            revision TEXT,
            production_year INT,
            status_id VARCHAR(36) NOT NULL,
            storage_place_id VARCHAR(36) NOT NULL,
            description TEXT,
            value INT
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS types (
            id VARCHAR(36) PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS vendors (
            id VARCHAR(36) PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS models (
            id VARCHAR(36) PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            vendor_id VARCHAR(36) NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS statuses (
            id VARCHAR(36) PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS storage_places (
            id VARCHAR(36) PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS photos (
            id VARCHAR(36) PRIMARY KEY,
            eksponat_id VARCHAR(36) NOT NULL,
            photo LONGBLOB NOT NULL
        )
    )");

    if (query.lastError().isValid()) {
        qDebug() << "itemList: Błąd tworzenia schematu:" << query.lastError().text();
    } else {
        qDebug() << "itemList: Schemat utworzony pomyślnie (MySQL).";
    }
}

void itemList::insertSampleData(QSqlDatabase &db)
{
    qDebug() << "itemList: Wstawianie danych przykładowych (MySQL)";
    QSqlQuery query(db);

    auto genId = []() { return QUuid::createUuid().toString(QUuid::WithoutBraces); };

    // Typy
    QString t1 = genId(), t2 = genId(), t3 = genId();
    query.exec(QString("INSERT IGNORE INTO types (id, name) VALUES ('%1','Komputer')").arg(t1));
    query.exec(QString("INSERT IGNORE INTO types (id, name) VALUES ('%1','Monitor')").arg(t2));
    query.exec(QString("INSERT IGNORE INTO types (id, name) VALUES ('%1','Kabel')").arg(t3));

    // Producenci
    QString v1 = genId(), v2 = genId(), v3 = genId();
    query.exec(QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Atari')").arg(v1));
    query.exec(QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Commodore')").arg(v2));
    query.exec(QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Sinclair')").arg(v3));

    // Modele
    QString m1 = genId(), m2 = genId(), m3 = genId();
    query.exec(
        QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','Atari 800XL','%2')")
            .arg(m1, v1));
    query.exec(
        QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','Amiga 500','%2')")
            .arg(m2, v2));
    query.exec(
        QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','ZX Spectrum','%2')")
            .arg(m3, v3));

    // Statusy
    QString s1 = genId(), s2 = genId(), s3 = genId();
    query.exec(QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','Sprawny')").arg(s1));
    query.exec(QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','Uszkodzony')").arg(s2));
    query.exec(QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','W naprawie')").arg(s3));

    // Miejsce przechowywania
    QString sp1 = genId(), sp2 = genId();
    query.exec(
        QString("INSERT IGNORE INTO storage_places (id, name) VALUES ('%1','Magazyn 1')").arg(sp1));
    query.exec(
        QString("INSERT IGNORE INTO storage_places (id, name) VALUES ('%1','Półka B3')").arg(sp2));

    // Eksponaty
    QString e1 = genId();
    query.prepare(R"(
        INSERT IGNORE INTO eksponaty
        (id, name, type_id, vendor_id, model_id, serial_number, production_year,
         status_id, storage_place_id, description, value)
        VALUES
        (:id, :name, :type_id, :vendor_id, :model_id, :serial, :year,
         :status, :storage, :desc, :val)
    )");
    query.bindValue(":id", e1);
    query.bindValue(":name", "Atari 800XL");
    query.bindValue(":type_id", t1);
    query.bindValue(":vendor_id", v1);
    query.bindValue(":model_id", m1);
    query.bindValue(":serial", "AT800-001");
    query.bindValue(":year", 1983);
    query.bindValue(":status", s1);
    query.bindValue(":storage", sp1);
    query.bindValue(":desc", "Klasyczny komputer Atari");
    query.bindValue(":val", 1);
    query.exec();

    QString e2 = genId();
    query.prepare(R"(
        INSERT IGNORE INTO eksponaty
        (id, name, type_id, vendor_id, model_id, serial_number, production_year,
         status_id, storage_place_id, description, value)
        VALUES
        (:id, :name, :type_id, :vendor_id, :model_id, :serial, :year,
         :status, :storage, :desc, :val)
    )");
    query.bindValue(":id", e2);
    query.bindValue(":name", "Amiga 500");
    query.bindValue(":type_id", t1);
    query.bindValue(":vendor_id", v2);
    query.bindValue(":model_id", m2);
    query.bindValue(":serial", "A500-1234");
    query.bindValue(":year", 1987);
    query.bindValue(":status", s1);
    query.bindValue(":storage", sp1);
    query.bindValue(":desc", "Klasyczny komputer Amiga");
    query.bindValue(":val", 2);
    query.exec();

    QString e3 = genId();
    query.prepare(R"(
        INSERT IGNORE INTO eksponaty
        (id, name, type_id, vendor_id, model_id, serial_number, production_year,
         status_id, storage_place_id, description, value)
        VALUES
        (:id, :name, :type_id, :vendor_id, :model_id, :serial, :year,
         :status, :storage, :desc, :val)
    )");
    query.bindValue(":id", e3);
    query.bindValue(":name", "ZX Spectrum");
    query.bindValue(":type_id", t1);
    query.bindValue(":vendor_id", v3);
    query.bindValue(":model_id", m3);
    query.bindValue(":serial", "ZXS-4567");
    query.bindValue(":year", 1982);
    query.bindValue(":status", s2);
    query.bindValue(":storage", sp2);
    query.bindValue(":desc", "Kultowy komputer Sinclair");
    query.bindValue(":val", 1);
    query.exec();

    if (query.lastError().isValid()) {
        qDebug() << "itemList: Błąd wstawiania danych przykładowych (MySQL)"
                 << query.lastError().text();
    } else {
        qDebug() << "itemList: Wstawiono dane przykładowe (MySQL)";
    }
}
