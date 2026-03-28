#ifndef DATABASEMIGRATION_H
#define DATABASEMIGRATION_H

#include <QObject>
#include <QVariant>
#include <QSqlDatabase>
#include <QString>

class DatabaseMigration : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseMigration(QObject *parent = nullptr);
    
    // Główna funkcja migracji, która zarządza całym procesem
    bool migrateUUIDs();

private:
    // Funkcje pomocnicze dla procesu migracji
    bool checkForBracedUUIDs();
    bool disableForeignKeyChecks();
    bool enableForeignKeyChecks();
    bool updateStatusesTable();
    bool updateStoragePlacesTable();
    bool updateEksponatyTable();
    bool updatePhotosTable();
    bool verifyMigration();
    bool addMissingColumns();
    bool fixBrokenUUIDs();
    bool hasMigrationTables() const;
    bool hasMatchingRows(const QString &sql, const QString &errorContext);
    bool executeStatement(const QString &sql,
                          const QList<QVariant> &bindValues,
                          const QString &errorContext,
                          int *affectedRows = nullptr);
    bool updateBracedIds(const QString &selectSql,
                         const QString &updatePrimarySql,
                         const QString &updateReferenceSql,
                         const QString &entityLabel,
                         const QString &referenceLabel = QString());
    bool fixBrokenUuidValue(const QString &tableName,
                            const QString &oldId,
                            const QString &newId,
                            const QString &entityLabel,
                            const QString &referenceTable = QString(),
                            const QString &referenceColumn = QString(),
                            const QString &referenceLabel = QString());
    bool verifyNoMalformedUuids(const QString &sql, const QString &label);
    
    // Funkcja pomocnicza do usuwania nawiasów klamrowych z UUID
    QString removeBraces(const QString &uuid);
    
    // Referencja do połączenia z bazą danych
    QSqlDatabase db;
};

#endif // DATABASEMIGRATION_H 
