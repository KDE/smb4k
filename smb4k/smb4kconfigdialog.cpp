/*
    The configuration dialog of Smb4K

    SPDX-FileCopyrightText: 2004-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigdialog.h"
#include "core/smb4ksettings.h"

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
    m_userInterfacePage = new Smb4KConfigPageUserInterface(this);
    QScrollArea *userInterfaceArea = new QScrollArea(this);
    userInterfaceArea->setWidget(m_userInterfacePage);
    userInterfaceArea->setWidgetResizable(true);
    userInterfaceArea->setFrameStyle(QFrame::NoFrame);

    m_networkPage = new Smb4KConfigPageNetwork(this);
    QScrollArea *networkArea = new QScrollArea(this);
    networkArea->setWidget(m_networkPage);
    networkArea->setWidgetResizable(true);
    networkArea->setFrameStyle(QFrame::NoFrame);

    m_mountingPage = new Smb4KConfigPageMounting(this);
    QScrollArea *mountingArea = new QScrollArea(this);
    mountingArea->setWidget(m_mountingPage);
    mountingArea->setWidgetResizable(true);
    mountingArea->setFrameStyle(QFrame::NoFrame);

    m_authenticationPage = new Smb4KConfigPageAuthentication(this);
    QScrollArea *authenticationArea = new QScrollArea(this);
    authenticationArea->setWidget(m_authenticationPage);
    authenticationArea->setWidgetResizable(true);
    authenticationArea->setFrameStyle(QFrame::NoFrame);

    m_synchronizationPage = new Smb4KConfigPageSynchronization(this);
    QScrollArea *synchronizationArea = new QScrollArea(this);
    synchronizationArea->setWidget(m_synchronizationPage);
    synchronizationArea->setWidgetResizable(true);
    synchronizationArea->setFrameStyle(QFrame::NoFrame);

    m_synchronizationPage->setEnabled(!QStandardPaths::findExecutable(QStringLiteral("rsync")).isEmpty());

    m_customSettingsPage = new Smb4KConfigPageCustomSettings(this);
    QScrollArea *customSettingsArea = new QScrollArea(this);
    customSettingsArea->setWidget(m_customSettingsPage);
    customSettingsArea->setWidgetResizable(true);
    customSettingsArea->setFrameStyle(QFrame::NoFrame);

    m_profilesPage = new Smb4KConfigPageProfiles(this);
    QScrollArea *profilesArea = new QScrollArea(this);
    profilesArea->setWidget(m_profilesPage);
    profilesArea->setWidgetResizable(true);
    profilesArea->setFrameStyle(QFrame::NoFrame);

    m_bookmarksPage = new Smb4KConfigPageBookmarks(this);
    QScrollArea *bookmarksArea = new QScrollArea(this);
    bookmarksArea->setWidget(m_bookmarksPage);
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

        m_bookmarksPage->setCompletionItems(completionItems);
    }

    //
    // Pages to the configuration dialog
    //
    m_userInterface = addPage(userInterfaceArea, Smb4KSettings::self(), i18n("User Interface"), QStringLiteral("preferences-desktop"));
    m_network = addPage(networkArea, Smb4KSettings::self(), i18n("Network"), QStringLiteral("preferences-system-network-server-share-windows"));
    m_mounting = addPage(mountingArea, Smb4KMountSettings::self(), i18n("Mounting"), QStringLiteral("media-mount"));
    m_authentication = addPage(authenticationArea, Smb4KSettings::self(), i18n("Authentication"), QStringLiteral("preferences-desktop-user-password"));
    m_synchronization = addPage(synchronizationArea, Smb4KSettings::self(), i18n("Synchronization"), QStringLiteral("folder-sync"));
    m_bookmarks = addPage(bookmarksArea, Smb4KSettings::self(), i18n("Bookmarks"), QStringLiteral("bookmarks"));
    m_customSettings = addPage(customSettingsArea, Smb4KSettings::self(), i18n("Custom Settings"), QStringLiteral("settings-configure"));
    m_profiles = addPage(profilesArea, Smb4KSettings::self(), i18n("Profiles"), QStringLiteral("preferences-system-users"));

    //
    // Connections
    //
    connect(m_customSettingsPage, &Smb4KConfigPageCustomSettings::customSettingsModified, this, &Smb4KConfigDialog::slotEnableApplyButton);
    connect(m_authenticationPage, &Smb4KConfigPageAuthentication::walletEntriesModified, this, &Smb4KConfigDialog::slotEnableApplyButton);
    connect(m_bookmarksPage, &Smb4KConfigPageBookmarks::bookmarksModified, this, &Smb4KConfigDialog::slotEnableApplyButton);
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
        if (!m_mountingPage->checkSettings()) {
            KMessageBox::error(this, errorMessage);
            setCurrentPage(m_mounting);
            return false;
        }
    }

    if (!page || page == m_synchronization) {
        if (!m_synchronizationPage->checkSettings()) {
            KMessageBox::error(this, errorMessage);
            setCurrentPage(m_synchronization);
            return false;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigDialog::updateSettings()
{
    m_authenticationPage->saveLoginCredentials();
    m_customSettingsPage->saveCustomSettings();

    m_bookmarksPage->saveBookmarks();
    QMap<QString, QStringList> completionItems = m_bookmarksPage->completionItems();

    KConfigGroup bookmarkGroup(Smb4KSettings::self()->config(), "BookmarkEditor");

    bookmarkGroup.writeEntry("CategoryCompletion", completionItems[QStringLiteral("CategoryCompletion")]);
    bookmarkGroup.writeEntry("LabelCompletion", completionItems[QStringLiteral("LabelCompletion")]);
    bookmarkGroup.writeEntry("IpAddressCompletion", completionItems[QStringLiteral("IpAddressCompletion")]);
    bookmarkGroup.writeEntry("LoginCompletion", completionItems[QStringLiteral("LoginCompletion")]);
    bookmarkGroup.writeEntry("WorkgroupCompletion", completionItems[QStringLiteral("WorkgroupCompletion")]);

    if (m_profilesPage->profilesChanged()) {
        m_profilesPage->applyChanges();
        m_customSettingsPage->loadCustomSettings();
        m_bookmarksPage->loadBookmarks();
    }

    // FIXME: If the "Ok" button is pressed, this does not work as expected.
    (void)checkSettings();

    KConfigGroup group(Smb4KSettings::self()->config(), "ConfigDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    KConfigDialog::updateSettings();
}

void Smb4KConfigDialog::slotEnableApplyButton()
{
    bool enable = false;

    enable = m_authenticationPage->loginCredentialsChanged();

    if (!enable) {
        enable = m_customSettingsPage->customSettingsChanged();
    }

    if (!enable && m_bookmarksPage) {
        enable = m_bookmarksPage->bookmarksChanged();
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
