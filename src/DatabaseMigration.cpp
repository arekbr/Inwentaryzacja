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

    // Sprawdź czy migracja jest potrzebna
    if (!checkForBracedUUIDs()) {
        qDebug() << "Nie znaleziono UUID-ów z nawiasami klamrowymi, migracja nie jest potrzebna.";
        return success;
    }

    // Jeśli są UUID-y do migracji, wykonaj migrację
    if (checkForBracedUUIDs()) {
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
    QSqlQuery query(db);
    
    // Sprawdź tabelę statusów
    query.exec("SELECT id FROM statuses WHERE id LIKE '{%}' LIMIT 1");
    if (query.next()) return true;
    
    // Sprawdź tabelę miejsc przechowywania
    query.exec("SELECT id FROM storage_places WHERE id LIKE '{%}' LIMIT 1");
    if (query.next()) return true;
    
    // Sprawdź referencje w tabeli eksponatów
    query.exec("SELECT id FROM eksponaty WHERE status_id LIKE '{%}' OR storage_place_id LIKE '{%}' LIMIT 1");
    if (query.next()) return true;
    
    return false;
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
    QSqlQuery query(db);
    QSqlQuery updateQuery(db);
    
    // Pobierz wszystkie rekordy statusów z UUID-ami w nawiasach
    query.exec("SELECT id, name FROM statuses WHERE id LIKE '{%}'");
    
    while (query.next()) {
        QString oldId = query.value("id").toString();
        QString newId = removeBraces(oldId);
        QString name = query.value("name").toString();
        
        // Zaktualizuj rekord statusu
        updateQuery.prepare("UPDATE statuses SET id = ? WHERE id = ?");
        updateQuery.addBindValue(newId);
        updateQuery.addBindValue(oldId);
        
        if (!updateQuery.exec()) {
            qDebug() << "Nie udało się zaktualizować statusu:" << name << "Błąd:" << updateQuery.lastError().text();
            return false;
        }
        
        // Zaktualizuj referencje w tabeli eksponatów
        updateQuery.prepare("UPDATE eksponaty SET status_id = ? WHERE status_id = ?");
        updateQuery.addBindValue(newId);
        updateQuery.addBindValue(oldId);
        
        if (!updateQuery.exec()) {
            qDebug() << "Nie udało się zaktualizować referencji do statusu w tabeli eksponatów dla:" << name;
            return false;
        }
    }
    
    return true;
}

bool DatabaseMigration::updateStoragePlacesTable()
{
    QSqlQuery query(db);
    QSqlQuery updateQuery(db);
    
    // Pobierz wszystkie rekordy miejsc przechowywania z UUID-ami w nawiasach
    query.exec("SELECT id, name FROM storage_places WHERE id LIKE '{%}'");
    
    while (query.next()) {
        QString oldId = query.value("id").toString();
        QString newId = removeBraces(oldId);
        QString name = query.value("name").toString();
        
        // Zaktualizuj rekord miejsca przechowywania
        updateQuery.prepare("UPDATE storage_places SET id = ? WHERE id = ?");
        updateQuery.addBindValue(newId);
        updateQuery.addBindValue(oldId);
        
        if (!updateQuery.exec()) {
            qDebug() << "Nie udało się zaktualizować miejsca przechowywania:" << name << "Błąd:" << updateQuery.lastError().text();
            return false;
        }
        
        // Zaktualizuj referencje w tabeli eksponatów
        updateQuery.prepare("UPDATE eksponaty SET storage_place_id = ? WHERE storage_place_id = ?");
        updateQuery.addBindValue(newId);
        updateQuery.addBindValue(oldId);
        
        if (!updateQuery.exec()) {
            qDebug() << "Nie udało się zaktualizować referencji do miejsca przechowywania w tabeli eksponatów dla:" << name;
            return false;
        }
    }
    
    return true;
}

