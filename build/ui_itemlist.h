/********************************************************************************
** Form generated from reading UI file 'itemlist.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ITEMLIST_H
#define UI_ITEMLIST_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_itemList
{
public:
    QTableView *tableView;
    QPushButton *pushButton;

    void setupUi(QWidget *itemList)
    {
        if (itemList->objectName().isEmpty())
            itemList->setObjectName("itemList");
        itemList->resize(400, 300);
        tableView = new QTableView(itemList);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(0, 90, 391, 192));
        pushButton = new QPushButton(itemList);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(300, 30, 87, 26));

        retranslateUi(itemList);

        QMetaObject::connectSlotsByName(itemList);
    } // setupUi

    void retranslateUi(QWidget *itemList)
    {
        itemList->setWindowTitle(QCoreApplication::translate("itemList", "Form", nullptr));
        pushButton->setText(QCoreApplication::translate("itemList", "Nowy", nullptr));
    } // retranslateUi

};

namespace Ui {
    class itemList: public Ui_itemList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ITEMLIST_H
