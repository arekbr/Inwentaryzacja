#include "EnrichPreviewDialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

EnrichPreviewDialog::EnrichPreviewDialog(const QString &oldDescription,
                                         const QString &newDescription,
                                         qint64 inputTokens,
                                         qint64 outputTokens,
                                         QWidget *parent)
    : QDialog(parent)
    , m_newDescription(newDescription)
{
    setWindowTitle(tr("Wzbogać opis AI — porównanie"));
    resize(1100, 700);

    auto *root = new QVBoxLayout(this);

    // Header info — token usage + cost estimate
    // Opus 4.7: $5/1M input, $25/1M output (per AiEnrichmentService::estimateCostUsd)
    const double cost = (inputTokens / 1'000'000.0) * 5.0
                      + (outputTokens / 1'000'000.0) * 25.0;
    auto *header = new QLabel(tr("Tokeny: %1 wejście, %2 wyjście  •  Szacunkowy koszt: $%3")
                              .arg(inputTokens).arg(outputTokens)
                              .arg(QString::number(cost, 'f', 4)),
                              this);
    header->setStyleSheet(QStringLiteral("color: #888; padding: 4px 8px; font-size: 11px;"));
    root->addWidget(header);

    // Side-by-side panes
    auto *paneRow = new QHBoxLayout;
    paneRow->setSpacing(8);

    // LEFT — stary opis
    auto *leftCol = new QVBoxLayout;
    auto *leftLabel = new QLabel(tr("📝 Obecny opis"), this);
    leftLabel->setStyleSheet(QStringLiteral("font-weight: bold; padding: 4px;"));
    leftCol->addWidget(leftLabel);
    auto *leftBrowser = new QTextBrowser(this);
    leftBrowser->setOpenExternalLinks(false);
    if (oldDescription.trimmed().isEmpty())
        leftBrowser->setPlainText(tr("(brak opisu)"));
    else
        leftBrowser->setMarkdown(oldDescription);
    leftCol->addWidget(leftBrowser);
    paneRow->addLayout(leftCol);

    // RIGHT — nowy opis (AI)
    auto *rightCol = new QVBoxLayout;
    auto *rightLabel = new QLabel(tr("🪄 Propozycja AI"), this);
    rightLabel->setStyleSheet(QStringLiteral("font-weight: bold; color: #2e7d32; padding: 4px;"));
    rightCol->addWidget(rightLabel);
    auto *rightBrowser = new QTextBrowser(this);
    rightBrowser->setOpenExternalLinks(false);
    rightBrowser->setMarkdown(newDescription);
    rightCol->addWidget(rightBrowser);
    paneRow->addLayout(rightCol);

    root->addLayout(paneRow, /*stretch*/ 1);

    // Button box — Accept / Reject (custom labels)
    auto *buttons = new QDialogButtonBox(this);
    auto *acceptBtn = buttons->addButton(tr("✓ Zapisz nowy opis"), QDialogButtonBox::AcceptRole);
    auto *rejectBtn = buttons->addButton(tr("✗ Zachowaj stary"), QDialogButtonBox::RejectRole);
    acceptBtn->setStyleSheet(QStringLiteral("background-color: #2e7d32; color: white; padding: 6px 12px;"));
    rejectBtn->setStyleSheet(QStringLiteral("padding: 6px 12px;"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}
