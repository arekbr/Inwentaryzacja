/**
 * @file utils.cpp
 * @brief Implementacja funkcji pomocniczej `setupDatabase` odpowiedzialnej za konfigurację połączenia z bazą danych.
 */

#include "utils.h"
#include "DatabaseMigration.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

bool setupDatabase(const QString &dbType,
                   const QString &dbSource,
                   const QString &host,
                   const QString &user,
                   const QString &password,
                   int port)
{
    QSqlDatabase::removeDatabase("default_connection");
    QSqlDatabase db = QSqlDatabase::addDatabase(dbType.compare("MySQL", Qt::CaseInsensitive) == 0
                                                    ? "QMYSQL"
                                                    : "QSQLITE",
                                                "default_connection");

    if (dbType.compare("MySQL", Qt::CaseInsensitive) == 0) {
        db.setHostName(host);
        db.setDatabaseName(dbSource);
        db.setUserName(user);
        db.setPassword(password);
        db.setPort(port);
    } else {
        db.setDatabaseName(dbSource);
    }

    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Błąd połączenia"), db.lastError().text());
        return false;
    }

    if (db.driverName() == "QSQLITE") {
        QSqlQuery pragmaQuery(db);
        pragmaQuery.exec("PRAGMA foreign_keys = ON");
    }

    DatabaseMigration migration;
    if (!migration.migrateUUIDs()) {
        qDebug() << "Ostrzeżenie: Migracja UUID nie powiodła się";
    }

    return ensureDatabaseSchema(db);
}
