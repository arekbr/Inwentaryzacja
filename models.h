#ifndef MODELS_H
#define MODELS_H

#include <QDialog> // Zamiast QWidget
#include <QSqlDatabase>

// Deklaracja wstępna klasy MainWindow
class MainWindow;

namespace Ui {
class models;
}

class models : public QDialog // Zamiast QWidget
{
    Q_OBJECT

public:
    explicit models(QWidget *parent = nullptr);
    ~models();
    void setMainWindow(MainWindow *mainWindow);
    void setVendorId(int vendorId);

private slots:
    void onSaveClicked();

private:
    Ui::models *ui;
    MainWindow *m_mainWindow;
    QSqlDatabase m_db;
    int m_vendorId;
};

#endif // MODELS_H
