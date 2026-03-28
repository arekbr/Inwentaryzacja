/**
 * @file DatabaseSchemaUtils.cpp
 * @brief Implementacja przygotowania i doszczelniania schematu bazy danych.
 */

#include "utils.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {

bool execSchemaQuery(QSqlQuery &query, const QString &sql, const char *context)
{
    if (!query.exec(sql)) {
        qDebug() << context << query.lastError().text();
        return false;
    }
    return true;
}

bool ensureHasOriginalPackagingColumn(QSqlDatabase &db)
{
    QSqlQuery query(db);

    if (db.driverName() == "QSQLITE") {
        if (!query.exec("PRAGMA table_info(eksponaty)")) {
            qDebug() << "Błąd sprawdzania kolumn SQLite dla eksponaty:" << query.lastError().text();
            return false;
        }

        while (query.next()) {
            if (query.value("name").toString() == "has_original_packaging")
                return true;
        }

        return execSchemaQuery(query,
                               "ALTER TABLE eksponaty ADD COLUMN has_original_packaging INTEGER DEFAULT 0",
                               "Błąd dodawania kolumny has_original_packaging w SQLite:");
    }

    if (!query.exec("SELECT COUNT(*) FROM information_schema.columns "
                    "WHERE table_schema = DATABASE() "
                    "AND table_name = 'eksponaty' "
                    "AND column_name = 'has_original_packaging'")) {
        qDebug() << "Błąd sprawdzania kolumny has_original_packaging w MySQL:" << query.lastError().text();
        return false;
    }

    if (query.next() && query.value(0).toInt() > 0)
        return true;

    return execSchemaQuery(query,
                           "ALTER TABLE eksponaty ADD COLUMN has_original_packaging BOOLEAN DEFAULT 0",
                           "Błąd dodawania kolumny has_original_packaging w MySQL:");
}

bool ensurePhotoIndex(QSqlDatabase &db)
{
    QSqlQuery query(db);

    if (db.driverName() == "QSQLITE") {
        return execSchemaQuery(query,
                               "CREATE INDEX IF NOT EXISTS idx_photos_eksponat_id ON photos(eksponat_id)",
                               "Błąd tworzenia indeksu zdjęć (SQLite):");
    }

    if (!query.exec("SELECT COUNT(*) FROM information_schema.statistics "
                    "WHERE table_schema = DATABASE() "
                    "AND table_name = 'photos' "
                    "AND index_name = 'idx_photos_eksponat_id'")) {
        qDebug() << "Błąd sprawdzania indeksu zdjęć w MySQL:" << query.lastError().text();
        return false;
    }

    if (query.next() && query.value(0).toInt() > 0)
        return true;

    return execSchemaQuery(query,
                           "CREATE INDEX idx_photos_eksponat_id ON photos (eksponat_id)",
                           "Błąd tworzenia indeksu zdjęć (MySQL):");
}

