/**
 * @file itemList.cpp
 * @brief Implementacja klasy itemList do zarządzania listą eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera implementację metod klasy itemList, odpowiedzialnej za wyświetlanie i zarządzanie
 * listą eksponatów w aplikacji inwentaryzacyjnej. Obsługuje połączenia z bazą danych MySQL,
 * filtrowanie kaskadowe, wyświetlanie miniatur zdjęć oraz interakcje użytkownika (dodawanie,
 * edycja, usuwanie, klonowanie rekordów). Wykorzystuje QSqlRelationalTableModel jako źródło
 * danych oraz ItemFilterProxyModel do dynamicznego filtrowania.
 */

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
#include <QTimer>
#include <functional>

/**
 * @brief Konstruktor klasy itemList.
 * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
 *
 * Inicjalizuje interfejs użytkownika, ustanawia połączenie z bazą danych MySQL, konfiguruje
 * model danych (QSqlRelationalTableModel), model proxy (ItemFilterProxyModel) oraz filtry
 * kaskadowe w combo boxach. Podłącza sygnały i sloty dla przycisków, tabeli i interakcji ze
 * zdjęciami. W razie potrzeby tworzy schemat bazy danych i wstawia przykładowe dane.
 */
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
    , m_currentHoveredItem(nullptr)
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
        db.setHostName("localhost");
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

    // Ustaw nagłówki kolumn
    m_sourceModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_sourceModel->setHeaderData(1, Qt::Horizontal, tr("Nazwa"));
    m_sourceModel->setHeaderData(2, Qt::Horizontal, tr("Typ"));
    m_sourceModel->setHeaderData(3, Qt::Horizontal, tr("Producent"));
    m_sourceModel->setHeaderData(4, Qt::Horizontal, tr("Model"));
    m_sourceModel->setHeaderData(5, Qt::Horizontal, tr("Numer seryjny"));
    m_sourceModel->setHeaderData(6, Qt::Horizontal, tr("Part number"));
    m_sourceModel->setHeaderData(7, Qt::Horizontal, tr("Revision"));
    m_sourceModel->setHeaderData(8, Qt::Horizontal, tr("Rok produkcji"));
    m_sourceModel->setHeaderData(9, Qt::Horizontal, tr("Status"));
    m_sourceModel->setHeaderData(10, Qt::Horizontal, tr("Miejsce przechowywania"));
    m_sourceModel->setHeaderData(11, Qt::Horizontal, tr("Opis"));
    m_sourceModel->setHeaderData(12, Qt::Horizontal, tr("Ilość"));

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

    // Podłączenie slotów dla kaskadowego filtrowania
    connect(filterTypeComboBox, &QComboBox::currentTextChanged, this, &itemList::onFilterChanged);
    connect(filterVendorComboBox, &QComboBox::currentTextChanged, this, &itemList::onFilterChanged);
    connect(filterModelComboBox, &QComboBox::currentTextChanged, this, &itemList::onFilterChanged);
    connect(filterStatusComboBox, &QComboBox::currentTextChanged, this, &itemList::onFilterChanged);
    connect(filterStorageComboBox, &QComboBox::currentTextChanged, this, &itemList::onFilterChanged);
    onFilterChanged();

    // Inicjalizacja timera do sprawdzania pozycji kursora
    m_hoverCheckTimer = new QTimer(this);
    connect(m_hoverCheckTimer, &QTimer::timeout, this, [this]() {
        QPoint globalPos = QCursor::pos();
        if (m_previewWindow && !m_previewWindow->geometry().contains(globalPos)) {
            QWidget *widgetUnderCursor = QApplication::widgetAt(globalPos);
            if (!qobject_cast<PhotoItem *>(widgetUnderCursor)) {
                if (m_previewWindow) {
                    m_previewWindow->close();
                    m_previewWindow = nullptr;
                    m_currentHoveredItem = nullptr;
                }
            }
        }
    });
}

/**
 * @brief Destruktor klasy itemList.
 *
 * Zatrzymuje timer utrzymujący połączenie z bazą danych i zwalnia zasoby interfejsu użytkownika.
 */
