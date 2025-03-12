#ifndef VENDORS_H
#define VENDORS_H

#include <QDialog> // Zamiast QWidget
#include <QSqlDatabase>

// Deklaracja wstępna klasy MainWindow
class MainWindow;

namespace Ui {
class vendors;
}

class vendors : public QDialog // Zamiast QWidget
{
    Q_OBJECT

public:
    explicit vendors(QWidget *parent = nullptr);
    ~vendors();
    void setMainWindow(MainWindow *mainWindow);

private slots:
    void onSaveClicked();

private:
    Ui::vendors *ui;
    MainWindow *m_mainWindow;
    QSqlDatabase m_db;
};

#endif // VENDORS_H
