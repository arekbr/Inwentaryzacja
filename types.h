#ifndef TYPES_H
#define TYPES_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class typy;
}

class typy : public QDialog
{
    Q_OBJECT
public:
    explicit typy(QWidget *parent = nullptr);
    ~typy();
    void setMainWindow(MainWindow *mainWindow);

private slots:
    void onSaveClicked();

private:
    Ui::typy *ui;
    MainWindow *m_mainWindow;
    QSqlDatabase m_db;
};

#endif // TYPES_H
