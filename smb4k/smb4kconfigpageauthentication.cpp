/*
    The configuration page for the authentication settings of Smb4K

    SPDX-FileCopyrightText: 2003-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpageauthentication.h"
#include "core/smb4kcredentialsmanager.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes
#include <KLocalizedString>
#include <KPasswordLineEdit>

Smb4KConfigPageAuthentication::Smb4KConfigPageAuthentication(QWidget *parent)
    : QWidget(parent)
{
    m_defaultCredentialsModified = false;

    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox *defaultLoginCredentialsBox = new QGroupBox(i18n("Default Login"), this);
    QGridLayout *defaultLoginCredentialsBoxLayout = new QGridLayout(defaultLoginCredentialsBox);

    QLabel *defaultUserNameLabel = new QLabel(i18n("Username:"), defaultLoginCredentialsBox);
    m_defaultUserName = new KLineEdit(defaultLoginCredentialsBox);
    m_defaultUserName->setClearButtonEnabled(true);
    // FIXME: Add completion items?

    defaultLoginCredentialsBoxLayout->addWidget(defaultUserNameLabel, 0, 0);
    defaultLoginCredentialsBoxLayout->addWidget(m_defaultUserName, 0, 1);

    QLabel *defaultPasswordLabel = new QLabel(i18n("Password:"), defaultLoginCredentialsBox);
    m_defaultPassword = new KPasswordLineEdit(defaultLoginCredentialsBox);
    m_defaultPassword->setClearButtonEnabled(true);

    defaultLoginCredentialsBoxLayout->addWidget(defaultPasswordLabel, 1, 0);
    defaultLoginCredentialsBoxLayout->addWidget(m_defaultPassword, 1, 1);

    layout->addWidget(defaultLoginCredentialsBox);

    layout->addStretch(100);

    loadDefaultLoginCredentials();

    connect(m_defaultUserName, &KLineEdit::textChanged, this, &Smb4KConfigPageAuthentication::slotDefaultUserNameEdited);
    connect(m_defaultPassword, &KPasswordLineEdit::passwordChanged, this, &Smb4KConfigPageAuthentication::slotDefaultPasswordEdited);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::activeProfileChanged, this, &Smb4KConfigPageAuthentication::loadDefaultLoginCredentials);
}

Smb4KConfigPageAuthentication::~Smb4KConfigPageAuthentication()
{
}

void Smb4KConfigPageAuthentication::saveDefaultLoginCredentials()
{
    (void)Smb4KCredentialsManager::self()->writeDefaultLoginCredentials(m_defaultUserName->text(), m_defaultPassword->password());
}

void Smb4KConfigPageAuthentication::loadDefaultLoginCredentials()
{
    QString user, pass;

    if (Smb4KCredentialsManager::self()->readDefaultLoginCredentials(&user, &pass)) {
        m_defaultUserName->setText(user);
        m_defaultPassword->setPassword(pass);
    } else {
        m_defaultUserName->clear();
        m_defaultPassword->clear();
    }
}

bool Smb4KConfigPageAuthentication::defaultLoginCredentialsChanged()
{
    return m_defaultCredentialsModified;
}

void Smb4KConfigPageAuthentication::slotDefaultUserNameEdited(const QString &userName)
{
    QString user, pass;

    (void)Smb4KCredentialsManager::self()->readDefaultLoginCredentials(&user, &pass);

    if (user != userName) {
        m_defaultCredentialsModified = true;
        Q_EMIT defaultLoginCredentialsModified();
    }
}

void Smb4KConfigPageAuthentication::slotDefaultPasswordEdited(const QString &password)
{
    QString user, pass;

    (void)Smb4KCredentialsManager::self()->readDefaultLoginCredentials(&user, &pass);

    if (pass != password) {
        m_defaultCredentialsModified = true;
        Q_EMIT defaultLoginCredentialsModified();
    }
}
