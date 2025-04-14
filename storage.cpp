#include "storage.h"
#include "ui_storage.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid>

storage::storage(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::storage),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie miejscami przechowywania"));

    connect(ui->pushButton_add, &QPushButton::clicked, this, &storage::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &storage::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &storage::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &storage::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

storage::~storage()
{
    delete ui;
}

void storage::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void storage::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM storage_places ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

void storage::onAddClicked()
{
    QString newStorage = ui->lineEdit->text().trimmed(); // Zakładam nazwę 'lineEdit'
    if (newStorage.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa miejsca przechowywania nie może być pusta."));
        return;
    }

    QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO storage_places (id, name) VALUES (:id, :name)");
    query.bindValue(":id", QUuid::createUuid().toString());
    query.bindValue(":name", newStorage);
    if (!query.exec()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać miejsca przechowywania: %1")
                                                    .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear(); // Zakładam nazwę 'lineEdit'
}

void storage::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać miejsce przechowywania do edycji."));
        return;
    }
    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj miejsce przechowywania"), tr("Nowa nazwa:"), QLineEdit::Normal, currentName, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM storage_places WHERE name = :name");
        query.bindValue(":name", currentName);
        if (query.exec() && query.next()) {
            QString id = query.value("id").toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE storage_places SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if (!updateQuery.exec()) {
                QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować miejsca przechowywania: %1")
                                                            .arg(updateQuery.lastError().text()));
            }
        }
    }
    refreshList();
}

void storage::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać miejsce przechowywania do usunięcia."));
        return;
    }
    QString storageName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć miejsce przechowywania: %1?").arg(storageName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM storage_places WHERE name = :name");
        query.bindValue(":name", storageName);
        if (!query.exec()) {
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć miejsca przechowywania: %1")
                                                        .arg(query.lastError().text()));
        }
    }
    refreshList();
}

void storage::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("storage_places", m_mainWindow->getNewItemStoragePlaceComboBox());
    }
    accept();
}
