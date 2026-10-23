#include "AiEnrichmentService.h"
#include "SecretStore.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

namespace {

constexpr const char *kAnthropicEndpoint = "https://api.anthropic.com/v1/messages";
constexpr const char *kAnthropicVersion  = "2023-06-01";
// Po odmowie filtrów bezpieczeństwa API samo ponawia zapytanie na modelu zalecanym dla
// kategorii odmowy (`fallbacks: "default"`). Haiku 5.5 tego nie obsługuje.
constexpr const char *kFallbackBeta      = "server-side-fallback-2026-07-01";

QString settingsKey()    { return QStringLiteral("ai/anthropic_api_key"); }  // tylko migracja z ≤ 1.5.0
QString settingsModel()  { return QStringLiteral("ai/model"); }
QString keychainKey()    { return QStringLiteral("anthropic_api_key"); }

QSettings appSettings()
{
    return QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                         + QStringLiteral("/inwentaryzacja.ini"),
                     QSettings::IniFormat);
}

/// Jeden odczyt sejfu na proces dla hasApiKey()/readApiKey(): każdy PreviewDialog pytałby
/// sejf od nowa, a przy aplikacji podpisanej ad hoc macOS może przy tym pytać usera.
struct ApiKeyCache
{
    bool loaded = false;
    QString key;
    QString error;
};

ApiKeyCache &apiKeyCache()
{
    static ApiKeyCache cache;
    return cache;
}

QString cachedApiKey(QString *errorMessage)
{
    ApiKeyCache &cache = apiKeyCache();
    if (!cache.loaded)
        AiEnrichmentService::storedApiKey();  // wypełnia cache
    if (errorMessage)
        *errorMessage = cache.error;
    return cache.key;
}

/// v1.5 #4: Polecenie AI generowane wg języka apki (QLocale).
/// Polski/angielski na start. Każdy nowy język = dodanie case w switch.
/// Output AI też w danym języku — Claude rozumie polecenie polskie i odpowiada po polsku.
QString systemPromptPolish(const QString &existingDescription,
                            const QString &itemName,
                            const QString &itemVendor,
                            const QString &itemModel)
{
    QString prompt = QStringLiteral(
        "Jesteś ekspertem od retro-komputerów (Amiga, Atari, Commodore, ZX Spectrum, "
        "early IBM PC, konsole 8/16 bit). Twoim zadaniem jest wzbogacić opis eksponatu "
        "muzealnego na podstawie zdjęć i istniejącego kontekstu.\n\n"
        "**Format odpowiedzi:** czysty markdown, 3-5 akapitów, polski. Bez nagłówka "
        "(tytuł jest osobno). Możesz używać **bold**, *italic*, list. Nie dodawaj "
        "metadanych ani komentarzy o procesie — tylko gotowy opis.\n\n"
        "**Jeśli istnieje opis** — zachowaj jego sens, dodaj kontekst historyczny, "
        "specyfikację techniczną, charakterystyczne cechy widoczne na zdjęciach.\n"
        "**Jeśli brak opisu** — wygeneruj pełny opis od zera z danych + zdjęć.\n\n"
        "## Kontekst eksponatu\n");

    if (!itemName.isEmpty())
        prompt += QStringLiteral("- **Nazwa:** %1\n").arg(itemName);
    if (!itemVendor.isEmpty())
        prompt += QStringLiteral("- **Producent:** %1\n").arg(itemVendor);
    if (!itemModel.isEmpty())
        prompt += QStringLiteral("- **Model:** %1\n").arg(itemModel);

    if (!existingDescription.trimmed().isEmpty())
    {
        prompt += QStringLiteral("\n## Istniejący opis (do wzbogacenia)\n\n");
        prompt += existingDescription;
    }
    else
    {
        prompt += QStringLiteral("\n## Istniejący opis\n\n_(brak — wygeneruj od zera)_\n");
    }
    return prompt;
}

