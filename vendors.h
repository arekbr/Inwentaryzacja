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
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onOkClicked();  // Slot dla przycisku OK – odświeża pole producenta i zamyka okno

private:
    void refreshList();

    Ui::vendors *ui;
    MainWindow *m_mainWindow;
    QSqlDatabase m_db;
};

#endif // VENDORS_H
