#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QWidget>
#include <QItemSelection>
#include <QSqlRelationalTableModel>
#include <QLabel> // Dodane dla QLabel w oknie podglądu

class PhotoItem; // Deklaracja zapowiadająca klasy PhotoItem

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
    void onRecordSaved(int recordId);
    void refreshList(int recordId = -1);
    void onPhotoHovered(PhotoItem *item);   // NOWY slot: Najechanie na zdjęcie
    void onPhotoUnhovered(PhotoItem *item); // NOWY slot: Opuszczenie zdjęcia

private:
    Ui::itemList *ui;
    QSqlRelationalTableModel *model;
    int m_currentRecordId;
    QWidget *m_previewWindow; // NOWE: Okno podglądu powiększonego zdjęcia
};

#endif // ITEMLIST_H
