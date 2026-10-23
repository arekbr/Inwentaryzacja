#include "AiEnrichmentService.h"

#include <QCoreApplication>
#include <QEventLoop>
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

#include <qtkeychain/keychain.h>

namespace {

constexpr const char *kAnthropicEndpoint = "https://api.anthropic.com/v1/messages";
constexpr const char *kAnthropicVersion  = "2023-06-01";
constexpr const char *kDefaultModel      = "claude-opus-4-7";

QString settingsKey()    { return QStringLiteral("ai/anthropic_api_key"); }  // tylko migracja z ≤ 1.5.0
QString settingsModel()  { return QStringLiteral("ai/model"); }
QString keychainKey()    { return QStringLiteral("anthropic_api_key"); }

QString keychainService()
{
    // Testy (QStandardPaths w trybie testowym) nie mogą ruszać prawdziwego wpisu usera.
    return QStandardPaths::isTestModeEnabled() ? QStringLiteral("Inwentaryzacja-test")
                                               : QStringLiteral("Inwentaryzacja");
}

QSettings appSettings()
{
    return QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                         + QStringLiteral("/inwentaryzacja.ini"),
                     QSettings::IniFormat);
}

/// QtKeychain działa asynchronicznie, a wołający (okno ustawień, PreviewDialog) potrzebują
/// wyniku od razu — czekamy na lokalnej pętli zdarzeń. Bez limitu czasu: porzucone zadanie
/// blokowałoby kolejkę QtKeychain (jedno zadanie naraz), a macOS może czekać na decyzję usera.
QKeychain::Error runKeychainJob(QKeychain::Job &job, QString *errorMessage)
{
    job.setAutoDelete(false);
    job.setInsecureFallback(false);  // nigdy jawnego tekstu zamiast sejfu
    job.setKey(keychainKey());
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();  // start() kolejkuje doStart — finished nie przyjdzie przed exec()
    loop.exec();
    if (job.error() != QKeychain::NoError && job.error() != QKeychain::EntryNotFound && errorMessage)
        *errorMessage = job.errorString();
    return job.error();
}

QString readKeychain(QString *errorMessage)
{
    QKeychain::ReadPasswordJob job(keychainService());
    return runKeychainJob(job, errorMessage) == QKeychain::NoError ? job.textData() : QString();
}

bool writeKeychain(const QString &value, QString *errorMessage)
{
    QKeychain::WritePasswordJob job(keychainService());
    job.setTextData(value);
    return runKeychainJob(job, errorMessage) == QKeychain::NoError;
}

bool deleteKeychain(QString *errorMessage)
{
    QKeychain::DeletePasswordJob job(keychainService());
    const QKeychain::Error error = runKeychainJob(job, errorMessage);
    return error == QKeychain::NoError || error == QKeychain::EntryNotFound;
}

