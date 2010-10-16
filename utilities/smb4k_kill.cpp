/***************************************************************************
    smb4k_kill  -   The (new) kill utility for Smb4K.
                             -------------------
    begin                : So Sep 12 2010
    copyright            : (C) 2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QDebug>

// KDE includes
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kstandarddirs.h>

// system includes
#include <stdlib.h>
#include <iostream>
using namespace std;

#define VERSION "0.8"


static const char description[] =
  I18N_NOOP( "This program kills processes invoked by Smb4K." );


static const char authors[] =
  I18N_NOOP( "(C) 2010, Alexander Reinholdt" );
  
  
int main( int argc, char *argv[] )
{
  KAboutData aboutData( "smb4k_kill",
                        "smb4k",
                        ki18n( "smb4k_kill" ),
                        VERSION,
                        ki18n( description ),
                        KAboutData::License_GPL_V2,
                        ki18n( authors ),
                        KLocalizedString(),
                        "http://smb4k.berlios.de",
                        "smb4k-bugs@lists.berlios.de" );

  QCoreApplication app( argc, argv );
  KComponentData componentData( aboutData );
                        
  KCmdLineArgs::init( argc, argv, componentData.aboutData() );

  KCmdLineOptions options;
  options.add( "+pid", ki18n( "The PID of the process that is to be killed" ) );

  KCmdLineArgs::addCmdLineOptions( options );

  // Get the command line argument.
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QStringList args_list;
  
  if ( args->count() == 1 )
  {
    args_list += args->arg( args->count() - 1 );
  }
  else
  {
    KCmdLineArgs::usageError( "No argument provided." );
    exit( EXIT_FAILURE );
  }
  
  // Find the kill binary.
  QStringList paths;
  paths << "/bin/";
  paths << "/sbin/";
  paths << "/usr/bin/";
  paths << "/usr/sbin/";
  paths << "/usr/local/bin/";
  paths << "/usr/local/sbin/";
  
  QString kill;
  
  for ( int i = 0; i < paths.size(); i++ )
  {
    kill = componentData.dirs()->findExe( "kill", paths.at( i ) );
    
    if ( !kill.isEmpty() )
    {
      break;
    }
    else
    {
      continue;
    }
  }
  
  if ( kill.isEmpty() )
  {
    cerr << argv[0] << ": " << I18N_NOOP( "Could not find kill binary." ) << endl;
    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }
  
  // Set up the process.
  KProcess proc;
  proc.setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
  proc.setProgram( kill, args_list );
  proc.setOutputChannelMode( KProcess::SeparateChannels );
  
  switch ( proc.execute() )
  {
    case -2:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "The internal process could not be started." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      exit( EXIT_FAILURE );
      break;
    }
    case -1:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "The internal process crashed with exit code " ) << proc.exitCode() << "." << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      exit( EXIT_FAILURE );
      break;
    }
    default:
    {
      // Check if there is output on stderr.
      QString stderr = QString::fromUtf8( proc.readAllStandardError() );
      
      if ( !stderr.isEmpty() )
      {
        cerr << argv[0] << ": " << I18N_NOOP( "An error occurred:") << endl;
        cerr << stderr.toUtf8().data(); // Output ends with newline
        cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
        exit( EXIT_FAILURE );
      }
      else
      {
        // Do nothing
      }
      
      // Check if there is output on stdout.
      QString stdout = QString::fromUtf8( proc.readAllStandardOutput() );
      
      if ( !stdout.isEmpty() )
      {
        cerr << argv[0] << ": " << stdout.toUtf8().data(); // Output ends with newline
      }
      else
      {
        // Do nothing
      }
      
      break;
    }
  }
  
  app.exit( EXIT_SUCCESS );
}
