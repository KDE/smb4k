/***************************************************************************
    smb4ksudowriterinterface  -  This class provides an interface to the
    smb4k_sudowriter utility program that writes entries to the sudoers
    file.
                             -------------------
    begin                : Sa Aug 2 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

// KDE includes
#include <kglobal.h>
#include <kshell.h>
#include <kstandarddirs.h>

// system includes
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

// application specific includes
#include <smb4ksudowriterinterface.h>
#include <smb4ksettings.h>
#include <smb4kprocess.h>
#include <smb4knotification.h>

class Smb4KSudoWriterInterfacePrivate
{
  public:
    /**
     * The Smb4KSudoWriterInterface instance
     */
    Smb4KSudoWriterInterface instance;
};

K_GLOBAL_STATIC( Smb4KSudoWriterInterfacePrivate, priv );


Smb4KSudoWriterInterface::Smb4KSudoWriterInterface()
: QObject()
{
}


Smb4KSudoWriterInterface::~Smb4KSudoWriterInterface()
{
}


Smb4KSudoWriterInterface *Smb4KSudoWriterInterface::self()
{
  return &priv->instance;
}


bool Smb4KSudoWriterInterface::addUser()
{
  if ( !Smb4KSettings::doNotModifySudoers() )
  {
    // Find smb4k_sudowriter program
    QString smb4k_sudowriter = KGlobal::dirs()->findResource( "exe", "smb4k_sudowriter" );

    if ( smb4k_sudowriter.isEmpty() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "smb4k_sudowriter" );
      return false;
    }
    else
    {
      // Do nothing
    }

    // Find kdesu program
    QString kdesu = KGlobal::dirs()->findResource( "exe", "kdesu" );

    if ( kdesu.isEmpty() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "kdesu" );
      return false;
    }
    else
    {
      // Do nothing
    }

    // Get the name of the current user.
    uid_t user_id = getuid();
    struct passwd *pw = getpwuid( user_id );
    QString user( pw->pw_name );

    QString command;
    command += kdesu;
    command += " -t -c ";
    command += KShell::quoteArg( smb4k_sudowriter+" --adduser="+user );

    Smb4KProcess p( Smb4KProcess::WriteSudoers, this );
    p.setShellCommand( command );
    p.setOutputChannelMode( KProcess::SeparateChannels );

    switch ( p.execute() )
    {
      case -2:
      case -1:
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( p.error() );
        return false;
        break;
      }
      default:
      {
        // smb4k_sudowriter won't generate any output if it successfully could
        // write to the sudoers file. So, just check stderr for output.

        QString stderr = QString::fromUtf8( p.readAllStandardError(), -1 ).trimmed();

        // Only report an error if smb4k_sudowriter complained.
        // Note: If kdesu is not installed or could not be found, Smb4K won't
        // start up, so we do not need to test for it here.
        if ( !stderr.isEmpty() && stderr.contains( "smb4k_sudowriter" ) )
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->sudowriterFailed( stderr );
          return false;
        }
        else
        {
          // Do nothing
        }

        break;
      }
    }
  }
  else
  {
    // Do nothing
  }

  return true;
}


bool Smb4KSudoWriterInterface::removeUser()
{
  if ( !Smb4KSettings::doNotModifySudoers() )
  {
    // Find smb4k_sudowriter program
    QString smb4k_sudowriter = KGlobal::dirs()->findResource( "exe", "smb4k_sudowriter" );

    if ( smb4k_sudowriter.isEmpty() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "smb4k_sudowriter" );
      return false;
    }
    else
    {
      // Do nothing
    }

    // Find kdesu program
    QString kdesu = KGlobal::dirs()->findResource( "exe", "kdesu" );

    if ( kdesu.isEmpty() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "kdesu" );
      return false;
    }
    else
    {
      // Do nothing
    }

    // Get the name of the current user.
    uid_t user_id = getuid();
    struct passwd *pw = getpwuid( user_id );
    QString user( pw->pw_name );

    QString command;
    command += kdesu;
    command += " -t -c ";
    command += KShell::quoteArg( smb4k_sudowriter+" --removeuser="+user );

    Smb4KProcess p( Smb4KProcess::WriteSudoers, this );
    p.setShellCommand( command );
    p.setOutputChannelMode( KProcess::SeparateChannels );

    switch ( p.execute() )
    {
      case -2:
      case -1:
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( p.error() );
        return false;
      }
      default:
      {
        // smb4k_sudowriter won't generate any output if it successfully could
        // write to the sudoers file. So, just check stderr for output.

        QString stderr = QString::fromUtf8( p.readAllStandardError(), -1 ).trimmed();

        // Only report an error if smb4k_sudowriter complained.
        // Note: If kdesu is not installed or could not be found, Smb4K won't
        // start up, so we do not need to test for it here.
        if ( !stderr.isEmpty() && stderr.contains( "smb4k_sudowriter" ) )
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->sudowriterFailed( stderr );
          return false;
        }
        else
        {
          // Do nothing
        }

        break;
      }
    }
  }
  else
  {
    // Do nothing
  }

  return true;
}

#include "smb4ksudowriterinterface.moc"
