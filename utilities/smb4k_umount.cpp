/***************************************************************************
    smb4k_umount.cpp  -  The (new) unmount utility for Smb4K.
                             -------------------
    begin                : So Okt 03 2010
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
#include <kurl.h>
#include <kmountpoint.h>

// system includes
#include <stdlib.h>
#include <iostream>
using namespace std;

#define VERSION "0.17"


static const char description[] =
  I18N_NOOP( "This program umounts SMBFS/CIFS shares." );


static const char authors[] =
  I18N_NOOP( "(C) 2010, Alexander Reinholdt" );
  
bool find_program( const char *name, char *path )
{
  const char *paths[] = { "/bin/", "/sbin/", "/usr/bin/", "/usr/sbin/", "/usr/local/bin/", "/usr/local/sbin/" };
  string file = "";

  for ( uint i = 0; i < sizeof( paths ) / sizeof( char * ); i++ )
  {
    string p( paths[i] );
    p.append( name );

    if ( access( p.c_str(), X_OK ) == 0 )
    {
      file.assign( p );
      break;
    }
  }

  if ( !strcmp( file.c_str(), "" ) )
  {
    return false;
  }

  int len = strlen( file.c_str() ) + 1;

  (void) strncpy( path, file.c_str(), len );
  path[len-1] = '\0';

  return true;
}
  
int main( int argc, char *argv[] )
{
  KAboutData aboutData( "smb4k_umount",
                        "smb4k",
                        ki18n( "smb4k_umount" ),
                        VERSION,
                        ki18n( description ),
                        KAboutData::License_GPL_V2,
                        ki18n( authors ),
                        KLocalizedString(),
                        "http://smb4k.berlios.de",
                        "smb4k-bugs@lists.berlios.de" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
#ifdef Q_OS_LINUX
  options.add( "l", ki18n( "Perform a lazy unmount" ) );
#endif
  options.add( "+mountpoint", ki18n( "The mountpoint of the share" ) );

  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false /* no GUI */ );
  
  // Get the command line argument.
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
#ifdef Q_OS_LINUX
  bool lazy_unmount = false;
#endif
  KUrl url;
  
  if ( args->count() == 0 )
  {
    KCmdLineArgs::usageError( "No arguments provided." );
    exit( EXIT_FAILURE );
  }
  else
  {
#ifdef Q_OS_LINUX
    if ( args->isSet( "l" ) )
    {
      lazy_unmount = true;
    }
    else
    {
      // Do nothing
    }
#endif

    for ( int i = 0; i < args->count(); i++ )
    {
      if ( args->url( i ).isValid() )
      {
        // Check if the mountpoint is valid and the file system is
        // also correct.
        KMountPoint::List mountpoints = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );
        
        if ( mountpoints.findByPath( args->url( i ).toLocalFile() ) &&
#ifndef Q_S_FREEBSD
             QString::compare( mountpoints.findByPath( args->url( i ).toLocalFile() )->mountType(), "cifs" ) == 0 )
#else
             QString::compare( mountpoints.findByPath( args->url( i ).toLocalFile() )->mountType(), "smbfs" ) == 0 )
#endif
        {              
          url = args->url( i );
        }
        else
        {
          // Do nothing
        }
        
        break;
      }
      else
      {
        // Do nothing
      }
    }

    if ( url.isEmpty() )
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Invalid mountpoint specified." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      exit( EXIT_FAILURE );
    }
    else
    {
      // Do nothing
    }
  }
  
  // Find the kill binary.
  char path[255];
  
#ifndef Q_OS_FREEBSD
  if ( !find_program( "umount.cifs", path ) )
  {
    if ( !find_program( "umount", path ) )
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Could not find umount binary." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      exit( EXIT_FAILURE );
    }
    else
    {
      // Do nothing
    }
  }
#else
  if ( !find_program( "umount", path ) )
  {
    cerr << argv[0] << ": " << I18N_NOOP( "Could not find umount binary." ) << endl;
    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    exit( EXIT_FAILURE );
  }
#endif
  else
  {
    // Do nothing
  }
  
  QString command = QString( "%1 %2 %3" ).arg( path )
                                         .arg( lazy_unmount ? "-l" : "" )
                                         .arg( url.toLocalFile() );
  
  
  KProcess proc;
  proc.setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
  proc.setShellCommand( command );
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
        cerr << argv[0] << ": " << stderr.toUtf8().data(); // Output ends with newline
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