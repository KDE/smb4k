/***************************************************************************
    main.cpp  -  Main file of the Smb4K program.
                             -------------------
    begin                : Sam M�  1 14:57:21 CET 2003
    copyright            : (C) 2003-2008 by Alexander Reinholdt
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

// Qt includes
#include <QStringList>
#include <QFile>

// KDE includes
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kuniqueapplication.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kglobal.h>

// system includes
#include <stdlib.h>

// application specific includes
#include <smb4kmainwindow.h>
#include <core/smb4ksettings.h>
#include <core/smb4kcore.h>


static const char description[] =
  I18N_NOOP( "The advanced network neighborhood browser for KDE." );

static const char authors[] =
  I18N_NOOP( "\u00A9 2003-2009 Alexander Reinholdt\n\u00A9 2004-2007 "
             "Massimo Callegari" );


int main( int argc, char *argv[] )
{
  KAboutData aboutData( "smb4k",
                        0,
                        ki18n( "Smb4K" ),
                        VERSION,
                        ki18n( description ),
                        KAboutData::License_GPL_V2,
                        ki18n( authors ),
                        KLocalizedString(),
                        "http://smb4k.berlios.de",
                        "smb4k-bugs@lists.berlios.de" );

  // Authors:
  aboutData.addAuthor( ki18n( "Alexander Reinholdt" ),
                       ki18n( "Developer" ),
                       "dustpuppy@users.berlios.de" );
  aboutData.addAuthor( ki18n( "Massimo Callegari" ),
                       ki18n( "Developer" ),
                       "massimocallegari@yahoo.it" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  // Add our options here.
  KCmdLineArgs::addCmdLineOptions( options );

  KUniqueApplication::addCmdLineOptions();

  // This is not needed, because KUniqueApplication::start()
  // is called directly before the application is executed, but
  // we use it anyway. There is no performance impact.
  if ( !KUniqueApplication::start() )
  {
    exit( 0 );
  }

  KUniqueApplication app;

  // We need to set this property because otherwise the application
  // will quit when it is embedded into the system tray, the main window
  // is hidden and the last window that was opened through the system
  // tray is closed.
  app.setQuitOnLastWindowClosed( false );

  // Check the current config file and remove it if it belongs to
  // a version < 0.9.0.
  KConfig config( "smb4krc" );

  if ( !config.groupList().isEmpty() &&
       (config.groupList().contains( "Browse Options" ) != 0 ||
        config.groupList().contains( "Mount Options" ) != 0 ||
        config.groupList().contains( "Rsync" ) != 0 ||
        config.groupList().contains( "Super User Privileges") != 0 ||
        config.groupList().contains( "User Interface" ) != 0 ||
        config.groupList().contains( "System" ) != 0) )
  {
    int return_value = KMessageBox::warningContinueCancel( 0, i18n( "<qt><p>Smb4K now uses a different configuration system. Thus, your old settings are obsolete and you have to reconfigure the application.</p><p>To ensure a clean transition, the current configuration file will be removed.</p></qt>" ) );

    if ( return_value == KMessageBox::Continue )
    {
      QString file_name = KGlobal::dirs()->locateLocal( "config", "smb4krc", KGlobal::mainComponent() );

      if ( !file_name.isEmpty() && QFile::exists( file_name ) )
      {
        QFile::remove( file_name );
      }
    }
    else
    {
      exit( 0 );
    }
  }

  // Warn the user that support for the "super" program has been terminated.
  KConfigGroup su_group( &config, "SuperUser" );

  if ( su_group.hasKey( "SuperUserProgram" ) &&
       QString::compare( su_group.readEntry( "SuperUserProgram" ), "Super" ) == 0 )
  {
    KMessageBox::information( 0, i18n( "<qt>Support for the program 'super' has been terminated. You have to reconfigure Smb4K.</qt>" ) );

    su_group.deleteEntry( "SuperUserProgram" );

    su_group.sync();
  }
  else
  {
    // Do nothing
  }

  // Launch the application:
  Smb4KMainWindow *main_window = new Smb4KMainWindow();

  if ( !Smb4KSettings::embedIntoSystemTray() || !Smb4KSettings::startMainWindowDocked() )
  {
    main_window->show();
  }

  // Initialize the core.
//   Smb4KCore::self()->init();

  return app.exec();
}
