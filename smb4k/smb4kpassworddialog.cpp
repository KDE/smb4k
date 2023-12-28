/*
 *  Password dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kpassworddialog.h"
#include "core/smb4khomesshareshandler.h"
#include "core/smb4kwalletmanager.h"

// KDE includes
#include <KLocalizedString>

Smb4KPasswordDialog::Smb4KPasswordDialog(QWidget *parent)
    : KPasswordDialog(parent, KPasswordDialog::ShowUsernameLine)
{
}

Smb4KPasswordDialog::~Smb4KPasswordDialog() noexcept
{
}

bool Smb4KPasswordDialog::setNetworkItem(const NetworkItemPtr &networkItem)
{
    bool success = false;

    if (networkItem) {
        m_networkItem = networkItem;

        switch (m_networkItem->type()) {
        case Host: {
            HostPtr host = m_networkItem.staticCast<Smb4KHost>();

            if (host) {
                Smb4KWalletManager::self()->readLoginCredentials(host);

                setPrompt(i18n("Please enter a username and a password for the host <b>%1</b>.", host->hostName()));
                setUsername(host->userName());
                setPassword(host->password());

                success = true;
            }

            break;
        }
        case Share: {
            SharePtr share = m_networkItem.staticCast<Smb4KShare>();

            if (share) {
                setPrompt(i18n("Please enter a username and a password for the share <b>%1</b>.", share->displayString()));

                if (share->isHomesShare()) {
                    QStringList homesUsers = Smb4KHomesSharesHandler::self()->homesUsers(share);
                    QMap<QString, QString> knownLogins;

                    for (const QString &user : homesUsers) {
                        SharePtr tempShare = SharePtr(new Smb4KShare(*share.data()));
                        tempShare->setUserName(user);

                        Smb4KWalletManager::self()->readLoginCredentials(tempShare);

                        knownLogins.insert(tempShare->userName(), tempShare->password());

                        tempShare.clear();
                    }

                    setKnownLogins(knownLogins);
                } else {
                    Smb4KWalletManager::self()->readLoginCredentials(share);
                    setUsername(share->userName());
                    setPassword(share->password());
                }

                success = true;
            }

            break;
        }
        default: {
            break;
        }
        }
    }

    return success;
}

void Smb4KPasswordDialog::accept()
{
    QUrl url = m_networkItem->url();
    url.setUserName(username());
    url.setPassword(password());

    m_networkItem->setUrl(url);
    Smb4KWalletManager::self()->writeLoginCredentials(m_networkItem);

    KPasswordDialog::accept();
}
