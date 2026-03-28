#ifndef DICTIONARYREPOSITORY_H
#define DICTIONARYREPOSITORY_H

#include <QSqlDatabase>
#include <QString>

class DictionaryRepository
{
public:
    explicit DictionaryRepository(QSqlDatabase database = QSqlDatabase::database("default_connection"));

    bool addEntry(const QString &tableName,
                  const QString &name,
                  QString *errorMessage,
                  const QString &parentColumn = QString(),
                  const QString &parentId = QString());

    bool renameEntry(const QString &tableName,
                     const QString &currentName,
                     const QString &newName,
                     QString *errorMessage);

    bool deleteEntry(const QString &tableName,
                     const QString &name,
                     QString *errorMessage,
                     const QString &nameColumn = QStringLiteral("name"));

private:
    QSqlDatabase m_db;
};

#endif // DICTIONARYREPOSITORY_H
