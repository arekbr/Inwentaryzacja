#include <QtTest>

#include "DictionaryRepository.h"
#include "ItemRepository.h"
#include "PhotoService.h"
#include "utils.h"

#include <QBuffer>
#include <QImage>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

class RepositoryTests : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void itemRepository_savesRecordAndPhotos();
    void itemRepository_deletesRecordAndPhotos();
    void dictionaryRepository_supportsCrud();
    void photoService_loadsStoredPhotos();

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

QTEST_MAIN(RepositoryTests)

#include "repository_tests.moc"
