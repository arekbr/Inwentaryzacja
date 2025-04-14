#include "vendors.h"
#include "ui_vendors.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid> // dodane

vendors::vendors(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::vendors),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie producentami"));

    // Połączenie przycisków
    connect(ui->pushButton_add, &QPushButton::clicked, this, &vendors::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &vendors::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &vendors::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &vendors::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

vendors::~vendors()
{
    delete ui;
}

void vendors::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void vendors::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM vendors ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

void vendors::onAddClicked()
{
    QString newVendor = ui->lineEdit->text().trimmed();
    if(newVendor.isEmpty()){
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa producenta nie może być pusta."));
        return;
    }
    // Generujemy UUID
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO vendors (id, name) VALUES (:id, :name)");
    query.bindValue(":id", newId);
    query.bindValue(":name", newVendor);
    if(!query.exec()){
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać producenta:\n%1")
                                                    .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

void vendors::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać producenta do edycji."));
        return;
    }
    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj producenta"), tr("Nowa nazwa:"),
                                            QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.trimmed().isEmpty()){
        // Odczytujemy ID
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM vendors WHERE name = :name");
        query.bindValue(":name", currentName);
        if(query.exec() && query.next()){
            QString id = query.value(0).toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE vendors SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if(!updateQuery.exec()){
                QMessageBox::critical(this, tr("Błąd"),
                                      tr("Nie udało się zaktualizować producenta:\n%1")
                                          .arg(updateQuery.lastError().text()));
            }
        }
    }
    refreshList();
}

void vendors::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać producenta do usunięcia."));
        return;
    }
    QString vendorName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć producenta: %1?").arg(vendorName),
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes){
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM vendors WHERE name = :name");
        query.bindValue(":name", vendorName);
        if(!query.exec()){
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć producenta:\n%1")
                                                        .arg(query.lastError().text()));
        }
    }
    refreshList();
}

void vendors::onOkClicked()
{
    if(m_mainWindow) {
        m_mainWindow->loadComboBoxData("vendors", m_mainWindow->getNewItemVendorComboBox());
    }
    accept();
}
