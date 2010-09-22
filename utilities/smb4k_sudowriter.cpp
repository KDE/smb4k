/***************************************************************************
    smb4k_sudowriter  -  This program writes to the sudoers file. It
    belongs to the utility programs of Smb4K.
                             -------------------
    begin                : Di Jul 29 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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
#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QString>
#include <QByteArray>
#include <QStringList>

// KDE includes
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kuser.h>
#include <kstandarddirs.h>

// system includes
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <iostream>
#include <stdlib.h>
using namespace std;

#define VERSION "0.4"


static const char description[] =
  I18N_NOOP( "This program writes to the sudoers file." );


static const char authors[] =
  I18N_NOOP( "(C) 2008-2010, Alexander Reinholdt" );


int createLockFile( QFile *file )
{
  // Return values:
  // 0 -- OK
  // 1 -- Could not create lock file
  // 2 -- Lock file exists
  
  file->setFileName( QString() );
  
  QStringList dirs;
  dirs << "/var/lock";
  dirs << "/var/tmp";
  dirs << "/tmp";
  
  // Check whether the file exists and set the file path
  // to the first one the user can write to.
  for ( int i = 0; i < dirs.size(); ++i )
  {
    QFileInfo info_file( dirs.at( i ), "smb4k.lock" );
    info_file.setCaching( false );
    
    QFileInfo info_dir( dirs.at( i ) );
    info_dir.setCaching( false );
    
    if ( !info_file.exists() )
    {
      if ( info_dir.permission( QFile::WriteUser ) && file->fileName().isEmpty() )
      {
        file->setFileName( info_file.absoluteFilePath() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      return 2;
    }
  }
  
  if ( !file->fileName().isEmpty() )
  {
    if ( file->open( QIODevice::WriteOnly|QIODevice::Text ) )
    {
      QTextStream ts( file );
      // Note: With Qt 4.3 this seems to be obsolete, we'll keep
      // it for now.
      ts.setCodec( QTextCodec::codecForLocale() );
      
#if QT_VERSION >= 0x040400
      if ( kapp )
      {
        ts << kapp->applicationPid() << endl;
      }
      else
      {
        ts << "0" << endl;
      }
#else
      ts << "0" << endl;
#endif
      file->close();
    }
    else
    {
      return 1;
    }
  }
  else
  {
    return 1;
  }
  
  return 0;
}
  
  
void removeLockFile( QFile *file )
{
  // Just remove the lock file. No further checking is needed.
  if ( file->exists() )
  {
    file->remove();
  }
  else
  {
    // Do nothing
  }
}


bool checkUsers( const QStringList &list )
{
  for ( int i = 0; i < list.size(); ++i )
  {
    KUser user( list.at( i ) );
    
    if ( !user.isValid() )
    {
      return false;
    }
    else
    {
      // Do nothing
    }
  }

  return true;
}


int findSudoers( QFile &file, mode_t &perms )
{
  // Return values 
  // 0 -- OK
  // 1 -- Not found
  // 2 -- Irregular file
  // 3 -- Not accessible
  // * -- error number
  
  file.setFileName( QString() );
  
  QStringList paths;
  paths << "/etc";
  paths << "/usr/local/etc";
  
  for ( int i = 0; i < paths.size(); ++i )
  {
    QFileInfo info( paths.at( i ), "sudoers" );
    info.setCaching( false );

    if ( info.exists() )
    {
      // Stat the file.
      struct stat buf;
      
      if ( lstat( info.absoluteFilePath().toUtf8().data(), &buf ) == -1 )
      {
        return errno;
      }
      else
      {
        // Do nothing
      }

      // Get the ids of the groups the user is in and check if
      // one of them matches buf.st_gid.
      KUser user( geteuid() );
      QList<KUserGroup> gids = user.groups();
      gid_t sup_gid = 65534; // set this to gid 'nobody' for initialization
      bool found_gid = false;

      for ( int i = 0; i < gids.size(); ++i )
      {
        if ( gids.at( i ).gid() == buf.st_gid )
        {
          sup_gid = gids.at( i ).gid();
          found_gid = true;
          break;
        }
        else
        {
          continue;
        }
      }

      if ( !S_ISREG( buf.st_mode ) || S_ISFIFO( buf.st_mode ) || S_ISLNK( buf.st_mode ) )
      {
        return 2;
      }
      else
      {
        // Do nothing
      }

      // Check the access rights. We need to read the file.
      if ( buf.st_uid != geteuid() && !found_gid &&
          (buf.st_mode & 00004) != (S_IWOTH | S_IROTH) /* others */ )
      {
        return 3;
      }
      else
      {
        // Do nothing
      }
      
      perms = buf.st_mode;
      
      file.setFileName( info.absoluteFilePath() );
      break;
    }
    else
    {
      continue;
    }
  }
  
  // This is correct!
  // false == 0
  // true == 1
  return file.fileName().isEmpty();
}


