/***************************************************************************
    smb4k_umount  -  This is the unmount utility of Smb4K.
                             -------------------
    begin                : Sa Sep 25 2004
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

#ifndef __FreeBSD__
#include <sys/statfs.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif

#ifdef __linux__
#include <sys/utsname.h>
#endif

#include <locale.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define SMB4K_UMOUNT_VERSION "0.16"


void info()
{
  cout << "This is smb4k_umount (version " << SMB4K_UMOUNT_VERSION << "), the unmount utility of Smb4K" << endl;
  cout << "Copyright (C) 2004-2010, Alexander Reinholdt" << endl;
  cout << endl;
  cout << "Usage:" << endl;
#ifndef __FreeBSD__
  cout << "  smb4k_umount {options} {mountpoint}" << endl;
#else
  cout << "  smb4k_umount {mountpoint}" << endl;
#endif
  cout << "  smb4k_umount --help" << endl;
  cout << "  smb4k_umount --version" << endl;
  cout << endl;
  cout << "Arguments:" << endl;
#ifdef __linux__
  cout << "  {options}" << endl;
  cout << "    -l\t\tPerform a lazy unmount. See the manual page of umount for" << endl;
  cout << "\t\tmore information. You need kernel version 2.4.11 or later." << endl;
#endif
  cout << endl;
  cout << "  {mountpoint}\tThe path where the share is mounted." << endl;
  cout << endl;
  cout << "  --help\tDisplay this help screen and exit." << endl;
  cout << "  --version\tDisplay the version information and exit." << endl;
  cout << endl;
}


void version()
{
  cout << "Version " << SMB4K_UMOUNT_VERSION << endl;
}


bool find_program( const char *name, char *path, bool verbose )
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
    if ( verbose )
    {
      cerr << "smb4k_umount: Could not find " << name << " binary" << endl;
    }
    return false;
  }

  int len = strlen( file.c_str() ) + 1;
  strncpy( path, file.c_str(), len );
  path[len-1] = '\0';

  return true;
}


bool check_filesystem( const char *path )
{
  bool ok = false;

  struct statfs filesystem;

  if ( statfs( path, &filesystem ) == -1 )
  {
    int err_code = errno;

    if ( err_code != EIO && err_code != EACCES )
    {
      // ok is still FALSE
      cerr << "smb4k_umount: " << strerror( err_code ) << endl;
    }
    else
    {
      ok = true;  // Bypass the check below, because it would yield ok == FALSE
                  // and we want to be able to unmount broken shares as well.
    }

    return ok;
  }

#ifndef __FreeBSD__
  // First entry is for CIFS, the second for SMBFS.
  if ( (uint)filesystem.f_type == 0xFF534D42 /* CIFS */ ||
       (uint)filesystem.f_type == 0x517B /* SMBFS */ )
  {
    ok = true;
  }
#else
  if ( !strncmp( filesystem.f_fstypename, "smbfs", strlen( "smbfs" ) ) )
  {
    ok = true;
  }
#endif
  else
  {
    // ok is still FALSE.
    cerr << "smb4k_umount: File system not supported" << endl;
  }

  return ok;
}


int main( int argc, char *argv[], char *envp[] )
{
  // First of all, set the locale
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
  char *mountpoint = NULL;
#ifdef __linux__
  bool lazy_unmount = false;
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
#ifdef __linux__
    c = getopt_long( argc, argv, "l", long_options, &option_index );
#else
    c = getopt_long( argc, argv, "", long_options, &option_index );
#endif

    if ( c == -1 )
    {
      break;
    }

    switch ( c )
    {
      case 0:
      {
        int len = strlen( long_options[option_index].name ) + 1;
        char opt[len];
        opt[0] = '\0';
        (void) strncpy( opt, long_options[option_index].name, len );
        opt[len-1] = '\0';

        if ( !strncmp( opt, "help", len ) )
        {
          info();
          exit( EXIT_SUCCESS );
        }
        else if ( !strncmp( opt, "version", len ) )
        {
          version();
          exit( EXIT_SUCCESS );
        }
        else
        {
          break;
        }

        break;
      }
#ifdef __linux__
      case 'l':
      {
        // Initiate a lazy unmount. The umount binary
        // will complain, if '-l' is not supported.
        lazy_unmount = true;
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
  if ( !find_program( "umount.cifs", path, false ) )
  {
    // Fall back to umount binary if we cannot find umount.cifs.
    // This is needed when dealing with CIFS utils from Samba 4.0.
    if ( !find_program( "umount", path, true ) )
    {
      exit( EXIT_FAILURE );
    }
  }

  int len = strlen( path ) + 1;
  new_argv[index] = new char[len];
  new_argv[index][0] = '\0';
  (void) strncpy( new_argv[index], path, len );
  new_argv[index][len-1] = '\0';

  index++;

#ifdef __linux__
  // Lazy unmount?
  if ( lazy_unmount )
  {
    len = strlen( "-l" ) + 1;

    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], "-l", len );
    new_argv[index][len-1] = '\0';

    index++;
  }
#endif
#else
  // We do not need to care about the user mode and
  // we also need not to check for the file system,
  // since there is only one.
  if ( !find_program( "umount", path, true ) )
  {
    exit( EXIT_FAILURE );
  }

  int length = strlen( path ) + 1;
  new_argv[index] = new char[length];
  new_argv[index][0] = '\0';
  (void) strncpy( new_argv[index], path, length );
  new_argv[index][length-1] = '\0';

  index++;
#endif

  // Add the mount point:
  if ( optind < argc )
  {
    while ( optind < argc )
    {
      if ( !mountpoint )
      {
        if ( argv[optind][0] != '\057' )
        {
          cerr << "smb4k_umount: Argument " << optind << " is not a mount point" << endl;
          exit( EXIT_FAILURE );
        }
#ifndef __FreeBSD__
        if ( !check_filesystem( argv[optind] ) )
#else
        if ( !check_filesystem( argv[optind] ) )
#endif
        {
          // Error message is given by check_filesystem()
          exit( EXIT_FAILURE );
        }

        int len = strlen( argv[optind] ) + 1;

        mountpoint = new char[len];
        mountpoint[0] = '\0';
        (void) strncpy( mountpoint, argv[optind], len );
        mountpoint[len-1] = '\0';

        optind++;
      }
      else
      {
        break;
      }
    }
  }

  if ( !mountpoint )
  {
    cerr << "smb4k_umount: No mount point was specified" << endl;
    exit( EXIT_FAILURE );
  }
  else
  {
    int len = strlen( mountpoint ) + 1;
    new_argv[index] = new char[len];
    new_argv[index][0] = '\0';
    (void) strncpy( new_argv[index], mountpoint, len );
    new_argv[index][len-1] = '\0';

    index++;
  }


  if ( index >= new_argc )
  {
    cerr << "smb4k_umount: There are too many arguments" << endl;
    exit( EXIT_FAILURE );
  }

  // Terminate new_argv:
  new_argv[index] = NULL;

  // Execute command:
  if ( execve( new_argv[0], new_argv, envp ) == -1 )
  {
    int err = errno;
    cerr << "smb4k_umount: " <<  strerror( err ) << endl;
    exit( EXIT_FAILURE );
  }

  return EXIT_SUCCESS;
}

