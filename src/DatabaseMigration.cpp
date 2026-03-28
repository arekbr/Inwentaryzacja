#include "DatabaseMigration.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

DatabaseMigration::DatabaseMigration(QObject *parent)
    : QObject(parent)
{
    db = QSqlDatabase::database("default_connection");
}

bool DatabaseMigration::migrateUUIDs()
{
    qDebug() << "Rozpoczynam proces migracji UUID...";

    if (!hasMigrationTables()) {
        qDebug() << "Brak pełnego schematu wymagającego migracji UUID - pomijam migrację.";
        return true;
    }

    bool success = true;

    // Dodaj brakujące kolumny (niezależnie od tego czy są UUID-y do migracji)
    if (!addMissingColumns()) {
        qDebug() << "Nie udało się dodać brakujących kolumn";
        success = false;
    }

    // Napraw uszkodzone UUID-y (brakujące nawiasy zamykające)
    if (!fixBrokenUUIDs()) {
        qDebug() << "Nie udało się naprawić uszkodzonych UUID-ów";
        success = false;
    }

    const bool hasBracedUuids = checkForBracedUUIDs();

    // Sprawdź czy migracja jest potrzebna
    if (!hasBracedUuids) {
        qDebug() << "Nie znaleziono UUID-ów z nawiasami klamrowymi, migracja nie jest potrzebna.";
        return success;
    }

    // Rozpocznij proces migracji
    if (!disableForeignKeyChecks()) {
        qDebug() << "Nie udało się wyłączyć sprawdzania kluczy obcych";
        return false;
    }
    
    // Aktualizuj tabele we właściwej kolejności
    if (!updateStatusesTable()) {
        qDebug() << "Nie udało się zaktualizować tabeli statusów";
        success = false;
    }
    
    if (!updateStoragePlacesTable()) {
        qDebug() << "Nie udało się zaktualizować tabeli miejsc przechowywania";
        success = false;
    }
    
    if (!updateEksponatyTable()) {
        qDebug() << "Nie udało się zaktualizować tabeli eksponatów";
        success = false;
    }

    if (!updatePhotosTable()) {
        qDebug() << "Nie udało się zaktualizować tabeli zdjęć";
        success = false;
    }

    // Włącz ponownie sprawdzanie kluczy obcych
    if (!enableForeignKeyChecks()) {
        qDebug() << "Nie udało się włączyć sprawdzania kluczy obcych";
        success = false;
    }

    // Zweryfikuj migrację
    if (success && !verifyMigration()) {
        qDebug() << "Weryfikacja migracji nie powiodła się";
        success = false;
    }

    if (success) {
        qDebug() << "Migracja zakończona pomyślnie";
    } else {
        qDebug() << "Migracja nie powiodła się";
    }

    return success;
}

bool DatabaseMigration::addMissingColumns()
{
    if (db.driverName() == "QMYSQL") {
        QSqlQuery query(db);
        
        // Sprawdź czy kolumna has_original_packaging istnieje
        query.exec("SELECT COUNT(*) FROM information_schema.columns "
                  "WHERE table_schema = DATABASE() "
                  "AND table_name = 'eksponaty' "
                  "AND column_name = 'has_original_packaging'");
        
        if (query.next() && query.value(0).toInt() == 0) {
            // Kolumna nie istnieje, dodaj ją
            qDebug() << "Dodaję kolumnę has_original_packaging do tabeli eksponaty";
            if (!query.exec("ALTER TABLE eksponaty "
                          "ADD COLUMN has_original_packaging BOOLEAN DEFAULT 0")) {
                qDebug() << "Nie udało się dodać kolumny has_original_packaging:" 
                        << query.lastError().text();
                return false;
            }
        }
    }
    return true;
}