bool DatabaseMigration::updateEksponatyTable()
{
    QSqlQuery query(db);
    QSqlQuery updateQuery(db);
    
    // Pobierz wszystkie rekordy eksponatów z UUID-ami w nawiasach
    query.exec("SELECT id FROM eksponaty WHERE id LIKE '{%}'");
    
    while (query.next()) {
        QString oldId = query.value("id").toString();
        QString newId = removeBraces(oldId);
        
        // Zaktualizuj rekord eksponatu
        updateQuery.prepare("UPDATE eksponaty SET id = ? WHERE id = ?");
        updateQuery.addBindValue(newId);
        updateQuery.addBindValue(oldId);
        
        if (!updateQuery.exec()) {
            qDebug() << "Nie udało się zaktualizować ID eksponatu:" << oldId << "Błąd:" << updateQuery.lastError().text();
            return false;
        }
        
        // Zaktualizuj referencje w tabeli zdjęć
        updateQuery.prepare("UPDATE photos SET eksponat_id = ? WHERE eksponat_id = ?");
        updateQuery.addBindValue(newId);
        updateQuery.addBindValue(oldId);
        
        if (!updateQuery.exec()) {
            qDebug() << "Nie udało się zaktualizować referencji do eksponatu w tabeli zdjęć dla:" << oldId;
            return false;
        }
    }
    
    return true;
}

bool DatabaseMigration::verifyMigration()
{
    QSqlQuery query(db);
    
    // Sprawdź czy pozostały jakieś UUID-y z nawiasami w tabelach
    QStringList checks = {
        "SELECT COUNT(*) FROM statuses WHERE id LIKE '{%}'",
        "SELECT COUNT(*) FROM storage_places WHERE id LIKE '{%}'",
        "SELECT COUNT(*) FROM eksponaty WHERE id LIKE '{%}' OR status_id LIKE '{%}' OR storage_place_id LIKE '{%}'",
        "SELECT COUNT(*) FROM photos WHERE id LIKE '{%}' OR eksponat_id LIKE '{%}'"
    };
    
    for (const QString &check : checks) {
        query.exec(check);
        if (query.next() && query.value(0).toInt() > 0) {
            qDebug() << "Weryfikacja nie powiodła się: Znaleziono pozostałe UUID-y z nawiasami";
            return false;
        }
    }
    
    return true;
}

QString DatabaseMigration::removeBraces(const QString &uuid)
{
    QString result = uuid;
    result.remove('{');
    result.remove('}');
    return result;
}

