#include "PreviewDialog.h"
#include "ui_PreviewDialog.h"

#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

PreviewDialog::PreviewDialog(QSqlDatabase db, const QString &recordId, QWidget *parent)
    : QDialog(parent), ui(new Ui::PreviewDialog), m_db(db), m_recordId(recordId)
{
    ui->setupUi(this);

    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->editButton, &QPushButton::clicked, this, [this]()
            {
                emit editRequested(m_recordId);
                accept();
            });

    loadRecord();
}

PreviewDialog::~PreviewDialog()
{
    delete ui;
}

void PreviewDialog::loadRecord()
{
    QSqlQuery q(m_db);
    q.prepare(
        "SELECT eksponaty.name, eksponaty.serial_number, eksponaty.part_number, "
        "       eksponaty.revision, eksponaty.production_year, eksponaty.description, "
        "       eksponaty.has_original_packaging, "
        "       types.name AS type_name, vendors.name AS vendor_name, "
        "       models.name AS model_name, statuses.name AS status_name, "
        "       storage_places.name AS storage_name "
        "FROM eksponaty "
        "LEFT JOIN types ON eksponaty.type_id = types.id "
        "LEFT JOIN vendors ON eksponaty.vendor_id = vendors.id "
        "LEFT JOIN models ON eksponaty.model_id = models.id "
        "LEFT JOIN statuses ON eksponaty.status_id = statuses.id "
        "LEFT JOIN storage_places ON eksponaty.storage_place_id = storage_places.id "
        "WHERE eksponaty.id = :id");
    q.bindValue(":id", m_recordId);

    if (!q.exec() || !q.next())
    {
        qWarning() << "PreviewDialog: nie udało się wczytać rekordu" << m_recordId
                   << q.lastError().text();
        ui->nameLabel->setText(tr("(brak rekordu)"));
        return;
    }

    const QString name = q.value("name").toString();
    const QString vendor = q.value("vendor_name").toString();
    const QString model = q.value("model_name").toString();
    const QString type = q.value("type_name").toString();
    const int year = q.value("production_year").toInt();
    const QString serial = q.value("serial_number").toString();
    const QString partNum = q.value("part_number").toString();
    const QString revision = q.value("revision").toString();
    const QString status = q.value("status_name").toString();
    const QString storage = q.value("storage_name").toString();
    const bool origPack = q.value("has_original_packaging").toBool();
    const QString description = q.value("description").toString();

    ui->nameLabel->setText(name.isEmpty() ? tr("(bez nazwy)") : name);

    QStringList metaParts;
    if (!vendor.isEmpty())
        metaParts << vendor;
    if (!model.isEmpty())
        metaParts << model;
    if (!type.isEmpty())
        metaParts << type;
    if (year > 0)
        metaParts << QString::number(year);
    ui->metaLabel->setText(metaParts.join(QStringLiteral(" · ")));

    QStringList detailParts;
    if (!serial.isEmpty())
        detailParts << tr("S/N: %1").arg(serial);
    if (!partNum.isEmpty())
        detailParts << tr("P/N: %1").arg(partNum);
    if (!revision.isEmpty())
        detailParts << tr("Rev: %1").arg(revision);
    if (!status.isEmpty())
        detailParts << tr("Status: %1").arg(status);
    if (!storage.isEmpty())
        detailParts << tr("Miejsce: %1").arg(storage);
    if (origPack)
        detailParts << tr("Oryginalne opakowanie");
    ui->detailsLabel->setText(detailParts.join(QStringLiteral("  |  ")));

    ui->descriptionView->setMarkdown(description);
}
