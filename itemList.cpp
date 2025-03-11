#include "itemList.h"
#include "ui_itemList.h"   // Plik generowany z itemsList.ui
#include "mainwindow.h"    // Okno formularza dodawania nowego eksponatu

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlError>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

itemList::itemList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::itemList)
{
    ui->setupUi(this);

    // Używamy domyślnego połączenia z bazą lub tworzymy nowe, jeśli jeszcze nie istnieje
    QSqlDatabase db;
    if (QSqlDatabase::contains("default_connection"))
        db = QSqlDatabase::database("default_connection");
    else {
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

    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();

    // Połączenie przycisku "Nowy" z otwarciem formularza dodawania nowego rekordu
    connect(ui->pushButton, &QPushButton::clicked, this, &itemList::onNewButtonClicked);
}

itemList::~itemList()
{
    delete ui;
}

void itemList::onNewButtonClicked()
{
    // Utworzenie okna formularza do dodawania nowego eksponatu
    MainWindow *addWindow = new MainWindow();
    addWindow->setAttribute(Qt::WA_DeleteOnClose);
    addWindow->show();
}
