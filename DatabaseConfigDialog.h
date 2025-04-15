#ifndef DATABASECONFIGDIALOG_H
#define DATABASECONFIGDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class DatabaseConfigDialog;
}

class DatabaseConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DatabaseConfigDialog(QWidget *parent = nullptr);
    ~DatabaseConfigDialog();

    QString selectedDatabaseType() const;
    QString sqliteFilePath() const;
    QString mysqlHost() const;
    QString mysqlDatabase() const;
    QString mysqlUser() const;
    QString mysqlPassword() const;
    int mysqlPort() const;

public slots:
    // Dodajemy deklarację metody accept() jako override funkcji z QDialog.
    void accept() override;

private slots:
    void on_dbTypeComboBox_currentIndexChanged(int index);

private:
    Ui::DatabaseConfigDialog *ui;
};

#endif // DATABASECONFIGDIALOG_H
