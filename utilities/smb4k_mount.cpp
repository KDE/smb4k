/***************************************************************************
    smb4k_mount  -  This is the mount utility for Smb4K.
                             -------------------
    begin                : Di Sep 21 2004
    copyright            : (C) 2004-2010 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <locale.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define SMB4K_MOUNT_VERSION "0.11"

void info()
{
  cout << "This is smb4k_mount (version " << SMB4K_MOUNT_VERSION << "), the mount utility of Smb4K" << endl;
  cout << "Copyright (C) 2004-2010, Alexander Reinholdt" << endl;
  cout << endl;
  cout << "Usage:" << endl;
  cout << "  smb4k_mount {options} {share} {mountpoint}" << endl;
  cout << "  smb4k_mount --help" << endl;
  cout << "  smb4k_mount --version" << endl;
  cout << endl;
  cout << "Arguments:" << endl;
#ifndef __FreeBSD__
  cout << "  {options}" << endl;
  cout << "    -o <list>\tWith this argument you define the options that should be passed" << endl;
  cout << "\t\tto the actual mount binary in a comma-separated list. See the" << endl;
  cout << "\t\tmanual page of mount.cifs and mount for more"<< endl;
  cout << "\t\tinformation." << endl;
#else
  cout << "  {options}\tAll options that can be passed to mount_smbfs. Please refer to" << endl;
  cout << "\t\tthe manual page of mount_smbfs to learn more." << endl;
#endif
  cout << endl;
  cout << "  {share}\tThe remote share that should be mounted." << endl;
  cout << endl;
  cout << "  {mountpoint}\tThe path where the share should be mounted to." << endl;
  cout << endl;
  cout << "  --help\tDisplay this help sceen and exit." << endl;
  cout << "  --version\tDisplay the version information and exit." << endl;
  cout << endl;
}


void version()
{
  cout << "Version " << SMB4K_MOUNT_VERSION << endl;
}


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
    cerr << "smb4k_mount: Could not find " << name << " binary" << endl;
    return false;
  }

  int len = strlen( file.c_str() ) + 1;

  (void) strncpy( path, file.c_str(), len );
  path[len-1] = '\0';

  return true;
}


int main( int argc, char *argv[], char *envp[] )
{
  // First of all, set the locale:
  (void) setlocale( LC_ALL, "" );

  if ( argc < 2 )
  {
    info();
    exit( EXIT_FAILURE );
  }

  int new_argc = argc + 1;
  char *new_argv[new_argc];
  int index = 0;
  char path[255];
  path[0] = '\0';
  bool have_share = false;
#ifndef __FreeBSD__
  char *mount_options = NULL;
#else
  char *charset = NULL;
  char *host = NULL;
  char *locale = NULL;
  char *access_rights = NULL;
  char *owner_attributes = NULL;
  char *retries = NULL;
  char *timeout = NULL;
  char *workgroup = NULL;
  char *case_opt = NULL;
  char *dir_mode = NULL;
  char *file_mode = NULL;
  char *gid = NULL;
  char *uid = NULL;
  char *options = NULL;
  bool no_password = false;
  char *user = NULL;
#endif

  // Get the options that were passed:
  int c;

  while ( 1 )
  {
    int option_index = 0;

    static struct option long_options[] =
    {
      { "help", 0, 0, 0 },
      { "version", 0, 0, 0 },
      { 0, 0, 0, 0 }
    };

#ifndef __FreeBSD__
    c = getopt_long( argc, argv, "o:", long_options, &option_index );
#else
    c = getopt_long( argc, argv, "E:I:L:M:NO:R:T:U:W:c:d:f:g:n:u:", long_options, &option_index );
#endif

    if ( c == -1 )
    {
      break;
    }

    switch ( c )
    {
      case 0:
      {
        // Get the length of the option string, create a
        // new string and copy the option into it:
        int len = strlen( long_options[option_index].name ) + 1;
        char opt[len];
        opt[0] = '\0';
        (void) strncpy( opt, long_options[option_index].name, len );
        opt[len-1] = '\0';

        // Now check which option has been provided:
        if ( !strncmp( opt, "help", len ) )
        {
          info();
          // That's it. Exit here.
          exit( EXIT_SUCCESS );
        }
        else if ( !strncmp( opt, "version", len ) )
        {
          version();
          // That's it. Exit here.
          exit( EXIT_SUCCESS );
        }
        else
        {
          break;
        }

        break;
      }
#ifndef __FreeBSD__
      case 'o':
      {
        int len = strlen( optarg ) + 1;

        if ( !mount_options )
        {
          mount_options = new char[len];
          mount_options[0] = '\0';
          (void) strncpy( mount_options, optarg, len );
          mount_options[len-1] = '\0';
        }

        break;
      }
#else
      case 'E':
      {
        if ( !charset )
        {
          int len = strlen( optarg ) + 1;
          charset = new char[len];
          charset[0] = '\0';
          (void) strncpy( charset, optarg, len );
          charset[len-1] = '\0';
        }

        break;
      }
      case 'I':
      {
        if ( !host )
        {
          int len = strlen( optarg ) + 1;
          host = new char[len];
          host[0] = '\0';
          (void) strncpy( host, optarg, len );
          host[len-1] = '\0';
        }

        break;
      }
      case 'L':
      {
        if ( !locale )
        {
          int len = strlen( optarg ) + 1;
          locale = new char[len];
          locale[0] = '\0';
          (void) strncpy( locale, optarg, len );
          locale[len-1] = '\0';
        }

        break;
      }
      case 'M':
      {
        if ( !access_rights )
        {
          int len = strlen( optarg ) + 1;
          access_rights = new char[len];
          access_rights[0] = '\0';
          (void) strncpy( access_rights, optarg, len );
          access_rights[len-1] = '\0';
        }

        break;
      }
      case 'N':
      {
        no_password = true;
        break;
      }
      case 'O':
      {
        if ( !owner_attributes )
        {
          int len = strlen( optarg ) + 1;
          owner_attributes = new char[len];
          owner_attributes[0] = '\0';
          (void) strncpy( owner_attributes, optarg, len );
          owner_attributes[len-1] = '\0';
        }

        break;
      }
      case 'R':
      {
        if ( !retries )
        {
          int len = strlen( optarg ) + 1;
          retries = new char[len];
          retries[0] = '\0';
          (void) strncpy( retries, optarg, len );
          retries[len-1] = '\0';
        }

        break;
      }
      case 'T':
      {
        if ( !timeout )
        {
          int len = strlen( optarg ) + 1;
          timeout = new char[len];
          timeout[0] = '\0';
          (void) strncpy( timeout, optarg, len );
          timeout[len-1] = '\0';
        }

        break;
      }
      case 'U':
      {
        if ( !user )
        {
          int len = strlen( optarg ) + 1;
          user = new char[len];
          user[0] = '\0';
          (void) strncpy( user, optarg, len );
          user[len-1] = '\0';
        }

        break;
      }
      case 'W':
      {
        if ( !workgroup )
        {
          int len = strlen( optarg ) + 1;
          workgroup = new char[len];
          workgroup[0] = '\0';
          (void) strncpy( workgroup, optarg, len );
          workgroup[len-1] = '\0';
        }

        break;
      }
      case 'c':
      {
        if ( !case_opt )
        {
          int len = strlen( optarg ) + 1;
          case_opt = new char[len];
          case_opt[0] = '\0';
          (void) strncpy( case_opt, optarg, len );
          case_opt[len-1] = '\0';
        }

        break;
      }
      case 'd':
      {
        if ( !dir_mode )
        {
          int len = strlen( optarg ) + 1;
          dir_mode = new char[len];
          dir_mode[0] = '\0';
          (void) strncpy( dir_mode, optarg, len );
          dir_mode[len-1] = '\0';
        }

        break;
      }
      case 'f':
      {
        if ( !file_mode )
        {
          int len = strlen( optarg ) + 1;
          file_mode = new char[len];
          file_mode[0] = '\0';
          (void) strncpy( file_mode, optarg, len );
          file_mode[len-1] = '\0';
        }

        break;
      }
      case 'g':
      {
        if ( !gid )
        {
          int len = strlen( optarg ) + 1;
          gid = new char[len];
          gid[0] = '\0';
          (void) strncpy( gid, optarg, len );
          gid[len-1] = '\0';
        }

        break;
      }
      case 'n':
      {
        if ( options )
        {
          int len = strlen( optarg ) + 1;
          options = new char[len];
          options[0] = '\0';
          (void) strncpy( options, optarg, len );
          options[len-1] = '\0';
        }

        break;
      }
      case 'u':
      {
        if ( !uid )
        {
          int len = strlen( optarg ) + 1;
          uid = new char[len];
          uid[0] = '\0';
          (void) strncpy( uid, optarg, len );
          uid[len-1] = '\0';
        }

        break;
      }
#endif
      case '?':
      {
        // Abort the program if an unknown option
        // is encountered:
        exit( EXIT_FAILURE );
      }
      default:
      {
        break;
      }
    }
  }

#ifndef __FreeBSD__
  if( !find_program( "mount.cifs", path ) )
  {
    exit( EXIT_FAILURE );
  }

  int len = strlen( path ) + 1;

  new_argv[index] = new char[len];
  new_argv[index][0] = '\0';
  (void) strncpy( new_argv[index], path, len );
  new_argv[index][len-1] = '\0';

  index++;

  // Add the service name and the mount point.
  if ( optind < argc )
  {
    while ( optind < argc )
    {
      // Check for the share:
      if ( (argv[optind][0] == '\057' && argv[optind][1] == '\057') /* slash */ ||
           (argv[optind][0] == '\134' && argv[optind][1] == '\134') /* back slash */ )
      {
        have_share = true;
      }

      len = strlen( argv[optind] ) + 1;

      new_argv[index] = new char[len];
      new_argv[index][0] = '\0';
      (void) strncpy( new_argv[index], argv[optind], len );
      new_argv[index][len-1] = '\0';

      optind++;
      index++;
    }
  }

  // Pass the arguments that were provided with the -o
  // option.
  // NOTE: mount.cifs wants the -o argument to be the last
  // one in the command string. So, do not change the order.
  if ( mount_options )
  {
    len = strlen( "-o" ) + 1;

    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-o", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( mount_options ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], mount_options, len );
    new_argv[index][len-1] = '\0';

    index++;
  }
