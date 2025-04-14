#include "itemList.h"
#include "ui_itemList.h"
#include "mainwindow.h"
#include "photoitem.h"

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

itemList::itemList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::itemList),
    m_currentRecordId(QString()),
    m_previewWindow(nullptr)
{
    ui->setupUi(this);
    qDebug() << "itemList: UI zainicjalizowane";

    QDir homeDir = QDir::home();
    QString defaultDbPath = homeDir.filePath("remote_sqlite/muzeum.db");
    qDebug() << "itemList: Domyślna ścieżka bazy:" << defaultDbPath;

    QSqlDatabase db;
    QSettings settings("Inwentaryzacja", "App");
    QString savedDbPath = settings.value("database_path", "").toString();
    qDebug() << "itemList: Zapisana ścieżka bazy:" << savedDbPath;

    QString selectedPath;
    bool dbOpened = false;

    if (!QSqlDatabase::contains("default_connection")) {
        qDebug() << "itemList: Tworzenie nowego połączenia z bazą";

        if (!savedDbPath.isEmpty() && QFile::exists(savedDbPath)) {
            selectedPath = savedDbPath;
            qDebug() << "itemList: Próba użycia zapisanej ścieżki:" << selectedPath;

            db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");
            db.setDatabaseName(selectedPath);

            if (db.open() && verifyDatabaseSchema(db)) {
                qDebug() << "itemList: Zapisana baza otwarta i poprawna";
                dbOpened = true;
            } else {
                qDebug() << "itemList: Zapisana baza istnieje, ale jest nieprawidłowa lub nie można jej otworzyć:" << db.lastError().text();
                db.close();
                QSqlDatabase::removeDatabase("default_connection");
                selectedPath.clear();
            }
        }

        if (!dbOpened) {
            QFileDialog::Options options = QFileDialog::DontConfirmOverwrite;
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
            selectedPath = QFileDialog::getSaveFileName(
                this,
                tr("Wybierz lub utwórz bazę danych"),
                defaultDbPath,
                "SQLite DB (*.db)",
                nullptr,
                options
                );
#else
            selectedPath = QFileDialog::getSaveFileName(
                this,
                tr("Wybierz lub utwórz bazę danych"),
                defaultDbPath,
                "SQLite DB (*.db)",
                nullptr,
                options
                );
#endif

            if (selectedPath.isEmpty()) {
                qDebug() << "itemList: Użytkownik nie wybrał ani nie podał nazwy bazy";
                QMessageBox::critical(this, tr("Błąd"), tr("Nie wybrano bazy danych. Aplikacja zostanie zamknięta."));
                qApp->quit();
                return;
            }
            qDebug() << "itemList: Wybrana ścieżka:" << selectedPath;

            db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");
            db.setDatabaseName(selectedPath);
            qDebug() << "itemList: Ustawiono nazwę bazy:" << selectedPath;

            if (QFile::exists(selectedPath)) {
                qDebug() << "itemList: Baza istnieje, próba otwarcia";
                if (!db.open()) {
                    qDebug() << "itemList: Błąd otwarcia bazy:" << db.lastError().text();
                    QMessageBox::critical(this, tr("Błąd"), tr("Nie można otworzyć bazy:\n%1").arg(db.lastError().text()));
                    qApp->quit();
                    return;
                }
                if (!verifyDatabaseSchema(db)) {
                    qDebug() << "itemList: Nieprawidłowy schemat bazy";
                    QMessageBox::critical(this, tr("Błąd"), tr("Wybrana baza danych ma nieprawidłowy schemat."));
                    db.close();
                    QSqlDatabase::removeDatabase("default_connection");
                    qApp->quit();
                    return;
                }
                qDebug() << "itemList: Schemat bazy zweryfikowany pozytywnie";
            } else {
                qDebug() << "itemList: Baza nie istnieje, tworzenie nowej";
                if (!db.open()) {
                    qDebug() << "itemList: Błąd tworzenia bazy:" << db.lastError().text();
                    QMessageBox::critical(this, tr("Błąd"), tr("Nie można utworzyć bazy:\n%1").arg(db.lastError().text()));
                    qApp->quit();
                    return;
                }
                createDatabaseSchema(db);
                qDebug() << "itemList: Schemat bazy utworzony";

                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    tr("Przykładowe dane"),
                    tr("Czy uzupełnić bazę przykładowymi rekordami?"),
                    QMessageBox::Yes | QMessageBox::No
                    );
                if (reply == QMessageBox::Yes) {
                    insertSampleData(db);
                    qDebug() << "itemList: Wstawiono przykładowe dane";
                }
            }
            settings.setValue("database_path", selectedPath);
            qDebug() << "itemList: Ścieżka zapisana w QSettings:" << selectedPath;
        }
    } else {
        db = QSqlDatabase::database("default_connection");
        qDebug() << "itemList: Użycie istniejącego połączenia";
        if (!db.isOpen()) {
            qDebug() << "itemList: Błąd - istniejące połączenie nie jest otwarte";
            QMessageBox::critical(this, tr("Błąd"), tr("Brak połączenia z bazą danych."));
            qApp->quit();
            return;
        }
    }

    qDebug() << "itemList: Inicjalizacja modelu relacyjnego";
    QSqlRelationalTableModel *relModel = new QSqlRelationalTableModel(this, db);
    relModel->setTable("eksponaty");
    relModel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);

    // Uwaga: relacje na TEXT mogą nie zawsze działać w QSqlRelation, zależy od wersji Qt.
    // Zostawiamy to, jak jest - jeśli będą problemy, trzeba je obsłużyć samodzielnie.
    relModel->setRelation(2, QSqlRelation("types", "id", "name"));
    relModel->setRelation(3, QSqlRelation("vendors", "id", "name"));
    relModel->setRelation(4, QSqlRelation("models", "id", "name"));
    relModel->setRelation(9, QSqlRelation("statuses", "id", "name"));
    relModel->setRelation(10, QSqlRelation("storage_places", "id", "name"));

    if (!relModel->select()) {
        qDebug() << "itemList: Błąd przy select():" << relModel->lastError().text();
    }

    relModel->setHeaderData(1, Qt::Horizontal, tr("Nazwa"));
    relModel->setHeaderData(2, Qt::Horizontal, tr("Typ"));
    relModel->setHeaderData(3, Qt::Horizontal, tr("Producent"));
    relModel->setHeaderData(4, Qt::Horizontal, tr("Model"));
    relModel->setHeaderData(5, Qt::Horizontal, tr("Numer seryjny"));
    relModel->setHeaderData(6, Qt::Horizontal, tr("Part number"));
    relModel->setHeaderData(7, Qt::Horizontal, tr("Revision"));
    relModel->setHeaderData(8, Qt::Horizontal, tr("Rok produkcji"));
    relModel->setHeaderData(9, Qt::Horizontal, tr("Status"));
    relModel->setHeaderData(10, Qt::Horizontal, tr("Miejsce przechowywania"));
    relModel->setHeaderData(11, Qt::Horizontal, tr("Opis"));
    relModel->setHeaderData(12, Qt::Horizontal, tr("Ilość"));

    ui->itemList_tableView->setItemDelegate(new QSqlRelationalDelegate(ui->itemList_tableView));
    model = relModel;

    ui->itemList_tableView->setModel(model);
    ui->itemList_tableView->resizeColumnsToContents();
    ui->itemList_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemList_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemList_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ukrywamy kolumnę 0, bo to jest ID w postaci tekstowej
    ui->itemList_tableView->hideColumn(0);
    // kolumna 13 nie istnieje w nowym schemacie (być może Ty masz inny layout)

    connect(ui->itemList_pushButton_new, &QPushButton::clicked, this, &itemList::onNewButtonClicked);
    connect(ui->itemList_pushButton_edit, &QPushButton::clicked, this, &itemList::onEditButtonClicked);
    connect(ui->itemList_pushButton_end, &QPushButton::clicked, this, &itemList::onEndButtonClicked);
    connect(ui->itemList_pushButton_delete, &QPushButton::clicked, this, &itemList::onDeleteButtonClicked);
    connect(ui->itemList_pushButton_clone, &QPushButton::clicked, this, &itemList::onCloneButtonClicked);

    QItemSelectionModel *selectionModel = ui->itemList_tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &itemList::onTableViewSelectionChanged);

    qDebug() << "itemList: Konstruktor zakończony";
}

