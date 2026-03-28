#ifndef PHOTOSERVICE_H
#define PHOTOSERVICE_H

#include <QList>
#include <QPixmap>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

struct StoredPhoto
{
    QString id;
    QPixmap pixmap;
};

class PhotoService
{
public:
    explicit PhotoService(QSqlDatabase database = QSqlDatabase::database("default_connection"));

    QList<StoredPhoto> loadStoredPhotos(const QString &itemId, QString *errorMessage) const;
    QList<QPixmap> loadPixmapsFromBuffer(const QList<QByteArray> &photoBuffer) const;
    QStringList movePhotosToDone(const QStringList &photoPaths, bool shouldMove) const;

private:
    QSqlDatabase m_db;
};

#endif // PHOTOSERVICE_H
