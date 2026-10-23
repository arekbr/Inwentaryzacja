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
/// **API key:** env var `ANTHROPIC_API_KEY` priority, fallback systemowy sejf
/// (QtKeychain: Pęk kluczy macOS / Menedżer poświadczeń Windows / Secret Service
/// Linux), usługa `Inwentaryzacja`, klucz `anthropic_api_key`. Wersje ≤ 1.5.0
/// trzymały klucz jawnym tekstem w ini (`ai/anthropic_api_key`) — przenoszony
/// do sejfu przy pierwszym odczycie.
///
/// **Model:** lista w `models()`, domyślnie Claude Opus 5.5 (`claude-opus-5-5`); wybór
/// w QSettings `ai/model` (okno ustawień).
///
/// **Lifetime:** instance-per-call OK (m_nam wewnętrzny). Sygnały async —
/// caller łączy do jednego call'u, po `enrichmentReady`/`enrichmentError`
/// może `deleteLater()`.
class AiEnrichmentService : public QObject
{
    Q_OBJECT  // sam daje tr() — Q_DECLARE_TR_FUNCTIONS jest dla NON-QObject classes

public:
    struct Model
    {
        const char *id;
        const char *name;  ///< do listy w ustawieniach (QT_TRANSLATE_NOOP, kontekst AiEnrichmentService)
        double inputUsdPerMTok;
        double outputUsdPerMTok;
        bool serverFallback;  ///< `fallbacks: "default"` po odmowie — Haiku 5.5 go nie obsługuje
    };

    /// Wynik odpowiedzi Messages API: opis albo komunikat błędu dla usera.
    struct ParsedResponse
    {
        QString description;
        QString model;  ///< model, który odpowiedział (po odmowie może to być model zastępczy)
        qint64 inputTokens = 0;
        qint64 outputTokens = 0;
        QString error;
    };

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
    /// guzika w UI). Zwraca true gdy env var lub systemowy sejf ma klucz.
    static bool hasApiKey();

    /// Klucz z systemowego sejfu (bez zmiennej środowiskowej) — dla okna ustawień.
    /// Przy okazji przenosi klucz z ini (wersje ≤ 1.5.0) do sejfu. Pusty wynik
    /// i pusty errorMessage = klucza nie ma; niepusty errorMessage = problem z sejfem.
    static QString storedApiKey(QString *errorMessage = nullptr);

    /// Zapisuje klucz w systemowym sejfie, pusty klucz usuwa wpis. Nigdy nie pisze
    /// klucza do ini. false + errorMessage, gdy sejf odmówił.
    static bool storeApiKey(const QString &apiKey, QString *errorMessage = nullptr);

    /// Modele do wyboru w ustawieniach z cennikiem; pierwszy jest domyślny.
    static const QList<Model> &models();

    /// Model z ustawień `ai/model`; spoza listy (np. z 1.6.x) = domyślny.
    static const Model &configuredModel();

    /// Wpis z models() o danym ID albo nullptr (np. model zastępczy spoza listy).
    static const Model *findModel(const QString &id);

    static double costUsd(const Model &model, qint64 inputTokens, qint64 outputTokens);

    /// Zgrubna estymata kosztu w USD przed wywołaniem (okno potwierdzenia, lista modeli).
    static double estimateCostUsd(const Model &model, int photoCount);

    /// Opis z odpowiedzi Messages API: czyta bloki po typie (thinking/fallback/text),
    /// obsługuje odmowę i ucięcie na limicie tokenów. Czysta funkcja — bez sieci.
    static ParsedResponse parseResponse(int httpStatus, const QByteArray &body);

signals:
    void enrichmentReady(const QString &newDescription, qint64 inputTokens, qint64 outputTokens,
                         const QString &model);
    void enrichmentError(const QString &message);

private slots:
    void onReplyFinished();

private:
    QString readApiKey(QString *errorMessage) const;
    QByteArray buildRequestBody(const Model &model,
                                const QString &existingDescription,
                                const QList<QByteArray> &photoBlobs,
                                const QString &itemName,
                                const QString &itemVendor,
                                const QString &itemModel) const;

    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply = nullptr;  // jeden in-flight request per service
};

#endif // AIENRICHMENTSERVICE_H
