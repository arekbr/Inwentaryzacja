#include <QtTest>

#include "AiEnrichmentService.h"
#include "DatabaseConfigDialog.h"
#include "DictionaryRepository.h"
#include "DatabaseMigration.h"
#include "DatabaseBackupService.h"
#include "ItemFilterProxyModel.h"
#include "ItemFormValidator.h"
#include "ItemRepository.h"
#include "PacmanAnimationModel.h"
#include "PreviewDialog.h"
#include "itemList.h"
#include "mainwindow.h"
#include "PhotoService.h"
#include "utils.h"

#include <QBuffer>
#include <QComboBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QTableView>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QScopeGuard>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUuid>

#include <qtkeychain/keychain.h>

// Klucz API ze zmiennej środowiskowej na czas testu: PreviewDialog (hasApiKey) nie sięga wtedy
// do sejfu — na macOS offscreen zadanie sejfu nigdy się nie kończy, a CI nie ma sejfu wcale.
class ScopedTestApiKey
{
public:
    ScopedTestApiKey() : m_previous(qgetenv("ANTHROPIC_API_KEY"))
    {
        qputenv("ANTHROPIC_API_KEY", "sk-ant-TEST-podglad");
    }
    ~ScopedTestApiKey()
    {
        if (m_previous.isEmpty())
            qunsetenv("ANTHROPIC_API_KEY");
        else
            qputenv("ANTHROPIC_API_KEY", m_previous);
    }

private:
    QByteArray m_previous;
};

// Wpisy sejfu testowego wprost przez QtKeychain, z pominięciem kodu aplikacji — test hasła MySQL
// ma być czerwony także na wersji, która w ogóle nie zapisuje hasła w sejfie.
static void runTestKeychainJob(QKeychain::Job &job, const QString &account)
{
    job.setAutoDelete(false);
    job.setKey(account);
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
}

static QString readTestKeychain(const QString &account)
{
    QKeychain::ReadPasswordJob job(QStringLiteral("Inwentaryzacja-test"));
    runTestKeychainJob(job, account);
    return job.error() == QKeychain::NoError ? job.textData() : QString();
}

static void deleteTestKeychain(const QString &account)
{
    QKeychain::DeletePasswordJob job(QStringLiteral("Inwentaryzacja-test"));
    runTestKeychainJob(job, account);
}

static QString testIniPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
           + QStringLiteral("/inwentaryzacja.ini");
}

// Zrzut widżetu do przeglądu wizualnego zmian UI: INW_ZRZUTY=<katalog> ./InwentaryzacjaTests
// (na platformie cocoa — offscreen nie rysuje stylu systemu). Bez zmiennej nic nie robi.
static void saveScreenshotIfRequested(QWidget &widget, const QString &name)
{
    const QString directory = qEnvironmentVariable("INW_ZRZUTY");
    if (directory.isEmpty())
        return;
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));
    widget.grab().save(directory + QLatin1Char('/') + name + QStringLiteral(".png"));
}

class RepositoryTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void itemRepository_savesRecordAndPhotos();
    void itemRepository_deletesRecordAndPhotos();
    void itemRepository_updatesExistingRecordWithoutDuplicatingPhotos();
    void itemRepository_updatesDescription();
    void itemRepository_updateDescriptionFailsForUnknownId();
    void itemRepository_bulkUpdatesStatusAndStorage();
    void dictionaryRepository_supportsCrud();
    void dictionaryRepository_addsModelWithParentVendor();
    void photoService_loadsStoredPhotos();
    void photoService_movesPhotosToDoneWhenEnabled();
    void photoService_keepsPhotosInPlaceWhenMoveDisabled();
    void databaseMigration_removesBracesFromAllRelevantTables();
    void databaseMigration_fixesKnownBrokenUuids();
    void databaseMigration_isNoOpWithoutSchema();
    void databaseBackupService_buildsSafeDumpArguments();
    void databaseBackupService_buildsArgumentsWithDefaultsExtraFile();
    void databaseBackupService_rejectsNonMySqlConnection();
    void itemFormValidator_rejectsEmptyName();
    void itemFormValidator_parsesNumericValue();
    void itemFormValidator_rejectsInvalidNumericValue();
    void itemFormValidator_requiresSelection();
    void itemFormValidator_checksModelVendorConsistency();
    void mainWindow_loadsComboBoxesOnConstruction();
    void mainWindow_setEditModeForNewRecordClearsFieldsAndDefaultsSelections();
    void mainWindow_setEditModeLoadsExistingRecord();
    void itemFilterProxyModel_searchesAcrossMultipleFields();
    void itemList_restoresSavedFilters();
    void pacmanAnimationModel_activatesAfterConfiguredDelay();
    void pacmanAnimationModel_requestsEatingInTime();
    void pacmanAnimationModel_reachesCollisionAndFinish();
    void aiApiKey_storesReadsAndDeletesInKeychain();
    void aiApiKey_migratesLegacyIniKeyToKeychain();
    void aiApiKey_storeRemovesLegacyIniKey();
    void previewDialog_enrichButtonInButtonRow();
    void previewDialog_showsPhotosAndFields();
    void itemList_showsDescriptionOfSelectedRow();
    void mysqlPassword_configDialogMigratesIniPasswordToKeychain();
    void mysqlPassword_configDialogStoresChangedPasswordInKeychain();
    void aiResponse_readsBlocksByType();
    void aiModels_unknownSavedModelFallsBackToDefault();
    void aiEnrichment_realCallReturnsDescription();
    void sqlDriver_mysqlPluginLoads();

private:
    QString keychainSkipReason() const;
    QString lookupId(const QString &tableName, const QString &name) const;
    QByteArray createPhotoBytes() const;
    ItemRecordData createSampleItem() const;

    QString m_connectionName;
    QSqlDatabase m_db;
};

void RepositoryTests::init()
{
    m_connectionName = QStringLiteral("test_connection_%1")
                           .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(QStringLiteral(":memory:"));
    QVERIFY2(m_db.open(), qPrintable(m_db.lastError().text()));
    QVERIFY(ensureDatabaseSchema(m_db));
}

void RepositoryTests::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void RepositoryTests::cleanup()
{
    const QString connectionName = m_connectionName;
    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

QString RepositoryTests::lookupId(const QString &tableName, const QString &name) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT id FROM %1 WHERE name = :name").arg(tableName));
    query.bindValue(QStringLiteral(":name"), name);
    if (!query.exec())
        return QString();
    if (!query.next())
        return QString();
    return query.value(0).toString();
}