int readSudoers( QFile &file, const mode_t &perms, QStringList &contents )
{
  // Return values:
  // 0 -- OK
  // 1 -- Could not read file
  // * -- Error code
  
  int return_value = 0;
  
  // Now set the permissions, so that we can read and write the file.
  if ( chmod( file.fileName().toUtf8().data(), S_IRUSR ) == -1 )
  {
    return_value = errno;
  }
  else
  {
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QTextStream ts( &file);
      // Note: With Qt 4.3 this seems to be obsolete, we'll keep
      // it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      while ( !ts.atEnd() )
      {
        contents.append( ts.readLine( 0 ) );
      }

      file.close();
    }
    else
    {
      return_value = 1;
    }
    
    // Reset the permissions.
    if ( chmod( file.fileName().toUtf8().data(), perms ) == -1 )
    {
      return_value = errno;
    }
    else
    {
      // Do nothing
    }
  }

  return return_value;
}


int writeSudoers( QFile &file, const mode_t &perms, const QStringList &contents )
{
  // Return values:
  // 0 -- OK
  // 1 -- Could not read file
  // * -- Error code
  
  int return_value = 0;
  
  // Now set the permissions, so that we can read and write the file.
  if ( chmod( file.fileName().toUtf8().data(), S_IWUSR ) == -1 )
  {
    return_value = errno;
  }
  else
  {
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, we'll keep
      // it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      ts << contents.join( "\n" ) << endl;

      file.close();
    }
    else
    {
      return_value = 1;
    }
    
    // Reset the permissions.
    if ( chmod( file.fileName().toUtf8().data(), perms ) == -1 )
    {
      return_value = errno;
    }
    else
    {
      // Do nothing
    }
  }

  return return_value;
}


