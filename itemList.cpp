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

    ui->itemList_tableView->setModel(model);
    ui->itemList_tableView->resizeColumnsToContents();
    ui->itemList_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemList_tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // Połączenie przycisków
    connect(ui->itemList_pushButton_new, &QPushButton::clicked, this, &itemList::onNewButtonClicked);
    connect(ui->itemList_pushButton_edit, &QPushButton::clicked, this, &itemList::onEditButtonClicked);
    connect(ui->itemList_pushButton_end, &QPushButton::clicked, this, &itemList::onEndButtonClicked);

    // Reakcja na zmianę zaznaczenia w tabeli
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

    // Ładowanie zdjęć
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
                QItemSelection selected;
                selected.select(model->index(row, 0), model->index(row, model->columnCount() - 1));
                onTableViewSelectionChanged(selected, QItemSelection());
                break;
            }
        }
    }
}