QByteArray RepositoryTests::createPhotoBytes() const
{
    QImage image(8, 8, QImage::Format_ARGB32);
    image.fill(Qt::red);

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

ItemRecordData RepositoryTests::createSampleItem() const
{
    ItemRecordData item;
    item.name = QStringLiteral("Testowy eksponat");
    item.serialNumber = QStringLiteral("SER-001");
    item.partNumber = QStringLiteral("PART-001");
    item.revision = QStringLiteral("A");
    item.productionYear = 1988;
    item.statusId = lookupId(QStringLiteral("statuses"), QStringLiteral("Sprawny"));
    item.typeId = lookupId(QStringLiteral("types"), QStringLiteral("Komputer"));
    item.vendorId = lookupId(QStringLiteral("vendors"), QStringLiteral("Atari"));
    item.modelId = lookupId(QStringLiteral("models"), QStringLiteral("Atari 800XL"));
    item.storagePlaceId = lookupId(QStringLiteral("storage_places"), QStringLiteral("Magazyn 1"));
    item.description = QStringLiteral("Opis testowy");
    item.value = 123;
    item.hasOriginalPackaging = true;
    return item;
}

void RepositoryTests::itemRepository_savesRecordAndPhotos()
{
    ItemRepository repository(m_db);
    QString savedItemId;
    QString errorMessage;

    QVERIFY2(repository.saveItem(createSampleItem(), {createPhotoBytes()}, &savedItemId, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(!savedItemId.isEmpty());

    QSqlQuery itemQuery(m_db);
    itemQuery.prepare(QStringLiteral("SELECT name, value FROM eksponaty WHERE id = :id"));
    itemQuery.bindValue(QStringLiteral(":id"), savedItemId);
    QVERIFY(itemQuery.exec());
    QVERIFY(itemQuery.next());
    QCOMPARE(itemQuery.value(0).toString(), QStringLiteral("Testowy eksponat"));
    QCOMPARE(itemQuery.value(1).toInt(), 123);

    QSqlQuery photoQuery(m_db);
    photoQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM photos WHERE eksponat_id = :id"));
    photoQuery.bindValue(QStringLiteral(":id"), savedItemId);
    QVERIFY(photoQuery.exec());
    QVERIFY(photoQuery.next());
    QCOMPARE(photoQuery.value(0).toInt(), 1);
}

void RepositoryTests::itemRepository_deletesRecordAndPhotos()
{
    ItemRepository repository(m_db);
    QString savedItemId;
    QString errorMessage;

    QVERIFY2(repository.saveItem(createSampleItem(), {createPhotoBytes()}, &savedItemId, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY2(repository.deleteItem(savedItemId, &errorMessage), qPrintable(errorMessage));

    QSqlQuery itemQuery(m_db);
    itemQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM eksponaty WHERE id = :id"));
    itemQuery.bindValue(QStringLiteral(":id"), savedItemId);
    QVERIFY(itemQuery.exec());
    QVERIFY(itemQuery.next());
    QCOMPARE(itemQuery.value(0).toInt(), 0);

    QSqlQuery photoQuery(m_db);
    photoQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM photos WHERE eksponat_id = :id"));
    photoQuery.bindValue(QStringLiteral(":id"), savedItemId);
    QVERIFY(photoQuery.exec());
    QVERIFY(photoQuery.next());
    QCOMPARE(photoQuery.value(0).toInt(), 0);
}

void RepositoryTests::itemRepository_updatesExistingRecordWithoutDuplicatingPhotos()
{
    ItemRepository repository(m_db);
    QString savedItemId;
    QString errorMessage;

    QVERIFY2(repository.saveItem(createSampleItem(), {createPhotoBytes()}, &savedItemId, &errorMessage),
             qPrintable(errorMessage));

    ItemRecordData updatedItem = createSampleItem();
    updatedItem.id = savedItemId;
    updatedItem.name = QStringLiteral("Eksponat po edycji");
    updatedItem.value = 999;
    updatedItem.editMode = true;

    QString updatedItemId;
    QVERIFY2(repository.saveItem(updatedItem, {createPhotoBytes()}, &updatedItemId, &errorMessage),
             qPrintable(errorMessage));
    QCOMPARE(updatedItemId, savedItemId);

    QSqlQuery itemQuery(m_db);
    itemQuery.prepare(QStringLiteral("SELECT name, value FROM eksponaty WHERE id = :id"));
    itemQuery.bindValue(QStringLiteral(":id"), savedItemId);
    QVERIFY(itemQuery.exec());
    QVERIFY(itemQuery.next());
    QCOMPARE(itemQuery.value(0).toString(), QStringLiteral("Eksponat po edycji"));
    QCOMPARE(itemQuery.value(1).toInt(), 999);

    QSqlQuery photoQuery(m_db);
    photoQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM photos WHERE eksponat_id = :id"));
    photoQuery.bindValue(QStringLiteral(":id"), savedItemId);
    QVERIFY(photoQuery.exec());
    QVERIFY(photoQuery.next());
    QCOMPARE(photoQuery.value(0).toInt(), 1);
}

void RepositoryTests::itemRepository_updatesDescription()
{
    // v1.5: AI enrichment update description path.
    ItemRepository repository(m_db);
    QString savedItemId;
    QString errorMessage;

    QVERIFY2(repository.saveItem(createSampleItem(), {createPhotoBytes()}, &savedItemId, &errorMessage),
             qPrintable(errorMessage));

    const QString newDescription = QStringLiteral("# Amiga 1200\n\nKomputer firmy Commodore z 1992 roku, "
                                                  "kontynuator linii 1000/2000/500/600. **AGA chipset** "
                                                  "(8-bit color palette, 256 kolorów na ekranie).");
    QVERIFY2(repository.updateDescription(savedItemId, newDescription, &errorMessage),
             qPrintable(errorMessage));

    QSqlQuery q(m_db);
    QVERIFY(q.exec(QStringLiteral("SELECT description FROM eksponaty WHERE id = '%1'").arg(savedItemId)));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), newDescription);
}

void RepositoryTests::itemRepository_updateDescriptionFailsForUnknownId()
{
    ItemRepository repository(m_db);
    QString errorMessage;

    QVERIFY(!repository.updateDescription(QStringLiteral("non-existent-uuid"),
                                          QStringLiteral("test"),
                                          &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("nie istnieje"), Qt::CaseInsensitive));
}

void RepositoryTests::itemRepository_bulkUpdatesStatusAndStorage()
{
    ItemRepository repository(m_db);
    QString firstItemId;
    QString secondItemId;
    QString errorMessage;

    ItemRecordData firstItem = createSampleItem();
    firstItem.name = QStringLiteral("Eksponat status 1");
    QVERIFY2(repository.saveItem(firstItem, {}, &firstItemId, &errorMessage), qPrintable(errorMessage));

    ItemRecordData secondItem = createSampleItem();
    secondItem.name = QStringLiteral("Eksponat status 2");
    secondItem.serialNumber = QStringLiteral("SER-002");
    QVERIFY2(repository.saveItem(secondItem, {}, &secondItemId, &errorMessage), qPrintable(errorMessage));

    const QString missingStatusId = lookupId(QStringLiteral("statuses"), QStringLiteral("brak"));
    const QString missingStorageId = lookupId(QStringLiteral("storage_places"), QStringLiteral("brak"));
    QVERIFY(!missingStatusId.isEmpty());
    QVERIFY(!missingStorageId.isEmpty());

    const QStringList ids = {firstItemId, secondItemId};
    QVERIFY2(repository.updateStatusForItems(ids, missingStatusId, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY2(repository.updateStoragePlaceForItems(ids, missingStorageId, &errorMessage),
             qPrintable(errorMessage));

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM eksponaty WHERE id IN (:id1, :id2) AND status_id = :statusId AND storage_place_id = :storageId"));
    query.bindValue(QStringLiteral(":id1"), firstItemId);
    query.bindValue(QStringLiteral(":id2"), secondItemId);
    query.bindValue(QStringLiteral(":statusId"), missingStatusId);
    query.bindValue(QStringLiteral(":storageId"), missingStorageId);
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 2);
}

void RepositoryTests::dictionaryRepository_supportsCrud()
{
    DictionaryRepository repository(m_db);
    QString errorMessage;
    const QString typeName = QStringLiteral("Kontroler testowy");
    const QString renamedTypeName = QStringLiteral("Kontroler testowy v2");

    QVERIFY2(repository.addEntry(QStringLiteral("types"), typeName, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(!lookupId(QStringLiteral("types"), typeName).isEmpty());

    QVERIFY2(repository.renameEntry(QStringLiteral("types"), typeName, renamedTypeName, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(!lookupId(QStringLiteral("types"), renamedTypeName).isEmpty());

    QVERIFY2(repository.deleteEntry(QStringLiteral("types"), renamedTypeName, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(lookupId(QStringLiteral("types"), renamedTypeName).isEmpty());
}

void RepositoryTests::dictionaryRepository_addsModelWithParentVendor()
{
    DictionaryRepository repository(m_db);
    QString errorMessage;
    const QString vendorId = lookupId(QStringLiteral("vendors"), QStringLiteral("Atari"));
    QVERIFY(!vendorId.isEmpty());

    QVERIFY2(repository.addEntry(QStringLiteral("models"),
                                 QStringLiteral("Atari Test Model"),
                                 &errorMessage,
                                 QStringLiteral("vendor_id"),
                                 vendorId),
             qPrintable(errorMessage));

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT vendor_id FROM models WHERE name = :name"));
    query.bindValue(QStringLiteral(":name"), QStringLiteral("Atari Test Model"));
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), vendorId);
}

void RepositoryTests::photoService_loadsStoredPhotos()
{
    ItemRepository repository(m_db);
    QString savedItemId;
    QString errorMessage;

    QVERIFY2(repository.saveItem(createSampleItem(), {createPhotoBytes()}, &savedItemId, &errorMessage),
             qPrintable(errorMessage));

    PhotoService photoService(m_db);
    const QList<StoredPhoto> photos = photoService.loadStoredPhotos(savedItemId, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(photos.size(), 1);
    QVERIFY(!photos.first().id.isEmpty());
    QVERIFY(!photos.first().pixmap.isNull());
}

void RepositoryTests::photoService_movesPhotosToDoneWhenEnabled()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = tempDir.filePath(QStringLiteral("photo.png"));
    QFile file(sourcePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(createPhotoBytes());
    file.close();

    PhotoService photoService(m_db);
    const QStringList failures = photoService.movePhotosToDone({sourcePath}, true);
    QVERIFY(failures.isEmpty());
    QVERIFY(!QFileInfo::exists(sourcePath));
    QVERIFY(QFileInfo::exists(tempDir.filePath(QStringLiteral("gotowe/photo.png"))));
}

void RepositoryTests::photoService_keepsPhotosInPlaceWhenMoveDisabled()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = tempDir.filePath(QStringLiteral("photo.png"));
    QFile file(sourcePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(createPhotoBytes());
    file.close();

    PhotoService photoService(m_db);
    const QStringList failures = photoService.movePhotosToDone({sourcePath}, false);
    QVERIFY(failures.isEmpty());
    QVERIFY(QFileInfo::exists(sourcePath));
    QVERIFY(!QFileInfo::exists(tempDir.filePath(QStringLiteral("gotowe/photo.png"))));
}

void RepositoryTests::databaseMigration_removesBracesFromAllRelevantTables()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString dbPath = tempDir.filePath(QStringLiteral("migration.sqlite"));
    QVERIFY(setupDatabase(QStringLiteral("SQLite3"), dbPath));

    QSqlDatabase migrationDb = QSqlDatabase::database(QStringLiteral("default_connection"));
    QVERIFY(migrationDb.isOpen());

    const QString statusId = QStringLiteral("{11111111-1111-1111-1111-111111111111}");
    const QString storageId = QStringLiteral("{22222222-2222-2222-2222-222222222222}");
    const QString itemId = QStringLiteral("{33333333-3333-3333-3333-333333333333}");
    const QString photoId = QStringLiteral("{44444444-4444-4444-4444-444444444444}");

    auto lookupIdInDb = [&migrationDb](const QString &tableName, const QString &name)
    {
        QSqlQuery lookupQuery(migrationDb);
        lookupQuery.prepare(QStringLiteral("SELECT id FROM %1 WHERE name = :name").arg(tableName));
        lookupQuery.bindValue(QStringLiteral(":name"), name);
        if (!lookupQuery.exec() || !lookupQuery.next())
            return QString();
        return lookupQuery.value(0).toString();
    };

    const QString typeId = lookupIdInDb(QStringLiteral("types"), QStringLiteral("Komputer"));
    const QString vendorId = lookupIdInDb(QStringLiteral("vendors"), QStringLiteral("Atari"));
    const QString modelId = lookupIdInDb(QStringLiteral("models"), QStringLiteral("Atari 800XL"));
    QVERIFY(!typeId.isEmpty());
    QVERIFY(!vendorId.isEmpty());
    QVERIFY(!modelId.isEmpty());

    QSqlQuery query(migrationDb);
    QVERIFY(query.exec(QStringLiteral("INSERT INTO statuses (id, name) VALUES ('%1', 'Status testowy UUID')").arg(statusId)));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO storage_places (id, name) VALUES ('%1', 'Miejsce testowe UUID')").arg(storageId)));
    QVERIFY(query.exec(QStringLiteral(
        "INSERT INTO eksponaty (id, name, type_id, vendor_id, model_id, status_id, storage_place_id, value, has_original_packaging) "
        "VALUES ('%1', 'Eksponat UUID', '%2', '%3', '%4', '%5', '%6', 7, 0)")
                           .arg(itemId, typeId, vendorId, modelId, statusId, storageId)));
    QVERIFY(query.exec(QStringLiteral("INSERT INTO photos (id, eksponat_id, photo) VALUES ('%1', '%2', X'89504E470D0A1A0A')")
                           .arg(photoId, itemId)));

    {
        DatabaseMigration migration;
        QVERIFY(migration.migrateUUIDs());
    }

    QVERIFY(query.exec("SELECT COUNT(*) FROM statuses WHERE id LIKE '{%}'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    QVERIFY(query.exec("SELECT COUNT(*) FROM storage_places WHERE id LIKE '{%}'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    QVERIFY(query.exec("SELECT COUNT(*) FROM eksponaty WHERE id LIKE '{%}' OR status_id LIKE '{%}' OR storage_place_id LIKE '{%}'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    QVERIFY(query.exec("SELECT COUNT(*) FROM photos WHERE id LIKE '{%}' OR eksponat_id LIKE '{%}'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    QVERIFY(query.exec(QStringLiteral("SELECT COUNT(*) FROM eksponaty WHERE id = '%1'")
                           .arg(QString(itemId).remove('{').remove('}'))));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);

    query = QSqlQuery();
    migrationDb.close();
    migrationDb = QSqlDatabase();
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
}

void RepositoryTests::databaseMigration_fixesKnownBrokenUuids()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString dbPath = tempDir.filePath(QStringLiteral("migration-broken.sqlite"));
    QVERIFY(setupDatabase(QStringLiteral("SQLite3"), dbPath));

    QSqlDatabase migrationDb = QSqlDatabase::database(QStringLiteral("default_connection"));
    QVERIFY(migrationDb.isOpen());

    auto lookupIdInDb = [&migrationDb](const QString &tableName, const QString &name)
    {
        QSqlQuery lookupQuery(migrationDb);
        lookupQuery.prepare(QStringLiteral("SELECT id FROM %1 WHERE name = :name").arg(tableName));
        lookupQuery.bindValue(QStringLiteral(":name"), name);
        if (!lookupQuery.exec() || !lookupQuery.next())
            return QString();
        return lookupQuery.value(0).toString();
    };

    const QString typeId = lookupIdInDb(QStringLiteral("types"), QStringLiteral("Komputer"));
    const QString vendorId = lookupIdInDb(QStringLiteral("vendors"), QStringLiteral("Atari"));
    const QString modelId = lookupIdInDb(QStringLiteral("models"), QStringLiteral("Atari 800XL"));
    QVERIFY(!typeId.isEmpty());
    QVERIFY(!vendorId.isEmpty());
    QVERIFY(!modelId.isEmpty());

    QSqlQuery query(migrationDb);
    QVERIFY(query.exec("INSERT INTO statuses (id, name) VALUES ('{b0ffafe8-a847-4b6f-bf65-54096df6ade', 'brak test')"));
    QVERIFY(query.exec("INSERT INTO statuses (id, name) VALUES ('{42668524-b93e-4c88-ba04-0d0fbf7683e', 'Sprawny test')"));
    QVERIFY(query.exec("INSERT INTO storage_places (id, name) VALUES ('{c120da85-24f7-45f6-8df1-5a4245952ef', 'brak miejsce test')"));
    QVERIFY(query.exec(QStringLiteral(
        "INSERT INTO eksponaty (id, name, type_id, vendor_id, model_id, status_id, storage_place_id, value, has_original_packaging) "
        "VALUES ('test-broken-item', 'Eksponat broken UUID', '%1', '%2', '%3', '{42668524-b93e-4c88-ba04-0d0fbf7683e', '{c120da85-24f7-45f6-8df1-5a4245952ef', 5, 0)")
                           .arg(typeId, vendorId, modelId)));

    {
        DatabaseMigration migration;
        QVERIFY(migration.migrateUUIDs());
    }

    QVERIFY(query.exec("SELECT COUNT(*) FROM statuses WHERE id = 'b0ffafe8-a847-4b6f-bf65-54096df6ade'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);

    QVERIFY(query.exec("SELECT COUNT(*) FROM statuses WHERE id = '42668524-b93e-4c88-ba04-0d0fbf7683e'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);

    QVERIFY(query.exec("SELECT COUNT(*) FROM storage_places WHERE id = 'c120da85-24f7-45f6-8df1-5a4245952ef'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);

    QVERIFY(query.exec("SELECT COUNT(*) FROM eksponaty WHERE status_id = '42668524-b93e-4c88-ba04-0d0fbf7683e' AND storage_place_id = 'c120da85-24f7-45f6-8df1-5a4245952ef'"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);

    query = QSqlQuery();
    migrationDb.close();
    migrationDb = QSqlDatabase();
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
}

void RepositoryTests::databaseMigration_isNoOpWithoutSchema()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QSqlDatabase emptyDb = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("default_connection"));
    emptyDb.setDatabaseName(QStringLiteral(":memory:"));
    QVERIFY2(emptyDb.open(), qPrintable(emptyDb.lastError().text()));

    {
        DatabaseMigration migration;
        QVERIFY(migration.migrateUUIDs());
    }

    emptyDb.close();
    emptyDb = QSqlDatabase();
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
}

void RepositoryTests::databaseBackupService_buildsSafeDumpArguments()
{
    MySqlConnectionInfo connectionInfo;
    connectionInfo.host = QStringLiteral("db.example.com");
    connectionInfo.database = QStringLiteral("retrodb");
    connectionInfo.user = QStringLiteral("arek");
    connectionInfo.port = 3307;

    const QStringList arguments = DatabaseBackupService::buildDumpArguments(connectionInfo);
    QVERIFY(arguments.contains(QStringLiteral("--single-transaction")));
    QVERIFY(arguments.contains(QStringLiteral("--quick")));
    QVERIFY(arguments.contains(QStringLiteral("--hex-blob")));
    QVERIFY(arguments.contains(QStringLiteral("--host=db.example.com")));
    QVERIFY(arguments.contains(QStringLiteral("--port=3307")));
    QVERIFY(arguments.contains(QStringLiteral("--user=arek")));
    QCOMPARE(arguments.last(), QStringLiteral("retrodb"));
}

void RepositoryTests::databaseBackupService_buildsArgumentsWithDefaultsExtraFile()
{
    // E-3 (audit 2026-04-26): defaults-extra-file scenariusz — user/password
    // sa w pliku, NIE w args. Bezpieczny security-wise.
    MySqlConnectionInfo connectionInfo;
    connectionInfo.host = QStringLiteral("db.example.com");
    connectionInfo.database = QStringLiteral("retrodb");
    connectionInfo.user = QStringLiteral("arek");
    connectionInfo.port = 3307;

    const QString defaultsPath = QStringLiteral("/tmp/test-mysqldump.cnf");
    const QStringList arguments =
        DatabaseBackupService::buildDumpArguments(connectionInfo, defaultsPath);

    // --defaults-extra-file MUSI byc PIERWSZYM argumentem (mysql convention)
    QCOMPARE(arguments.first(),
             QStringLiteral("--defaults-extra-file=/tmp/test-mysqldump.cnf"));
    // Standardowe args nadal obecne
    QVERIFY(arguments.contains(QStringLiteral("--single-transaction")));
    QVERIFY(arguments.contains(QStringLiteral("--host=db.example.com")));
    QVERIFY(arguments.contains(QStringLiteral("--port=3307")));
    // --user= NIE w args (jest w defaults file, byloby duplicate)
    QVERIFY(!arguments.contains(QStringLiteral("--user=arek")));
    QCOMPARE(arguments.last(), QStringLiteral("retrodb"));
}

void RepositoryTests::databaseBackupService_rejectsNonMySqlConnection()
{
    // E-5 (audit 2026-04-26): SQLite teraz MA backup (VACUUM INTO + gzip) —
    // ten test zmienil sens. Stary main test setup uzywa ':memory:' (nie da
    // sie backup'owac), wiec tworzymy osobny SQLite na pliku.
    const QString sqlitePath = QDir::tempPath() + QStringLiteral("/inwentaryzacja-test-source.db");
    QFile::remove(sqlitePath);
    const QString outputPath = QDir::tempPath() + QStringLiteral("/inwentaryzacja-test-sqlite-backup.sql.gz");
    QFile::remove(outputPath);

    const QString connName = QStringLiteral("test-sqlite-backup-source");
    {
        QSqlDatabase fileDb = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
        fileDb.setDatabaseName(sqlitePath);
        QVERIFY2(fileDb.open(), qPrintable(fileDb.lastError().text()));
        QVERIFY(ensureDatabaseSchema(fileDb));

        DatabaseBackupService backupService(fileDb);
        QString errorMessage;
        DatabaseBackupService::BackupResult result;

        QVERIFY2(backupService.backupToGzipFile(outputPath, &errorMessage, &result),
                 qPrintable(errorMessage));
        QVERIFY(QFile::exists(outputPath));
        QVERIFY(result.compressedBytes > 0);
        QVERIFY(result.uncompressedBytes > 0);
        QVERIFY(result.gzipVerified);

        fileDb.close();
    }
    QSqlDatabase::removeDatabase(connName);

    QFile::remove(sqlitePath);
    QFile::remove(outputPath);

    // Plus: in-memory SQLite NIE da się backup'ować — test rejection
    DatabaseBackupService memoryBackup(m_db);
    QString memErr;
    QVERIFY(!memoryBackup.backupToGzipFile(outputPath, &memErr));
    QVERIFY(memErr.contains(QStringLiteral("memory"), Qt::CaseInsensitive));
}

void RepositoryTests::itemFormValidator_rejectsEmptyName()
{
    const ItemValidationResult result = ItemFormValidator::validateName(QStringLiteral("   "));
    QVERIFY(!result.isValid);
    QCOMPARE(result.field, ItemValidationField::Name);
}

void RepositoryTests::itemFormValidator_parsesNumericValue()
{
    const ItemValidationResult emptyResult = ItemFormValidator::validateNumericValue(QString());
    QVERIFY(emptyResult.isValid);
    QCOMPARE(emptyResult.parsedValue, 0);

    const ItemValidationResult parsedResult = ItemFormValidator::validateNumericValue(QStringLiteral(" 42 "));
    QVERIFY(parsedResult.isValid);
    QCOMPARE(parsedResult.parsedValue, 42);
}

void RepositoryTests::itemFormValidator_rejectsInvalidNumericValue()
{
    const ItemValidationResult result = ItemFormValidator::validateNumericValue(QStringLiteral("4a2"));
    QVERIFY(!result.isValid);
    QCOMPARE(result.field, ItemValidationField::Value);
}

void RepositoryTests::itemFormValidator_requiresSelection()
{
    const ItemValidationResult result =
        ItemFormValidator::validateSelection(QString(), QStringLiteral("Status"), ItemValidationField::Status);
    QVERIFY(!result.isValid);
    QCOMPARE(result.field, ItemValidationField::Status);
}

void RepositoryTests::itemFormValidator_checksModelVendorConsistency()
{
    const QString atariVendorId = lookupId(QStringLiteral("vendors"), QStringLiteral("Atari"));
    const QString commodoreVendorId = lookupId(QStringLiteral("vendors"), QStringLiteral("Commodore"));
    const QString atariModelId = lookupId(QStringLiteral("models"), QStringLiteral("Atari 800XL"));
    QVERIFY(!atariVendorId.isEmpty());
    QVERIFY(!commodoreVendorId.isEmpty());
    QVERIFY(!atariModelId.isEmpty());

    const ItemValidationResult validResult =
        ItemFormValidator::validateModelVendorConsistency(m_db, atariVendorId, atariModelId);
    QVERIFY(validResult.isValid);

    const ItemValidationResult invalidResult =
        ItemFormValidator::validateModelVendorConsistency(m_db, commodoreVendorId, atariModelId);
    QVERIFY(!invalidResult.isValid);
    QCOMPARE(invalidResult.field, ItemValidationField::Model);
}

void RepositoryTests::mainWindow_loadsComboBoxesOnConstruction()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString dbPath = tempDir.filePath(QStringLiteral("mainwindow.sqlite"));
    QVERIFY(setupDatabase(QStringLiteral("SQLite3"), dbPath));

    {
        MainWindow window;
        QVERIFY(window.getNewItemTypeComboBox());
        QVERIFY(window.getNewItemVendorComboBox());
        QVERIFY(window.getNewItemModelComboBox());
        QVERIFY(window.getNewItemStatusComboBox());
        QVERIFY(window.getNewItemStoragePlaceComboBox());

        QVERIFY(window.getNewItemTypeComboBox()->count() > 0);
        QVERIFY(window.getNewItemVendorComboBox()->count() > 0);
        QVERIFY(window.getNewItemModelComboBox()->count() > 0);
        QVERIFY(window.getNewItemStatusComboBox()->count() > 0);
        QVERIFY(window.getNewItemStoragePlaceComboBox()->count() > 0);
    }

    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("default_connection"));
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
}

void RepositoryTests::mainWindow_setEditModeForNewRecordClearsFieldsAndDefaultsSelections()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString dbPath = tempDir.filePath(QStringLiteral("mainwindow-new.sqlite"));
    QVERIFY(setupDatabase(QStringLiteral("SQLite3"), dbPath));

    {
        MainWindow window;
        window.setEditMode(false, QString());

        QLineEdit *nameEdit = window.findChild<QLineEdit *>(QStringLiteral("New_item_name"));
        QVERIFY(nameEdit);
        QCOMPARE(nameEdit->text(), QString());

        QCOMPARE(window.getNewItemStatusComboBox()->currentText(), QStringLiteral("brak"));
        QCOMPARE(window.getNewItemStoragePlaceComboBox()->currentText(), QStringLiteral("brak"));
    }

    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("default_connection"));
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
}

void RepositoryTests::mainWindow_setEditModeLoadsExistingRecord()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString dbPath = tempDir.filePath(QStringLiteral("mainwindow-edit.sqlite"));
    QVERIFY(setupDatabase(QStringLiteral("SQLite3"), dbPath));

    QSqlDatabase formDb = QSqlDatabase::database(QStringLiteral("default_connection"));
    QVERIFY(formDb.isOpen());

    ItemRepository repository(formDb);
    QString savedItemId;
    QString errorMessage;

    auto lookupIdInDb = [&formDb](const QString &tableName, const QString &name)
    {
        QSqlQuery query(formDb);
        query.prepare(QStringLiteral("SELECT id FROM %1 WHERE name = :name").arg(tableName));
        query.bindValue(QStringLiteral(":name"), name);
        if (!query.exec() || !query.next())
            return QString();
        return query.value(0).toString();
    };

    ItemRecordData item;
    item.name = QStringLiteral("Eksponat UI");
    item.serialNumber = QStringLiteral("UI-001");
    item.statusId = lookupIdInDb(QStringLiteral("statuses"), QStringLiteral("Sprawny"));
    item.typeId = lookupIdInDb(QStringLiteral("types"), QStringLiteral("Komputer"));
    item.vendorId = lookupIdInDb(QStringLiteral("vendors"), QStringLiteral("Atari"));
    item.modelId = lookupIdInDb(QStringLiteral("models"), QStringLiteral("Atari 800XL"));
    item.storagePlaceId = lookupIdInDb(QStringLiteral("storage_places"), QStringLiteral("Magazyn 1"));
    QVERIFY2(repository.saveItem(item, {}, &savedItemId, &errorMessage), qPrintable(errorMessage));

    {
        MainWindow window;
        window.setEditMode(true, savedItemId);

        QLineEdit *nameEdit = window.findChild<QLineEdit *>(QStringLiteral("New_item_name"));
        QLineEdit *serialEdit = window.findChild<QLineEdit *>(QStringLiteral("New_item_serialNumber"));
        QVERIFY(nameEdit);
        QVERIFY(serialEdit);

        QCOMPARE(nameEdit->text(), QStringLiteral("Eksponat UI"));
        QCOMPARE(serialEdit->text(), QStringLiteral("UI-001"));
        QCOMPARE(window.getNewItemVendorComboBox()->currentText(), QStringLiteral("Atari"));
        QCOMPARE(window.getNewItemModelComboBox()->currentText(), QStringLiteral("Atari 800XL"));
    }

    formDb.close();
    formDb = QSqlDatabase();
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
}

void RepositoryTests::itemFilterProxyModel_searchesAcrossMultipleFields()
{
    QStandardItemModel model(2, 14);

    model.setData(model.index(0, 1), QStringLiteral("Atari komputer"));
    model.setData(model.index(0, 3), QStringLiteral("Atari"));
    model.setData(model.index(0, 4), QStringLiteral("800XL"));
    model.setData(model.index(0, 5), QStringLiteral("SER-800"));
    model.setData(model.index(0, 6), QStringLiteral("PART-ATARI"));
    model.setData(model.index(0, 11), QStringLiteral("Opis pierwszego rekordu"));
    model.setData(model.index(0, 13), true);

    model.setData(model.index(1, 1), QStringLiteral("Commodore komputer"));
    model.setData(model.index(1, 3), QStringLiteral("Commodore"));
    model.setData(model.index(1, 4), QStringLiteral("C64"));
    model.setData(model.index(1, 5), QStringLiteral("SER-C64"));
    model.setData(model.index(1, 6), QStringLiteral("PART-C64"));
    model.setData(model.index(1, 11), QStringLiteral("Opis drugiego rekordu"));
    model.setData(model.index(1, 13), false);

    ItemFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    proxy.setNameFilter(QStringLiteral("800XL"));
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setNameFilter(QStringLiteral("SER-C64"));
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setNameFilter(QStringLiteral("Commodore"));
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setNameFilter(QStringLiteral("drugiego"));
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setNameFilter(QStringLiteral("nie-istnieje"));
    QCOMPARE(proxy.rowCount(), 0);

    proxy.setNameFilter(QString());
    proxy.setOriginalPackagingFilter(true);
    QCOMPARE(proxy.rowCount(), 1);
}

void RepositoryTests::itemList_restoresSavedFilters()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                           + QStringLiteral("/inwentaryzacja.ini"),
                       QSettings::IniFormat);
    settings.clear();
    settings.sync();

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString dbPath = tempDir.filePath(QStringLiteral("itemlist-filters.sqlite"));
    QVERIFY(setupDatabase(QStringLiteral("SQLite3"), dbPath));

    QSqlDatabase listDb = QSqlDatabase::database(QStringLiteral("default_connection"));
    QVERIFY(listDb.isOpen());

    ItemRepository repository(listDb);
    QString savedItemId;
    QString errorMessage;
    auto lookupIdInDb = [&listDb](const QString &tableName, const QString &name)
    {
        QSqlQuery query(listDb);
        query.prepare(QStringLiteral("SELECT id FROM %1 WHERE name = :name").arg(tableName));
        query.bindValue(QStringLiteral(":name"), name);
        if (!query.exec() || !query.next())
            return QString();
        return query.value(0).toString();
    };

    ItemRecordData item;
    item.name = QStringLiteral("Eksponat filtrów");
    item.statusId = lookupIdInDb(QStringLiteral("statuses"), QStringLiteral("Sprawny"));
    item.typeId = lookupIdInDb(QStringLiteral("types"), QStringLiteral("Komputer"));
    item.vendorId = lookupIdInDb(QStringLiteral("vendors"), QStringLiteral("Atari"));
    item.modelId = lookupIdInDb(QStringLiteral("models"), QStringLiteral("Atari 800XL"));
    item.storagePlaceId = lookupIdInDb(QStringLiteral("storage_places"), QStringLiteral("Magazyn 1"));
    item.description = QStringLiteral("Test filtrowania");

    QVERIFY2(repository.saveItem(item, {}, &savedItemId, &errorMessage),
             qPrintable(errorMessage));

    {
        itemList window;

        auto *searchEdit = window.findChild<QLineEdit *>(QStringLiteral("filterNameLineEdit"));
        auto *vendorCombo = window.findChild<QComboBox *>(QStringLiteral("filterVendorComboBox"));
        auto *clearButton = window.findChild<QPushButton *>(QStringLiteral("clearFiltersButton"));
        QVERIFY(searchEdit);
        QVERIFY(vendorCombo);
        QVERIFY(clearButton);

        searchEdit->setText(QStringLiteral("Atari"));
        const int atariIndex = vendorCombo->findText(QStringLiteral("Atari"));
        QVERIFY(atariIndex >= 0);
        vendorCombo->setCurrentIndex(atariIndex);
        QCOMPARE(vendorCombo->currentText(), QStringLiteral("Atari"));
        window.close();
    }

    {
        itemList window;

        auto *searchEdit = window.findChild<QLineEdit *>(QStringLiteral("filterNameLineEdit"));
        auto *vendorCombo = window.findChild<QComboBox *>(QStringLiteral("filterVendorComboBox"));
        auto *clearButton = window.findChild<QPushButton *>(QStringLiteral("clearFiltersButton"));
        QVERIFY(searchEdit);
        QVERIFY(vendorCombo);
        QVERIFY(clearButton);

        QCOMPARE(searchEdit->text(), QStringLiteral("Atari"));
        QCOMPARE(vendorCombo->currentText(), QStringLiteral("Atari"));

        clearButton->click();
        QCOMPARE(searchEdit->text(), QString());
        QCOMPARE(vendorCombo->currentText(), QStringLiteral("Wszystkie"));
    }

    listDb.close();
    listDb = QSqlDatabase();
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
}

void RepositoryTests::pacmanAnimationModel_activatesAfterConfiguredDelay()
{
    PacmanAnimationModel model;
    PacmanAnimationModel::Config config;
    config.activationDelayMs = 100;
    config.ghostDelayMs = 0;
    config.eatCharIntervalMs = 50;
    config.collisionFrameIntervalMs = 10;
    config.collisionHoldMs = 20;
    config.pacmanSpeedPxPerSecond = 100.0;
    model.configure(config);
    model.start(60, 10, 3);

    QCOMPARE(model.state(), PacmanAnimationModel::State::Delay);
    QVERIFY(!model.showOverlay());

    model.advance(99);
    QCOMPARE(model.state(), PacmanAnimationModel::State::Delay);
    QVERIFY(!model.consumeActivatedFlag());

    model.advance(1);
    QCOMPARE(model.state(), PacmanAnimationModel::State::Eating);
    QVERIFY(model.consumeActivatedFlag());
    QVERIFY(model.showOverlay());
}

void RepositoryTests::pacmanAnimationModel_requestsEatingInTime()
{
    PacmanAnimationModel model;
    PacmanAnimationModel::Config config;
    config.activationDelayMs = 0;
    config.ghostDelayMs = 1000;
    config.eatCharIntervalMs = 50;
    config.pacmanSpeedPxPerSecond = 100.0;
    model.configure(config);
    model.start(60, 10, 3);

    model.advance(1);
    QVERIFY(model.consumeActivatedFlag());

    model.advance(50);
    QCOMPARE(model.takePendingEatCount(), 1);

    model.setCurrentTextWidth(40);
    model.advance(50);
    QCOMPARE(model.takePendingEatCount(), 1);
    QVERIFY(model.pacmanX() < 60.0);
}

void RepositoryTests::pacmanAnimationModel_reachesCollisionAndFinish()
{
    PacmanAnimationModel model;
    PacmanAnimationModel::Config config;
    config.activationDelayMs = 0;
    config.ghostDelayMs = 0;
    config.eatCharIntervalMs = 50;
    config.collisionFrameIntervalMs = 10;
    config.collisionHoldMs = 20;
    config.pacmanSpeedPxPerSecond = 100.0;
    model.configure(config);
    model.start(60, 10, 3);

    model.advance(1);
    QVERIFY(model.consumeActivatedFlag());

    for (int iteration = 0; iteration < 100 && !model.isCollided(); ++iteration) {
        model.advance(10);
        const int pendingEatCount = model.takePendingEatCount();
        if (pendingEatCount > 0) {
            const int currentWidth = qMax(0, 60 - ((iteration + 1) * 10));
            model.setCurrentTextWidth(currentWidth);
        }
    }

    QVERIFY(model.isCollided());
    QVERIFY(model.showGhost());

    for (int iteration = 0; iteration < 100 && !model.consumeFinishedFlag(); ++iteration) {
        model.advance(10);
    }

    QVERIFY(model.state() == PacmanAnimationModel::State::Finished);
    QVERIFY(!model.showGhost());
}

// Testy sejfu piszą do usługi "Inwentaryzacja-test" (tryb testowy QStandardPaths),
// nigdy do prawdziwego wpisu "Inwentaryzacja".
QString RepositoryTests::keychainSkipReason() const
{
#ifdef Q_OS_MACOS
    // Backend macOS oddaje wynik przez główną kolejkę GCD, której dyspozytor zdarzeń
    // platformy offscreen nie opróżnia — test wisiałby. CTest ma osobny wpis keychain_tests.
    if (QGuiApplication::platformName() == QLatin1String("offscreen"))
        return QStringLiteral("Sejf macOS wymaga platformy cocoa (wpis CTest keychain_tests).");
#endif
    if (!QKeychain::isAvailable())
        return QStringLiteral("Brak systemowego sejfu na tej maszynie.");
    return QString();
}

void RepositoryTests::aiApiKey_storesReadsAndDeletesInKeychain()
{
    const QString skipReason = keychainSkipReason();
    if (!skipReason.isEmpty())
        QSKIP(qPrintable(skipReason));
    const auto removeKey = qScopeGuard([] { AiEnrichmentService::storeApiKey(QString()); });

    const QString key = QStringLiteral("sk-ant-TEST-zapis-odczyt");
    QString error;
    QVERIFY2(AiEnrichmentService::storeApiKey(key, &error), qPrintable(error));
    QCOMPARE(AiEnrichmentService::storedApiKey(&error), key);
    QVERIFY2(error.isEmpty(), qPrintable(error));

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                           + QStringLiteral("/inwentaryzacja.ini"),
                       QSettings::IniFormat);
    QVERIFY(!settings.contains(QStringLiteral("ai/anthropic_api_key")));

    QVERIFY2(AiEnrichmentService::storeApiKey(QString(), &error), qPrintable(error));
    QVERIFY(AiEnrichmentService::storedApiKey(&error).isEmpty());
    QVERIFY2(error.isEmpty(), qPrintable(error));
}

void RepositoryTests::aiApiKey_migratesLegacyIniKeyToKeychain()
{
    const QString skipReason = keychainSkipReason();
    if (!skipReason.isEmpty())
        QSKIP(qPrintable(skipReason));
    const auto removeKey = qScopeGuard([] { AiEnrichmentService::storeApiKey(QString()); });

    const QString iniPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                            + QStringLiteral("/inwentaryzacja.ini");
    const QString legacyKey = QStringLiteral("sk-ant-TEST-z-ini-1-5-0");
    {
        QSettings settings(iniPath, QSettings::IniFormat);
        settings.setValue(QStringLiteral("ai/anthropic_api_key"), legacyKey);
    }

    QString error;
    QCOMPARE(AiEnrichmentService::storedApiKey(&error), legacyKey);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    {
        QSettings settings(iniPath, QSettings::IniFormat);
        QVERIFY(!settings.contains(QStringLiteral("ai/anthropic_api_key")));
    }

    // Ini jest już puste — drugi odczyt idzie z sejfu.
    QCOMPARE(AiEnrichmentService::storedApiKey(&error), legacyKey);
    QVERIFY2(error.isEmpty(), qPrintable(error));
}

void RepositoryTests::aiApiKey_storeRemovesLegacyIniKey()
{
    const QString skipReason = keychainSkipReason();
    if (!skipReason.isEmpty())
        QSKIP(qPrintable(skipReason));
    const auto removeKey = qScopeGuard([] { AiEnrichmentService::storeApiKey(QString()); });

    // Stary klucz w ini (migracja się nie udała) nie może nadpisać nowego klucza z okna.
    const QString iniPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                            + QStringLiteral("/inwentaryzacja.ini");
    {
        QSettings settings(iniPath, QSettings::IniFormat);
        settings.setValue(QStringLiteral("ai/anthropic_api_key"), QStringLiteral("sk-ant-TEST-stary"));
    }

    const QString newKey = QStringLiteral("sk-ant-TEST-nowy");
    QString error;
    QVERIFY2(AiEnrichmentService::storeApiKey(newKey, &error), qPrintable(error));
    {
        QSettings settings(iniPath, QSettings::IniFormat);
        QVERIFY(!settings.contains(QStringLiteral("ai/anthropic_api_key")));
    }
    QCOMPARE(AiEnrichmentService::storedApiKey(&error), newKey);
}

void RepositoryTests::previewDialog_enrichButtonInButtonRow()
{
    // Przycisk AI pojawia się tylko z kluczem — podajemy go zmienną środowiskową, bez sejfu.
    const ScopedTestApiKey apiKey;

    ItemRepository repository(m_db);
    QString itemId;
    QString errorMessage;
    QVERIFY2(repository.saveItem(createSampleItem(), {}, &itemId, &errorMessage), qPrintable(errorMessage));

    PreviewDialog dialog(m_db, itemId);
    QPushButton *enrichButton = nullptr;
    for (QPushButton *button : dialog.findChildren<QPushButton *>())
    {
        if (button->text().contains(QStringLiteral("Wzbogać")))
            enrichButton = button;
    }
    QVERIFY(enrichButton);

    // Do 1.6.0 przycisk nie trafiał do żadnego układu i wisiał ucięty w lewym górnym rogu okna.
    auto *buttonRow = dialog.findChild<QHBoxLayout *>(QStringLiteral("buttonLayout"));
    QVERIFY(buttonRow);
    QVERIFY(buttonRow->indexOf(enrichButton) >= 0);
}

void RepositoryTests::previewDialog_showsPhotosAndFields()
{
    const ScopedTestApiKey apiKey;
    ItemRepository repository(m_db);
    QString itemId;
    QString errorMessage;
    QVERIFY2(repository.saveItem(createSampleItem(), {createPhotoBytes(), createPhotoBytes()},
                                 &itemId, &errorMessage),
             qPrintable(errorMessage));

    {
        PreviewDialog dialog(m_db, itemId);
        auto *photos = dialog.findChild<QListWidget *>(QStringLiteral("photosList"));
        QVERIFY(photos);
        QCOMPARE(photos->count(), 2);
        QVERIFY(!photos->isHidden());

        const auto field = [&dialog](const char *name)
        {
            auto *label = dialog.findChild<QLabel *>(QLatin1String(name));
            return label ? label->text() : QStringLiteral("<brak pola>");
        };
        QCOMPARE(field("valueType"), QStringLiteral("Komputer"));
        QCOMPARE(field("valueVendor"), QStringLiteral("Atari"));
        QCOMPARE(field("valueModel"), QStringLiteral("Atari 800XL"));
        QCOMPARE(field("valueSerial"), QStringLiteral("SER-001"));
        QCOMPARE(field("valuePartNumber"), QStringLiteral("PART-001"));
        QCOMPARE(field("valueRevision"), QStringLiteral("A"));
        QCOMPARE(field("valueYear"), QStringLiteral("1988"));
        QCOMPARE(field("valueStatus"), QStringLiteral("Sprawny"));
        QCOMPARE(field("valueStorage"), QStringLiteral("Magazyn 1"));
        QCOMPARE(field("valuePackaging"), QStringLiteral("Tak"));
        saveScreenshotIfRequested(dialog, QStringLiteral("podglad-ze-zdjeciami"));
    }

    // Brzeg: bez zdjęć i bez roku — lista miniatur ukryta, puste pole jako „—”.
    ItemRecordData bare = createSampleItem();
    bare.productionYear = 0;
    bare.hasOriginalPackaging = false;
    QString bareId;
    QVERIFY2(repository.saveItem(bare, {}, &bareId, &errorMessage), qPrintable(errorMessage));
    PreviewDialog bareDialog(m_db, bareId);
    auto *barePhotos = bareDialog.findChild<QListWidget *>(QStringLiteral("photosList"));
    QVERIFY(barePhotos);
    QCOMPARE(barePhotos->count(), 0);
    QVERIFY(barePhotos->isHidden());
    QCOMPARE(bareDialog.findChild<QLabel *>(QStringLiteral("valueYear"))->text(), QStringLiteral("—"));
    QCOMPARE(bareDialog.findChild<QLabel *>(QStringLiteral("valuePackaging"))->text(), QStringLiteral("Nie"));
}

void RepositoryTests::itemList_showsDescriptionOfSelectedRow()
{
    QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                           + QStringLiteral("/inwentaryzacja.ini"),
                       QSettings::IniFormat);
    settings.clear();
    settings.sync();

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(setupDatabase(QStringLiteral("SQLite3"), tempDir.filePath(QStringLiteral("itemlist-opis.sqlite"))));
    // Zamknięcie połączenia PRZED sprzątnięciem katalogu (Windows trzyma otwarty plik SQLite);
    // strażnik stoi przed oknem, więc okno (z modelem na tym połączeniu) znika pierwsze.
    const auto closeDb = qScopeGuard([] {
        QSqlDatabase::database(QStringLiteral("default_connection"), false).close();
        QSqlDatabase::removeDatabase(QStringLiteral("default_connection"));
    });
    QSqlDatabase listDb = QSqlDatabase::database(QStringLiteral("default_connection"));
    QVERIFY(listDb.isOpen());

    ItemRepository repository(listDb);
    auto lookupIdInDb = [&listDb](const QString &tableName, const QString &name)
    {
        QSqlQuery query(listDb);
        query.prepare(QStringLiteral("SELECT id FROM %1 WHERE name = :name").arg(tableName));
        query.bindValue(QStringLiteral(":name"), name);
        if (!query.exec() || !query.next())
            return QString();
        return query.value(0).toString();
    };

    // Model listy łączy słowniki INNER JOIN-em — rekord bez nich nie trafiłby na listę.
    ItemRecordData item;
    item.name = QStringLiteral("Eksponat z opisem");
    item.statusId = lookupIdInDb(QStringLiteral("statuses"), QStringLiteral("Sprawny"));
    item.typeId = lookupIdInDb(QStringLiteral("types"), QStringLiteral("Komputer"));
    item.vendorId = lookupIdInDb(QStringLiteral("vendors"), QStringLiteral("Atari"));
    item.modelId = lookupIdInDb(QStringLiteral("models"), QStringLiteral("Atari 800XL"));
    item.storagePlaceId = lookupIdInDb(QStringLiteral("storage_places"), QStringLiteral("Magazyn 1"));
    item.description = QStringLiteral("**Opis widoczny** na liście");
    QString itemId;
    QString errorMessage;
    QVERIFY2(repository.saveItem(item, {createPhotoBytes()}, &itemId, &errorMessage), qPrintable(errorMessage));

    itemList window;
    auto *table = window.findChild<QTableView *>(QStringLiteral("itemList_tableView"));
    auto *description = window.findChild<QTextBrowser *>(QStringLiteral("itemList_descriptionView"));
    QVERIFY(table);
    QVERIFY(description);
    QVERIFY(description->toPlainText().isEmpty());

    table->selectRow(0);
    QCOMPARE(description->toPlainText().trimmed(), QStringLiteral("Opis widoczny na liście"));
    saveScreenshotIfRequested(window, QStringLiteral("lista-z-opisem"));

    table->clearSelection();
    QVERIFY(description->toPlainText().isEmpty());

    // Dwa zaznaczone wiersze = brak opisu; odznaczenie jednego (jak Ctrl-klik: pusta delta
    // `selected`) ma pokazać opis tego, który został zaznaczony.
    ItemRecordData second = item;
    second.name = QStringLiteral("Drugi eksponat");
    second.description = QStringLiteral("Opis drugiego");
    QString secondId;
    QVERIFY2(repository.saveItem(second, {}, &secondId, &errorMessage), qPrintable(errorMessage));
    QVERIFY(QMetaObject::invokeMethod(&window, "refreshList", Q_ARG(QString, QString())));
    QCOMPARE(table->model()->rowCount(), 2);

    QItemSelectionModel *selection = table->selectionModel();
    const QModelIndex row0 = table->model()->index(0, 0);
    const QModelIndex row1 = table->model()->index(1, 0);
    selection->select(row0, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    selection->select(row1, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    QVERIFY(description->toPlainText().isEmpty());

    selection->select(row1, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    const QString remainingName = table->model()->index(0, 1).data().toString();
    QCOMPARE(description->toPlainText().trimmed(),
             remainingName == item.name ? QStringLiteral("Opis widoczny na liście")
                                        : QStringLiteral("Opis drugiego"));
    window.close();
}

void RepositoryTests::mysqlPassword_configDialogMigratesIniPasswordToKeychain()
{
    const QString skipReason = keychainSkipReason();
    if (!skipReason.isEmpty())
        QSKIP(qPrintable(skipReason));
    const auto removeEntry = qScopeGuard([] { deleteTestKeychain(QStringLiteral("mysql_password")); });

    // Wersje ≤ 1.6.1 trzymały hasło jawnym tekstem w ini.
    {
        QSettings settings(testIniPath(), QSettings::IniFormat);
        settings.setValue(QStringLiteral("Database/MySQL/Password"), QStringLiteral("haslo-TEST-z-ini"));
    }
    {
        DatabaseConfigDialog dialog;
        QCOMPARE(dialog.mysqlPassword(), QStringLiteral("haslo-TEST-z-ini"));
    }
    {
        QSettings settings(testIniPath(), QSettings::IniFormat);
        QVERIFY(!settings.contains(QStringLiteral("Database/MySQL/Password")));
    }
    QCOMPARE(readTestKeychain(QStringLiteral("mysql_password")), QStringLiteral("haslo-TEST-z-ini"));

    // Ini jest już bez hasła — następne okno bierze je z sejfu.
    DatabaseConfigDialog dialog;
    QCOMPARE(dialog.mysqlPassword(), QStringLiteral("haslo-TEST-z-ini"));
    saveScreenshotIfRequested(dialog, QStringLiteral("konfiguracja-bazy"));
}

void RepositoryTests::mysqlPassword_configDialogStoresChangedPasswordInKeychain()
{
    const QString skipReason = keychainSkipReason();
    if (!skipReason.isEmpty())
        QSKIP(qPrintable(skipReason));
    const auto removeEntry = qScopeGuard([] { deleteTestKeychain(QStringLiteral("mysql_password")); });

    {
        DatabaseConfigDialog dialog;
        auto *passwordEdit = dialog.findChild<QLineEdit *>(QStringLiteral("passwordLineEdit"));
        QVERIFY(passwordEdit);
        passwordEdit->setText(QStringLiteral("haslo-TEST-nowe"));
        passwordEdit->setModified(true);  // jak po wpisaniu z klawiatury
        dialog.accept();
    }
    {
        QSettings settings(testIniPath(), QSettings::IniFormat);
        QVERIFY(!settings.contains(QStringLiteral("Database/MySQL/Password")));
    }
    QCOMPARE(readTestKeychain(QStringLiteral("mysql_password")), QStringLiteral("haslo-TEST-nowe"));

    // OK bez zmiany pola nie rusza sejfu (pusty odczyt po odmowie sejfu nie może skasować hasła).
    {
        DatabaseConfigDialog dialog;
        QCOMPARE(dialog.mysqlPassword(), QStringLiteral("haslo-TEST-nowe"));
        dialog.accept();
    }
    QCOMPARE(readTestKeychain(QStringLiteral("mysql_password")), QStringLiteral("haslo-TEST-nowe"));
}

void RepositoryTests::aiResponse_readsBlocksByType()
{
    // Modele 5.5 zawsze myślą: pierwszy blok to `thinking` (1.6.1 brał content[0] i zgłaszał błąd).
    AiEnrichmentService::ParsedResponse parsed = AiEnrichmentService::parseResponse(200, R"({
        "model": "claude-opus-5-5", "stop_reason": "end_turn",
        "content": [{"type": "thinking", "thinking": "", "signature": "x"},
                    {"type": "text", "text": "Opis **Amigi**"}],
        "usage": {"input_tokens": 5000, "output_tokens": 1200}})");
    QVERIFY2(parsed.error.isEmpty(), qPrintable(parsed.error));
    QCOMPARE(parsed.description, QStringLiteral("Opis **Amigi**"));
    QCOMPARE(parsed.model, QStringLiteral("claude-opus-5-5"));  // cena wg modelu, który odpowiedział
    QCOMPARE(parsed.inputTokens, 5000);
    QCOMPARE(parsed.outputTokens, 1200);

    // Model zastępczy po odmowie: opis to tekst po bloku `fallback`, nie urwany fragment przed nim.
    parsed = AiEnrichmentService::parseResponse(200, R"({
        "stop_reason": "end_turn",
        "content": [{"type": "text", "text": "urwany fragment"},
                    {"type": "fallback", "from": {"model": "claude-opus-5-5"}, "to": {"model": "claude-opus-4-8"}},
                    {"type": "text", "text": "Pełny opis"}],
        "usage": {"input_tokens": 10, "output_tokens": 20}})");
    QVERIFY2(parsed.error.isEmpty(), qPrintable(parsed.error));
    QCOMPARE(parsed.description, QStringLiteral("Pełny opis"));

    // Odmowa przychodzi jako HTTP 200.
    parsed = AiEnrichmentService::parseResponse(200, R"({
        "stop_reason": "refusal", "stop_details": {"type": "refusal", "category": "cyber"}, "content": []})");
    QVERIFY(parsed.description.isEmpty());
    QVERIFY2(parsed.error.contains(QStringLiteral("cyber")), qPrintable(parsed.error));

    // Ucięcie na limicie tokenów = błąd, nie połowa opisu.
    parsed = AiEnrichmentService::parseResponse(200, R"({
        "stop_reason": "max_tokens", "content": [{"type": "text", "text": "Pół opisu"}]})");
    QVERIFY(parsed.description.isEmpty());
    QVERIFY(!parsed.error.isEmpty());

    parsed = AiEnrichmentService::parseResponse(400, R"({
        "type": "error", "error": {"type": "invalid_request_error", "message": "zły model"}})");
    QVERIFY2(parsed.error.contains(QStringLiteral("zły model")), qPrintable(parsed.error));
}

void RepositoryTests::aiModels_unknownSavedModelFallsBackToDefault()
{
    const auto restore = qScopeGuard([] {
        QSettings(testIniPath(), QSettings::IniFormat).remove(QStringLiteral("ai/model"));
    });
    const auto saveModel = [](const QString &id) {
        QSettings settings(testIniPath(), QSettings::IniFormat);
        settings.setValue(QStringLiteral("ai/model"), id);
        settings.sync();
    };

    // 1.6.x zapisywało claude-opus-4-7 — koszt i zapytanie mają dotyczyć modelu z listy.
    saveModel(QStringLiteral("claude-opus-4-7"));
    QCOMPARE(QLatin1String(AiEnrichmentService::configuredModel().id), QLatin1String("claude-opus-5-5"));
    QVERIFY(AiEnrichmentService::configuredModel().serverFallback);

    saveModel(QStringLiteral("claude-haiku-5-5"));
    QCOMPARE(QLatin1String(AiEnrichmentService::configuredModel().id), QLatin1String("claude-haiku-5-5"));
    QVERIFY(!AiEnrichmentService::configuredModel().serverFallback);

    for (const AiEnrichmentService::Model &model : AiEnrichmentService::models())
        QVERIFY(AiEnrichmentService::estimateCostUsd(model, 2) > 0.0);
}

void RepositoryTests::aiEnrichment_realCallReturnsDescription()
{
    // Płatne wywołanie prawdziwego API (każdy model z listy) — tylko na żądanie:
    // INW_PRAWDZIWE_AI=1 [INW_ZDJECIE=zdjecie.jpg] ./InwentaryzacjaTests aiEnrichment_realCallReturnsDescription
    if (qEnvironmentVariableIsEmpty("INW_PRAWDZIWE_AI"))
        QSKIP("Płatne wywołanie API — uruchom z INW_PRAWDZIWE_AI=1.");
    QVERIFY2(!qEnvironmentVariableIsEmpty("ANTHROPIC_API_KEY"), "Ustaw ANTHROPIC_API_KEY.");

    QByteArray photo = createPhotoBytes();
    if (const QString path = qEnvironmentVariable("INW_ZDJECIE"); !path.isEmpty())
    {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(path));
        photo = file.readAll();
    }

    const auto restore = qScopeGuard([] {
        QSettings(testIniPath(), QSettings::IniFormat).remove(QStringLiteral("ai/model"));
    });
    for (const AiEnrichmentService::Model &model : AiEnrichmentService::models())
    {
        {
            QSettings settings(testIniPath(), QSettings::IniFormat);
            settings.setValue(QStringLiteral("ai/model"), QString::fromLatin1(model.id));
        }
        AiEnrichmentService service;
        QSignalSpy ready(&service, &AiEnrichmentService::enrichmentReady);
        QSignalSpy failed(&service, &AiEnrichmentService::enrichmentError);
        QElapsedTimer timer;
        timer.start();
        service.enrichDescription(QString(), {photo}, QStringLiteral("Commodore 64"),
                                  QStringLiteral("Commodore"), QStringLiteral("C64"));
        QTRY_VERIFY_WITH_TIMEOUT(ready.count() + failed.count() > 0, 200000);
        QVERIFY2(failed.isEmpty(), qPrintable(failed.isEmpty() ? QString() : failed.first().first().toString()));

        const qint64 inputTokens = ready.first().at(1).toLongLong();
        const qint64 outputTokens = ready.first().at(2).toLongLong();
        qInfo().noquote() << model.id << "czas" << timer.elapsed() << "ms, tokeny" << inputTokens << "/"
                          << outputTokens << "koszt $" << AiEnrichmentService::costUsd(model, inputTokens, outputTokens)
                          << "estymata $" << AiEnrichmentService::estimateCostUsd(model, 1) << "opis"
                          << ready.first().at(0).toString().size() << "znaków";
        QVERIFY(!ready.first().at(0).toString().trimmed().isEmpty());
    }
}

void RepositoryTests::sqlDriver_mysqlPluginLoads()
{
    // Paczki Windows do 1.6.1 nie miały pluginu QMYSQL. CI uruchamia ten test z katalogu paczki
    // (INW_WYMAGAJ_QMYSQL=1) — brak pluginu albo libmariadb.dll obok exe ma być czerwony.
    if (qEnvironmentVariableIsEmpty("INW_WYMAGAJ_QMYSQL"))
        QSKIP("Sprawdzenie paczki — uruchom z INW_WYMAGAJ_QMYSQL=1.");
    QVERIFY2(QSqlDatabase::isDriverAvailable(QStringLiteral("QMYSQL")),
             qPrintable(QStringLiteral("Dostępne sterowniki: ") + QSqlDatabase::drivers().join(QStringLiteral(", "))));
    {
        const QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"), QStringLiteral("sonda_qmysql"));
        QVERIFY2(db.isValid(), "Plugin QMYSQL się nie załadował (brak biblioteki libmariadb obok programu?)");
    }
    QSqlDatabase::removeDatabase(QStringLiteral("sonda_qmysql"));
}

QTEST_MAIN(RepositoryTests)

#include "repository_tests.moc"
