#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QString>

namespace Ui
{
    class PreviewDialog;
}

class PreviewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreviewDialog(QSqlDatabase db, const QString &recordId, QWidget *parent = nullptr);
    ~PreviewDialog();

signals:
    void editRequested(const QString &recordId);

private:
    void loadRecord();

    Ui::PreviewDialog *ui;
    QSqlDatabase m_db;
    QString m_recordId;
};

#endif // PREVIEWDIALOG_H
