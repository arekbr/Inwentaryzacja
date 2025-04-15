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
    m_db = QSqlDatabase::database("default_connection"); // MySQL
    setWindowTitle(tr("Zarządzanie typami sprzętu"));

    connect(ui->pushButton_add,    &QPushButton::clicked, this, &typy::onAddClicked);
    connect(ui->pushButton_edit,   &QPushButton::clicked, this, &typy::onEditClicked);
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
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("SELECT name FROM types ORDER BY name ASC", m_db);
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania (MySQL): %1")
                                                    .arg(model->lastError().text()));
        return;
    }
    ui->listView->setModel(model);
    ui->listView->setModelColumn(0);
}

void typy::onAddClicked()
{
    QString txt = ui->type_lineEdit->text().trimmed();
    if (txt.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa typu nie może być pusta."));
        return;
    }
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO types (id, name) VALUES (:id, :name)");
    q.bindValue(":id", uuid);
    q.bindValue(":name", txt);

    if (!q.exec()) {
        QMessageBox::critical(this, tr("Błąd"),
                              tr("Nie udało się dodać:\n%1").arg(q.lastError().text()));
    }
    refreshList();
    ui->type_lineEdit->clear();
}

void typy::onEditClicked()
{
    QModelIndex idx = ui->listView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Wybierz rekord do edycji."));
        return;
    }
    QString oldName = idx.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj typ"),
                                            tr("Nowa nazwa:"),
                                            QLineEdit::Normal,
                                            oldName, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        QSqlQuery q(m_db);
        q.prepare("SELECT id FROM types WHERE name=:nm");
        q.bindValue(":nm", oldName);
        if (q.exec() && q.next()) {
            QString id = q.value(0).toString();
            QSqlQuery q2(m_db);
            q2.prepare("UPDATE types SET name=:n WHERE id=:id");
            q2.bindValue(":n", newName.trimmed());
            q2.bindValue(":id", id);
            if (!q2.exec()) {
                QMessageBox::critical(this, tr("Błąd"),
                                      tr("Nie udało się zaktualizować:\n%1")
                                          .arg(q2.lastError().text()));
            }
        }
    }
    refreshList();
}

void typy::onDeleteClicked()
{
    QModelIndex idx = ui->listView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Wybierz rekord do usunięcia."));
        return;
    }
    QString name = idx.data(Qt::DisplayRole).toString();
    auto ans = QMessageBox::question(this, tr("Potwierdzenie"),
                                     tr("Usunąć typ %1?").arg(name),
                                     QMessageBox::Yes|QMessageBox::No);
    if (ans==QMessageBox::Yes) {
        QSqlQuery q(m_db);
        q.prepare("DELETE FROM types WHERE name=:n");
        q.bindValue(":n", name);
        if (!q.exec()) {
            QMessageBox::critical(this, tr("Błąd"),
                                  tr("Nie udało się usunąć:\n%1")
                                      .arg(q.lastError().text()));
        }
    }
    refreshList();
}

void typy::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("types", m_mainWindow->getNewItemTypeComboBox());
    }
    accept();
}
