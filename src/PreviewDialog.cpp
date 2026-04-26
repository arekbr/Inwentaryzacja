#include "PreviewDialog.h"
#include "ui_PreviewDialog.h"

#include "AiEnrichmentService.h"
#include "EnrichPreviewDialog.h"
#include "ItemRepository.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QProgressDialog>
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

    // v1.5: dynamicznie dodany guzik "Wzbogać opis AI" (nie ruszamy .ui).
    // Pojawia się tylko gdy klucz API dostępny (env var lub QSettings).
    if (AiEnrichmentService::hasApiKey())
    {
        auto *enrichBtn = new QPushButton(tr("🪄 Wzbogać opis AI"), this);
        enrichBtn->setStyleSheet(QStringLiteral(
            "background-color: #1976d2; color: white; padding: 6px 12px; "
            "border-radius: 4px; font-weight: bold;"));
        // Insert na początku button row (przed Edit)
        if (auto *row = qobject_cast<QHBoxLayout *>(ui->editButton->parentWidget()->layout()))
            row->insertWidget(0, enrichBtn);
        connect(enrichBtn, &QPushButton::clicked, this, &PreviewDialog::onEnrichClicked);
    }

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

    // v1.5: cache dla AI call
    m_currentName = name;
    m_currentVendor = vendor;
    m_currentModel = model;
    m_currentDescription = description;
}

QList<QByteArray> PreviewDialog::fetchPhotos(int limit) const
{
    QList<QByteArray> photos;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT photo FROM photos WHERE eksponat_id = :id LIMIT %1").arg(limit));
    q.bindValue(QStringLiteral(":id"), m_recordId);
    if (!q.exec())
    {
        qWarning() << "PreviewDialog::fetchPhotos: SQL error" << q.lastError().text();
        return photos;
    }
    while (q.next())
    {
        const QByteArray blob = q.value(0).toByteArray();
        if (!blob.isEmpty())
            photos.append(blob);
    }
    return photos;
}

void PreviewDialog::onEnrichClicked()
{
    // 1) Pobierz zdjęcia (max 5 — Anthropic limit)
    const QList<QByteArray> photos = fetchPhotos(/*limit*/ 5);
    if (photos.isEmpty())
    {
        QMessageBox::warning(this, tr("Brak zdjęć"),
                             tr("Eksponat nie ma zdjęć w bazie. AI potrzebuje obrazu do analizy."));
        return;
    }

    // 2) Cost estimate confirm
    const double cost = AiEnrichmentService::estimateCostUsd(photos.size());
    const QString costStr = QString::number(cost, 'f', 4);
    const auto reply = QMessageBox::question(this, tr("Wzbogać opis AI"),
        tr("Wyślę %1 zdjęć do Claude Opus 4.7. Szacunkowy koszt: ~$%2.\n\nKontynuować?")
            .arg(photos.size()).arg(costStr),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reply != QMessageBox::Yes)
        return;

    // 3) Busy dialog (modal, nieanullowalny — abort wymaga reply abort)
    if (m_busyDialog)
        m_busyDialog->deleteLater();
    m_busyDialog = new QProgressDialog(tr("Pytam AI...\n\n(Claude Opus z vision: 15-30 s)"),
                                        QString(), 0, 0, this);
    m_busyDialog->setWindowModality(Qt::WindowModal);
    m_busyDialog->setMinimumDuration(0);
    m_busyDialog->setCancelButton(nullptr);
    m_busyDialog->show();

    // 4) Service — instance-per-call
    if (m_enrichService)
        m_enrichService->deleteLater();
    m_enrichService = new AiEnrichmentService(this);
    connect(m_enrichService, &AiEnrichmentService::enrichmentReady,
            this, &PreviewDialog::onEnrichmentReady);
    connect(m_enrichService, &AiEnrichmentService::enrichmentError,
            this, &PreviewDialog::onEnrichmentError);

    m_enrichService->enrichDescription(m_currentDescription, photos,
                                        m_currentName, m_currentVendor, m_currentModel);
}

void PreviewDialog::onEnrichmentReady(const QString &newDescription, qint64 inTokens, qint64 outTokens)
{
    if (m_busyDialog)
    {
        m_busyDialog->close();
        m_busyDialog->deleteLater();
        m_busyDialog = nullptr;
    }

    EnrichPreviewDialog preview(m_currentDescription, newDescription, inTokens, outTokens, this);
    if (preview.exec() != QDialog::Accepted)
        return;

    // User accepted — UPDATE description w bazie
    ItemRepository repo(m_db);
    QString errorMessage;
    if (!repo.updateDescription(m_recordId, preview.newDescription(), &errorMessage))
    {
        QMessageBox::critical(this, tr("Błąd zapisu"),
                              tr("Nie udało się zapisać nowego opisu:\n\n%1").arg(errorMessage));
        return;
    }

    // Reload preview — descriptionView pokaże nowy markdown
    loadRecord();
    QMessageBox::information(this, tr("Zapisano"),
                             tr("Nowy opis został zapisany w bazie."));
}

void PreviewDialog::onEnrichmentError(const QString &message)
{
    if (m_busyDialog)
    {
        m_busyDialog->close();
        m_busyDialog->deleteLater();
        m_busyDialog = nullptr;
    }
    QMessageBox::critical(this, tr("Błąd AI"), message);
}
