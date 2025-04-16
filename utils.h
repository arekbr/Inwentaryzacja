#ifndef UTILS_H
#define UTILS_H

#include <QString>

// Deklaracja funkcji
bool setupDatabase(const QString &dbType,
                   const QString &dbSource,
                   const QString &host = QString(),
                   const QString &user = QString(),
                   const QString &password = QString(),
                   int port = 0);

#endif // UTILS_H
