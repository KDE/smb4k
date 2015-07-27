/***************************************************************************
    smb4kconfigdialog  -  The configuration dialog of Smb4K
                             -------------------
    begin                : Sa Apr 14 2007
    copyright            : (C) 2004-2015 by Alexander Reinholdt
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

#ifndef SMB4KCONFIGDIALOG_H
#define SMB4KCONFIGDIALOG_H

// Qt includes
#include <QtWidgets/QAbstractButton>

// KDE includes
#include <KConfigWidgets/KConfigDialog>

// forward declarations
class Smb4KSettings;

/**
 * This is the (new) configuration dialog of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KConfigDialog : public KConfigDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor
     * @param parent        The parent widget
     * @param args          The argument list
     */
    Smb4KConfigDialog(QWidget *parent, const QList<QVariant> &args);
    
    /**
     * The destructor
     */
    ~Smb4KConfigDialog();

  protected slots:
    /**
     * Reimplemented from KConfigDialog. Used to do things that 
     * KConfigDialog::updateSettings() does not do.
     */
    void updateSettings();
    
    /**
     * Reimplemented from KConfigDialog. Used to do things before
     * the dialog is shown.
     */
    void updateWidgets();
    
//     /**
//      * Reimplemented from KDialog. This slot overwrites the standard
//      * behavior of the "Apply" and "OK" button, so that application
//      * actions can be executed like saving the super user settings to
//      * the system files.
//      *
//      * @param button        The button code
//      */
//     void slotButtonClicked(int button);

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
     * Enable/disable the "Apply" button if settings that are not managed by
     * KConfig XT have changed.
     */
    void slotEnableApplyButton();
    
    /**
     * Reload custom options
     */
    void slotReloadCustomOptions();
    
    /**
     * This slot is used to check the settings of the different pages.
     * 
     * @param current     the current dialog page
     * @param before      the previous dialog page
     */
    void slotCheckPage(KPageWidgetItem *current, KPageWidgetItem *before);
    
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
     * "Mounting" page
     */
    KPageWidgetItem *m_mounting;
    
    /**
     * "Synchronization" page
     */
    KPageWidgetItem *m_synchronization;
    
    /**
     * "Custom Options" page
     */
    KPageWidgetItem *m_custom_options;
    
    /**
     * "Profiles" page
     */
    KPageWidgetItem *m_profiles;
    
    /**
     * Set up the config dialog.
     */
    void setupDialog();

    /**
     * Load the custom Samba options
     */
    void loadCustomOptions();

    /**
     * Save the custom Samba options
     */
    void saveCustomOptions();
    
    /**
     * Takes care that the changes made to the profiles are propagated
     * to the core classes via the profiles manager.
     */
    void propagateProfilesChanges();
    
    /**
     * Checks the settings in the Network page.
     * 
     * @returns TRUE if everything is OK and FALSE otherwise.
     */
    bool checkNetworkPage();
    
    /**
     * Checks the settings in the Shares page.
     * 
     * @returns TRUE if everything is OK and FALSE otherwise.
     */
    bool checkSharesPage();
    
    /**
     * Checks the settings in the Mounting page.
     * 
     * @returns TRUE if everything is OK and FALSE otherwise.
     */
    bool checkMountingPage();
    
    /**
     * Checks the settings in the Synchronization page.
     * 
     * @returns TRUE if everything is OK and FALSE otherwise.
     */
    bool checkSynchronizationPage();

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
