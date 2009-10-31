/***************************************************************************
    smb4k_kill  -  This is the kill utility for Smb4K.
                             -------------------
    begin                : So Sep 26 2004
    copyright            : (C) 2004-2009 by Alexander Reinholdt
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

#define SMB4K_KILL_VERSION "0.7"

void info()
{
	cout << "This is smb4k_kill (version " << SMB4K_KILL_VERSION << "), the kill utility of Smb4K" << endl;
	cout << "Copyright (C) 2004-2007, Alexander Reinholdt" << endl;
	cout << endl;
	cout << "Usage:" << endl;
	cout << "  smb4k_kill {pid}" << endl;
	cout << "  smb4k_kill --help" << endl;
	cout << "  smb4k_kill --version" << endl;
	cout << endl;
	cout << "Arguments:" << endl;
	cout << "  {pid}\t\tThe ID of the process that should be terminated." << endl;
	cout << endl;
	cout << "  --help\tDisplay this help screen and exit." << endl;
	cout << "  --version\tDisplay the version information and exit." << endl;
	cout << endl;
}


void version()
{
	cout << "Version: " << SMB4K_KILL_VERSION << endl;
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
    cerr << "smb4k_kill: Could not find " << name << " binary" << endl;
    return false;
  }

  int len = strlen( file.c_str() ) + 1;

  (void) strncpy( path, file.c_str(), len );
  path[len-1] = '\0';

  return true;
}


int main( int argc, char *argv[], char *envp[] )
{
  // Set the locale:
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

  // Get the options that were passed.
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

    c = getopt_long( argc, argv, "", long_options, &option_index );

    if ( c == -1 )
    {
      break;
    }

    switch ( c )
    {
      case 0:
      {
        // Copy the option:
        int len = strlen( long_options[option_index].name ) + 1;
        char opt[len];
        opt[0] = '\0';
        (void) strncpy( opt, long_options[option_index].name, len );
        opt[len-1] = '\0';

        // Now check which option has been provided:
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

  if ( !find_program( "kill", path ) )
  {
    exit( EXIT_FAILURE );
  }

  int len = strlen( path ) + 1;

  new_argv[index] = new char[len];
  new_argv[index][0] = '\0';
  (void) strncpy( new_argv[index], path, len );
  new_argv[index][len-1] = '\0';

  index++;

  if ( optind < argc )
  {
    while ( optind < argc )
    {
      len = strlen( argv[optind] ) + 1;

      new_argv[index] = new char[len];
      new_argv[index][0] = '\0';
      (void) strncpy( new_argv[index], argv[optind], len );
      new_argv[index][len-1] = '\0';

      optind++;
      index++;
    }
  }

  if ( index >= new_argc )
  {
    cerr << "smb4k_kill: There are too many arguments" << endl;
    exit( EXIT_FAILURE );
  }

  // Terminate the new argv:
  new_argv[index] = NULL;

  // Execute command:
  if ( execve( new_argv[0], new_argv, envp ) == -1 )
  {
    int err = errno;
    cerr << "smb4k_kill: " << strerror( err ) << endl;
    exit( EXIT_FAILURE );
  }

  return EXIT_SUCCESS;
}
