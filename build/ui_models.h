/********************************************************************************
** Form generated from reading UI file 'models.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODELS_H
#define UI_MODELS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_models
{
public:
    QLineEdit *lineEdit;
    QLabel *label;

    void setupUi(QWidget *models)
    {
        if (models->objectName().isEmpty())
            models->setObjectName("models");
        models->resize(400, 300);
        lineEdit = new QLineEdit(models);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setGeometry(QRect(40, 60, 113, 26));
        label = new QLabel(models);
        label->setObjectName("label");
        label->setGeometry(QRect(40, 30, 66, 18));

        retranslateUi(models);

        QMetaObject::connectSlotsByName(models);
    } // setupUi

    void retranslateUi(QWidget *models)
    {
        models->setWindowTitle(QCoreApplication::translate("models", "Form", nullptr));
        label->setText(QCoreApplication::translate("models", "Model", nullptr));
    } // retranslateUi

};

namespace Ui {
    class models: public Ui_models {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODELS_H
