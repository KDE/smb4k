/***************************************************************************
    smb4kconfigdialog  -  The configuration dialog of Smb4K
                             -------------------
    begin                : Sa Apr 14 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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

#ifndef SMB4KCONFIGDIALOG_H
#define SMB4KCONFIGDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kconfigdialog.h>

// application specific includes
#include <core/smb4ksudowriterinterface.h>

// forward declarations
class Smb4KSettings;

/**
 * This is the (new) configuration dialog of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KConfigDialog : public KConfigDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent        The parent widget
     *
     * @param name          The name of this dialog
     *
     * @param settings      The Smb4KSettings object that needs to be passed
     *                      so that the settings can be managed.
     */
    Smb4KConfigDialog( QWidget *parent, const char *name, Smb4KSettings *settings );

    /**
     * The constructor for use with the KGenericLibrary.
     *
     * @param parent        The parent widget
     *
     * @param args          The argument list
     */
    Smb4KConfigDialog( QWidget *parent, const QStringList &args );

    /**
     * The destructor
     */
    ~Smb4KConfigDialog();

  protected:
    /**
     * Reimplemented from QWidget to do last things before the
     * configuration dialog is shown.
     *
     * @param e             The show event object
     */
    void showEvent( QShowEvent *e );

  protected slots:
    /**
     * Reimplemented from KDialog. This slot overwrites the standard
     * behavior of the "Apply" and "OK" button, so that application
     * actions can be executed like saving the super user settings to
     * the system files.
     *
     * @param button        The button code
     */
    void slotButtonClicked( int button );

    /**
     * The 'Remove Entries' button in the 'Super User' page has been
     * clicked. Initialize the removal of Smb4K's configuration entries
     * from the configuration file of the currently chosen program.
     */
    void slotRemoveSuperUserEntries();
    
    /**
     * This slot is connected to the "Load" button of the "Wallet Entries" tab
     * of the "Authentication" configuration page. It loads the authentication
     * information and puts it into the list view.
     */
    void slotLoadAuthenticationInformation();
    
    /**
     * This slot is connected to the "Save" button of the "Wallet Entries" tab
     * of the "Authentication" configuration page. It saves the authentication
     * information.
     */
    void slotSaveAuthenticationInformation();
    
    /**
     * This slot is connected to the Smb4KAuthOptions::setDefaultLogin() signal.
     * It defines the default login.
     */
    void slotSetDefaultLogin();
    
    /**
     * Enable/disbale the "Apply" button if settings that are not managed by
     * KConfig XT have changed.
     */
    void slotEnableApplyButton();
    
  private:
    /**
     * "User Interface" page
     */
    KPageWidgetItem *m_user_interface;
    
    /**
     * "Network" page
     */
    KPageWidgetItem *m_network;
    
    /**
     * "Shares" page
     */
    KPageWidgetItem *m_shares;
    
    /**
     * "Authentication" page
     */
    KPageWidgetItem *m_authentication;
    
    /**
     * "Samba" page
     */
    KPageWidgetItem *m_samba;
    
    /**
     * "Synchronization" page
     */
    KPageWidgetItem *m_synchronization;
    
    /**
     * "Super User" page
     */
    KPageWidgetItem *m_super_user;
    
    /**
     * "Laptop Support" page 
     */
    KPageWidgetItem *m_laptop_support;
    
    /**
     * Set up the config dialog.
     */
    void setupDialog();

    /**
     * Load the custom Samba options
     */
    void loadCustomSambaOptions();

    /**
     * Save the custom Samba options
     */
    void saveCustomSambaOptions();

    /**
     * Write super user configuration entries to configuration file.
     */
    void writeSuperUserEntries();

    /**
     * Checks that mandatorily needed input is provided for settings that
     * need it. This function will report all missing input to the user
     * via a message box.
     *
     * @returns TRUE if the check passed and FALSE if it failed.
     */
    bool checkSettings();
};

#endif
