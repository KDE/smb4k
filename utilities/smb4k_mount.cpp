/***************************************************************************
    smb4k_umount.cpp  -  The (new) mount utility for Smb4K.
                             -------------------
    begin                : So Okt 10 2010
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
#include <QCoreApplication>
#include <QDir>

// KDE includes
#include <klocale.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kprocess.h>
#include <kmountpoint.h>

// system includes
#include <iostream>
using namespace std;

#define VERSION "0.11"


static const char description[] =
  I18N_NOOP( "This program mounts SMBFS/CIFS shares." );


static const char authors[] =
  I18N_NOOP( "(C) 2010, Alexander Reinholdt" );
  

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "smb4k_mount",
                        "smb4k",
                        ki18n( "smb4k_mount" ),
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
#ifndef Q_OS_FREEBSD
  options.add( "o <optlist>", ki18n( "A comma-separated list of mount options" ) );
#else
  options.add( "E <cs1:cs2>",   ki18n( "The local (<cs1>) and the server's (<cs2>) character set" ) );
  options.add( "I <host>",      ki18n( "Do not use NetBIOS name resolver and connect directly to <host>, "
                                       "which can be either a valid DNS name or an IP address" ) );
  options.add( "L <locale>",    ki18n( "Use <locale> for lower/upper case conversion routines" ) );
  options.add( "M <cr:sr>",     ki18n( "Assign access rights to the newly created connection" ) );
  options.add( "N",             ki18n( "Do not ask for a password" ) );
  options.add( "O <co:cg/so:sg> ", ki18n( "Assign owner/group attributes to the newly created connection" );
  options.add( "R <count>",     ki18n( "How many retries should be done before the SMB requester decides "
                                       "to drop the connection" ) );
  options.add( "T <timeout>",   ki18n( "Timeout in seconds for each request" ) );
  options.add( "U <user>",      ki18n( "Username to authenticate with" ) );
  options.add( "W <workgroup>", ki18n( "This option specifies the workgroup to be used in the "
                                       "authentication request" ) );
  options.add( "c <case>",      ki18n( "Set a case option which affects name representation" ) );
  options.add( "f <mode>",      ki18n( "Specify permissions that should be assigned to files" ) );
  options.add( "d <mode>",      ki18n( "Specify permissions that should be assigned to directories" ) );
  options.add( "u <uid>",       ki18n( "User ID assigned to files" ) );
  options.add( "g <gid>",       ki18n( "Group ID assigned to files" ) );
  
  options.add( "",              ki18n( "For further information see the manual page of mount_smbfs." ) );
#endif
  options.add( "+unc", ki18n( "The UNC of the remote share" ) );
  options.add( "+mountpoint", ki18n( "The mountpoint for the remote share" ) );
  
  KCmdLineArgs::addCmdLineOptions( options );
  
  // Get the command line argument.
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QStringList args_list;
  KUrl local;
  KUrl remote;
  
  if ( args->count() == 0 )
  {
    KCmdLineArgs::usageError( "No arguments provided." );
    exit( EXIT_FAILURE );
  }
  else
  {
#ifndef Q_OS_FREEBSD
    if ( args->isSet( "o" ) )
    {
      args_list += QString( "-o %1" ).arg( args->getOption( "o" ) );
    }
    else
    {
      // Do nothing
    }
#else
    if ( args->isSet( "E" ) )
    {
      args_list += QString( "-E %1" ).arg( args->getOption( "E" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "I" ) )
    {
      args_list += QString( "-I %1" ).arg( args->getOption( "I" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "L" ) )
    {
      args_list += QString( "-L %1" ).arg( args->getOption( "L" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "M" ) )
    {
      args_list += QString( "-M %1" ).arg( args->getOption( "M" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "N" ) )
    {
      args_list += "-N";
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "O" ) )
    {
      args_list += QString( "-O %1" ).arg( args->getOption( "O" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "R" ) )
    {
      args_list += QString( "-R %1" ).arg( args->getOption( "R" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "T" ) )
    {
      args_list += QString( "-T %1" ).arg( args->getOption( "T" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "U" ) )
    {
      args_list += QString( "-U %1" ).arg( args->getOption( "U" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "W" ) )
    {
      args_list += QString( "-W %1" ).arg( args->getOption( "W" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "c" ) )
    {
      args_list += QString( "-c %1" ).arg( args->getOption( "c" ) );
    }
    else
    {
      // Do nothing
    }
    
     if ( args->isSet( "f" ) )
    {
      args_list += QString( "-f %1" ).arg( args->getOption( "f" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "d" ) )
    {
      args_list += QString( "-d %1" ).arg( args->getOption( "d" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "u" ) )
    {
      args_list += QString( "-u %1" ).arg( args->getOption( "u" ) );
    }
    else
    {
      // Do nothing
    }
    
    if ( args->isSet( "g" ) )
    {
      args_list += QString( "-g %1" ).arg( args->getOption( "g" ) );
    }
    else
    {
      // Do nothing
    }
#endif

    for ( int i = 0; i < args->count(); i++ )
    {
      // Check whether the argument is a local path.
      if ( args->url( i ).isValid() )
      {
        // This is the local one
        if ( QDir( args->url( i ).toLocalFile() ).exists() )
        {
          // Check that the mountpoint is not already in use.
          KMountPoint::List mountpoints = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );
          
          for( int j = 0; j < mountpoints.size(); j++ )
          {
#ifndef Q_OS_FREEBSD
            if ( QString::compare( args->url( i ).toLocalFile(), mountpoints.at( j )->mountPoint(), Qt::CaseInsensitive ) == 0 )
#else
            if ( QString::compare( args->url( i ).toLocalFile(), mountpoints.at( j )->mountPoint(), Qt::CaseInsensitive ) == 0 )
#endif
            {
              cerr << argv[0] << ": " << I18N_NOOP( "The mountpoint is already in use." ) << endl;
              cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
              exit( EXIT_FAILURE );
              break;
            }
            else
            {
              continue;
            }
          }          
          
          local = args->url( i );
        }
        else
        {
          // This is the remote one
          remote = args->url( i );
        }
      }
      else
      {
        // Do nothing
      }
    }
  }
  
  // Find the mount binary.
  QStringList paths;
  paths << "/bin/";
  paths << "/sbin/";
  paths << "/usr/bin/";
  paths << "/usr/sbin/";
  paths << "/usr/local/bin/";
  paths << "/usr/local/sbin/";
  
  QString mount;
  
  for ( int i = 0; i < paths.size(); i++ )
  {
#ifndef Q_OS_FREEBSD
    mount = componentData.dirs()->findExe( "mount.cifs", paths.at( i ) );
#else
    mount = componentData.dirs()->findExe( "mount_smbfs", paths.at( i ) );
#endif
    
    if ( !mount.isEmpty() )
    {
      break;
    }
    else
    {
      continue;
    }
  }
  
  if ( mount.isEmpty() )
  {
    cerr << argv[0] << ": " << I18N_NOOP( "Could not find umount binary." ) << endl;
    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }
  
#ifndef Q_OS_FREEBSD
  args_list.prepend( local.pathOrUrl() );
  args_list.prepend( remote.pathOrUrl() );
#else
  args_list += remote.pathOrUrl();
  args_list += local.pathOrUrl();
#endif
                                            
  KProcess proc;
  proc.setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
  proc.setProgram( mount, args_list );
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