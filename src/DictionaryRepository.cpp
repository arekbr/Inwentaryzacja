#include "DictionaryRepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {

QString formatDbError(const QString &context, const QString &details)
{
    return QObject::tr("%1\n%2").arg(context, details);
}

}

DictionaryRepository::DictionaryRepository(QSqlDatabase database)
    : m_db(database)
{
}

bool DictionaryRepository::addEntry(const QString &tableName,
                                    const QString &name,
                                    QString *errorMessage,
                                    const QString &parentColumn,
                                    const QString &parentId)
{
    QSqlQuery query(m_db);

    if (!parentColumn.isEmpty()) {
        query.prepare(QString("INSERT INTO %1 (id, name, %2) VALUES (:id, :name, :parentId)")
                          .arg(tableName, parentColumn));
        query.bindValue(":parentId", parentId);
    } else {
        query.prepare(QString("INSERT INTO %1 (id, name) VALUES (:id, :name)").arg(tableName));
    }

    query.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.bindValue(":name", name);

    if (!query.exec()) {
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się dodać wpisu do słownika."),
                                          query.lastError().text());
        return false;
    }

    if (errorMessage)
        errorMessage->clear();
    return true;
}

bool DictionaryRepository::renameEntry(const QString &tableName,
                                       const QString &currentName,
                                       const QString &newName,
                                       QString *errorMessage)
{
    QSqlQuery query(m_db);
    query.prepare(QString("SELECT id FROM %1 WHERE name = :name").arg(tableName));
    query.bindValue(":name", currentName);
    if (!query.exec()) {
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się odczytać wpisu słownika do edycji."),
                                          query.lastError().text());
        return false;
    }

    if (!query.next()) {
        if (errorMessage)
            *errorMessage = QObject::tr("Nie znaleziono rekordu do edycji.");
        return false;
    }

    const QString id = query.value(0).toString();
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare(QString("UPDATE %1 SET name = :newName WHERE id = :id").arg(tableName));
    updateQuery.bindValue(":newName", newName);
    updateQuery.bindValue(":id", id);
    if (!updateQuery.exec()) {
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się zmienić nazwy wpisu słownika."),
                                          updateQuery.lastError().text());
        return false;
    }

    if (errorMessage)
        errorMessage->clear();
    return true;
}

bool DictionaryRepository::deleteEntry(const QString &tableName,
                                       const QString &name,
                                       QString *errorMessage,
                                       const QString &nameColumn)
{
    QSqlQuery query(m_db);
    query.prepare(QString("DELETE FROM %1 WHERE %2 = :name").arg(tableName, nameColumn));
    query.bindValue(":name", name);
    if (!query.exec()) {
        if (errorMessage)
            *errorMessage = formatDbError(QObject::tr("Nie udało się usunąć wpisu ze słownika."),
                                          query.lastError().text());
        return false;
    }

    if (errorMessage)
        errorMessage->clear();
    return true;
}
