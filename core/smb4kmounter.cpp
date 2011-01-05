/***************************************************************************
    smb4kmounter.cpp  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2010 by Alexander Reinholdt
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
#include <kmountpoint.h>

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
#include <smb4kglobal.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4kdefs.h>
#include <smb4khomesshareshandler.h>
#include <smb4kmounter_p.h>
#include <smb4kwalletmanager.h>
#include <smb4kprocess.h>
#include <smb4knotification.h>

using namespace Smb4KGlobal;

#define TIMEOUT 50

K_GLOBAL_STATIC( Smb4KMounterPrivate, priv );



Smb4KMounter::Smb4KMounter() : QObject()
{
  m_timeout = 0;

  connect( kapp,                        SIGNAL( aboutToQuit() ),
           this,                        SLOT( slotAboutToQuit() ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( buttonPressed( Smb4KSolidInterface::ButtonType ) ),
           this,                        SLOT( slotHardwareButtonPressed( Smb4KSolidInterface::ButtonType ) ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( wokeUp() ),
           this,                        SLOT( slotComputerWokeUp() ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( networkStatusChanged( Smb4KSolidInterface::ConnectionStatus ) ),
           this,                        SLOT( slotNetworkStatusChanged( Smb4KSolidInterface::ConnectionStatus ) ) );
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
  startTimer( TIMEOUT );

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

  QString key1, key2;

  if ( !share->isHomesShare() )
  {
    key1 = "mount_"+share->unc();
    key2 = "unmount_"+share->unc();
  }
  else
  {
    key1 = "mount_"+share->homeUNC();
    key2 = "unmount_"+share->homeUNC();
  }

  if ( m_cache.contains( key1 ) )
  {
    Action( "de.berlios.smb4k.mounthelper.mount" ).stop();
  }
  else if ( m_cache.contains( key2 ) )
  {
    Action( "de.berlios.smb4k.mounthelper.unmount" ).stop();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::abortAll()
{
  if ( !kapp->closingDown() )
  {
    foreach ( const QString &key, m_cache )
    {
      if ( key.startsWith( "mount_" ) )
      {
        Action( "de.berlios.smb4k.mounthelper.mount" ).stop();
      }
      else if ( key.startsWith( "unmount_" ) )
      {
        Action( "de.berlios.smb4k.mounthelper.unmount" ).stop();
      }
      else
      {
        // Do nothing
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

  QString key1, key2;

  if ( !share->isHomesShare() )
  {
    key1 = "mount_"+share->unc();
    key2 = "unmount_"+share->unc();
  }
  else
  {
    key1 = "mount_"+share->homeUNC();
    key2 = "unmount_"+share->homeUNC();
  }

  return (m_cache.contains( key1 ) || m_cache.contains( key2 ));
}


void Smb4KMounter::triggerRemounts()
{
  if ( Smb4KSettings::remountShares() || priv->hardwareReason() )
  {
    QList<Action> actions;
    QList<Smb4KSambaOptionsInfo *> list = Smb4KSambaOptionsHandler::self()->sharesToRemount();

    for ( int i = 0; i < list.size(); ++i )
    {
      QList<Smb4KShare *> mounted_shares = findShareByUNC( list.at( i )->unc() );

      if ( !mounted_shares.isEmpty() )
      {
        bool mount = true;

        for ( int j = 0; j < mounted_shares.size(); ++j )
        {
          if ( !mounted_shares.at( j )->isForeign() )
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

          Action mountAction;

          if ( createMountAction( &share, &mountAction ) )
          {
            connect( mountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
                     this, SLOT( slotShareMounted( ActionReply ) ) );
            connect( mountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
                     this, SLOT( slotActionFinished( ActionReply ) ) );

            actions << mountAction;

            m_cache << mountAction.arguments().value( "key" ).toString();

            emit aboutToStart( &share, MountShare );

            priv->addRemount();
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
      else
      {
        // First of all initialize the wallet manager.
        Smb4KWalletManager::self()->init( 0 );

        // Mount the share.
        Smb4KShare share( list.at( i )->unc() );
        share.setWorkgroupName( list.at( i )->workgroupName() );
        share.setHostIP( list.at( i )->ip() );

        Action mountAction;

        if ( createMountAction( &share, &mountAction ) )
        {
          connect( mountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
                   this, SLOT( slotShareMounted( ActionReply ) ) );
          connect( mountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
                   this, SLOT( slotActionFinished( ActionReply ) ) );

          actions << mountAction;

          m_cache << mountAction.arguments().value( "key" ).toString();

          emit aboutToStart( &share, MountShare );

          priv->addRemount();
        }
        else
        {
          // Do nothing
        }
      }
    }

    if ( !actions.isEmpty() )
    {
      QList<Action> denied;
      Action::executeActions( actions, &denied, "de.berlios.smb4k.mounthelper" );

      if ( !denied.isEmpty() )
      {
        for ( int i = 0; i < denied.size(); i++ )
        {
          Smb4KShare share( denied.at( i ).arguments().value( "unc" ).toUrl().toString( QUrl::None ) );
          share.setWorkgroupName( denied.at( i ).arguments().value( "workgroup" ).toString() );

          Smb4KNotification *notification = new Smb4KNotification();
          notification->unmountingFailed( &share, i18n( "The authentication action was denied." ) );
        }
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
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::import()
{
  KMountPoint::List mount_points = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );
  QList<Smb4KShare> mounted_shares;

  for ( int i = 0; i < mount_points.size(); ++i )
  {
#ifndef Q_OS_FREEBSD
    if ( QString::compare( mount_points.at( i )->mountType(), "cifs" ) == 0 )
#else
    if ( QString::compare( mount_points.at( i )->mountType(), "smbfs" ) == 0 )
#endif
    {
      Smb4KShare share;
      share.setUNC( mount_points.at( i )->mountedFrom() );
      share.setPath( mount_points.at( i )->mountPoint() );

#ifndef Q_OS_FREEBSD
      share.setFileSystem( Smb4KShare::CIFS );

      // Check if the share is new and we have to open /proc/mounts (if it exists)
      // to acquire all needed information.
      if ( findShareByPath( mount_points.at( i )->mountPoint().toUtf8() ) == NULL && QFile::exists( "/proc/mounts" ) )
      {
        QStringList contents;
        QFile proc_mounts( "/proc/mounts" );

        if ( proc_mounts.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
          QTextStream ts( &proc_mounts );
          // Note: With Qt 4.3 this seems to be obsolete, but we'll
          // keep it for now.
          ts.setCodec( "UTF-8" );
          ts.setAutoDetectUnicode( true );

          while ( 1 )
          {
            // Only import CIFS shares.
            QString line = ts.readLine( 0 );

            if ( !line.isNull() )
            {
              if ( line.contains( " cifs " ) )
              {
                contents << line;
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

          proc_mounts.close();
        }
        else
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->openingFileFailed( proc_mounts );
          return;
        }

        // Now find the share entry and extract to needed data.
        for ( int j = 0; j < contents.size(); ++j )
        {
          QString entry = contents.at( j );

          if ( entry.contains( mount_points.at( i )->mountPoint() ) )
          {
            // Get the options string. Since the string ends with something
            // like " 0 0", we need to remove the last four characters.
            QString mount_options = entry.section( " cifs ", 1, 1 ).remove( entry.length() - 4, 4 ).trimmed();

            // Domain
            if ( mount_options.contains( "domain=" ) )
            {
              QString tmp = mount_options.section( "domain=", 1, 1 );

              if ( tmp.contains( "," ) )
              {
                // The domain entry is somewhere in the middle of the options
                // string.
                share.setWorkgroupName( tmp.section( ",", 0, 0 ) );
              }
              else
              {
                // The domain entry is at the end of the options string.
                share.setWorkgroupName( tmp );
              }
            }
            else
            {
              // Do nothing
            }

            // IP address
            if ( mount_options.contains( "addr=" ) )
            {
              QString tmp = mount_options.section( "addr=", 1, 1 );

              if ( tmp.contains( "," ) )
              {
                // The IP address entry is somewhere in the middle of the options
                // string.
                share.setHostIP( tmp.section( ",", 0, 0 ) );
              }
              else
              {
                // The IP address entry is at the end of the options string.
                share.setHostIP( tmp );
              }
            }
            else
            {
              // Do nothing
            }

            // Login
            if ( mount_options.contains( "username=" ) )
            {
              QString tmp = mount_options.section( "username=", 1, 1 );

              if ( tmp.contains( "," ) )
              {
                // The user name entry is somewhere in the middle of the options
                // string.
                QString user = tmp.section( ",", 0, 0 );
                share.setLogin( user.isEmpty() ? "guest" : user );
              }
              else
              {
                // The user name entry is at the end of the options string.
                share.setLogin( tmp.isEmpty() ? "guest" : tmp );
              }
            }
            else if ( mount_options.contains( "user=" ) )
            {
              QString tmp = mount_options.section( "user=", 1, 1 );

              if ( tmp.contains( "," ) )
              {
                // The user name entry is somewhere in the middle of the options
                // string.
                QString user = tmp.section( ",", 0, 0 );
                share.setLogin( user.isEmpty() ? "guest" : user );
              }
              else
              {
                // The user name entry is at the end of the options string.
                share.setLogin( tmp.isEmpty() ? "guest" : tmp );
              }
            }
            else
            {
              // Do nothing
            }

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
        // The share is either already known or the user disabled support for
        // the proc file system in the kernel. Either way, just populate all
        // possible entries. The rest will be added/updated by the code below.
        QString login = mount_points.at( i )->mountOptions().join( "," ).section( "user=", 1, 1 ).section( ",", 0, 0 ).trimmed();
        share.setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'user=' entries
      }
#else
      share.setFileSystem( Smb4KShare::SMBFS );
      QString login = mount_points.at( i )->mountOptions().join( "," ).section( "username=", 1, 1 ).section( ",", 0, 0 ).trimmed();
      share.setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'username=' entries
      qDebug() << "Domain and ip address?";
#endif
      share.setIsMounted( true );
      mounted_shares += share;
    }
    else
    {
      continue;
    }
  }

  // Check which shares were unmounted, emit the unmounted() signal
  // on each of the unmounted shares and remove them from the global
  // list.
  // NOTE: The unmount() signal is emitted *BEFORE* the share is removed
  // from the global list! You need to account for that in your application.
  bool found = false;

  for ( int i = 0; i < mountedSharesList().size(); ++i )
  {
    for ( int j = 0; j < mounted_shares.size(); ++j )
    {
      // Check the mount point, since that is unique.
      if ( QString::compare( mountedSharesList().at( i )->canonicalPath(), mounted_shares.at( j ).canonicalPath() ) == 0 ||
           QString::compare( mountedSharesList().at( i )->path(), mounted_shares.at( j ).path() ) == 0 )
      {
        found = true;
        break;
      }
      else
      {
        continue;
      }
    }

    if ( !found )
    {
      mountedSharesList()[i]->setIsMounted( false );
      emit unmounted( mountedSharesList().at( i ) );
      removeMountedShare( mountedSharesList().at( i ) );
    }
    else
    {
      // Do nothing
    }

    found = false;
  }

  // Now add additional information to the shares in the temporary
  // list and insert them to the global list. At the same time, remove
  // the old entry from the list. Also, emit either the updated() or
  // the mounted() signal.
  for ( int i = 0; i < mounted_shares.size(); ++i )
  {
    Smb4KShare *mounted_share = findShareByPath( mounted_shares.at( i ).canonicalPath() );

    if ( mounted_share )
    {
      // Check share.
      if ( !mounted_share->isInaccessible() )
      {
        check( &mounted_shares[i] );
      }
      else
      {
        mounted_shares[i].setInaccessible( true );
      }

      // Copy data.
      if ( !mounted_share->login().isEmpty() &&
           QString::compare( mounted_share->login(), mounted_shares.at( i ).login() ) != 0 )
      {
        mounted_shares[i].setLogin( mounted_share->login() );
      }
      else
      {
        // Do nothing
      }

      if ( !mounted_share->workgroupName().isEmpty() &&
           QString::compare( mounted_share->workgroupName(), mounted_shares.at( i ).workgroupName() ) != 0 )
      {
        mounted_shares[i].setWorkgroupName( mounted_share->workgroupName() );
      }
      else
      {
        // Do nothing
      }

      if ( !mounted_share->hostIP().isEmpty() &&
           QString::compare( mounted_share->hostIP(), mounted_shares.at( i ).hostIP() ) != 0 )
      {
        mounted_shares[i].setHostIP( mounted_share->hostIP() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Check share.
      check( &mounted_shares[i] );
    }

    // Is this a mount that was done by the user or by
    // someone else (or the system)?
    if ( (mounted_shares.at( i ).uid() == getuid() && mounted_shares.at( i ).gid() == getgid()) ||
         (!mounted_shares.at( i ).isInaccessible() &&
          (QString::fromUtf8( mounted_shares.at( i ).path() ).startsWith( Smb4KSettings::mountPrefix().path() ) ||
           QString::fromUtf8( mounted_shares.at( i ).canonicalPath() ).startsWith( QDir::homePath() ))) ||
         (!mounted_shares.at( i ).isInaccessible() &&
          (QString::fromUtf8( mounted_shares.at( i ).canonicalPath() ).startsWith( QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath() ) ||
           QString::fromUtf8( mounted_shares.at( i ).canonicalPath() ).startsWith( QDir::home().canonicalPath() ))) )
    {
      mounted_shares[i].setForeign( false );
    }
    else
    {
      mounted_shares[i].setForeign( true );
    }

    // Get the host that shares this resource and check if we
    // need to set the IP address or workgroup/domain.
    Smb4KHost *host = findHost( mounted_shares.at( i ).hostName(), mounted_shares.at( i ).workgroupName() );

    if ( host )
    {
      // Set the IP address if necessary.
      if ( mounted_shares.at( i ).hostIP().isEmpty() || QString::compare( host->ip(), mounted_shares.at( i ).hostIP() ) != 0 )
      {
        mounted_shares[i].setHostIP( host->ip() );
      }
      else
      {
        // Do nothing
      }

      // Set the workgroup/domain name if necessary.
      if ( mounted_shares.at( i ).workgroupName().isEmpty() )
      {
        mounted_shares[i].setWorkgroupName( host->workgroupName() );
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

    // Now we add the new shares to the global list of shares. We
    // also honor Smb4KSettings::showAllShares() here.
    // 
    // Emit the appropriate signals after the share was processed.
    if ( mounted_share )
    {
      // Honor Smb4KSettings::showAllShares() here.
      if ( !mounted_shares.at( i ).isForeign() || Smb4KSettings::showAllShares() )
      {
        // This share was previouly mounted.
        removeMountedShare( mounted_share );

        Smb4KShare *new_share = new Smb4KShare( mounted_shares[i] );

        // To avoid incompatibilities, we remove a trailing slash from
        // the UNC now, if it is present.
        if ( new_share->unc( QUrl::None ).endsWith( "/" ) )
        {
          QString u = new_share->unc( QUrl::None );
          u.chop( 1 );
          new_share->setUNC( u );
        }
        else
        {
          // Do nothing
        }

        addMountedShare( new_share );
        emit updated( new_share );
      }
      else
      {
        mounted_share->setIsMounted( false );
        emit unmounted( mounted_share );
        removeMountedShare( mounted_share );
      }
    }
    else
    {
      // Honor Smb4KSettings::showAllShares() here.
      if ( !mounted_shares.at( i ).isForeign() || Smb4KSettings::showAllShares() )
      {
        // This is a new share.
        Smb4KShare *new_share = new Smb4KShare( mounted_shares[i] );

        // To avoid incompatibilities, we remove a trailing slash from
        // the UNC now, if it is present.
        if ( new_share->unc( QUrl::None ).endsWith( "/" ) )
        {
          QString u = new_share->unc( QUrl::None );
          u.chop( 1 );
          new_share->setUNC( u );
        }
        else
        {
          // Do nothing
        }

        addMountedShare( new_share );
        emit mounted( new_share );
      }
      else
      {
        // Do nothing
      }
    }
  }
}


void Smb4KMounter::mountShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  // Check if the UNC is valid. Otherwise, we can just return here
  // with an error message.
  if ( !share->url().isValid() )
  {
    // FIXME: Throw an error.
    qDebug() << "Invalid UNC";
    return;
  }
  else
  {
    // Do nothing
  }

  Action mountAction;

  if ( createMountAction( share, &mountAction ) )
  {
    m_cache << mountAction.arguments().value( "key" ).toString();

    connect( mountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
             this, SLOT( slotShareMounted( ActionReply ) ) );
    connect( mountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
             this, SLOT( slotActionFinished( ActionReply ) ) );

    if ( m_cache.size() == 0 )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      m_state = MOUNTER_MOUNT;
      emit stateChanged();
    }
    else
    {
      // Already running
    }

    emit aboutToStart( share, MountShare );

    ActionReply reply = mountAction.execute();

    if ( reply.failed() )
    {
      // If the action failed, show an error message.
      Smb4KNotification *notification = new Smb4KNotification();

      if ( reply.type() == ActionReply::KAuthError )
      {
        switch ( reply.errorCode() )
        {
          case ActionReply::NoResponder:
          {
            notification->actionFailed( Smb4KNotification::MountAction, "NoResponder" );
            break;
          }
          case ActionReply::NoSuchAction:
          {
            notification->actionFailed( Smb4KNotification::MountAction, "NoSuchAction" );
            break;
          }
          case ActionReply::InvalidAction:
          {
            notification->actionFailed( Smb4KNotification::MountAction, "InvalidAction" );
            break;
          }
          case ActionReply::AuthorizationDenied:
          {
            notification->actionFailed( Smb4KNotification::MountAction, "AuthorizationDenied" );
            break;
          }
//           case ActionReply::UserCancelled:
//           {
//             notification->actionFailed( Smb4KNotification::MountAction, "UserCancelled" );
//             break;
//           }
          case ActionReply::HelperBusy:
          {
            notification->actionFailed( Smb4KNotification::MountAction, "HelperBusy" );
            break;
          }
          case ActionReply::DBusError:
          {
            notification->actionFailed( Smb4KNotification::MountAction, "DBusError" );
            break;
          }
          default:
          {
            break;
          }
        }
      }
      else
      {
        notification->actionFailed( Smb4KNotification::MountAction, QString() );
      }

      int index = m_cache.indexOf( mountAction.arguments().value( "key" ).toString() );
      m_cache.removeAt( index );

      emit finished( share, MountShare );
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


void Smb4KMounter::unmountShare( Smb4KShare *share, bool force, bool silent )
{
  Q_ASSERT( share );

  if ( !priv->aboutToQuit() )
  {
    // Execute asynchroneously.
    Action unmountAction;

    if ( createUnmountAction( share, force, silent, &unmountAction ) )
    {
      m_cache << unmountAction.arguments().value( "key" ).toString();

      connect( unmountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
               this, SLOT( slotShareUnmounted( ActionReply ) ) );
      connect( unmountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
               this, SLOT( slotActionFinished( ActionReply ) ) );

      if ( m_cache.size() == 0 )
      {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        m_state = MOUNTER_UNMOUNT;
        emit stateChanged();
      }
      else
      {
        // Already running
      }

      emit aboutToStart( share, UnmountShare );

      ActionReply reply = unmountAction.execute();

      if ( reply.failed() )
      {
        // If the action failed, show an error message.
        Smb4KNotification *notification = new Smb4KNotification();

        if ( reply.type() == ActionReply::KAuthError )
        {
          switch ( reply.errorCode() )
          {
            case ActionReply::NoResponder:
            {
              notification->actionFailed( Smb4KNotification::UnmountAction, "NoResponder" );
              break;
            }
            case ActionReply::NoSuchAction:
            {
              notification->actionFailed( Smb4KNotification::UnmountAction, i18n( "NoSuchAction" ) );
              break;
            }
            case ActionReply::InvalidAction:
            {
              notification->actionFailed( Smb4KNotification::UnmountAction, "InvalidAction" );
              break;
            }
            case ActionReply::AuthorizationDenied:
            {
              notification->actionFailed( Smb4KNotification::UnmountAction, "AuthorizationDenied" );
              break;
            }
//            case ActionReply::UserCancelled:
//            {
//              notification->actionFailed( Smb4KNotification::UnmountAction, "UserCanceled" );
//              break;
//            }
            case ActionReply::HelperBusy:
            {
              notification->actionFailed( Smb4KNotification::UnmountAction, "HelperBusy" );
              break;
            }
            case ActionReply::DBusError:
            {
              notification->actionFailed( Smb4KNotification::UnmountAction, "DBusError" );
              break;
            }
            default:
            {
              break;
            }
          }
        }
        else
        {
          notification->actionFailed( Smb4KNotification::UnmountAction, QString() );
        }

        int index = m_cache.indexOf( unmountAction.arguments().value( "key" ).toString() );
        m_cache.removeAt( index );

        emit finished( share, MountShare );
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
  else
  {
    Action unmountAction;

    if ( createUnmountAction( share, force, true, &unmountAction ) )
    {
      emit aboutToStart( share, UnmountShare );
      unmountAction.execute();
      emit finished( share, UnmountShare );
    }
    else
    {
      // Do nothing.
    }
  }
}


void Smb4KMounter::unmountAllShares()
{
  // Never use
  //
  //    while ( !mountedSharesList().isEmpty() ) { ... }
  //
  // here, because then the mounter will loop indefinitely when the
  // unmounting of a share fails.

  QList<Action> actions;

  QListIterator<Smb4KShare *> it( mountedSharesList() );

  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();

    Action unmountAction;

    if ( createUnmountAction( share, false, false, &unmountAction ) )
    {
      connect( unmountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
               this, SLOT( slotShareUnmounted( ActionReply ) ) );
      connect( unmountAction.watcher(), SIGNAL( actionPerformed( ActionReply ) ),
               this, SLOT( slotActionFinished( ActionReply ) ) );

      actions << unmountAction;

      m_cache << unmountAction.arguments().value( "key" ).toString();

      emit aboutToStart( share, UnmountShare );

      priv->addUnmount();
    }
    else
    {
      // Do nothing
    }
  }

  if ( !actions.isEmpty() )
  {
    QList<Action> denied;
    Action::executeActions( actions, &denied, "de.berlios.smb4k.mounthelper" );

    if ( !denied.isEmpty() )
    {
      for ( int i = 0; i < denied.size(); i++ )
      {
        Smb4KShare *share = findShareByPath( actions.at( i ).arguments().value( "mountpoint" ).toByteArray() );

        if ( share )
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->unmountingFailed( share, i18n( "The authentication was denied." ) );
        }
        else
        {
          // Do nothing
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
    // Do nothing
  }
}


bool Smb4KMounter::createMountAction( Smb4KShare *share, Action *action )
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

  for ( int i = 0; i < paths.size(); i++ )
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

  // Check if the share has already been mounted or a mount
  // is currently in progress.
  QList<Smb4KShare *> list;

  if ( share->isHomesShare() )
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

    if ( !Smb4KHomesSharesHandler::self()->specifyUser( share, parent ) )
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    list = findShareByUNC( share->homeUNC( QUrl::None ) );
  }
  else
  {
    list = findShareByUNC( share->unc( QUrl::None ) );
  }

  // Check if it is already mounted:
  for ( int i = 0; i != list.size(); ++i )
  {
    if ( !list.at( i )->isForeign() )
    {
      return false;
    }
    else
    {
      continue;
    }
  }

  // Check if the mount process is in progress.
  QString key;

  if ( !share->isHomesShare() )
  {
    key = "mount_"+share->unc();
  }
  else
  {
    key = "mount_"+share->homeUNC();
  }

  if ( m_cache.contains( key ) )
  {
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

  // Get the authentication information.
  Smb4KAuthInfo authInfo( share );
  Smb4KWalletManager::self()->readAuthInfo( &authInfo );

  // Set the authentication information
  share->setAuthInfo( &authInfo );
  
  // Set the file system
#ifndef Q_OS_FREEBSD
  share->setFileSystem( Smb4KShare::CIFS );
#else
  share->setFileSystem( Smb4KShare::SMBFS );
#endif
//   share->setLogin( QString::fromUtf8( authInfo.login() ) );

  // Compile the arguments
  QString arguments;
  QMap<QString, QString> global_options = Smb4KSambaOptionsHandler::self()->globalSambaOptions();
  Smb4KSambaOptionsInfo *options_info  = Smb4KSambaOptionsHandler::self()->findItem( share, true );

  // Set the port before passing the full UNC. This is mainly
  // needed for FreeBSD.
  if ( options_info )
  {
    share->setPort( options_info->smbPort() != -1 ? options_info->smbPort() : Smb4KSettings::remoteSMBPort() );
  }
  else
  {
    share->setPort( Smb4KSettings::remoteSMBPort() );
  }

#ifndef Q_OS_FREEBSD
  arguments += "-o";
  arguments += " ";

  // Workgroup
  arguments += !share->workgroupName().trimmed().isEmpty() ?
               QString( "domain=%1" ).arg( KShell::quoteArg( share->workgroupName() ) ) :
               "";

  // Host IP
  arguments += !share->hostIP().trimmed().isEmpty() ?
               QString( ",ip=%1" ).arg( share->hostIP() ) :
               "";

  // User
  arguments += !authInfo.login().isEmpty() ?
               QString( ",user=%1" ).arg( authInfo.login() ) :
               ",guest";

  // Client's and server's NetBIOS name
  // According to the manual page, this is only needed when port 139
  // is used. So, we only pass the NetBIOS name in that case.
  if ( Smb4KSettings::remoteFileSystemPort() == 139 || (options_info && options_info->fileSystemPort() == 139) )
  {
    // The client's NetBIOS name.
    if ( !Smb4KSettings::netBIOSName().isEmpty() )
    {
      arguments += QString( ",netbiosname=%1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
    }
    else
    {
      if ( !global_options["netbios name"].isEmpty() )
      {
        arguments += QString( ",netbiosname=%1" ).arg( KShell::quoteArg( global_options["netbios name"] ) );
      }
      else
      {
        // Do nothing
      }
    }

    // The server's NetBIOS name.
    arguments += ",servern="+KShell::quoteArg( share->hostName() );
  }
  else
  {
    // Do nothing
  }

  // UID
  arguments += QString( ",uid=%1" ).arg( options_info ? options_info->uid() : (uid_t)Smb4KSettings::userID().toInt() );

  // GID
  arguments += QString( ",gid=%1" ).arg( options_info ? options_info->gid() : (gid_t)Smb4KSettings::groupID().toInt() );

  // Client character set
  switch ( Smb4KSettings::clientCharset() )
  {
    case Smb4KSettings::EnumClientCharset::default_charset:
    {
      if ( !global_options["unix charset"].isEmpty() )
      {
        arguments += QString( ",iocharset=%1" ).arg( global_options["unix charset"].toLower() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      arguments += QString( ",iocharset=%1" ).arg( Smb4KSettings::self()->clientCharsetItem()->label() );
      break;
    }
  }

  // Port
  arguments += QString( ",port=%1" ).arg( (options_info && options_info->fileSystemPort() != -1) ?
               options_info->fileSystemPort() : Smb4KSettings::remoteFileSystemPort() );

  // Write access
  if ( options_info )
  {
    switch ( options_info->writeAccess() )
    {
      case Smb4KSambaOptionsInfo::ReadWrite:
      {
        arguments += ",rw";
        break;
      }
      case Smb4KSambaOptionsInfo::ReadOnly:
      {
        arguments += ",ro";
        break;
      }
      default:
      {
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            arguments += ",rw";
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            arguments += ",ro";
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
        arguments += ",rw";
        break;
      }
      case Smb4KSettings::EnumWriteAccess::ReadOnly:
      {
        arguments += ",ro";
        break;
      }
      default:
      {
        break;
       }
    }
  }

  // File mask
  arguments += !Smb4KSettings::fileMask().isEmpty() ? QString( ",file_mode=%1" ).arg( Smb4KSettings::fileMask() ) : "";

  // Directory mask
  arguments += !Smb4KSettings::directoryMask().isEmpty() ? QString( ",dir_mode=%1" ).arg( Smb4KSettings::directoryMask() ) : "";

  // Permission checks
  arguments += Smb4KSettings::permissionChecks() ? ",perm" : ",noperm";

  // Client controls IDs
  arguments += Smb4KSettings::clientControlsIDs() ? ",setuids" : ",nosetuids";

  // Server inode numbers
  arguments += Smb4KSettings::serverInodeNumbers() ? ",serverino" : ",noserverino";

  // Inode data caching
  arguments += Smb4KSettings::noInodeDataCaching() ? ",directio" : "";

  // Translate reserved characters
  arguments += Smb4KSettings::translateReservedChars() ? ",mapchars" : ",nomapchars";

  // Locking
  arguments += Smb4KSettings::noLocking() ? ",nolock" : "";

  // Security mode
  switch ( Smb4KSettings::securityMode() )
  {
    case Smb4KSettings::EnumSecurityMode::None:
    {
      arguments += ",sec=none";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5:
    {
      arguments += ",sec=krb5";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5i:
    {
      arguments += ",sec=krb5i";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlm:
    {
      arguments += ",sec=ntlm";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmi:
    {
      arguments += ",sec=ntlmi";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2:
    {
      arguments += ",sec=ntlmv2";
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2i:
    {
      arguments += ",sec=ntlmv2i";
      break;
    }
    default:
    {
      break;
    }
  }

  // Global custom options provided by the user
  arguments += !Smb4KSettings::customCIFSOptions().isEmpty() ? ","+Smb4KSettings::customCIFSOptions() : "";

  // Fix existing comma, if necessary.
  if ( arguments.endsWith( "," ) )
  {
    arguments.truncate( arguments.length() - 1 );
  }
  else
  {
    // Do nothing
  }
#else
  // Workgroup
  arguments += !share->workgroupName().isEmpty() ?
               QString( " -W %1" ).arg( KShell::quoteArg( share->workgroupName() ) ) :
               "";

  // Host IP
  arguments += !share->hostIP().isEmpty() ? QString( " -I %1" ).arg( share->hostIP() ) : "";

  // Do not ask for a password. Use ~/.nsmbrc instead.
  arguments += " -N";

  // UID
  arguments += QString( " -u %1" ).arg( options_info ? options_info->uid() : (uid_t)Smb4KSettings::userID().toInt() );

  // GID
  arguments += QString( " -g %1" ).arg( options_info ?  options_info->gid() : (uid_t)Smb4KSettings::groupID().toInt() );

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
      charset = Smb4KSettings::self()->clientCharsetItem()->label();
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
      codepage = Smb4KSettings::self()->serverCodepageItem()->label();
      break;
    }
  }

  arguments += (!charset.isEmpty() && !codepage.isEmpty()) ? QString( " -E %1:%2" ).arg( charset, codepage ) : "";

  // File mask
  arguments += !Smb4KSettings::fileMask().isEmpty() ? QString( " -f %1" ).arg( Smb4KSettings::fileMask() ) : "";

  // Directory mask
  arguments += !Smb4KSettings::directoryMask().isEmpty() ? QString( " -d %1" ).arg( Smb4KSettings::directoryMask() ) : "";
#endif

  action->setName( "de.berlios.smb4k.mounthelper.mount" );
  action->setHelperID( "de.berlios.smb4k.mounthelper" );
  action->addArgument( "mount_binary", mount );
  action->addArgument( "mount_arguments", arguments );
  action->addArgument( "key", key );

  // Now add everything we need to create an Smb4KShare object in slotShareMounted().
  if ( !share->isHomesShare() )
  {
    action->addArgument( "unc", share->url() );
  }
  else
  {
    action->addArgument( "unc", share->homeURL() );
  }
  action->addArgument( "workgroup", share->workgroupName() );
  action->addArgument( "comment", share->comment() );
  action->addArgument( "host_ip", share->hostIP() );
  action->addArgument( "mountpoint", share->canonicalPath() );

  return true;
}


bool Smb4KMounter::createUnmountAction( Smb4KShare *share, bool force, bool silent, Action *action )
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

  for ( int i = 0; i < paths.size(); i++ )
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

 // Check if the umount process is in progress.
  QString key;

  if ( !share->isHomesShare() )
  {
    key = "unmount_"+share->unc();
  }
  else
  {
    key = "unmount_"+share->homeUNC();
  }

  if ( m_cache.contains( key ) )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // Complain if the share is a foreign one and unmounting those
  // is prohibited.
  if ( share->isForeign() && !Smb4KSettings::unmountForeignShares() )
  {
    if ( !silent )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->unmountingNotAllowed( share );
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

  // Ask whether the user indeed wishes to force the unmounting.
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
  }
  else
  {
    // Do nothing
  }

  // Compile the arguments.
  QString arguments;

#ifdef __linux__
  if ( force )
  {
    arguments += "-l"; // lazy unmount
  }
  else
  {
    // Do nothing
  }
#endif

  action->setName( "de.berlios.smb4k.mounthelper.unmount" );
  action->setHelperID( "de.berlios.smb4k.mounthelper" );
  action->addArgument( "umount_binary", umount );
  action->addArgument( "umount_arguments", arguments );
  action->addArgument( "key", key );

  // Now add everything we need.
  action->addArgument( "unc", share->url() );
  action->addArgument( "mountpoint", share->canonicalPath() );

  return true;
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
    thread.wait();
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
    for ( int i = 0; i < mountedSharesList().size(); ++i )
    {
      if ( !mountedSharesList().at( i )->isForeign() )
      {
        Smb4KSambaOptionsHandler::self()->addRemount( mountedSharesList().at( i ) );
      }
      else
      {
        Smb4KSambaOptionsHandler::self()->removeRemount( mountedSharesList().at( i ) );
      }
    }
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
  if ( !kapp->startingUp() && !isRunning() )
  {
    if ( !m_retries.isEmpty() )
    {
      mountShare( &m_retries.first() );
      m_retries.removeFirst();
    }
    else
    {
      // Do nothing
    }
    
    if ( m_timeout == Smb4KSettings::checkInterval() )
    {
      // Import the mounted shares.
      import();
      m_timeout = 0;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing and wait until the application started up.
  }
  
  m_timeout += TIMEOUT;
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


void Smb4KMounter::slotActionFinished( ActionReply reply )
{
  int index = m_cache.indexOf( reply.data().value( "key" ).toString() );
  if ( index != -1 )
  {
    m_cache.removeAt( index );
  }
  else
  {
    // Do nothing
  }

  // Now process the action.
  if ( !reply.failed() )
  {
    Smb4KShare share;

    if ( reply.data().value( "key" ).toString().startsWith( "mount_" ) )
    {
      // Check if a mount error occurred.
      QString stderr( reply.data()["stderr"].toString() );

      if ( !stderr.isEmpty() )
      {
        // Ooops, something went wrong. We do not check for the mounted share
        // but create a share from the values provided by reply, so that we
        // can emit signals etc.
        share.setUNC( reply.data()["unc"].toUrl().toString( QUrl::None ) );
        share.setWorkgroupName( reply.data()["workgroup"].toString() );
        share.setHostIP( reply.data()["host_ip"].toString() );
        share.setComment( reply.data()["comment"].toString() );
        share.setPath( reply.data()["mountpoint"].toString() );
        
#ifndef Q_OS_FREEBSD
        if ( stderr.contains( "mount error 13", Qt::CaseSensitive ) || stderr.contains( "mount error(13)" )
            /* authentication error */ )
        {
          Smb4KAuthInfo authInfo( &share );

          if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
          {
            // Kill the currently active override cursor. Another
            // one will be set in an instant by mountShare().
            QApplication::restoreOverrideCursor();
            m_retries << share;
          }
          else
          {
            // Do nothing
          }
        }
        else if ( (stderr.contains( "mount error 6" ) || stderr.contains( "mount error(6)" )) /* bad share name */ &&
                  share.shareName().contains( "_", Qt::CaseSensitive ) )
        {
          // Kill the currently active override cursor. Another
          // one will be set in an instant by mountShare().
          QApplication::restoreOverrideCursor();
          share.setShareName( static_cast<QString>( share.shareName() ).replace( "_", " " ) );
          m_retries << share;
        }
        else if ( stderr.contains( "mount error 101" ) || stderr.contains( "mount error(101)" ) /* network unreachable */ )
        {
          qDebug() << "Network unreachable ..." << endl;
        }
 #else
        if ( stderr.contains( "Authentication error" ) )
        {
          Smb4KAuthInfo authInfo( &share );

          if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
          {
            // Kill the currently active override cursor. Another
            // one will be set in an instant by mountShare().
            QApplication::restoreOverrideCursor();
            m_retries << share;
          }
          else
          {
            // Do nothing
          }
        }
 #endif
        else
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->mountingFailed( &share, stderr );
        }
      }
      else
      {
        // Everything is fine, we can check for the mounted share.
        share = *(findShareByPath( reply.data().value( "mountpoint" ).toByteArray() ));
      }

      emit finished( &share, MountShare );
    }
    else if ( reply.data().value( "key" ).toString().startsWith( "unmount_" ) )
    {
      // Create a share for emitting the signals.
      share.setUNC( reply.data()["unc"].toUrl().toString( QUrl::None ) );
      share.setWorkgroupName( reply.data()["workgroup"].toString() );
      share.setHostIP( reply.data()["host_ip"].toString() );
      share.setComment( reply.data()["comment"].toString() );
      share.setPath( reply.data()["mountpoint"].toString() );

      // Check if an error occurred.
      QString stderr( reply.data().value( "stderr" ).toString() );

      if ( !stderr.isEmpty() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->unmountingFailed( &share, stderr );
      }
      else
      {
        // Do nothing
      }

      emit finished( &share, UnmountShare );
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // If the action failed, show an error message.
    Smb4KNotification *notification = new Smb4KNotification();
    notification->actionFailed( Smb4KNotification::UnmountAction, QString() );
  }

  if ( m_cache.size() == 0 )
  {
    m_state = MOUNTER_STOP;
    emit stateChanged();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotShareMounted( ActionReply reply )
{
  if ( !reply.failed() )
  {
    // Check that the share has not already been entered into the list.
    Smb4KShare *share = findShareByPath( reply.data()["mountpoint"].toByteArray() );

    if ( !share )
    {
      // Create a Smb4KShare object from the information returned
      // by 'reply'.
      // NOTE: Remove the password from the URL/UNC to avoid empty user info
      // in the new share object if the password contains special characters!
      share = new Smb4KShare( reply.data()["unc"].toUrl().toString( QUrl::RemovePassword ) );
      share->setWorkgroupName( reply.data()["workgroup"].toString() );
      share->setComment( reply.data()["comment"].toString() );
      share->setHostIP( reply.data()["host_ip"].toString() );
      share->setPath( reply.data()["mountpoint"].toString() );

      // Check that we actually mounted the share and emit
      // the mounted() signal if we found it.
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

      if ( mountpoint_found )
      {
        // Set the share as mounted.
        share->setIsMounted( true );

        // Check the usage, etc.
        check( share );

        addMountedShare( share );

        // Check whether this was a remount or not, do the necessary
        // things and notify the user.
        if ( priv->pendingRemounts() != 0 && Smb4KSambaOptionsHandler::self()->findItem( share ) != NULL )
        {
          Smb4KSambaOptionsHandler::self()->removeRemount( share );
          priv->removeRemount();

          if ( priv->pendingRemounts() == 0 )
          {
            Smb4KNotification *notification = new Smb4KNotification( this );
            notification->sharesRemounted( priv->initialRemounts(), priv->initialRemounts() );
            priv->clearRemounts();
          }
          else
          {
            if ( !m_cache.isEmpty() )
            {
              bool still_mounting = false;

              if ( reply.data().value( "key" ).toString().startsWith( "mount_" ) &&
                  Smb4KSambaOptionsHandler::self()->findItem( share ) != NULL )
              {
                still_mounting = true;
              }
              else
              {
                // Do nothing
              }

              if ( !still_mounting )
              {
                Smb4KNotification *notification = new Smb4KNotification( this );
                notification->sharesRemounted( priv->initialRemounts(), (priv->initialRemounts() - priv->pendingRemounts()) );
                priv->clearRemounts();
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              Smb4KNotification *notification = new Smb4KNotification( this );
              notification->sharesRemounted( priv->initialRemounts(), (priv->initialRemounts() - priv->pendingRemounts()) );
              priv->clearRemounts();
            }
          }
        }
        else
        {
          Smb4KNotification *notification = new Smb4KNotification( this );
          notification->shareMounted( share );
        }

        // Finally, emit the mounted() signal.
        emit mounted( share );
      }
      else
      {
        delete share;
      }
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing. Errors are reported by slotActionFinished().
  }
}


void Smb4KMounter::slotShareUnmounted( ActionReply reply )
{
  if ( !reply.failed() )
  {
    // Get the share that was unmounted.
    Smb4KShare *share = findShareByPath( reply.data()["mountpoint"].toByteArray() );

    if ( share )
    {
      // Check that we actually unmounted the share and emit
      // the unmounted() signal if it is really gone.
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
        // Clean up the mount prefix.
        if ( qstrncmp( reply.data()["mountpoint"].toString().toUtf8(),
             QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8(),
             QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8().length() ) == 0 )
        {
          QDir dir( reply.data()["mountpoint"].toString() );

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

        // Set the share as unmounted.
        share->setIsMounted( false );

        if ( priv->pendingUnmounts() != 0 )
        {
          priv->removeUnmount();

          if ( priv->pendingUnmounts() == 0 )
          {
            Smb4KNotification *notification = new Smb4KNotification( this );
            notification->allSharesUnmounted( priv->initialUnmounts(), priv->initialUnmounts() );
            priv->clearUnmounts();
          }
          else
          {
            if ( !m_cache.isEmpty() )
            {
              bool still_unmounting = false;

              foreach ( const QString &key, m_cache )
              {
                if ( key.startsWith( "unmount_" ) )
                {
                  still_unmounting = true;
                  break;
                }
                else
                {
                  continue;
                }
              }

              if ( !still_unmounting )
              {
                Smb4KNotification *notification = new Smb4KNotification( this );
                notification->allSharesUnmounted( priv->initialUnmounts(), (priv->initialUnmounts() - priv->pendingUnmounts()) );
                priv->clearUnmounts();
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              Smb4KNotification *notification = new Smb4KNotification( this );
              notification->allSharesUnmounted( priv->initialUnmounts(), (priv->initialUnmounts() - priv->pendingUnmounts()) );
              priv->clearUnmounts();
            }
          }
        }
        else
        {
          Smb4KNotification *notification = new Smb4KNotification( this );
          notification->shareUnmounted( share );
        }

        emit unmounted( share );
        removeMountedShare( share );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing. The share is already gone.
    }
  }
  else
  {
    // Do nothing. Error are reported by slotActionFinished()
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
