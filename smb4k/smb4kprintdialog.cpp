/*
 *  Print dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kprintdialog.h"
#include "core/smb4kclient.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QDir>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGroup>
#include <KFileItem>
#include <KIconLoader>
#include <KLocalizedString>
#include <KWindowConfig>

Smb4KPrintDialog::Smb4KPrintDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Print Dialog"));
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *descriptionWidget = new QWidget(this);

    QHBoxLayout *descriptionWidgetLayout = new QHBoxLayout(descriptionWidget);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("printer")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignBottom);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionPixmap);

    m_descriptionText = new QLabel(this);
    m_descriptionText->setText(i18n("Print a file."));
    m_descriptionText->setWordWrap(true);
    m_descriptionText->setAlignment(Qt::AlignBottom);
    m_descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(m_descriptionText);

    layout->addWidget(descriptionWidget);

    QWidget *inputWidget = new QWidget(this);

    QGridLayout *inputWidgetLayout = new QGridLayout(inputWidget);
    inputWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *fileLabel = new QLabel(i18n("File:"), inputWidget);
    m_fileInput = new KUrlRequester(QUrl::fromLocalFile(QDir::homePath() + QDir::separator()), inputWidget);
    m_fileInput->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
    connect(m_fileInput, &KUrlComboRequester::textChanged, this, &Smb4KPrintDialog::slotUrlChanged);

    inputWidgetLayout->addWidget(fileLabel, 0, 0);
    inputWidgetLayout->addWidget(m_fileInput, 0, 1);

    QLabel *copiesLabel = new QLabel(i18n("Copies:"), inputWidget);
    m_copiesInput = new QSpinBox(inputWidget);
    m_copiesInput->setValue(1);
    m_copiesInput->setMinimum(1);
    connect(m_copiesInput, &QSpinBox::valueChanged, this, &Smb4KPrintDialog::slotCopiesChanged);

    inputWidgetLayout->addWidget(copiesLabel, 1, 0);
    inputWidgetLayout->addWidget(m_copiesInput, 1, 1);

    layout->addWidget(inputWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);

    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_cancelButton->setShortcut(QKeySequence::Cancel);
    m_cancelButton->setDefault(true);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KPrintDialog::slotCancelButtonClicked);

    m_printButton = buttonBox->addButton(i18n("Print"), QDialogButtonBox::ActionRole);
    m_printButton->setShortcut(QKeySequence::Print);
    m_printButton->setEnabled(false);
    connect(m_printButton, &QPushButton::clicked, this, &Smb4KPrintDialog::slotPrintButtonClicked);

    layout->addWidget(buttonBox);

    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KPrintDialog::~Smb4KPrintDialog()
{
}

bool Smb4KPrintDialog::setPrinterShare(SharePtr printer)
{
    if (!printer->isPrinter()) {
        return false;
    }

    m_descriptionText->setText(i18n("Print a file to printer <b>%1</b>.", printer->displayString()));

    m_printer = printer;

    return true;
}

void Smb4KPrintDialog::enablePrintButton()
{
    QUrl pathUrl = m_fileInput->url();
    int copies = m_copiesInput->value();

    m_printButton->setEnabled(pathUrl.isValid() && copies > 0);
}

void Smb4KPrintDialog::slotCancelButtonClicked(bool checked)
{
    Q_UNUSED(checked)
    reject();
}

void Smb4KPrintDialog::slotPrintButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    Smb4KClient::self()->printFile(m_printer, KFileItem(m_fileInput->url()), m_copiesInput->value());

    KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    accept();
}

void Smb4KPrintDialog::slotUrlChanged(const QString &path)
{
    Q_UNUSED(path)
    enablePrintButton();
}

void Smb4KPrintDialog::slotCopiesChanged(int copies)
{
    Q_UNUSED(copies)
    enablePrintButton();
}
