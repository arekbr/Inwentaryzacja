#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QComboBox>
#include <QItemSelection>
#include <QLabel>
#include <QSettings>
#include <QSqlRelationalTableModel>
#include <QWidget>
#include "ItemFilterProxyModel.h"
#include "photoitem.h"

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
    void onAboutClicked();
    void onTableViewSelectionChanged(const QItemSelection &selected,
                                     const QItemSelection &deselected);
    void onRecordSaved(const QString &recordId);
    void refreshList(const QString &recordId = QString());
    void onPhotoHovered(PhotoItem *item);
    void onPhotoUnhovered(PhotoItem *item);
    void onPhotoClicked(PhotoItem *item);
    void onCloneButtonClicked();
    void onFilterChanged();            // slot reagujący na zmianę któregoś filtra

private:
    bool verifyDatabaseSchema(QSqlDatabase &db);
    void createDatabaseSchema(QSqlDatabase &db);
    void insertSampleData(QSqlDatabase &db);
    void initFilters(QSqlDatabase &db);
    void refreshFilters();
    void updateFilterComboBoxes();     // metoda odbudowująca listy w ComboBoxach

    Ui::itemList *ui;
    QSqlRelationalTableModel *m_sourceModel; // oryginalny model danych
    ItemFilterProxyModel *m_proxyModel;      // model proxy dla filtrowania
    QComboBox *filterTypeComboBox;           // filtry
    QComboBox *filterVendorComboBox;
    QComboBox *filterModelComboBox;
    QComboBox *filterStatusComboBox;
    QComboBox *filterStorageComboBox;
    QString m_currentRecordId;
    QWidget *m_previewWindow;
};

#endif // ITEMLIST_H