void removeLegacySettingsKey()
{
    QSettings s = appSettings();
    if (s.contains(settingsKey()))  // bez contains() remove() przepisywałby ini przy każdym odczycie
        s.remove(settingsKey());
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
    ApiKeyCache result{true, QString(), QString()};

    // Wersje ≤ 1.5.0 trzymały klucz jawnym tekstem w ini. Kasujemy go z ini dopiero po
    // zgodnym odczycie zwrotnym z sejfu — awaria sejfu nie może zgubić klucza.
    QSettings s = appSettings();
    const QString legacyKey = s.value(settingsKey()).toString();
    if (!legacyKey.isEmpty())
    {
        QString keychainError;
        if (writeKeychain(legacyKey, &keychainError) && readKeychain(&keychainError) == legacyKey)
        {
            removeLegacySettingsKey();
        }
        else
        {
            result.error = tr("Klucz nadal leży jawnym tekstem w pliku ustawień — systemowy sejf "
                              "odmówił zapisu: %1")
                               .arg(keychainError.isEmpty() ? tr("odczyt zwrotny niezgodny") : keychainError);
        }
        result.key = legacyKey;  // apka działa dalej jak w 1.5.0, nawet gdy sejf odmówił
    }
    else
    {
        removeLegacySettingsKey();  // pusty wpis zostawiony przez 1.5.0
        QString keychainError;
        result.key = readKeychain(&keychainError);
        if (!keychainError.isEmpty())
            result.error = tr("Systemowy sejf niedostępny: %1").arg(keychainError);
    }

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
    const bool ok = apiKey.isEmpty() ? deleteKeychain(&keychainError)
                                     : writeKeychain(apiKey, &keychainError);

    // Stary klucz z ini wygrałby przy następnym starcie i nadpisał nowy (migracja) — usuwamy
    // go po udanym zapisie, a przy usuwaniu klucza zawsze (user chce, żeby klucza nie było).
    if (ok || apiKey.isEmpty())
        removeLegacySettingsKey();

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

double AiEnrichmentService::estimateCostUsd(int photoCount)
{
    // Opus 4.7: $5/1M input tokens, $25/1M output tokens (cached 2026-04).
    // Photo ~1500 tokens (Anthropic standard), prompt ~500, output ~800.
    const int inputTokens  = photoCount * 1500 + 500;
    const int outputTokens = 800;
    return (inputTokens / 1'000'000.0) * 5.0
         + (outputTokens / 1'000'000.0) * 25.0;
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

QByteArray AiEnrichmentService::buildRequestBody(const QString &existingDescription,
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

    QSettings s = appSettings();
    const QString model = s.value(settingsModel(), QString::fromLatin1(kDefaultModel)).toString();

    QJsonObject root;
    root.insert(QStringLiteral("model"),      model);
    root.insert(QStringLiteral("max_tokens"), 1500);
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

    const QByteArray body = buildRequestBody(existingDescription, photoBlobs,
                                             itemName, itemVendor, itemModel);

    QNetworkRequest request(QUrl(QString::fromLatin1(kAnthropicEndpoint)));
    request.setRawHeader("x-api-key",         apiKey.toUtf8());
    request.setRawHeader("anthropic-version", kAnthropicVersion);
    request.setRawHeader("content-type",      "application/json");
    // 60s timeout — Opus z vision często 15-30s
    request.setTransferTimeout(60'000);

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

    const QJsonDocument doc = QJsonDocument::fromJson(rawBody);
    if (!doc.isObject())
    {
        emit enrichmentError(tr("AI zwróciło niepoprawną odpowiedź (nie-JSON, HTTP %1).").arg(httpStatus));
        return;
    }
    const QJsonObject root = doc.object();

    if (httpStatus >= 400)
    {
        // Anthropic API error format: {"type": "error", "error": {"type": ..., "message": ...}}
        const QJsonObject err = root.value(QStringLiteral("error")).toObject();
        const QString errType    = err.value(QStringLiteral("type")).toString();
        const QString errMessage = err.value(QStringLiteral("message")).toString();
        emit enrichmentError(tr("Błąd Anthropic API (HTTP %1, %2): %3")
                             .arg(httpStatus).arg(errType, errMessage));
        return;
    }

    // Success path — parsuj content[0].text
    const QJsonArray content = root.value(QStringLiteral("content")).toArray();
    if (content.isEmpty())
    {
        emit enrichmentError(tr("AI zwróciło pustą odpowiedź (brak content blocks)."));
        return;
    }
    const QJsonObject firstBlock = content.first().toObject();
    if (firstBlock.value(QStringLiteral("type")).toString() != QStringLiteral("text"))
    {
        emit enrichmentError(tr("AI zwróciło nieoczekiwany typ bloku (%1).")
                             .arg(firstBlock.value(QStringLiteral("type")).toString()));
        return;
    }

    const QString newDescription = firstBlock.value(QStringLiteral("text")).toString();
    if (newDescription.trimmed().isEmpty())
    {
        emit enrichmentError(tr("AI zwróciło pusty opis."));
        return;
    }

    const QJsonObject usage = root.value(QStringLiteral("usage")).toObject();
    const qint64 inputTokens  = usage.value(QStringLiteral("input_tokens")).toVariant().toLongLong();
    const qint64 outputTokens = usage.value(QStringLiteral("output_tokens")).toVariant().toLongLong();

    emit enrichmentReady(newDescription, inputTokens, outputTokens);
}
