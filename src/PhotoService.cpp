#include "PhotoService.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>

PhotoService::PhotoService(QSqlDatabase database)
    : m_db(database)
{
}

QList<StoredPhoto> PhotoService::loadStoredPhotos(const QString &itemId, QString *errorMessage) const
{
    QList<StoredPhoto> photos;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", itemId);
    if (!query.exec()) {
        if (errorMessage)
            *errorMessage = query.lastError().text();
        return photos;
    }

    while (query.next()) {
        QPixmap pixmap;
        if (!pixmap.loadFromData(query.value("photo").toByteArray())) {
            qDebug() << "Nie można załadować BLOB zdjęcia";
            continue;
        }

        StoredPhoto photo;
        photo.id = query.value("id").toString();
        photo.pixmap = pixmap;
        photos.append(photo);
    }

    if (errorMessage)
        errorMessage->clear();
    return photos;
}

QList<QPixmap> PhotoService::loadPixmapsFromBuffer(const QList<QByteArray> &photoBuffer) const
{
    QList<QPixmap> pixmaps;

    for (const QByteArray &photoData : photoBuffer) {
        QPixmap pixmap;
        if (!pixmap.loadFromData(photoData)) {
            qDebug() << "Błąd loadFromData w buforze zdjęć";
            continue;
        }
        pixmaps.append(pixmap);
    }

    return pixmaps;
}

QStringList PhotoService::movePhotosToDone(const QStringList &photoPaths, bool shouldMove) const
{
    QStringList moveFailures;
    if (!shouldMove)
        return moveFailures;

    for (const QString &orig : photoPaths) {
        QFileInfo fileInfo(orig);
        const QString doneDir = fileInfo.absolutePath() + QDir::separator() + QStringLiteral("gotowe");
        QDir().mkpath(doneDir);
        const QString destination = doneDir + QDir::separator() + fileInfo.fileName();
        if (!QFile::rename(orig, destination))
            moveFailures.append(fileInfo.fileName());
    }

    return moveFailures;
}
