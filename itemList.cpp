#include "itemList.h"
#include "ui_itemList.h"
#include "mainwindow.h"

#include <QSqlDatabase>
#include <QSqlTableModel>
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

itemList::itemList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::itemList),
    m_currentRecordId(-1) // Inicjalizacja zmiennej
{
    ui->setupUi(this);

    // Użycie domyślnego połączenia z bazą lub tworzenie nowego, jeśli nie istnieje
    QSqlDatabase db;
    if (QSqlDatabase::contains("default_connection")) {
        db = QSqlDatabase::database("default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");

        #ifdef Q_OS_LINUX
        db.setDatabaseName("/home/arekbr/inwentaryzacja/muzeum.db");
        #elif defined(Q_OS_MAC)
        db.setDatabaseName("/Users/Arek/inwentaryzacja/muzeum.db");
        #else
        #error "System operacyjny nie jest obsługiwany"
        #endif

        // db.setDatabaseName("/home/arekbr/inwentaryzacja/muzeum.db");
        if (!db.open()) {
            QMessageBox::critical(this, tr("Błąd bazy danych"),
                                  tr("Nie udało się otworzyć bazy danych:\n%1")
                                      .arg(db.lastError().text()));
            return;
        }
    }

    // Ustawienie modelu SQL do wyświetlania tabeli "eksponaty"
    model = new QSqlTableModel(this, db);
    model->setTable("eksponaty");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    ui->itemList_tableView->setModel(model);
    ui->itemList_tableView->resizeColumnsToContents();
    ui->itemList_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemList_tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // Połączenie przycisków z odpowiednimi slotami
    connect(ui->itemList_pushButton_new, &QPushButton::clicked, this, &itemList::onNewButtonClicked);
    connect(ui->itemList_pushButton_edit, &QPushButton::clicked, this, &itemList::onEditButtonClicked);
    connect(ui->itemList_pushButton_end, &QPushButton::clicked, this, &itemList::onEndButtonClicked);

    // Reakcja na zmianę zaznaczenia w tabeli – wyświetlenie obrazu w QGraphicsView
    QItemSelectionModel *selectionModel = ui->itemList_tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &itemList::onTableViewSelectionChanged);
}

itemList::~itemList()
{
    delete ui;
}

void itemList::onNewButtonClicked()
{
    // Otwieramy okno formularza w trybie dodawania
    MainWindow *addWindow = new MainWindow(this); // Ustawiamy itemList jako parent
    addWindow->setAttribute(Qt::WA_DeleteOnClose);
    addWindow->setEditMode(false); // false = tryb dodawania
    addWindow->show();
    connect(addWindow, &MainWindow::recordSaved, this, &itemList::onRecordSaved); // Podłączamy do nowego slotu
}

void itemList::onEditButtonClicked()
{
    // Sprawdzamy, czy użytkownik zaznaczył rekord do edycji
    QItemSelectionModel *selectionModel = ui->itemList_tableView->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać rekord do edycji."));
        return;
    }
    // Zakładamy, że kolumna 0 zawiera identyfikator rekordu
    QModelIndex index = selectionModel->selectedRows().first();
    int recordId = model->data(model->index(index.row(), 0)).toInt();
    m_currentRecordId = recordId; // Zapisz aktualny rekord

    // Otwieramy MainWindow w trybie edycji
    MainWindow *editWindow = new MainWindow(this); // Ustawiamy itemList jako parent
    editWindow->setAttribute(Qt::WA_DeleteOnClose);
    editWindow->setEditMode(true, recordId);
    editWindow->show();
    connect(editWindow, &MainWindow::recordSaved, this, &itemList::onRecordSaved); // Podłączamy do nowego slotu
}

void itemList::onEndButtonClicked()
{
    qApp->quit();
}

void itemList::onRecordSaved(int recordId)
{
    refreshList(recordId); // Przekazujemy ID do refreshList
}

void itemList::onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    // Jeśli nie ma zaznaczenia – czyścimy QGraphicsView
    if (selected.indexes().isEmpty()) {
        ui->itemList_graphicsView->setScene(nullptr);
        m_currentRecordId = -1;
        return;
    }

    // Pobierz ID rekordu z pierwszej kolumny (id w tabeli eksponaty)
    QModelIndex index = selected.indexes().first();
    int row = index.row();
    m_currentRecordId = model->data(model->index(row, 0)).toInt(); // Zaktualizuj bieżące ID

    // Pobierz wszystkie zdjęcia z tabeli photos dla danego rekordu
    QSqlQuery query(QSqlDatabase::database("default_connection"));
    query.prepare("SELECT photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", m_currentRecordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęć:" << query.lastError().text();
        ui->itemList_graphicsView->setScene(nullptr);
        return;
    }

    // Utwórz nową scenę graficzną
    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbnailSize = 80; // Rozmiar miniatur (kwadrat 80x80 pikseli)
    int x = 5; // Początkowa pozycja X z marginesem
    int y = 5; // Początkowa pozycja Y z marginesem
    const int spacing = 5; // Odstęp między miniaturami

    while (query.next()) {
        QByteArray imageData = query.value("photo").toByteArray();
        QPixmap pixmap;
        if (!pixmap.loadFromData(imageData)) {
            qDebug() << "Nie można załadować zdjęcia z danych BLOB dla rekordu" << m_currentRecordId;
            continue;
        }

        // Skaluj obraz do rozmiaru miniatury, zachowując proporcje
        QPixmap scaledPixmap = pixmap.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Dodaj miniaturę do sceny
        QGraphicsPixmapItem *item = scene->addPixmap(scaledPixmap);
        item->setPos(x, y);

        // Aktualizuj pozycję dla następnej miniatury
        x += thumbnailSize + spacing;
        if (x + thumbnailSize > ui->itemList_graphicsView->width() - 10) {
            x = 5;
            y += thumbnailSize + spacing;
        }
    }

    // Ustaw scenę w widżecie
    if (!scene->items().isEmpty()) {
        scene->setSceneRect(0, 0, ui->itemList_graphicsView->width() - 10, y + thumbnailSize + 5);
        ui->itemList_graphicsView->setScene(scene);
        ui->itemList_graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    } else {
        ui->itemList_graphicsView->setScene(nullptr);
        delete scene;
    }
}

void itemList::refreshList(int recordId)
{
    // Zapisz aktualne ID rekordu, jeśli nie podano nowego
    if (recordId == -1 && m_currentRecordId != -1) {
        recordId = m_currentRecordId;
    }

    // Odświeżenie danych w modelu
    model->select();
    ui->itemList_tableView->resizeColumnsToContents();

    // Ustaw fokus na rekordzie o podanym ID
    if (recordId != -1) {
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex index = model->index(row, 0); // Kolumna 0 to id
            if (model->data(index).toInt() == recordId) {
                QItemSelectionModel *selectionModel = ui->itemList_tableView->selectionModel();
                selectionModel->clearSelection();
                selectionModel->select(model->index(row, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                m_currentRecordId = recordId; // Zaktualizuj bieżące ID
                // Ręcznie wywołaj odświeżenie zdjęć
                QItemSelection selected;
                selected.select(model->index(row, 0), model->index(row, model->columnCount() - 1));
                onTableViewSelectionChanged(selected, QItemSelection());
                break;
            }
        }
    }
}
