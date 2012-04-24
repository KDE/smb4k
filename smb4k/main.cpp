/***************************************************************************
    main.cpp  -  Main file of the Smb4K program.
                             -------------------
    begin                : Sam M�  1 14:57:21 CET 2003
    copyright            : (C) 2003-2012 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#include <config.h>

// Qt includes
#include <QStringList>
#include <QFile>

// KDE includes
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kuniqueapplication.h>
#include <kglobal.h>

// application specific includes
#include <smb4kmainwindow.h>
#include <core/smb4ksettings.h>
#include <core/smb4kglobal.h>

using namespace Smb4KGlobal;


static const char description[] =
  I18N_NOOP( "The advanced network neighborhood browser for KDE." );

static const char authors[] =
  I18N_NOOP( "\u00A9 2003-2012 Alexander Reinholdt" );


extern "C" KDE_EXPORT int kdemain( int argc, char **argv )
{
  KAboutData aboutData( "smb4k",
                        0,
                        ki18n( "Smb4K" ),
                        VERSION,
                        ki18n( description ),
                        KAboutData::License_GPL_V2,
                        ki18n( authors ),
                        KLocalizedString(),
                        "http://smb4k.sourceforge.net",
                        "smb4k-bugs@lists.sourceforge.net" );

  // Authors:
  aboutData.addAuthor( ki18n( "Alexander Reinholdt" ),
                       ki18n( "Developer" ),
                       "alexander.reinholdt@kdemail.net" );

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

  // Launch the main window.
  Smb4KMainWindow *main_window = new Smb4KMainWindow();
  main_window->setVisible( !Smb4KSettings::startMainWindowDocked() );

  // Initialize the core. Use a busy cursor.
  initCore( true );

  return app.exec();
}
