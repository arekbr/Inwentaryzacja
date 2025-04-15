#include "itemList.h"
#include "ui_itemList.h"
#include "mainwindow.h"
#include "photoitem.h"
#include "fullscreenphotoviewer.h"

#include <QSqlDatabase>
#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QItemSelectionModel>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QFile>
#include <QtMath>
#include <QUuid>

/*bool ensureDatabaseExists(const QString &dbName, const QString &host, int port, const QString &user, const QString &pass)
{
    const QString connectionName = "mysql_admin_check";

    if (QSqlDatabase::contains(connectionName))
        QSqlDatabase::removeDatabase(connectionName);

    QSqlDatabase adminDb = QSqlDatabase::addDatabase("QMYSQL", connectionName);
    adminDb.setHostName(host);
    adminDb.setPort(port);
    adminDb.setUserName(user);
    adminDb.setPassword(pass);
    adminDb.setDatabaseName("mysql"); // systemowa baza zawsze istnieje
    qDebug() << adminDb.drivers();



    if (!adminDb.open()) {
        qDebug() << "[MySQL] Błąd połączenia (admin):" << adminDb.lastError().text();
        return false;
    }

    QSqlQuery q(adminDb);
    if (!q.exec(QString("CREATE DATABASE IF NOT EXISTS `%1`").arg(dbName))) {
        qDebug() << "[MySQL] Błąd CREATE DATABASE:" << q.lastError().text();
        return false;
    }

    adminDb.close();
    QSqlDatabase::removeDatabase(connectionName);
    return true;
}
*/
itemList::itemList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::itemList),
    m_currentRecordId(QString()),
    m_previewWindow(nullptr)
{
  //  const QString mysqlHost = "127.0.0.1";
  //  const int mysqlPort = 3306;
  //  const QString mysqlUser = "root";
  //  const QString mysqlPass = "123456";
  //  const QString mysqlDb   = "inwentaryzacja";

    // Upewniamy się, że baza istnieje
  //  if (!ensureDatabaseExists(mysqlDb, mysqlHost, mysqlPort, mysqlUser, mysqlPass)) {
  //      QMessageBox::critical(this, tr("Błąd krytyczny"), tr("Nie można utworzyć bazy danych '%1'.").arg(mysqlDb));
  //      qApp->quit();
  //      return;
  //  }
    ui->setupUi(this);
    qDebug() << "itemList: Konstruktor start (MySQL)";

    // Tworzymy połączenie do MySQL, jeśli nie istnieje
    QSqlDatabase db;
    if (!QSqlDatabase::contains("default_connection")) {
        qDebug() << "itemList: Tworzenie połączenia do MySQL";
        qDebug() << db.drivers();


        db = QSqlDatabase::addDatabase("QMYSQL", "default_connection");

        // Tu wpisz swoje parametry
        db.setHostName("IP SERWERA");       // Adres serwera MySQL
        db.setDatabaseName("nazwa bazy");  // Nazwa bazy
        db.setUserName("nazwa usera");             // Użytkownik
        db.setPassword("haslo");         // Hasło
        // db.setPort(3306); // domyślnie

        if (!db.open()) {
            qDebug() << "itemList: Błąd połączenia z MySQL:" << db.lastError().text();
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie można połączyć z MySQL:\n%1")
                                      .arg(db.lastError().text()));
            qApp->quit();
            return;
        } else {
            qDebug() << "itemList: Połączenie z MySQL nawiązane.";
        }

        // Sprawdźmy, czy tabele istnieją
        if (!verifyDatabaseSchema(db)) {
            qDebug() << "itemList: Tabele nie istnieją lub schemat nieprawidłowy -> tworzymy";
            createDatabaseSchema(db);
            insertSampleData(db);
        }

    } else {
        // Korzystamy z już istniejącego "default_connection"
        db = QSqlDatabase::database("default_connection");
        if (!db.isOpen()) {
            qDebug() << "itemList: Błąd - MySQL nie jest otwarte";
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Połączenie z MySQL jest zamknięte."));
            qApp->quit();
            return;
        }
    }

    // Tworzymy model relacyjny
    QSqlRelationalTableModel *relModel = new QSqlRelationalTableModel(this, db);
    relModel->setTable("eksponaty");
    relModel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);

    // Ustawiamy relacje (klucze obce)
    relModel->setRelation(2,  QSqlRelation("types",           "id", "name"));
    relModel->setRelation(3,  QSqlRelation("vendors",         "id", "name"));
    relModel->setRelation(4,  QSqlRelation("models",          "id", "name"));
    relModel->setRelation(9,  QSqlRelation("statuses",        "id", "name"));
    relModel->setRelation(10, QSqlRelation("storage_places",  "id", "name"));

    if (!relModel->select()) {
        qDebug() << "itemList: Błąd select() w MySQL:" << relModel->lastError().text();
    }

    // Nagłówki
    relModel->setHeaderData(1,  Qt::Horizontal, tr("Nazwa"));
    relModel->setHeaderData(2,  Qt::Horizontal, tr("Typ"));
    relModel->setHeaderData(3,  Qt::Horizontal, tr("Producent"));
    relModel->setHeaderData(4,  Qt::Horizontal, tr("Model"));
    relModel->setHeaderData(5,  Qt::Horizontal, tr("Numer seryjny"));
    relModel->setHeaderData(6,  Qt::Horizontal, tr("Part number"));
    relModel->setHeaderData(7,  Qt::Horizontal, tr("Revision"));
    relModel->setHeaderData(8,  Qt::Horizontal, tr("Rok produkcji"));
    relModel->setHeaderData(9,  Qt::Horizontal, tr("Status"));
    relModel->setHeaderData(10, Qt::Horizontal, tr("Miejsce przechowywania"));
    relModel->setHeaderData(11, Qt::Horizontal, tr("Opis"));
    relModel->setHeaderData(12, Qt::Horizontal, tr("Ilość"));

    model = relModel;
    ui->itemList_tableView->setItemDelegate(new QSqlRelationalDelegate(ui->itemList_tableView));
    ui->itemList_tableView->setModel(model);

    ui->itemList_tableView->resizeColumnsToContents();
    ui->itemList_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemList_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemList_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ukryj kolumnę 0 (id)
    ui->itemList_tableView->hideColumn(0);

    // Podpinki przycisków
    connect(ui->itemList_pushButton_new,    &QPushButton::clicked, this, &itemList::onNewButtonClicked);
    connect(ui->itemList_pushButton_edit,   &QPushButton::clicked, this, &itemList::onEditButtonClicked);
    connect(ui->itemList_pushButton_end,    &QPushButton::clicked, this, &itemList::onEndButtonClicked);
    connect(ui->itemList_pushButton_delete, &QPushButton::clicked, this, &itemList::onDeleteButtonClicked);
    connect(ui->itemList_pushButton_clone,  &QPushButton::clicked, this, &itemList::onCloneButtonClicked);

    // Reakcja na zaznaczenie wiersza
    QItemSelectionModel *selModel = ui->itemList_tableView->selectionModel();
    connect(selModel, &QItemSelectionModel::selectionChanged,
            this, &itemList::onTableViewSelectionChanged);

    qDebug() << "itemList: Konstruktor done (MySQL).";
}