bool seedDictionaryData(QSqlDatabase &db)
{
    QSqlQuery query(db);
    const QString insertPrefix = db.driverName() == "QSQLITE" ? "INSERT OR IGNORE" : "INSERT IGNORE";
    auto genId = []() { return QUuid::createUuid().toString(QUuid::WithoutBraces); };

    const QString t1 = genId(), t2 = genId(), t3 = genId();
    if (!execSchemaQuery(query,
                         QString("%1 INTO types(id,name) VALUES('%2','Komputer')").arg(insertPrefix, t1),
                         "Błąd dodawania typu Komputer:")
        || !execSchemaQuery(query,
                            QString("%1 INTO types(id,name) VALUES('%2','Konsola')").arg(insertPrefix, t2),
                            "Błąd dodawania typu Konsola:")
        || !execSchemaQuery(query,
                            QString("%1 INTO types(id,name) VALUES('%2','Akcesorium')")
                                .arg(insertPrefix, t3),
                            "Błąd dodawania typu Akcesorium:")) {
        return false;
    }

    const QString v1 = genId(), v2 = genId(), v3 = genId();
    if (!execSchemaQuery(query,
                         QString("%1 INTO vendors(id,name) VALUES('%2','Atari')").arg(insertPrefix, v1),
                         "Błąd dodawania producenta Atari:")
        || !execSchemaQuery(query,
                            QString("%1 INTO vendors(id,name) VALUES('%2','Commodore')")
                                .arg(insertPrefix, v2),
                            "Błąd dodawania producenta Commodore:")
        || !execSchemaQuery(query,
                            QString("%1 INTO vendors(id,name) VALUES('%2','Sinclair')")
                                .arg(insertPrefix, v3),
                            "Błąd dodawania producenta Sinclair:")) {
        return false;
    }

    const QString m1 = genId(), m2 = genId(), m3 = genId();
    if (!execSchemaQuery(query,
                         QString("%1 INTO models(id,name,vendor_id) VALUES('%2','Atari 800XL','%3')")
                             .arg(insertPrefix, m1, v1),
                         "Błąd dodawania modelu Atari 800XL:")
        || !execSchemaQuery(query,
                            QString("%1 INTO models(id,name,vendor_id) VALUES('%2','Amiga 500','%3')")
                                .arg(insertPrefix, m2, v2),
                            "Błąd dodawania modelu Amiga 500:")
        || !execSchemaQuery(query,
                            QString("%1 INTO models(id,name,vendor_id) VALUES('%2','ZX Spectrum','%3')")
                                .arg(insertPrefix, m3, v3),
                            "Błąd dodawania modelu ZX Spectrum:")) {
        return false;
    }

    const QString s0 = genId(), s1 = genId(), s2 = genId(), s3 = genId();
    if (!execSchemaQuery(query,
                         QString("%1 INTO statuses(id,name) VALUES('%2','brak')").arg(insertPrefix, s0),
                         "Błąd dodawania statusu brak:")
        || !execSchemaQuery(query,
                            QString("%1 INTO statuses(id,name) VALUES('%2','Sprawny')").arg(insertPrefix, s1),
                         "Błąd dodawania statusu Sprawny:")
        || !execSchemaQuery(query,
                            QString("%1 INTO statuses(id,name) VALUES('%2','Uszkodzony')").arg(insertPrefix, s2),
                            "Błąd dodawania statusu Uszkodzony:")
        || !execSchemaQuery(query,
                            QString("%1 INTO statuses(id,name) VALUES('%2','W naprawie')")
                                .arg(insertPrefix, s3),
                            "Błąd dodawania statusu W naprawie:")) {
        return false;
    }

    const QString sp0 = genId(), sp1 = genId(), sp2 = genId();
    return execSchemaQuery(query,
                           QString("%1 INTO storage_places(id,name) VALUES('%2','brak')")
                               .arg(insertPrefix, sp0),
                           "Błąd dodawania miejsca brak:")
           && execSchemaQuery(query,
                              QString("%1 INTO storage_places(id,name) VALUES('%2','Magazyn 1')")
                                  .arg(insertPrefix, sp1),
                              "Błąd dodawania miejsca Magazyn 1:")
           && execSchemaQuery(query,
                              QString("%1 INTO storage_places(id,name) VALUES('%2','Półka B3')")
                                  .arg(insertPrefix, sp2),
                              "Błąd dodawania miejsca Półka B3:");
}

