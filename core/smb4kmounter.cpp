/***************************************************************************
    smb4kmounter.cpp  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2009 by Alexander Reinholdt
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
#include <QApplication>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QDesktopWidget>
#ifdef __FreeBSD__
#include <QFileInfo>
#endif

// KDE includes
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kshell.h>
#include <kstandarddirs.h>

// system includes
#ifdef __FreeBSD__
#include <pwd.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#else
#include <stdio.h>
#include <mntent.h>
#endif

// Application specific includes
#include <smb4kmounter.h>
#include <smb4kauthinfo.h>
#include <smb4ksambaoptionsinfo.h>
#include <smb4kcoremessage.h>
#include <smb4kglobal.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4kdefs.h>
#include <smb4khomesshareshandler.h>
#include <smb4kmounter_p.h>
#include <smb4kwalletmanager.h>
#include <smb4kprocess.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KMounterPrivate, priv );



Smb4KMounter::Smb4KMounter() : QObject()
{
  m_working = false;
  m_timer_id = -1;
  m_timeout = 0;

  connect( kapp,    SIGNAL( aboutToQuit() ),
           this,    SLOT( slotAboutToQuit() ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( buttonPressed( Smb4KSolidInterface::ButtonType ) ),
           this, SLOT( slotHardwareButtonPressed( Smb4KSolidInterface::ButtonType ) ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( wokeUp() ),
           this, SLOT( slotComputerWokeUp() ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( networkStatusChanged( Smb4KSolidInterface::ConnectionStatus ) ),
           this, SLOT( slotNetworkStatusChanged( Smb4KSolidInterface::ConnectionStatus ) ) );
}


Smb4KMounter::~Smb4KMounter()
{
}


Smb4KMounter *Smb4KMounter::self()
{
  return &priv->instance;
}

void Smb4KMounter::init()
{
  m_timeout = Smb4KSettings::checkInterval();
  m_timer_id = startTimer( m_timeout );

  import();

  if ( Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Connected ||
       Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Unknown )
  {
    priv->setHardwareReason( false );
    triggerRemounts();
  }
  else
  {
    // Do nothing and wait until the network becomes available.
  }
}


void Smb4KMounter::abort( Smb4KShare *share )
{
  Q_ASSERT( share );

  Smb4KProcess *p = findChild<Smb4KProcess *>( share->unc( QUrl::None ) );

  if ( p && (p->state() == KProcess::Running || p->state() == KProcess::Starting) )
  {
    if ( Smb4KSettings::alwaysUseSuperUser() ||
         (Smb4KSettings::useForceUnmount() && p->type() == Smb4KProcess::Unmount) )
    {
      // Find smb4k_kill program
      QString smb4k_kill = KGlobal::dirs()->findResource( "exe", "smb4k_kill" );

      if ( smb4k_kill.isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smb4k_kill" );
        return;
      }
      else
      {
        // Do nothing
      }

      // Find sudo program
      QString sudo = KStandardDirs::findExe( "sudo" );

      if ( sudo.isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sudo" );
        return;
      }
      else
      {
        // Do nothing
      }

      Smb4KProcess kill_proc( Smb4KProcess::Kill, this );
      kill_proc.setShellCommand( sudo+" "+smb4k_kill+" "+QString( p->pid() ) );
      kill_proc.setOutputChannelMode( KProcess::MergedChannels );
      kill_proc.start();
      kill_proc.waitForFinished( -1 );

      // Tell the process that it was just killed/aborted.
      if ( p )
      {
        p->abort();
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      p->abort();
    }
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KMounter::isAborted( Smb4KShare *share )
{
  Q_ASSERT( share );

  Smb4KProcess *p = findChild<Smb4KProcess *>( share->unc( QUrl::None ) );
  return (p && p->isAborted());
}


void Smb4KMounter::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QList<Smb4KProcess *> processes = findChildren<Smb4KProcess *>();

    for ( int i = 0; i < processes.size(); ++i )
    {
      if ( processes.at( i )->state() == KProcess::Running || processes.at( i )->state() == KProcess::Starting )
      {
        processes.at( i )->abort();
        continue;
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // priv has already been deleted
  }
}


bool Smb4KMounter::isRunning( Smb4KShare *share )
{
  Q_ASSERT( share );

  Smb4KProcess *p = findChild<Smb4KProcess *>( share->unc( QUrl::None ) );
  return (p && p->state() == KProcess::Running);
}


void Smb4KMounter::triggerRemounts()
{
  if ( Smb4KSettings::remountShares() || priv->hardwareReason() )
  {
    QList<Smb4KSambaOptionsInfo *> list = Smb4KSambaOptionsHandler::self()->sharesToRemount();

    for ( int i = 0; i < list.size(); ++i )
    {
      QList<Smb4KShare *> shares = findShareByUNC( list.at( i )->unc() );

      if ( !shares.isEmpty() )
      {
        bool mount = true;

        for ( int j = 0; j < shares.size(); ++j )
        {
          if ( !shares.at( j )->isForeign() )
          {
            mount = false;

            break;
          }
          else
          {
            continue;
          }
        }

        if ( mount )
        {
          // First of all initialize the wallet manager.
          Smb4KWalletManager::self()->init( 0 );

          // Mount the share.
          Smb4KShare share( list.at( i )->unc() );
          share.setWorkgroupName( list.at( i )->workgroupName() );
          share.setHostIP( list.at( i )->ip() );

          mountShare( &share );

          // Remove the remount flag if the share was successfully
          // mounted.
          QList<Smb4KShare *> mounted_shares = findShareByUNC( share.unc() );

          for ( int i = 0; i < mounted_shares.size(); ++i )
          {
            if ( !mounted_shares.at( i )->isForeign() )
            {
              Smb4KSambaOptionsHandler::self()->removeRemount( mounted_shares.at( i ) );
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
      else
      {
        // First of all initialize the wallet manager.
        Smb4KWalletManager::self()->init( 0 );

        // Mount the share.
        Smb4KShare share( list.at( i )->unc() );
        share.setWorkgroupName( list.at( i )->workgroupName() );
        share.setHostIP( list.at( i )->ip() );

        mountShare( &share );

        // Remove the remount flag if the share was successfully
        // mounted.
        QList<Smb4KShare *> mounted_shares = findShareByUNC( share.unc() );

        for ( int i = 0; i < mounted_shares.size(); ++i )
        {
          if ( !mounted_shares.at( i )->isForeign() )
          {
            Smb4KSambaOptionsHandler::self()->removeRemount( mounted_shares.at( i ) );
            break;
          }
          else
          {
            continue;
          }
        }
      }
    }

    // To be on the save side, we write the options to the hard disk now.
    Smb4KSambaOptionsHandler::self()->sync();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::import()
{
  QList<Smb4KShare *> shares;

#ifndef __FreeBSD__

  // Prefer reading from /proc/mounts, because it carries more information.
  // If /proc/mounts does not exist, fall back to using the getmntent() system
  // call.
  if ( QFile::exists( "/proc/mounts" ) )
  {
    QStringList contents;
    QFile file( "/proc/mounts" );

    // Read /proc/mounts.
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, but we'll
      // keep it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      QString line;

      // Since we are operating on a symlink, we need to read
      // the file this way. Using ts.atEnd() won't work, because
      // it would immediately return TRUE.
      while ( 1 )
      {
        line = ts.readLine( 0 );

        if ( !line.isNull() )
        {
          if ( line.contains( " cifs ", Qt::CaseSensitive ) )
          {
            contents.append( line );
            continue;
          }
          else
          {
            continue;
          }
        }
        else
        {
          break;
        }
      }

      file.close();
    }
    else
    {
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, file.fileName() );
      return;
    }

    // Process the entries.
    if ( !contents.isEmpty() )
    {
      for ( int i = 0; i < contents.size(); ++i )
      {
        // Get the UNC and the path.
        QString unc_and_path = contents.at( i ).section( " cifs ", 0, 0 ).trimmed();
        QString unc = unc_and_path.section( " ", 0, 0 ).trimmed().replace( "\\040", "\040" );
        QString path = unc_and_path.section( " ", 1, 1 ).trimmed().replace( "\\040", "\040" );

        Smb4KShare *share = findShareByPath( path.toUtf8() );

        if ( !share )
        {
          QString domain = contents.at( i ).section( "domain=", 1, 1 ).section( ",", 0, 0 ).trimmed();
          QString ip = contents.at( i ).section( "addr=", 1, 1 ).section( ",", 0, 0 ).trimmed();

          share = new Smb4KShare( unc );
          share->setPath( path );
          share->setWorkgroupName( domain );
          share->setFileSystem( Smb4KShare::CIFS );
          share->setHostIP( ip );
          share->setIsMounted( true );
        }
        else
        {
          // Do nothing
        }

        // Set the login if necessary.
        // FIXME: Maybe we can also move this into the if statement above...
        if ( !share->loginIsSet() )
        {
          QString login = contents.at( i ).section( "username=", 1, 1 ).section( ",", 0, 0 ).trimmed();
          share->setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'username=' entries
        }
        else
        {
          // Do nothing
        }

        // Try to get the IP address if it could not be discovered previously.
        Smb4KHost *host = findHost( share->hostName(), share->workgroupName() );

        if ( host )
        {
          if ( share->hostIP().isEmpty() || QString::compare( host->ip(), share->hostIP() ) != 0 )
          {
            share->setHostIP( host->ip() );
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

        // Check if the share is inaccessible and get the UID and GID,
        // if necessary.
        if ( !share->isInaccessible() )
        {
          check( share );
        }
        else
        {
          // Do nothing
        }

        // Check if the share is foreign.
        if ( (share->uid() == getuid() && share->gid() == getgid()) ||
             (!share->isInaccessible() &&
             (QString::fromUtf8( share->path() ).startsWith( Smb4KSettings::mountPrefix().path() ) ||
             QString::fromUtf8( share->canonicalPath() ).startsWith( QDir::homePath() ))) ||
             (!share->isInaccessible() &&
             (QString::fromUtf8( share->canonicalPath() ).startsWith( QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath() ) ||
             QString::fromUtf8( share->canonicalPath() ).startsWith( QDir::home().canonicalPath() ))) )
        {
          share->setForeign( false );
        }
        else
        {
          share->setForeign( true );
        }

        shares.append( new Smb4KShare( *share ) );
      }
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    FILE *file = setmntent( "/etc/mtab", "r" );
    struct mntent *entry = NULL;

    while ( (entry = getmntent( file )) != NULL )
    {
      if ( !strncmp( entry->mnt_type, "cifs", strlen( entry->mnt_type ) + 1 ) )
      {
        QString path( entry->mnt_dir );
        Smb4KShare *share = findShareByPath( path.toUtf8() );

        if ( !share )
        {
          QString unc( entry->mnt_fsname );

          share = new Smb4KShare( unc );
          share->setPath( path );
          share->setFileSystem( Smb4KShare::CIFS );
          share->setIsMounted( true );
        }
        else
        {
          // Do nothing
        }

        // Set login if necessary.
        // Note: It seems that user=USER might contain a wrong entry if
        // we mount with the guest option...
        char *user = NULL;

        if ( (user = hasmntopt( entry, "user" )) != NULL )
        {
          share->setLogin( QString::fromUtf8( user ).section( "=", 1, 1 ).trimmed() );
        }
        else
        {
          share->setLogin( "guest" );
        }

        // Try to get the IP address and workgroup/domain if they
        // could not be discovered previously.
        Smb4KHost *host = findHost( share->hostName(), share->workgroupName() /* is maybe empty */);

        if ( host )
        {
          if ( share->hostIP().isEmpty() || QString::compare( host->ip(), share->hostIP() ) != 0 )
          {
            share->setHostIP( host->ip() );
          }
          else
          {
            // Do nothing
          }

          if ( share->workgroupName().isEmpty() )
          {
            share->setWorkgroupName( host->workgroupName() );
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

        // Check if the share is inaccessible and get the UID and GID,
        // if necessary.
        if ( !share->isInaccessible() )
        {
          check( share );
        }
        else
        {
          // Do nothing
        }

        // Check if the share is foreign.
        if ( (share->uid() == getuid() && share->gid() == getgid()) ||
             (!share->isInaccessible() &&
             (QString::fromUtf8( share->path() ).startsWith( Smb4KSettings::mountPrefix().path() ) ||
             QString::fromUtf8( share->canonicalPath() ).startsWith( QDir::homePath() ))) ||
             (!share->isInaccessible() &&
             (QString::fromUtf8( share->canonicalPath() ).startsWith( QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath() ) ||
             QString::fromUtf8( share->canonicalPath() ).startsWith( QDir::home().canonicalPath() ))) )
        {
          share->setForeign( false );
        }
        else
        {
          share->setForeign( true );
        }

        shares.append( new Smb4KShare( *share ) );
      }
      else
      {
        continue;
      }
    }
  }

