#ifndef ITEMREPOSITORY_H
#define ITEMREPOSITORY_H

#include <QByteArray>
#include <QCoreApplication>
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

/// O-5 (audit 2026-04-26): m_db jest QSqlDatabase HANDLE — refcounted reference
/// do globalnego connection pool Qt. Repository NIE OWNS connection.
///
/// **Lifetime contract:**
/// - Connection ("default_connection" lub custom name z konstruktora) MUSI istnieć
///   przez całe życie ItemRepository.
/// - NIE wywołuj `QSqlDatabase::removeDatabase(name)` dopóki repository żyje —
///   handle stanie się dangling, kolejne metody dadzą "connection is not open".
/// - Jeśli aplikacja switchuje bazę w runtime (np. user zmienia Settings →
///   Inna baza), zniszcz wszystkie repository PRZED removeDatabase.
///
/// Sygnatury metod używają `m_db.isOpen()` jako runtime check, ale `isValid()`
/// (handle wskazuje na żywy connection) NIE jest sprawdzany. Trust caller contract.
class ItemRepository
{
    // O-6 (audit 2026-04-26): translation context = "ItemRepository" zamiast
    // "QObject" (do ktorego trafialy wszystkie tr() z free functions).
    // Czyszczy konteksty translacyjne — translator widzi grupowane stringi.
    Q_DECLARE_TR_FUNCTIONS(ItemRepository)

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

    /// v1.5 (AI enrichment): UPDATE eksponaty SET description = :desc WHERE id = :id.
    /// Używane przez "Wzbogać opis AI" w PreviewDialog po accept.
    bool updateDescription(const QString &itemId,
                           const QString &newDescription,
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
