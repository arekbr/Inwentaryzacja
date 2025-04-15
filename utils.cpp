#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>

bool setupDatabase(const QString &dbType,
                   const QString &dbSource,   // Dla SQLite: ścieżka do pliku; dla MySQL: nazwa bazy
                   const QString &host = QString(),
                   const QString &user = QString(),
                   const QString &password = QString(),
                   int port = 0)
{
    QSqlDatabase db;
    // Jeśli połączenie o nazwie "default_connection" już istnieje, je pobieramy.
    if (QSqlDatabase::contains("default_connection"))
        db = QSqlDatabase::database("default_connection");
    else {
        if(dbType == "SQLite3") {
            db = QSqlDatabase::addDatabase("QSQLITE", "default_connection");
            db.setDatabaseName(dbSource);
        } else if(dbType == "MySQL") {
            db = QSqlDatabase::addDatabase("QMYSQL", "default_connection");
            db.setHostName(host);
            db.setDatabaseName(dbSource);
            db.setUserName(user);
            db.setPassword(password);
            if(port != 0)
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
