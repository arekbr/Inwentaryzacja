/********************************************************************************
** Form generated from reading UI file 'vendors.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VENDORS_H
#define UI_VENDORS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_vendors
{
public:
    QLabel *label;
    QLineEdit *lineEdit;

    void setupUi(QWidget *vendors)
    {
        if (vendors->objectName().isEmpty())
            vendors->setObjectName("vendors");
        vendors->resize(400, 300);
        label = new QLabel(vendors);
        label->setObjectName("label");
        label->setGeometry(QRect(70, 70, 81, 18));
        lineEdit = new QLineEdit(vendors);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setGeometry(QRect(70, 110, 113, 26));

        retranslateUi(vendors);

        QMetaObject::connectSlotsByName(vendors);
    } // setupUi

    void retranslateUi(QWidget *vendors)
    {
        vendors->setWindowTitle(QCoreApplication::translate("vendors", "Form", nullptr));
        label->setText(QCoreApplication::translate("vendors", "Producent", nullptr));
    } // retranslateUi

};

namespace Ui {
    class vendors: public Ui_vendors {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VENDORS_H
