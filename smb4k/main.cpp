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
  I18N_NOOP( "\u00A9 2003-2008, Alexander Reinholdt\n\u00A9 2004-2007, "
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

  // All our credits:
  aboutData.addCredit( ki18n( "Leopold Palomo Avellaneda" ),
                       ki18n( "Catalan translation" ),
                       "lepalom@wol.es" );
  aboutData.addCredit( ki18n( "Radoslaw Zawartko" ),
                       ki18n( "Polish translation" ),
                       "radzaw@lnet.szn.pl" );
  aboutData.addCredit( ki18n( "Nick Chen" ),
                       ki18n( "Chinese Simplified translation" ),
                       "nick_chen75@hotmail.com" );
  aboutData.addCredit( ki18n( "Stanislav Yudin" ),
                       ki18n( "Russian translation" ),
                       "decvar@mail.berlios.de" );
  aboutData.addCredit( ki18n( "Marc Hansen" ),
                       ki18n( "Swedish translation and intensive testing" ),
                       "marc.hansen@gmx.de" );
  aboutData.addCredit( ki18n( "Giovanni Degani" ),
                       ki18n( "Brazilian Portuguese translation" ),
                       "tiefox@terra.com.br" );
  aboutData.addCredit( ki18n( "Ivan Petrouchtchak" ),
                       ki18n( "Ukrainian translation" ),
                       "iip@telus.net" );
  aboutData.addCredit( ki18n( "Karoly Barcza" ),
                       ki18n( "Hungarian translation" ),
                       "kbarcza@blackpanther.hu" );
  aboutData.addCredit( ki18n( "Quique" ),
                       ki18n( "Spanish translation" ),
                       "quique@sindominio.net" );
  aboutData.addCredit( ki18n( "Michal Šulek" ),
                       ki18n( "Slovak translation" ),
                       "reloadshot@atlas.sk" );
  aboutData.addCredit( ki18n( "Nicolas Ternisien" ),
                       ki18n( "French translation" ),
                       "nicolast@libertysurf.fr" );
  aboutData.addCredit( ki18n( "Toyohiro Asukai" ),
                       ki18n( "Japanese translation" ),
                       "toyohiro@ksmplus.com" );
  aboutData.addCredit( ki18n( "Atanas Mavrov" ),
                       ki18n( "Bulgarian translation" ),
                       "bugar@developer.bg" );
  aboutData.addCredit( ki18n( "Isidoro Russo" ),
                       ki18n( "Italian translation" ),
                       "risidoro@aliceposta.it" );
  aboutData.addCredit( ki18n( "Nils Kristian Tomren" ),
                       ki18n( "Norwegian translations" ),
                       "project@nilsk.net" );
  aboutData.addCredit( ki18n( "Alois Nešpor" ),
                       ki18n( "Czech translation" ),
                       "alois.nespor@seznam.cz" );
  aboutData.addCredit( ki18n( "Martín Carr" ),
                       ki18n( "Spanish translation" ),
                       "tincarr@gmail.com" );
  aboutData.addCredit( ki18n( "Görkem Çetin" ),
                       ki18n( "Turkish translation" ),
                       "gorkem@gorkemcetin.com" );
  aboutData.addCredit( ki18n( "Jack Liu" ),
                       ki18n( "Chinese Traditional translation" ),
                       "chuany@chuany.net" );
  aboutData.addCredit( ki18n( "Arnar Leósson" ),
                       ki18n( "Icelandic translation" ),
                       "leosson@frisurf.no" );
  aboutData.addCredit( ki18n( "Michael Brinkloev" ),
                       ki18n( "Danish translation" ),
                       "mhb@qxp.dk" );
  aboutData.addCredit( ki18n( "Joop Beris" ),
                       ki18n( "Dutch translation" ),
                       "jberis@risse.nl" );
  aboutData.addCredit( ki18n( "Lamarque V. Souza" ),
                       ki18n( "Brazilian Portuguese translation" ),
                       "lamarque_souza@hotmail.com" );
  aboutData.addCredit( ki18n( "Serdar Soytetir" ),
                       ki18n( "Turkish translation" ),
                       "sendirom@gmail.com" );
  aboutData.addCredit( ki18n( "Wei-Lun Chao" ),
                       ki18n( "Chinese Traditional translation" ),
                       "chaoweilun@users.berlios.de" );
  aboutData.addCredit( ki18n( "Rashid N. Achilov" ),
                       ki18n( "Testing of Smb4K under FreeBSD" ),
                       "achilov-rn@askd.ru" );
  aboutData.addCredit( ki18n( "Jerzy Trzeciak" ),
                       ki18n( "Polish translation" ),
                       "artusek@wp.pl" );

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
