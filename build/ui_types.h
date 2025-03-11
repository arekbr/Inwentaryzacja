/********************************************************************************
** Form generated from reading UI file 'types.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TYPES_H
#define UI_TYPES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_typy
{
public:
    QLineEdit *type_lineEdit;
    QLabel *label;

    void setupUi(QWidget *typy)
    {
        if (typy->objectName().isEmpty())
            typy->setObjectName("typy");
        typy->resize(400, 300);
        type_lineEdit = new QLineEdit(typy);
        type_lineEdit->setObjectName("type_lineEdit");
        type_lineEdit->setGeometry(QRect(30, 60, 113, 26));
        label = new QLabel(typy);
        label->setObjectName("label");
        label->setGeometry(QRect(40, 40, 66, 18));

        retranslateUi(typy);

        QMetaObject::connectSlotsByName(typy);
    } // setupUi

    void retranslateUi(QWidget *typy)
    {
        typy->setWindowTitle(QCoreApplication::translate("typy", "Form", nullptr));
        label->setText(QCoreApplication::translate("typy", "Typ", nullptr));
    } // retranslateUi

};

namespace Ui {
    class typy: public Ui_typy {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TYPES_H
