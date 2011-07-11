/***************************************************************************
    smb4kmounter.cpp  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QApplication>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QDesktopWidget>
#include <QTimer>
#ifdef __FreeBSD__
#include <QFileInfo>
#endif

// KDE includes
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
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
#include <smb4kglobal.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4khomesshareshandler.h>
#include <smb4kmounter_p.h>
#include <smb4kwalletmanager.h>
#include <smb4kprocess.h>
#include <smb4knotification.h>
#include <smb4kbookmarkhandler.h>
#include <smb4kcustomoptionsmanager.h>
#include <smb4kcustomoptions.h>

using namespace Smb4KGlobal;

#define TIMEOUT 50

K_GLOBAL_STATIC( Smb4KMounterPrivate, p );



Smb4KMounter::Smb4KMounter() : KCompositeJob( 0 )
{
  m_timeout = 0;
  m_dialog = NULL;

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
  return &p->instance;
}


void Smb4KMounter::abort( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  QString unc;

  if ( !share->isHomesShare() )
  {
    unc = share->unc();
  }
  else
  {
    unc = share->homeUNC();
  }

  QListIterator<KJob *> it( subjobs() );
  
  while ( it.hasNext() )
  {
    KJob *job = it.next();
    
    if ( QString::compare( job->objectName(), QString( "MountJob_%1" ).arg( unc ), Qt::CaseInsensitive ) == 0 )
    {
      job->kill( KJob::EmitResult );
      continue;
    }
    else if ( QString::compare( job->objectName(), QString( "UnmountJob_%1" )
                 .arg( QString::fromUtf8( share->canonicalPath() ) ), Qt::CaseInsensitive ) == 0 )
    {
      job->kill( KJob::EmitResult );
      continue;
    }
    else
    {
      continue;
    }
  }
}


void Smb4KMounter::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QListIterator<KJob *> it( subjobs() );
    
    while ( it.hasNext() )
    {
      it.next()->kill( KJob::EmitResult );
    }
  }
  else
  {
    // p has already been deleted
  }
}


bool Smb4KMounter::isRunning( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  QString unc;

  if ( !share->isHomesShare() )
  {
    unc = share->unc();
  }
  else
  {
    unc = share->homeUNC();
  }

  QListIterator<KJob *> it( subjobs() );
  
  while ( it.hasNext() )
  {
    KJob *job = it.next();
    
    if ( QString::compare( job->objectName(), QString( "MountJob_%1" ).arg( unc ), Qt::CaseInsensitive ) == 0 )
    {
      return true;
    }
    else if ( QString::compare( job->objectName(), QString( "UnmountJob_%1" ).arg( unc ), Qt::CaseInsensitive ) == 0 )
    {
      return true;
    }
    else
    {
      continue;
    }
  }
  
  return false;
}


void Smb4KMounter::triggerRemounts()
{
  if ( Smb4KSettings::remountShares() || p->hardwareReason() )
  {
    // Get the shares that are to be remounted
    QList<Smb4KCustomOptions *> list = Smb4KCustomOptionsManager::self()->sharesToRemount();
    QList<Smb4KShare *> remounts;

    if ( !list.isEmpty() )
    {
      // Check which ones actually need to be remounted.
      for ( int i = 0; i < list.size(); ++i )
      {
        QList<Smb4KShare *> mounted_shares = findShareByUNC( list.at( i )->share()->unc() );

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
            Smb4KShare *share = new Smb4KShare( *list.at( i )->share() );
            remounts << share;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          Smb4KShare *share = new Smb4KShare( *list.at( i )->share() );
          remounts << share;
        }
      }

      if ( !remounts.isEmpty() )
      {
        mountShares( remounts );
      }
      else
      {
        // Do nothing
      }

      // Clear list
      while ( !remounts.isEmpty() )
      {
        delete remounts.takeFirst();
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
      Smb4KShare share( mount_points.at( i )->mountedFrom() );
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

      // Try to get the login from the mount options.
      if ( share.login().isEmpty() )
      {
	QString login = mount_points.at( i )->mountOptions().join( "," ).section( "username=", 1, 1 ).section( ",", 0, 0 ).trimmed();
	share.setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'username=' entries
      }
      else
      {
	// Do nothing
      }
//       qDebug() << "Domain and ip address?";
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


void Smb4KMounter::mountShare( Smb4KShare *share, QWidget *parent )
{
  Q_ASSERT( share );

  // Check that the URL is valid. Otherwise, we can just return here
  // with an error message.
  if ( !share->url().isValid() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->invalidURLPassed();
    return;
  }
  else
  {
    // Do nothing
  }
  
  // Check if the share has already been mounted or a mount
  // is currently in progress.
  QList<Smb4KShare *> mounted_shares;
  QString unc;
  bool mounted = false;

  if ( share->isHomesShare() )
  {
    if ( !Smb4KHomesSharesHandler::self()->specifyUser( share, parent ) )
    {
      return;
    }
    else
    {
      // Do nothing
    }
    
    unc = share->homeUNC( QUrl::None );
    mounted_shares = findShareByUNC( unc );
  }
  else
  {
    unc = share->unc( QUrl::None );
    mounted_shares = findShareByUNC( unc );
  }
  
  // Check if it is already mounted:
  for ( int i = 0; i != mounted_shares.size(); ++i )
  {
    if ( !mounted_shares.at( i )->isForeign() )
    {
      mounted = true;
      break;
    }
    else
    {
      continue;
    }
  } 
  
  if ( !mounted )
  {
    QListIterator<KJob *> it( subjobs() );
    
    while ( it.hasNext() )
    {
      KJob *job = it.next();
      
      if ( QString::compare( job->objectName(), QString( "MountJob_%1" ).arg( unc ), Qt::CaseInsensitive ) == 0 )
      {
        // Already running
        return;
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    return;
  }
  
  Smb4KAuthInfo authInfo( share );
  Smb4KWalletManager::self()->readAuthInfo( &authInfo );
  share->setAuthInfo( &authInfo );
  
  // Create a new job and add it to the subjobs
  Smb4KMountJob *job = new Smb4KMountJob( this );
  job->setObjectName( QString( "MountJob_%1" ).arg( unc ) );
  job->setupMount( share, parent );

  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( authError( Smb4KMountJob * ) ), SLOT( slotAuthError( Smb4KMountJob * ) ) );
  connect( job, SIGNAL( retry( Smb4KMountJob * ) ), SLOT( slotRetryMounting( Smb4KMountJob * ) ) );
  connect( job, SIGNAL( aboutToStart( const QList<Smb4KShare> & ) ), SLOT( slotAboutToStartMounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( finished( const QList<Smb4KShare> & ) ), SLOT( slotFinishedMounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( mounted( Smb4KShare * ) ), SLOT( slotShareMounted( Smb4KShare * ) ) );

  if ( !hasSubjobs() )
  {
    QApplication::setOverrideCursor( Qt::BusyCursor );
  }
  else
  {
    // Do nothing
  }
  
  addSubjob( job );

  job->start();
}


void Smb4KMounter::openMountDialog( QWidget *parent )
{
  if ( !m_dialog )
  {
    Smb4KShare share;
    
    m_dialog = new Smb4KMountDialog( &share, parent );

    if ( m_dialog->exec() == KDialog::Accepted && m_dialog->validUserInput() )
    {
      // Pass the share to mountShare().
      mountShare( &share, parent );
      
      // Bookmark the share if the user wants this.
      if ( m_dialog->bookmarkShare() )
      {
        Smb4KBookmarkHandler::self()->addBookmark( &share );
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

    delete m_dialog;
    m_dialog = NULL;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::mountShares( const QList<Smb4KShare *> &shares, QWidget *parent )
{
  QListIterator<Smb4KShare *> it( shares );
  QList<Smb4KShare *> shares_to_mount;
  
  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();
    
    // Check that the URL is valid. Otherwise, we can just continue here
    // with an error message.
    if ( !share->url().isValid() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->invalidURLPassed();
      continue;
    }
    else
    {
      // Do nothing
    }

    // Check if the share has already been mounted or a mount
    // is currently in progress.
    QList<Smb4KShare *> mounted_shares;
    QString unc;
    bool mounted = false;

    if ( share->isHomesShare() )
    {
      if ( !Smb4KHomesSharesHandler::self()->specifyUser( share, parent ) )
      {
        continue;
      }
      else
      {
        // Do nothing
      }
    
      unc = share->homeUNC( QUrl::None );
    }
    else
    {
      unc = share->unc( QUrl::None );
    }
    
    mounted_shares = findShareByUNC( unc );

    // Check if it is already mounted:
    for ( int i = 0; i != mounted_shares.size(); ++i )
    {
      if ( !mounted_shares.at( i )->isForeign() )
      {
        mounted = true;
        break;
      }
      else
      {
        continue;
      }
    } 
  
    if ( !mounted )
    {
      QListIterator<KJob *> job_it( subjobs() );
      bool running = false;
    
      while ( job_it.hasNext() )
      {
        KJob *job = job_it.next();
      
        if ( QString::compare( job->objectName(), QString( "MountJob_%1" ).arg( unc ), Qt::CaseInsensitive ) == 0 )
        {
          // Already running
          running = true;
          break;
        }
        else
        {
          continue;
        }
      }
      
      if ( !running )
      {
        Smb4KAuthInfo authInfo( share );
        Smb4KWalletManager::self()->readAuthInfo( &authInfo );
        share->setAuthInfo( &authInfo );
        shares_to_mount << share;
        p->addMount();
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
  
  // Create a new job and add it to the subjobs
  Smb4KMountJob *job = new Smb4KMountJob( this );
  job->setObjectName( QString( "MountJob_bulk-%1" ).arg( shares.size() ) );
  job->setupMount( shares_to_mount, parent );

  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( authError( Smb4KMountJob * ) ), SLOT( slotAuthError( Smb4KMountJob * ) ) );
  connect( job, SIGNAL( retry( Smb4KMountJob * ) ), SLOT( slotRetryMounting( Smb4KMountJob * ) ) );
  connect( job, SIGNAL( aboutToStart( const QList<Smb4KShare> & ) ), SLOT( slotAboutToStartMounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( finished( const QList<Smb4KShare> & ) ), SLOT( slotFinishedMounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( mounted( Smb4KShare * ) ), SLOT( slotShareMounted( Smb4KShare * ) ) );
  
  if ( !hasSubjobs() )
  {
    QApplication::setOverrideCursor( Qt::BusyCursor );
  }
  else
  {
    // Do nothing
  }
  
  addSubjob( job );

  job->start();
}



void Smb4KMounter::unmountShare( Smb4KShare *share, bool force, bool silent, QWidget *parent )
{
  Q_ASSERT( share );

  // Check that the URL is valid. Otherwise, we can just return here
  // with an error message.
  if ( !share->url().isValid() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->invalidURLPassed();
    return;
  }
  else
  {
    // Do nothing
  }
  
  // Check if the unmount process is already in progress.
  QListIterator<KJob *> it( subjobs() );
    
  while ( it.hasNext() )
  {
    KJob *job = it.next();
      
    if ( QString::compare( job->objectName(), QString( "UnmountJob_%1" )
          .arg( QString::fromUtf8( share->canonicalPath() ) ), Qt::CaseInsensitive ) == 0 )
    {
      // Already running
      return;
    }
    else
    {
      continue;
    }
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

    return;
  }
  else
  {
    // Do nothing
  }
  
  // Create a new job and add it to the subjobs
  Smb4KUnmountJob *job = new Smb4KUnmountJob( this );
  job->setObjectName( QString( "UnmountJob_%1" ).arg( QString::fromUtf8( share->canonicalPath() ) ) );
  job->setupUnmount( share, force, silent, parent );

  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( aboutToStart( const QList<Smb4KShare> & ) ), SLOT( slotAboutToStartUnmounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( finished( const QList<Smb4KShare> & ) ), SLOT( slotFinishedUnmounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( unmounted( Smb4KShare * ) ), SLOT( slotShareUnmounted( Smb4KShare * ) ) );

  if ( !hasSubjobs() )
  {
    QApplication::setOverrideCursor( Qt::BusyCursor );
  }
  else
  {
    // Do nothing
  }
  
  addSubjob( job );

  job->start();
}


void Smb4KMounter::unmountShares( const QList<Smb4KShare *> &shares, bool force, bool silent, QWidget *parent )
{
  // Check if an unmount process is already in progress.
  QListIterator<Smb4KShare *> it( shares );
  QList<Smb4KShare *> shares_to_unmount;
  
  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();  
    QListIterator<KJob *> job_it( subjobs() );
    bool found = false;
    
    while ( job_it.hasNext() )
    {
      KJob *job = job_it.next();
      
      if ( QString::compare( job->objectName(), QString( "UnmountJob_%1" )
            .arg( QString::fromUtf8( share->canonicalPath() ) ), Qt::CaseInsensitive ) == 0 )
      {
        found = true;
        return;
      }
      else
      {
        continue;
      }
    }
    
    if ( !found )
    {
      shares_to_unmount << share;
      p->addUnmount();
    }
    else
    {
      // Do nothing
    }
  }
  
  // Create a new job and add it to the subjobs
  Smb4KUnmountJob *job = new Smb4KUnmountJob( this );
  job->setObjectName( QString( "UnmountJob_bulk-%1" ).arg( shares.size() ) );
  job->setupUnmount( shares_to_unmount, force, silent, parent );

  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( aboutToStart( const QList<Smb4KShare> & ) ), SLOT( slotAboutToStartUnmounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( finished( const QList<Smb4KShare> & ) ), SLOT( slotFinishedUnmounting( const QList<Smb4KShare> & ) ) );
  connect( job, SIGNAL( unmounted( Smb4KShare * ) ), SLOT( slotShareUnmounted( Smb4KShare * ) ) );

  if ( !hasSubjobs() )
  {
    QApplication::setOverrideCursor( Qt::BusyCursor );
  }
  else
  {
    // Do nothing
  }
  
  addSubjob( job );

  job->start();  
}


void Smb4KMounter::unmountAllShares( QWidget *parent )
{
  unmountShares( mountedSharesList(), false, false, parent );
}


void Smb4KMounter::start()
{
  // Avoid a race with QApplication and use 50 ms here.
  QTimer::singleShot( 50, this, SLOT( slotStartJobs() ) );
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
  if ( (Smb4KSettings::remountShares() && p->aboutToQuit()) || p->hardwareReason() )
  {
    for ( int i = 0; i < mountedSharesList().size(); ++i )
    {
      if ( !mountedSharesList().at( i )->isForeign() )
      {
        Smb4KCustomOptionsManager::self()->addRemount( mountedSharesList().at( i ) );
      }
      else
      {
        Smb4KCustomOptionsManager::self()->removeRemount( mountedSharesList().at( i ) );
      }
    }
  }
  else
  {
    if ( !Smb4KSettings::remountShares() )
    {
      Smb4KCustomOptionsManager::self()->clearRemounts();
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
      QList<Smb4KShare *> shares;
      
      for ( int i = 0; i < m_retries.size(); ++i )
      {
        shares << &m_retries[i];
      }
      
      mountShares( shares );
      m_retries.clear();
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


void Smb4KMounter::slotStartJobs()
{
  startTimer( TIMEOUT );

  import();

  if ( Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Connected ||
       Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Unknown )
  {
    p->setHardwareReason( false );
    triggerRemounts();
  }
  else
  {
    // Do nothing and wait until the network becomes available.
  }
}


void Smb4KMounter::slotAboutToQuit()
{
  // Tell the application it is about to quit.
  p->setAboutToQuit();

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


void Smb4KMounter::slotJobFinished( KJob *job )
{
  removeSubjob( job );

  if ( !hasSubjobs() )
  {
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotAuthError( Smb4KMountJob *job )
{
  if ( job )
  {
    for ( int i = 0; i < job->authErrors().size(); ++i )
    {
      Smb4KAuthInfo authInfo( &job->authErrors().at( i ) );

      if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, job->parentWidget() ) )
      {
        m_retries << job->authErrors().at( i );
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


void Smb4KMounter::slotRetryMounting( Smb4KMountJob *job )
{
  if ( job )
  {
    for ( int i = 0; i < job->retries().size(); ++i )
    {
      m_retries << job->retries().at( i );
    }
  }
  else
  {
    // Do nothing
  }
}



void Smb4KMounter::slotShareMounted( Smb4KShare *share )
{
  Q_ASSERT( share );

  // Check that the share has not already been entered into the list.
  Smb4KShare *known_share = findShareByPath( share->canonicalPath() );

  if ( !known_share )
  {
    // Copy incoming share, because it will be deleted shortly 
    // after Smb4KMountJob::mounted() signal was emitted.
    known_share = new Smb4KShare( *share );
    
    if ( known_share->isHomesShare() )
    {
      known_share->setURL( share->homeURL() );
    }
    else
    {
      // Do nothing
    }

    // Check the usage, etc.
    check( known_share );

    // Add the share
    addMountedShare( known_share );

    // Check whether the share stems from a bulk mount or not.
    if ( p->pendingMounts() != 0 )
    {
      if ( Smb4KSettings::remountShares() )
      {
        Smb4KCustomOptionsManager::self()->removeRemount( known_share );
      }
      else
      {
        // Do nothing
      }

      p->removeMount();

      if ( p->pendingMounts() == 0 )
      {
        Smb4KNotification *notification = new Smb4KNotification( this );
        notification->sharesMounted( p->initialMounts(), p->initialMounts() );
        p->clearMounts();
      }
      else
      {
        if ( hasSubjobs() )
        {
          bool still_mounting = false;

          QListIterator<KJob *> it( subjobs() );

          while ( it.hasNext() )
          {
            KJob *job = it.next();

            if ( job->objectName().startsWith( "MountJob_bulk" ) )
            {
              still_mounting = true;
              break;
            }
            else
            {
              continue;
            }
          }

          if ( !still_mounting )
          {
            Smb4KNotification *notification = new Smb4KNotification( this );
            notification->sharesMounted( p->initialMounts(), (p->initialMounts() - p->pendingMounts()) );
            p->clearMounts();
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          Smb4KNotification *notification = new Smb4KNotification( this );
          notification->sharesMounted( p->initialMounts(), (p->initialMounts() - p->pendingMounts()) );
          p->clearMounts();
        }
      }
    }
    else
    {
      Smb4KNotification *notification = new Smb4KNotification( this );
      notification->shareMounted( known_share );
    }
    
    // Emit the mounted() signal.
    emit mounted( known_share );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotShareUnmounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  // Get the share that was unmounted.
  Smb4KShare *known_share = findShareByPath( share->canonicalPath() );
  
  if ( known_share )
  {
    // Set the share as unmounted. Since the unmount job
    // works with an internal copy, we have to set this here!
    known_share->setIsMounted( false );
    
    if ( p->pendingUnmounts() != 0 )
    {
      p->removeUnmount();

      if ( p->pendingUnmounts() == 0 )
      {
        Smb4KNotification *notification = new Smb4KNotification( this );
        notification->allSharesUnmounted( p->initialUnmounts(), p->initialUnmounts() );
        p->clearUnmounts();
      }
      else
      {
        if ( hasSubjobs() )
        {
          bool still_unmounting = false;
              
          QListIterator<KJob *> it( subjobs() );
              
          while ( it.hasNext() )
          {
            KJob *job = it.next();
                
            if ( job->objectName().startsWith( "UnmountJob_bulk" ) )
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
            notification->allSharesUnmounted( p->initialUnmounts(), (p->initialUnmounts() - p->pendingUnmounts()) );
            p->clearUnmounts();
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          Smb4KNotification *notification = new Smb4KNotification( this );
          notification->allSharesUnmounted( p->initialUnmounts(), (p->initialUnmounts() - p->pendingUnmounts()) );
          p->clearUnmounts();
        }
      }
    }
    else
    {
      Smb4KNotification *notification = new Smb4KNotification( this );
      notification->shareUnmounted( known_share );
    }
    
    // Clean up the mount prefix.
    if ( qstrncmp( known_share->canonicalPath(),
         QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8(),
         QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8().length() ) == 0 )
    {
      QDir dir( QString::fromUtf8( known_share->canonicalPath() ) );

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

    // Emit the unmounted() signal and remove the share from the
    // list of mounted shares.
    emit unmounted( known_share );
    removeMountedShare( known_share );
  }
  else
  {
    // Do nothing
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
        p->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        p->setHardwareReason( false );
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
        p->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        p->setHardwareReason( false );
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
        p->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        p->setHardwareReason( false );
      }
      else
      {
        // Do nothing
      }
      break;
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
      p->setHardwareReason( true );
      triggerRemounts();
      p->setHardwareReason( false );
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
      p->setHardwareReason( true );
      triggerRemounts();
      p->setHardwareReason( false );
      break;
    }
    case Smb4KSolidInterface::Disconnected:
    {
      p->setHardwareReason( true );
      abortAll();
      saveSharesForRemount();
      unmountAllShares();
      p->setHardwareReason( false );
      break;
    }
    case Smb4KSolidInterface::Unknown:
    {
      p->setHardwareReason( true );
      triggerRemounts();
      p->setHardwareReason( false );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KMounter::slotAboutToStartMounting( const QList<Smb4KShare> &shares )
{
  QList<Smb4KShare> list = shares;
  
  for ( int i = 0; i < list.size(); ++i )
  {
    emit aboutToStart( &list[i], MountShare );
  }
}


void Smb4KMounter::slotFinishedMounting( const QList<Smb4KShare> &shares )
{
  QList<Smb4KShare> list = shares;
  
  for ( int i = 0; i < list.size(); ++i )
  {
    emit finished( &list[i], MountShare );
  }
}


void Smb4KMounter::slotAboutToStartUnmounting( const QList<Smb4KShare> &shares )
{
  QList<Smb4KShare> list = shares;
  
  for ( int i = 0; i < list.size(); ++i )
  {
    emit aboutToStart( &list[i], UnmountShare );
  }
}


void Smb4KMounter::slotFinishedUnmounting( const QList<Smb4KShare> &shares )
{
  QList<Smb4KShare> list = shares;
  
  for ( int i = 0; i < list.size(); ++i )
  {
    emit finished( &list[i], UnmountShare );
  }
}

#include "smb4kmounter.moc"
