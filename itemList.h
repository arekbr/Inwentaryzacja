#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QWidget>
#include <QItemSelection>
#include <QSqlRelationalTableModel>
#include <QLabel>
#include <QSettings>

class PhotoItem;

namespace Ui {
class itemList;
}

class itemList : public QWidget
{
    Q_OBJECT
public:
    explicit itemList(QWidget *parent = nullptr);
    ~itemList();

private slots:
    void onNewButtonClicked();
    void onEditButtonClicked();
    void onEndButtonClicked();
    void onDeleteButtonClicked();
    void onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onRecordSaved(const QString &recordId);
    void refreshList(const QString &recordId = QString());
    void onPhotoHovered(PhotoItem *item);
    void onPhotoUnhovered(PhotoItem *item);
    void onCloneButtonClicked();

private:
    bool verifyDatabaseSchema(QSqlDatabase &db);
    void createDatabaseSchema(QSqlDatabase &db);
    void insertSampleData(QSqlDatabase &db);

    Ui::itemList *ui;
    QSqlRelationalTableModel *model;
    QString m_currentRecordId; // ZMIANA: było int
    QWidget *m_previewWindow;
};

#endif // ITEMLIST_H