itemList::~itemList()
{
    delete m_previewWindow;
    delete ui;
    qDebug() << "itemList: Destruktor wywołany";
}

bool itemList::verifyDatabaseSchema(QSqlDatabase &db)
{
    qDebug() << "itemList: Weryfikacja schematu bazy";
    QStringList requiredTables = {
        "eksponaty", "types", "vendors", "models",
        "statuses", "storage_places", "photos"
    };
    QStringList tables = db.tables();
    for (const QString &table : requiredTables) {
        if (!tables.contains(table)) {
            qDebug() << "itemList: Brak tabeli:" << table;
            return false;
        }
    }
    qDebug() << "itemList: Schemat poprawny";
    return true;
}

void itemList::createDatabaseSchema(QSqlDatabase &db)
{
    qDebug() << "itemList: Tworzenie schematu bazy";
    QSqlQuery query(db);
    query.exec("PRAGMA foreign_keys = ON;");
    query.exec("PRAGMA journal_mode=WAL;"); // Włącz WAL

    // ZAMIANA: id TEXT PRIMARY KEY + foreign key TEXT
    query.exec(R"(
        CREATE TABLE eksponaty (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            type_id TEXT NOT NULL,
            vendor_id TEXT NOT NULL,
            model_id TEXT NOT NULL,
            serial_number TEXT,
            part_number TEXT,
            revision TEXT,
            production_year INTEGER CHECK(production_year BETWEEN 1900 AND 2100),
            status_id TEXT NOT NULL,
            storage_place_id TEXT NOT NULL,
            description TEXT,
            value INTEGER CHECK(value >= 0),
            FOREIGN KEY (type_id) REFERENCES types(id) ON DELETE RESTRICT ON UPDATE CASCADE,
            FOREIGN KEY (vendor_id) REFERENCES vendors(id) ON DELETE RESTRICT ON UPDATE CASCADE,
            FOREIGN KEY (model_id) REFERENCES models(id) ON DELETE RESTRICT ON UPDATE CASCADE,
            FOREIGN KEY (status_id) REFERENCES statuses(id) ON DELETE RESTRICT ON UPDATE CASCADE,
            FOREIGN KEY (storage_place_id) REFERENCES storage_places(id) ON DELETE RESTRICT ON UPDATE CASCADE
        )
    )");

    query.exec(R"(
        CREATE TABLE types (
            id TEXT PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE vendors (
            id TEXT PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE models (
            id TEXT PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            vendor_id TEXT NOT NULL,
            FOREIGN KEY (vendor_id) REFERENCES vendors(id) ON DELETE CASCADE ON UPDATE CASCADE
        )
    )");

    query.exec(R"(
        CREATE TABLE statuses (
            id TEXT PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE storage_places (
            id TEXT PRIMARY KEY,
            name TEXT UNIQUE NOT NULL
        )
    )");

    query.exec(R"(
        CREATE TABLE photos (
            id TEXT PRIMARY KEY,
            eksponat_id TEXT NOT NULL,
            photo BLOB NOT NULL,
            FOREIGN KEY (eksponat_id) REFERENCES eksponaty(id) ON DELETE CASCADE ON UPDATE CASCADE
        )
    )");

    // Indeks po numerze seryjnym
    query.exec("CREATE INDEX idx_serial_number ON eksponaty(serial_number);");

    if (query.lastError().isValid()) {
        qDebug() << "itemList: Błąd tworzenia schematu:" << query.lastError().text();
    } else {
        qDebug() << "itemList: Schemat utworzony pomyślnie";
    }
}

void itemList::insertSampleData(QSqlDatabase &db)
{
    qDebug() << "itemList: Wstawianie przykładowych danych";
    QSqlQuery query(db);

    // W ramach przykładu generujemy proste ID (UUID)
    auto genId = []() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    };

    QString t1 = genId();
    QString t2 = genId();
    QString t3 = genId();

    query.prepare("INSERT INTO types (id, name) VALUES (:id, :name)");
    query.bindValue(":id", t1); query.bindValue(":name", "Komputer"); query.exec();
    query.bindValue(":id", t2); query.bindValue(":name", "Monitor");  query.exec();
    query.bindValue(":id", t3); query.bindValue(":name", "Kabel");    query.exec();

    QString v1 = genId();
    QString v2 = genId();
    QString v3 = genId();

    query.prepare("INSERT INTO vendors (id, name) VALUES (:id, :name)");
    query.bindValue(":id", v1); query.bindValue(":name", "Atari");      query.exec();
    query.bindValue(":id", v2); query.bindValue(":name", "Commodore"); query.exec();
    query.bindValue(":id", v3); query.bindValue(":name", "Sinclair");  query.exec();

    QString m1 = genId();
    QString m2 = genId();
    QString m3 = genId();

    query.prepare("INSERT INTO models (id, name, vendor_id) VALUES (:id, :name, :vendor_id)");
    query.bindValue(":id", m1); query.bindValue(":name", "Atari 800XL");  query.bindValue(":vendor_id", v1); query.exec();
    query.bindValue(":id", m2); query.bindValue(":name", "Amiga 500");    query.bindValue(":vendor_id", v2); query.exec();
    query.bindValue(":id", m3); query.bindValue(":name", "ZX Spectrum");  query.bindValue(":vendor_id", v3); query.exec();

    QString s1 = genId();
    QString s2 = genId();
    QString s3 = genId();

    query.prepare("INSERT INTO statuses (id, name) VALUES (:id, :name)");
    query.bindValue(":id", s1); query.bindValue(":name", "Sprawny");    query.exec();
    query.bindValue(":id", s2); query.bindValue(":name", "Uszkodzony"); query.exec();
    query.bindValue(":id", s3); query.bindValue(":name", "W naprawie"); query.exec();

    QString sp1 = genId();
    QString sp2 = genId();

    query.prepare("INSERT INTO storage_places (id, name) VALUES (:id, :name)");
    query.bindValue(":id", sp1); query.bindValue(":name", "Magazyn 1"); query.exec();
    query.bindValue(":id", sp2); query.bindValue(":name", "Półka B3");  query.exec();

    QString e1 = genId();
    query.prepare(R"(
        INSERT INTO eksponaty (id, name, type_id, vendor_id, model_id, serial_number,
            production_year, status_id, storage_place_id, description, value)
        VALUES (:id, :name, :type_id, :vendor_id, :model_id, :serial, :year,
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

    // analogicznie...
    QString e2 = genId();
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
        qDebug() << "itemList: Błąd wstawiania danych przykładowych:" << query.lastError().text();
    } else {
        qDebug() << "itemList: Dane przykładowe wstawione";
    }
}

void itemList::onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    if (selected.indexes().isEmpty()) {
        ui->itemList_graphicsView->setScene(nullptr);
        m_currentRecordId.clear();
        return;
    }
    QModelIndex index = selected.indexes().first();
    int row = index.row();
    // Indeks kolumny 0 to TEXT ID
    QString recordId = model->data(model->index(row, 0)).toString();
    m_currentRecordId = recordId;

    QSqlQuery query(QSqlDatabase::database("default_connection"));
    query.prepare("SELECT photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", m_currentRecordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęć:" << query.lastError().text();
        ui->itemList_graphicsView->setScene(nullptr);
        return;
    }

    int viewWidth = ui->itemList_graphicsView->viewport()->width() - 10;
    int viewHeight = ui->itemList_graphicsView->viewport()->height() - 10;

    QList<QPixmap> pixmaps;
    while (query.next()) {
        QByteArray imageData = query.value("photo").toByteArray();
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            pixmaps.append(pixmap);
        } else {
            qDebug() << "Nie można załadować zdjęcia BLOB.";
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
    for (int i = 0; i < photoCount; ++i) {
        QPixmap original = pixmaps[i];
        QPixmap scaled = original.scaled(
            maxThumbnailWidth, maxThumbnailHeight,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );

        PhotoItem *item = new PhotoItem();
        item->setPixmap(scaled);
        item->setData(0, QVariant(original));
        item->setPos(x, y);
        scene->addItem(item);

        connect(item, &PhotoItem::hovered, this, &itemList::onPhotoHovered);
        connect(item, &PhotoItem::unhovered, this, &itemList::onPhotoUnhovered);

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

void itemList::onPhotoHovered(PhotoItem *item)
{
    if (m_previewWindow) {
        m_previewWindow->close();
        m_previewWindow = nullptr;
    }

    QPixmap originalPixmap = item->data(0).value<QPixmap>();
    if (originalPixmap.isNull()) {
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qDebug() << "Nie można uzyskać informacji o ekranie.";
        return;
    }
    QRect screenGeometry = screen->availableGeometry();
    int screenCenterX = screenGeometry.center().x();
    int screenCenterY = screenGeometry.center().y();

    int maxWidth = screenGeometry.width() * 0.8;
    int maxHeight = screenGeometry.height() * 0.8;
    QPixmap scaledPixmap = originalPixmap.scaled(
        maxWidth, maxHeight,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    m_previewWindow = new QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    m_previewWindow->setAttribute(Qt::WA_TranslucentBackground);
    m_previewWindow->setStyleSheet(
        "background-color: rgba(0, 0, 0, 200); border-radius: 10px;"
        );

    QLabel *imageLabel = new QLabel(m_previewWindow);
    imageLabel->setStyleSheet("background: transparent;");
    imageLabel->setPixmap(scaledPixmap);

    m_previewWindow->setFixedSize(scaledPixmap.width() + 20, scaledPixmap.height() + 20);
    imageLabel->setGeometry(10, 10, scaledPixmap.width(), scaledPixmap.height());

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
    QItemSelectionModel *selectionModel = ui->itemList_tableView->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do edycji."));
        return;
    }
    QModelIndex index = selectionModel->selectedRows().first();
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
    QItemSelectionModel *selModel = ui->itemList_tableView->selectionModel();
    if (!selModel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do usunięcia."));
        return;
    }
    QModelIndexList selected = selModel->selectedRows();
    if (selected.isEmpty())
        return;

    int row = selected.first().row();
    QString recordId = model->data(model->index(row, 0)).toString();

    QMessageBox::StandardButton btn = QMessageBox::question(
        this,
        tr("Potwierdzenie"),
        tr("Czy na pewno chcesz usunąć rekord o ID %1?").arg(recordId),
        QMessageBox::Yes | QMessageBox::No
        );

    if (btn == QMessageBox::Yes) {
        QSqlQuery query(QSqlDatabase::database("default_connection"));
        query.prepare("DELETE FROM eksponaty WHERE id = :id");
        query.bindValue(":id", recordId);
        if (!query.exec()) {
            QMessageBox::critical(this, tr("Błąd"),
                                  tr("Nie udało się usunąć rekordu:\n%1")
                                      .arg(query.lastError().text()));
        } else {
            model->select();
            QMessageBox::information(this, tr("Sukces"), tr("Rekord został usunięty."));
        }
    }
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
                QItemSelectionModel *sel = ui->itemList_tableView->selectionModel();
                sel->clearSelection();
                sel->select(model->index(row, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                m_currentRecordId = recordId;
                break;
            }
        }
    }
}

void itemList::onCloneButtonClicked()
{
    QItemSelectionModel *selectionModel = ui->itemList_tableView->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do klonowania."));
        return;
    }
    QModelIndex index = selectionModel->selectedRows().first();
    QString recordId = model->data(model->index(index.row(), 0)).toString();

    MainWindow *cloneWindow = new MainWindow(this);
    cloneWindow->setAttribute(Qt::WA_DeleteOnClose);
    cloneWindow->setCloneMode(recordId);
    connect(cloneWindow, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    cloneWindow->show();
}
