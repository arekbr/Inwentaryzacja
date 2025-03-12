#ifndef VENDORS_H
#define VENDORS_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class vendors;
}

class vendors : public QDialog
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
