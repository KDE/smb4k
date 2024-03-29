/*
    This is the wallet manager of Smb4K.

    SPDX-FileCopyrightText: 2008-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KWALLETMANAGER_H
#define SMB4KWALLETMANAGER_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QList>
#include <QWidget>

// forward declarations
class Smb4KAuthInfo;
class Smb4KWalletManagerPrivate;

/**
 * This class manages the access to the digital wallet where the login
 * credentials are stored.
 *
 * If the user chooses to not use the wallet, a password dialog is shown every
 * time authentication information is needed.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KWalletManager : public QObject
{
    Q_OBJECT

    friend class Smb4KWalletManagerPrivate;

public:
    /**
     * The constructor
     */
    explicit Smb4KWalletManager(QObject *parent = nullptr);

    /**
     * The destructor
     */
    ~Smb4KWalletManager();

    /**
     * This is a static pointer to this class.
     */
    static Smb4KWalletManager *self();

    /**
     * Read the login credentials for the given @p networkItem object.
     *
     * @param networkItem   The NetworkItemPtr object
     */
    void readLoginCredentials(const NetworkItemPtr &networkItem);

    /**
     * Read the login credentials for the given URL in the @p authInfo object.
     *
     * To get the default login credetials, pass an Smb4KAuthInfo object with
     * type Smb4KGlobal::UnknownNetworkItem to this function.
     *
     * @param authInfo      The Smb4KAuthInfo object
     */
    void readLoginCredentials(Smb4KAuthInfo *authInfo);

    /**
     * Write the login credentials stored in the @p networkItem object to the
     * wallet.
     *
     * @param networkItem   The NetworkItemPtr object that holds the credentials
     */
    void writeLoginCredentials(const NetworkItemPtr &networkItem);

    /**
     * Write the login credentials stored in the @p authInfo object to the wallet.
     *
     * If the wallet system is disabled, this function will do nothing.
     *
     * @param authInfo      The Smb4KAuthInfo object that holds the credentials
     */
    void writeLoginCredentials(Smb4KAuthInfo *authInfo);

    /**
     * Write login credentials stored in the list @p list to the wallet.
     *
     * If the wallet system is disabled, this function will do nothing.
     *
     * @param list          The login credentials list
     */
    void writeLoginCredentialsList(const QList<Smb4KAuthInfo *> &list);

    /**
     * Returns the list of login credentials stored in the wallet or an empty
     * list if the wallet is not open, no entries are defined or the wallet
     * system is disabled.
     *
     * @returns a list of all login credentials
     */
    QList<Smb4KAuthInfo *> loginCredentialsList();

    /**
     * This function returns TRUE if the wallet system can be/is used and
     * FALSE otherwise.
     *
     * @returns TRUE if the wallet system can be/is used.
     */
    bool useWalletSystem() const;

    /**
     * This function returns TRUE if the wallet contains already default
     * credentials.
     *
     * @returns TRUE if the wallet contains a default credentials entry.
     */
    bool hasDefaultCredentials();

Q_SIGNALS:
    /**
     * This signal is emitted when the wallet manager was initialized
     * and is ready to process authentication information.
     */
    void initialized();

    /**
     * This signal is emitted when the credentials for @param url were updated.
     * If the list of credentials was cleared, an empty URL is emitted.
     *
     * @param QUrl          The URL for which the credentials were updated/added
     */
    void credentialsUpdated(const QUrl &url);

private:
    /**
     * Initialize the wallet manager
     */
    bool init();

    /**
     * Read credentials form the wallet
     */
    bool read(Smb4KAuthInfo *authInfo);

    /**
     * Write credentials to the wallet
     */
    void write(Smb4KAuthInfo *authInfo);

    /**
     * Remove all credentials from the wallet
     */
    void clear();

    /**
     * Pointer to the Smb4KWalletManagerPrivate class
     */
    const QScopedPointer<Smb4KWalletManagerPrivate> d;
};

#endif
