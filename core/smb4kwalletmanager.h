/***************************************************************************
    smb4kwalletmanager  -  This is the wallet manager of Smb4K.
                             -------------------
    begin                : Sa Dez 27 2008
    copyright            : (C) 2008-2015 by Alexander Reinholdt
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

#ifndef SMB4KWALLETMANAGER_H
#define SMB4KWALLETMANAGER_H

// Qt includes
#include <QtCore/QList>
#include <QtWidgets/QWidget>

// forward declarations
class Smb4KAuthInfo;
class Smb4KBasicNetworkItem;
class Smb4KWalletManagerPrivate;

/**
 * This class manages the access to the digital wallet where the authentication
 * information is stored.
 * 
 * If the user chooses to no use the wallet, a password dialog is shown every
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
     * Read the authentication for a certain network item. This functions
     * adds the login and password (if present) to the past @p item. A 
     * pre-defined login name is honored.
     *
     * If you pass an empty item, the default authentication information will
     * be set or none at all, depending on the settings the user chose.
     *
     * @param networkItem     The network item for that the authentication
     *                        information should be acquired
     */
    void readAuthInfo(Smb4KBasicNetworkItem *networkItem);

    /**
     * This function reads the default authentication information and enters it
     * into @p authInfo. If no default authentication data is present this 
     * function does nothing. 
     * 
     * Please note that this function does not check if the user disabled the use 
     * of the default login. It is always returned.
     *
     * @param authInfo        The Smb4KAuthInfo object that will be populated
     *                        with the default authentication information.
     */
    void readDefaultAuthInfo(Smb4KAuthInfo *authInfo);

    /**
     * Write the authentication information provided by the network item to 
     * the wallet or to the internal list if no wallet should be used.
     * 
     * @param networkItem     The network item for that the authentication
     *                        information should be saved
     */
    void writeAuthInfo(Smb4KBasicNetworkItem *networkItem);

    /**
     * This function writes the default authentication information to the
     * wallet. If the wallet is not used, the authentication information is
     * not of type Smb4KAuthInfo::Default or empty, this function does nothing.
     * 
     * @param authInfo        The Smb4KAuthInfo object
     */
    void writeDefaultAuthInfo(Smb4KAuthInfo *authInfo);

    /**
     * Show the password dialog. This function takes an Smb4KBasicNetworkItem
     * object @p item, shows an authentication dialog and saves the data if
     * it is new or updated.
     *
     * @param networkItem     The network item for that the authentication
     *                        information should be entered
     *
     * @param parent          The optional parent widget of the password dialog
     *
     * @returns TRUE if successful and FALSE otherwise
     */
    bool showPasswordDialog(Smb4KBasicNetworkItem *networkItem,
                             QWidget *parent = 0);

    /**
     * This function returns TRUE if the wallet system can be/is used and
     * FALSE otherwise.
     *
     * @returns TRUE if the wallet system can be/is used.
     */
    bool useWalletSystem() const;

    /**
     * Returns the list of authentication information objects stored in the 
     * wallet or an empty list if the wallet is not open, no entries are 
     * defined or the wallet system is disabled.
     * 
     * @returns a list of all wallet entries.
     */
    QList<Smb4KAuthInfo *> walletEntries();
    
    /**
     * Writes a list of authentication information objects to the wallet. If 
     * the wallet system is disabled or the wallet is not open, this function 
     * will do nothing.
     * 
     * @param entries       The list of authentication information objects
     */
    void writeWalletEntries(const QList<Smb4KAuthInfo *> &entries);
    
    /**
     * This function returns TRUE if the wallet is used and open and 
     * FALSE otherwise.
     * 
     * @returns TRUE if the wallet is used and open.
     */
    bool walletIsOpen() const;

  Q_SIGNALS:
    /**
     * This signal is emitted when the wallet manager was initialized
     * and is ready to process authentication information.
     */
    void initialized();
    
  protected Q_SLOTS:
    /**
     * This slot is invoked when the wallet is opened.
     */
    void slotWalletOpened(bool success);

  private:
    /**
     * Initialize the wallet manager.
     */
    void init();

    /**
     * Pointer to the Smb4KWalletManagerPrivate class
     */
    const QScopedPointer<Smb4KWalletManagerPrivate> d;
};

#endif
