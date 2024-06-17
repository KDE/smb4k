/*
    This class provides the credentials manager used by Smb4K

    SPDX-FileCopyrightText: 2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kcredentialsmanager.h"
#include "smb4knotification.h"
#include "smb4kprofilemanager.h"
#include "smb4ksettings.h"

// Qt includes
#include <QDebug>
#include <QEventLoop>

// QtKeychain include
#include <qt6keychain/keychain.h>

// KDE & Qt includes for migrate() function
#include <KLocalizedString>
#include <KMessageBox>
#include <KWallet>
#include <QApplication>

class Smb4KCredentialsManagerStatic
{
public:
    Smb4KCredentialsManager instance;
};

class Smb4KCredentialsManagerPrivate
{
};

Q_GLOBAL_STATIC(Smb4KCredentialsManagerStatic, p);

Smb4KCredentialsManager::Smb4KCredentialsManager(QObject *parent)
    : QObject(parent)
    , d(new Smb4KCredentialsManagerPrivate)
{
    // For backward compatibility. Remove in the future again.
    migrate();
}

Smb4KCredentialsManager::~Smb4KCredentialsManager() noexcept
{
}

Smb4KCredentialsManager *Smb4KCredentialsManager::self()
{
    return &p->instance;
}

bool Smb4KCredentialsManager::readLoginCredentials(const NetworkItemPtr &networkItem)
{
    Q_ASSERT(networkItem);
    bool success = false;

    if (networkItem) {
        QString credentials;

        switch (networkItem->type()) {
        case Host: {
            QString key = networkItem->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);

            int returnCode = read(key, &credentials);

            if (returnCode == QKeychain::EntryNotFound) {
                key = QStringLiteral("DEFAULT::") + Smb4KProfileManager::self()->activeProfile();
                returnCode = read(key, &credentials);
            }

            success = (returnCode == QKeychain::NoError);

            break;
        }
        case Share: {
            SharePtr share = networkItem.staticCast<Smb4KShare>();
            QString key;

            if (!share->isHomesShare()) {
                key = share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);
            } else {
                key = share->homeUrl().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);
            }

            int returnCode = read(key, &credentials);

            if (returnCode == QKeychain::EntryNotFound) {
                key = share->url().adjusted(QUrl::RemovePath | QUrl::StripTrailingSlash).toString(QUrl::RemovePassword | QUrl::RemovePort);

                returnCode = read(key, &credentials);

                if (returnCode == QKeychain::EntryNotFound) {
                    key = QStringLiteral("DEFAULT::") + Smb4KProfileManager::self()->activeProfile();
                    returnCode = read(key, &credentials);
                }
            }

            success = (returnCode == QKeychain::NoError);

            break;
        }
        default: {
            QString key = QStringLiteral("DEFAULT::") + Smb4KProfileManager::self()->activeProfile();
            int returnCode = read(key, &credentials);
            success = (returnCode == QKeychain::NoError);
            break;
        }
        }

        QUrl url = networkItem->url();
        url.setUserInfo(credentials);

        networkItem->setUrl(url);
    }

    return success;
}

bool Smb4KCredentialsManager::writeLoginCredentials(const NetworkItemPtr &networkItem)
{
    Q_ASSERT(networkItem);
    bool success = false;

    if (networkItem) {
        switch (networkItem->type()) {
        case Host: {
            QString key = networkItem->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);
            success = (write(key, networkItem->url().userInfo()) == QKeychain::NoError);
            break;
        }
        case Share: {
            SharePtr share = networkItem.staticCast<Smb4KShare>();
            QString key;

            if (!share->isHomesShare()) {
                key = share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);
            } else {
                key = share->homeUrl().toString(QUrl::RemoveUserInfo | QUrl::RemovePort);
            }

            success = (write(key, share->url().userInfo()) == QKeychain::NoError);
            break;
        }
        default: {
            break;
        }
        }
    }

    if (success) {
        Q_EMIT credentialsUpdated(networkItem->url());
    }

    return success;
}

bool Smb4KCredentialsManager::readDefaultLoginCredentials(QString *username, QString *password)
{
    QString credentials;
    QString key = QStringLiteral("DEFAULT::") + Smb4KProfileManager::self()->activeProfile();

    if (read(key, &credentials) == QKeychain::NoError) {
        *username = credentials.section(QStringLiteral(":"), 0, 0);
        *password = credentials.section(QStringLiteral(":"), 1, -1);
        return true;
    }

    return false;
}

bool Smb4KCredentialsManager::writeDefaultLoginCredentials(const QString &username, const QString &password)
{
    bool success = false;

    if (!username.isEmpty() /* allow empty passwords */) {
        QString key = QStringLiteral("DEFAULT::") + Smb4KProfileManager::self()->activeProfile();
        success = (write(key, username + QStringLiteral(":") + password) == QKeychain::NoError);
    } else {
        QString key = QStringLiteral("DEFAULT::") + Smb4KProfileManager::self()->activeProfile();
        success = (remove(key) == QKeychain::NoError);
    }

    return success;
}

