/*
    This is the wallet manager of Smb4K.

    SPDX-FileCopyrightText: 2008-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kwalletmanager.h"
#include "smb4kauthinfo.h"
#include "smb4kglobal.h"
#include "smb4khomesshareshandler.h"
#include "smb4khost.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

// Qt includes
#include <QApplication>
#include <QPointer>

// KDE includes
#include <KWallet>

using namespace Smb4KGlobal;

class Smb4KWalletManagerPrivate
{
public:
    KWallet::Wallet *wallet;
};

class Smb4KWalletManagerStatic
{
public:
    Smb4KWalletManager instance;
};

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

void Smb4KWalletManager::readLoginCredentials(const NetworkItemPtr &networkItem)
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
                    (void)read(&authInfo);
                }
            } else {
                authInfo.setUrl(networkItem->url());
                (void)read(&authInfo);
            }

            QUrl url = networkItem->url();

            url.setUserName(authInfo.userName());
            url.setPassword(authInfo.password());

            networkItem->setUrl(url);
        }
    }
}

void Smb4KWalletManager::readLoginCredentials(Smb4KAuthInfo *authInfo)
{
    if (authInfo) {
        if (authInfo->type() == Host || authInfo->type() == Share) {
            (void)read(authInfo);
        }
    }
}

void Smb4KWalletManager::writeLoginCredentials(const NetworkItemPtr &networkItem)
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
        } else if (networkItem->type() == UnknownNetworkItem) {
            Smb4KAuthInfo authInfo;
            authInfo.setUserName(networkItem->url().userName());
            authInfo.setPassword(networkItem->url().password());
            write(&authInfo);
        }
    }
}

void Smb4KWalletManager::writeLoginCredentials(Smb4KAuthInfo *authInfo)
{
    if (authInfo) {
        if (authInfo->type() == Host || authInfo->type() == Share || authInfo->type() == UnknownNetworkItem) {
            write(authInfo);
        }
    }
}

void Smb4KWalletManager::writeLoginCredentialsList(const QList<Smb4KAuthInfo *> &list)
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
                    authInfo->setUserName(it.value().value(QStringLiteral("Login")));
                    authInfo->setPassword(it.value().value(QStringLiteral("Password")));
                } else {
                    QUrl url(it.key(), QUrl::TolerantMode);
                    authInfo->setUrl(url);
                    authInfo->setUserName(it.value().value(QStringLiteral("Login")));
                    authInfo->setPassword(it.value().value(QStringLiteral("Password")));
                }

                entries << authInfo;
            }
        }
    }

    return entries;
}

bool Smb4KWalletManager::useWalletSystem() const
{
    return (KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet());
}

bool Smb4KWalletManager::hasDefaultCredentials()
{
    if (init()) {
        if (d->wallet->hasEntry(QStringLiteral("DEFAULT_LOGIN"))) {
            return true;
        }
    }

    return false;
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
                    if (!d->wallet->hasFolder(QStringLiteral("Smb4K"))) {
                        d->wallet->createFolder(QStringLiteral("Smb4K"));
                        d->wallet->setFolder(QStringLiteral("Smb4K"));
                    } else {
                        d->wallet->setFolder(QStringLiteral("Smb4K"));
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

    Q_EMIT initialized();

    return (d->wallet && d->wallet->isOpen());
}

bool Smb4KWalletManager::read(Smb4KAuthInfo *authInfo)
{
    bool success = false;

    if (init()) {
        if (authInfo->type() != Smb4KGlobal::UnknownNetworkItem) {
            //
            // Get the string representation of the URL
            //
            QString itemUrlString;
            QString testString = authInfo->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);

            //
            // Check if an entry exists for the given URL. If not, try to
            // get the correct string / key by a case insensitive comparision.
            //
            if (!d->wallet->hasEntry(testString)) {
                //
                // Get all keys of the saved login credentials
                //
                QStringList walletEntries = d->wallet->entryList();

                //
                // Find the correct key for these login credentials
                //
                for (const QString &entry : qAsConst(walletEntries)) {
                    if (QString::compare(entry, testString, Qt::CaseInsensitive) == 0) {
                        itemUrlString = entry;
                        break;
                    }
                }
            } else {
                itemUrlString = testString;
            }

            //
            // Read the login credentials from the wallet. Use the default login
            // as fallback.
            //
            if (!itemUrlString.isEmpty()) {
                QMap<QString, QString> credentials;

                if (d->wallet->readMap(itemUrlString, credentials) == 0) {
                    authInfo->setUserName(credentials.value(QStringLiteral("Login")));
                    authInfo->setPassword(credentials.value(QStringLiteral("Password")));
                    success = true;
                }
            } else {
                if (Smb4KSettings::useDefaultLogin()) {
                    QMap<QString, QString> credentials;

                    if (d->wallet->readMap(QStringLiteral("DEFAULT_LOGIN"), credentials) == 0) {
                        authInfo->setUserName(credentials.value(QStringLiteral("Login")));
                        authInfo->setPassword(credentials.value(QStringLiteral("Password")));
                        success = true;
                    }
                }
            }
        } else {
            if (Smb4KSettings::useDefaultLogin()) {
                QMap<QString, QString> credentials;

                if (d->wallet->readMap(QStringLiteral("DEFAULT_LOGIN"), credentials) == 0) {
                    authInfo->setUserName(credentials.value(QStringLiteral("Login")));
                    authInfo->setPassword(credentials.value(QStringLiteral("Password")));
                    success = true;
                }
            }
        }
    }

    return success;
}

void Smb4KWalletManager::write(Smb4KAuthInfo *authInfo)
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
            credentials.insert(QStringLiteral("Login"), authInfo->userName());
            credentials.insert(QStringLiteral("Password"), authInfo->password());

            if (d->wallet->writeMap(key, credentials) == 0) {
                d->wallet->sync();
            }
        }

        Q_EMIT credentialsUpdated(authInfo->url());
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

        Q_EMIT credentialsUpdated(QUrl());
    }
}
