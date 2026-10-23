#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QByteArray>
#include <QDialog>
#include <QList>
#include <QSqlDatabase>
#include <QString>

namespace Ui
{
    class PreviewDialog;
}

class AiEnrichmentService;

class PreviewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreviewDialog(QSqlDatabase db, const QString &recordId, QWidget *parent = nullptr);
    ~PreviewDialog();

signals:
    void editRequested(const QString &recordId);
    /// Opis zapisany w bazie (np. po „Wzbogać opis AI”) — lista odświeża wiersz i panel opisu.
    void descriptionChanged(const QString &recordId);

private slots:
    /// v1.5: handler "🪄 Wzbogać opis AI"
    void onEnrichClicked();
    void onEnrichmentReady(const QString &newDescription, qint64 inTokens, qint64 outTokens,
                           const QString &model);
    void onEnrichmentError(const QString &message);

private:
    void loadRecord();
    void loadPhotos();
    QList<QByteArray> fetchPhotos() const;

    Ui::PreviewDialog *ui;
    QSqlDatabase m_db;
    QString m_recordId;

    // v1.5: cache meta z loadRecord do AI call (uniknij re-SELECT przy enrich)
    QString m_currentName;
    QString m_currentVendor;
    QString m_currentModel;
    QString m_currentDescription;
    QList<QByteArray> m_photoBlobs;  // wszystkie zdjęcia z bazy — także te bez miniatury (dla AI)

    AiEnrichmentService *m_enrichService = nullptr;
    class QProgressDialog *m_busyDialog = nullptr;
};

#endif // PREVIEWDIALOG_H