#else

  struct statfs *buf;
  int count = getmntinfo( &buf, 0 );

  if ( count == 0 )
  {
    int err_code = errno;
    Smb4KCoreMessage::error( ERROR_IMPORTING_SHARES, QString::null, strerror( err_code ) );
    return;
  }

  for ( int i = 0; i < count; ++i )
  {
    if ( !strcmp( buf[i].f_fstypename, "smbfs" ) )
    {
      QByteArray path( buf[i].f_mntonname );

      Smb4KShare *existing_share = findShareByPath( path );
      Smb4KShare *share = NULL;

      if ( existing_share )
      {
        share = new Smb4KShare( *existing_share );

        if ( share->fileSystem() != Smb4KShare::SMBFS )
        {
          QFileInfo info( QString( buf[i].f_mntonname )+"/." );

          share->setFileSystem( Smb4KShare::SMBFS );
          share->setUID( info.ownerId() );
          share->setGID( info.groupId() );
        }
        else
        {
          // Do nothing
        }

      }
      else
      {
        QString share_name( buf[i].f_mntfromname );

        QFileInfo info( QString( buf[i].f_mntonname )+"/." );

        share = new Smb4KShare( share_name );
        share->setPath( path );
        share->setFileSystem ( !strcmp( buf[i].f_fstypename, "smbfs" ) ?
                               Smb4KShare::SMBFS :
                               Smb4KShare::Unknown );
        share->setUID( info.ownerId() );
        share->setGID( info.groupId() );
        share->setIsMounted( true );
      }

      // Test if share is broken
      if ( (existing_share && !existing_share->isInaccessible()) || !existing_share )
      {
        check( share );
      }
      else
      {
        // Since new_share is a copy of existing_share, we do not need to do
        // anything here.
      }

      shares.append( share );
    }
  }

  // Apparently, under FreeBSD we do not need to delete
  // the pointer (see manual page).

