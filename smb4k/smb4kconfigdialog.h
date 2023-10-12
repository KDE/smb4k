/*
    The configuration dialog of Smb4K

    SPDX-FileCopyrightText: 2004-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGDIALOG_H
#define SMB4KCONFIGDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4kconfigpageauthentication.h"
#include "smb4kconfigpagebookmarks.h"
#include "smb4kconfigpagecustomsettings.h"
#include "smb4kconfigpagemounting.h"
#include "smb4kconfigpagenetwork.h"
#include "smb4kconfigpageprofiles.h"
#include "smb4kconfigpagesynchronization.h"
#include "smb4kconfigpageuserinterface.h"

// Qt includes
#include <QAbstractButton>

// KDE includes
#include <KConfigDialog>

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

protected Q_SLOTS:
    /**
     * Reimplemented from KConfigDialog. Used to do things that
     * KConfigDialog::updateSettings() does not do.
     */
    void updateSettings() override;

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
     * Set up the config dialog.
     */
    void setupDialog();

    /**
     * Checks that mandatory needed input is provided for settings that
     * need it. This function will report all missing input to the user
     * via a message box.
     *
     * @param page          The page to check
     *
     * @returns TRUE if the check passed and FALSE if it failed.
     */
    bool checkSettings(KPageWidgetItem *page = nullptr);

    Smb4KConfigPageUserInterface *m_userInterfacePage;
    Smb4KConfigPageNetwork *m_networkPage;
    Smb4KConfigPageMounting *m_mountingPage;
    Smb4KConfigPageAuthentication *m_authenticationPage;
    Smb4KConfigPageSynchronization *m_synchronizationPage;
    Smb4KConfigPageCustomSettings *m_customSettingsPage;
    Smb4KConfigPageProfiles *m_profilesPage;
    Smb4KConfigPageBookmarks *m_bookmarksPage;

    KPageWidgetItem *m_userInterface;
    KPageWidgetItem *m_network;
    KPageWidgetItem *m_mounting;
    KPageWidgetItem *m_authentication;
    KPageWidgetItem *m_synchronization;
    KPageWidgetItem *m_customSettings;
    KPageWidgetItem *m_profiles;
    KPageWidgetItem *m_bookmarks;
};

#endif
