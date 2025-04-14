#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QWidget>
#include <QItemSelection>
#include <QSqlRelationalTableModel>
#include <QLabel>
#include <QSettings>
#include "photoitem.h" // aby klasa PhotoItem była znana (sygnał clicked)

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

    // Metoda wywoływana przy zaznaczeniu wiersza w QTableView
    void onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // Slot wywoływany, gdy okno edycji/dodawania zapisze rekord
    void onRecordSaved(const QString &recordId);

    // Odświeżenie listy (modelu) i ewentualne zaznaczenie jakiegoś ID
    void refreshList(const QString &recordId = QString());

    // Obsługa najechania myszką na PhotoItem
    void onPhotoHovered(PhotoItem *item);
    void onPhotoUnhovered(PhotoItem *item);

    // NOWY slot: obsługa kliknięcia w miniaturę
    void onPhotoClicked(PhotoItem *item);

    void onCloneButtonClicked();

private:
    bool verifyDatabaseSchema(QSqlDatabase &db);
    void createDatabaseSchema(QSqlDatabase &db);
    void insertSampleData(QSqlDatabase &db);

    Ui::itemList *ui;
    QSqlRelationalTableModel *model;
    QString m_currentRecordId;
    QWidget *m_previewWindow; // do podglądu w hover
};

#endif // ITEMLIST_H