itemList::~itemList()
{
    delete m_previewWindow;
    delete ui;
    qDebug() << "itemList: Destruktor";
}

// ----------------------------------------------------------------------------
// verifyDatabaseSchema
// ----------------------------------------------------------------------------
bool itemList::verifyDatabaseSchema(QSqlDatabase &db)
{
    qDebug() << "itemList: verifyDatabaseSchema (MySQL)";
    // sprawdź, czy wszystkie tabele tam są:
    QStringList required = {
        "eksponaty", "types", "vendors", "models",
        "statuses", "storage_places", "photos"
    };

    // QSqlDatabase::tables() w MySQL
    QStringList tables = db.tables();
    for (const QString &t : required) {
        if (!tables.contains(t, Qt::CaseInsensitive)) {
            qDebug() << "Brak tabeli:" << t;
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// createDatabaseSchema
// ----------------------------------------------------------------------------
void itemList::createDatabaseSchema(QSqlDatabase &db)
{
    qDebug() << "itemList: Tworzenie schematu (MySQL)";
    QSqlQuery query(db);

    // W MySQL: tworzysz tabele, jeśli nie istnieją
    // Używamy VARCHAR(36) dla UUID, LONGBLOB dla zdjęć
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
        qDebug() << "itemList: createDatabaseSchema error:" << query.lastError().text();
    } else {
        qDebug() << "itemList: createDatabaseSchema OK (MySQL).";
    }
}

// ----------------------------------------------------------------------------
// insertSampleData
// ----------------------------------------------------------------------------
void itemList::insertSampleData(QSqlDatabase &db)
{
    qDebug() << "itemList: insertSampleData (MySQL)";
    QSqlQuery query(db);

    auto genId = [](){ return QUuid::createUuid().toString(QUuid::WithoutBraces); };

    // typy
    QString t1 = genId(), t2 = genId(), t3 = genId();
    query.exec(QString("INSERT IGNORE INTO types (id, name) VALUES ('%1','Komputer')").arg(t1));
    query.exec(QString("INSERT IGNORE INTO types (id, name) VALUES ('%1','Monitor')").arg(t2));
    query.exec(QString("INSERT IGNORE INTO types (id, name) VALUES ('%1','Kabel')").arg(t3));

    // vendors
    QString v1 = genId(), v2 = genId(), v3 = genId();
    query.exec(QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Atari')").arg(v1));
    query.exec(QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Commodore')").arg(v2));
    query.exec(QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Sinclair')").arg(v3));

    // models
    QString m1 = genId(), m2 = genId(), m3 = genId();
    query.exec(QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','Atari 800XL','%2')")
                   .arg(m1).arg(v1));
    query.exec(QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','Amiga 500','%2')")
                   .arg(m2).arg(v2));
    query.exec(QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','ZX Spectrum','%2')")
                   .arg(m3).arg(v3));

    // statuses
    QString s1 = genId(), s2 = genId(), s3 = genId();
    query.exec(QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','Sprawny')").arg(s1));
    query.exec(QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','Uszkodzony')").arg(s2));
    query.exec(QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','W naprawie')").arg(s3));

    // storage_places
    QString sp1 = genId(), sp2 = genId();
    query.exec(QString("INSERT IGNORE INTO storage_places (id, name) VALUES ('%1','Magazyn 1')").arg(sp1));
    query.exec(QString("INSERT IGNORE INTO storage_places (id, name) VALUES ('%1','Półka B3')").arg(sp2));

    // eksponaty
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
        qDebug() << "itemList: Błąd wstawiania danych (MySQL)" << query.lastError().text();
    } else {
        qDebug() << "itemList: Wstawiono dane przykładowe (MySQL)";
    }
}

// ----------------------------------------------------------------------------
// Reakcja na zaznaczenie wiersza -> wczytanie zdjęć
// ----------------------------------------------------------------------------
void itemList::onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    if (selected.indexes().isEmpty()) {
        ui->itemList_graphicsView->setScene(nullptr);
        m_currentRecordId.clear();
        return;
    }
    QModelIndex index = selected.indexes().first();
    int row = index.row();
    QString recordId = model->data(model->index(row, 0)).toString();
    m_currentRecordId = recordId;

    // Odczytujemy zdjęcia
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
        QPixmap scaled = original.scaled(
            maxThumbnailWidth, maxThumbnailHeight,
            Qt::KeepAspectRatio, Qt::SmoothTransformation
            );
        PhotoItem *item = new PhotoItem();
        item->setPixmap(scaled);
        item->setData(0, QVariant(original));
        item->setPos(x, y);
        scene->addItem(item);

        connect(item, &PhotoItem::hovered,   this, &itemList::onPhotoHovered);
        connect(item, &PhotoItem::unhovered, this, &itemList::onPhotoUnhovered);
        connect(item, &PhotoItem::clicked,   this, &itemList::onPhotoClicked);

        x += scaled.width() + spacing;
        if ((i + 1) % cols == 0) {
            x = 5;
            y += scaled.height() + spacing;
        }
    }

    int totalWidth  = cols * maxThumbnailWidth + (cols - 1) * spacing + 10;
    int totalHeight = rows * maxThumbnailHeight + (rows - 1) * spacing + 10;
    scene->setSceneRect(0, 0, totalWidth, totalHeight);
    ui->itemList_graphicsView->setScene(scene);
    ui->itemList_graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

// ----------------------------------------------------------------------------
// Hover podgląd mini
// ----------------------------------------------------------------------------
void itemList::onPhotoHovered(PhotoItem *item)
{
    if (m_previewWindow) {
        m_previewWindow->close();
        m_previewWindow = nullptr;
    }

    QPixmap originalPixmap = item->data(0).value<QPixmap>();
    if (originalPixmap.isNull()) return;

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect screenGeometry = screen->availableGeometry();
    int screenCenterX = screenGeometry.center().x();
    int screenCenterY = screenGeometry.center().y();

    int maxWidth = screenGeometry.width() * 0.8;
    int maxHeight = screenGeometry.height() * 0.8;
    QPixmap scaled = originalPixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    m_previewWindow = new QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
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

// ----------------------------------------------------------------------------
// Klik -> fullscreen
// ----------------------------------------------------------------------------
void itemList::onPhotoClicked(PhotoItem *item)
{
    QPixmap original = item->data(0).value<QPixmap>();
    if (original.isNull()) return;

    FullScreenPhotoViewer *viewer = new FullScreenPhotoViewer(original, this);
    viewer->show();
}

// ----------------------------------------------------------------------------
// Pozostałe sloty
// ----------------------------------------------------------------------------
void itemList::onNewButtonClicked()
{
    MainWindow *addWindow = new MainWindow(this);
    addWindow->setAttribute(Qt::WA_DeleteOnClose);
    addWindow->setEditMode(false, QString());
    connect(addWindow, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    addWindow->show();
}

void itemList::onEditButtonClicked()
{
    QItemSelectionModel *selModel = ui->itemList_tableView->selectionModel();
    if (!selModel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do edycji."));
        return;
    }
    QModelIndex index = selModel->selectedRows().first();
    QString recordId = model->data(model->index(index.row(), 0)).toString();
    m_currentRecordId = recordId;

    MainWindow *editWindow = new MainWindow(this);
    editWindow->setAttribute(Qt::WA_DeleteOnClose);
    editWindow->setEditMode(true, recordId);
    connect(editWindow, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    editWindow->show();
}

void itemList::onDeleteButtonClicked()
{
    QItemSelectionModel *sel = ui->itemList_tableView->selectionModel();
    if (!sel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do usunięcia."));
        return;
    }
    int row = sel->selectedRows().first().row();
    QString recordId = model->data(model->index(row, 0)).toString();

    auto btn = QMessageBox::question(this,
                                     tr("Potwierdzenie"),
                                     tr("Czy na pewno usunąć rekord %1?").arg(recordId),
                                     QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        QSqlQuery query(QSqlDatabase::database("default_connection"));
        query.prepare("DELETE FROM eksponaty WHERE id = :id");
        query.bindValue(":id", recordId);
        if (!query.exec()) {
            QMessageBox::critical(this, tr("Błąd"),
                                  tr("Nie udało się usunąć:\n%1").arg(query.lastError().text()));
        } else {
            model->select();
            QMessageBox::information(this, tr("Sukces"), tr("Rekord usunięty."));
        }
    }
}

void itemList::onCloneButtonClicked()
{
    QItemSelectionModel *sel = ui->itemList_tableView->selectionModel();
    if (!sel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do klonowania."));
        return;
    }
    int row = sel->selectedRows().first().row();
    QString recordId = model->data(model->index(row, 0)).toString();

    MainWindow *clone = new MainWindow(this);
    clone->setAttribute(Qt::WA_DeleteOnClose);
    clone->setCloneMode(recordId);
    connect(clone, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    clone->show();
}

void itemList::onEndButtonClicked()
{
    qApp->quit();
}

void itemList::onRecordSaved(const QString &recordId)
{
    refreshList(recordId);
}

void itemList::refreshList(const QString &recordId)
{
    model->select();
    ui->itemList_tableView->resizeColumnsToContents();

    if (!recordId.isEmpty()) {
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex idx = model->index(row, 0);
            if (model->data(idx).toString() == recordId) {
                QItemSelectionModel *s = ui->itemList_tableView->selectionModel();
                s->clearSelection();
                s->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                m_currentRecordId = recordId;
                break;
            }
        }
    }
}
