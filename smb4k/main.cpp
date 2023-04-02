/*
    Main file of the Smb4K program.

    SPDX-FileCopyrightText: 2003-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "core/smb4kglobal.h"
#include "core/smb4ksettings.h"
#include "smb4kmainwindow.h"

// Qt includes
#include <QApplication>
#include <QString>

// KDE includes
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KWindowSystem>
#include <kwindowsystem_version.h>

#include <smb4k_version.h>

using namespace Smb4KGlobal;

int main(int argc, char **argv)
{
#if QT_VERSION <= QT_VERSION_CHECK(6, 0, 0)
    // Set attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    // Create the application
    QApplication app(argc, argv);

    // Connect the application with the translation catalog
    KLocalizedString::setApplicationDomain("smb4k");

    // Create the about data for Smb4K
    KAboutData aboutData(QStringLiteral("smb4k"),
                         i18n("Smb4K"),
                         QStringLiteral(SMB4K_VERSION_STRING),
                         i18n("Advanced network neighborhood browser and Samba share mounting utility"),
                         KAboutLicense::GPL_V2,
                         i18n("\u00A9 2003-2023 Alexander Reinholdt"),
                         QString(),
                         QStringLiteral("https://smb4k.sourceforge.io"));

    // DBus prefix
    aboutData.setOrganizationDomain("kde.org");
    aboutData.setDesktopFileName(QStringLiteral("org.kde.") + aboutData.componentName());

    // Authors
    aboutData.addAuthor(i18n("Alexander Reinholdt"), i18n("Developer"), QStringLiteral("alexander.reinholdt@kdemail.net"));

    // Credits:
    // People who supported the Smb4K development by donating
    // money
    aboutData.addCredit(i18n("Wolfgang Geisendörfer"), i18n("Donator"), QStringLiteral("wdm-lin@gmx.net"));

    // Register about data
    KAboutData::setApplicationData(aboutData);

    // Now add the data to the application
    app.setApplicationName(aboutData.componentName());
    app.setApplicationDisplayName(aboutData.displayName());
    app.setOrganizationDomain(aboutData.organizationDomain());
    app.setApplicationVersion(aboutData.version());

    // We need to set this property because otherwise the application
    // will quit when it is embedded into the system tray, the main window
    // is hidden and the last window that was opened through the system
    // tray is closed.
    app.setQuitOnLastWindowClosed(false);

    // Program icon
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("smb4k")));

    // Launch the main window
    Smb4KMainWindow *mainWindow = new Smb4KMainWindow();
    mainWindow->setVisible(!Smb4KSettings::startMainWindowDocked());

    // Initialize the core. Use a busy cursor.
    initCore(true);

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

    // Use a crash handler
    KCrash::setDrKonqiEnabled(true);

    // Start the application
    return app.exec();
}
