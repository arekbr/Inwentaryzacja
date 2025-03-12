#include "vendors.h"
#include "ui_vendors.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include "mainwindow.h"

vendors::vendors(QWidget *parent)
    : QDialog(parent) // Zamiast QWidget
    , ui(new Ui::vendors)
    , m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");

    // Ustawiamy tytuł okna
    setWindowTitle(tr("Dodaj nowego producenta"));

    // Połączenie sygnału z slotem
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &vendors::onSaveClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

vendors::~vendors()
{
    delete ui;
}

void vendors::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void vendors::onSaveClicked()
{
    QString vendorName = ui->lineEdit->text().trimmed();
    if (vendorName.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa producenta nie może być pusta."));
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO vendors (name) VALUES (:name)");
    query.bindValue(":name", vendorName);
    if (query.exec()) {
        if (m_mainWindow) {
            m_mainWindow->loadComboBoxData("vendors", m_mainWindow->getNewItemVendorComboBox());
            m_mainWindow->setEditMode(m_mainWindow->getEditMode(), m_mainWindow->getRecordId());
        }
        accept(); // Zamyka dialog z sukcesem
    } else {
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać producenta:\n%1").arg(query.lastError().text()));
    }
}
