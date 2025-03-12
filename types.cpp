#include "types.h"
#include "ui_types.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include "mainwindow.h"

typy::typy(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::typy),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Dodaj nowy typ"));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &typy::onSaveClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

typy::~typy()
{
    delete ui;
}

void typy::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void typy::onSaveClicked()
{
    QString typeName = ui->type_lineEdit->text().trimmed();
    if (typeName.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa typu nie może być pusta."));
        return;
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO types (name) VALUES (:name)");
    query.bindValue(":name", typeName);
    if (query.exec()) {
        if (m_mainWindow) {
            m_mainWindow->loadComboBoxData("types", m_mainWindow->getNewItemTypeComboBox());
            m_mainWindow->setEditMode(m_mainWindow->getEditMode(), m_mainWindow->getRecordId());
        }
        accept();
    } else {
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać typu:\n%1").arg(query.lastError().text()));
    }
}
