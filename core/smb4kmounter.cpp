/***************************************************************************
    smb4kmounter.cpp  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2013 by Alexander Reinholdt
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

// Application specific includes
#include "smb4kmounter.h"
#include "smb4kmounter_p.h"
#include "smb4kauthinfo.h"
#include "smb4kglobal.h"
#include "smb4kshare.h"
#include "smb4ksettings.h"
#include "smb4khomesshareshandler.h"
#include "smb4kwalletmanager.h"
#include "smb4kprocess.h"
#include "smb4knotification.h"
#include "smb4kbookmarkhandler.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kbookmark.h"
#include "smb4kprofilemanager.h"

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtTest/QTest>
#include <QtNetwork/QUdpSocket>

// KDE includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kmountpoint.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kdiskfreespaceinfo.h>

using namespace Smb4KGlobal;

#define TIMEOUT 50

K_GLOBAL_STATIC( Smb4KMounterStatic, p );



Smb4KMounter::Smb4KMounter( QObject *parent )
: KCompositeJob( parent ), d( new Smb4KMounterPrivate )
{
  setAutoDelete( false );

  if ( !coreIsInitialized() )
  {
    setDefaultSettings();
  }
  else
  {
    // Do nothing
  }

  d->importTimeout = 0;
  d->remountTimeout = 0;
  d->remountAttempts = 0;
  d->checks = 0;
  d->dialog = 0;
  d->aboutToQuit = false;
  d->firstImportDone = false;
  d->importsAllowed = true;

  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
          this,                         SLOT(slotAboutToQuit()));

  connect(Smb4KSolidInterface::self(),  SIGNAL(buttonPressed(Smb4KSolidInterface::ButtonType)),
          this,                         SLOT(slotHardwareButtonPressed(Smb4KSolidInterface::ButtonType)));

  connect(Smb4KSolidInterface::self(),  SIGNAL(wokeUp()),
          this,                         SLOT(slotComputerWokeUp()));

  connect(Smb4KSolidInterface::self(),  SIGNAL(networkStatusChanged(Smb4KSolidInterface::ConnectionStatus)),
          this,                         SLOT(slotNetworkStatusChanged(Smb4KSolidInterface::ConnectionStatus)));
  
  connect(Smb4KProfileManager::self(),  SIGNAL(settingsChanged()),
          this,                         SLOT(slotProfileSettingsChanged()));
}


Smb4KMounter::~Smb4KMounter()
{
  while ( !d->importedShares.isEmpty() )
  {
    delete d->importedShares.takeFirst();
  }

  while ( !d->retries.isEmpty() )
  {
    delete d->retries.takeFirst();
  }
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
    else if ( QString::compare( job->objectName(), QString( "UnmountJob_%1" ).arg( share->canonicalPath() ), Qt::CaseInsensitive ) == 0 )
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
  if ( !QCoreApplication::closingDown() )
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


bool Smb4KMounter::isRunning()
{
  return hasSubjobs();
}


void Smb4KMounter::triggerRemounts( bool fill_list )
{
  if ( Smb4KSettings::remountShares() /* one-time remounts */ || 
       !Smb4KCustomOptionsManager::self()->sharesToRemount().isEmpty() /* permanent remounts */ || 
       d->hardwareReason /* hardware reason */ )
  {
    if ( fill_list )
    {
      // Get the shares that are to be remounted
      QList<Smb4KCustomOptions *> list = Smb4KCustomOptionsManager::self()->sharesToRemount();

      if ( !list.isEmpty() )
      {
        // Check which ones actually need to be remounted.
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
              Smb4KShare *share = new Smb4KShare();
              share->setURL( list.at( i )->url() );
              share->setWorkgroupName( list.at( i )->workgroupName() );
              share->setHostIP( list.at( i )->ip() );

              if ( !share->url().isEmpty() )
              {
                d->remounts << share;
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
            Smb4KShare *share = new Smb4KShare();
            share->setURL( list.at( i )->url() );
            share->setWorkgroupName( list.at( i )->workgroupName() );
            share->setHostIP( list.at( i )->ip() );

            if ( !share->url().isEmpty() )
            {
              d->remounts << share;
            }
            else
            {
              // Do nothing
            }
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
    
    if ( !d->remounts.isEmpty() )
    {
      mountShares( d->remounts );
    
      // Wait until done.
      while ( hasSubjobs() )
      {
        QTest::qWait( TIMEOUT );
      }
    }
    else
    {
      // Do nothing
    }

    d->remountAttempts++;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::import( bool check_inaccessible )
{
  KMountPoint::List mount_points = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );

  for ( int i = 0; i < mount_points.size(); ++i )
  {
#ifndef Q_OS_FREEBSD
    if ( QString::compare( mount_points.at( i )->mountType(), "cifs" ) == 0 )
#else
    if ( QString::compare( mount_points.at( i )->mountType(), "smbfs" ) == 0 )
#endif
    {
      Smb4KShare *share = new Smb4KShare( mount_points.at( i )->mountedFrom() );
      share->setPath( mount_points.at( i )->mountPoint() );

#ifndef Q_OS_FREEBSD
      share->setFileSystem( Smb4KShare::CIFS );

      // Check if the share is new and we have to open /proc/mounts (if it exists)
      // to acquire all needed information.
      if ( findShareByPath( mount_points.at( i )->mountPoint() ) == NULL && QFile::exists( "/proc/mounts" ) )
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
          Smb4KNotification::openingFileFailed(proc_mounts);
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
                share->setWorkgroupName( tmp.section( ',', 0, 0 ) );
              }
              else
              {
                // The domain entry is at the end of the options string.
                share->setWorkgroupName( tmp );
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
                share->setHostIP( tmp.section( ',', 0, 0 ) );
              }
              else
              {
                // The IP address entry is at the end of the options string.
                share->setHostIP( tmp );
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
                QString user = tmp.section( ',', 0, 0 );
                share->setLogin( user.isEmpty() ? "guest" : user );
              }
              else
              {
                // The user name entry is at the end of the options string.
                share->setLogin( tmp.isEmpty() ? "guest" : tmp );
              }
            }
            else if ( mount_options.contains( "user=" ) )
            {
              QString tmp = mount_options.section( "user=", 1, 1 );

              if ( tmp.contains( "," ) )
              {
                // The user name entry is somewhere in the middle of the options
                // string.
                QString user = tmp.section( ',', 0, 0 );
                share->setLogin( user.isEmpty() ? "guest" : user );
              }
              else
              {
                // The user name entry is at the end of the options string.
                share->setLogin( tmp.isEmpty() ? "guest" : tmp );
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
        QString login = mount_points.at( i )->mountOptions().join( "," ).section( "user=", 1, 1 ).section( ',', 0, 0 ).trimmed();
        share->setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'user=' entries
      }
#else
      share->setFileSystem( Smb4KShare::SMBFS );

      // Try to get the login from the mount options.
      if ( share->login().isEmpty() )
      {
        QString login = mount_points.at( i )->mountOptions().join( "," ).section( "username=", 1, 1 ).section( ',', 0, 0 ).trimmed();
        share->setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'username=' entries
      }
      else
      {
        // Do nothing
      }
//       qDebug() << "Domain and ip address?";
#endif
      share->setIsMounted( true );
      d->importedShares << share;

      // Do not delete the share here.
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
    for ( int j = 0; j < d->importedShares.size(); ++j )
    {
      // Check the mount point, since that is unique. We will
      // only use Smb4KShare::path(), so that we do not run into
      // trouble if a share is inaccessible.
      if ( QString::compare( mountedSharesList().at( i )->path(), d->importedShares.at( j )->path() ) == 0 )
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
      emit mountedSharesListChanged();
    }
    else
    {
      // Do nothing
    }

    found = false;
  }
  
#ifndef Q_OS_FREEBSD
  // Now stat the imported shares to get information about them.
  // Do not use Smb4KShare::canonicalPath() here, otherwise we might
  // get lock-ups with inaccessible shares.
  if ( Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Connected )
  {
    for ( int i = 0; i < d->importedShares.size(); ++i )
    {
      // Check if the share is inaccessible and should be checked.
      Smb4KShare *share = findShareByPath( d->importedShares.at( i )->path() );

      if ( share )
      {
        if ( share->isInaccessible() && !check_inaccessible )
        {
          continue;
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
      
      KUrl url;
      url.setPath( d->importedShares.at( i )->path() );
      KIO::StatJob *job = KIO::stat( url, KIO::HideProgressInfo );
      job->setDetails( 0 );
      connect( job, SIGNAL(result(KJob*)), SLOT(slotStatResult(KJob*)) );

      // Do not use addSubJob(), because that would confuse isRunning, etc.
      job->start();
    }

    if ( !d->firstImportDone && d->importedShares.isEmpty() )
    {
      d->firstImportDone = true;
    }
    else
    {
      // Do nothing. d->firstImportDone will be set in slotStatResult().
    }
  }
  else
  {
    // Do nothing
  }
#else
  // FIXME: Remove this as soon as Solid's network module supports FreeBSD.
  for ( int i = 0; i < d->importedShares.size(); ++i )
  {
    // Check if the share is inaccessible and should be checked.
    Smb4KShare *share = findShareByPath( d->importedShares.at( i )->path() );

    if ( share )
    {
      if ( share->isInaccessible() && !check_inaccessible )
      {
        continue;
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
     
    KUrl url;
    url.setPath( d->importedShares.at( i )->path() );
    KIO::StatJob *job = KIO::stat( url, KIO::HideProgressInfo );
    job->setDetails( 0 );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotStatResult(KJob*)) );

    // Do not use addSubJob(), because that would confuse isRunning, etc.
    job->start();
  }

  if ( !d->firstImportDone && d->importedShares.isEmpty() )
  {
    d->firstImportDone = true;
  }
  else
  {
    // Do nothing. d->firstImportDone will be set in slotStatResult().
  }
#endif
}


void Smb4KMounter::mountShare( Smb4KShare *share, QWidget *parent )
{
  Q_ASSERT( share );

  // Check that the URL is valid. Otherwise, we can just return here
  // with an error message.
  if ( !share->url().isValid() )
  {
    Smb4KNotification::invalidURLPassed();
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
    if ( !Smb4KHomesSharesHandler::self()->specifyUser( share, true, parent ) )
    {
      return;
    }
    else
    {
      // Do nothing
    }
    
    unc = share->homeUNC();
  }
  else
  {
    unc = share->unc();
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
  
  // Check if the host should be woken up before we start mounting the
  // share.
  if ( Smb4KSettings::enableWakeOnLAN() )
  {
    Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions( share->url().upUrl() );
  
    if ( options && options->wolSendBeforeMount() )
    {
      emit aboutToStart( share, WakeUp );
      
      QUdpSocket *socket = new QUdpSocket( this );
      QHostAddress addr;
      
      // Use the host's IP address directly from the share object.
      if ( !share->hostIP().isEmpty() )
      {
        addr.setAddress( share->hostIP() );
      }
      else
      {
        addr.setAddress( "255.255.255.255" );
      }
          
      // Construct magic sequence
      QByteArray sequence;

      // 6 times 0xFF
      for ( int j = 0; j < 6; ++j )
      {
        sequence.append( QChar( 0xFF ).toAscii() );
      }
          
      // 16 times the MAC address
      QStringList parts = options->macAddress().split( ':', QString::SkipEmptyParts );
          
      for ( int j = 0; j < 16; ++j )
      {
        for ( int k = 0; k < parts.size(); ++k )
        {
          sequence.append( QChar( QString( "0x%1" ).arg( parts.at( k ) ).toInt( 0, 16 ) ).toAscii() );
        }
      }
          
      socket->writeDatagram( sequence, addr, 9 );
      
      delete socket;
      
      // Wait the defined time
      int stop = 1000 * Smb4KSettings::wakeOnLANWaitingTime() / 250;
      int i = 0;
      
      while ( i++ < stop )
      {
        QTest::qWait( 250 );
      }
      
      emit finished( share, WakeUp );
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
  
  // Get the authentication information
  Smb4KWalletManager::self()->readAuthInfo( share );
  
  // Create a new job and add it to the subjobs
  Smb4KMountJob *job = new Smb4KMountJob( this );
  job->setObjectName( QString( "MountJob_%1" ).arg( unc ) );
  job->setupMount( share, parent );
  
  connect( job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)) );
  connect( job, SIGNAL(authError(Smb4KMountJob*)), SLOT(slotAuthError(Smb4KMountJob*)) );
  connect( job, SIGNAL(retry(Smb4KMountJob*)), SLOT(slotRetryMounting(Smb4KMountJob*)) );
  connect( job, SIGNAL(aboutToStart(QList<Smb4KShare*>)), SLOT(slotAboutToStartMounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(finished(QList<Smb4KShare*>)), SLOT(slotFinishedMounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(mounted(Smb4KShare*)), SLOT(slotShareMounted(Smb4KShare*)) );

  if ( !hasSubjobs() && modifyCursor() )
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
  if ( !d->dialog )
  {
    Smb4KShare *share = new Smb4KShare();
    
    d->dialog = new Smb4KMountDialog( share, parent );

    if ( d->dialog->exec() == KDialog::Accepted && d->dialog->validUserInput() )
    {
      // Pass the share to mountShare().
      mountShare( share, parent );
      
      // Bookmark the share if the user wants this.
      if ( d->dialog->bookmarkShare() )
      {
        Smb4KBookmarkHandler::self()->addBookmark( share );
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

    delete d->dialog;
    d->dialog = NULL;

    delete share;
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
      Smb4KNotification::invalidURLPassed();
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
      if ( !Smb4KHomesSharesHandler::self()->specifyUser( share, false, parent ) )
      {
        continue;
      }
      else
      {
        // Do nothing
      }
    
      unc = share->homeUNC();
    }
    else
    {
      unc = share->unc();
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
          // Make sure that we do not get false rejections when mounting is
          // retried due to an authentication error.
          if ( static_cast<Smb4KMountJob *>( job )->authErrors().isEmpty() )
          {
            running = true;
            break;
          }
          else
          {
            continue;
          }
        }
        else
        {
          continue;
        }
      }
      
      if ( !running )
      {
        Smb4KWalletManager::self()->readAuthInfo( share );
        shares_to_mount << share;
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
  job->setObjectName( QString( "MountJob_bulk-%1" ).arg( shares_to_mount.size() ) );
  job->setupMount( shares_to_mount, parent );

  connect( job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)) );
  connect( job, SIGNAL(authError(Smb4KMountJob*)), SLOT(slotAuthError(Smb4KMountJob*)) );
  connect( job, SIGNAL(retry(Smb4KMountJob*)), SLOT(slotRetryMounting(Smb4KMountJob*)) );
  connect( job, SIGNAL(aboutToStart(QList<Smb4KShare*>)), SLOT(slotAboutToStartMounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(finished(QList<Smb4KShare*>)), SLOT(slotFinishedMounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(mounted(Smb4KShare*)), SLOT(slotShareMounted(Smb4KShare*)) );
  
  if ( !hasSubjobs() && modifyCursor() )
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



void Smb4KMounter::unmountShare( Smb4KShare *share, bool silent, QWidget *parent )
{
  Q_ASSERT( share );

  // Check that the URL is valid. Otherwise, we can just return here
  // with an error message.
  if ( !share->url().isValid() )
  {
    Smb4KNotification::invalidURLPassed();
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
      
    if ( QString::compare( job->objectName(), QString( "UnmountJob_%1" ).arg( share->canonicalPath() ), Qt::CaseInsensitive ) == 0 )
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
  if ( share->isForeign() )
  {
    if ( !Smb4KSettings::unmountForeignShares() )
    {
      if ( !silent )
      {
        Smb4KNotification::unmountingNotAllowed(share);
      }
      else
      {
        // Do nothing
      }
      return;
    }
    else
    {
      if ( !silent )
      {
        if ( KMessageBox::warningYesNo( parent,
             i18n( "<qt><p>The share <b>%1</b> is mounted to <br><b>%2</b> and owned by user <b>%3</b>.</p>"
                   "<p>Do you really want to unmount it?</p></qt>",
                   share->unc(), share->path(), share->owner() ),
             i18n( "Foreign Share" ) ) == KMessageBox::No )
        {
          return;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Without the confirmation of the user, we are not
        // unmounting a foreign share!
        return;
      }
    }
  }
  else
  {
    // Do nothing
  }

  // Ask the user if he/she really wants to forcibly unmount an 
  // inaccessible share (Linux only)
  bool force = false;

#ifdef Q_OS_LINUX
  if ( share->isInaccessible() )
  {
    force = Smb4KSettings::forceUnmountInaccessible();
  }
  else
  {
    // Do nothing
  }
#endif
  
  // Create a new job and add it to the subjobs
  Smb4KUnmountJob *job = new Smb4KUnmountJob( this );
  job->setObjectName( QString( "UnmountJob_%1" ).arg( share->canonicalPath() ) );
  job->setupUnmount( share, force, silent, (d->aboutToQuit || d->hardwareReason), parent );

  connect( job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)) );
  connect( job, SIGNAL(aboutToStart(QList<Smb4KShare*>)), SLOT(slotAboutToStartUnmounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(finished(QList<Smb4KShare*>)), SLOT(slotFinishedUnmounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(unmounted(Smb4KShare*)), SLOT(slotShareUnmounted(Smb4KShare*)) );

  if ( !hasSubjobs() && modifyCursor() )
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


void Smb4KMounter::unmountShares( const QList<Smb4KShare *> &shares, bool silent, QWidget *parent )
{
  // Check if an unmount process is already in progress.
  QListIterator<Smb4KShare *> it( shares );
  QList<Smb4KShare *> shares_to_unmount;
  bool have_inaccessible_shares = false;
  
  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();

    if ( !have_inaccessible_shares && share->isInaccessible() )
    {
      have_inaccessible_shares = true;
    }
    else
    {
      // Do nothing
    }
    
    QListIterator<KJob *> job_it( subjobs() );
    bool found = false;
    
    while ( job_it.hasNext() )
    {
      KJob *job = job_it.next();
      
      if ( QString::compare( job->objectName(), QString( "UnmountJob_%1" ).arg( share->canonicalPath() ), Qt::CaseInsensitive ) == 0 )
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
      // Complain if the share is a foreign one and unmounting those
      // is prohibited.
      if ( share->isForeign() )
      {
        if ( !Smb4KSettings::unmountForeignShares() )
        {
          if ( !silent )
          {
            Smb4KNotification::unmountingNotAllowed(share);
          }
          else
          {
            // Do nothing
          }
          continue;
        }
        else
        {
          if ( !silent )
          {
            if ( KMessageBox::warningYesNo( parent,
                i18n( "<qt><p>The share <b>%1</b> is mounted to <br><b>%2</b> and owned by user <b>%3</b>.</p>"
                      "<p>Do you really want to unmount it?</p></qt>",
                      share->unc(), share->path(), share->owner() ),
                i18n( "Foreign Share" ) ) == KMessageBox::No )
            {
              continue;
            }
            else
            {
              // Do nothing
            }
          }
          else
          {
            if ( d->aboutToQuit )
            {
              continue;
            }
            else
            {
              // Do nothing
            }
          }
        }
      }
      else
      {
        // Do nothing
      }

      shares_to_unmount << share;
    }
    else
    {
      // Do nothing
    }
  }

  // Ask the user if he/she really wants to forcibly unmount an
  // inaccessible share
  bool force = false;

#ifdef Q_OS_LINUX
  if ( have_inaccessible_shares )
  {
    force = Smb4KSettings::forceUnmountInaccessible();
  }
  else
  {
    // Do nothing
  }
#endif
  
  // Create a new job and add it to the subjobs
  Smb4KUnmountJob *job = new Smb4KUnmountJob( this );
  job->setObjectName( QString( "UnmountJob_bulk-%1" ).arg( shares.size() ) );
  job->setupUnmount( shares_to_unmount, force, silent, (d->aboutToQuit || d->hardwareReason), parent );
    
  connect( job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)) );
  connect( job, SIGNAL(aboutToStart(QList<Smb4KShare*>)), SLOT(slotAboutToStartUnmounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(finished(QList<Smb4KShare*>)), SLOT(slotFinishedUnmounting(QList<Smb4KShare*>)) );
  connect( job, SIGNAL(unmounted(Smb4KShare*)), SLOT(slotShareUnmounted(Smb4KShare*)) );

  if ( !hasSubjobs() && modifyCursor() )
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
  if ( !d->aboutToQuit )
  {
    unmountShares( mountedSharesList(), false, parent );
  }
  else
  {
    unmountShares( mountedSharesList(), true, parent );
  }
}


void Smb4KMounter::start()
{
  // Avoid a race with QApplication and use 50 ms here.
  QTimer::singleShot( 50, this, SLOT(slotStartJobs()) );
}


void Smb4KMounter::check( Smb4KShare *share )
{
  // Get the info about the usage, etc.
  KDiskFreeSpaceInfo space_info = KDiskFreeSpaceInfo::freeSpaceInfo( share->path() );

  if ( space_info.isValid() )
  {
    share->setInaccessible( false );
    share->setFreeDiskSpace( space_info.available() );
    share->setTotalDiskSpace( space_info.size() );
    share->setUsedDiskSpace( space_info.used() );
      
    // Get the owner an group, if possible.
    QFileInfo file_info( share->path() );
    file_info.setCaching( false );

    if ( file_info.exists() )
    {
      share->setUID( (K_UID)file_info.ownerId() );
      share->setGID( (K_GID)file_info.groupId() );
      share->setInaccessible( !(file_info.isDir() && file_info.isExecutable()) );
    }
    else
    {
      share->setInaccessible( true );
      share->setFreeDiskSpace( 0 );
      share->setTotalDiskSpace( 0 );
      share->setUsedDiskSpace( 0 );
      share->setUID( (K_UID)-1 );
      share->setGID( (K_GID)-1 );
    }
  }
  else
  {
    share->setInaccessible( true );
    share->setFreeDiskSpace( 0 );
    share->setTotalDiskSpace( 0 );
    share->setUsedDiskSpace( 0 );
    share->setUID( (K_UID)-1 );
    share->setGID( (K_GID)-1 );
  } 
}


void Smb4KMounter::saveSharesForRemount()
{
  if ( (Smb4KSettings::remountShares() && d->aboutToQuit) || d->hardwareReason )
  {
    // Save currently mounted shares.
    for ( int i = 0; i < mountedSharesList().size(); ++i )
    {
      if ( !mountedSharesList().at( i )->isForeign() )
      {
        Smb4KCustomOptionsManager::self()->addRemount( mountedSharesList().at( i ), false );
      }
      else
      {
        Smb4KCustomOptionsManager::self()->removeRemount( mountedSharesList().at( i ), false );
      }
    }

    // Save failed remounts.
    for ( int i = 0; i < d->remounts.size(); ++i )
    {
      Smb4KCustomOptionsManager::self()->addRemount( d->remounts.at( i ), false );
    }
  }
  else
  {
    if ( !Smb4KSettings::remountShares() )
    {
      Smb4KCustomOptionsManager::self()->clearRemounts( false );
    }
    else
    {
      // Do nothing
    }
  }

  while ( !d->remounts.isEmpty() )
  {
    delete d->remounts.takeFirst();
  }
}


void Smb4KMounter::cleanup()
{
  if ( !d->obsoleteMountpoints.isEmpty() )
  {
    while ( !d->obsoleteMountpoints.isEmpty() )
    {
      QString path = d->obsoleteMountpoints.takeFirst();
      
      if ( path.startsWith( Smb4KSettings::mountPrefix().path() ) )
      {
        QDir dir( path );

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
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::timerEvent( QTimerEvent * )
{
  // Import newly mounted shares. This is only done when imports
  // are allowed (no auth error occurred and a password dialog is
  // open at the moment) and there are currently no subjobs.
  if ( d->importTimeout >= Smb4KSettings::checkInterval() && d->importedShares.isEmpty() )
  {
    if ( d->importsAllowed && !hasSubjobs() )
    {
      if ( d->checks == 10 )
      {
        import( true );
        d->checks = 0;
      }
      else
      {
        import( false );
        d->checks += 1;
      }
        
      d->importTimeout = -TIMEOUT;
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
  
  d->importTimeout += TIMEOUT;
  
  // Try to remount those shares that could not be mounted 
  // before. Do this only if there are no subjobs, because we
  // do not want to get crashes because a share was invalidated
  // during processing the shares.
  if ( (Smb4KSettings::remountShares() || !Smb4KCustomOptionsManager::self()->sharesToRemount().isEmpty()) && 
       Smb4KSettings::remountAttempts() > d->remountAttempts )
  {
    // Inhibit automatic sleeping.
    int cookie = Smb4KSolidInterface::self()->beginSleepSuppression(i18n("Remounting shares. Please wait."));
    
    if ( d->firstImportDone && !hasSubjobs() )
    {
      if ( d->remountAttempts == 0 )
      {
        triggerRemounts(true);
      }
      else if ( !d->remounts.isEmpty() &&
                d->remountTimeout >= (60000 * Smb4KSettings::remountInterval()) )
      {
        triggerRemounts(false);
        d->remountTimeout = -TIMEOUT;
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
    
    d->remountTimeout += TIMEOUT;
    
    // Allow automatic sleeping.
    Smb4KSolidInterface::self()->endSleepSuppression(cookie);
  }
  else
  {
    // Do nothing
  }
  
  // Retry mounting those shares that failed. This is also only 
  // done when there are no subjobs.
  if ( !d->retries.isEmpty() && !hasSubjobs() )
  {
    // Inhibit automatic sleeping.
    int cookie = Smb4KSolidInterface::self()->beginSleepSuppression(i18n("Remounting shares. Please wait."));
    
    mountShares( d->retries );
      
    while ( !d->retries.isEmpty() )
    {
      delete d->retries.takeFirst();
    }
    
    // Allow automatic sleeping.
    Smb4KSolidInterface::self()->endSleepSuppression(cookie);
  }
  else
  {
    // Do nothing
  }
  
  // Finally, clean up the mount prefix.
  cleanup();
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KMounter::slotStartJobs()
{
  import( true );

  if ( Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Connected ||
       Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::UnknownStatus )
  {
    d->hardwareReason = false;
  }
  else
  {
    // Do nothing and wait until the network becomes available.
  }

  d->timerId = startTimer( TIMEOUT );
}


void Smb4KMounter::slotAboutToQuit()
{
  // Tell the application it is about to quit.
  d->aboutToQuit = true;

  // Abort any actions.
  abortAll();

  // Save the shares that need to be remounted.
  saveSharesForRemount();

  // Unmount the shares if the user chose to do so.
  if ( Smb4KSettings::unmountSharesOnExit() )
  {
    unmountAllShares();
    
    // Wait until done.
    while ( hasSubjobs() )
    {
      QTest::qWait( TIMEOUT );
    }
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

  if ( !hasSubjobs() && modifyCursor() )
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
  // Do not allow imports when an authentication error 
  // occurred. We do not want to operate on a network
  // item that might get invalidated between imports.
  d->importsAllowed = false;
  
  if ( job )
  {
    for ( int i = 0; i < job->authErrors().size(); ++i )
    {
      if ( Smb4KWalletManager::self()->showPasswordDialog( job->authErrors()[i], job->parentWidget() ) )
      {
        d->retries << new Smb4KShare( *job->authErrors().at( i ) );
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
  
  d->importsAllowed = true;
}


void Smb4KMounter::slotRetryMounting( Smb4KMountJob *job )
{
  if ( job )
  {
    for ( int i = 0; i < job->retries().size(); ++i )
    {
      d->retries << new Smb4KShare( *job->retries().at( i ) );
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

  // Remove the share from the list of shares that are to be remounted.
  QMutableListIterator<Smb4KShare *> s( d->remounts );

  while ( s.hasNext() )
  {
    Smb4KShare *remount = s.next();

    if ( !share->isForeign() && QString::compare( remount->unc(), share->unc(), Qt::CaseInsensitive ) == 0 )
    {
      s.remove();
      break;
    }
    else
    {
      continue;
    }
  }
  
  // Check that the share has not already been entered into the list.
  Smb4KShare *known_share = findShareByPath( share->canonicalPath() );

  if ( !known_share )
  {
    // Copy incoming share, because it will be deleted shortly
    // after Smb4KMountJob::mounted() signal was emitted.
    known_share = new Smb4KShare( *share );

    // Check the usage, etc.
    check( known_share );

    // Add the share
    addMountedShare( known_share );

    if ( Smb4KSettings::remountShares() )
    {
      Smb4KCustomOptionsManager::self()->removeRemount( known_share, false );
    }
    else
    {
      // Do nothing
    }

    // Emit the mounted() signal.
    emit mounted( known_share );
    emit mountedSharesListChanged();
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

    // Emit the unmounted() signal. We do it here, because if we do it
    // after the mount prefix was cleaned up, Smb4KShare::canonicalPath()
    // would return an empty string.
    emit unmounted( known_share );

    // Schedule the obsolete mountpoint for removal.
    d->obsoleteMountpoints << known_share->canonicalPath();

    // Remove the share from the list of mounted shares.
    removeMountedShare( known_share );
    emit mountedSharesListChanged();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotHardwareButtonPressed( Smb4KSolidInterface::ButtonType type )
{
  int cookie = Smb4KSolidInterface::self()->beginSleepSuppression(i18n("Unmounting shares. Please wait."));
  d->hardwareReason = true;
  
  switch ( type )
  {
    case Smb4KSolidInterface::SleepButton:
    {
      if (Smb4KSettings::unmountWhenSleepButtonPressed() && !mountedSharesList().isEmpty())
      {
        abortAll();
        saveSharesForRemount();
        unmountAllShares();

        // Wait until done
        while ( hasSubjobs() )
        {
          QTest::qWait( TIMEOUT );
        }
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Smb4KSolidInterface::LidButton:
    {
      if (Smb4KSettings::unmountWhenLidButtonPressed() && !mountedSharesList().isEmpty())
      {
        abortAll();
        saveSharesForRemount();
        unmountAllShares();

        // Wait until done
        while ( hasSubjobs() )
        {
          QTest::qWait( TIMEOUT );
        }
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Smb4KSolidInterface::PowerButton:
    {
      if (Smb4KSettings::unmountWhenPowerButtonPressed() && !mountedSharesList().isEmpty())
      {
        abortAll();
        saveSharesForRemount();
        unmountAllShares();

        // Wait until done
        while ( hasSubjobs() )
        {
          QTest::qWait( TIMEOUT );
        }
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
  
  d->hardwareReason = false;
  Smb4KSolidInterface::self()->endSleepSuppression(cookie);
  qDebug() << "Good night!";
}


void Smb4KMounter::slotComputerWokeUp()
{
}


void Smb4KMounter::slotNetworkStatusChanged( Smb4KSolidInterface::ConnectionStatus status )
{
  switch ( status )
  {
    case Smb4KSolidInterface::Disconnected:
    {
      d->hardwareReason = true;
      abortAll();
      saveSharesForRemount();
      unmountAllShares();

      // Wait until done
      while ( hasSubjobs() )
      {
        QTest::qWait( TIMEOUT );
      }
      
      d->hardwareReason = false;
      break;
    }
    default:
    {
      d->hardwareReason = true;
      triggerRemounts( true );
      d->hardwareReason = false;
      break;
    }
  }
}


void Smb4KMounter::slotAboutToStartMounting( const QList<Smb4KShare *> &shares )
{
  for ( int i = 0; i < shares.size(); ++i )
  {
    emit aboutToStart( shares[i], MountShare );
  }
}


void Smb4KMounter::slotFinishedMounting( const QList<Smb4KShare *> &shares )
{
  // Emit finished() signal and count the number of
  // failed mounts.
  int failed_mounts = 0;
  
  for ( int i = 0; i < shares.size(); ++i )
  {
    emit finished( shares[i], MountShare );

    if ( !shares.at( i )->isMounted() )
    {
      failed_mounts++;
    }
    else
    {
      // Do nothing
    }
  }

  // Notify the user.
  if ( shares.size() - failed_mounts != 0 )
  {
    if ( shares.size() > 1 )
    {
      Smb4KNotification::sharesMounted(shares.size(), (shares.size() - failed_mounts));
    }
    else
    {
      Smb4KNotification::shareMounted(shares.first());
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotAboutToStartUnmounting( const QList<Smb4KShare *> &shares )
{
  for ( int i = 0; i < shares.size(); ++i )
  {
    emit aboutToStart( shares[i], UnmountShare );
  }
}


void Smb4KMounter::slotFinishedUnmounting( const QList<Smb4KShare *> &shares )
{
  // Emit finished() signal and count the number of
  // failed mounts.
  int failed_unmounts = 0;
  
  for ( int i = 0; i < shares.size(); ++i )
  {
    emit finished( shares[i], UnmountShare );

    if ( shares.at( i )->isMounted() )
    {
      failed_unmounts++;
    }
    else
    {
      // Do nothing
    }
  }

  if ( shares.size() - failed_unmounts != 0 )
  {
    if ( shares.size() > 1 )
    {
      Smb4KNotification::sharesUnmounted(shares.size(), (shares.size() - failed_unmounts));
    }
    else
    {
      Smb4KNotification::shareUnmounted(shares.first());
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotStatResult( KJob *job )
{
  Q_ASSERT( job );

  KIO::StatJob *stat = static_cast<KIO::StatJob *>( job );
  QString path = stat->url().pathOrUrl();
  
  Smb4KShare *share = NULL;
  
  for ( int i = 0; i < d->importedShares.size(); ++i )
  {
    if ( QString::compare( d->importedShares.at( i )->path(), path ) == 0 )
    {
      share = d->importedShares.takeAt( i );
      break;
    }
    else
    {
      continue;
    }
  }

  if ( share )
  {
    if ( stat->error() == 0 /* no error */ )
    {
      // We do not use KIO::StatJob::statResult(), because its
      // information is limited.
      check( share );
    }
    else
    {
      share->setInaccessible( true );
      share->setFreeDiskSpace( 0 );
      share->setTotalDiskSpace( 0 );
      share->setUsedDiskSpace( 0 );
      share->setUID( (K_UID)-1 );
      share->setGID( (K_GID)-1 );
    }

    // Is this a mount that was done by the user or by
    // someone else (or the system)?
    if ( (share->uid() == getuid() && share->gid() == getgid()) ||
         (share->path().startsWith( Smb4KSettings::mountPrefix().path() ) || share->path().startsWith( QDir::homePath() )) ||
         (share->canonicalPath().startsWith( QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath() ) ||
         share->canonicalPath().startsWith( QDir::home().canonicalPath() )) )
    {
      share->setForeign( false );
    }
    else
    {
      share->setForeign( true );
    }

    // Copy data from the mounted share to the newly discovered
    // one. We can use Smb4KShare::canonicalPath() here, because
    // Smb4KShare::isInaccessibe() has already been set.
    Smb4KShare *mounted_share = findShareByPath( share->canonicalPath() );

    if ( mounted_share )
    {
      if ( !mounted_share->login().isEmpty() && QString::compare( mounted_share->login(), share->login() ) != 0 )
      {
        share->setLogin( mounted_share->login() );
      }
      else
      {
        // Do nothing
      }

      if ( !mounted_share->workgroupName().isEmpty() && !mounted_share->hostIP().isEmpty() )
      {
        if ( share->workgroupName().isEmpty() )
        {
          share->setWorkgroupName( mounted_share->workgroupName() );
        }
        else
        {
          // Do nothing
        }

        if ( share->hostIP().isEmpty() )
        {
          share->setHostIP( mounted_share->hostIP() );
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Get the host that shares this resource and check if we
        // need to set the IP address or workgroup/domain.
        Smb4KHost *host = findHost( share->hostName(), share->workgroupName() );

        if ( host )
        {
          // Set the IP address if necessary.
          if ( share->hostIP().isEmpty() || QString::compare( host->ip(), share->hostIP() ) != 0 )
          {
            share->setHostIP( host->ip() );
          }
          else
          {
            // Do nothing
          }

          // Set the workgroup/domain name if necessary.
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
          if ( !mounted_share->hostIP().isEmpty() && share->hostIP().isEmpty() )
          {
            share->setHostIP( mounted_share->hostIP() );
          }
          else
          {
            // Do nothing
          }

          if ( !mounted_share->workgroupName().isEmpty() && share->workgroupName().isEmpty() )
          {
            share->setWorkgroupName( mounted_share->workgroupName() );
          }
          else
          {
            // Do nothing
          }
        }
      }

      // Now remove the obsolete share entry from the global list
      // of shares and add the stat'ed one. Emit the appropriate
      // signal when done.
      if ( !share->isForeign() || Smb4KSettings::detectAllShares() )
      {
        // This share was previouly mounted.
        removeMountedShare( mounted_share );

        Smb4KShare *new_share = new Smb4KShare( *share );
        addMountedShare( new_share );
        emit updated( new_share );
      }
      else
      {
        mounted_share->setIsMounted( false );
        emit unmounted( mounted_share );
        removeMountedShare( mounted_share );
        emit mountedSharesListChanged();
      }
    }
    else
    {
      // Get the host that shares this resource and check if we
      // need to set the IP address or workgroup/domain.
      Smb4KHost *host = findHost( share->hostName(), share->workgroupName() );

      if ( host )
      {
        // Set the IP address if necessary.
        if ( share->hostIP().isEmpty() || QString::compare( host->ip(), share->hostIP() ) != 0 )
        {
          share->setHostIP( host->ip() );
        }
        else
        {
          // Do nothing
        }

        // Set the workgroup/domain name if necessary.
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

      // Now add the stat'ed share to the global list of shares.
      // Emit the appropriate signal when done.
      if ( !share->isForeign() || Smb4KSettings::detectAllShares() )
      {
        // This is a new share.
        Smb4KShare *new_share = new Smb4KShare( *share );
        addMountedShare( new_share );
        emit mounted( new_share );
        emit mountedSharesListChanged();
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

  delete share;

  if ( !d->firstImportDone && d->importedShares.isEmpty() )
  {
    d->firstImportDone = true;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotProfileSettingsChanged()
{
  // Stop the timer.
  killTimer(d->timerId);
  
  // Abort all actions that are currently running.
  if (isRunning())
  {
    abortAll();
  }
  else
  {
    // Do nothing
  }
  
  // Clear all remounts.
  while (!d->remounts.isEmpty())
  {
    delete d->remounts.takeFirst();
  }
  
  // Clear all retries.
  while (!d->retries.isEmpty())
  {
    delete d->retries.takeFirst();
  }
  
  // Unmount all shares and wait until done.
  unmountAllShares();
  
  while ( hasSubjobs() )
  {
    QTest::qWait(TIMEOUT);
  }
  
  // Reset some variables.
  d->importTimeout = 0;
  d->remountTimeout = 0;
  d->remountAttempts = 0;
  d->checks = 0;
  d->firstImportDone = false;
  d->importsAllowed = true;
  
  // Import the shares currently mounted.
  import(true);
  
  // Start the timer again.
  d->timerId = startTimer(TIMEOUT);
}



#include "smb4kmounter.moc"