bool DatabaseMigration::checkForBracedUUIDs()
{
    return hasMatchingRows(QStringLiteral("SELECT id FROM statuses WHERE id LIKE '{%}' LIMIT 1"),
                           QStringLiteral("Nie udało się sprawdzić UUID w tabeli statuses"))
           || hasMatchingRows(QStringLiteral("SELECT id FROM storage_places WHERE id LIKE '{%}' LIMIT 1"),
                              QStringLiteral("Nie udało się sprawdzić UUID w tabeli storage_places"))
           || hasMatchingRows(QStringLiteral("SELECT id FROM eksponaty "
                                            "WHERE id LIKE '{%}' "
                                            "OR status_id LIKE '{%}' "
                                            "OR storage_place_id LIKE '{%}' "
                                            "LIMIT 1"),
                              QStringLiteral("Nie udało się sprawdzić UUID w tabeli eksponaty"))
           || hasMatchingRows(QStringLiteral("SELECT id FROM photos WHERE id LIKE '{%}' OR eksponat_id LIKE '{%}' LIMIT 1"),
                              QStringLiteral("Nie udało się sprawdzić UUID w tabeli photos"));
}

bool DatabaseMigration::disableForeignKeyChecks()
{
    QSqlQuery query(db);
    if (db.driverName() == "QMYSQL") {
        return query.exec("SET FOREIGN_KEY_CHECKS = 0");
    } else {
        return query.exec("PRAGMA foreign_keys = OFF");
    }
}

bool DatabaseMigration::enableForeignKeyChecks()
{
    QSqlQuery query(db);
    if (db.driverName() == "QMYSQL") {
        return query.exec("SET FOREIGN_KEY_CHECKS = 1");
    } else {
        return query.exec("PRAGMA foreign_keys = ON");
    }
}

bool DatabaseMigration::updateStatusesTable()
{
    return updateBracedIds(QStringLiteral("SELECT id FROM statuses WHERE id LIKE '{%}'"),
                           QStringLiteral("UPDATE statuses SET id = ? WHERE id = ?"),
                           QStringLiteral("UPDATE eksponaty SET status_id = ? WHERE status_id = ?"),
                           QStringLiteral("statusów"),
                           QStringLiteral("statusów w tabeli eksponatów"));
}

bool DatabaseMigration::updateStoragePlacesTable()
{
    return updateBracedIds(QStringLiteral("SELECT id FROM storage_places WHERE id LIKE '{%}'"),
                           QStringLiteral("UPDATE storage_places SET id = ? WHERE id = ?"),
                           QStringLiteral("UPDATE eksponaty SET storage_place_id = ? WHERE storage_place_id = ?"),
                           QStringLiteral("miejsc przechowywania"),
                           QStringLiteral("miejsc przechowywania w tabeli eksponatów"));
}

bool DatabaseMigration::updateEksponatyTable()
{
    return updateBracedIds(QStringLiteral("SELECT id FROM eksponaty WHERE id LIKE '{%}'"),
                           QStringLiteral("UPDATE eksponaty SET id = ? WHERE id = ?"),
                           QStringLiteral("UPDATE photos SET eksponat_id = ? WHERE eksponat_id = ?"),
                           QStringLiteral("eksponatów"),
                           QStringLiteral("eksponatów w tabeli zdjęć"));
}

bool DatabaseMigration::updatePhotosTable()
{
    return updateBracedIds(QStringLiteral("SELECT id FROM photos WHERE id LIKE '{%}'"),
                           QStringLiteral("UPDATE photos SET id = ? WHERE id = ?"),
                           QString(),
                           QStringLiteral("zdjęć"));
}

bool DatabaseMigration::verifyMigration()
{
    return verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) FROM statuses WHERE id LIKE '{%}'"),
                                  QStringLiteral("statuses"))
           && verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) FROM storage_places WHERE id LIKE '{%}'"),
                                     QStringLiteral("storage_places"))
           && verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) FROM eksponaty "
                                                    "WHERE id LIKE '{%}' OR status_id LIKE '{%}' OR storage_place_id LIKE '{%}'"),
                                     QStringLiteral("eksponaty"))
           && verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) FROM photos WHERE id LIKE '{%}' OR eksponat_id LIKE '{%}'"),
                                     QStringLiteral("photos"));
}