QString systemPromptEnglish(const QString &existingDescription,
                             const QString &itemName,
                             const QString &itemVendor,
                             const QString &itemModel)
{
    QString prompt = QStringLiteral(
        "You are a retro-computer expert (Amiga, Atari, Commodore, ZX Spectrum, "
        "early IBM PC, 8/16-bit consoles). Your task is to enrich a museum exhibit "
        "description based on photos and existing context.\n\n"
        "**Response format:** clean markdown, 3-5 paragraphs, English. No header "
        "(title is separate). You can use **bold**, *italic*, lists. Do not add "
        "metadata or process comments — only the finished description.\n\n"
        "**If a description exists** — preserve its meaning, add historical context, "
        "technical specifications, characteristic features visible in photos.\n"
        "**If no description** — generate full description from scratch using meta + photos.\n\n"
        "## Exhibit context\n");

    if (!itemName.isEmpty())
        prompt += QStringLiteral("- **Name:** %1\n").arg(itemName);
    if (!itemVendor.isEmpty())
        prompt += QStringLiteral("- **Vendor:** %1\n").arg(itemVendor);
    if (!itemModel.isEmpty())
        prompt += QStringLiteral("- **Model:** %1\n").arg(itemModel);

    if (!existingDescription.trimmed().isEmpty())
    {
        prompt += QStringLiteral("\n## Existing description (to enrich)\n\n");
        prompt += existingDescription;
    }
    else
    {
        prompt += QStringLiteral("\n## Existing description\n\n_(none — generate from scratch)_\n");
    }
    return prompt;
}

QString systemPrompt(const QString &existingDescription,
                     const QString &itemName,
                     const QString &itemVendor,
                     const QString &itemModel)
{
    // Język wg QLocale aplikacji. Domyślnie polski.
    // Dodawanie nowych: switch case na QLocale::Language.
    const QLocale::Language lang = QLocale().language();
    if (lang == QLocale::English)
        return systemPromptEnglish(existingDescription, itemName, itemVendor, itemModel);
    // TODO: niemiecki, hiszpański, etc. wg planu wielu języków
    return systemPromptPolish(existingDescription, itemName, itemVendor, itemModel);
}

QString detectImageMediaType(const QByteArray &blob)
{
    // Magic bytes — najczęstsze formaty
    if (blob.startsWith("\xFF\xD8\xFF"))         return QStringLiteral("image/jpeg");
    if (blob.startsWith("\x89PNG\r\n\x1A\n"))    return QStringLiteral("image/png");
    if (blob.startsWith("GIF87a") || blob.startsWith("GIF89a"))
                                                  return QStringLiteral("image/gif");
    if (blob.size() >= 12 && blob.mid(8, 4) == "WEBP")
                                                  return QStringLiteral("image/webp");
    return QStringLiteral("image/jpeg");  // fallback — Anthropic akceptuje, ale może rzucić błąd
}

}  // namespace

