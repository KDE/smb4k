/*
 *  Synchronization dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4ksynchronizationdialog.h"
#include "core/smb4ksettings.h"
#include "core/smb4ksynchronizer.h"

// Qt includes
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>
#include <KUrlCompletion>
#include <KWindowConfig>

Smb4KSynchronizationDialog::Smb4KSynchronizationDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Synchronization"));
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *descriptionWidget = new QWidget(this);

    QHBoxLayout *descriptionLayout = new QHBoxLayout(descriptionWidget);
    descriptionLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("folder-sync")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignBottom);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionLayout->addWidget(descriptionPixmap);

    m_descriptionText = new QLabel(descriptionWidget);
    m_descriptionText->setText(i18n("Synchronization"));
    m_descriptionText->setWordWrap(true);
    m_descriptionText->setAlignment(Qt::AlignBottom);
    m_descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    descriptionLayout->addWidget(m_descriptionText);

    layout->addWidget(descriptionWidget);
    layout->addSpacing(layout->spacing());

    QWidget *inputWidget = new QWidget(this);

    QGridLayout *inputWidgetLayout = new QGridLayout(inputWidget);
    inputWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *sourceLabel = new QLabel(i18n("Source:"));

    m_sourceInput = new KUrlRequester(this);
    m_sourceInput->setMode(KFile::Directory | KFile::LocalOnly);
    m_sourceInput->lineEdit()->setSqueezedTextEnabled(true);
    m_sourceInput->completionObject()->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_sourceInput->completionObject()->setMode(KUrlCompletion::FileCompletion);

    connect(m_sourceInput, &KUrlRequester::textChanged, this, &Smb4KSynchronizationDialog::slotSourcePathChanged);

    QLabel *destinationLabel = new QLabel(i18n("Destination:"));

    m_destinationInput = new KUrlRequester(this);
    m_destinationInput->setMode(KFile::Directory | KFile::LocalOnly);
    m_destinationInput->lineEdit()->setSqueezedTextEnabled(true);
    m_destinationInput->completionObject()->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_destinationInput->completionObject()->setMode(KUrlCompletion::FileCompletion);

    connect(m_destinationInput, &KUrlRequester::textChanged, this, &Smb4KSynchronizationDialog::slotDestinationPathChanged);

    inputWidgetLayout->addWidget(sourceLabel, 0, 0);
    inputWidgetLayout->addWidget(m_sourceInput, 0, 1);
    inputWidgetLayout->addWidget(destinationLabel, 1, 0);
    inputWidgetLayout->addWidget(m_destinationInput, 1, 1);

    layout->addWidget(inputWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_swapButton = buttonBox->addButton(i18n("Swap Paths"), QDialogButtonBox::ActionRole);
    m_swapButton->setEnabled(false);
    m_synchronizeButton = buttonBox->addButton(i18n("Synchronize"), QDialogButtonBox::ActionRole);
    m_synchronizeButton->setEnabled(false);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_cancelButton->setShortcut(QKeySequence::Cancel);

    connect(m_swapButton, &QPushButton::clicked, this, &Smb4KSynchronizationDialog::slotSwapPaths);
    connect(m_synchronizeButton, &QPushButton::clicked, this, &Smb4KSynchronizationDialog::slotSynchronize);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KSynchronizationDialog::reject);

    layout->addWidget(buttonBox);

    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "SynchronizationDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KSynchronizationDialog::~Smb4KSynchronizationDialog()
{
}

bool Smb4KSynchronizationDialog::setShare(const SharePtr &share)
{
    if (share->isPrinter() && share->isInaccessible()) {
        return false;
    }

    m_descriptionText->setText(i18n("Please provide the source and destination directory for the synchronization of <b>%1</b>.", share->displayString()));

    m_sourceInput->setUrl(QUrl::fromLocalFile(share->path()));
    m_destinationInput->setUrl(
        QUrl::fromLocalFile(Smb4KSettings::rsyncPrefix().path() + QDir::separator() + share->hostName() + QDir::separator() + share->shareName())
            .adjusted(QUrl::NormalizePathSegments));

    m_synchronizeButton->setDefault(true);

    return true;
}

void Smb4KSynchronizationDialog::slotSourcePathChanged(const QString &path)
{
    Q_UNUSED(path);

    bool enable =
        !m_sourceInput->url().isEmpty() && m_sourceInput->url().isValid() && !m_destinationInput->url().isEmpty() && m_destinationInput->url().isValid();

    m_swapButton->setEnabled(enable);
    m_synchronizeButton->setEnabled(enable);
}

void Smb4KSynchronizationDialog::slotDestinationPathChanged(const QString &path)
{
    Q_UNUSED(path);

    bool enable =
        !m_sourceInput->url().isEmpty() && m_sourceInput->url().isValid() && !m_destinationInput->url().isEmpty() && m_destinationInput->url().isValid();

    m_swapButton->setEnabled(enable);
    m_synchronizeButton->setEnabled(enable);
}

void Smb4KSynchronizationDialog::slotSwapPaths()
{
    QUrl sourceUrl = m_sourceInput->url();
    QUrl destinationUrl = m_destinationInput->url();

    m_sourceInput->setUrl(destinationUrl);
    m_destinationInput->setUrl(sourceUrl);
}

void Smb4KSynchronizationDialog::slotSynchronize()
{
    Smb4KSynchronizer::self()->synchronize(m_sourceInput->url(), m_destinationInput->url());

    KConfigGroup group(Smb4KSettings::self()->config(), "SynchronizationDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    accept();
}