QString DatabaseMigration::removeBraces(const QString &uuid)
{
    QString result = uuid;
    result.remove('{');
    result.remove('}');
    return result;
}

bool DatabaseMigration::hasMigrationTables() const
{
    static const QStringList requiredTables = {
        QStringLiteral("statuses"),
        QStringLiteral("storage_places"),
        QStringLiteral("eksponaty"),
        QStringLiteral("photos")
    };

    const QStringList existingTables = db.tables();
    for (const QString &tableName : requiredTables) {
        if (!existingTables.contains(tableName, Qt::CaseInsensitive))
            return false;
    }

    return true;
}

bool DatabaseMigration::hasMatchingRows(const QString &sql, const QString &errorContext)
{
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qDebug() << errorContext << query.lastError().text();
        return false;
    }

    return query.next();
}

bool DatabaseMigration::executeStatement(const QString &sql,
                                         const QList<QVariant> &bindValues,
                                         const QString &errorContext,
                                         int *affectedRows)
{
    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant &bindValue : bindValues)
        query.addBindValue(bindValue);

    if (!query.exec()) {
        qDebug() << errorContext << query.lastError().text();
        return false;
    }

    if (affectedRows)
        *affectedRows = query.numRowsAffected();
    return true;
}

bool DatabaseMigration::updateBracedIds(const QString &selectSql,
                                        const QString &updatePrimarySql,
                                        const QString &updateReferenceSql,
                                        const QString &entityLabel,
                                        const QString &referenceLabel)
{
    QSqlQuery query(db);
    if (!query.exec(selectSql)) {
        qDebug() << "Nie udało się pobrać rekordów do migracji" << entityLabel << query.lastError().text();
        return false;
    }

    while (query.next()) {
        const QString oldId = query.value(0).toString();
        const QString newId = removeBraces(oldId);

        if (!executeStatement(updatePrimarySql,
                              {newId, oldId},
                              QStringLiteral("Nie udało się zaktualizować UUID dla %1").arg(entityLabel))) {
            return false;
        }

        if (!updateReferenceSql.isEmpty()
            && !executeStatement(updateReferenceSql,
                                 {newId, oldId},
                                 QStringLiteral("Nie udało się zaktualizować referencji %1").arg(referenceLabel))) {
            return false;
        }
    }

    return true;
}

bool DatabaseMigration::fixBrokenUuidValue(const QString &tableName,
                                           const QString &oldId,
                                           const QString &newId,
                                           const QString &entityLabel,
                                           const QString &referenceTable,
                                           const QString &referenceColumn,
                                           const QString &referenceLabel)
{
    int affectedRows = 0;
    if (!executeStatement(QStringLiteral("UPDATE %1 SET id = ? WHERE id = ?").arg(tableName),
                          {newId, oldId},
                          QStringLiteral("Błąd podczas naprawy UUID %1:").arg(entityLabel),
                          &affectedRows)) {
        return false;
    }

    if (affectedRows <= 0)
        return true;

    qDebug() << "Zaktualizowano" << entityLabel << "(" << affectedRows << " rekord)";

    if (referenceTable.isEmpty() || referenceColumn.isEmpty())
        return true;

    int updatedReferences = 0;
    if (!executeStatement(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %2 = ?").arg(referenceTable, referenceColumn),
                          {newId, oldId},
                          QStringLiteral("Błąd podczas aktualizacji referencji %1:").arg(referenceLabel),
                          &updatedReferences)) {
        return false;
    }

    qDebug() << "Zaktualizowano referencje" << referenceLabel << "(" << updatedReferences << " rekordów)";
    return true;
}

bool DatabaseMigration::verifyNoMalformedUuids(const QString &sql, const QString &label)
{
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qDebug() << "Weryfikacja migracji nie mogła wykonać zapytania dla" << label << ":"
                 << query.lastError().text();
        return false;
    }

    if (!query.next())
        return true;

    if (query.value(0).toInt() > 0) {
        qDebug() << "Weryfikacja nie powiodła się: znaleziono pozostałe UUID-y w tabeli" << label;
        return false;
    }

    return true;
}

