/***************************************************************************
    smb4k_sudowriter  -  This program writes to the sudoers file. It
    belongs to the utility programs of Smb4K.
                             -------------------
    begin                : Di Jul 29 2008
    copyright            : (C) 2008 by Alexander Reinholdt
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
#include <cstdlib>

using namespace std;

static QFile lock_file;

static const char description[] =
  I18N_NOOP( "This program writes to the sudoers file." );

static const char authors[] =
  I18N_NOOP( "(c) 2008, Alexander Reinholdt" );

int createLockFile()
{
  // Determine the directory where to write the lock file. First, try
  // /var/lock and than /var/tmp. If that does not work either, fall
  // back to /tmp.
  QList<QByteArray> dirs;
  dirs << "/var/lock";
  dirs << "/var/tmp";
  dirs << "/tmp";

  struct stat buf;

  for ( int i = 0; i < dirs.size(); ++i )
  {
    // First check if the directory is available and writable
    if ( lstat( dirs.at( i ), &buf ) == -1 )
    {
      int error_number = errno;

      if ( error_number != EACCES && error_number != ENOENT )
      {
        return error_number;
      }
    }
    else
    {
      // Continue
    }

    // Get the ids of the groups the user is in and check if
    // on of them matches buf.st_gid.
    KUser user( geteuid() );
    QList<KUserGroup> gids = user.groups();
    gid_t sup_gid = 65534; // set this to gid 'nobody' for initialization
    bool found_gid = false;

    for ( int j = 0; j < gids.size(); ++j )
    {
      if ( gids.at( j ).gid() == buf.st_gid )
      {
        sup_gid = gids.at( j ).gid();
        found_gid = false;

        break;
      }
      else
      {
        continue;
      }
    }

    // Check whether we are stat'ing a directory and that the
    // user has read/write permissions.
    if ( S_ISDIR( buf.st_mode ) /* is directory */ &&
        ((buf.st_uid == getuid() && (buf.st_mode & 00600) == (S_IWUSR | S_IRUSR)) /* user */ ||
        (found_gid && buf.st_gid == sup_gid && (buf.st_mode & 00060) == (S_IWGRP | S_IRGRP)) /* group */ ||
        ((buf.st_mode & 00006) == (S_IWOTH | S_IROTH)) /* others */) )
    {
      lock_file.setFileName( dirs.at( i )+"/smb4k.lock" );

      break;
    }
    else
    {
      continue;
    }
  }

  if ( !lock_file.exists() )
  {
    if ( lock_file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QTextStream ts( &lock_file );
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

      lock_file.close();
    }
    else
    {
      return 1;
    }
  }
  else
  {
    return 2;
  }

  return 0;
}

void removeLockFile()
{
  // Just remove the lock file. No further checking is needed.
  if ( lock_file.exists() )
  {
    lock_file.remove();
  }
  else
  {
    // Do nothing
  }
}

const QByteArray findFile( const QString &filename )
{
  QStringList paths;
  paths << "/etc";
  paths << "/usr/local/etc";

  QString canonical_path;

  for ( int i = 0; i < paths.size(); ++i )
  {
    QDir::setCurrent( paths.at( i ) );

    if ( QFile::exists( filename ) )
    {
      canonical_path = QDir::current().canonicalPath()+QDir::separator()+filename;

      break;
    }
    else
    {
      continue;
    }
  }

  return canonical_path.toLocal8Bit();
}

bool checkUsers( const QStringList &list )
{
  for ( int i = 0; i < list.size(); ++i )
  {
    if ( getpwnam( list.at( i ).toLocal8Bit() ) == NULL )
    {
      return false;
    }
    else
    {
      continue;
    }
  }

  return true;
}

int checkFile( const QByteArray &path )
{
  // Stat the file, so that we know that it is safe to
  // read from and write to it and whether we need to
  // ask for the super user's password:
  struct stat buf;

  if ( lstat( path, &buf ) == -1 )
  {
    return errno;
  }
  else
  {
    // Do nothing
  }

  // Get the ids of the groups the user is in and check if
  // on of them matches buf.st_gid.
  KUser user( geteuid() );
  QList<KUserGroup> gids = user.groups();
  gid_t sup_gid = 65534; // set this to gid 'nobody' for initialization
  bool found_gid = false;

  for ( int i = 0; i < gids.size(); ++i )
  {
    if ( gids.at( i ).gid() == buf.st_gid )
    {
      sup_gid = gids.at( i ).gid();
      found_gid = false;

      break;
    }
    else
    {
      continue;
    }
  }

  if ( !S_ISREG( buf.st_mode ) || S_ISFIFO( buf.st_mode ) || S_ISLNK( buf.st_mode ) )
  {
    return 1;
  }
  else
  {
    // Do nothing
  }

  // Check the access rights. We need to read the file.
  if ( buf.st_uid != geteuid() && !found_gid &&
       (buf.st_mode & 00004) != (S_IWOTH | S_IROTH) /* others */ )
  {
    return 2;
  }
  else
  {
    // Do nothing
  }

  return 0;
}

