#ifndef DATABASEMIGRATION_H
#define DATABASEMIGRATION_H

#include <QObject>
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
    bool verifyMigration();
    bool addMissingColumns();
    bool fixBrokenUUIDs();
    
    // Funkcja pomocnicza do usuwania nawiasów klamrowych z UUID
    QString removeBraces(const QString &uuid);
    
    // Referencja do połączenia z bazą danych
    QSqlDatabase db;
};

#endif // DATABASEMIGRATION_H 