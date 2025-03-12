#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QWidget>
#include <QItemSelection>

class QSqlTableModel;

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
    void onTableViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void refreshList(int recordId = -1);
    void onRecordSaved(int recordId); // Nowy slot do pośredniczenia

private:
    Ui::itemList *ui;
    QSqlTableModel *model;
    int m_currentRecordId;
};

#endif // ITEMLIST_H
