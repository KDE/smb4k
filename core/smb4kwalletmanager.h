/*
    This is the wallet manager of Smb4K.
    -------------------
    begin                : Sa Dez 27 2008
    SPDX-FileCopyrightText: 2008-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit Smb4KWalletManager(QObject *parent = 0);

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
     * Show the password dialog. This function takes an Smb4KBasicNetworkItem
     * object @p item, shows an authentication dialog and saves the data if
     * it is new or updated.
     *
     * @param networkItem     The network item for that the authentication
     *                        information should be entered
     *
     * @returns TRUE if successful and FALSE otherwise
     */
    bool showPasswordDialog(const NetworkItemPtr &networkItem);

    /**
     * This function returns TRUE if the wallet system can be/is used and
     * FALSE otherwise.
     *
     * @returns TRUE if the wallet system can be/is used.
     */
    bool useWalletSystem() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the wallet manager was initialized
     * and is ready to process authentication information.
     */
    void initialized();

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
