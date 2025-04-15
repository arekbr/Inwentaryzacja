#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>

// Funkcja inicjująca połączenie z bazą danych w zależności od typu.
// Parametry:
//   dbType: "SQLite3" lub "MySQL"
//   dbSource: dla SQLite – ścieżka do pliku bazy;
//             dla MySQL – nazwa bazy danych,
//   host, user, password, port – używane tylko dla MySQL.
inline bool setupDatabase(const QString &dbType,
                          const QString &dbSource,
                          const QString &host = QString(),
                          const QString &user = QString(),
                          const QString &password = QString(),
                          int port = 0)
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("default_connection"))
        db = QSqlDatabase::database("default_connection");
    else {
        if (dbType == "SQLite3") {
            db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");
            db.setDatabaseName(dbSource);
        } else if (dbType == "MySQL") {
            db = QSqlDatabase::addDatabase("QMYSQL", "default_connection");
            db.setHostName(host);
            db.setDatabaseName(dbSource);
            db.setUserName(user);
            db.setPassword(password);
            if (port != 0)
                db.setPort(port);
        } else {
            QMessageBox::critical(nullptr, QObject::tr("Błąd konfiguracji"),
                                  QObject::tr("Nieznany typ bazy danych: %1").arg(dbType));
            return false;
        }
    }
    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Błąd połączenia"),
                              db.lastError().text());
        return false;
    }
    return true;
}

#endif // UTILS_H