#endif

  // Replace the current global list with the new one.
  replaceMountedSharesList( shares );

  emit updated();
}


bool Smb4KMounter::mountShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  // Find smb4k_mount program
  QString smb4k_mount = KGlobal::dirs()->findResource( "exe", "smb4k_mount" );

  if ( smb4k_mount.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smb4k_mount" );
    return false;
  }
  else
  {
    // Do nothing
  }

  // Find the sudo program
  QString sudo = KStandardDirs::findExe( "sudo" );

  // Check that sudo is installed in the case it is needed.
  if ( Smb4KSettings::alwaysUseSuperUser() && sudo.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sudo" );
    return false;
  }
  else
  {
    // Do nothing
  }

  // Copy the share to avoid problems with 'homes' shares.
  Smb4KShare internal_share( *share );

  if ( internal_share.isHomesShare() )
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

    if ( !Smb4KHomesSharesHandler::self()->specifyUser( &internal_share, parent ) )
    {
      return false;
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

  // Before doing anything else let's check that the
  // share has not already been mounted by the user:
  QList<Smb4KShare *> list = findShareByUNC( internal_share.unc() );

  for ( int i = 0; i != list.size(); ++i )
  {
    if ( !list.at( i )->isForeign() )
    {
      Smb4KShare *mounted_share = findShareByPath( list.at( i )->path() );
      emit mounted( mounted_share );
      return true;
    }
    else
    {
      continue;
    }
  }

  // Assemble the mount point and create it:
  QString path = Smb4KSettings::mountPrefix().path() +
                 QDir::separator() +
                 (Smb4KSettings::forceLowerCaseSubdirs() ?
                 internal_share.hostName().toLower() :
                 internal_share.hostName()) +
                 QDir::separator() +
                 (Smb4KSettings::forceLowerCaseSubdirs() ?
                 internal_share.shareName().toLower() :
                 internal_share.shareName());

  QDir dir( QDir::cleanPath( path ) );

  if ( !dir.mkpath( dir.path() ) )
  {
    Smb4KCoreMessage::error( ERROR_MKDIR_FAILED, dir.path() );
    return false;
  }
  else
  {
    internal_share.setPath( dir.path() );
  }

  // Get the authentication information.
  Smb4KAuthInfo authInfo( &internal_share );
  Smb4KWalletManager::self()->readAuthInfo( &authInfo );

  // Set the login and the file system for the share.
#ifndef __FreeBSD__
  internal_share.setFileSystem( Smb4KShare::CIFS );
#else
  internal_share.setFileSystem( Smb4KShare::SMBFS );
#endif
//   internal_share.setLogin( !authInfo.login().isEmpty() ? QString::fromUtf8( authInfo.login() ) : "guest" );
  internal_share.setLogin( QString::fromUtf8( authInfo.login() ) );

  // Compile the command
  QString command;

  if ( Smb4KSettings::alwaysUseSuperUser() )
  {
    command += sudo;
    command += " "+smb4k_mount;
  }
  else
  {
    command += smb4k_mount;
  }

#ifndef __FreeBSD__
  command += " -o";
  command += " ";
  command += !internal_share.workgroupName().trimmed().isEmpty() ?
             QString( "domain=%1" ).arg( KShell::quoteArg( internal_share.workgroupName() ) ) :
             "";
  command += !internal_share.hostIP().trimmed().isEmpty() ?
             QString( ",ip=%1" ).arg( internal_share.hostIP() ) :
             "";
  command += !authInfo.login().isEmpty() ?
             QString( ",user=%1" ).arg( QString::fromUtf8( authInfo.login() ) ) :
             ",guest";
  command += Smb4KSambaOptionsHandler::self()->mountOptions( &internal_share );
#else
  if ( !internal_share.workgroupName().isEmpty() )
  {
    command += QString( " -W %1" ).arg( KShell::quoteArg( internal_share.workgroupName() ) );
  }
  else
  {
    // Do nothing
  }

  if ( !internal_share.hostIP().isEmpty() )
  {
    command += QString( " -I %1" ).arg( internal_share.hostIP() );
  }
  else
  {
    // Do nothing
  }

  command += " -N";
  command += Smb4KSambaOptionsHandler::self()->mountOptions( &internal_share );
#endif
  command += " --";
#ifndef __FreeBSD__
  command += " "+KShell::quoteArg( internal_share.unc() );
#else
  Smb4KSambaOptionsInfo *options_info  = Smb4KSambaOptionsHandler::self()->findItem( &internal_share, true );

  if ( options_info )
  {
    share->setPort( options_info->port() != -1 ? options_info->port() : Smb4KSettings::remoteSMBPort() );
  }
  else
  {
    share->setPort( Smb4KSettings::remoteSMBPort() );
  }

  command += " "+KShell::quoteArg( internal_share.unc( QUrl::RemoveScheme|QUrl::RemovePassword ) );
#endif
  command += " "+KShell::quoteArg( internal_share.path() );

  // Start to mount the share.
  if ( findChildren<Smb4KProcess *>().size() == 0 )
  {
    m_working = true;
    m_state = MOUNTER_MOUNT;
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  // Create the process and set the command.
  Smb4KProcess *p = new Smb4KProcess( Smb4KProcess::Mount, this );
  p->setObjectName( share->unc( QUrl::None ) );
  p->setShellCommand( command );
  p->setOutputChannelMode( KProcess::SeparateChannels );

#ifndef __FreeBSD__
  // Set the password. It is not needed under FreeBSD due to a
  // different implementation.
  p->setEnv( "PASSWD", !authInfo.password().isEmpty() ? authInfo.password() : "", true );
#endif

  QApplication::setOverrideCursor( Qt::WaitCursor );
  emit aboutToStart( &internal_share, Smb4KMounter::MountShare );

  bool success = true;

  // Start the process.
  switch ( p->execute() )
  {
    case -2:
    {
      Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, p->error() );
      success = false;
      break;
    }
    case -1:
    {
      if ( !p->isAborted() )
      {
        Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, p->error() );
      }
      else
      {
        // Do nothing
      }

      success = false;
      break;
    }
    default:
    {
      // The Samba programs report their errors to stdout. So,
      // get the (merged) output from stdout and check if there
      // was an error.
      QString stdout = QString::fromUtf8( p->readAllStandardOutput(), -1 ).trimmed();

      if ( !stdout.isEmpty() )
      {
#ifndef __FreeBSD__
        if ( stdout.contains( "mount error 13", Qt::CaseSensitive ) || stdout.contains( "mount error(13)" )
             /* authentication error */ )
        {
          if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
          {
            mountShare( &internal_share );
          }
          else
          {
            // Do nothing
          }
        }
        else if ( (stdout.contains( "mount error 6" ) || stdout.contains( "mount error(6)" )) /* bad share name */ &&
                  internal_share.shareName().contains( "_", Qt::CaseSensitive ) )
        {
          QString name = static_cast<QString>( internal_share.shareName() ).replace( "_", " " );
          internal_share.setShareName( name );
          mountShare( &internal_share );
        }
        else if ( stdout.contains( "mount error 101" ) || stdout.contains( "mount error(101)" ) /* network unreachable */ )
        {
          kDebug() << "Network unreachable ..." << endl;
        }
#else
        if ( stdout.contains( "Authentication error" ) )
        {
          if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
          {
            mountShare( &internal_share );
          }
          else
          {
            // Do nothing
          }
        }
#endif
        else
        {
          Smb4KCoreMessage::error( ERROR_MOUNTING_SHARE, internal_share.unc(), stdout );
        }

        success = false;
      }
      else
      {
        // No error occurred, so add the share to the list of
        // mounted shares and emit the mounted() and updeted()
        // signals.
        check( &internal_share );

        internal_share.setIsMounted( true );
        share->setIsMounted( true );

        Smb4KShare *new_share = new Smb4KShare( internal_share );
        addMountedShare( new_share );
        emit mounted( new_share );
        emit updated();
      }

      break;
    }
  }

  emit finished( &internal_share, Smb4KMounter::MountShare );
  QApplication::restoreOverrideCursor();

  delete p;

  if ( findChildren<Smb4KProcess *>().size() == 0 )
  {
    m_working = false;
    m_state = MOUNTER_STOP;
    emit stateChanged();
  }
  else
  {
    // Still running
  }

  return success;
}


