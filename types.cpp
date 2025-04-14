#include "types.h"
#include "ui_types.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid>

typy::typy(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::typy),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie typami sprzętu"));

    connect(ui->pushButton_add, &QPushButton::clicked, this, &typy::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &typy::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &typy::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &typy::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

typy::~typy()
{
    delete ui;
}

void typy::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void typy::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM types ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

void typy::onAddClicked()
{
    QString newType = ui->type_lineEdit->text().trimmed();
    if(newType.isEmpty()){
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa typu nie może być pusta."));
        return;
    }

    QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO types (id, name) VALUES (:id, :name)");
    query.bindValue(":id", QUuid::createUuid().toString());
    query.bindValue(":name", newType);
    if(!query.exec()){
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać typu: %1")
                                                    .arg(query.lastError().text()));
    }
    refreshList();
    ui->type_lineEdit->clear();
}

void typy::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać typ do edycji."));
        return;
    }
    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj typ"), tr("Nowa nazwa:"), QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.trimmed().isEmpty()){
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM types WHERE name = :name");
        query.bindValue(":name", currentName);
        if(query.exec() && query.next()){
            QString id = query.value("id").toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE types SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if(!updateQuery.exec()){
                QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować typu: %1")
                                                            .arg(updateQuery.lastError().text()));
            }
        }
    }
    refreshList();
}

void typy::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać typ do usunięcia."));
        return;
    }
    QString typeName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć typ: %1?").arg(typeName),
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes){
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM types WHERE name = :name");
        query.bindValue(":name", typeName);
        if(!query.exec()){
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć typu: %1")
                                                        .arg(query.lastError().text()));
        }
    }
    refreshList();
}

void typy::onOkClicked()
{
    if(m_mainWindow) {
        m_mainWindow->loadComboBoxData("types", m_mainWindow->getNewItemTypeComboBox());
    }
    accept();
}
