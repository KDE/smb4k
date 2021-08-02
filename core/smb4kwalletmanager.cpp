/***************************************************************************
    This is the wallet manager of Smb4K.
                             -------------------
    begin                : Sa Dez 27 2008
    copyright            : (C) 2008-2021 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// application specific includes
#include "smb4kwalletmanager.h"
#include "smb4kauthinfo.h"
#include "smb4kglobal.h"
#include "smb4khomesshareshandler.h"
#include "smb4khost.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"
#include "smb4kwalletmanager_p.h"

// Qt includes
#include <QApplication>
#include <QPointer>
#include <QTest>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KWalletManagerStatic, p);

Smb4KWalletManager::Smb4KWalletManager(QObject *parent)
    : QObject(parent)
    , d(new Smb4KWalletManagerPrivate)
{
    d->wallet = nullptr;
}

Smb4KWalletManager::~Smb4KWalletManager()
{
}

Smb4KWalletManager *Smb4KWalletManager::self()
{
    return &p->instance;
}

void Smb4KWalletManager::readLoginCredentials(const NetworkItemPtr& networkItem)
{
    if (networkItem) {
        if (networkItem->type() == Host || networkItem->type() == Share) {
            Smb4KAuthInfo authInfo;
            
            if (networkItem->type() == Share) {
                SharePtr share = networkItem.staticCast<Smb4KShare>();
                
                if (share->isHomesShare()) {
                    authInfo.setUrl(share->homeUrl());
                } else {
                    authInfo.setUrl(share->url());
                }
                
                //
                // Read the credentials for the share. Fall back to the URL of the
                // host, if no credentials for the share could be found.
                // 
                if (!read(&authInfo)) {
                    authInfo.setUrl(share->url().adjusted(QUrl::RemovePath | QUrl::StripTrailingSlash));
                    (void) read(&authInfo);
                }
            } else {
                authInfo.setUrl(networkItem->url());
                (void) read(&authInfo);
            }
            
            QUrl url = networkItem->url();
            
            url.setUserName(authInfo.userName());
            url.setPassword(authInfo.password());
            
            networkItem->setUrl(url);
        }
    }
}

void Smb4KWalletManager::readLoginCredentials(Smb4KAuthInfo* authInfo)
{
    if (authInfo) {
        if (authInfo->type() == Host || authInfo->type() == Share) {
            (void) read(authInfo);
        }
    }
}

void Smb4KWalletManager::writeLoginCredentials(const NetworkItemPtr& networkItem)
{
    if (networkItem) {
        if (networkItem->type() == Host || networkItem->type() == Share) {
            Smb4KAuthInfo authInfo;
            
            if (networkItem->type() == Share) {
                SharePtr share = networkItem.staticCast<Smb4KShare>();
                
                if (share->isHomesShare()) {
                    authInfo.setUrl(share->homeUrl());
                } else {
                    authInfo.setUrl(share->url());
                }
            } else {
                authInfo.setUrl(networkItem->url());
            }

            write(&authInfo);
        }
    }
}

void Smb4KWalletManager::writeLoginCredentials(Smb4KAuthInfo* authInfo)
{
    if (authInfo) {
        if (authInfo->type() == Host || authInfo->type() == Share) {
            write(authInfo);
        }
    }
}

void Smb4KWalletManager::writeLoginCredentialsList(const QList<Smb4KAuthInfo *>& list)
{
    //
    // For the sake of simplicity, clear all wallet entries
    // 
    clear();

    //
    // Write the new list to the wallet
    // 
    for (Smb4KAuthInfo *authInfo : list) {
        write(authInfo);
    }
}

QList<Smb4KAuthInfo *> Smb4KWalletManager::loginCredentialsList()
{
    QList<Smb4KAuthInfo *> entries;
    
    if (init()) {
        bool ok = false;
        QMap<QString, QMap<QString, QString>> allWalletEntries = d->wallet->mapList(&ok);
        
        if (ok) {
            QMapIterator<QString, QMap<QString, QString>> it(allWalletEntries);
            
            while (it.hasNext()) {
                it.next();
                
                Smb4KAuthInfo *authInfo = new Smb4KAuthInfo();
                
                if (it.key() == QStringLiteral("DEFAULT_LOGIN")) {
                    authInfo->setUserName(it.value().value("Login"));
                    authInfo->setPassword(it.value().value("Password"));
                } else {
                    QUrl url(it.key(), QUrl::TolerantMode);
                    authInfo->setUrl(url);
                    authInfo->setUserName(it.value().value("Login"));
                    authInfo->setPassword(it.value().value("Password"));
                }
                
                entries << authInfo;
            }
        }
    }

    return entries;
}

bool Smb4KWalletManager::showPasswordDialog(const NetworkItemPtr &networkItem)
{
    //
    // Define the return value here
    //
    bool success = false;

    //
    // Check that the network item is not null
    //
    if (networkItem) {
        //
        // Get the known logins (for homes shares) and read the authentication
        // information.
        //
        QMap<QString, QString> knownLogins;

        switch (networkItem->type()) {
        case Share: {
            //
            // Cast the network item
            //
            SharePtr share = networkItem.staticCast<Smb4KShare>();

            //
            // If the share is a 'homes' share, read the known logins
            // for that share.
            //
            if (share->isHomesShare()) {
                //
                // Get the known logins
                //
                QStringList userList = Smb4KHomesSharesHandler::self()->homesUsers(share);

                //
                // Read the authentication information for all known logins
                //
                for (const QString &user : qAsConst(userList)) {
                    //
                    // Create a temp share
                    //
                    SharePtr tempShare = share;

                    //
                    // Set the login
                    //
                    tempShare->setLogin(user);

                    //
                    // Read the authentication information
                    //
                    readLoginCredentials(tempShare);

                    //
                    // Save the authentication data in the map
                    //
                    knownLogins.insert(tempShare->login(), tempShare->password());

                    //
                    // Clear the temp share
                    //
                    tempShare.clear();
                }
            } else {
                readLoginCredentials(networkItem);
            }

            break;
        }
        default: {
            readLoginCredentials(networkItem);
            break;
        }
        }

        //
        // Set up the password dialog and show it
        //
        QPointer<Smb4KPasswordDialog> dlg = new Smb4KPasswordDialog(networkItem, knownLogins, QApplication::activeWindow());

        // 
        // On closure, write the login credentials to the wallet
        // 
        if (dlg->exec() == Smb4KPasswordDialog::Accepted) {
            writeLoginCredentials(networkItem);
            success = true;
        }

        delete dlg;
    }

    return success;
}

bool Smb4KWalletManager::useWalletSystem() const
{
    return (KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet());
}

bool Smb4KWalletManager::init()
{
    if (useWalletSystem()) {
        //
        // Get a pointer to the wallet, if we do not have one yet
        //
        if (!d->wallet) {
            //
            // Open the wallet synchronously.
            //
            d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), QApplication::activeWindow() ? QApplication::activeWindow()->winId() : 0);

            //
            // Check if the wallet was opened successfully. Set the
            // right folder in case it was.
            //
            if (d->wallet) {
                if (d->wallet->isOpen()) {
                    if (!d->wallet->hasFolder("Smb4K")) {
                        d->wallet->createFolder("Smb4K");
                        d->wallet->setFolder("Smb4K");
                    } else {
                        d->wallet->setFolder("Smb4K");
                    }
                } else {
                    Smb4KNotification::credentialsNotAccessible();
                }
            } else {
                delete d->wallet;
                d->wallet = nullptr;

                Smb4KNotification::openingWalletFailed(KWallet::Wallet::NetworkWallet());
            }
        }
    } else {
        if (d->wallet) {
            //
            // Close the wallet, if Smb4K is the only application that
            // is using it. Thus, use force=false, otherwise it will definitely
            // be closed.
            //
            d->wallet->closeWallet(KWallet::Wallet::NetworkWallet(), false);

            //
            // Delete the wallet and set it to 0.
            //
            delete d->wallet;
            d->wallet = nullptr;
        }
    }
    
    emit initialized();
    
    return (d->wallet && d->wallet->isOpen());
}


bool Smb4KWalletManager::read(Smb4KAuthInfo* authInfo)
{
    bool success = false;
    
    if (init()) {
        if (authInfo->type() != Smb4KGlobal::UnknownNetworkItem) {
            //
            // Get the string representation of the URL
            // 
            QString itemUrlString = authInfo->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);
            
            //
            // Check if an entry exists for the given URL. If not, try to
            // get the correct string / key by a case insensitive comparision.
            // 
            if (!d->wallet->hasEntry(itemUrlString)) {
                //
                // Get all keys of the saved login credentials
                // 
                QStringList walletEntries = d->wallet->entryList();
            
                //
                // Find the correct key for these login credentials
                // 
                for (const QString &entry : qAsConst(walletEntries)) {
                    if (QString::compare(entry, itemUrlString, Qt::CaseInsensitive) == 0) {
                        itemUrlString = entry;
                        break;
                    }
                }
            }
            
            //
            // Read the login credentials from the wallet. Use the default login 
            // as fallback.
            // 
            if (!itemUrlString.isEmpty()) {
                QMap<QString, QString> credentials;
                
                if (d->wallet->readMap(itemUrlString, credentials) == 0) {
                    authInfo->setUserName(credentials.value("Login"));
                    authInfo->setPassword(credentials.value("Password"));
                    success = true;
                }
            } else {
                if (Smb4KSettings::useDefaultLogin()) {
                    QMap<QString, QString> credentials;
                
                    if (d->wallet->readMap("DEFAULT_LOGIN", credentials) == 0) {
                        authInfo->setUserName(credentials.value("Login"));
                        authInfo->setPassword(credentials.value("Password"));
                        success = true;
                    }
                }
            }
        } else {
            if (Smb4KSettings::useDefaultLogin()) {
                QMap<QString, QString> credentials;
                
                if (d->wallet->readMap("DEFAULT_LOGIN", credentials) == 0) {
                    authInfo->setUserName(credentials.value("Login"));
                    authInfo->setPassword(credentials.value("Password"));
                    success = true;
                }
            }
        }
    }
    
    return success;
}

void Smb4KWalletManager::write(Smb4KAuthInfo* authInfo)
{
    if (init()) {
        //
        // Get the key for the wallet entry
        // 
        QString key;
        
        if (authInfo->type() != UnknownNetworkItem) {
            key = authInfo->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);
        } else {
            key = QStringLiteral("DEFAULT_LOGIN");
        }
        
        //
        // Write the credentials to the wallet.
        //
        if (!authInfo->userName().isEmpty() /* allow empty passwords */) {
            QMap<QString, QString> credentials;
            credentials.insert("Login", authInfo->userName());
            credentials.insert("Password", authInfo->password());

            if (d->wallet->writeMap(key, credentials) == 0) {
                d->wallet->sync();
            }
        }
    }
}

void Smb4KWalletManager::clear()
{
    if (init()) {
        //
        // Get the list of wallet entries
        //
        QStringList entryList = d->wallet->entryList();
        
        //
        // Remove all wallet entries
        // 
        for (const QString &entry : qAsConst(entryList)) {
            d->wallet->removeEntry(entry);
        }
        
        d->wallet->sync();
    }
}



