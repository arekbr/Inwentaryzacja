/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *New_item_PushButton_OK;
    QPushButton *New_item_PushButton_Cancel;
    QLineEdit *New_item_ID;
    QLineEdit *New_item_name;
    QComboBox *New_item_type;
    QComboBox *New_item_vendor;
    QComboBox *New_item_model;
    QLineEdit *New_item_serialNumber;
    QDateEdit *New_item_ProductionDate;
    QComboBox *New_item_status;
    QComboBox *New_item_storagePlace;
    QPlainTextEdit *New_item_description;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;
    QPushButton *New_item_addPhoto;
    QGraphicsView *graphicsView;
    QLineEdit *New_item_partNumber;
    QLabel *label_11;
    QLineEdit *New_item_revision;
    QLabel *label_12;
    QLineEdit *New_item_value;
    QLabel *label_13;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1029, 958);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        New_item_PushButton_OK = new QPushButton(centralwidget);
        New_item_PushButton_OK->setObjectName("New_item_PushButton_OK");
        New_item_PushButton_OK->setGeometry(QRect(480, 330, 100, 32));
        New_item_PushButton_Cancel = new QPushButton(centralwidget);
        New_item_PushButton_Cancel->setObjectName("New_item_PushButton_Cancel");
        New_item_PushButton_Cancel->setGeometry(QRect(630, 320, 100, 32));
        New_item_ID = new QLineEdit(centralwidget);
        New_item_ID->setObjectName("New_item_ID");
        New_item_ID->setGeometry(QRect(30, 40, 113, 21));
        New_item_name = new QLineEdit(centralwidget);
        New_item_name->setObjectName("New_item_name");
        New_item_name->setGeometry(QRect(30, 90, 113, 21));
        New_item_type = new QComboBox(centralwidget);
        New_item_type->setObjectName("New_item_type");
        New_item_type->setGeometry(QRect(30, 140, 103, 32));
        New_item_vendor = new QComboBox(centralwidget);
        New_item_vendor->setObjectName("New_item_vendor");
        New_item_vendor->setGeometry(QRect(30, 210, 103, 32));
        New_item_model = new QComboBox(centralwidget);
        New_item_model->setObjectName("New_item_model");
        New_item_model->setGeometry(QRect(30, 270, 103, 32));
        New_item_serialNumber = new QLineEdit(centralwidget);
        New_item_serialNumber->setObjectName("New_item_serialNumber");
        New_item_serialNumber->setGeometry(QRect(20, 340, 113, 21));
        New_item_ProductionDate = new QDateEdit(centralwidget);
        New_item_ProductionDate->setObjectName("New_item_ProductionDate");
        New_item_ProductionDate->setGeometry(QRect(20, 390, 110, 22));
        New_item_status = new QComboBox(centralwidget);
        New_item_status->setObjectName("New_item_status");
        New_item_status->setGeometry(QRect(20, 440, 103, 32));
        New_item_storagePlace = new QComboBox(centralwidget);
        New_item_storagePlace->setObjectName("New_item_storagePlace");
        New_item_storagePlace->setGeometry(QRect(20, 500, 103, 32));
        New_item_description = new QPlainTextEdit(centralwidget);
        New_item_description->setObjectName("New_item_description");
        New_item_description->setGeometry(QRect(240, 360, 104, 71));
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(30, 20, 131, 16));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(30, 70, 58, 16));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(30, 120, 58, 16));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(30, 190, 71, 16));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(30, 250, 91, 16));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(20, 310, 101, 16));
        label_7 = new QLabel(centralwidget);
        label_7->setObjectName("label_7");
        label_7->setGeometry(QRect(20, 370, 91, 16));
        label_8 = new QLabel(centralwidget);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(20, 420, 58, 16));
        label_9 = new QLabel(centralwidget);
        label_9->setObjectName("label_9");
        label_9->setGeometry(QRect(10, 480, 161, 16));
        label_10 = new QLabel(centralwidget);
        label_10->setObjectName("label_10");
        label_10->setGeometry(QRect(250, 340, 58, 16));
        New_item_addPhoto = new QPushButton(centralwidget);
        New_item_addPhoto->setObjectName("New_item_addPhoto");
        New_item_addPhoto->setGeometry(QRect(440, 60, 100, 32));
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName("graphicsView");
        graphicsView->setGeometry(QRect(610, 30, 256, 192));
        New_item_partNumber = new QLineEdit(centralwidget);
        New_item_partNumber->setObjectName("New_item_partNumber");
        New_item_partNumber->setGeometry(QRect(220, 50, 113, 26));
        label_11 = new QLabel(centralwidget);
        label_11->setObjectName("label_11");
        label_11->setGeometry(QRect(220, 20, 91, 18));
        New_item_revision = new QLineEdit(centralwidget);
        New_item_revision->setObjectName("New_item_revision");
        New_item_revision->setGeometry(QRect(210, 140, 113, 26));
        label_12 = new QLabel(centralwidget);
        label_12->setObjectName("label_12");
        label_12->setGeometry(QRect(210, 110, 66, 18));
        New_item_value = new QLineEdit(centralwidget);
        New_item_value->setObjectName("New_item_value");
        New_item_value->setGeometry(QRect(220, 230, 113, 26));
        label_13 = new QLabel(centralwidget);
        label_13->setObjectName("label_13");
        label_13->setGeometry(QRect(220, 200, 66, 18));
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        New_item_PushButton_OK->setText(QCoreApplication::translate("MainWindow", "Zapisz", nullptr));
        New_item_PushButton_Cancel->setText(QCoreApplication::translate("MainWindow", "Anuluj", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Numer (autoID)", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Nazwa", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Typ", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Producent", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Model", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Numer seryjny", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Data produkcji", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Status", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "Miejsce przechowywania", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "Opis", nullptr));
        New_item_addPhoto->setText(QCoreApplication::translate("MainWindow", "Dodaj zdj\304\231cia", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Part number", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Revision", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "Ilo\305\233\304\207", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
