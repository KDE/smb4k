/***************************************************************************
    smb4kwalletmanager  -  This is the wallet manager of Smb4K.
                             -------------------
    begin                : Sa Dez 27 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KWALLETMANAGER_H
#define SMB4KWALLETMANAGER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>
#include <QList>

// KDE includes
#include <kwallet.h>
#include <kdemacros.h>

// forward declarations
class Smb4KAuthInfo;

/**
 * This class manages the access to the digital wallet where the authentication
 * information is stored.
 *
 * This class also provides an internal list, where authentication information
 * can be stored temporarily if the user does not want to use the digital wallet
 * provided by KDE.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KWalletManager : public QObject
{
  Q_OBJECT

  friend class Smb4KWalletManagerPrivate;

  public:
    /**
     * This is a static pointer to this class.
     */
    static Smb4KWalletManager *self();

    /**
     * Initialize the wallet manager. It will try to open the wallet
     * synchronously - except @p async is TRUE - and set up the appropriate
     * folder.
     *
     * Normally, you do not need to call this function, because it will be
     * invoked by the read and write functions of this class if needed.
     *
     * @param parent          The optional parent widget for the wallet
     *
     * @param async           If this value is set to TRUE, the wallet will
     *                        be opened asynchronously. You need to connect
     *                        to the walletOpened() signal immediately to find
     *                        out, if the wallet could be opened.
     *
     *                        The default value is FALSE.
     */
    void init( QWidget *parent,
               bool async = false );

    /**
     * Read the authentication information for a a certain host or share.
     * This function will fill @p authInfo with the required information if
     * present.
     *
     * @param authInfo        The Smb4KAuthInfo object
     */
    void readAuthInfo( Smb4KAuthInfo *authInfo );

    /**
     * Write the authentication information provided by @p authInfo to the
     * wallet.
     *
     * If no wallet is used, authentication information that has type "Default"
     * will not be stored.
     *
     * @param auhInfo         The Smb4KAuthInfo object
     */
    void writeAuthInfo( Smb4KAuthInfo *authInfo );

    /**
     * Show the password dialog. This function takes an Smb4KAuthInfo object
     * @p authInfo, shows an authentication dialog and saves the data if it
     * was updated or is new.
     *
     * @param authInfo        The Smb4KAuthInfo object
     *
     * @param parent          The optional parent of the password dialog
     *
     * @returns TRUE if successful and FALSE otherwise.
     */
    bool showPasswordDialog( Smb4KAuthInfo *authInfo,
                             QWidget *parent = 0 );

    /**
     * This function returns TRUE if the wallet system can be/is used and
     * FALSE otherwise.
     *
     * @returns TRUE if the wallet system can be/is used.
     */
    bool useWalletSystem();

    /**
     * The state enumeration
     */
    enum State { UseWallet,
                 RememberAuthInfo,
                 ForgetAuthInfo,
                 Unknown };

    /**
     * This function returns the current state of the wallet manager. The
     * state is defined in the State enumeration.
     *
     * @returns the current state.
     */
    State currentState() { return m_state; }
    
    /**
     * Returns the list of authentication informations stored in the wallet
     * or an empty list if the wallet is not open, no entries are defined or 
     * the wallet system is disabled.
     * 
     * @returns a list of all wallet entries.
     */
    QList<Smb4KAuthInfo *> entries();
    
    /**
     * Writes a list of authentication informations to the wallet.
     * 
     * @param entries       The list of authentication informations
     */
    void writeEntries( const QList<Smb4KAuthInfo *> &entries );

  signals:
    /**
     * This signal is emitted if the wallet was opened asynchronously.
     *
     * @param success         TRUE if the wallet could be opened
     *                        successfully.
     */
    void walletOpened( bool success );

    /**
     * This signal is emitted when the wallet manager was initialized
     * and is ready to process authentication information.
     *
     * If you want to find out if the wallet manager actually uses a
     * wallet or only an internal list (in case the wallet could not
     * be opened), you have to use the currentState() function.
     */
    void initialized();

  protected slots:
    /**
     * This slot is called if the wallet was opened asynchronously. It
     * is used to set up the right folder.
     *
     * @param success         TRUE if the wallet could be opened
     *                        successfully.
     */
    void slotWalletOpened( bool success );

  private:
    /**
     * The constructor
     */
    Smb4KWalletManager();

    /**
     * The destructor
     */
    ~Smb4KWalletManager();

    /**
     * The wallet
     */
    KWallet::Wallet *m_wallet;

    /**
     * Creates the Smb4K subfolder and makes the wallet use it.
     */
    void setupFolder();

    /**
     * The current state
     */
    State m_state;

    /**
     * Internal list for storing authentication information
     */
    QList<Smb4KAuthInfo *> m_list;

#ifdef Q_OS_FREEBSD
    /**
     * This function invokes s shell, encrypts the password using the
     * smbutil utility and writes it to the ~/.nsmbrc file.
     *
     * @param authInfo        The authentication information
     */
    void writeToConfigFile( Smb4KAuthInfo *authInfo );
#endif
};

#endif
