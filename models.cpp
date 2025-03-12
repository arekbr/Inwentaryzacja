#include "models.h"
#include "ui_models.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include "mainwindow.h"

models::models(QWidget *parent)
    : QDialog(parent) // Zamiast QWidget
    , ui(new Ui::models)
    , m_mainWindow(nullptr)
    , m_vendorId(-1)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");

    // Ustawiamy tytuł okna
    setWindowTitle(tr("Dodaj nowy model"));

    // Połączenie sygnału z slotem
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &models::onSaveClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
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

void models::onSaveClicked()
{
    QString modelName = ui->lineEdit->text().trimmed();
    if (modelName.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa modelu nie może być pusta."));
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO models (name, vendor_id) VALUES (:name, :vendor_id)");
    query.bindValue(":name", modelName);
    query.bindValue(":vendor_id", m_vendorId != -1 ? m_vendorId : (m_mainWindow ? m_mainWindow->getNewItemVendorComboBox()->currentData().toInt() : -1));
    if (query.exec()) {
        if (m_mainWindow) {
            m_mainWindow->loadComboBoxData("models", m_mainWindow->getNewItemModelComboBox());
            m_mainWindow->setEditMode(m_mainWindow->getEditMode(), m_mainWindow->getRecordId());
        }
        accept(); // Zamyka dialog z sukcesem
    } else {
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać modelu:\n%1").arg(query.lastError().text()));
    }
}
