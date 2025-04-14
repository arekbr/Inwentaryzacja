#ifndef MODELS_H
#define MODELS_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class models;
}

class models : public QDialog
{
    Q_OBJECT
public:
    explicit models(QWidget *parent = nullptr);
    ~models();

    void setMainWindow(MainWindow *mainWindow);
    void setVendorId(const QString &vendorId); // ZMIANA: był int

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onOkClicked();  // Slot dla przycisku OK – odświeża pole modeli i zamyka okno

private:
    void refreshList();

    Ui::models *ui;
    MainWindow *m_mainWindow;
    QSqlDatabase m_db;
    QString m_vendorId; // ZMIANA: było int
};

#endif // MODELS_H
