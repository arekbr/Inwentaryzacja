#ifndef STORAGE_H
#define STORAGE_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class storage;
}

class storage : public QDialog
{
    Q_OBJECT
public:
    explicit storage(QWidget *parent = nullptr);
    ~storage();
    void setMainWindow(MainWindow *mainWindow);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onOkClicked();

private:
    void refreshList();

    Ui::storage *ui;
    MainWindow *m_mainWindow;
    QSqlDatabase m_db;
};

#endif // STORAGE_H
