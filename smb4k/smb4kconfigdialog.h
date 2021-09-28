/*
    The configuration dialog of Smb4K
    -------------------
    begin                : Sa Apr 14 2007
    SPDX-FileCopyrightText: 2004-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGDIALOG_H
#define SMB4KCONFIGDIALOG_H

// Qt includes
#include <QAbstractButton>

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
    void updateSettings() override;

    /**
     * Reimplemented from KConfigDialog. Used to do things before
     * the dialog is shown.
     */
    void updateWidgets() override;

    /**
     * Reimplemented from QDialog. Used to reset things after the dialog was
     * closed via the 'Cancel' button.
     */
    void reject() override;

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
     * "Authentication" page
     */
    KPageWidgetItem *m_authentication;

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
     * Checks that mandatory needed input is provided for settings that
     * need it. This function will report all missing input to the user
     * via a message box.
     *
     * @returns TRUE if the check passed and FALSE if it failed.
     */
    bool checkSettings();
};

#endif
