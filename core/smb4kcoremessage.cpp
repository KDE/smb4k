/***************************************************************************
    smb4kcoremessage  -  This class provides messages for use with the
    core classes of Smb4K.
                             -------------------
    begin                : Sa Mär 8 2008
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
#include <QDesktopWidget>

// KDE includes
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>

// application specific includes
#include "smb4kcoremessage.h"


void Smb4KCoreMessage::error( int code, const QString &text, const QString &details )
{
  QWidget *parent = 0;

  if ( kapp )
  {
    if ( kapp->activeWindow() )
    {
      parent = kapp->activeWindow();
    }
    else
    {
      parent = kapp->desktop();
    }
  }
  else
  {
    // Do nothing
  }

  switch ( code )
  {
    case ERROR_UNMOUNTING_NOT_ALLOWED:
    {
      KMessageBox::error( parent, i18n( "<qt>You are not allowed to unmount this share. It is owned by another user.</qt>" ) );

      break;
    }
//     case ERROR_MOUNTING_SHARE:
//     {
//       if ( details.trimmed().isEmpty() )
//       {
//         KMessageBox::error( parent, i18n( "<qt><p>The share %1 could not be mounted.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>", text ) );
//       }
//       else {
//         KMessageBox::detailedError( parent, i18n( "<qt><p>The share %1 could not be mounted.</p><p>Read the error message under \"Details\" to find out more.</p></qt>", text ), details );
//       }
// 
//       break;
//     }
    case ERROR_UNMOUNTING_SHARE:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>The share %1 could not be unmounted.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>", text ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>The share %1 could not be unmounted.</p><p>Read the error message under \"Details\" to find out more.</p></qt>", text ), details );
      }

      break;
    }
    case ERROR_FILE_NOT_FOUND:
    {
      KMessageBox::error( parent, i18n( "<qt>The file \"%1\" could not be found.</qt>", text ) );

      break;
    }
    case ERROR_GETTING_HOSTNAME:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>The name of your computer could not be determined by using the gethostname() system call.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>" ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>The name of your computer could not be determined by using the gethostname() system call.</p><p>Read the error message under \"Details\" to find out more.</p></qt>" ), details );
      }

      break;
    }
    case ERROR_MISSING_PROGRAMS:
    {
      KMessageBox::error( parent, i18n( "<qt><p>Either your PATH environment variable is not set properly or there are the following programs missing on your system:</p><p>%1</p><p>Please correct this and restart Smb4K.</p></qt>", text ) );

      break;
    }
    case ERROR_MKDIR_FAILED:
    {
      KMessageBox::error( parent, i18n( "<qt>The directory \"%1\" could not be created.</qt>", text ) );

      break;
    }
    case ERROR_FEATURE_NOT_ENABLED:
    {
      KMessageBox::error( parent, i18n( "<qt>This feature has not been enabled.</qt>" ) );

      break;
    }
    case ERROR_BOOKMARK_PRINTER:
    {
      KMessageBox::error( parent, i18n( "<qt>Printers cannot be bookmarked.</qt>" ) );

      break;
    }
    case ERROR_SUDOWRITER:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>An error occurred while writing to the sudoers file.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>" ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>An error occurred while writing to the sudoers file.</p><p>Read the error message under \"Details\" to find out more.</p></qt>" ), details );
      }

      break;
    }
    case ERROR_COMMAND_NOT_FOUND:
    {
      KMessageBox::error( parent, i18n( "<qt>The command \"%1\" could not be found.</qt>", text ) );

      break;
    }
    case ERROR_PRINTING:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>The file \"%1\" could not be printed.</p></p>Detailed information cannot be provided because there was no error message.</p></qt>", text ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>The file \"%1\" could not be printed.</p><p>Read the error message under \"Details\" to find out more.</p></qt>", text ), details );
      }

      break;
    }
    case ERROR_CREATING_TEMP_FILE:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>The temporary file could not be created.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>" ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>The temporary file could not be created.</p><p>Read the error message under \"Details\" to find out more.</p></qt>" ), details );
      }

      break;
    }
    case ERROR_SYNCHRONIZING:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt></p>The synchronization could not be finished successfully.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>" ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>The synchronization could not be finished successfully.</p><p>Read the error message under \"Details\" to find out more.</p></qt>" ), details );
      }

      break;
    }
    case ERROR_SEARCHING:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>The search could not be finished successfully.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>" ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>The search could not be finished successfully.</p><p>Read the error message under \"Details\" to find out more.</p></qt>" ), details );
      }

      break;
    }
    case ERROR_OPENING_FILE:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt>The file \"%1\" could not be opened.</qt>", text ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>The file \"%1\" could not be opened.</p><p>Read the error message under \"Details\" to find out more.</p></qt>", text ), details );
      }

      break;
    }
    case ERROR_XML_ERROR:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>An error occurred while parsing the XML file \"%1\".</p><p>Detailed information cannot be provided because there was no error message.</p></qt>", text ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>An error occurred while parsing the XML file \"%1\".</p><p>Read the error message under \"Details\" to find out more.</p></qt>", text ), details );
      }

      break;
    }
    case ERROR_UNKNOWN:
    default:
    {
      if ( details.trimmed().isEmpty() )
      {
        KMessageBox::error( parent, i18n( "<qt><p>An unknown error occurred.</p><p>Detailed information cannot be provided because there was no error message.</p></qt>" ) );
      }
      else
      {
        KMessageBox::detailedError( parent, i18n( "<qt><p>An unknown error occurred.</p><p>Read the error message under \"Details\" to find out more.</p></qt>" ), details );
      }

      break;
    }
  }
}


void Smb4KCoreMessage::processError( int code, QProcess::ProcessError error )
{
  QWidget *parent = 0;

  if ( kapp )
  {
    if ( kapp->activeWindow() )
    {
      parent = kapp->activeWindow();
    }
    else
    {
      parent = kapp->desktop();
    }
  }
  else
  {
    // Do nothing
  }

  switch( code )
  {
    case ERROR_PROCESS_ERROR:
    {
      switch ( error )
      {
        case QProcess::FailedToStart:
        {
          KMessageBox::error( parent, i18n( "<qt>The process failed to start (error code: %1).</qt>", error ) );

          break;
        }
        case QProcess::Crashed:
        {
          KMessageBox::error( parent, i18n( "<qt>The process crashed (error code: %1).</qt>", error ) );

          break;
        }
        case QProcess::Timedout:
        {
          KMessageBox::error( parent, i18n( "<qt>The process timed out (error code: %1).</qt>", error ) );

          break;
        }
        case QProcess::WriteError:
        {
          KMessageBox::error( parent, i18n( "<qt>Could not write to the process (error code: %1).</qt>", error ) );

          break;
        }
        case QProcess::ReadError:
        {
          KMessageBox::error( parent, i18n( "<qt>Could not read from the process (error code: %1).</qt>", error ) );

          break;
        }
        case QProcess::UnknownError:
        default:
        {
          KMessageBox::error( parent, i18n( "<qt>The process reported an unknown error.</qt>" ) );

          break;
        }
      }

      break;
    }
    case ERROR_PROCESS_EXIT:
    {
      KMessageBox::error( parent, i18n( "<qt>The process exited unexpectedly.</qt>" ) );

      break;
    }
    default:
    {
      break;
    }
  }
}


int Smb4KCoreMessage::warning( int code, const QString &/*text*/, const QString &/*details*/ )
{
  QWidget *parent = 0;

  if ( kapp )
  {
    if ( kapp->activeWindow() )
    {
      parent = kapp->activeWindow();
    }
    else
    {
      parent = kapp->desktop();
    }
  }
  else
  {
    // Do nothing
  }

  int result = 0;

  switch ( code )
  {
    default:
    {
      break;
    }
  }

  return result;
}

