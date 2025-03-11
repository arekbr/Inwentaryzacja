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
#include <QPixmap>
#include <QApplication>

itemList::itemList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::itemList)
{
    ui->setupUi(this);

    // Użycie domyślnego połączenia z bazą lub tworzenie nowego, jeśli nie istnieje
    QSqlDatabase db;
    if (QSqlDatabase::contains("default_connection")) {
        db = QSqlDatabase::database("default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");
        db.setDatabaseName("/home/arekbr/inwentaryzacja/muzeum.db");
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
    connect(addWindow, &MainWindow::recordSaved, this, &itemList::refreshList); // Połączenie sygnału
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

    // Otwieramy MainWindow w trybie edycji
    MainWindow *editWindow = new MainWindow(this); // Ustawiamy itemList jako parent
    editWindow->setAttribute(Qt::WA_DeleteOnClose);
    editWindow->setEditMode(true, recordId);
    editWindow->show();
    connect(editWindow, &MainWindow::recordSaved, this, &itemList::refreshList); // Połączenie sygnału
}

void itemList::onEndButtonClicked()
{
    qApp->quit();
}

void itemList::onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    // Jeśli nie ma zaznaczenia – czyścimy QGraphicsView
    if (selected.indexes().isEmpty()) {
        ui->itemList_graphicsView->setScene(nullptr);
        return;
    }

    // Pobierz ID rekordu z pierwszej kolumny (id w tabeli eksponaty)
    QModelIndex index = selected.indexes().first();
    int row = index.row();
    int recordId = model->data(model->index(row, 0)).toInt();

    // Pobierz zdjęcie z tabeli photos
    QSqlQuery query(QSqlDatabase::database("default_connection"));
    query.prepare("SELECT photo FROM photos WHERE eksponat_id = :id LIMIT 1");
    query.bindValue(":id", recordId);
    if (!query.exec()) {
        qDebug() << "Błąd pobierania zdjęcia:" << query.lastError().text();
        ui->itemList_graphicsView->setScene(nullptr);
        return;
    }

    QGraphicsScene *scene = new QGraphicsScene(this);
    if (query.next()) {
        QByteArray imageData = query.value("photo").toByteArray();
        QPixmap pixmap;
        if (!pixmap.loadFromData(imageData)) {
            qDebug() << "Nie można załadować zdjęcia z danych BLOB dla rekordu" << recordId;
            ui->itemList_graphicsView->setScene(nullptr);
            delete scene;
            return;
        }

        // Pobieramy rozmiar widżetu QGraphicsView
        QSize viewSize = ui->itemList_graphicsView->size();
        // Odejmujemy marginesy (np. 10 pikseli), aby obraz nie dotykał krawędzi
        int targetWidth = viewSize.width() - 10;
        int targetHeight = viewSize.height() - 10;

        // Skalujemy obraz, zachowując proporcje (Aspect Ratio)
        QPixmap scaledPixmap = pixmap.scaled(targetWidth, targetHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Centrujemy obraz w scenie
        scene->addPixmap(scaledPixmap);
        scene->setSceneRect(0, 0, scaledPixmap.width(), scaledPixmap.height());
        ui->itemList_graphicsView->setScene(scene);

        // Dopasowujemy widok, aby obraz był w pełni widoczny
        ui->itemList_graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    } else {
        // Jeśli nie ma zdjęcia, czyścimy widok
        ui->itemList_graphicsView->setScene(nullptr);
        delete scene;
    }
}

void itemList::refreshList()
{
    // Odświeżenie danych w modelu
    model->select();
    ui->itemList_tableView->resizeColumnsToContents();
}
