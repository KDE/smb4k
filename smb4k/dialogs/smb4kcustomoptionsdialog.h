/***************************************************************************
    smb4kcustomoptionsdialog  -  With this dialog the user can define
    custom Samba options for hosts or shares.
                             -------------------
    begin                : So Jun 25 2006
    copyright            : (C) 2006-2008 by Alexander Reinholdt
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

#ifndef SMB4KCUSTOMOPTIONSDIALOG_H
#define SMB4KCUSTOMOPTIONSDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QCheckBox>

// KDE includes
#include <kdialog.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kdemacros.h>

// forward declarations
class Smb4KHost;
class Smb4KShare;

class KDE_EXPORT Smb4KCustomOptionsDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param host            The host (server) for which the custom options should be defined.
     *
     * @param parent          The parent of this dialog
     */
    Smb4KCustomOptionsDialog( Smb4KHost *host, QWidget *parent = 0 );

    /**
     * Another constructor, similar to the one above.
     *
     * @param share           The share for which the custom options should be defined.
     *
     * @param parent          The parent of this dialog
     */
    Smb4KCustomOptionsDialog( Smb4KShare *share, QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KCustomOptionsDialog();

    /**
     * This function returns TRUE if the dialog has been initialized correctly
     * and may be shown now. It will always return TRUE if you want to set options
     * for a server or for a share, that is not a 'homes' share. Only in the case
     * of a homes share it may return FALSE, if you didn't choose a user name.
     *
     * @returns               TRUE if the dialog has been set up correctly.
     */
    bool isInitialized() { return m_initialized; }

  protected slots:
    /**
     * Is invoked when the port value changed
     *
     * @param int             The port number
     */
    void slotPortChanged( int value );

    /**
     * Is invoked when the protocol value changed
     *
     * @param t               The protocol
     */
    void slotProtocolChanged( const QString &p );

    /**
     * Is invoked when the user clicked the 'Use Kerberos'
     * check box.
     *
     * @param on              TRUE if the check box was
     *                        checked and FALSE otherwise
     */
    void slotKerberosToggled( bool on );

    /**
     * Commit the custom options provided for the selected
     * network item.
     */
    void slotOKButtonClicked();

    /**
     * Is invoked if the "Default" button has been pressed.
     */
    void slotDefaultButtonClicked();

    /**
     * This slot is invoked when the  "Write Access" value changed.
     *
     * @param rw              Either 'read-write' or 'read-only' (localized).
     */
    void slotWriteAccessChanged( const QString &rw );

    /**
     * This slot is invoked when the UID value changed.
     *
     * @param uid             The UID value
     */
    void slotUIDChanged( const QString &uid );

    /**
     * This slot is invoked when the GID value changed.
     *
     * @param gid             The UID value
     */
    void slotGIDChanged( const QString &gid );

  private:
    /**
     * Enumeration
     */
    enum ItemType{ Host, Share };

    /**
     * The item type
     */
    int m_type;

    /**
     * Sets up the dialog
     */
    void setupDialog();

    /**
     * The host item (is NULL if you process a share).
     */
    Smb4KHost *m_host;

    /**
     * The share item (is NULL if you process a host).
     */
    Smb4KShare *m_share;

    /**
     * Port input
     */
    KIntNumInput *m_port_input;

#ifndef __FreeBSD__
    /**
     * This combo box determines if the user wants to mount a share
     * readwrite or readonly.
     */
    KComboBox *m_rw_input;
#endif

    /**
     * The protocol
     */
    KComboBox *m_proto_input;

    /**
     * Boolean that is TRUE if the dialog has been initialized
     * correctly and my be shown now.
     */
    bool m_initialized;

    /**
     * This check box will determine if the user wants to try to
     * authenticate with Kerberos or not. This is needed for Active
     * Directory stuff.
     */
    QCheckBox *m_kerberos;

    /**
     * This combo box holds the values of the UID that the user can
     * chose from.
     */
    KComboBox *m_uid_input;

    /**
     * This combo box holds the values of the GID that the user can
     * chose from.
     */
    KComboBox *m_gid_input;
};

#endif
