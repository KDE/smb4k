/*
    The configuration dialog of Smb4K

    SPDX-FileCopyrightText: 2004-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigdialog.h"
#include "core/smb4ksettings.h"
#include "smb4kconfigpageauthentication.h"
#include "smb4kconfigpagebookmarks.h"
#include "smb4kconfigpagecustomsettings.h"
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
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KUrlRequester>
#include <KWindowConfig>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY(Smb4KConfigDialogFactory, registerPlugin<Smb4KConfigDialog>();)

Smb4KConfigDialog::Smb4KConfigDialog(QWidget *parent, const QList<QVariant> & /*args*/)
    : KConfigDialog(parent, QStringLiteral("ConfigDialog"), Smb4KSettings::self())
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    setupDialog();
}

Smb4KConfigDialog::~Smb4KConfigDialog()
{
}

void Smb4KConfigDialog::setupDialog()
{
    // Add the pages:
    Smb4KConfigPageUserInterface *userInterfacePage = new Smb4KConfigPageUserInterface(this);
    QScrollArea *userInterfaceArea = new QScrollArea(this);
    userInterfaceArea->setWidget(userInterfacePage);
    userInterfaceArea->setWidgetResizable(true);
    userInterfaceArea->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageNetwork *networkPage = new Smb4KConfigPageNetwork(this);
    QScrollArea *networkArea = new QScrollArea(this);
    networkArea->setWidget(networkPage);
    networkArea->setWidgetResizable(true);
    networkArea->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageMounting *mountingPage = new Smb4KConfigPageMounting(this);
    QScrollArea *mountingArea = new QScrollArea(this);
    mountingArea->setWidget(mountingPage);
    mountingArea->setWidgetResizable(true);
    mountingArea->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageAuthentication *authenticationPage = new Smb4KConfigPageAuthentication(this);
    QScrollArea *authenticationArea = new QScrollArea(this);
    authenticationArea->setWidget(authenticationPage);
    authenticationArea->setWidgetResizable(true);
    authenticationArea->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageSynchronization *synchronizationPage = new Smb4KConfigPageSynchronization(this);
    QScrollArea *synchronizationArea = new QScrollArea(this);
    synchronizationArea->setWidget(synchronizationPage);
    synchronizationArea->setWidgetResizable(true);
    synchronizationArea->setFrameStyle(QFrame::NoFrame);

    synchronizationPage->setEnabled(!QStandardPaths::findExecutable(QStringLiteral("rsync")).isEmpty());

    Smb4KConfigPageCustomSettings *customSettingsPage = new Smb4KConfigPageCustomSettings(this);
    QScrollArea *customSettingsArea = new QScrollArea(this);
    customSettingsArea->setWidget(customSettingsPage);
    customSettingsArea->setWidgetResizable(true);
    customSettingsArea->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageProfiles *profilesPage = new Smb4KConfigPageProfiles(this);
    QScrollArea *profilesArea = new QScrollArea(this);
    profilesArea->setWidget(profilesPage);
    profilesArea->setWidgetResizable(true);
    profilesArea->setFrameStyle(QFrame::NoFrame);

    Smb4KConfigPageBookmarks *bookmarksPage = new Smb4KConfigPageBookmarks(this);
    QScrollArea *bookmarksArea = new QScrollArea(this);
    bookmarksArea->setWidget(bookmarksPage);
    bookmarksArea->setWidgetResizable(true);
    bookmarksArea->setFrameStyle(QFrame::NoFrame);

    KConfigGroup bookmarkGroup(Smb4KSettings::self()->config(), "BookmarkEditor");

    if (bookmarkGroup.exists()) {
        QMap<QString, QStringList> completionItems;
        completionItems[QStringLiteral("CategoryCompletion")] = bookmarkGroup.readEntry("CategoryCompletion", QStringList());
        completionItems[QStringLiteral("LabelCompletion")] = bookmarkGroup.readEntry("LabelCompletion", QStringList());
        // For backward compatibility (since Smb4K 3.3.0)
        if (bookmarkGroup.hasKey(QStringLiteral("IPCompletion"))) {
            completionItems[QStringLiteral("IpAddressCompletion")] = bookmarkGroup.readEntry("IPCompletion", QStringList());
            bookmarkGroup.deleteEntry("IPCompletion");
        } else {
            completionItems[QStringLiteral("IpAddressCompletion")] = bookmarkGroup.readEntry("IpAddressCompletion", QStringList());
        }
        completionItems[QStringLiteral("LoginCompletion")] = bookmarkGroup.readEntry("LoginCompletion", QStringList());
        completionItems[QStringLiteral("WorkgroupCompletion")] = bookmarkGroup.readEntry("WorkgroupCompletion", QStringList());

        bookmarksPage->setCompletionItems(completionItems);
    }

    //
    // Pages to the configuration dialog
    //
    m_user_interface = addPage(userInterfaceArea, Smb4KSettings::self(), i18n("User Interface"), QStringLiteral("preferences-desktop"));
    m_network = addPage(networkArea, Smb4KSettings::self(), i18n("Network"), QStringLiteral("preferences-system-network-server-share-windows"));
    m_mounting = addPage(mountingArea, Smb4KMountSettings::self(), i18n("Mounting"), QStringLiteral("media-mount"));
    m_authentication = addPage(authenticationArea, Smb4KSettings::self(), i18n("Authentication"), QStringLiteral("preferences-desktop-user-password"));
    m_synchronization = addPage(synchronizationArea, Smb4KSettings::self(), i18n("Synchronization"), QStringLiteral("folder-sync"));
    m_bookmarks = addPage(bookmarksArea, Smb4KSettings::self(), i18n("Bookmarks"), QStringLiteral("bookmarks"));
    m_custom_settings = addPage(customSettingsArea, Smb4KSettings::self(), i18n("Custom Settings"), QStringLiteral("settings-configure"));
    m_profiles = addPage(profilesArea, Smb4KSettings::self(), i18n("Profiles"), QStringLiteral("preferences-system-users"));

    //
    // Connections
    //
    connect(customSettingsPage, &Smb4KConfigPageCustomSettings::customSettingsModified, this, &Smb4KConfigDialog::slotEnableApplyButton);
    connect(authenticationPage, &Smb4KConfigPageAuthentication::walletEntriesModified, this, &Smb4KConfigDialog::slotEnableApplyButton);
    connect(bookmarksPage, &Smb4KConfigPageBookmarks::bookmarksModified, this, &Smb4KConfigDialog::slotEnableApplyButton);
    connect(this, &Smb4KConfigDialog::currentPageChanged, this, &Smb4KConfigDialog::slotCheckPage);

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
    Smb4KConfigPageAuthentication *authenticationPage = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();

    if (authenticationPage) {
        authenticationPage->saveLoginCredentials();
    }

    Smb4KConfigPageCustomSettings *customSettingsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomSettings *>();

    if (customSettingsPage) {
        customSettingsPage->saveCustomSettings();
    }

    Smb4KConfigPageBookmarks *bookmarksPage = m_bookmarks->widget()->findChild<Smb4KConfigPageBookmarks *>();

    if (bookmarksPage) {
        bookmarksPage->saveBookmarks();
        QMap<QString, QStringList> completionItems = bookmarksPage->completionItems();

        KConfigGroup bookmarkGroup(Smb4KSettings::self()->config(), "BookmarkEditor");

        bookmarkGroup.writeEntry("CategoryCompletion", completionItems[QStringLiteral("CategoryCompletion")]);
        bookmarkGroup.writeEntry("LabelCompletion", completionItems[QStringLiteral("LabelCompletion")]);
        bookmarkGroup.writeEntry("IpAddressCompletion", completionItems[QStringLiteral("IpAddressCompletion")]);
        bookmarkGroup.writeEntry("LoginCompletion", completionItems[QStringLiteral("LoginCompletion")]);
        bookmarkGroup.writeEntry("WorkgroupCompletion", completionItems[QStringLiteral("WorkgroupCompletion")]);
    }

    Smb4KConfigPageProfiles *profilesPage = m_profiles->widget()->findChild<Smb4KConfigPageProfiles *>();

    if (profilesPage) {
        if (profilesPage->profilesChanged()) {
            profilesPage->applyChanges();

            Smb4KConfigPageCustomSettings *customSettingsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomSettings *>();

            if (customSettingsPage) {
                customSettingsPage->loadCustomSettings();
            }

            Smb4KConfigPageBookmarks *bookmarksPage = m_bookmarks->widget()->findChild<Smb4KConfigPageBookmarks *>();

            if (bookmarksPage) {
                bookmarksPage->loadBookmarks();
            }
        }
    }

    (void)checkSettings();

    KConfigGroup group(Smb4KSettings::self()->config(), "ConfigDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    KConfigDialog::updateSettings();
}

void Smb4KConfigDialog::slotEnableApplyButton()
{
    bool enable = false;

    Smb4KConfigPageAuthentication *authenticationPage = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();

    if (authenticationPage) {
        enable = authenticationPage->loginCredentialsChanged();
    }

    Smb4KConfigPageCustomSettings *customSettingsPage = m_custom_settings->widget()->findChild<Smb4KConfigPageCustomSettings *>();

    if (!enable && customSettingsPage) {
        enable = customSettingsPage->customSettingsChanged();
    }

    Smb4KConfigPageBookmarks *boookmarksPage = m_bookmarks->widget()->findChild<Smb4KConfigPageBookmarks *>();

    if (!enable && boookmarksPage) {
        enable = boookmarksPage->bookmarksChanged();
    }

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
