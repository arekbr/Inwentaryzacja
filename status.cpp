#include "status.h"
#include "ui_status.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"

status::status(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::status),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie statusami"));

    connect(ui->pushButton_add, &QPushButton::clicked, this, &status::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &status::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &status::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &status::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

status::~status()
{
    delete ui;
}

void status::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void status::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM statuses ORDER BY name ASC", m_db);
    if(queryModel->lastError().isValid()){
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

void status::onAddClicked()
{
    QString newStatus = ui->lineEdit->text().trimmed();
    if(newStatus.isEmpty()){
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa statusu nie może być pusta."));
        return;
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO statuses (name) VALUES (:name)");
    query.bindValue(":name", newStatus);
    if(!query.exec()){
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać statusu:\n%1")
                                                    .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

void status::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać status do edycji."));
        return;
    }
    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj status"), tr("Nowa nazwa:"), QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.trimmed().isEmpty()){
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM statuses WHERE name = :name");
        query.bindValue(":name", currentName);
        if(query.exec() && query.next()){
            int id = query.value(0).toInt();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE statuses SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if(!updateQuery.exec()){
                QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować statusu:\n%1")
                                                            .arg(updateQuery.lastError().text()));
            }
        }
    }
    refreshList();
}

void status::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać status do usunięcia."));
        return;
    }
    QString statusName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć status: %1?").arg(statusName),
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes){
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM statuses WHERE name = :name");
        query.bindValue(":name", statusName);
        if(!query.exec()){
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć statusu:\n%1")
                                                        .arg(query.lastError().text()));
        }
    }
    refreshList();
}

void status::onOkClicked()
{
    if(m_mainWindow) {
        m_mainWindow->loadComboBoxData("statuses", m_mainWindow->getNewItemStatusComboBox());
    }
    accept();
}
