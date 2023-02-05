/*
    The configuration dialog of Smb4K

    SPDX-FileCopyrightText: 2004-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigdialog.h"
#include "core/smb4ksettings.h"
#include "smb4kconfigpageauthentication.h"
#include "smb4kconfigpagecustomoptions.h"
#include "smb4kconfigpagemounting.h"
#include "smb4kconfigpagenetwork.h"
#include "smb4kconfigpageprofiles.h"
#include "smb4kconfigpagesynchronization.h"
#include "smb4kconfigpageuserinterface.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QList>
#include <QPair>
#include <QScrollArea>
#include <QSize>
#include <QStandardPaths>
#include <QWindow>

// KDE includes
#include <KConfigGui/KWindowConfig>
#include <KCoreAddons/KPluginFactory>
#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlRequester>
#include <KWidgetsAddons/KMessageBox>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY(Smb4KConfigDialogFactory, registerPlugin<Smb4KConfigDialog>();)

Smb4KConfigDialog::Smb4KConfigDialog(QWidget *parent, const QList<QVariant> & /*args*/)
    : KConfigDialog(parent, QStringLiteral("ConfigDialog"), Smb4KSettings::self())
{
    setupDialog();
}

Smb4KConfigDialog::~Smb4KConfigDialog()
{
}