bool seedMysqlSampleItems(QSqlDatabase &db)
{
    if (db.driverName() == "QSQLITE")
        return true;

    auto genId = []() { return QUuid::createUuid().toString(QUuid::WithoutBraces); };
    QSqlQuery query(db);

    const QString t1 = genId(), v1 = genId(), v2 = genId(), v3 = genId();
    const QString m1 = genId(), m2 = genId(), m3 = genId();
    const QString s1 = genId(), s2 = genId();
    const QString sp1 = genId(), sp2 = genId();

    if (!execSchemaQuery(query,
                         QString("INSERT IGNORE INTO types (id, name) VALUES ('%1','Komputer')").arg(t1),
                         "Błąd dodawania typu Komputer (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Atari')").arg(v1),
                            "Błąd dodawania producenta Atari (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Commodore')").arg(v2),
                            "Błąd dodawania producenta Commodore (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO vendors (id, name) VALUES ('%1','Sinclair')").arg(v3),
                            "Błąd dodawania producenta Sinclair (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','Atari 800XL','%2')")
                                .arg(m1, v1),
                            "Błąd dodawania modelu Atari 800XL (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','Amiga 500','%2')")
                                .arg(m2, v2),
                            "Błąd dodawania modelu Amiga 500 (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO models (id, name, vendor_id) VALUES ('%1','ZX Spectrum','%2')")
                                .arg(m3, v3),
                            "Błąd dodawania modelu ZX Spectrum (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','Sprawny')").arg(s1),
                            "Błąd dodawania statusu Sprawny (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO statuses (id, name) VALUES ('%1','Uszkodzony')").arg(s2),
                            "Błąd dodawania statusu Uszkodzony (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO storage_places (id, name) VALUES ('%1','Magazyn 1')")
                                .arg(sp1),
                            "Błąd dodawania miejsca Magazyn 1 (MySQL):")
        || !execSchemaQuery(query,
                            QString("INSERT IGNORE INTO storage_places (id, name) VALUES ('%1','Półka B3')")
                                .arg(sp2),
                            "Błąd dodawania miejsca Półka B3 (MySQL):")) {
        return false;
    }

    struct SampleItem {
        const char *name;
        const char *serial;
        int year;
        const char *description;
        int value;
    };

    const SampleItem sampleItems[] = {
        {"Atari 800XL", "AT800-001", 1983, "Klasyczny komputer Atari", 1},
        {"Amiga 500", "A500-1234", 1987, "Klasyczny komputer Amiga", 2},
        {"ZX Spectrum", "ZXS-4567", 1982, "Kultowy komputer Sinclair", 1},
    };

    const QString vendorIds[] = {v1, v2, v3};
    const QString modelIds[] = {m1, m2, m3};
    const QString statusIds[] = {s1, s1, s2};
    const QString storageIds[] = {sp1, sp1, sp2};

    for (int index = 0; index < 3; ++index) {
        query.prepare(R"(
            INSERT IGNORE INTO eksponaty
            (id, name, type_id, vendor_id, model_id, serial_number, production_year,
             status_id, storage_place_id, description, value)
            VALUES
            (:id, :name, :type_id, :vendor_id, :model_id, :serial, :year,
             :status, :storage, :desc, :val)
        )");
        query.bindValue(":id", genId());
        query.bindValue(":name", sampleItems[index].name);
        query.bindValue(":type_id", t1);
        query.bindValue(":vendor_id", vendorIds[index]);
        query.bindValue(":model_id", modelIds[index]);
        query.bindValue(":serial", sampleItems[index].serial);
        query.bindValue(":year", sampleItems[index].year);
        query.bindValue(":status", statusIds[index]);
        query.bindValue(":storage", storageIds[index]);
        query.bindValue(":desc", sampleItems[index].description);
        query.bindValue(":val", sampleItems[index].value);
        if (!query.exec()) {
            qDebug() << "Błąd dodawania przykładowego eksponatu (MySQL):" << query.lastError().text();
            return false;
        }
    }

    return true;
}

}

