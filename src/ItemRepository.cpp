#include "ItemRepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {

QString formatDbError(const QString &context, const QString &details)
{
    return QObject::tr("%1\n%2").arg(context, details);
}

}

ItemRepository::ItemRepository(QSqlDatabase database)
    : m_db(database)
{
}

bool ItemRepository::saveItem(const ItemRecordData &item,
                              const QList<QByteArray> &newPhotos,
                              QString *savedItemId,
                              QString *errorMessage)
{
    if (!m_db.isOpen()) {
        if (errorMessage)
            *errorMessage = QObject::tr("Połączenie z bazą danych jest zamknięte.");
        return false;
    }

    QString itemId = item.id;
    if (itemId.isEmpty())
        itemId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    if (!m_db.transaction()) {
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się rozpocząć transakcji zapisu."),
                                          m_db.lastError().text());
        return false;
    }

    QSqlQuery query(m_db);
    if (!item.editMode) {
        query.prepare(R"(
            INSERT INTO eksponaty
            (id, name, serial_number, part_number, revision, production_year,
             status_id, type_id, vendor_id, model_id, storage_place_id,
             description, value, has_original_packaging)
            VALUES
            (:id, :name, :serial_number, :part_number, :revision, :production_year,
             :status_id, :type_id, :vendor_id, :model_id, :storage_place_id,
             :description, :value, :has_original_packaging)
        )");
    } else {
        query.prepare(R"(
            UPDATE eksponaty
            SET name=:name,
                serial_number=:serial_number,
                part_number=:part_number,
                revision=:revision,
                production_year=:production_year,
                status_id=:status_id,
                type_id=:type_id,
                vendor_id=:vendor_id,
                model_id=:model_id,
                storage_place_id=:storage_place_id,
                description=:description,
                value=:value,
                has_original_packaging=:has_original_packaging
            WHERE id=:id
        )");
    }

    query.bindValue(":id", itemId);
    query.bindValue(":name", item.name);
    query.bindValue(":serial_number", item.serialNumber);
    query.bindValue(":part_number", item.partNumber);
    query.bindValue(":revision", item.revision);
    query.bindValue(":production_year", item.productionYear);
    query.bindValue(":status_id", item.statusId);
    query.bindValue(":type_id", item.typeId);
    query.bindValue(":vendor_id", item.vendorId);
    query.bindValue(":model_id", item.modelId);
    query.bindValue(":storage_place_id", item.storagePlaceId);
    query.bindValue(":description", item.description);
    query.bindValue(":value", item.value);
    query.bindValue(":has_original_packaging", item.hasOriginalPackaging);

    if (!query.exec()) {
        m_db.rollback();
        if (errorMessage)
            *errorMessage = formatDbError(item.editMode
                                              ? QObject::tr("Nie udało się zaktualizować eksponatu.")
                                              : QObject::tr("Nie udało się dodać eksponatu."),
                                          query.lastError().text());
        return false;
    }

    if (!item.editMode) {
        for (const QByteArray &photoData : newPhotos) {
            QSqlQuery photoQuery(m_db);
            photoQuery.prepare(R"(
                INSERT INTO photos (id, eksponat_id, photo)
                VALUES (:id, :itemId, :photo)
            )");
            photoQuery.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
            photoQuery.bindValue(":itemId", itemId);
            photoQuery.bindValue(":photo", photoData);

            if (!photoQuery.exec()) {
                m_db.rollback();
                if (errorMessage)
                    *errorMessage = formatDbError(QObject::tr("Nie udało się zapisać zdjęcia eksponatu."),
                                                  photoQuery.lastError().text());
                return false;
            }
        }
    }

    if (!m_db.commit()) {
        m_db.rollback();
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się zatwierdzić zapisu w bazie danych."),
                                          m_db.lastError().text());
        return false;
    }

    if (savedItemId)
        *savedItemId = itemId;
    if (errorMessage)
        errorMessage->clear();
    return true;
}

