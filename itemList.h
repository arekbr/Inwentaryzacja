#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QWidget>

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

private:
    Ui::itemList *ui;
    QSqlTableModel *model;
};

#endif // ITEMLIST_H
