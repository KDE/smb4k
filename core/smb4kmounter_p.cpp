/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2012 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kmounter_p.h"
#include "smb4ksettings.h"
#include "smb4knotification.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"

// Qt includes
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

// KDE includes
#include <kdiskfreespaceinfo.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmountpoint.h>
#include <kshell.h>

using namespace Smb4KGlobal;


Smb4KMountJob::Smb4KMountJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_processed( 0 )
{
  setCapabilities( KJob::Killable );
}


Smb4KMountJob::~Smb4KMountJob()
{
  while ( !m_shares.isEmpty() )
  {
    delete m_shares.takeFirst();
  }

  while ( !m_auth_errors.isEmpty() )
  {
    delete m_auth_errors.takeFirst();
  }

  while ( !m_retries.isEmpty() )
  {
    delete m_retries.takeFirst();
  }
}


void Smb4KMountJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartMount() ) );
}


void Smb4KMountJob::setupMount( Smb4KShare *share, QWidget *parent )
{
  Q_ASSERT( share );
  m_shares << new Smb4KShare( *share );
  m_parent_widget = parent;
}


void Smb4KMountJob::setupMount( const QList<Smb4KShare*> &shares, QWidget *parent )
{
  QListIterator<Smb4KShare *> it( shares );

  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();
    Q_ASSERT( share );
    m_shares << new Smb4KShare( *share );
  }

  m_parent_widget = parent;
}


bool Smb4KMountJob::createMountAction( Smb4KShare *share, Action *action )
{
  Q_ASSERT( share );
  Q_ASSERT( action );

  // Find the mount program.
  QString mount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  for ( int i = 0; i < paths.size(); ++i )
  {
#ifndef Q_OS_FREEBSD
    mount = KGlobal::dirs()->findExe( "mount.cifs", paths.at( i ) );
#else
    mount = KGlobal::dirs()->findExe( "mount_smbfs", paths.at( i ) );
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
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( mount );
    return false;
  }
  else
  {
    // Do nothing
  }

  // Assemble the mount point and create it.
  QString mountpoint;
  mountpoint += Smb4KSettings::mountPrefix().path();
  mountpoint += QDir::separator();
  mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->hostName().toLower() : share->hostName());
  mountpoint += QDir::separator();

  if ( !share->isHomesShare() )
  {
    mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->shareName().toLower() : share->shareName());
  }
  else
  {
    mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->login().toLower() : share->login());
  }

  QDir dir( QDir::cleanPath( mountpoint ) );

  if ( !dir.mkpath( dir.path() ) )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->mkdirFailed( dir );
    return false;
  }
  else
  {
    share->setPath( dir.path() );
  }

  // Set the file system
#ifndef Q_OS_FREEBSD
  share->setFileSystem( Smb4KShare::CIFS );
#else
  share->setFileSystem( Smb4KShare::SMBFS );
#endif

  QStringList arguments;
  QMap<QString, QString> global_options = globalSambaOptions();
  Smb4KCustomOptions *options  = Smb4KCustomOptionsManager::self()->findOptions( share );

