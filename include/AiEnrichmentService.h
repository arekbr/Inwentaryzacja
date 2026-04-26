#ifndef AIENRICHMENTSERVICE_H
#define AIENRICHMENTSERVICE_H

#include <QByteArray>
#include <QCoreApplication>
#include <QList>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

/// v1.5: Async wrapper na Anthropic Messages API dla "Wzbogać opis AI"
/// w PreviewDialog. Wysyła zdjęcia eksponatu + istniejący description + meta
/// jako kontekst, dostaje wzbogacony markdown.
///
/// **Architektura:** A z RFC v1.5 (direct HTTP, brak Python dependency,
/// brak zewnętrznego backendu). Jeśli RFC zmienia się na C (mobile backend),
/// ten klass jest jednym refaktorem swap'em.
///
/// **API key:** env var `ANTHROPIC_API_KEY` priority, fallback `QSettings`
/// klucz `ai/anthropic_api_key` (z ostrzeżeniem o lokalnym storage plaintext).
///
/// **Model:** Claude Opus 4.7 (`claude-opus-4-7`). Można zmienić w QSettings
/// `ai/model` (np. `claude-sonnet-4-6` dla 3× tańszej alternatywy).
///
/// **Lifetime:** instance-per-call OK (m_nam wewnętrzny). Sygnały async —
/// caller łączy do jednego call'u, po `enrichmentReady`/`enrichmentError`
/// może `deleteLater()`.
class AiEnrichmentService : public QObject
{
    Q_OBJECT  // sam daje tr() — Q_DECLARE_TR_FUNCTIONS jest dla NON-QObject classes

public:
    explicit AiEnrichmentService(QObject *parent = nullptr);
    ~AiEnrichmentService() override;

    /// Async — emit `enrichmentReady(newDescription, usage)` lub `enrichmentError(message)`.
    /// @param existingDescription istniejący opis (może być pusty — wtedy "from scratch")
    /// @param photoBlobs zdjęcia jako JPEG/PNG bytes (max 5 zalecane, każde do 5MB raw — Anthropic limit)
    /// @param itemName  np. "Amiga 1200"
    /// @param itemVendor np. "Commodore"
    /// @param itemModel np. "1200"
    void enrichDescription(const QString &existingDescription,
                           const QList<QByteArray> &photoBlobs,
                           const QString &itemName,
                           const QString &itemVendor,
                           const QString &itemModel);

    /// Synchroniczne sprawdzenie czy klucz API jest dostępny (przed pokazaniem
    /// guzika w UI). Zwraca true gdy env var lub QSettings ma klucz.
    static bool hasApiKey();

    /// Estymata kosztu w USD przed call'em (do confirm dialog).
    /// Bardzo zgrubna: Opus 4.7 input ~$5/1M tok, output ~$25/1M tok.
    /// Photo ~1500 tokens każde, prompt ~500, output ~800 tokens.
    static double estimateCostUsd(int photoCount);

signals:
    void enrichmentReady(const QString &newDescription, qint64 inputTokens, qint64 outputTokens);
    void enrichmentError(const QString &message);

private slots:
    void onReplyFinished();

private:
    QString readApiKey(QString *errorMessage) const;
    QByteArray buildRequestBody(const QString &existingDescription,
                                const QList<QByteArray> &photoBlobs,
                                const QString &itemName,
                                const QString &itemVendor,
                                const QString &itemModel) const;

    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply = nullptr;  // jeden in-flight request per service
};

#endif // AIENRICHMENTSERVICE_H