AiEnrichmentService::AiEnrichmentService(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

AiEnrichmentService::~AiEnrichmentService()
{
    if (m_reply)
    {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

bool AiEnrichmentService::hasApiKey()
{
    const QString envKey = QProcessEnvironment::systemEnvironment()
                               .value(QStringLiteral("ANTHROPIC_API_KEY"));
    if (!envKey.isEmpty())
        return true;

    return !cachedApiKey(nullptr).isEmpty();
}

QString AiEnrichmentService::storedApiKey(QString *errorMessage)
{
    // Wersje ≤ 1.5.0 trzymały klucz jawnym tekstem w ini — SecretStore przenosi go do sejfu.
    const SecretStore::ReadResult stored = SecretStore::read(keychainKey(), settingsKey());
    ApiKeyCache result{true, stored.value, QString()};
    if (stored.stillInIni)
        result.error = tr("Klucz nadal leży jawnym tekstem w pliku ustawień — systemowy sejf "
                          "odmówił zapisu: %1")
                           .arg(stored.keychainError.isEmpty() ? tr("odczyt zwrotny niezgodny")
                                                               : stored.keychainError);
    else if (!stored.keychainError.isEmpty())
        result.error = tr("Systemowy sejf niedostępny: %1").arg(stored.keychainError);

    // Cache dopiero PO odczycie — pętla zdarzeń w trakcie czekania na sejf może wywołać
    // hasApiKey() z innego okna. Błąd nie nadpisuje wcześniej odczytanego klucza: chwilowa
    // odmowa sejfu przy otwarciu ustawień nie wyłącza AI do końca sesji.
    ApiKeyCache &cache = apiKeyCache();
    if (result.error.isEmpty() || !cache.loaded)
        cache = result;

    if (errorMessage)
        *errorMessage = result.error;
    return result.key;
}

bool AiEnrichmentService::storeApiKey(const QString &apiKey, QString *errorMessage)
{
    QString keychainError;
    const bool ok = SecretStore::write(keychainKey(), settingsKey(), apiKey, &keychainError);
    if (!ok)
    {
        apiKeyCache().loaded = false;  // następny odczyt sprawdzi sejf od nowa
        if (errorMessage)
        {
            *errorMessage = apiKey.isEmpty()
                ? tr("Nie udało się usunąć klucza z systemowego sejfu: %1. "
                     "Klucz nadal tam jest i AI dalej z niego korzysta.")
                      .arg(keychainError)
                : tr("Nie udało się zapisać klucza w systemowym sejfie: %1. "
                     "Klucz nie został zapisany — możesz ustawić zmienną "
                     "środowiskową ANTHROPIC_API_KEY.")
                      .arg(keychainError);
        }
        return false;
    }

    apiKeyCache() = ApiKeyCache{true, apiKey, QString()};
    return true;
}

const QList<AiEnrichmentService::Model> &AiEnrichmentService::models()
{
    // Cennik Anthropic (USD za 1M tokenów, stan 2026-10). Pierwszy wpis = domyślny model.
    static const QList<Model> list{
        {"claude-opus-5-5", QT_TRANSLATE_NOOP("AiEnrichmentService", "Claude Opus 5.5 — najlepszy"),
         4.0, 20.0, true},
        {"claude-sonnet-5-5",
         QT_TRANSLATE_NOOP("AiEnrichmentService", "Claude Sonnet 5.5 — 2× tańszy, prawie tak dobry"),
         2.0, 10.0, true},
        {"claude-haiku-5-5",
         QT_TRANSLATE_NOOP("AiEnrichmentService", "Claude Haiku 5.5 — najtańszy i najszybszy"),
         0.10, 0.50, false},
    };
    return list;
}

const AiEnrichmentService::Model &AiEnrichmentService::configuredModel()
{
    // Model spoza listy (np. claude-opus-4-7 zapisany przez 1.6.x) = domyślny: koszt pokazany
    // przed wywołaniem musi dotyczyć modelu, który naprawdę zostanie użyty.
    const Model *model = findModel(appSettings().value(settingsModel()).toString());
    return model ? *model : models().first();
}

const AiEnrichmentService::Model *AiEnrichmentService::findModel(const QString &id)
{
    for (const Model &model : models())
    {
        if (id == QLatin1String(model.id))
            return &model;
    }
    return nullptr;
}

double AiEnrichmentService::costUsd(const Model &model, qint64 inputTokens, qint64 outputTokens)
{
    return inputTokens / 1'000'000.0 * model.inputUsdPerMTok
         + outputTokens / 1'000'000.0 * model.outputUsdPerMTok;
}

double AiEnrichmentService::estimateCostUsd(const Model &model, int photoCount)
{
    // Pomiar 23.10.2026 (zdjęcie z bazy 1800×1355, wszystkie trzy modele): wejście 3594 tokeny
    // = zdjęcie ~3000 + polecenie ~600; wyjście z myśleniem 1321–2067 tokenów.
    return costUsd(model, photoCount * 3000 + 600, 2000);
}

QString AiEnrichmentService::readApiKey(QString *errorMessage) const
{
    const QString envKey = QProcessEnvironment::systemEnvironment()
                               .value(QStringLiteral("ANTHROPIC_API_KEY"));
    if (!envKey.isEmpty())
        return envKey;

    QString keychainError;
    const QString storedKey = cachedApiKey(&keychainError);
    if (!storedKey.isEmpty())
        return storedKey;

    if (errorMessage)
    {
        *errorMessage = AiEnrichmentService::tr(
            "Brak klucza API Anthropic. Ustaw zmienną środowiskową "
            "ANTHROPIC_API_KEY albo wpisz klucz w Ustawieniach (sekcja AI).");
        if (!keychainError.isEmpty())
            *errorMessage += QStringLiteral("\n") + keychainError;
    }
    return QString();
}

QByteArray AiEnrichmentService::buildRequestBody(const Model &model,
                                                  const QString &existingDescription,
                                                  const QList<QByteArray> &photoBlobs,
                                                  const QString &itemName,
                                                  const QString &itemVendor,
                                                  const QString &itemModel) const
{
    QJsonArray contentArray;

    // Zdjęcia jako image content blocks (Anthropic Vision)
    for (const QByteArray &blob : photoBlobs)
    {
        QJsonObject source;
        source.insert(QStringLiteral("type"),       QStringLiteral("base64"));
        source.insert(QStringLiteral("media_type"), detectImageMediaType(blob));
        source.insert(QStringLiteral("data"),       QString::fromLatin1(blob.toBase64()));

        QJsonObject imageBlock;
        imageBlock.insert(QStringLiteral("type"),   QStringLiteral("image"));
        imageBlock.insert(QStringLiteral("source"), source);
        contentArray.append(imageBlock);
    }

    // Tekst prompt jako ostatni block
    QJsonObject textBlock;
    textBlock.insert(QStringLiteral("type"), QStringLiteral("text"));
    textBlock.insert(QStringLiteral("text"),
                     systemPrompt(existingDescription, itemName, itemVendor, itemModel));
    contentArray.append(textBlock);

    QJsonObject userMessage;
    userMessage.insert(QStringLiteral("role"),    QStringLiteral("user"));
    userMessage.insert(QStringLiteral("content"), contentArray);

    QJsonArray messages;
    messages.append(userMessage);

    QJsonObject root;
    root.insert(QStringLiteral("model"),      QString::fromLatin1(model.id));
    // Modele 5.5 zawsze myślą, a myślenie liczy się do max_tokens — 1500 z 1.6.x ucinałoby opis.
    root.insert(QStringLiteral("max_tokens"), 16000);
    // Jawnie: domyślny poziom różni się między modelami (Opus/Haiku 5.5 medium, Sonnet 5.5 high).
    root.insert(QStringLiteral("output_config"),
                QJsonObject{{QStringLiteral("effort"), QStringLiteral("medium")}});
    if (model.serverFallback)
        root.insert(QStringLiteral("fallbacks"), QStringLiteral("default"));
    root.insert(QStringLiteral("messages"),   messages);

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

void AiEnrichmentService::enrichDescription(const QString &existingDescription,
                                             const QList<QByteArray> &photoBlobs,
                                             const QString &itemName,
                                             const QString &itemVendor,
                                             const QString &itemModel)
{
    if (m_reply)
    {
        emit enrichmentError(tr("Inny request AI jest już w toku — odczekaj lub anuluj."));
        return;
    }

    QString errorMessage;
    const QString apiKey = readApiKey(&errorMessage);
    if (apiKey.isEmpty())
    {
        emit enrichmentError(errorMessage);
        return;
    }

    if (photoBlobs.isEmpty())
    {
        emit enrichmentError(tr("Brak zdjęć eksponatu — AI potrzebuje obrazu do analizy."));
        return;
    }
    if (photoBlobs.size() > 5)
    {
        emit enrichmentError(tr("Za dużo zdjęć (%1). Anthropic limit: 5 obrazów per request. "
                                "Wybierz maksymalnie 5.").arg(photoBlobs.size()));
        return;
    }

    const Model &model = configuredModel();
    const QByteArray body = buildRequestBody(model, existingDescription, photoBlobs,
                                             itemName, itemVendor, itemModel);

    QNetworkRequest request(QUrl(QString::fromLatin1(kAnthropicEndpoint)));
    request.setRawHeader("x-api-key",         apiKey.toUtf8());
    request.setRawHeader("anthropic-version", kAnthropicVersion);
    request.setRawHeader("content-type",      "application/json");
    if (model.serverFallback)
        request.setRawHeader("anthropic-beta", kFallbackBeta);
    // Odpowiedź przychodzi w całości po zakończeniu generowania (z myśleniem) — zapas na wolniejsze odpowiedzi.
    request.setTransferTimeout(180'000);

    m_reply = m_nam->post(request, body);
    connect(m_reply, &QNetworkReply::finished, this, &AiEnrichmentService::onReplyFinished);
}

void AiEnrichmentService::onReplyFinished()
{
    if (!m_reply)
        return;

    QNetworkReply *reply = m_reply;
    m_reply = nullptr;

    const QByteArray rawBody = reply->readAll();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError netErr = reply->error();
    reply->deleteLater();

    if (netErr != QNetworkReply::NoError && httpStatus < 400)
    {
        // Czysty błąd sieci (timeout, DNS, etc.) — nie mamy nawet HTTP response
        emit enrichmentError(tr("Błąd sieci podczas zapytania do AI: %1").arg(reply->errorString()));
        return;
    }

    const ParsedResponse parsed = parseResponse(httpStatus, rawBody);
    if (!parsed.error.isEmpty())
    {
        emit enrichmentError(parsed.error);
        return;
    }
    emit enrichmentReady(parsed.description, parsed.inputTokens, parsed.outputTokens, parsed.model);
}

AiEnrichmentService::ParsedResponse AiEnrichmentService::parseResponse(int httpStatus,
                                                                       const QByteArray &body)
{
    ParsedResponse result;
    const QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject())
    {
        result.error = tr("AI zwróciło niepoprawną odpowiedź (nie-JSON, HTTP %1).").arg(httpStatus);
        return result;
    }
    const QJsonObject root = doc.object();

    if (httpStatus >= 400)
    {
        // Anthropic API error format: {"type": "error", "error": {"type": ..., "message": ...}}
        const QJsonObject err = root.value(QStringLiteral("error")).toObject();
        result.error = tr("Błąd Anthropic API (HTTP %1, %2): %3")
                           .arg(httpStatus)
                           .arg(err.value(QStringLiteral("type")).toString(),
                                err.value(QStringLiteral("message")).toString());
        return result;
    }

    // Odmowa (filtry bezpieczeństwa albo sam model) przychodzi jako HTTP 200 — przed treścią.
    const QString stopReason = root.value(QStringLiteral("stop_reason")).toString();
    if (stopReason == QLatin1String("refusal"))
    {
        const QString category = root.value(QStringLiteral("stop_details")).toObject()
                                     .value(QStringLiteral("category")).toString();
        result.error = category.isEmpty()
            ? tr("AI odmówiło opisania eksponatu.")
            : tr("AI odmówiło opisania eksponatu (kategoria: %1).").arg(category);
        return result;
    }
    if (stopReason == QLatin1String("max_tokens"))
    {
        result.error = tr("AI przerwało odpowiedź w połowie (limit tokenów) — opis byłby niepełny.");
        return result;
    }

    // Modele 5.5 zawsze myślą: odpowiedź zaczyna się blokami `thinking` (bez treści), a gdy po
    // odmowie odpowiadał model zastępczy, przed jego tekstem stoi blok `fallback`. Opis = tekst
    // po ostatnim bloku `fallback` (fragment odrzuconego modelu przed nim się nie liczy).
    QString description;
    for (const QJsonValue &blockValue : root.value(QStringLiteral("content")).toArray())
    {
        const QJsonObject block = blockValue.toObject();
        const QString type = block.value(QStringLiteral("type")).toString();
        if (type == QLatin1String("fallback"))
            description.clear();
        else if (type == QLatin1String("text"))
            description += block.value(QStringLiteral("text")).toString();
    }
    if (description.trimmed().isEmpty())
    {
        result.error = tr("AI zwróciło pusty opis.");
        return result;
    }

    const QJsonObject usage = root.value(QStringLiteral("usage")).toObject();
    result.description  = description;
    result.model        = root.value(QStringLiteral("model")).toString();
    result.inputTokens  = usage.value(QStringLiteral("input_tokens")).toVariant().toLongLong();
    result.outputTokens = usage.value(QStringLiteral("output_tokens")).toVariant().toLongLong();
    return result;
}
