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
    m_currentRecordId(-1)
{
    ui->setupUi(this);

    // Użycie domyślnego połączenia lub utworzenie nowego, jeśli nie istnieje
    QSqlDatabase db;
    if (QSqlDatabase::contains("default_connection"))
        db = QSqlDatabase::database("default_connection");
    else {
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
                                  tr("Nie udało się otworzyć bazy danych:\n%1")
                                      .arg(db.lastError().text()));
            return;
        }
    }

    // Ustawienie modelu SQL dla tabeli "eksponaty"
    model = new QSqlTableModel(this, db);
    model->setTable("eksponaty");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    // Ustawienie przyjaznych nagłówków dla użytkownika
    // Kolumna indeks 0 (ID) oraz 13 (image_path) nie są potrzebne
    model->setHeaderData(1, Qt::Horizontal, tr("Nazwa"));
    model->setHeaderData(2, Qt::Horizontal, tr("Typ"));
    model->setHeaderData(3, Qt::Horizontal, tr("Producent"));
    model->setHeaderData(4, Qt::Horizontal, tr("Model"));
    model->setHeaderData(5, Qt::Horizontal, tr("Numer seryjny"));
    model->setHeaderData(6, Qt::Horizontal, tr("Part number"));
    model->setHeaderData(7, Qt::Horizontal, tr("Revision"));
    model->setHeaderData(8, Qt::Horizontal, tr("Rok produkcji"));
    model->setHeaderData(9, Qt::Horizontal, tr("Status"));
    model->setHeaderData(10, Qt::Horizontal, tr("Miejsce przechowywania"));
    model->setHeaderData(11, Qt::Horizontal, tr("Opis"));
    model->setHeaderData(12, Qt::Horizontal, tr("Ilość"));
    // Kolumna 0 (ID) i 13 (image_path) ukrywamy:
    ui->itemList_tableView->setModel(model);
    ui->itemList_tableView->resizeColumnsToContents();
    ui->itemList_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemList_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemList_tableView->hideColumn(0);
    ui->itemList_tableView->hideColumn(13);

    // Połączenie przycisków
    connect(ui->itemList_pushButton_new, &QPushButton::clicked, this, &itemList::onNewButtonClicked);
    connect(ui->itemList_pushButton_edit, &QPushButton::clicked, this, &itemList::onEditButtonClicked);
    connect(ui->itemList_pushButton_end, &QPushButton::clicked, this, &itemList::onEndButtonClicked);
    connect(ui->itemList_pushButton_delete, &QPushButton::clicked, this, &itemList::onDeleteButtonClicked);

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
    MainWindow *addWindow = new MainWindow(this);
    addWindow->setAttribute(Qt::WA_DeleteOnClose);
    addWindow->setEditMode(false);
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
    int recordId = model->data(model->index(index.row(), 0)).toInt();
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

    // Pobierz pierwszy zaznaczony wiersz
    QModelIndexList selected = selModel->selectedRows();
    if (selected.isEmpty())
        return;

    int row = selected.first().row();
    // Zakładamy, że kolumna 0 zawiera ID rekordu
    int recordId = model->data(model->index(row, 0)).toInt();

    // Potwierdzenie usunięcia
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
            refreshList();  // Odśwież widok po usunięciu
            QMessageBox::information(this, tr("Sukces"), tr("Rekord został usunięty."));
        }
    }
}

void itemList::onEndButtonClicked()
{
    qApp->quit();
}

void itemList::onRecordSaved(int recordId)
{
    refreshList(recordId);
}

void itemList::onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    if (selected.indexes().isEmpty()) {
        ui->itemList_graphicsView->setScene(nullptr);
        m_currentRecordId = -1;
        return;
    }
    QModelIndex index = selected.indexes().first();
    int row = index.row();
    m_currentRecordId = model->data(model->index(row, 0)).toInt();

    // Ładowanie zdjęć (kod pozostaje bez zmian)
    QSqlQuery query(QSqlDatabase::database("default_connection"));
    query.prepare("SELECT photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", m_currentRecordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęć:" << query.lastError().text();
        ui->itemList_graphicsView->setScene(nullptr);
        return;
    }
    QGraphicsScene *scene = new QGraphicsScene(this);
    const int thumbnailSize = 80;
    const int spacing = 5;
    int x = 5, y = 5;
    while (query.next()) {
        QByteArray imageData = query.value("photo").toByteArray();
        QPixmap pixmap;
        if (!pixmap.loadFromData(imageData)) {
            qDebug() << "Nie można załadować zdjęcia BLOB.";
            continue;
        }
        QPixmap scaled = pixmap.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QGraphicsPixmapItem *item = scene->addPixmap(scaled);
        item->setPos(x, y);
        x += thumbnailSize + spacing;
        if (x + thumbnailSize > ui->itemList_graphicsView->width() - 10) {
            x = 5;
            y += thumbnailSize + spacing;
        }
    }
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
    model->select();
    ui->itemList_tableView->resizeColumnsToContents();
    if (recordId != -1) {
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex idx = model->index(row, 0);
            if (model->data(idx).toInt() == recordId) {
                QItemSelectionModel *sel = ui->itemList_tableView->selectionModel();
                sel->clearSelection();
                sel->select(model->index(row, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                m_currentRecordId = recordId;
                break;
            }
        }
    }
}
