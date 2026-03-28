#include <QtTest>

#include "DictionaryRepository.h"
#include "DatabaseMigration.h"
#include "ItemFormValidator.h"
#include "ItemRepository.h"
#include "PhotoService.h"
#include "utils.h"

#include <QBuffer>
#include <QImage>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUuid>

class RepositoryTests : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void itemRepository_savesRecordAndPhotos();
    void itemRepository_deletesRecordAndPhotos();
    void itemRepository_updatesExistingRecordWithoutDuplicatingPhotos();
    void dictionaryRepository_supportsCrud();
    void dictionaryRepository_addsModelWithParentVendor();
    void photoService_loadsStoredPhotos();
    void photoService_movesPhotosToDoneWhenEnabled();
    void photoService_keepsPhotosInPlaceWhenMoveDisabled();
    void databaseMigration_removesBracesFromAllRelevantTables();
    void databaseMigration_fixesKnownBrokenUuids();
    void databaseMigration_isNoOpWithoutSchema();
    void itemFormValidator_rejectsEmptyName();
    void itemFormValidator_parsesNumericValue();
    void itemFormValidator_rejectsInvalidNumericValue();
    void itemFormValidator_requiresSelection();
    void itemFormValidator_checksModelVendorConsistency();

private:
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

QTEST_MAIN(RepositoryTests)

#include "repository_tests.moc"
