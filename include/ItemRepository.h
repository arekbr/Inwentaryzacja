#ifndef ITEMREPOSITORY_H
#define ITEMREPOSITORY_H

#include <QByteArray>
#include <QList>
#include <QSqlDatabase>
#include <QString>

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

private:
    QSqlDatabase m_db;
};

#endif // ITEMREPOSITORY_H