bool ItemRepository::deleteItem(const QString &itemId, QString *errorMessage)
{
    if (!m_db.isOpen()) {
        if (errorMessage)
            *errorMessage = QObject::tr("Połączenie z bazą danych jest zamknięte.");
        return false;
    }

    if (!m_db.transaction()) {
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się rozpocząć transakcji usuwania."),
                                          m_db.lastError().text());
        return false;
    }

    // O-4 (audit 2026-04-26): osobne QSqlQuery per statement.
    // Re-use jednego obiektu na MySQL/MariaDB driver cachuje server-side prepared
    // stmt handles do końca życia QSqlQuery — w long-running procesie bije
    // w max_prepared_stmt_count (default 16382, error 1461). Każdy w własnym
    // scope = handle finalizowany od razu.
    {
        QSqlQuery photoDelete(m_db);
        photoDelete.prepare("DELETE FROM photos WHERE eksponat_id = :id");
        photoDelete.bindValue(":id", itemId);
        if (!photoDelete.exec()) {
            m_db.rollback();
            if (errorMessage)
                *errorMessage = formatDbError(QObject::tr("Nie udało się usunąć zdjęć eksponatu."),
                                              photoDelete.lastError().text());
            return false;
        }
    }

    {
        QSqlQuery itemDelete(m_db);
        itemDelete.prepare("DELETE FROM eksponaty WHERE id = :id");
        itemDelete.bindValue(":id", itemId);
        if (!itemDelete.exec()) {
            m_db.rollback();
            if (errorMessage)
                *errorMessage = formatDbError(QObject::tr("Nie udało się usunąć eksponatu."),
                                              itemDelete.lastError().text());
            return false;
        }
    }

    if (!m_db.commit()) {
        m_db.rollback();
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się zatwierdzić usuwania w bazie danych."),
                                          m_db.lastError().text());
        return false;
    }

    if (errorMessage)
        errorMessage->clear();
    return true;
}

bool ItemRepository::updateStatusForItems(const QStringList &itemIds,
                                          const QString &statusId,
                                          QString *errorMessage)
{
    return updateItemsColumn(itemIds,
                             QStringLiteral("status_id"),
                             statusId,
                             QObject::tr("zmiany statusu"),
                             errorMessage);
}

bool ItemRepository::updateStoragePlaceForItems(const QStringList &itemIds,
                                                const QString &storagePlaceId,
                                                QString *errorMessage)
{
    return updateItemsColumn(itemIds,
                             QStringLiteral("storage_place_id"),
                             storagePlaceId,
                             QObject::tr("zmiany miejsca przechowywania"),
                             errorMessage);
}

bool ItemRepository::updateItemsColumn(const QStringList &itemIds,
                                       const QString &columnName,
                                       const QString &valueId,
                                       const QString &operationLabel,
                                       QString *errorMessage)
{
    if (!m_db.isOpen()) {
        if (errorMessage)
            *errorMessage = QObject::tr("Połączenie z bazą danych jest zamknięte.");
        return false;
    }

    if (itemIds.isEmpty()) {
        if (errorMessage)
            *errorMessage = QObject::tr("Nie wybrano żadnych rekordów do aktualizacji.");
        return false;
    }

    if (valueId.isEmpty()) {
        if (errorMessage)
            *errorMessage = QObject::tr("Nie wybrano docelowej wartości dla operacji.");
        return false;
    }

    if (!m_db.transaction()) {
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się rozpocząć transakcji %1.")
                                              .arg(operationLabel),
                                          m_db.lastError().text());
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE eksponaty SET %1 = :value WHERE id = :id").arg(columnName));

    for (const QString &itemId : itemIds) {
        query.bindValue(QStringLiteral(":value"), valueId);
        query.bindValue(QStringLiteral(":id"), itemId);
        if (!query.exec()) {
            m_db.rollback();
            if (errorMessage)
                *errorMessage = formatDbError(QObject::tr("Nie udało się wykonać %1.")
                                                  .arg(operationLabel),
                                              query.lastError().text());
            return false;
        }
    }

    if (!m_db.commit()) {
        m_db.rollback();
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się zatwierdzić %1 w bazie danych.")
                                              .arg(operationLabel),
                                          m_db.lastError().text());
        return false;
    }

    if (errorMessage)
        errorMessage->clear();
    return true;
}
