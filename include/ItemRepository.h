#ifndef ITEMREPOSITORY_H
#define ITEMREPOSITORY_H

#include <QByteArray>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

struct ItemRecordData
{
    QString id;
    QString name;
    QString serialNumber;
    QString partNumber;
    QString revision;
    int productionYear = 0;
    QString statusId;
    QString typeId;
    QString vendorId;
    QString modelId;
    QString storagePlaceId;
    QString description;
    int value = 0;
    bool hasOriginalPackaging = false;
    bool editMode = false;
};

class ItemRepository
{
public:
    explicit ItemRepository(QSqlDatabase database = QSqlDatabase::database("default_connection"));

    bool saveItem(const ItemRecordData &item,
                  const QList<QByteArray> &newPhotos,
                  QString *savedItemId,
                  QString *errorMessage);

    bool deleteItem(const QString &itemId, QString *errorMessage);
    bool updateStatusForItems(const QStringList &itemIds,
                              const QString &statusId,
                              QString *errorMessage);
    bool updateStoragePlaceForItems(const QStringList &itemIds,
                                    const QString &storagePlaceId,
                                    QString *errorMessage);

private:
    bool updateItemsColumn(const QStringList &itemIds,
                           const QString &columnName,
                           const QString &valueId,
                           const QString &operationLabel,
                           QString *errorMessage);

    QSqlDatabase m_db;
};

#endif // ITEMREPOSITORY_H