bool Smb4KMounter::unmountShare( Smb4KShare *share, bool force, bool noMessage )
{
  Q_ASSERT( share );

  // Find the smb4k_umount program
  QString smb4k_umount = KGlobal::dirs()->findResource( "exe", "smb4k_umount" );

  if ( smb4k_umount.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smb4k_umount" );
    return false;
  }
  else
  {
    // Do nothing
  }

  // Find the sudo program
  QString sudo = KStandardDirs::findExe( "sudo" );

  // Check that sudo is installed in the case it is needed.
  if ( ((force && Smb4KSettings::useForceUnmount()) || Smb4KSettings::alwaysUseSuperUser()) &&
       sudo.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sudo" );
    return false;
  }
  else
  {
    // Do nothing
  }

  // Copy the share to avoid problems with 'homes' shares.
  Smb4KShare internal_share( *share );

  if ( internal_share.isForeign() && !Smb4KSettings::unmountForeignShares() )
  {
    if ( !noMessage )
    {
      Smb4KCoreMessage::error( ERROR_UNMOUNTING_NOT_ALLOWED );
    }
    else
    {
      // Do nothing
    }

    return false;
  }
  else
  {
    // Do nothing
  }

  if ( force )
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

    // Ask the user, if he/she really wants to force the unmounting.
    if ( KMessageBox::questionYesNo( parent, i18n( "<qt>Do you really want to force the unmounting of this share?</qt>" ), QString(), KStandardGuiItem::yes(), KStandardGuiItem::no(), "Dont Ask Forced", KMessageBox::Notify ) == KMessageBox::No )
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    // If 'force' is TRUE but the user choose to not force
    // unmounts, throw an error here and exit.
    if ( !Smb4KSettings::useForceUnmount() )
    {
      if ( !noMessage )
      {
        Smb4KCoreMessage::error( ERROR_FEATURE_NOT_ENABLED );
      }
      else
      {
        // Do nothing
      }

      return false;
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

  // Compile the command.
  QString command;

  if ( (force && Smb4KSettings::useForceUnmount()) || Smb4KSettings::alwaysUseSuperUser() )
  {
    command += sudo;
    command += " "+smb4k_umount;
  }
  else
  {
    command += smb4k_umount;
  }

#ifdef __linux__
  if ( force && Smb4KSettings::useForceUnmount() )
  {
    command += " -l"; // lazy unmount
  }
  else
  {
    // Do nothing
  }
#endif

  command += " --";
  command += " "+KShell::quoteArg( share->canonicalPath() );

  // Start to unmount the share.
  if ( findChildren<Smb4KProcess *>().size() == 0 )
  {
    m_working = true;
    m_state = MOUNTER_UNMOUNT;
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  // Create the process and set the command.
  Smb4KProcess *p = new Smb4KProcess( Smb4KProcess::Unmount, this );
  p->setObjectName( share->unc( QUrl::None ) );
  p->setShellCommand( command );
  p->setOutputChannelMode( KProcess::SeparateChannels );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  emit aboutToStart( &internal_share, Smb4KMounter::UnmountShare );

  bool success = true;

  // Start the process.
  switch ( p->execute() )
  {
    case -2:
    {
      Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, p->error() );
      success = false;
      break;
    }
    case -1:
    {
      if ( !p->isAborted() )
      {
        Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, p->error() );
      }
      else
      {
        // Do nothing
      }

      success = false;
      break;
    }
    default:
    {
      // The Samba programs report their errors to stdout. So,
      // get the (merged) output from stdout and check if there
      // was an error.
      QString stderr = QString::fromUtf8( p->readAllStandardOutput(), -1 ).trimmed();

      if ( !stderr.isEmpty() )
      {
        // Filter out everything we do not consider an error.
        QStringList before = stderr.split( "\n" );
        QStringList after;

        foreach ( QString str, before )
        {
          if ( !str.trimmed().startsWith( "smb4k_umount(" ) &&
               !str.trimmed().startsWith( "sudo: unable to resolve host" ) )
          {
            // This is neither a sudo problem nor debug information. We are
            // indeed facing a problem.
            after += str;
          }
          else
          {
            // Do nothing
          }
        }

        stderr = after.join( "\n" );

        if ( !stderr.trimmed().isEmpty() )
        {
          Smb4KCoreMessage::error( ERROR_UNMOUNTING_SHARE, internal_share.unc(), stderr );
          success = false;
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

      // Clean up the mount prefix.
      if ( qstrncmp( internal_share.canonicalPath(),
                     QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8(),
                     QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8().length() ) == 0 )
      {
        QDir dir( internal_share.canonicalPath() );

        if ( dir.rmdir( dir.canonicalPath() ) )
        {
          dir.cdUp();
          dir.rmdir( dir.canonicalPath() );
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Do nothing here. Do not remove any paths that are outside the
        // mount prefix.
      }

      // Find the share in the list, emit the unmounted() signal,
      // remove the share from the global list and exit with the
      // updated() signal.
      if ( success )
      {
        Smb4KShare *listed_share = findShareByPath( internal_share.path() );

        if ( listed_share )
        {
          removeMountedShare( listed_share );
          internal_share.setIsMounted( false );
          share->setIsMounted( false );
          emit unmounted( listed_share );
          emit updated();
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

      break;
    }
  }

  emit finished( &internal_share, Smb4KMounter::UnmountShare );
  QApplication::restoreOverrideCursor();

  delete p;

  if ( findChildren<Smb4KProcess *>().size() == 0 )
  {
    m_working = false;
    m_state = MOUNTER_STOP;
    emit stateChanged();
  }
  else
  {
    // Still running
  }

  return success;
}


bool Smb4KMounter::unmountAllShares()
{
  bool success = true;

  // Never use while( !mountedSharesList().isEmpty() ) {} here,
  // because then the mounter will loop indefinitely when the
  // unmounting of a share fails.
  QListIterator<Smb4KShare *> it( *mountedSharesList() );

  while ( it.hasNext() )
  {
    if ( !unmountShare( it.next(), false, true ) )
    {
      success = false;
    }
    else
    {
      // Do nothing
    }
  }

  return success;
}


void Smb4KMounter::prepareForShutdown()
{
  slotAboutToQuit();
}


void Smb4KMounter::check( Smb4KShare *share )
{
  if ( share )
  {
    CheckThread thread( share, this );
    thread.start();
    thread.check();
    thread.wait( 100 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::saveSharesForRemount()
{
  if ( (Smb4KSettings::remountShares() && priv->aboutToQuit()) || priv->hardwareReason() )
  {
    for ( int i = 0; i < mountedSharesList()->size(); ++i )
    {
      if ( !mountedSharesList()->at( i )->isForeign() )
      {
        Smb4KSambaOptionsHandler::self()->addRemount( mountedSharesList()->at( i ) );
      }
      else
      {
        Smb4KSambaOptionsHandler::self()->removeRemount( mountedSharesList()->at( i ) );
      }
    }

    Smb4KSambaOptionsHandler::self()->sync();
  }
  else
  {
    if ( !Smb4KSettings::remountShares() )
    {
      Smb4KSambaOptionsHandler::self()->clearRemounts();
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KMounter::timerEvent( QTimerEvent * )
{
  if ( !kapp->startingUp() && !m_working )
  {
    // Import the mounted shares.
    import();
  }
  else
  {
    // Do nothing and wait until the application started up.
  }

  if ( m_timeout != Smb4KSettings::checkInterval() )
  {
    m_timeout = Smb4KSettings::checkInterval();
    killTimer( m_timer_id );
    m_timer_id = startTimer( m_timeout );
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KMounter::slotAboutToQuit()
{
  // Tell the application it is about to quit.
  priv->setAboutToQuit();

  // Abort any actions.
  abortAll();

  // Save the shares that need to be remounted.
  saveSharesForRemount();

  // Unmount the shares if the user chose to do so.
  if ( Smb4KSettings::unmountSharesOnExit() )
  {
    unmountAllShares();
  }
  else
  {
    // Do nothing
  }

  // Clean up the mount prefix.
  QDir dir;
  dir.cd( Smb4KSettings::mountPrefix().path() );
  QStringList dirs = dir.entryList( QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort );

  QList<Smb4KShare *> inaccessible = findInaccessibleShares();

  // Remove all directories from the list that belong to
  // inaccessible shares.
  for ( int i = 0; i < inaccessible.size(); ++i )
  {
    int index = dirs.indexOf( inaccessible.at( i )->hostName(), 0 );

    if ( index != -1 )
    {
      dirs.removeAt( index );
      continue;
    }
    else
    {
      continue;
    }
  }

  // Now it is save to remove all empty directories.
  for ( int i = 0; i < dirs.size(); ++i )
  {
    dir.cd( dirs.at( i ) );

    QStringList subdirs = dir.entryList( QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort );

    for ( int k = 0; k < subdirs.size(); ++k )
    {
      dir.rmdir( subdirs.at( k ) );
    }

    dir.cdUp();
    dir.rmdir( dirs.at( i ) );
  }
}


void Smb4KMounter::slotHardwareButtonPressed( Smb4KSolidInterface::ButtonType type )
{
  switch ( type )
  {
    case Smb4KSolidInterface::SleepButton:
    {
      if ( Smb4KSettings::unmountWhenSleepButtonPressed() )
      {
        priv->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        priv->setHardwareReason( false );
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case Smb4KSolidInterface::LidButton:
    {
      if ( Smb4KSettings::unmountWhenLidButtonPressed() )
      {
        priv->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        priv->setHardwareReason( false );
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case Smb4KSolidInterface::PowerButton:
    {
      if ( Smb4KSettings::unmountWhenPowerButtonPressed() )
      {
        priv->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        priv->setHardwareReason( false );
      }
      else
      {
        // Do nothing
      }
    }
    default:
    {
      break;
    }
  }
}


void Smb4KMounter::slotComputerWokeUp()
{
  // Only trigger a remount here, if the network connection is
  // established. If the computer is still disconnected,
  // slotNetworkStatusChanged() will initiate the remounting.
  switch ( Smb4KSolidInterface::self()->networkStatus() )
  {
    case Smb4KSolidInterface::Connected:
    case Smb4KSolidInterface::Unknown:
    {
      priv->setHardwareReason( true );
      triggerRemounts();
      priv->setHardwareReason( false );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KMounter::slotNetworkStatusChanged( Smb4KSolidInterface::ConnectionStatus status )
{
  switch ( status )
  {
    case Smb4KSolidInterface::Connected:
    {
      priv->setHardwareReason( true );
      triggerRemounts();
      priv->setHardwareReason( false );
      break;
    }
    case Smb4KSolidInterface::Disconnected:
    {
      priv->setHardwareReason( true );
      abortAll();
      saveSharesForRemount();
      unmountAllShares();
      priv->setHardwareReason( false );
      break;
    }
    case Smb4KSolidInterface::Unknown:
    {
      priv->setHardwareReason( true );
      triggerRemounts();
      priv->setHardwareReason( false );
      break;
    }
    default:
    {
      break;
    }
  }
}

#include "smb4kmounter.moc"
