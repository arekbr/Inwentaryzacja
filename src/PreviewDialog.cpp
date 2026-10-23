#include "PreviewDialog.h"
#include "ui_PreviewDialog.h"

#include "AiEnrichmentService.h"
#include "EnrichPreviewDialog.h"
#include "ItemRepository.h"
#include "fullscreenphotoviewer.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
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
    // Pojawia się tylko gdy klucz API dostępny (env var lub systemowy sejf).
    if (AiEnrichmentService::hasApiKey())
    {
        auto *enrichBtn = new QPushButton(tr("🪄 Wzbogać opis AI"), this);
        enrichBtn->setStyleSheet(QStringLiteral(
            "background-color: #1976d2; color: white; padding: 6px 12px; "
            "border-radius: 4px; font-weight: bold;"));
        // Insert na początku button row (przed Edit). Układ wprost z .ui — rodzic przycisku Edytuj
        // to okno z układem PIONOWYM, więc rzutowanie jego layout() na QHBoxLayout dawało nullptr
        // i przycisk wisiał bez układu, ucięty w lewym górnym rogu (1.5.0–1.6.0).
        ui->buttonLayout->insertWidget(0, enrichBtn);
        connect(enrichBtn, &QPushButton::clicked, this, &PreviewDialog::onEnrichClicked);
    }

    connect(ui->photosList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item)
            {
                // Pełne zdjęcie dekodowane dopiero teraz — w pozycji trzymamy bajty JPEG/PNG,
                // nie zdekodowany obraz (zdjęcie z aparatu to kilkadziesiąt MB w pamięci).
                QPixmap original;
                if (original.loadFromData(item->data(Qt::UserRole).toByteArray()))
                    (new FullScreenPhotoViewer(original, this))->show();  // WA_DeleteOnClose
            });

    loadRecord();
    loadPhotos();
}

PreviewDialog::~PreviewDialog()
{
    delete ui;
}

void PreviewDialog::loadRecord()
{
    // Do wczytania (i przy błędzie odczytu) pola pokazują „—”, nie puste miejsca.
    for (QLabel *label : {ui->valueType, ui->valueVendor, ui->valueModel, ui->valueSerial,
                          ui->valuePartNumber, ui->valueRevision, ui->valueYear, ui->valueStatus,
                          ui->valueStorage, ui->valuePackaging})
        label->setText(QStringLiteral("—"));

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

    // Wszystkie pola tylko do odczytu; puste jako „—”, żeby było widać, że pole istnieje.
    const auto showField = [](QLabel *label, const QString &value)
    {
        label->setText(value.isEmpty() ? QStringLiteral("—") : value);
    };
    showField(ui->valueType, type);
    showField(ui->valueVendor, vendor);
    showField(ui->valueModel, model);
    showField(ui->valueSerial, serial);
    showField(ui->valuePartNumber, partNum);
    showField(ui->valueRevision, revision);
    showField(ui->valueYear, year > 0 ? QString::number(year) : QString());
    showField(ui->valueStatus, status);
    showField(ui->valueStorage, storage);
    ui->valuePackaging->setText(origPack ? tr("Tak") : tr("Nie"));

    ui->descriptionView->setMarkdown(description);

    // v1.5: cache dla AI call
    m_currentName = name;
    m_currentVendor = vendor;
    m_currentModel = model;
    m_currentDescription = description;
}

void PreviewDialog::loadPhotos()
{
    // Ta sama ścieżka SQL co zdjęcia dla AI (fetchPhotos). Zdekodowana zostaje tylko miniatura.
    ui->photosList->clear();
    for (const QByteArray &blob : fetchPhotos())
    {
        QPixmap pixmap;
        if (!pixmap.loadFromData(blob))
        {
            qWarning() << "PreviewDialog: nie można odczytać zdjęcia eksponatu" << m_recordId;
            continue;
        }
        const QPixmap thumbnail = pixmap.scaled(ui->photosList->iconSize(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation);
        auto *item = new QListWidgetItem(QIcon(thumbnail), QString(), ui->photosList);
        item->setData(Qt::UserRole, blob);
    }
    ui->photosList->setVisible(ui->photosList->count() > 0);
}

QList<QByteArray> PreviewDialog::fetchPhotos(int limit) const
{
    QList<QByteArray> photos;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT photo FROM photos WHERE eksponat_id = :id")
              + (limit > 0 ? QStringLiteral(" LIMIT %1").arg(limit) : QString()));
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

    // 2) Potwierdzenie kosztu — z opcją "nie pytaj ponownie" w QSettings
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                           + QStringLiteral("/inwentaryzacja.ini"),
                       QSettings::IniFormat);
    if (!settings.value(QStringLiteral("ai/skip_cost_confirm"), false).toBool())
    {
        const double cost = AiEnrichmentService::estimateCostUsd(photos.size());
        const QString costStr = QString::number(cost, 'f', 4);

        // Własny dialog (QMessageBox::question nie ma natywnie checkbox "nie pytaj ponownie")
        QMessageBox box(QMessageBox::Question, tr("Wzbogać opis AI"),
                        tr("Wyślę %1 zdjęć do Claude. Szacunkowy koszt: ~$%2.\n\nKontynuować?")
                            .arg(photos.size()).arg(costStr),
                        QMessageBox::Yes | QMessageBox::No, this);
        box.setDefaultButton(QMessageBox::Yes);
        QCheckBox dontAsk(tr("Nie pytaj ponownie (można włączyć z powrotem w Ustawieniach)"), &box);
        box.setCheckBox(&dontAsk);
        if (box.exec() != QMessageBox::Yes)
            return;
        if (dontAsk.isChecked())
            settings.setValue(QStringLiteral("ai/skip_cost_confirm"), true);
    }

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
    emit descriptionChanged(m_recordId);
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
