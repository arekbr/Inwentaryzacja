#ifndef ENRICHPREVIEWDIALOG_H
#define ENRICHPREVIEWDIALOG_H

#include <QDialog>
#include <QString>

class QLabel;
class QTextBrowser;

/// v1.5: Modal dialog pokazujący stary vs nowy opis (markdown render),
/// user wybiera "Zachowaj nowy" lub "Anuluj". Po `accept()` caller
/// dostaje newDescription() i robi UPDATE w bazie.
///
/// Layout: dwa QTextBrowser side-by-side (lewy = stary, prawy = nowy),
/// labele "Bez zmian" / "AI", buttonbox accept/reject u dołu, info o tokenach.
class EnrichPreviewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EnrichPreviewDialog(const QString &oldDescription,
                                 const QString &newDescription,
                                 qint64 inputTokens,
                                 qint64 outputTokens,
                                 QWidget *parent = nullptr);

    QString newDescription() const { return m_newDescription; }

private:
    QString m_newDescription;
};

#endif // ENRICHPREVIEWDIALOG_H