bool findUtilityPrograms()
{
  if ( KGlobal::dirs()->findResource( "exe", "smb4k_kill" ).isEmpty() ||
       KGlobal::dirs()->findResource( "exe", "smb4k_umount" ).isEmpty() ||
       KGlobal::dirs()->findResource( "exe", "smb4k_mount" ).isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  return true;
}

int main ( int argc, char *argv[] )
{
  KAboutData aboutData( "smb4k_sudowriter",
                        "smb4k",
                        ki18n( "smb4k_sudowriter" ),
                        "0.2",
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

  // Before doing anything else, create the lock file.
  int return_value = 0;

  if ( (return_value = createLockFile()) != 0 )
  {
    switch ( return_value )
    {
      case 1:
      {
        cerr << argv[0] << ": " << I18N_NOOP( "The lock file could not be created." ) << endl;
        break;
      }
      case 2:
      {
        cerr << argv[0] << ": " << I18N_NOOP( "Another user is currently editing the sudoers file." ) << endl;
        break;
      }
      default:
      {
        cerr << argv[0] << ": " << strerror( return_value ) << endl;
        break;
      }
    }

    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // Check that everything is OK.
  QStringList adduser    = args->getOptionList( "adduser" );
  QStringList removeuser = args->getOptionList( "removeuser" );

  // Throw an error if no argument was provided.
  if ( adduser.isEmpty() && removeuser.isEmpty() )
  {
    KCmdLineArgs::usageError( i18n( "No arguments given." ) );
    removeLockFile();
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }

  // Check that the given users are all valid.
  if ( !checkUsers( adduser ) || !checkUsers( removeuser ) )
  {
    cerr << argv[0] << ": " << I18N_NOOP( "An invalid user name has been provided." ) << endl;
    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    removeLockFile();
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }

  // Find the sudoers file.
  QByteArray path = findFile( "sudoers" );

  if ( path.isEmpty() )
  {
    cerr << argv[0] << ": " << I18N_NOOP( "The sudoers file was not found." ) << endl;
    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    removeLockFile();
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }

  // Check that the file is regular.
  if ( (return_value = checkFile( path )) != 0 )
  {
    switch ( return_value )
    {
      case 1:
      {
        cerr << argv[0] << ": " << I18N_NOOP( "The sudoers file is irregular." ) << endl;
        break;
      }
      case 2:
      {
        cerr << argv[0] << ": " << I18N_NOOP( "Cannot access sudoers file." ) << endl;
        break;
      }
      default:
      {
        cerr << argv[0] << ": " << strerror( return_value ) << endl;
        break;
      }
    }

    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    removeLockFile();
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }

  // Check that the utility programs can actually be found.
  if ( !findUtilityPrograms() )
  {
    cerr << argv[0] << ": " << I18N_NOOP( "One or more utility programs could not be found." ) << endl;
    cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
    removeLockFile();
    exit( EXIT_FAILURE );
  }
  else
  {
    // Do nothing
  }

  // Now work with the sudoers file.
  QFile file( path );

  // Save the original permissions for later.
  QFile::Permissions perms = file.permissions();

  // Temporarily give the *owner* the permission to
  // write to the file.
  file.setPermissions( QFile::WriteOwner | QFile::ReadOwner );

  QStringList contents;

  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QTextStream ts( &file );
    // Note: With Qt 4.3 this seems to be obsolete, we'll keep
    // it for now.
    ts.setCodec( QTextCodec::codecForLocale() );

    while ( !ts.atEnd() )
    {
      contents.append( ts.readLine( 0 ) );
    }

    file.close();
    file.setPermissions( perms );
  }
  else
  {
    file.setPermissions( perms );
    cerr << argv[0] << ": " << file.errorString().toLocal8Bit().data() << endl;
    removeLockFile();
    exit( EXIT_FAILURE );
  }

  // Find the beginning and the end of the entries in
  // the sudoers file:
  int begin = contents.indexOf( "# Entries for Smb4K users.", 0 );
  int end = contents.lastIndexOf( "# End of Smb4K user entries.", -1 );

  bool write = false;

  // Add user(s).
  if ( !adduser.isEmpty() )
  {
    if ( begin == -1 && end == -1 )
    {
      // Get the hostname.
      size_t hostnamelen = 255;
      char *hn = new char[hostnamelen];

      if ( gethostname( hn, hostnamelen ) == -1 )
      {
        int error_number = errno;
        cerr << argv[0] << ": " << strerror( error_number ) << endl;
        removeLockFile();
        exit( EXIT_FAILURE );
      }
      else
      {
        // Do nothing
      }

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
      contents.append( "User_Alias\tSMB4KUSERS = "+QString( "%1" ).arg( adduser.join( "," ) ) );
      contents.append( "Defaults:SMB4KUSERS\tenv_keep += \"PASSWD USER\"" );
      contents.append( "SMB4KUSERS\t"+hostname+" = NOPASSWD: "
                       +KGlobal::dirs()->findResource( "exe", "smb4k_kill" ) );
      contents.append( "SMB4KUSERS\t"+hostname+" = NOPASSWD: "
                       +KGlobal::dirs()->findResource( "exe", "smb4k_umount" ) );
      contents.append( "SMB4KUSERS\t"+hostname+" = NOPASSWD: "
                       +KGlobal::dirs()->findResource( "exe", "smb4k_mount" ) );
      contents.append( "# End of Smb4K user entries." );

      write = true;
    }
    else if ( begin != -1 && end != -1 )
    {
      for ( int i = begin; i != end; ++i )
      {
        if ( contents.at( i ).startsWith( "User_Alias\tSMB4KUSERS" ) )
        {
          for ( int j = 0; j < adduser.size(); ++j )
          {
            if ( !contents.at( i ).contains( adduser.at( j ) ) )
            {
              contents[i].append( ","+adduser.at( j ) );
              continue;
            }
            else
            {
              continue;
            }
          }

          write = true;
          break;
        }
        else
        {
          continue;
        }
      }
    }
    else
    {
      cerr << argv[0] << ": " << I18N_NOOP( "The Smb4K section does not conform with the required format." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile();
      exit( EXIT_FAILURE );
    }
  }
  else
  {
    // Do nothing
  }

  // Remove user(s).
  if ( !removeuser.isEmpty() )
  {
    if ( begin != -1 && end != -1 )
    {
      for ( int i = begin; i != end; ++i )
      {
        if ( contents.at( i ).startsWith( "User_Alias\tSMB4KUSERS" ) )
        {
          QString users = contents.at( i ).section( "=", 1, 1 ).trimmed();

          if ( !users.contains( "," ) )
          {
            // In this case, there is only one user in the list. Check if
            // it is the user who requested the removal:
            for ( int j = 0; j < removeuser.size(); ++j )
            {
              if ( QString::compare( users, removeuser.at( j ) ) == 0 )
              {
                // They are equal. Remove all entries:
                int k = begin;

                while ( k != end + 1 ) // We want to remove line 'end' as well.
                {
                  contents.removeAt( begin );

                  k++;
                }

                write = true;

                break;
              }
              else
              {
                // They are not equal: Do nothing.
                break;
              }
            }

            break;
          }
          else
          {
            // In this case there is more than one user in the list.
            // Remove the user who requested the removal:
            QStringList list = users.split( ",", QString::SkipEmptyParts );
            int index = 0;

            for ( int j = 0; j < removeuser.size(); ++j )
            {
              index = list.indexOf( removeuser.at( j ), 0 );

              if ( index != -1 )
              {
                list.removeAt( index );
                contents[i].replace( users, list.join( "," ) );

                write = true;

                continue;
              }
              else
              {
                continue;
              }
            }

            break;
          }

          break;
        }
        else
        {
          continue;
        }
      }
    }
    else if ( begin == -1 && end == -1 )
    {
      // Do nothing
    }
    else
    {
      cerr << argv[0] << ": " << I18N_NOOP( "The Smb4K section does not conform with the required format." ) << endl;
      cerr << argv[0] << ": " << I18N_NOOP( "Aborting." ) << endl;
      removeLockFile();
      exit( EXIT_FAILURE );
    }
  }
  else
  {
    // Nothing to do.
  }

  if ( write )
  {
    // Temporarily give the *owner* the permission to
    // write to the file.
    file.setPermissions( QFile::WriteOwner | QFile::ReadOwner );

    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, we'll keep
      // it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      ts << contents.join( "\n" ) << endl;

      file.close();
      file.setPermissions( perms );
    }
    else
    {
      file.setPermissions( perms );
      cerr << argv[0] << ": " << file.errorString().toLocal8Bit().data() << endl;
      removeLockFile();
      exit( EXIT_FAILURE );
    }
  }
  else
  {
    // No modifications are needed.
  }

  // File permissions were fixed above.

  args->clear();

  removeLockFile();

  app.exit( EXIT_SUCCESS );
}
