/********************************************************************************
** Form generated from reading UI file 'itemList.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ITEMLIST_H
#define UI_ITEMLIST_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_itemList
{
public:
    QTableView *itemList_tableView;
    QPushButton *itemList_pushButton_new;
    QGraphicsView *itemList_graphicsView;
    QPushButton *itemList_pushButton_edit;
    QPushButton *itemList_pushButton_end;

    void setupUi(QWidget *itemList)
    {
        if (itemList->objectName().isEmpty())
            itemList->setObjectName("itemList");
        itemList->resize(1353, 618);
        itemList_tableView = new QTableView(itemList);
        itemList_tableView->setObjectName("itemList_tableView");
        itemList_tableView->setGeometry(QRect(20, 90, 1321, 192));
        itemList_pushButton_new = new QPushButton(itemList);
        itemList_pushButton_new->setObjectName("itemList_pushButton_new");
        itemList_pushButton_new->setGeometry(QRect(350, 330, 87, 26));
        itemList_graphicsView = new QGraphicsView(itemList);
        itemList_graphicsView->setObjectName("itemList_graphicsView");
        itemList_graphicsView->setGeometry(QRect(30, 330, 256, 192));
        itemList_pushButton_edit = new QPushButton(itemList);
        itemList_pushButton_edit->setObjectName("itemList_pushButton_edit");
        itemList_pushButton_edit->setGeometry(QRect(470, 330, 87, 26));
        itemList_pushButton_end = new QPushButton(itemList);
        itemList_pushButton_end->setObjectName("itemList_pushButton_end");
        itemList_pushButton_end->setGeometry(QRect(350, 410, 87, 26));

        retranslateUi(itemList);

        QMetaObject::connectSlotsByName(itemList);
    } // setupUi

    void retranslateUi(QWidget *itemList)
    {
        itemList->setWindowTitle(QCoreApplication::translate("itemList", "Form", nullptr));
        itemList_pushButton_new->setText(QCoreApplication::translate("itemList", "Nowy", nullptr));
        itemList_pushButton_edit->setText(QCoreApplication::translate("itemList", "Edycja", nullptr));
        itemList_pushButton_end->setText(QCoreApplication::translate("itemList", "Koniec", nullptr));
    } // retranslateUi

};

namespace Ui {
    class itemList: public Ui_itemList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ITEMLIST_H
