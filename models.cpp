#include "models.h"
#include "ui_models.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"

models::models(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::models),
    m_mainWindow(nullptr),
    m_vendorId(-1)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie modelami sprzętu"));

    // Połączenie przycisków (przyciski Dodaj, Edytuj, Skasuj w widoku, a przycisk OK w QDialogButtonBox)
    connect(ui->pushButton_add, &QPushButton::clicked, this, &models::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &models::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &models::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &models::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

models::~models()
{
    delete ui;
}

void models::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void models::setVendorId(int vendorId)
{
    m_vendorId = vendorId;
}

void models::refreshList()
{
    // Pobieramy tylko kolumnę z nazwami modeli, aby QListView pokazywał same nazwy
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM models ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                      .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

void models::onAddClicked()
{
    QString newModel = ui->lineEdit->text().trimmed();
    if(newModel.isEmpty()){
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa modelu nie może być pusta."));
        return;
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO models (name, vendor_id) VALUES (:name, :vendor_id)");
    query.bindValue(":name", newModel);
    int vendorId = (m_vendorId != -1) ? m_vendorId :
                       (m_mainWindow ? m_mainWindow->getNewItemVendorComboBox()->currentData().toInt() : -1);
    query.bindValue(":vendor_id", vendorId);
    if(!query.exec()){
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać modelu:\n%1")
                                                      .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

void models::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać model do edycji."));
        return;
    }
    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj model"), tr("Nowa nazwa:"), QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.trimmed().isEmpty()){
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM models WHERE name = :name");
        query.bindValue(":name", currentName);
        if(query.exec() && query.next()){
            int id = query.value(0).toInt();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE models SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if(!updateQuery.exec()){
                QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować modelu:\n%1")
                                                              .arg(updateQuery.lastError().text()));
            }
        }
    }
    refreshList();
}

void models::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać model do usunięcia."));
        return;
    }
    QString modelName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć model: %1?").arg(modelName),
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes){
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM models WHERE name = :name");
        query.bindValue(":name", modelName);
        if(!query.exec()){
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć modelu:\n%1")
                                                          .arg(query.lastError().text()));
        }
    }
    refreshList();
}

void models::onOkClicked()
{
    // Po kliknięciu OK – odśwież pole modeli w głównym oknie (jeśli ustawiono główne okno)
    if(m_mainWindow) {
        m_mainWindow->loadComboBoxData("models", m_mainWindow->getNewItemModelComboBox());
    }
    accept();
}
