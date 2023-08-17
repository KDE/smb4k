/*
 *  Print dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kprintdialog.h"

// Qt includes
#include <QDialogButtonBox>
#include <QVBoxLayout>

// KDE includes
#include <KLocalizedString>

Smb4KPrintDialog::Smb4KPrintDialog(QWidget* parent)
: QDialog(parent)
{
    setWindowTitle(i18n("Print Dialog"));
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);

    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_cancelButton->setShortcut(QKeySequence::Cancel);
    m_cancelButton->setDefault(true);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KPrintDialog::slotCancelButtonClicked);

    m_printButton = buttonBox->addButton(i18n("Print"), QDialogButtonBox::ActionRole);
    m_printButton->setShortcut(QKeySequence::Print);
    connect(m_printButton, &QPushButton::clicked, this, &Smb4KPrintDialog::slotPrintButtonClicked);

    layout->addWidget(buttonBox);
}

Smb4KPrintDialog::~Smb4KPrintDialog()
{
}

void Smb4KPrintDialog::setPrinterShare(SharePtr printer)
{
    if (!printer->isPrinter()) {
        return;
    }
}

void Smb4KPrintDialog::slotCancelButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    reject();
}

void Smb4KPrintDialog::slotPrintButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    accept();
}