itemList::~itemList()
{
    if (m_keepAliveTimer)
        m_keepAliveTimer->stop();
    delete ui;
}

/**
 * @brief Inicjalizuje filtry combo boxów.
 * @param db Referencja do obiektu bazy danych.
 *
 * Wypełnia combo boxy danymi z odpowiednich tabel bazy danych (types, vendors, models, statuses,
 * storage_places) i podłącza sloty do aktualizacji filtrów w modelu proxy. Każdy combo box
 * zawiera opcję "Wszystkie" oraz unikalne wartości z danej tabeli.
 */
void itemList::initFilters(QSqlDatabase &db)
{
    auto initFilter = [&](QComboBox *cb, const QString &table, std::function<void(const QString &)> setter) {
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

/**
 * @brief Odświeża filtry combo boxów.
 *
 * Zachowuje aktualnie wybrane wartości filtrów, odświeża dane w combo boxach na podstawie
 * zawartości bazy danych i przywraca wybrane wartości, jeśli nadal istnieją.
 */
void itemList::refreshFilters()
{
    QSqlDatabase db = QSqlDatabase::database("default_connection");
    if (!db.isOpen()) {
        qDebug() << "itemList: Błąd - połączenie z bazą danych zamknięte podczas odświeżania filtrów.";
        return;
    }

    QString currentType = filterTypeComboBox->currentText();
    QString currentVendor = filterVendorComboBox->currentText();
    QString currentModel = filterModelComboBox->currentText();
    QString currentStatus = filterStatusComboBox->currentText();
    QString currentStorage = filterStorageComboBox->currentText();

    initFilters(db);

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

/**
 * @brief Obsługuje zmianę zaznaczenia w tabeli eksponatów.
 * @param selected Zaznaczone indeksy.
 * @param deselected Odznaczone indeksy.
 *
 * Wczytuje zdjęcia powiązane z wybranym eksponatem z tabeli photos i wyświetla ich miniatury
 * w QGraphicsView. Ustawia siatkę miniaturek w zależności od liczby zdjęć i rozmiaru widoku.
 */
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

        connect(item, &PhotoItem::clicked, this, &itemList::onPhotoHovered);
        // Emituje sygnał kliknięcia po jednokrotnym kliknięciu lewym przyciskiem myszy

        connect(item,
                &PhotoItem::doubleClicked,
                this,
                &itemList::onPhotoClicked); // Emituje sygnał podwójnego kliknięcia lewym przyciskiem myszy

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

/**
 * @brief Otwiera okno dodawania nowego eksponatu.
 *
 * Tworzy nowe okno MainWindow w trybie dodawania rekordu i podłącza sygnał zapisu rekordu.
 */
void itemList::onNewButtonClicked()
{
    MainWindow *w = new MainWindow(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setEditMode(false, QString());
    connect(w, &MainWindow::recordSaved, this, &itemList::onRecordSaved);
    w->show();
}

/**
 * @brief Otwiera okno edycji wybranego eksponatu.
 *
 * Sprawdza, czy wybrano rekord, pobiera jego ID i otwiera okno MainWindow w trybie edycji.
 */
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

/**
 * @brief Otwiera okno klonowania wybranego eksponatu.
 *
 * Sprawdza, czy wybrano rekord, pobiera jego ID i otwiera okno MainWindow w trybie klonowania.
 */
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

/**
 * @brief Usuwa wybrany eksponat po potwierdzeniu.
 *
 * Sprawdza, czy wybrano rekord, wyświetla okno potwierdzenia i usuwa rekord z tabeli eksponaty.
 */
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

/**
 * @brief Zamyka aplikację.
 */
void itemList::onEndButtonClicked()
{
    qApp->quit();
}

/**
 * @brief Wyświetla okno "O programie".
 *
 * Pokazuje informacje o aplikacji, w tym nazwę, wersję i autorów.
 */
void itemList::onAboutClicked()
{
    const QString html = tr("<h3>%1</h3>"
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

/**
 * @brief Wyświetla podgląd zdjęcia po najechaniu na miniaturę.
 * @param item Wskaźnik na element PhotoItem.
 *
 * Tworzy okno podglądu z powiększonym zdjęciem, centruje je względem miniaturki i przesuwa kursor
 * na środek podglądu. Timer m_hoverCheckTimer sprawdza, czy kursor opuścił podgląd.
 */
void itemList::onPhotoHovered(PhotoItem *item)
{
    if (m_previewWindow && m_currentHoveredItem == item)
        return;

    if (m_previewWindow) {
        m_previewWindow->close();
        m_previewWindow = nullptr;
        m_currentHoveredItem = nullptr;
    }

    QPixmap originalPixmap = item->data(0).value<QPixmap>();
    if (originalPixmap.isNull())
        return;

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    QRect screenGeometry = screen->availableGeometry();
    int maxX = screenGeometry.right();
    int maxY = screenGeometry.bottom();

    int maxWidth = screenGeometry.width() * 0.8;
    int maxHeight = screenGeometry.height() * 0.8;
    QPixmap scaled = originalPixmap.scaled(maxWidth,
                                           maxHeight,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

    m_previewWindow = new QWidget(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    m_previewWindow->installEventFilter(this);

    m_previewWindow->setAttribute(Qt::WA_TranslucentBackground);
    m_previewWindow->setStyleSheet("background-color: rgba(0, 0, 0, 200); border-radius: 10px;");

    QLabel *imageLabel = new QLabel(m_previewWindow);
    imageLabel->setStyleSheet("background: transparent;");
    imageLabel->setPixmap(scaled);

    m_previewWindow->setFixedSize(scaled.width() + 20, scaled.height() + 20);
    imageLabel->setGeometry(10, 10, scaled.width(), scaled.height());
    QPoint originalPos = item->scenePos().toPoint();
    QPoint globalThumbPos = ui->itemList_graphicsView->viewport()->mapToGlobal(originalPos);

    int previewW = m_previewWindow->width();
    int previewH = m_previewWindow->height();

    int windowX = globalThumbPos.x() - previewW / 2;
    int windowY = globalThumbPos.y() - previewH / 2;

    windowX = qBound(screenGeometry.left(), windowX, maxX - previewW);
    windowY = qBound(screenGeometry.top(), windowY, maxY - previewH);

    m_previewWindow->move(windowX, windowY);

    m_previewWindow->raise();
    m_previewWindow->activateWindow();
    m_previewWindow->show();

    QPoint centerInGlobal = m_previewWindow->mapToGlobal(QPoint(m_previewWindow->width() / 2,
                                                                m_previewWindow->height() / 2));
    QCursor::setPos(centerInGlobal);
    m_currentHoveredItem = item;
    if (m_hoverCheckTimer && !m_hoverCheckTimer->isActive())
        m_hoverCheckTimer->start(100);
}

/**
 * @brief Ukrywa podgląd zdjęcia po opuszczeniu miniatury.
 * @param item Wskaźnik na element PhotoItem.
 *
 * Zamyka okno podglądu po krótkim opóźnieniu, jeśli kursor nie znajduje się nad oknem podglądu.
 */
void itemList::onPhotoUnhovered(PhotoItem *item)
{
    if (item != m_currentHoveredItem)
        return;

    QTimer::singleShot(100, this, [=]() {
        QPoint globalCursorPos = QCursor::pos();
        if (m_previewWindow && m_previewWindow->geometry().contains(globalCursorPos)) {
            m_previewHovered = true;
            return;
        }

        m_currentHoveredItem = nullptr;

        if (m_previewWindow) {
            m_previewWindow->close();
            if (m_hoverCheckTimer)
                m_hoverCheckTimer->stop();

            m_previewWindow = nullptr;
            m_previewHovered = false;
        }
    });
}

/**
 * @brief Obsługuje zdarzenia filtrowania dla okna podglądu zdjęć.
 * @param watched Obiekt, dla którego przetwarzane jest zdarzenie.
 * @param event Wskaźnik na zdarzenie.
 * @return true, jeśli zdarzenie zostało obsłużone; false w przeciwnym razie.
 *
 * Zamyka okno podglądu, gdy kursor opuści jego obszar.
 */
bool itemList::eventFilter(QObject *watched, QEvent *event)
{
    if (!watched || !event || !m_previewWindow)
        return QWidget::eventFilter(watched, event);

    if (watched == m_previewWindow) {
        if (event->type() == QEvent::Leave) {
            m_previewHovered = false;
            if (m_previewWindow) {
                m_previewWindow->close();
                m_previewWindow = nullptr;
                m_currentHoveredItem = nullptr;
            }
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

/**
 * @brief Otwiera zdjęcie w trybie pełnoekranowym.
 * @param item Wskaźnik na element PhotoItem.
 *
 * Tworzy nowe okno FullScreenPhotoViewer z oryginalnym zdjęciem.
 */
void itemList::onPhotoClicked(PhotoItem *item)
{
    QPixmap original = item->data(0).value<QPixmap>();
    if (original.isNull())
        return;

    FullScreenPhotoViewer *viewer = new FullScreenPhotoViewer(original, this);
    viewer->show();
}

/**
 * @brief Odświeża listę po zapisaniu rekordu.
 * @param recordId ID zapisanego rekordu.
 *
 * Wywołuje odświeżenie listy eksponatów i ustawia zaznaczenie na zapisanym rekordzie.
 */
void itemList::onRecordSaved(const QString &recordId)
{
    refreshList(recordId);
}

/**
 * @brief Odświeża listę eksponatów.
 * @param recordId Opcjonalne ID rekordu do wybrania po odświeżeniu.
 *
 * Odświeża model danych, filtry i tabelę, a jeśli podano recordId, zaznacza odpowiedni rekord.
 */
void itemList::refreshList(const QString &recordId)
{
    m_sourceModel->select();
    ui->itemList_tableView->resizeColumnsToContents();

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

/**
 * @brief Weryfikuje schemat bazy danych.
 * @param db Referencja do obiektu bazy danych.
 * @return true, jeśli wszystkie wymagane tabele istnieją; false w przeciwnym razie.
 *
 * Sprawdza, czy w bazie danych istnieją tabele: eksponaty, types, vendors, models, statuses,
 * storage_places oraz photos.
 */
bool itemList::verifyDatabaseSchema(QSqlDatabase &db)
{
    QStringList tables = {"eksponaty", "types", "vendors", "models", "statuses", "storage_places", "photos"};
    for (const QString &t : tables) {
        if (!db.tables().contains(t, Qt::CaseInsensitive))
            return false;
    }
    return true;
}

/**
 * @brief Tworzy schemat bazy danych.
 * @param db Referencja do obiektu bazy danych.
 *
 * Tworzy tabele eksponaty, types, vendors, models, statuses, storage_places i photos w bazie
 * danych MySQL, jeśli jeszcze nie istnieją.
 */
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

/**
 * @brief Wstawia przykładowe dane do bazy danych.
 * @param db Referencja do obiektu bazy danych.
 *
 * Dodaje przykładowe rekordy do tabel types, vendors, models, statuses, storage_places oraz
 * eksponaty, aby umożliwić testowanie aplikacji.
 */
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

/**
 * @brief Aktualizuje filtry po zmianie wartości w combo boxach.
 *
 * Inicjalizuje timer utrzymujący połączenie z bazą danych, aktualizuje filtry w modelu proxy
 * na podstawie wybranych wartości w combo boxach i odbudowuje listy wartości w combo boxach
 * za pomocą updateFilterComboBoxes().
 */
void itemList::onFilterChanged()
{
    m_keepAliveTimer = new QTimer(this);
    connect(m_keepAliveTimer, &QTimer::timeout, this, []() {
        QSqlQuery q(QSqlDatabase::database("default_connection"));
        q.exec("SELECT 1");
    });
    m_keepAliveTimer->start(30000);

    auto sel = [&](QComboBox *cb) {
        QString t = cb->currentText();
        return t == tr("Wszystkie") ? QString() : t;
    };
    m_proxyModel->setTypeFilter(sel(filterTypeComboBox));
    m_proxyModel->setVendorFilter(sel(filterVendorComboBox));
    m_proxyModel->setModelFilter(sel(filterModelComboBox));
    m_proxyModel->setStatusFilter(sel(filterStatusComboBox));
    m_proxyModel->setStorageFilter(sel(filterStorageComboBox));

    updateFilterComboBoxes();
}

/**
 * @brief Odbudowuje listy w combo boxach filtrów.
 *
 * Pobiera aktualne wybory z combo boxów i odbudowuje ich zawartość, uwzględniając zależności
 * między filtrami (kaskadowe filtrowanie). Zapytania SQL używają JOIN, aby uwzględnić wszystkie
 * tabele słownikowe.
 */
void itemList::updateFilterComboBoxes()
{
    QSqlDatabase db = QSqlDatabase::database("default_connection");
    if (!db.isOpen())
        return;

    QString selType = filterTypeComboBox->currentText() == tr("Wszystkie") ? QString() : filterTypeComboBox->currentText();
    QString selVendor = filterVendorComboBox->currentText() == tr("Wszystkie") ? QString() : filterVendorComboBox->currentText();
    QString selModel = filterModelComboBox->currentText() == tr("Wszystkie") ? QString() : filterModelComboBox->currentText();
    QString selStatus = filterStatusComboBox->currentText() == tr("Wszystkie") ? QString() : filterStatusComboBox->currentText();
    QString selStorage = filterStorageComboBox->currentText() == tr("Wszystkie") ? QString() : filterStorageComboBox->currentText();

    struct Filter {
        QComboBox *cb;
        QString field;
    };
    QVector<Filter> filters = {
        {filterTypeComboBox, "types.name"},
        {filterVendorComboBox, "vendors.name"},
        {filterModelComboBox, "models.name"},
        {filterStatusComboBox, "statuses.name"},
        {filterStorageComboBox, "storage_places.name"}
    };

    const QString baseJoins = R"(
        FROM eksponaty
        JOIN types ON eksponaty.type_id = types.id
        JOIN vendors ON eksponaty.vendor_id = vendors.id
        JOIN models ON eksponaty.model_id = models.id
        JOIN statuses ON eksponaty.status_id = statuses.id
        JOIN storage_places ON eksponaty.storage_place_id = storage_places.id
    )";

    for (auto &f : filters) {
        f.cb->blockSignals(true);
        QString prev = f.cb->currentText();
        f.cb->clear();
        f.cb->addItem(tr("Wszystkie"));

        QString sql = QString(
                          "SELECT DISTINCT %1 %2 "
                          "WHERE (:selType IS NULL OR types.name = :selType) "
                          "AND (:selVendor IS NULL OR vendors.name = :selVendor) "
                          "AND (:selModel IS NULL OR models.name = :selModel) "
                          "AND (:selStatus IS NULL OR statuses.name = :selStatus) "
                          "AND (:selStorage IS NULL OR storage_places.name = :selStorage) "
                          "ORDER BY %1")
                          .arg(f.field, baseJoins);

        QSqlQuery q(db);
        q.prepare(sql);
        q.bindValue(":selType", selType.isEmpty() ? QVariant() : selType);
        q.bindValue(":selVendor", selVendor.isEmpty() ? QVariant() : selVendor);
        q.bindValue(":selModel", selModel.isEmpty() ? QVariant() : selModel);
        q.bindValue(":selStatus", selStatus.isEmpty() ? QVariant() : selStatus);
        q.bindValue(":selStorage", selStorage.isEmpty() ? QVariant() : selStorage);
        if (!q.exec()) {
            qDebug() << "Błąd updateFilterComboBoxes:" << q.lastError().text();
        }
        while (q.next()) {
            f.cb->addItem(q.value(0).toString());
        }

        int idx = f.cb->findText(prev);
        f.cb->setCurrentIndex(idx != -1 ? idx : 0);
        f.cb->blockSignals(false);
    }
}
