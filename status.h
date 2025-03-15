#ifndef STATUS_H
#define STATUS_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class status;
}

class status : public QDialog
{
    Q_OBJECT
public:
    explicit status(QWidget *parent = nullptr);
    ~status();
    void setMainWindow(MainWindow *mainWindow);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onOkClicked(); // Slot dla przycisku OK – odświeża pole statusów w głównym oknie

private:
    void refreshList();

    Ui::status *ui;
    MainWindow *m_mainWindow;
    QSqlDatabase m_db;
};

#endif // STATUS_H