int modifyContents( QStringList &contents, const QStringList &add, const QStringList &remove )
{
  // Return value
  // 0 -- OK
  // 1 -- Could not find utility programs
  // 2 -- Wrong format
  // * -- Error code

  int return_value = 0;
  
  // Find the beginning and the end of the entries in
  // the sudoers file:
  int begin = contents.indexOf( "# Entries for Smb4K users.", 0 );
  int end = contents.lastIndexOf( "# End of Smb4K user entries.", -1 );
  
  if ( begin == -1 && end == -1 )
  {
    if ( !add.isEmpty() )
    {
      QString smb4k_kill   = KGlobal::dirs()->findResource( "exe", "smb4k_kill" );
      QString smb4k_mount  = KGlobal::dirs()->findResource( "exe", "smb4k_mount" );
      QString smb4k_umount = KGlobal::dirs()->findResource( "exe", "smb4k_umount" );
      
      if ( !smb4k_kill.isEmpty() && !smb4k_mount.isEmpty() && !smb4k_umount.isEmpty() )
      {
        // Get the hostname.
        size_t hostnamelen = 255;
        char *hn = new char[hostnamelen];

        if ( gethostname( hn, hostnamelen ) == -1 )
        {
          return_value = errno;
        }
        else
        {
          QString hostname( hn );
          delete [] hn;

          // Add the new entries.
          if ( !contents.last().trimmed().isEmpty() )
          {
            contents.append( "" );
          }
          else
          {
            // Do not add empty line to the end.
          }

          contents.append( "# Entries for Smb4K users." );
          contents.append( "# Generated by Smb4K. Please do not modify!" );
          contents.append( "User_Alias\tSMB4KUSERS = "+QString( "%1" ).arg( add.join( "," ) ) );
          contents.append( "Defaults:SMB4KUSERS\tenv_keep += \"PASSWD USER\"" );
          contents.append( "SMB4KUSERS\t"+hostname+" = NOPASSWD: "+smb4k_kill );
          contents.append( "SMB4KUSERS\t"+hostname+" = NOPASSWD: "+smb4k_umount );
          contents.append( "SMB4KUSERS\t"+hostname+" = NOPASSWD: "+smb4k_mount );
          contents.append( "# End of Smb4K user entries." );
        }
      }
      else
      {
        return_value = 1;
      }
    }
    else
    {
      // Do nothing
    }
  }
  else if ( begin != -1 && end != -1 )
  {
    // Modify list of users
    QStringList users;
    int index = 0;
    
    for ( int i = begin; i != end; ++i )
    {
      if ( contents.at( i ).startsWith( "User_Alias\tSMB4KUSERS" ) )
      {
        index = i;
        users = contents.at( i ).section( "=", 1, 1 ).trimmed().split( ",", QString::SkipEmptyParts );
        
        // Add users
        for ( int j = 0; j < add.size(); ++j )
        {
          if ( !users.contains( add.at( j ) ) )
          {
            users << add.at( j );
          }
          else
          {
            // Do nothing
          }
        }
        
        // Remove users
        for ( int j = 0; j < remove.size(); ++j )
        {
          if ( users.contains( remove.at( j ) ) )
          {
            users.removeAll( remove.at( j ) );
          }
          else
          {
            // Do nothing
          }
        }
        
        break;
      }
      else
      {
        continue;
      }
    }
    
    if ( !users.isEmpty() )
    {
      contents.replace( index, "User_Alias\tSMB4KUSERS = "+QString( "%1" ).arg( users.join( "," ) ) );
    }
    else
    {
      int lines = end - begin + 1;
      int count = 0;
      
      while ( count != lines )
      {
        contents.removeAt( begin );
        count++;
      }
      
      if ( contents.last().trimmed().isEmpty() )
      {
        contents.removeLast();
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    return_value = 2;
  }
  
  return return_value;
}


int main ( int argc, char *argv[] )
{
  KAboutData aboutData( "smb4k_sudowriter",
                        "smb4k",
                        ki18n( "smb4k_sudowriter" ),
                        VERSION,
                        ki18n( description ),
                        KAboutData::License_GPL_V2,
                        ki18n( authors ),
                        KLocalizedString(),
                        "http://smb4k.berlios.de",
                        "smb4k-bugs@lists.berlios.de" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add( "adduser <user>",
               ki18n( "Add an user to the sudoers file" ),
               0 );
  options.add( "removeuser <user>",
               ki18n( "Remove an user from the sudoers file" ),
               0 );

  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false /* no GUI */ );
  
  // Create the lock file.
  QFile lock_file;
  QFile sudoers;
  
  // Get the command line arguments.
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // Check that everything is OK.
  QStringList adduser, removeuser;
  QStringList adduser_list    = args->getOptionList( "adduser" );
  QStringList removeuser_list = args->getOptionList( "removeuser" );
  
  for ( int i = 0; i < adduser_list.size(); ++i )
  {
    if ( adduser_list.at( i ).contains( "," ) )
    {
      adduser += adduser_list.at( i ).split( ",", QString::SkipEmptyParts );
    }
    else
    {
      adduser << adduser_list.at( i );
    }
  }
  
  for ( int i = 0; i < removeuser_list.size(); ++i )
  {
    if ( removeuser_list.at( i ).contains( "," ) )
    {
      removeuser += removeuser_list.at( i ).split( ",", QString::SkipEmptyParts );
    }
    else
    {
      removeuser << removeuser_list.at( i );
    }
  }
  
  // Show usage if no argument was provided.
  if ( adduser.isEmpty() && removeuser.isEmpty() )
  {
    KCmdLineArgs::usageError( i18n( "No valid arguments provided." ) );
    exit( EXIT_FAILURE );
  }
  else
  {
    switch( createLockFile( &lock_file ) )
    {
      case 1:
      {
        cerr << argv[0] << ": " << I18N_NOOP( "The lock file could not be created." ) << endl;
        cerr << argv[0] << ": " << lock_file.errorString().toUtf8().data() << endl;
        cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
        exit( EXIT_FAILURE );
        break;
      }
      case 2:
      {
        cerr << argv[0] << ": " << I18N_NOOP( "The sudoers file is currently being edited." ) << endl;
        cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
        exit( EXIT_FAILURE );
        break;
      }
      default:
      {
        break;
      }
    }
  }

  // Check that the given users are all valid.
  if ( !checkUsers( adduser ) || !checkUsers( removeuser ) )
  {
    cerr << argv[0] << ": " << I18N_NOOP( "An invalid username has been provided." ) << endl;
    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    removeLockFile( &lock_file );
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }
  
  // Find the sudoers file.
  int return_value;
  mode_t permissions;
  
  switch ( (return_value = findSudoers( sudoers, permissions )) )
  {
    case 0:
    {
      // OK
      break;
    }
    case 1:
    {
      // Not found
      cerr << argv[0] << ": " << I18N_NOOP( "The sudoers file was not found." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
    case 2:
    {
      // File irregular
      cerr << argv[0] << ": " << I18N_NOOP( "The sudoers file is irregular." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
    case 3:   
    {
      // File not accessible
      cerr << argv[0] << ": " << I18N_NOOP( "Cannot access sudoers file." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
    default:
    {
      cerr << argv[0] << ": " << strerror( return_value ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;      
    }
  }
  
  // Read the sudoers file.
  QStringList contents;
  
  switch ( ( return_value = readSudoers( sudoers, permissions, contents )) )
  {
    case 0:
    {
      break;
    }
    case 1:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Could not read from sudoers file." ) << endl;
      cerr << argv[0] << ": " << sudoers.errorString().toUtf8().data();
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
    default:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Could not set file permissions for sudoers file." ) << endl;
      cerr << argv[0] << ": " << strerror( return_value ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
  }
  
  // Modify the contents of the sudoers file.
  switch ( (return_value = modifyContents( contents, adduser, removeuser )) )
  {
    case 0:
    {
      break;
    }
    case 1:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Could not find utility programs (smb4k_kill, smb4k_umount, smb4k_mount)." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
    case 2:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Wrong format of Smb4K section in sudoers file detected." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Please fix this manually." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
    default:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Could not compile Smb4K section for sudoers file." ) << endl;
      cerr << argv[0] << ": " << strerror( return_value ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
  }
  
  // Write the sudoers file.
  switch ( ( return_value = writeSudoers( sudoers, permissions, contents )) )
  {
    case 0:
    {
      break;
    }
    case 1:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Could not write to sudoers file." ) << endl;
      cerr << argv[0] << ": " << sudoers.errorString().toUtf8().data();
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
    default:
    {
      cerr << argv[0] << ": " << I18N_NOOP( "Could not set file permissions for sudoers file." ) << endl;
      cerr << argv[0] << ": " << strerror( return_value ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile( &lock_file );
      exit( EXIT_FAILURE );
      break;
    }
  }
  
  contents.clear();
  args->clear();
  
  removeLockFile( &lock_file );

  app.exit( EXIT_SUCCESS );
}