bool ensureDatabaseSchema(QSqlDatabase &db)
{
    if (!db.isOpen()) {
        qDebug() << "Próba przygotowania schematu dla zamkniętej bazy danych.";
        return false;
    }

    QSqlQuery query(db);
    const bool isSqlite = db.driverName() == "QSQLITE";
    const QStringList requiredTables = {"eksponaty", "types", "vendors", "models", "statuses", "storage_places", "photos"};
    const QStringList existingTables = db.tables();
    bool schemaMissing = false;

    for (const QString &tableName : requiredTables) {
        if (!existingTables.contains(tableName, Qt::CaseInsensitive)) {
            schemaMissing = true;
            break;
        }
    }

    if (isSqlite) {
        if (!execSchemaQuery(query, "PRAGMA foreign_keys = ON", "Błąd włączania kluczy obcych SQLite:"))
            return false;

        if (schemaMissing) {
            bool schemaOk = true;
            auto execOrTrack = [&query, &schemaOk](const QString &sql, const char *context)
            {
                if (!execSchemaQuery(query, sql, context)) {
                    schemaOk = false;
                    return false;
                }
                return true;
            };

            execOrTrack(R"(
                CREATE TABLE IF NOT EXISTS eksponaty (
                  id TEXT PRIMARY KEY,
                  name TEXT NOT NULL,
                  type_id TEXT NOT NULL,
                  vendor_id TEXT NOT NULL,
                  model_id TEXT NOT NULL,
                  serial_number TEXT,
                  part_number TEXT,
                  revision TEXT,
                  production_year INTEGER,
                  status_id TEXT NOT NULL,
                  storage_place_id TEXT NOT NULL,
                  description TEXT,
                  value INTEGER,
                  has_original_packaging INTEGER DEFAULT 0
                )
            )",
                        "Błąd tworzenia tabeli eksponaty (SQLite):");
            execOrTrack(R"(
                CREATE TABLE IF NOT EXISTS types (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )",
                        "Błąd tworzenia tabeli types (SQLite):");
            execOrTrack(R"(
                CREATE TABLE IF NOT EXISTS vendors (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )",
                        "Błąd tworzenia tabeli vendors (SQLite):");
            execOrTrack(R"(
                CREATE TABLE IF NOT EXISTS models (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL,
                  vendor_id TEXT NOT NULL
                )
            )",
                        "Błąd tworzenia tabeli models (SQLite):");
            execOrTrack(R"(
                CREATE TABLE IF NOT EXISTS statuses (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )",
                        "Błąd tworzenia tabeli statuses (SQLite):");
            execOrTrack(R"(
                CREATE TABLE IF NOT EXISTS storage_places (
                  id TEXT PRIMARY KEY,
                  name TEXT UNIQUE NOT NULL
                )
            )",
                        "Błąd tworzenia tabeli storage_places (SQLite):");
            execOrTrack(R"(
                CREATE TABLE IF NOT EXISTS photos (
                  id TEXT PRIMARY KEY,
                  eksponat_id TEXT NOT NULL,
                  photo BLOB NOT NULL,
                  FOREIGN KEY (eksponat_id) REFERENCES eksponaty(id) ON DELETE CASCADE
                )
            )",
                        "Błąd tworzenia tabeli photos (SQLite):");

            if (!schemaOk)
                return false;

            if (!seedDictionaryData(db))
                return false;
        }
    } else {
        bool schemaOk = true;
        auto execOrTrack = [&query, &schemaOk](const QString &sql, const char *context)
        {
            if (!execSchemaQuery(query, sql, context)) {
                schemaOk = false;
                return false;
            }
            return true;
        };

        execOrTrack(R"(
            CREATE TABLE IF NOT EXISTS eksponaty (
                id VARCHAR(36) PRIMARY KEY,
                name TEXT NOT NULL,
                type_id VARCHAR(36) NOT NULL,
                vendor_id VARCHAR(36) NOT NULL,
                model_id VARCHAR(36) NOT NULL,
                serial_number TEXT,
                part_number TEXT,
                revision TEXT,
                production_year INT,
                status_id VARCHAR(36) NOT NULL,
                storage_place_id VARCHAR(36) NOT NULL,
                description TEXT,
                value INT,
                has_original_packaging BOOLEAN DEFAULT 0
            )
        )",
                    "Błąd tworzenia tabeli eksponaty (MySQL):");
        execOrTrack(R"(
            CREATE TABLE IF NOT EXISTS types (
                id VARCHAR(36) PRIMARY KEY,
                name TEXT UNIQUE NOT NULL
            )
        )",
                    "Błąd tworzenia tabeli types (MySQL):");
        execOrTrack(R"(
            CREATE TABLE IF NOT EXISTS vendors (
                id VARCHAR(36) PRIMARY KEY,
                name TEXT UNIQUE NOT NULL
            )
        )",
                    "Błąd tworzenia tabeli vendors (MySQL):");
        execOrTrack(R"(
            CREATE TABLE IF NOT EXISTS models (
                id VARCHAR(36) PRIMARY KEY,
                name TEXT UNIQUE NOT NULL,
                vendor_id VARCHAR(36) NOT NULL
            )
        )",
                    "Błąd tworzenia tabeli models (MySQL):");
        execOrTrack(R"(
            CREATE TABLE IF NOT EXISTS statuses (
                id VARCHAR(36) PRIMARY KEY,
                name TEXT UNIQUE NOT NULL
            )
        )",
                    "Błąd tworzenia tabeli statuses (MySQL):");
        execOrTrack(R"(
            CREATE TABLE IF NOT EXISTS storage_places (
                id VARCHAR(36) PRIMARY KEY,
                name TEXT UNIQUE NOT NULL
            )
        )",
                    "Błąd tworzenia tabeli storage_places (MySQL):");
        execOrTrack(R"(
            CREATE TABLE IF NOT EXISTS photos (
                id VARCHAR(36) PRIMARY KEY,
                eksponat_id VARCHAR(36) NOT NULL,
                photo LONGBLOB NOT NULL,
                CONSTRAINT fk_photos_eksponat
                    FOREIGN KEY (eksponat_id) REFERENCES eksponaty(id)
                    ON DELETE CASCADE
            )
        )",
                    "Błąd tworzenia tabeli photos (MySQL):");

        if (!schemaOk)
            return false;

        if (!seedDictionaryData(db))
            return false;

        if (schemaMissing && !seedMysqlSampleItems(db))
            return false;
    }

    return ensureHasOriginalPackagingColumn(db) && ensurePhotoIndex(db);
}
