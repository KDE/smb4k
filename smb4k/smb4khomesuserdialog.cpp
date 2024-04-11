/*
 *  Dialog to choose a user for a 'homes' share
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4khomesuserdialog.h"
#include "core/smb4khomesshareshandler.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGroup>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>
#include <KWindowConfig>

Smb4KHomesUserDialog::Smb4KHomesUserDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Specify User"));
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *descriptionWidget = new QWidget(this);
    QHBoxLayout *descriptionWidgetLayout = new QHBoxLayout(descriptionWidget);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("user")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignVCenter);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionPixmap);

    m_descriptionText = new QLabel(this);
    m_descriptionText->setText(i18n("Please specify a username."));
    m_descriptionText->setWordWrap(true);
    m_descriptionText->setAlignment(Qt::AlignVCenter);
    m_descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(m_descriptionText);

    layout->addWidget(descriptionWidget);
    layout->addSpacing(layout->spacing());

    QWidget *inputWidget = new QWidget(this);
    QGridLayout *inputWidgetLayout = new QGridLayout(inputWidget);
    inputWidgetLayout->setContentsMargins(0, 0, 0, 0);
    inputWidgetLayout->setColumnStretch(0, 0);
    inputWidgetLayout->setColumnStretch(1, 1);

    QLabel *userNameLabel = new QLabel(i18n("Username:"), inputWidget);

    m_userNameInput = new KComboBox(inputWidget);
    m_userNameInput->setEditable(true);

    connect(m_userNameInput->lineEdit(), &KLineEdit::editingFinished, this, &Smb4KHomesUserDialog::slotHomesUserNameEntered);
    connect(m_userNameInput, &KComboBox::currentTextChanged, this, &Smb4KHomesUserDialog::slotHomesUserNameChanged);

    inputWidgetLayout->addWidget(userNameLabel, 0, 0);
    inputWidgetLayout->addWidget(m_userNameInput, 0, 1);

    layout->addWidget(inputWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_okButton->setShortcut(QKeySequence::Save);
    m_okButton->setEnabled(false);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_cancelButton->setShortcut(QKeySequence::Cancel);
    m_cancelButton->setDefault(true);

    connect(m_okButton, &QPushButton::clicked, this, &Smb4KHomesUserDialog::slotOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KHomesUserDialog::reject);

    layout->addWidget(buttonBox);

    create();

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("HomesUserDialog"));
    QSize dialogSize;

    if (dialogGroup.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), QStringLiteral("CompletionItems"));

    if (completionGroup.exists()) {
        m_userNameInput->completionObject()->setItems(completionGroup.readEntry("HomesUsersCompletion", QStringList()));
    }
}

Smb4KHomesUserDialog::~Smb4KHomesUserDialog() noexcept
{
}

bool Smb4KHomesUserDialog::setShare(SharePtr homesShare)
{
    if (!homesShare->isHomesShare()) {
        return false;
    }

    m_share = homesShare;
    m_descriptionText->setText(i18n("Please specify a username for share<br><b>%1</b>.", m_share->displayString()));
    m_userNameInput->addItems(Smb4KHomesSharesHandler::self()->homesUsers(m_share));
    m_userNameInput->setCurrentItem(QStringLiteral(""));

    return true;
}

void Smb4KHomesUserDialog::slotHomesUserNameEntered()
{
    if (!m_userNameInput->currentText().isEmpty()) {
        m_userNameInput->completionObject()->addItem(m_userNameInput->currentText());
    }
}

void Smb4KHomesUserDialog::slotHomesUserNameChanged(const QString &text)
{
    m_okButton->setEnabled(!text.isEmpty());
}

void Smb4KHomesUserDialog::slotOkClicked()
{
    QString userName = m_userNameInput->currentText();

    if (!userName.isEmpty()) {
        if (!m_share->userName().isEmpty() && m_share->userName() != userName) {
            m_share->setPassword(QString());
        }

        m_share->setUserName(userName);
    }

    QStringList homesUsers;

    for (int i = 0; i < m_userNameInput->count(); ++i) {
        homesUsers << m_userNameInput->itemText(i);
    }

    if (!homesUsers.contains(m_userNameInput->currentText())) {
        homesUsers << m_userNameInput->currentText();
    }

    Smb4KHomesSharesHandler::self()->addHomesUsers(m_share, homesUsers);

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("HomesUserDialog"));
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), QStringLiteral("CompletionItems"));
    completionGroup.writeEntry("HomesUsersCompletion", m_userNameInput->completionObject()->items());

    accept();
}
