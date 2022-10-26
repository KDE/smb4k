/*
    Private helpers for the homes shares handler

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4khomesshareshandler_p.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KConfigGui/KWindowConfig>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

Smb4KHomesUsers::Smb4KHomesUsers(const SharePtr &s, const QStringList &u)
{
    m_workgroupName = s->workgroupName();
    m_hostName = s->hostName();
    m_shareName = s->shareName();
    m_hostIp.setAddress(s->hostIpAddress());
    m_userList = u;
}

Smb4KHomesUsers::Smb4KHomesUsers(const Smb4KHomesUsers &u)
{
    m_workgroupName = u.workgroupName();
    m_hostName = u.hostName();
    m_shareName = u.shareName();
    m_hostIp.setAddress(u.hostIP());
    m_userList = u.userList();
    m_profile = u.profile();
}

Smb4KHomesUsers::Smb4KHomesUsers()
{
}

Smb4KHomesUsers::~Smb4KHomesUsers()
{
}

QString Smb4KHomesUsers::workgroupName() const
{
    return m_workgroupName;
}

void Smb4KHomesUsers::setWorkgroupName(const QString &name)
{
    m_workgroupName = name;
}

QString Smb4KHomesUsers::hostName() const
{
    return m_hostName;
}

void Smb4KHomesUsers::setHostName(const QString &name)
{
    m_hostName = name;
}

QString Smb4KHomesUsers::shareName() const
{
    return m_shareName;
}

void Smb4KHomesUsers::setShareName(const QString &name)
{
    m_shareName = name;
}

QString Smb4KHomesUsers::hostIP() const
{
    return m_hostIp.toString();
}

void Smb4KHomesUsers::setHostIP(const QString &ip)
{
    m_hostIp.setAddress(ip);
}

QStringList Smb4KHomesUsers::userList() const
{
    return m_userList;
}

void Smb4KHomesUsers::setUserList(const QStringList &users)
{
    m_userList = users;
}

QString Smb4KHomesUsers::profile() const
{
    return m_profile;
}

void Smb4KHomesUsers::setProfile(const QString &profile)
{
    m_profile = profile;
}

Smb4KHomesUserDialog::Smb4KHomesUserDialog(const SharePtr &share, QWidget *parent)
    : QDialog(parent)
    , m_share(share)
{
    //
    // Set the window title
    //
    setWindowTitle(i18n("Specify User"));

    //
    // Setup the view
    //
    setupView();

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "HomesUserDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    //
    // Fill the completion object
    //
    m_user_combo->completionObject()->setItems(group.readEntry("HomesUsersCompletion", QStringList()));
}

Smb4KHomesUserDialog::~Smb4KHomesUserDialog()
{
}

void Smb4KHomesUserDialog::setupView()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *description = new QWidget(this);

    QHBoxLayout *descriptionLayout = new QHBoxLayout(description);
    descriptionLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *pixmap = new QLabel(description);
    QPixmap user_pix = KDE::icon(QStringLiteral("user-identity")).pixmap(KIconLoader::SizeHuge);
    pixmap->setPixmap(user_pix);
    pixmap->setAlignment(Qt::AlignBottom);

    QLabel *label = new QLabel(i18n("Please specify a username for share<br><b>%1</b>.", m_share->displayString()), description);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignBottom);

    descriptionLayout->addWidget(pixmap, 0);
    descriptionLayout->addWidget(label, Qt::AlignBottom);

    QWidget *input = new QWidget(this);

    QGridLayout *inputLayout = new QGridLayout(input);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setColumnStretch(0, 0);
    inputLayout->setColumnStretch(1, 1);

    QLabel *input_label = new QLabel(i18n("User:"), input);

    m_user_combo = new KComboBox(true, input);
    m_user_combo->setDuplicatesEnabled(false);
    m_user_combo->setEditable(true);

    inputLayout->addWidget(input_label, 0, 0);
    inputLayout->addWidget(m_user_combo, 0, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    m_clear_button = buttonBox->addButton(i18n("Clear List"), QDialogButtonBox::ActionRole);
    m_clear_button->setIcon(KDE::icon(QStringLiteral("edit-clear")));
    m_clear_button->setEnabled(false);
    m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
    m_ok_button->setEnabled(false);
    m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);

    m_ok_button->setShortcut(Qt::CTRL | Qt::Key_Return);
    m_cancel_button->setShortcut(Qt::Key_Escape);

    m_ok_button->setDefault(true);

    layout->addWidget(description, 0);
    layout->addWidget(input, 0);
    layout->addWidget(buttonBox, 0);

    m_user_combo->setFocus();

    connect(m_user_combo, SIGNAL(currentTextChanged(QString)), SLOT(slotTextChanged(QString)));
    connect(m_user_combo->lineEdit(), SIGNAL(editingFinished()), SLOT(slotHomesUserEntered()));
    connect(m_clear_button, SIGNAL(clicked()), SLOT(slotClearClicked()));
    connect(m_ok_button, SIGNAL(clicked()), SLOT(slotOkClicked()));
    connect(m_cancel_button, SIGNAL(clicked()), SLOT(reject()));
}

void Smb4KHomesUserDialog::setUserNames(const QStringList &users)
{
    if (!users.isEmpty()) {
        m_user_combo->addItems(users);
        m_user_combo->setCurrentItem(QStringLiteral(""));
        m_clear_button->setEnabled(true);
    }
}

QStringList Smb4KHomesUserDialog::userNames()
{
    QStringList users;

    for (int i = 0; i < m_user_combo->count(); ++i) {
        users << m_user_combo->itemText(i);
    }

    if (!users.contains(m_user_combo->currentText())) {
        users << m_user_combo->currentText();
    }

    return users;
}

void Smb4KHomesUserDialog::slotTextChanged(const QString &text)
{
    m_ok_button->setEnabled(!text.isEmpty());
}

void Smb4KHomesUserDialog::slotClearClicked()
{
    m_user_combo->clearEditText();
    m_user_combo->clear();
    m_clear_button->setEnabled(false);
}

void Smb4KHomesUserDialog::slotOkClicked()
{
    KConfigGroup group(Smb4KSettings::self()->config(), "HomesUserDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.writeEntry("HomesUsersCompletion", m_user_combo->completionObject()->items());
    accept();
}

void Smb4KHomesUserDialog::slotHomesUserEntered()
{
    KCompletion *completion = m_user_combo->completionObject();

    if (!m_user_combo->currentText().isEmpty()) {
        completion->addItem(m_user_combo->currentText());
    }
}