#ifndef Q_OS_FREEBSD
  // Set the port before passing the full UNC.
  if ( options )
  {
    share->setPort( options->fileSystemPort() != Smb4KSettings::remoteFileSystemPort() ?
                    options->fileSystemPort() : Smb4KSettings::remoteFileSystemPort() );
  }
  else
  {
    share->setPort( Smb4KSettings::remoteFileSystemPort() );
  }

  QStringList args_list;

  // Workgroup
  if ( !share->workgroupName().trimmed().isEmpty() )
  {
    args_list << QString( "domain=%1" ).arg( KShell::quoteArg( share->workgroupName() ) );
  }
  else
  {
    // Do nothing
  }

  // Host IP
  if ( !share->hostIP().trimmed().isEmpty() )
  {
    args_list << QString( "ip=%1" ).arg( share->hostIP() );
  }
  else
  {
    // Do nothing
  }

  // User
  if ( !share->login().isEmpty() )
  {
    args_list << QString( "user=%1" ).arg( share->login() );
  }
  else
  {
    args_list << "guest";
  }

  // Client's and server's NetBIOS name
  // According to the manual page, this is only needed when port 139
  // is used. So, we only pass the NetBIOS name in that case.
  if ( Smb4KSettings::remoteFileSystemPort() == 139 || (options && options->fileSystemPort() == 139) )
  {
    // The client's NetBIOS name.
    if ( !Smb4KSettings::netBIOSName().isEmpty() )
    {
      args_list << QString( "netbiosname=%1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
    }
    else
    {
      if ( !global_options["netbios name"].isEmpty() )
      {
        args_list << QString( "netbiosname=%1" ).arg( KShell::quoteArg( global_options["netbios name"] ) );
      }
      else
      {
        // Do nothing
      }
    }

    // The server's NetBIOS name.
    args_list << QString( "servern=%1" ).arg( KShell::quoteArg( share->hostName() ) );
  }
  else
  {
    // Do nothing
  }

  // UID
  args_list << QString( "uid=%1" ).arg( options ? options->uid() : (uid_t)Smb4KSettings::userID().toInt() );

  // GID
  args_list << QString( "gid=%1" ).arg( options ? options->gid() : (gid_t)Smb4KSettings::groupID().toInt() );

  // Client character set
  switch ( Smb4KSettings::clientCharset() )
  {
    case Smb4KSettings::EnumClientCharset::default_charset:
    {
      if ( !global_options["unix charset"].isEmpty() )
      {
        args_list << QString( "iocharset=%1" ).arg( global_options["unix charset"].toLower() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      args_list << QString( "iocharset=%1" )
                   .arg( Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::clientCharset() ).label );
      break;
    }
  }

  // Port
  args_list << QString( "port=%1" )
               .arg( (options && options->fileSystemPort() != Smb4KSettings::remoteFileSystemPort()) ?
                     options->fileSystemPort() : Smb4KSettings::remoteFileSystemPort() );

  // Write access
  if ( options )
  {
    switch ( options->writeAccess() )
    {
      case Smb4KCustomOptions::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KCustomOptions::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            args_list << "rw";
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            args_list << "ro";
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
    }
  }
  else
  {
    switch ( Smb4KSettings::writeAccess() )
    {
      case Smb4KSettings::EnumWriteAccess::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KSettings::EnumWriteAccess::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        break;
       }
    }
  }

  // File mask
  if ( !Smb4KSettings::fileMask().isEmpty() )
  {
    args_list << QString( "file_mode=%1" ).arg( Smb4KSettings::fileMask() );
  }
  else
  {
    // Do nothing
  }

  // Directory mask
  if ( !Smb4KSettings::directoryMask().isEmpty() )
  {
    args_list << QString( "dir_mode=%1" ).arg( Smb4KSettings::directoryMask() );
  }
  else
  {
    // Do nothing
  }

  // Permission checks
  if ( Smb4KSettings::permissionChecks() )
  {
    args_list << "perm";
  }
  else
  {
    args_list << "noperm";
  }

  // Client controls IDs
  if ( Smb4KSettings::clientControlsIDs() )
  {
    args_list << "setuids";
  }
  else
  {
    args_list << "nosetuids";
  }

  // Server inode numbers
  if ( Smb4KSettings::serverInodeNumbers() )
  {
    args_list << "serverino";
  }
  else
  {
    args_list << "noserverino";
  }

  // Inode data caching
  if ( Smb4KSettings::noInodeDataCaching() )
  {
    args_list << "directio";
  }
  else
  {
    // Do nothing
  }

  // Translate reserved characters
  if ( Smb4KSettings::translateReservedChars() )
  {
    args_list << "mapchars";
  }
  else
  {
    args_list << "nomapchars";
  }

  // Locking
  if ( Smb4KSettings::noLocking() )
  {
    args_list << "nolock";
  }
  else
  {
    // Do nothing
  }

  // Security mode
  switch ( Smb4KSettings::securityMode() )
  {
    case Smb4KSettings::EnumSecurityMode::None:
    {
      args_list << "sec=none";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5:
    {
      args_list << "sec=krb5";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5i:
    {
      args_list << "sec=krb5i";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlm:
    {
      args_list << "sec=ntlm";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmi:
    {
      args_list << "sec=ntlmi";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2:
    {
      args_list << "sec=ntlmv2";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2i:
    {
      args_list << "sec=ntlmv2i";
      break;
    }
    default:
    {
      // Smb4KSettings::EnumSecurityMode::Default,
      break;
    }
  }

  // Global custom options provided by the user
  if ( !Smb4KSettings::customCIFSOptions().isEmpty() )
  {
    args_list += Smb4KSettings::customCIFSOptions().split( ',', QString::SkipEmptyParts );
  }
  else
  {
    // Do nothing
  }

  arguments << "-o";
  arguments << args_list.join( "," );
#else
  if ( options )
  {
    share->setPort( options->smbPort() != Smb4KSettings::remoteSMBPort() ?
                    options->smbPort() : Smb4KSettings::remoteSMBPort() );
  }
  else
  {
    share->setPort( Smb4KSettings::remoteSMBPort() );
  }

  // Workgroup
  if ( !share->workgroupName().isEmpty() )
  {
    arguments << "-W";
    arguments << KShell::quoteArg( share->workgroupName() );
  }
  else
  {
    // Do nothing
  }

  // Host IP
  if ( !share->hostIP().isEmpty() )
  {
    arguments << "-I";
    arguments << share->hostIP();
  }
  else
  {
    // Do nothing
  }

  // Do not ask for a password. Use ~/.nsmbrc instead.
  arguments << "-N";

  // UID
  if ( options )
  {
    arguments << "-u";
    arguments << QString( "%1" ).arg( options->uid() );
  }
  else
  {
    arguments << "-u";
    arguments << QString( "%1" ).arg( (K_UID)Smb4KSettings::userID().toInt() );
  }

  // GID
  if ( options )
  {
    arguments << "-g";
    arguments << QString( "%1" ).arg( options->gid() );
  }
  else
  {
    arguments << "-g";
    arguments << QString( "%1" ).arg( (K_GID)Smb4KSettings::groupID().toInt() );
  }

  // Client character set and server codepage
  QString charset, codepage;

  switch ( Smb4KSettings::clientCharset() )
  {
    case Smb4KSettings::EnumClientCharset::default_charset:
    {
      charset = global_options["unix charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      charset = Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::clientCharset() ).label;
      break;
    }
  }

  switch ( Smb4KSettings::serverCodepage() )
  {
    case Smb4KSettings::EnumServerCodepage::default_codepage:
    {
      codepage = global_options["dos charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      codepage = Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::serverCodepage() ).label;
      break;
    }
  }

  if ( !charset.isEmpty() && !codepage.isEmpty() )
  {
    arguments << "-E";
    arguments << QString( "%1:%2" ).arg( charset, codepage );
  }
  else
  {
    // Do nothing
  }

  // File mask
  if ( !Smb4KSettings::fileMask().isEmpty() )
  {
    arguments << "-f";
    arguments << Smb4KSettings::fileMask();
  }
  else
  {
    // Do nothing
  }

  // Directory mask
  if ( !Smb4KSettings::directoryMask().isEmpty() )
  {
    arguments << "-d";
    arguments << Smb4KSettings::directoryMask();
  }
  else
  {
    // Do nothing
  }

  if ( !share->login().isEmpty() )
  {
    arguments << "-U";
    arguments << share->login();
  }
  else
  {
    arguments << "-N";
  }
#endif

  // Compile the mount command.
  QStringList mount_command;
  mount_command << mount;
#ifndef Q_OS_FREEBSD
  if ( !share->isHomesShare() )
  {
    mount_command << share->unc();
  }
  else
  {
    mount_command << share->homeUNC();
  }

  mount_command << share->canonicalPath();
  mount_command += arguments;
#else
  mount_command += arguments;

  if ( !share->isHomesShare() )
  {
    mount_command << share->unc();
  }
  else
  {
    mount_command << share->homeUNC();
  }

  mount_command << share->canonicalPath();
#endif

  action->setName( "net.sourceforge.smb4k.mounthelper.mount" );
  action->setHelperID( "net.sourceforge.smb4k.mounthelper" );
  action->addArgument( "command", mount_command );
  action->addArgument( "home_dir", QDir::homePath() );

  if ( !share->isHomesShare() )
  {
    action->addArgument( "url", share->url() );
  }
  else
  {
    action->addArgument( "url", share->homeURL() );
  }

  action->addArgument( "workgroup", share->workgroupName() );
  action->addArgument( "comment", share->comment() );
  action->addArgument( "ip", share->hostIP() );
  action->addArgument( "mountpoint", share->canonicalPath() );

  return true;
}


bool Smb4KMountJob::doKill()
{
  Action( "net.sourceforge.smb4k.mounthelper.mount" ).stop();
  return KJob::doKill();
}


void Smb4KMountJob::slotStartMount()
{
  QList<Action> actions;
  QMutableListIterator<Smb4KShare *> it( m_shares );

  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();
    Action mountAction;

    if ( createMountAction( share, &mountAction ) )
    {
      connect( mountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
               this, SLOT( slotActionFinished( ActionReply ) ) );

      actions << mountAction;
    }
    else
    {
      // Do nothing
    }
  }

  if ( !actions.isEmpty() )
  {
    emit aboutToStart( m_shares );
    Action::executeActions( actions, NULL, "net.sourceforge.smb4k.mounthelper" );
  }
  else
  {
    // No aboutToStart() signal should have been emitted,
    // so there is no need to emit a finished() signal.
    emitResult();
  }
}


void Smb4KMountJob::slotActionFinished( ActionReply reply )
{
  // Count the processed actions.
  m_processed++;

  if ( reply.succeeded() )
  {
    QMutableListIterator<Smb4KShare *> it( m_shares );

    while( it.hasNext() )
    {
      Smb4KShare *share = it.next();

      // Check if the mount process reported an error
      QString stderr( reply.data()["stderr"].toString() );

      if ( QString::compare( share->canonicalPath(), reply.data()["mountpoint"].toString() ) == 0 && !stderr.isEmpty() )
      {
#ifndef Q_OS_FREEBSD
        if ( stderr.contains( "mount error 13", Qt::CaseSensitive ) || stderr.contains( "mount error(13)" )
            /* authentication error */ )
        {
          m_auth_errors << new Smb4KShare( *share );
          emit authError( this );
        }
        else if ( (stderr.contains( "mount error 6" ) || stderr.contains( "mount error(6)" )) /* bad share name */ &&
                  share->shareName().contains( "_", Qt::CaseSensitive ) )
        {
          QString share_name = share->shareName();
          share->setShareName( share_name.replace( '_', ' ' ) );
          m_retries << new Smb4KShare( *share );
          emit retry( this );
        }
        else if ( stderr.contains( "mount error 101" ) || stderr.contains( "mount error(101)" ) /* network unreachable */ )
        {
          qDebug() << "Network unreachable ..." << endl;
        }
#else
        if ( stderr.contains( "Authentication error" ) )
        {
          m_auth_errors << new Smb4KShare( *share );
          emit authError( this );
        }
#endif
        else
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->mountingFailed( share, stderr );
        }
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // The auth action failed. Report this.
    Smb4KNotification *notification = new Smb4KNotification();

    if ( reply.type() == ActionReply::KAuthError )
    {
      notification->actionFailed( reply.errorCode() );
    }
    else
    {
      notification->actionFailed();
    }
  }

  if ( m_processed == m_shares.size() )
  {
    // Give the operating system some time to process the mounts
    // before we invoke KMountPoint::currentMountPoints().
    QTimer::singleShot( 100, this, SLOT( slotFinishJob() ) );
  }
}


void Smb4KMountJob::slotFinishJob()
{
  QMutableListIterator<Smb4KShare *> it( m_shares );
  Smb4KShare *share = NULL;

  while ( it.hasNext() )
  {
    share = it.next();

    // Check which share has been mounted and emit the mounted() signal
    // if appropriate.
    if ( !share->isMounted() )
    {
      KMountPoint::List mount_points = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );

      for ( int i = 0; i < mount_points.size(); ++i )
      {
        if ( QString::compare( mount_points.at( i )->mountPoint(), share->path() ) == 0 ||
             QString::compare( mount_points.at( i )->mountPoint(), share->canonicalPath() ) == 0 )
        {
          share->setIsMounted( true );
          emit mounted( share );
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
      // Do nothing
    }
  }

  // Emit result() signal and tell the job to finish.
  emitResult();
  emit finished( m_shares );
}


Smb4KUnmountJob::Smb4KUnmountJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_processed( 0 )
{
  setCapabilities( KJob::Killable );
}


Smb4KUnmountJob::~Smb4KUnmountJob()
{
  while ( !m_shares.isEmpty() )
  {
    delete m_shares.takeFirst();
  }
}


void Smb4KUnmountJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartUnmount() ) );
}


void Smb4KUnmountJob::synchronousStart()
{
  m_started = true;
  slotStartUnmount();
}


void Smb4KUnmountJob::setupUnmount( Smb4KShare *share, bool force, bool silent, QWidget *parent )
{
  Q_ASSERT( share );
  m_shares << new Smb4KShare( *share );
  m_force = force;
  m_silent = silent;
  m_parent_widget = parent;
}


void Smb4KUnmountJob::setupUnmount( const QList<Smb4KShare *> &shares, bool force, bool silent, QWidget* parent )
{
  QListIterator<Smb4KShare *> it( shares );

  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();
    Q_ASSERT( share );
    m_shares << new Smb4KShare( *share );
  }

  m_force = force;
  m_silent = silent;
  m_parent_widget = parent;
}


bool Smb4KUnmountJob::createUnmountAction( Smb4KShare *share, bool force, bool silent, Action *action )
{
  Q_ASSERT( share );
  Q_ASSERT( action );

  // Find the umount program.
  QString umount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  for ( int i = 0; i < paths.size(); ++i )
  {
    umount = KGlobal::dirs()->findExe( "umount", paths.at( i ) );

    if ( !umount.isEmpty() )
    {
      break;
    }
    else
    {
      continue;
    }
  }

  if ( umount.isEmpty() && !silent )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( umount );
    return false;
  }
  else
  {
    // Do nothing
  }

  // Compile the command
  QStringList unmount_command;
  unmount_command << umount;

#ifdef Q_OS_LINUX
  if ( force )
  {
    unmount_command << "-l"; // lazy unmount
  }
  else
  {
    // Do nothing
  }
#endif

  unmount_command << share->canonicalPath();

  action->setName( "net.sourceforge.smb4k.mounthelper.unmount" );
  action->setHelperID( "net.sourceforge.smb4k.mounthelper" );
  action->addArgument( "command", unmount_command );

  // Now add everything we need.
  action->addArgument( "url", share->url() );
  action->addArgument( "mountpoint", share->canonicalPath() );

  return true;
}


bool Smb4KUnmountJob::doKill()
{
  Action( "net.sourceforge.smb4k.mounthelper.unmount" ).stop();
  return KJob::doKill();
}


void Smb4KUnmountJob::slotStartUnmount()
{
  QList<Action> actions;
  QMutableListIterator<Smb4KShare *> it( m_shares );

  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();
    Action unmountAction;

    if ( createUnmountAction( share, m_force, m_silent, &unmountAction ) )
    {
      connect( unmountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
               this, SLOT( slotActionFinished( ActionReply ) ) );

      actions << unmountAction;
    }
    else
    {
      // Do nothing
    }
  }

  if ( !actions.isEmpty() )
  {
    emit aboutToStart( m_shares );
    Action::executeActions( actions, NULL, "net.sourceforge.smb4k.mounthelper" );
  }
  else
  {
    // No aboutToStart() signal should have been emitted,
    // so there is no need to emit a finished() signal.
    emitResult();
  }
}


void Smb4KUnmountJob::slotActionFinished( ActionReply reply )
{
  m_processed++;

  if ( reply.succeeded() )
  {
    // Although theoretically the ActionReply object should
    // return the right url, it seems that this is not the
    // case with bulk operations. So, we just check which of
    // the shares has been mounted and emit the mounted signal.
    QMutableListIterator<Smb4KShare *> it( m_shares );

    while( it.hasNext() )
    {
      Smb4KShare *share = it.next();

      // Check if the unmount process reported an error
      QString stderr( reply.data()["stderr"].toString() );

      if ( QString::compare( share->canonicalPath(), reply.data()["mountpoint"].toString() ) == 0 && !stderr.isEmpty() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->unmountingFailed( share, stderr );
      }
      else
      {
        // Do nothing
      }
    }

  }
  else
  {
    // The auth action failed. Report this.
    Smb4KNotification *notification = new Smb4KNotification();

    if ( reply.type() == ActionReply::KAuthError )
    {
      notification->actionFailed( reply.errorCode() );
    }
    else
    {
      notification->actionFailed();
    }
  }

  if ( m_processed == m_shares.size() )
  {
    // Give the operating system some time to process the unmounts
    // before we invoke KMountPoint::currentMountPoints(). It seems
    // that we need at least 500 ms, so that even slow systems have
    // the opportunity to unregister the mounts.
    QTimer::singleShot( 500, this, SLOT( slotFinishJob() ) );
  }
}


void Smb4KUnmountJob::slotFinishJob()
{
  QMutableListIterator<Smb4KShare *> it( m_shares );
  Smb4KShare *share = NULL;

  while ( it.hasNext() )
  {
    share = it.next();

    // Check if the share has been unmounted and emit the unmounted()
    // signal if appropriate.
    if ( share->isMounted() )
    {
      KMountPoint::List mount_points = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );
      bool mountpoint_found = false;

      for ( int i = 0; i < mount_points.size(); ++i )
      {
        if ( QString::compare( mount_points.at( i )->mountPoint(), share->path() ) == 0 ||
             QString::compare( mount_points.at( i )->mountPoint(), share->canonicalPath() ) == 0 )
        {
          mountpoint_found = true;
          break;
        }
        else
        {
          continue;
        }
      }

      if ( !mountpoint_found )
      {
        share->setIsMounted( false );
        emit unmounted( share );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }
  }

  // Emit result() signal and tell the job to finish.
  emitResult();
  emit finished( m_shares );
}



Smb4KMountDialog::Smb4KMountDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ), m_share( share ), m_valid( true )
{
  setCaption( i18n( "Mount Share" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  setupView();

  connect( this, SIGNAL( okClicked() ), SLOT( slotOkClicked() ) );
  connect( this, SIGNAL( cancelClicked() ), SLOT( slotCancelClicked() ) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  restoreDialogSize( group );

  m_share_input->completionObject()->setItems( group.readEntry( "ShareNameCompletion", QStringList() ) );
  m_ip_input->completionObject()->setItems( group.readEntry( "IPAddressCompletion", QStringList() ) );
  m_workgroup_input->completionObject()->setItems( group.readEntry( "WorkgroupCompletion", QStringList() ) );
}


Smb4KMountDialog::~Smb4KMountDialog()
{
}


void Smb4KMountDialog::setupView()
{
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QVBoxLayout *layout = new QVBoxLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QWidget *description = new QWidget( main_widget );

  QHBoxLayout *desc_layout = new QHBoxLayout( description );
  desc_layout->setSpacing( 5 );
  desc_layout->setMargin( 0 );

  QLabel *pixmap = new QLabel( description );
  QPixmap mount_pix = KIcon( "view-form", KIconLoader::global(), QStringList( "emblem-mounted" ) ).pixmap( KIconLoader::SizeHuge );
  pixmap->setPixmap( mount_pix );
  pixmap->setAlignment( Qt::AlignBottom );

  QLabel *label = new QLabel( i18n( "Enter the location (UNC address) and optionally the IP address and "
                                    "workgroup to mount a share." ), description );
  label->setWordWrap( true );
  label->setAlignment( Qt::AlignBottom );

  desc_layout->addWidget( pixmap, 0 );
  desc_layout->addWidget( label, Qt::AlignBottom );

  QWidget *edit_widget = new QWidget( main_widget );

  QGridLayout *edit_layout = new QGridLayout( edit_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *shareLabel = new QLabel( i18n( "UNC Address:" ), edit_widget );
  m_share_input = new KLineEdit( edit_widget );
  m_share_input->setWhatsThis( i18n( "The Uniform Naming Convention (UNC) address "
    "describes the location of the share. It has the following syntax: "
    "//[USER@]HOST/SHARE. The username is optional." ) );
//   m_share_input->setToolTip( i18n( "The UNC address of the share" ) );
  m_share_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_share_input->setClearButtonShown( true );
  m_share_input->setMinimumWidth( 200 );
  m_share_input->setFocus();

  QLabel *addressLabel = new QLabel( i18n( "IP Address:" ), edit_widget );
  m_ip_input = new KLineEdit( edit_widget);
  m_ip_input->setWhatsThis( i18n( "The Internet Protocol (IP) address identifies the "
    "host in the network and indicates where it is. It has two valid formats, the one "
    "known as IP version 4 (e.g. 192.168.2.11) and the version 6 format "
    "(e.g. 2001:0db8:85a3:08d3:1319:8a2e:0370:7334)." ) );
//   m_ip_input->setToolTip( i18n( "The IP address of the host where the share is located" ) );
  m_ip_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_ip_input->setClearButtonShown( true );
  m_ip_input->setMinimumWidth( 200 );

  QLabel *workgroupLabel = new QLabel( i18n( "Workgroup:" ), edit_widget );
  m_workgroup_input = new KLineEdit( edit_widget );
  m_workgroup_input->setWhatsThis( i18n( "The workgroup or domain identifies the "
    "peer-to-peer computer network the host is located in." ) );
//   m_workgroup_input->setToolTip( i18n( "The workgroup where the host is located" ) );
  m_workgroup_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_workgroup_input->setClearButtonShown( true );
  m_workgroup_input->setMinimumWidth( 200 );

  edit_layout->addWidget( shareLabel, 0, 0, 0 );
  edit_layout->addWidget( m_share_input, 0, 1, 0 );
  edit_layout->addWidget( addressLabel, 1, 0, 0 );
  edit_layout->addWidget( m_ip_input, 1, 1, 0 );
  edit_layout->addWidget( workgroupLabel, 2, 0, 0 );
  edit_layout->addWidget( m_workgroup_input, 2, 1, 0 );

  m_bookmark = new QCheckBox( i18n( "Add this share to the bookmarks" ), main_widget );
  m_bookmark->setWhatsThis( i18n( "If you tick this checkbox, the share will be bookmarked "
    "and you can access it e.g. through the \"Bookmarks\" menu entry in the main window." ) );
//   m_bookmark->setToolTip( i18n( "Add this share to the bookmarks" ) );

  layout->addWidget( description, Qt::AlignBottom );
  layout->addWidget( edit_widget, 0 );
  layout->addWidget( m_bookmark, 0 );

  slotChangeInputValue( m_share_input->text() );

  // Connections
  connect( m_share_input,     SIGNAL( textChanged ( const QString & ) ) ,
           this,              SLOT( slotChangeInputValue( const QString & ) ) );

  connect( m_share_input,     SIGNAL( editingFinished() ),
           this,              SLOT( slotShareNameEntered() ) );

  connect( m_ip_input,        SIGNAL( editingFinished() ),
           this,              SLOT( slotIPEntered() ) );

  connect( m_workgroup_input, SIGNAL( editingFinished() ),
           this,              SLOT( slotWorkgroupEntered() ) );
}


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMountDialog::slotChangeInputValue( const QString& _test)
{
  enableButtonOk( !_test.isEmpty() );
}


void Smb4KMountDialog::slotOkClicked()
{
  if ( !m_share_input->text().trimmed().isEmpty() )
  {
    QUrl url;

    // Take care of Windows-like UNC addresses:
    if ( m_share_input->text().trimmed().startsWith( QLatin1String( "\\" ) ) )
    {
      QString unc = m_share_input->text();
      unc.replace( "\\", "/" );
      url = QUrl( unc );
    }
    else
    {
      url = QUrl( m_share_input->text().trimmed() );
    }

    url.setScheme( "smb" );

    if ( url.isValid() && !url.host().isEmpty() /* no invalid host name */ &&
         url.path().length() > 1 /* share name length */ && !url.path().endsWith( '/' ) )
    {
      m_share->setURL( url );
      m_share->setWorkgroupName( m_workgroup_input->text().trimmed() );
      m_share->setHostIP( m_ip_input->text().trimmed() );
    }
    else
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->invalidURLPassed();
      m_valid = false;
    }
  }
  else
  {
    // Do nothing
  }

  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
  group.writeEntry( "ShareNameCompletion", m_share_input->completionObject()->items() );
  group.writeEntry( "IPAddressCompletion", m_ip_input->completionObject()->items() );
  group.writeEntry( "WorkgroupCompletion", m_workgroup_input->completionObject()->items() );
}


void Smb4KMountDialog::slotCancelClicked()
{
  Smb4KMounter::self()->abort( m_share );
}


void Smb4KMountDialog::slotShareNameEntered()
{
  KCompletion *completion = m_share_input->completionObject();
  QUrl url( m_share_input->userText() );
  url.setScheme( "smb" );

  if ( url.isValid() && !url.isEmpty() )
  {
    completion->addItem( m_share_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotIPEntered()
{
  KCompletion *completion = m_ip_input->completionObject();

  if ( !m_ip_input->userText().isEmpty() )
  {
    completion->addItem( m_ip_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotWorkgroupEntered()
{
  KCompletion *completion = m_workgroup_input->completionObject();

  if ( !m_workgroup_input->userText().isEmpty() )
  {
    completion->addItem( m_workgroup_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


#include "smb4kmounter_p.moc"