bool Smb4KCredentialsManager::hasDefaultCredentials() const
{
    QString credentials;
    QString key = QStringLiteral("DEFAULT::") + Smb4KProfileManager::self()->activeProfile();

    if (read(key, &credentials) == QKeychain::NoError) {
        return true;
    }

    return false;
}

int Smb4KCredentialsManager::read(const QString &key, QString *credentials) const
{
    int returnValue = QKeychain::NoError;
    QString errorMessage;

    QEventLoop loop;

    QKeychain::ReadPasswordJob *readPasswordJob = new QKeychain::ReadPasswordJob(QStringLiteral("Smb4K"));
    readPasswordJob->setAutoDelete(true);
    readPasswordJob->setKey(key);

    QObject::connect(readPasswordJob, &QKeychain::ReadPasswordJob::finished, [&]() {
        if ((returnValue = readPasswordJob->error()) == QKeychain::NoError) {
            *credentials = readPasswordJob->textData();
        } else {
            errorMessage = readPasswordJob->errorString();
        }

        loop.quit();
    });

    readPasswordJob->start();

    loop.exec();

    switch (returnValue) {
    case QKeychain::CouldNotDeleteEntry:
    case QKeychain::AccessDenied:
    case QKeychain::NoBackendAvailable:
    case QKeychain::NotImplemented:
    case QKeychain::OtherError: {
        Smb4KNotification::keychainError(errorMessage);
        break;
    }
    default: {
        break;
    }
    }

    return returnValue;
}

int Smb4KCredentialsManager::write(const QString &key, const QString &credentials) const
{
    int returnValue = QKeychain::NoError;
    QString errorMessage;

    QEventLoop loop;

    QKeychain::WritePasswordJob *writePasswordJob = new QKeychain::WritePasswordJob(QStringLiteral("Smb4K"));
    writePasswordJob->setAutoDelete(true);
    writePasswordJob->setKey(key);
    writePasswordJob->setTextData(credentials);

    QObject::connect(writePasswordJob, &QKeychain::WritePasswordJob::finished, [&]() {
        if ((returnValue = writePasswordJob->error()) != QKeychain::NoError) {
            errorMessage = writePasswordJob->errorString();
        }

        loop.quit();
    });

    writePasswordJob->start();

    loop.exec();

    switch (returnValue) {
    case QKeychain::CouldNotDeleteEntry:
    case QKeychain::AccessDenied:
    case QKeychain::NoBackendAvailable:
    case QKeychain::NotImplemented:
    case QKeychain::OtherError: {
        Smb4KNotification::keychainError(errorMessage);
        break;
    }
    default: {
        break;
    }
    }

    return returnValue;
}