#else
  // We do not need to probe for the user mode, because we
  // only have mount_smbfs available. Also, we do not have
  // to check the file system. There is only one possible...
  if ( !find_program( "mount_smbfs", path ) )
  {
    exit( EXIT_FAILURE );
  }

  int len = strlen( path ) + 1;

  new_argv[index] = new char[len];
  new_argv[index][0] = '\0';
  (void) strncpy( new_argv[index], path, len );
  new_argv[index][len-1] = '\0';

  index++;

  // Add the arguments:
  if ( charset )
  {
    len = strlen( "-E" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-E", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( charset ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], charset, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( host )
  {
    len = strlen( "-I" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-I", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( host ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], host, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( locale )
  {
    len = strlen( "-L" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-L", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( locale ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], locale, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( access_rights )
  {
    len = strlen( "-M" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index],"-M" , len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( access_rights ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], access_rights, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( no_password )
  {
    len = strlen( "-N" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-N", len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( owner_attributes )
  {
    len = strlen( "-O" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-O", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( owner_attributes ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], owner_attributes, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( retries )
  {
    len = strlen( "-R" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-R", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( retries ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], retries, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( timeout )
  {
    len = strlen( "-I" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index],"-I" , len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( timeout ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], timeout, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( workgroup )
  {
    len = strlen( "-W" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-W", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( workgroup ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], workgroup, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( case_opt )
  {
    len = strlen( "-c" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-c", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( case_opt ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], case_opt, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( dir_mode )
  {
    len = strlen( "-d" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-d", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( dir_mode ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], dir_mode, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( file_mode )
  {
    len = strlen( "-f" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-f", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( file_mode ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], file_mode, len );
    new_argv[index][len-1] = '\0';

    index++;

  }

  if ( gid )
  {
    len = strlen( "-g" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-g", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( gid ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], gid, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( uid )
  {
    len = strlen( "-u" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-u", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( uid ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], uid, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  if ( options )
  {
    len = strlen( "-n" ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-n", len );
    new_argv[index][len-1] = '\0';

    index++;

    len = strlen( options ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], options, len );
    new_argv[index][len-1] = '\0';

    index++;
  }

  // Add the service name and the mount point.
  if ( optind < argc )
  {
    while ( optind < argc )
    {
      // Check for the share:
      if ( (argv[optind][0] == '\057' && argv[optind][1] == '\057') /* slash */ ||
           (argv[optind][0] == '\134' && argv[optind][1] == '\134') /* back slash */ )
      {
        have_share = true;
      }

      len = strlen( argv[optind] ) + 1;

      new_argv[index] = new char[len];
      new_argv[index][0] = '\0';
      (void) strncpy( new_argv[index], argv[optind], len );
      new_argv[index][len-1] = '\0';

      optind++;
      index++;
    }
  }
#endif

  if ( !have_share )
  {
    cerr << "smb4k_mount: No share was specified" << endl;
    exit( EXIT_FAILURE );
  }

  if ( index >= new_argc )
  {
    cerr << "smb4k_mount: There are too many arguments" << endl;
    exit( EXIT_SUCCESS );
  }

  // Terminate new_argv:
  new_argv[index] = NULL;

  // Execute command:
  if ( execve( new_argv[0], new_argv, envp ) == -1 )
  {
    int err = errno;
    cerr << "smb4k_mount: " << strerror( err ) << endl;
    exit( EXIT_FAILURE );
  }

  return EXIT_SUCCESS;
}