bool DatabaseMigration::fixBrokenUUIDs()
{
    qDebug() << "Rozpoczynam naprawę uszkodzonych UUID-ów...";
    bool success = true;
    QSqlQuery query(db);

    // Wyłącz sprawdzanie kluczy obcych
    if (!disableForeignKeyChecks()) {
        qDebug() << "Nie udało się wyłączyć sprawdzania kluczy obcych";
        return false;
    }

    // 1. Naprawa statusu "brak"
    query.prepare("UPDATE statuses SET id = 'b0ffafe8-a847-4b6f-bf65-54096df6ade' "
                 "WHERE id = '{b0ffafe8-a847-4b6f-bf65-54096df6ade'");
    if (!query.exec()) {
        qDebug() << "Błąd podczas naprawy UUID statusu 'brak':" << query.lastError().text();
        success = false;
    } else if (query.numRowsAffected() > 0) {
        qDebug() << "Zaktualizowano status 'brak' (" << query.numRowsAffected() << " rekord)";
        
        query.prepare("UPDATE eksponaty SET status_id = 'b0ffafe8-a847-4b6f-bf65-54096df6ade' "
                     "WHERE status_id = '{b0ffafe8-a847-4b6f-bf65-54096df6ade'");
        if (!query.exec()) {
            qDebug() << "Błąd podczas aktualizacji referencji do statusu 'brak':" << query.lastError().text();
            success = false;
        } else {
            qDebug() << "Zaktualizowano referencje do statusu 'brak' (" << query.numRowsAffected() << " rekordów)";
        }
    }

    // 2. Naprawa statusu "Sprawny"
    query.prepare("UPDATE statuses SET id = '42668524-b93e-4c88-ba04-0d0fbf7683e' "
                 "WHERE id = '{42668524-b93e-4c88-ba04-0d0fbf7683e'");
    if (!query.exec()) {
        qDebug() << "Błąd podczas naprawy UUID statusu 'Sprawny':" << query.lastError().text();
        success = false;
    } else if (query.numRowsAffected() > 0) {
        qDebug() << "Zaktualizowano status 'Sprawny' (" << query.numRowsAffected() << " rekord)";
        
        query.prepare("UPDATE eksponaty SET status_id = '42668524-b93e-4c88-ba04-0d0fbf7683e' "
                     "WHERE status_id = '{42668524-b93e-4c88-ba04-0d0fbf7683e'");
        if (!query.exec()) {
            qDebug() << "Błąd podczas aktualizacji referencji do statusu 'Sprawny':" << query.lastError().text();
            success = false;
        } else {
            qDebug() << "Zaktualizowano referencje do statusu 'Sprawny' (" << query.numRowsAffected() << " rekordów)";
        }
    }

    // 3. Naprawa statusu "uszkodzony" (bez referencji w eksponatach)
    query.prepare("UPDATE statuses SET id = '62a1ee77-f482-4036-b979-7eb8bd9f4fb' "
                 "WHERE id = '{62a1ee77-f482-4036-b979-7eb8bd9f4fb'");
    if (!query.exec()) {
        qDebug() << "Błąd podczas naprawy UUID statusu 'uszkodzony':" << query.lastError().text();
        success = false;
    } else if (query.numRowsAffected() > 0) {
        qDebug() << "Zaktualizowano status 'uszkodzony' (" << query.numRowsAffected() << " rekord)";
    }

    // 4. Naprawa miejsca przechowywania "brak"
    query.prepare("UPDATE storage_places SET id = 'c120da85-24f7-45f6-8df1-5a4245952ef' "
                 "WHERE id = '{c120da85-24f7-45f6-8df1-5a4245952ef'");
    if (!query.exec()) {
        qDebug() << "Błąd podczas naprawy UUID miejsca przechowywania 'brak':" << query.lastError().text();
        success = false;
    } else if (query.numRowsAffected() > 0) {
        qDebug() << "Zaktualizowano miejsce przechowywania 'brak' (" << query.numRowsAffected() << " rekord)";
        
        query.prepare("UPDATE eksponaty SET storage_place_id = 'c120da85-24f7-45f6-8df1-5a4245952ef' "
                     "WHERE storage_place_id = '{c120da85-24f7-45f6-8df1-5a4245952ef'");
        if (!query.exec()) {
            qDebug() << "Błąd podczas aktualizacji referencji do miejsca przechowywania 'brak':" << query.lastError().text();
            success = false;
        } else {
            qDebug() << "Zaktualizowano referencje do miejsca przechowywania 'brak' (" << query.numRowsAffected() << " rekordów)";
        }
    }

    // Włącz z powrotem sprawdzanie kluczy obcych
    if (!enableForeignKeyChecks()) {
        qDebug() << "Nie udało się włączyć sprawdzania kluczy obcych";
        success = false;
    }

    // Końcowa weryfikacja
    query.exec("SELECT COUNT(*) as count_status FROM statuses WHERE id LIKE '{%}' OR id LIKE '{%'");
    if (query.next() && query.value(0).toInt() > 0) {
        qDebug() << "UWAGA: Nadal istnieją niepoprawne UUID w tabeli statuses:" << query.value(0).toInt();
        success = false;
    }

    query.exec("SELECT COUNT(*) as count_storage FROM storage_places WHERE id LIKE '{%}' OR id LIKE '{%'");
    if (query.next() && query.value(0).toInt() > 0) {
        qDebug() << "UWAGA: Nadal istnieją niepoprawne UUID w tabeli storage_places:" << query.value(0).toInt();
        success = false;
    }

    query.exec("SELECT COUNT(*) as count_eksponaty FROM eksponaty WHERE status_id LIKE '{%}' OR storage_place_id LIKE '{%}'");
    if (query.next() && query.value(0).toInt() > 0) {
        qDebug() << "UWAGA: Nadal istnieją niepoprawne UUID w tabeli eksponaty:" << query.value(0).toInt();
        success = false;
    }

    if (success) {
        qDebug() << "Naprawa UUID-ów zakończona sukcesem - wszystkie ID są teraz w poprawnym formacie";
    } else {
        qDebug() << "Naprawa UUID-ów zakończona z błędami - sprawdź logi powyżej";
    }

    return success;
} 