/*
    Main file of the Smb4K program.
    -------------------
    begin                : Sam Mär  1 14:57:21 CET 2003
    SPDX-FileCopyrightText: 2003-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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

// application specific includes
#include "core/smb4kglobal.h"
#include "core/smb4ksettings.h"
#include "smb4kmainwindow.h"

// Qt includes
#include <QApplication>
#include <QString>

// KDE includes
#include <KCoreAddons/KAboutData>
#include <KCoreAddons/Kdelibs4ConfigMigrator>
#include <KCoreAddons/Kdelibs4Migration>
#include <KCrash/KCrash>
#include <KDBusAddons/KDBusService>
#include <KI18n/KLocalizedString>

#include <smb4k_version.h>

using namespace Smb4KGlobal;

int main(int argc, char **argv)
{
    // Migrate KDE4 configuration and XML files
    QStringList configFiles;
    configFiles << QLatin1String("smb4krc");

    Kdelibs4ConfigMigrator migrator(QLatin1String("smb4k"));
    migrator.setConfigFiles(configFiles);

    if (migrator.migrate()) {
        Kdelibs4Migration migration;

        if (migration.kdeHomeFound()) {
            //
            // NOTE: We need the 'smb4k' subdirectory, since no QApplication
            // is running at this point.
            //

            // New location
            QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + "smb4k";

            // XML files
            QString bookmarks = migration.locateLocal("data", "smb4k/bookmarks.xml");
            QString options = migration.locateLocal("data", "smb4k/custom_options.xml");
            QString homes = migration.locateLocal("data", "smb4k/homes_shares.xml");

            // Copy the files if they don't already exist
            if (!bookmarks.isEmpty() && QFile().exists(bookmarks)) {
                if (!QDir().exists(path)) {
                    QDir().mkpath(path);
                } else {
                    // Do nothing
                }

                QFile(bookmarks).copy(path + QDir::separator() + "bookmarks.xml");
            } else {
                // Do nothing
            }

            if (!options.isEmpty() && QFile().exists(options)) {
                if (!QDir().exists(path)) {
                    QDir().mkpath(path);
                } else {
                    // Do nothing
                }

                QFile(options).copy(path + QDir::separator() + "custom_options.xml");
            } else {
                // Do nothing
            }

            if (!homes.isEmpty() && QFile().exists(homes)) {
                if (!QDir().exists(path)) {
                    QDir().mkpath(path);
                } else {
                    // Do nothing
                }

                QFile(homes).copy(path + QDir::separator() + "homes_shares.xml");
            } else {
                // Do nothing
            }
        } else {
            // Do nothing
        }
    } else {
        // Do nothing
    }

    // Set attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

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
                         i18n("\u00A9 2003-2021 Alexander Reinholdt"),
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
    app.setWindowIcon(QIcon::fromTheme(QLatin1String("smb4k")));

    // Launch the main window
    Smb4KMainWindow *mainWindow = new Smb4KMainWindow();
    mainWindow->setVisible(!Smb4KSettings::startMainWindowDocked());

    // Initialize the core. Use a busy cursor.
    initCore(true);

    // Unique application
    const KDBusService service(KDBusService::Unique);

    // Use a crash handler
    KCrash::setDrKonqiEnabled(true);

    // Start the application
    return app.exec();
}