bool DatabaseMigration::fixBrokenUUIDs()
{
    qDebug() << "Rozpoczynam naprawę uszkodzonych UUID-ów...";
    bool success = true;
    // Wyłącz sprawdzanie kluczy obcych
    if (!disableForeignKeyChecks()) {
        qDebug() << "Nie udało się wyłączyć sprawdzania kluczy obcych";
        return false;
    }

    success = fixBrokenUuidValue(QStringLiteral("statuses"),
                                 QStringLiteral("{b0ffafe8-a847-4b6f-bf65-54096df6ade"),
                                 QStringLiteral("b0ffafe8-a847-4b6f-bf65-54096df6ade"),
                                 QStringLiteral("status 'brak'"),
                                 QStringLiteral("eksponaty"),
                                 QStringLiteral("status_id"),
                                 QStringLiteral("do statusu 'brak'"))
              && success;

    success = fixBrokenUuidValue(QStringLiteral("statuses"),
                                 QStringLiteral("{42668524-b93e-4c88-ba04-0d0fbf7683e"),
                                 QStringLiteral("42668524-b93e-4c88-ba04-0d0fbf7683e"),
                                 QStringLiteral("status 'Sprawny'"),
                                 QStringLiteral("eksponaty"),
                                 QStringLiteral("status_id"),
                                 QStringLiteral("do statusu 'Sprawny'"))
              && success;

    success = fixBrokenUuidValue(QStringLiteral("statuses"),
                                 QStringLiteral("{62a1ee77-f482-4036-b979-7eb8bd9f4fb"),
                                 QStringLiteral("62a1ee77-f482-4036-b979-7eb8bd9f4fb"),
                                 QStringLiteral("status 'uszkodzony'"))
              && success;

    success = fixBrokenUuidValue(QStringLiteral("storage_places"),
                                 QStringLiteral("{c120da85-24f7-45f6-8df1-5a4245952ef"),
                                 QStringLiteral("c120da85-24f7-45f6-8df1-5a4245952ef"),
                                 QStringLiteral("miejsce przechowywania 'brak'"),
                                 QStringLiteral("eksponaty"),
                                 QStringLiteral("storage_place_id"),
                                 QStringLiteral("do miejsca przechowywania 'brak'"))
              && success;

    // Włącz z powrotem sprawdzanie kluczy obcych
    if (!enableForeignKeyChecks()) {
        qDebug() << "Nie udało się włączyć sprawdzania kluczy obcych";
        success = false;
    }

    success = verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) as count_status FROM statuses WHERE id LIKE '{%' AND id NOT LIKE '%}'"),
                                     QStringLiteral("statuses"))
              && success;
    success = verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) as count_storage FROM storage_places WHERE id LIKE '{%' AND id NOT LIKE '%}'"),
                                     QStringLiteral("storage_places"))
              && success;
    success = verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) as count_eksponaty FROM eksponaty "
                                                    "WHERE (status_id LIKE '{%' AND status_id NOT LIKE '%}') "
                                                    "OR (storage_place_id LIKE '{%' AND storage_place_id NOT LIKE '%}')"),
                                     QStringLiteral("eksponaty"))
              && success;
    success = verifyNoMalformedUuids(QStringLiteral("SELECT COUNT(*) as count_photos FROM photos "
                                                    "WHERE (id LIKE '{%' AND id NOT LIKE '%}') "
                                                    "OR (eksponat_id LIKE '{%' AND eksponat_id NOT LIKE '%}')"),
                                     QStringLiteral("photos"))
              && success;

    if (success) {
        qDebug() << "Naprawa UUID-ów zakończona sukcesem - wszystkie ID są teraz w poprawnym formacie";
    } else {
        qDebug() << "Naprawa UUID-ów zakończona z błędami - sprawdź logi powyżej";
    }

    return success;
} 