void Smb4KConfigDialog::setupDialog()
{
    // Close destructively
    setAttribute(Qt::WA_DeleteOnClose, true);

    // Add the pages:
    Smb4KConfigPageUserInterface *interface_options = new Smb4KConfigPageUserInterface(this);
    QScrollArea *interface_area = new QScrollArea(this);
    interface_area->setWidget(interface_options);
    interface_area->setWidgetResizable(true);
    interface_area->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageNetwork *network_options = new Smb4KConfigPageNetwork(this);
    QScrollArea *network_area = new QScrollArea(this);
    network_area->setWidget(network_options);
    network_area->setWidgetResizable(true);
    network_area->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageMounting *mount_options = new Smb4KConfigPageMounting(this);
    QScrollArea *mount_area = new QScrollArea(this);
    mount_area->setWidget(mount_options);
    mount_area->setWidgetResizable(true);
    mount_area->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageAuthentication *auth_options = new Smb4KConfigPageAuthentication(this);
    QScrollArea *auth_area = new QScrollArea(this);
    auth_area->setWidget(auth_options);
    auth_area->setWidgetResizable(true);
    auth_area->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageSynchronization *rsync_options = new Smb4KConfigPageSynchronization(this);
    QScrollArea *rsync_area = new QScrollArea(this);
    rsync_area->setWidget(rsync_options);
    rsync_area->setWidgetResizable(true);
    rsync_area->setFrameStyle(QFrame::NoFrame);

    rsync_options->setEnabled(!QStandardPaths::findExecutable(QStringLiteral("rsync")).isEmpty());

    Smb4KConfigPageCustomOptions *custom_options = new Smb4KConfigPageCustomOptions(this);
    QScrollArea *custom_area = new QScrollArea(this);
    custom_area->setWidget(custom_options);
    custom_area->setWidgetResizable(true);
    custom_area->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageProfiles *profiles_page = new Smb4KConfigPageProfiles(this);
    QScrollArea *profiles_area = new QScrollArea(this);
    profiles_area->setWidget(profiles_page);
    profiles_area->setWidgetResizable(true);
    profiles_area->setFrameStyle(QFrame::NoFrame);

    //
    // Pages to the configuration dialog
    //
    m_user_interface = addPage(interface_area, Smb4KSettings::self(), i18n("User Interface"), QStringLiteral("preferences-desktop"));
    m_network = addPage(network_area, Smb4KSettings::self(), i18n("Network"), QStringLiteral("preferences-system-network"));
    m_mounting = addPage(mount_area, Smb4KMountSettings::self(), i18n("Mounting"), QStringLiteral("preferences-system-network-server-share"));
    m_authentication = addPage(auth_area, Smb4KSettings::self(), i18n("Authentication"), QStringLiteral("preferences-desktop-user-password"));
    m_synchronization = addPage(rsync_area, Smb4KSettings::self(), i18n("Synchronization"), QStringLiteral("folder-sync"));
    m_custom_settings = addPage(custom_area, Smb4KSettings::self(), i18n("Custom Settings"), QStringLiteral("settings-configure"));
    m_profiles = addPage(profiles_area, Smb4KSettings::self(), i18n("Profiles"), QStringLiteral("preferences-system-users"));

    //
    // Connections
    //
    connect(custom_options, SIGNAL(customSettingsModified()), this, SLOT(slotEnableApplyButton()));
    connect(auth_options, SIGNAL(walletEntriesModified()), this, SLOT(slotEnableApplyButton()));
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)), this, SLOT(slotCheckPage(KPageWidgetItem *, KPageWidgetItem *)));

    //
    // Dialog size
    //
    create();
    windowHandle()->resize(QSize(800, 600));

    KConfigGroup group(Smb4KSettings::self()->config(), "ConfigDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

bool Smb4KConfigDialog::checkSettings(KPageWidgetItem *page)
{
    QString errorMessage = i18n("<qt>An incorrect setting has been found. You are now taken to the corresponding configuration page to fix it.</qt>");

    if (!page || page == m_mounting) {
        Smb4KConfigPageMounting *mountingPage = m_mounting->widget()->findChild<Smb4KConfigPageMounting *>();

        if (mountingPage) {
            if (!mountingPage->checkSettings()) {
                KMessageBox::error(this, errorMessage);
                setCurrentPage(m_mounting);
                return false;
            }
        }
    }

    if (!page || page == m_synchronization) {
        Smb4KConfigPageSynchronization *synchronizationPage = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();

        if (synchronizationPage) {
            if (!synchronizationPage->checkSettings()) {
                KMessageBox::error(this, errorMessage);
                setCurrentPage(m_synchronization);
                return false;
            }
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigDialog::updateSettings()
{
    Smb4KConfigPageCustomOptions *customOptionsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomOptions *>();

    if (customOptionsPage) {
        customOptionsPage->saveCustomOptions();
    }

    Smb4KConfigPageAuthentication *authenticationPage = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();

    if (authenticationPage) {
        authenticationPage->saveLoginCredentials();
    }

    Smb4KConfigPageProfiles *profilesPage = m_profiles->widget()->findChild<Smb4KConfigPageProfiles *>();

    if (profilesPage) {
        profilesPage->applyChanges();

        //
        // Finally reload the custom options.
        //
        Smb4KConfigPageCustomOptions *customOptionsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomOptions *>();

        if (customOptionsPage) {
            customOptionsPage->loadCustomOptions();
        }
    }

    (void)checkSettings();

    KConfigGroup group(Smb4KSettings::self()->config(), "ConfigDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    KConfigDialog::updateSettings();
}

void Smb4KConfigDialog::updateWidgets()
{
    Smb4KConfigPageCustomOptions *customOptionsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomOptions *>();

    if (customOptionsPage) {
        customOptionsPage->loadCustomOptions();
    }

    KConfigDialog::updateWidgets();
}

void Smb4KConfigDialog::reject()
{
    Smb4KConfigPageCustomOptions *customOptionsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomOptions *>();

    if (customOptionsPage) {
        customOptionsPage->resetCustomOptions();
    }

    QDialog::reject();
}

void Smb4KConfigDialog::slotEnableApplyButton()
{
    //
    // Check if we need to enable the Apply button
    //
    bool enable = false;

    //
    // Check the wallet entries
    //
    Smb4KConfigPageAuthentication *authenticationPage = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();

    if (authenticationPage) {
        enable = authenticationPage->loginCredentialsChanged();
    }

    //
    // Check the custom options
    //
    Smb4KConfigPageCustomOptions *customOptionsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomOptions *>();

    if (!enable && customOptionsPage) {
        enable = customOptionsPage->customSettingsChanged();
    }

    //
    // Enable/disable the apply button
    //
    QPushButton *applyButton = buttonBox()->button(QDialogButtonBox::Apply);

    if (applyButton) {
        applyButton->setEnabled(enable);
    }
}

void Smb4KConfigDialog::slotCheckPage(KPageWidgetItem *current, KPageWidgetItem *before)
{
    Q_UNUSED(current);
    checkSettings(before);
}

#include "smb4kconfigdialog.moc"
