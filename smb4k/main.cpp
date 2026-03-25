/*
    Main file of the Smb4K program.

    SPDX-FileCopyrightText: 2003-2026 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "core/smb4kclient.h"
#include "core/smb4kcustomsettingsmanager.h"
#include "core/smb4kmounter.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksettings.h"
#include "smb4kmainwindow.h"

// Qt includes
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>
#include <QTimer>

// KDE includes
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KWindowSystem>
#include <kwindowsystem_version.h>

#include <smb4k_version.h>

using namespace Smb4KGlobal;

static void handleEnableProfiles(const QString &on)
{
    QString value = on.toLower();
    bool enable = false;

    if (value == QStringLiteral("on")) {
        enable = true;
    } else if (value == QStringLiteral("off")) {
        enable = false;
    } else {
        QTextStream stream(stderr);
        stream << i18n("The argument is invalid. Please use either 'on' or 'off'.") << Qt::endl;
        return;
    }

    Smb4KSettings::setUseProfiles(enable);
    Smb4KSettings::self()->save();
}

static void handleSetProfileOption(const QString &profileName)
{
    if (!Smb4KSettings::useProfiles()) {
        QTextStream stream(stderr);
        stream << i18n("Profiles are disabled.") << Qt::endl;
        return;
    }

    QStringList profiles = Smb4KProfileManager::self()->profilesList();

    if (!profiles.contains(profileName)) {
        QTextStream stream(stderr);
        stream << i18n("The profile name is invalid.") << Qt::endl;
        return;
    }

    Smb4KSettings::setActiveProfile(profileName);
    Smb4KSettings::self()->save();
}

static void handleListProfiles()
{
    if (!Smb4KSettings::useProfiles()) {
        QTextStream stream(stderr);
        stream << i18n("Profiles are disabled.") << Qt::endl;
        return;
    }

    QTextStream stream(stdout);
    QStringList profiles = Smb4KProfileManager::self()->profilesList();

    for (const QString &profile : std::as_const(profiles)) {
        stream << profile << (profile == Smb4KProfileManager::self()->activeProfile() ? i18n(" (active)") : QString()) << Qt::endl;
    }
}

static void handleRemountShares()
{
    QList<CustomSettingsPtr> remounts = Smb4KCustomSettingsManager::self()->sharesToRemount();

    for (const CustomSettingsPtr &settings : std::as_const(remounts)) {
        SharePtr share = SharePtr::create();
        share->setUrl(settings->url());
        share->setHostIpAddress(settings->ipAddress());
        share->setWorkgroupName(settings->workgroupName());
        Smb4KMounter::self()->mountShare(share);
    }
}

static void handleMountShare(const QStringList &urls) {
    for (const QString &u : std::as_const(urls)) {
        QUrl url;
        QString userInfo, userInput = u;

        if (userInput.startsWith(QStringLiteral("\\"))) {
            userInput.replace(QStringLiteral("\\"), QStringLiteral("/"));
        }

        // If a UNC with user info is passed, QUrl seems to make wrong assuptions,
        // so just cut out the user info and set it later.
        if (userInput.startsWith(QStringLiteral("//")) && userInput.contains(QStringLiteral("@"))) {
            userInfo = userInput.section(QStringLiteral("@"), 0, -2).section(QStringLiteral("/"), 2, -1);
            userInput.remove(userInfo + QStringLiteral("@"));
        }

        url = QUrl::fromUserInput(userInput).adjusted(QUrl::StripTrailingSlash);
        url.setScheme(QStringLiteral("smb"));

        if (!userInfo.isEmpty()) {
            url.setUserInfo(userInfo);
        }

        // We do not need to check the URL for validity here, because Smb4KMounter::mount()
        // will do this for us. So, we can just pass the created share object without
        // further processing.
        SharePtr share = SharePtr::create(url);
        Smb4KMounter::self()->mountShare(share);
    }
}

int main(int argc, char **argv)
{
    // Create the application
    QScopedPointer<QCoreApplication> app(argc > 1 ? new QCoreApplication(argc, argv) : new QApplication(argc, argv));

    // Connect the application with the translation catalog
    KLocalizedString::setApplicationDomain("smb4k");

    // Create the about data for Smb4K
    KAboutData aboutData(QStringLiteral("smb4k"),
                         i18n("Smb4K"),
                         QStringLiteral(SMB4K_VERSION_STRING),
                         i18n("Advanced network neighborhood browser and Samba share mounting utility"),
                         KAboutLicense::GPL_V2,
                         i18n("\u00A9 2003-2026 Alexander Reinholdt"),
                         QString(),
                         QStringLiteral("https://smb4k.sourceforge.io"));
    aboutData.setOrganizationDomain("kde.org");
    aboutData.setDesktopFileName(QStringLiteral("org.kde.") + aboutData.componentName());
    aboutData.addAuthor(i18n("Alexander Reinholdt"), i18n("Developer"), QStringLiteral("alexander.reinholdt@kdemail.net"));
    aboutData.addCredit(i18n("Wolfgang Geisendörfer"), i18n("Donator"), QStringLiteral("wdm-lin@gmx.net"));

    KAboutData::setApplicationData(aboutData);

    KCrash::setDrKonqiEnabled(true);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    QCommandLineOption enableProfilesOption(QStringLiteral("enable-profiles"),
                                            i18n("Enable or disable the usage of profiles. <arg> can be 'on' or 'off'."),
                                            QStringLiteral("arg"));
    parser.addOption(enableProfilesOption);

    QCommandLineOption setProfileOption(QStringLiteral("set-profile"), i18n("Set the active profile to <profile>."), QStringLiteral("profile"));
    parser.addOption(setProfileOption);

    QCommandLineOption listProfilesOption(QStringLiteral("list-profiles"), i18n("List all profiles."));
    parser.addOption(listProfilesOption);

    QCommandLineOption remountSharesOption(QStringLiteral("remount-shares"), i18n("Remount all shares that were previously used."));
    parser.addOption(remountSharesOption);

    QCommandLineOption mountShareOption(QStringLiteral("mount"), i18n("Mount the share pointed to by <url>."), QStringLiteral("url"));
    parser.addOption(mountShareOption);

    parser.process(*app);
    aboutData.processCommandLine(&parser);

    QApplication *fullApp = qobject_cast<QApplication *>(app.data());

    // Handle non-GUI mode
    if (!fullApp) {
        if (parser.isSet(enableProfilesOption)) {
            handleEnableProfiles(parser.value(enableProfilesOption));
        } else if (parser.isSet(setProfileOption)) {
            handleSetProfileOption(parser.value(setProfileOption));
        } else if (parser.isSet(listProfilesOption)) {
            handleListProfiles();
        } else if (parser.isSet(remountSharesOption)) {
            handleRemountShares();
        } else if (parser.isSet(mountShareOption)) {
            handleMountShare(parser.values(mountShareOption));
        }

        QTimer::singleShot(0, &QCoreApplication::quit);
        return app->exec();
    }

    // We need to set this property because otherwise the application
    // will quit when it is embedded into the system tray, the main window
    // is hidden and the last window that was opened through the system
    // tray is closed.
    fullApp->setQuitOnLastWindowClosed(false);
    fullApp->setWindowIcon(QIcon::fromTheme(QStringLiteral("smb4k")));

    Smb4KMainWindow *mainWindow = new Smb4KMainWindow();
    mainWindow->setVisible(!Smb4KSettings::startMainWindowDocked());

    // Unique application
    const KDBusService service(KDBusService::Unique);

    QObject::connect(&service, &KDBusService::activateRequested, mainWindow, [mainWindow](const QStringList & /*args*/, const QString & /*workingDir*/) {
        if (mainWindow->isVisible()) {
            KWindowSystem::updateStartupId(mainWindow->windowHandle());
            KWindowSystem::activateWindow(mainWindow->windowHandle());
        } else {
            mainWindow->setVisible(true);
        }
    });

    Smb4KClient::self()->start();
    Smb4KMounter::self()->start();

    return app->exec();
}
