#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QWidget>
#include <QItemSelection>
#include <QSqlRelationalTableModel>
#include <QLabel>
#include <QSettings>
#include "photoitem.h" // dla sygnałów hovered, clicked

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

    // Po zaznaczeniu wiersza w QTableView
    void onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // Po zapisie rekordu w MainWindow:
    void onRecordSaved(const QString &recordId);

    // Odświeża model i (ew.) zaznacza wskazany rekord:
    void refreshList(const QString &recordId = QString());

    // Obsługa hover:
    void onPhotoHovered(PhotoItem *item);
    void onPhotoUnhovered(PhotoItem *item);

    // Kliknięcie miniatury -> fullscreen
    void onPhotoClicked(PhotoItem *item);

    void onCloneButtonClicked();

private:
    bool verifyDatabaseSchema(QSqlDatabase &db);
    void createDatabaseSchema(QSqlDatabase &db);
    void insertSampleData(QSqlDatabase &db);

    Ui::itemList *ui;
    QSqlRelationalTableModel *model;
    QString m_currentRecordId;
    QWidget *m_previewWindow;
};

#endif // ITEMLIST_H