int Smb4KCredentialsManager::remove(const QString &key)
{
    int returnValue = QKeychain::NoError;
    QString errorMessage;

    QEventLoop loop;

    QKeychain::DeletePasswordJob *deletePasswordJob = new QKeychain::DeletePasswordJob(QStringLiteral("Smb4K"));
    deletePasswordJob->setAutoDelete(true);
    deletePasswordJob->setKey(key);

    QObject::connect(deletePasswordJob, &QKeychain::DeletePasswordJob::finished, [&]() {
        if ((returnValue = deletePasswordJob->error()) != QKeychain::NoError) {
            errorMessage = deletePasswordJob->errorString();
        }

        loop.quit();
    });

    deletePasswordJob->start();

    loop.exec();

    switch (returnValue) {
    case QKeychain::CouldNotDeleteEntry:
    case QKeychain::AccessDenied:
    case QKeychain::NoBackendAvailable:
    case QKeychain::NotImplemented:
    case QKeychain::OtherError: {
        Smb4KNotification::keychainError(errorMessage);
        break;
    }
    default: {
        break;
    }
    }

    return returnValue;
}

void Smb4KCredentialsManager::migrate()
{
    // Only consider migrating login credentials if Smb4K was already installed and
    // no migration has been done before.
    QString configFile = QStandardPaths::locate(Smb4KSettings::self()->config()->locationType(), Smb4KSettings::self()->config()->mainConfigName());
    KConfigGroup authenticationGroup(Smb4KSettings::self()->config(), QStringLiteral("Authentication"));

    if (QFile::exists(configFile) && !authenticationGroup.hasKey(QStringLiteral("MigratedToKeychain"))) {
        int returnValue = QKeychain::NoError;

        KMessageBox::information(QApplication::activeWindow() ? QApplication::activeWindow() : nullptr,
                                 i18n("The way Smb4K stores the login credentials changed. They will now be migrated.\n\n"
                                      "This change is incompatible with earlier versions of Smb4K."),
                                 i18n("Migrate Login Credentials"));

        KWallet::Wallet *wallet =
            KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), QApplication::activeWindow() ? QApplication::activeWindow()->winId() : 0);

        if (wallet && wallet->isOpen()) {
            if (wallet->hasFolder(QStringLiteral("Smb4K"))) {
                wallet->setFolder(QStringLiteral("Smb4K"));

                bool ok = false;
                QMap<QString, QMap<QString, QString>> allWalletEntries = wallet->mapList(&ok);

                if (ok) {
                    QMapIterator<QString, QMap<QString, QString>> it(allWalletEntries);

                    while (it.hasNext()) {
                        it.next();

                        QString key, userInfo;

                        if (it.key() == QStringLiteral("DEFAULT_LOGIN")) {
                            // We migrate the default login to the default profile.
                            key = QStringLiteral("DEFAULT::") /* the default profile is an empty string */;
                            userInfo = it.value().value(QStringLiteral("Login")) + QStringLiteral(":") + it.value().value(QStringLiteral("Password"));
                        } else {
                            QUrl url;
                            url.setUrl(it.key(), QUrl::TolerantMode);
                            url.setUserName(it.value().value(QStringLiteral("Login")));
                            url.setPassword(it.value().value(QStringLiteral("Password")));

                            key = it.key();
                            userInfo = url.userInfo();
                        }

                        // We don't want to have any collision, if KWallet is used as
                        // secure storage.
                        QString backupKey = it.key() + QStringLiteral(".backup");
                        wallet->renameEntry(it.key(), backupKey);

                        if ((returnValue = write(key, userInfo)) != QKeychain::NoError) {
                            wallet->renameEntry(backupKey, it.key());
                            break;
                        }

                        wallet->removeEntry(backupKey);
                    }
                }
            }
        }

        wallet->closeWallet(KWallet::Wallet::NetworkWallet(), false);
        delete wallet;

        if (returnValue == QKeychain::NoError) {
            authenticationGroup.writeEntry(QStringLiteral("MigratedToKeychain"), true);
            authenticationGroup.sync();
        }
    } else {
        authenticationGroup.writeEntry(QStringLiteral("MigratedToKeychain"), false);
        authenticationGroup.sync();
    }
}
