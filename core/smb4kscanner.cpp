/***************************************************************************
    smb4kscanner  -  This class retrieves all workgroups, servers and
    shares found on the network neighborhood
                             -------------------
    begin                : So Mai 22 2011
    copyright            : (C) 2011 by Alexander Reinholdt
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
#include <QTimer>
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <kglobal.h>
#include <kapplication.h>

// application specific includes
#include <smb4kscanner.h>
#include <smb4kscanner_p.h>
#include <smb4ksettings.h>
#include <smb4kbasicnetworkitem.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kshare.h>
#include <smb4kglobal.h>
#include <smb4kipaddressscanner.h>
#include <smb4kauthinfo.h>
#include <smb4kwalletmanager.h>
#include <smb4knotification.h>

using namespace Smb4KGlobal;

#define TIMER_INTERVAL 250

K_GLOBAL_STATIC( Smb4KScannerPrivate, p );


Smb4KScanner::Smb4KScanner() : KCompositeJob( 0 )
{
  m_interval = 0;
  m_scanning_allowed = true;
  
  connect( QCoreApplication::instance(), SIGNAL( aboutToQuit() ), SLOT( slotAboutToQuit() ) );
}


Smb4KScanner::~Smb4KScanner()
{
}


Smb4KScanner *Smb4KScanner::self()
{
  return &p->instance;
}


bool Smb4KScanner::isRunning()
{
  return !subjobs().isEmpty();
}


bool Smb4KScanner::isRunning( Smb4KScanner::Process process, Smb4KBasicNetworkItem *item )
{
  bool running = false;

  switch ( process )
  {
    case LookupDomains:
    {
      // We do not need a network item with this kind
      // of process. We'll just test if at least on job labeled
      // 'LookupDomainsJob' or 'ScanBAreasJob' is running.
      for ( int i = 0; i < subjobs().size(); ++i )
      {
        if ( QString::compare( subjobs().at( i )->objectName(), "LookupDomainsJob" ) == 0 ||
             QString::compare( subjobs().at( i )->objectName(), "ScanBAreasJob" ) == 0 )
        {
          running = true;
          break;
        }
        else
        {
          continue;
        }
      }
      break;
    }
    case LookupDomainMembers:
    {
      if ( item && item->type() == Smb4KBasicNetworkItem::Workgroup )
      {
        // Only return TRUE if a job for the passed workgroup is running.
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( item );
        
        if ( workgroup )
        {
          for ( int i = 0; i < subjobs().size(); ++i )
          {
            if ( QString::compare( subjobs().at( i )->objectName(),
                 QString( "LookupDomainMembersJob_%1" ).arg( workgroup->workgroupName() ), Qt::CaseInsensitive ) == 0 )
            {
              running = true;
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupDomainMembersJob".
        for ( int i = 0; i < subjobs().size(); ++i )
        {
          if ( subjobs().at( i )->objectName().startsWith( "LookupDomainMembersJob" ) )
          {
            running = true;
            break;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    case LookupShares:
    {
      if ( item && item->type() == Smb4KBasicNetworkItem::Host )
      {
        // Only return TRUE if a job for the passed host is running.
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        
        if ( host )
        {
          for ( int i = 0; i < subjobs().size(); ++i )
          {
            if ( QString::compare( subjobs().at( i )->objectName(),
                 QString( "LookupSharesJob_%1" ).arg( host->hostName() ), Qt::CaseInsensitive ) == 0 )
            {
              running = true;
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupSharesJob".
        for ( int i = 0; i < subjobs().size(); ++i )
        {
          if ( subjobs().at( i )->objectName().startsWith( "LookupSharesJob" ) )
          {
            running = true;
            break;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    case LookupInfo:
    {
      if ( item && item->type() == Smb4KBasicNetworkItem::Host )
      {
        // Only return TRUE if a job for the passed host is running.
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        
        if ( host )
        {
          for ( int i = 0; i < subjobs().size(); ++i )
          {
            if ( QString::compare( subjobs().at( i )->objectName(),
                 QString( "LookupInfoJob_%1" ).arg( host->hostName() ), Qt::CaseInsensitive ) == 0 )
            {
              running = true;
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupInfoJob".
        for ( int i = 0; i < subjobs().size(); ++i )
        {
          if ( subjobs().at( i )->objectName().startsWith( "LookupInfoJob" ) )
          {
            running = true;
            break;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }

  return running;
}


void Smb4KScanner::abortAll()
{
  for ( int i = 0; i < subjobs().size(); ++i )
  {
    subjobs().at( i )->kill( KJob::EmitResult );
  }
}


void Smb4KScanner::abort( Smb4KScanner::Process process, Smb4KBasicNetworkItem *item )
{
  switch ( process )
  {
    case LookupDomains:
    {
      // We do not need a network item with this kind
      // of process. We'll just kill all jobs labeled
      // 'LookupDomainsJob' and 'ScanBAreasJob'.
      for ( int i = 0; i < subjobs().size(); ++i )
      {
        if ( QString::compare( subjobs().at( i )->objectName(), "LookupDomainsJob" ) == 0 ||
             QString::compare( subjobs().at( i )->objectName(), "ScanBAreasJob" ) == 0 )
        {
          subjobs().at( i )->kill( KJob::EmitResult );
          continue;
        }
        else
        {
          continue;
        }
      }
      break;
    }
    case LookupDomainMembers:
    {
      if ( item && item->type() == Smb4KBasicNetworkItem::Workgroup )
      {
        // Only kill a job if the workgroup matches.
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( item );
        
        if ( workgroup )
        {
          for ( int i = 0; i < subjobs().size(); ++i )
          {
            if ( QString::compare( subjobs().at( i )->objectName(),
                 QString( "LookupDomainMembersJob_%1" ).arg( workgroup->workgroupName() ), Qt::CaseInsensitive ) == 0 )
            {
              subjobs().at( i )->kill( KJob::EmitResult );
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupDomainMembersJob".
        for ( int i = 0; i < subjobs().size(); ++i )
        {
          if ( subjobs().at( i )->objectName().startsWith( "LookupDomainMembersJob" ) )
          {
            subjobs().at( i )->kill( KJob::EmitResult );
            continue;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    case LookupShares:
    {
      if ( item && item->type() == Smb4KBasicNetworkItem::Host )
      {
        // Only kill a job if the host matches
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        
        if ( host )
        {
          for ( int i = 0; i < subjobs().size(); ++i )
          {
            if ( QString::compare( subjobs().at( i )->objectName(),
                 QString( "LookupSharesJob_%1" ).arg( host->hostName() ), Qt::CaseInsensitive ) == 0 )
            {
              subjobs().at( i )->kill( KJob::EmitResult );
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupSharesJob".
        for ( int i = 0; i < subjobs().size(); ++i )
        {
          if ( subjobs().at( i )->objectName().startsWith( "LookupSharesJob" ) )
          {
            subjobs().at( i )->kill( KJob::EmitResult );
            continue;
          }
          else
          {
            continue;
          }
        }
      }

      break;
    }
    case LookupInfo:
    {
      if ( item && item->type() == Smb4KBasicNetworkItem::Host )
      {
        // Only return TRUE if a job for the passed host is running.
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        
        if ( host )
        {
          for ( int i = 0; i < subjobs().size(); ++i )
          {
            if ( QString::compare( subjobs().at( i )->objectName(),
                 QString( "LookupInfoJob_%1" ).arg( host->hostName() ), Qt::CaseInsensitive ) == 0 )
            {
              subjobs().at( i )->kill( KJob::EmitResult );
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupInfoJob".
        for ( int i = 0; i < subjobs().size(); ++i )
        {
          if ( subjobs().at( i )->objectName().startsWith( "LookupInfoJob" ) )
          {
            subjobs().at( i )->kill( KJob::EmitResult );
            continue;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KScanner::start()
{
  // Avoid a race with QApplication and use 50 ms here.
  QTimer::singleShot( 50, this, SLOT( slotStartJobs() ) );
}


void Smb4KScanner::lookupDomains( QWidget *parent )
{
  if ( Smb4KSettings::lookupDomains() )
  {
    Smb4KLookupDomainsJob *job = new Smb4KLookupDomainsJob( this );
    job->setObjectName( "LookupDomainsJob" );
    job->setupLookup( parent );

    connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
    connect( job, SIGNAL( aboutToStart() ), SLOT( slotAboutToStartDomainsLookup() ) );
    connect( job, SIGNAL( finished() ), SLOT( slotDomainsLookupFinished() ) );
    connect( job, SIGNAL( workgroups( const QList<Smb4KWorkgroup> & ) ), SLOT( slotWorkgroups( const QList<Smb4KWorkgroup> & ) ) );

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
  else if ( Smb4KSettings::queryCurrentMaster() )
  {
    Smb4KQueryMasterJob *job = new Smb4KQueryMasterJob( this );
    job->setObjectName( "LookupDomainsJob" );
    job->setupLookup( QString(), parent );

    connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
    connect( job, SIGNAL( aboutToStart() ), SLOT( slotAboutToStartDomainsLookup() ) );
    connect( job, SIGNAL( finished() ), SLOT( slotDomainsLookupFinished() ) );
    connect( job, SIGNAL( workgroups( const QList<Smb4KWorkgroup> & ) ), SLOT( slotWorkgroups( const QList<Smb4KWorkgroup> & ) ) );
    connect( job, SIGNAL( authError( Smb4KQueryMasterJob * ) ), SLOT( slotAuthError( Smb4KQueryMasterJob * ) ) );

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
  else if ( Smb4KSettings::queryCustomMaster() )
  {
    // If the custom master browser entry is empty, warn the user
    // and tell him/her that we are going to query the current master
    // browser instead.
    if ( Smb4KSettings::customMasterBrowser().isEmpty() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->emptyCustomMasterBrowser();
    }
    else
    {
      // Do nothing
    }
    
    Smb4KQueryMasterJob *job = new Smb4KQueryMasterJob( this );
    job->setObjectName( "LookupDomainsJob" );
    job->setupLookup( Smb4KSettings::customMasterBrowser(), parent );

    connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
    connect( job, SIGNAL( aboutToStart() ), SLOT( slotAboutToStartDomainsLookup() ) );
    connect( job, SIGNAL( finished() ), SLOT( slotDomainsLookupFinished() ) );
    connect( job, SIGNAL( workgroups( const QList<Smb4KWorkgroup> & ) ), SLOT( slotWorkgroups( const QList<Smb4KWorkgroup> & ) ) );
    connect( job, SIGNAL( authError( Smb4KQueryMasterJob * ) ), SLOT( slotAuthError( Smb4KQueryMasterJob * ) ) );

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
  else if ( Smb4KSettings::scanBroadcastAreas() )
  {
    if ( !Smb4KSettings::broadcastAreas().isEmpty() )
    {
      Smb4KScanBAreasJob *job = new Smb4KScanBAreasJob( this );
      job->setObjectName( "ScanBAreasJob" );
      job->setupScan( parent );

      connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
      connect( job, SIGNAL( aboutToStart() ), SLOT( slotAboutToStartDomainsLookup() ) );
      connect( job, SIGNAL( finished() ), SLOT( slotDomainsLookupFinished() ) );
      connect( job, SIGNAL( workgroups( const QList<Smb4KWorkgroup> & ) ), SLOT( slotWorkgroups( const QList<Smb4KWorkgroup> & ) ) );
      connect( job, SIGNAL( hosts( const QList<Smb4KHost> & ) ), SLOT( slotHosts( const QList<Smb4KHost> & ) ) );

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
    else
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->emptyBroadcastAreas();
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KScanner::lookupDomainMembers( Smb4KWorkgroup *workgroup, QWidget *parent )
{
  Q_ASSERT( workgroup );

  Smb4KLookupDomainMembersJob *job = new Smb4KLookupDomainMembersJob( this );
  job->setObjectName( QString( "LookupDomainMembersJob_%1" ).arg( workgroup->workgroupName() ) );
  job->setupLookup( workgroup, parent );

  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( aboutToStart( Smb4KWorkgroup * ) ), SLOT( slotAboutToStartHostsLookup( Smb4KWorkgroup * ) ) );
  connect( job, SIGNAL( finished( Smb4KWorkgroup * ) ), SLOT( slotHostsLookupFinished( Smb4KWorkgroup * ) ) );
  connect( job, SIGNAL( hosts( Smb4KWorkgroup *, const QList<Smb4KHost> & ) ), SLOT( slotHosts( Smb4KWorkgroup *, const QList<Smb4KHost> & ) ) );
  connect( job, SIGNAL( authError( Smb4KLookupDomainMembersJob * ) ), SLOT( slotAuthError( Smb4KLookupDomainMembersJob * ) ) );

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


void Smb4KScanner::lookupShares( Smb4KHost *host, QWidget *parent )
{
  Q_ASSERT( host );
  
  Smb4KLookupSharesJob *job = new Smb4KLookupSharesJob( this );
  job->setObjectName( QString( "LookupSharesJob_%1" ).arg( host->hostName() ) );
  job->setupLookup( host, parent );
  
  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( aboutToStart( Smb4KHost * ) ), SLOT( slotAboutToStartSharesLookup( Smb4KHost * ) ) );
  connect( job, SIGNAL( finished( Smb4KHost * ) ), SLOT( slotSharesLookupFinished( Smb4KHost * ) ) );
  connect( job, SIGNAL( shares( Smb4KHost *, const QList<Smb4KShare> & ) ), SLOT( slotShares( Smb4KHost *, const QList<Smb4KShare> &) ) );
  connect( job, SIGNAL( authError( Smb4KLookupSharesJob * ) ), SLOT( slotAuthError( Smb4KLookupSharesJob * ) ) );
  
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


void Smb4KScanner::lookupInfo( Smb4KHost *host, QWidget *parent )
{
  Q_ASSERT( host );
  
  // Check if the additional information (Server, OS) has already been
  // aquired previously or if we need to start a lookup job.
  Smb4KHost *known_host = findHost( host->hostName(), host->workgroupName() );
  
  if ( known_host && known_host->infoChecked() )
  {
    emit info( known_host );
    return;
  }
  else
  {
    // Do nothing
  }
  
  Smb4KLookupInfoJob *job = new Smb4KLookupInfoJob( this );
  job->setObjectName( QString( "LookupInfoJob_%1" ).arg( host->hostName() ) );
  job->setupLookup( host, parent );
    
  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( aboutToStart( Smb4KHost * ) ), SLOT( slotAboutToStartSharesLookup( Smb4KHost * ) ) );
  connect( job, SIGNAL( finished( Smb4KHost * ) ), SLOT( slotSharesLookupFinished( Smb4KHost * ) ) );
  connect( job, SIGNAL( info( Smb4KHost * ) ), SLOT( slotInfo( Smb4KHost * ) ) );
    
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


void Smb4KScanner::timerEvent( QTimerEvent */*e*/ )
{
  if ( Smb4KSettings::periodicScanning() )
  {
    if ( m_interval == 0 )
    {
      if ( m_periodic_jobs.isEmpty() )
      {
        // This case occurs when the user enables periodic scanning during 
        // runtime. We need to fill the list of periodic jobs here, so that
        // we can immediately start periodic scanning.
        m_periodic_jobs << LookupDomains;
        m_periodic_jobs << LookupDomainMembers;
        m_periodic_jobs << LookupShares;
      }
      else
      {
        // This is the regular case. We do not need to do anything.        
      }
      
      Process p = m_periodic_jobs.takeFirst();
      
      switch ( p )
      {
        case LookupDomains:
        {
          m_scanning_allowed = false;
          lookupDomains();
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
      if ( m_interval >= (Smb4KSettings::scanInterval() * 60000 /* milliseconds */) )
      {
        // Reset interval
        m_interval = 0;
        
        // Fill list
        m_periodic_jobs << LookupDomains;
        m_periodic_jobs << LookupDomainMembers;
        m_periodic_jobs << LookupShares;
      }
      else
      {
        // Check if we need to do something.
        // Do not start any process before the previous has not finished.
        if ( !m_periodic_jobs.isEmpty() && m_scanning_allowed )
        {
          Process p = m_periodic_jobs.takeFirst();
          
          switch ( p )
          {
            case LookupDomainMembers:
            {
              for ( int i = 0; i < workgroupsList().size(); ++i )
              {
                m_scanning_allowed = false;
                lookupDomainMembers( workgroupsList()[i] );
              }
              break;
            }
            case LookupShares:
            {
              for ( int i = 0; i < hostsList().size(); ++i )
              {
                m_scanning_allowed = false;
                lookupShares( hostsList()[i] );
              }
              break;
            }
            default:
            {
              break;
            }
          };
        }
        else
        {
          // Do nothing
        }
      }
    }
    
    m_interval += TIMER_INTERVAL;
  }
  else
  {
    // Periodic scanning is not enabled or has been disabled
    // during runtime. So, reset the interval, if necessary.
    if ( m_interval != 0 )
    {
      m_interval = 0;
    }
    else
    {
      // Do nothing
    }
  }
}



/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KScanner::slotAboutToQuit()
{
  abortAll();
}


void Smb4KScanner::slotStartJobs()
{
  // If the user wants to have periodic scanning of the network
  // neighborhood, set it up here here.
  if ( Smb4KSettings::periodicScanning() )
  {
    // Fill list
    m_periodic_jobs << LookupDomains;
    m_periodic_jobs << LookupDomainMembers;
    m_periodic_jobs << LookupShares;
  }
  else
  {
    lookupDomains( 0 );
  }
  
  // Start the timer in any case. Thus, we are able to switch
  // to periodic scanning seamlessly in the timerEvent() function.
  startTimer( 250 );
}


void Smb4KScanner::slotJobFinished( KJob *job )
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


void Smb4KScanner::slotAuthError( Smb4KQueryMasterJob *job )
{
  Smb4KAuthInfo authInfo;
  
  if ( !job->masterBrowser().isEmpty() )
  {
    Smb4KHost master;

    if ( QHostAddress( job->masterBrowser() ).protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
    {
      master.setHostName( job->masterBrowser() );
    }
    else
    {
      master.setIP( job->masterBrowser() );
    }
    
    authInfo.setHost( &master );
  }
  else
  {
    authInfo.useDefaultAuthInfo();
  }

  if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, job->parentWidget() ) )
  {
    // Start a query job with the returned master browser.
    Smb4KQueryMasterJob *job = new Smb4KQueryMasterJob( this );
    job->setObjectName( "LookupDomainsJob" );
    job->setupLookup( job->masterBrowser(), job->parentWidget() );

    connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
    connect( job, SIGNAL( aboutToStart() ), SLOT( slotAboutToStartDomainsLookup() ) );
    connect( job, SIGNAL( finished() ), SLOT( slotDomainsLookupFinished() ) );
    connect( job, SIGNAL( workgroups( const QList<Smb4KWorkgroup> & ) ), SLOT( slotWorkgroups( const QList<Smb4KWorkgroup> & ) ) );
    connect( job, SIGNAL( authError( Smb4KQueryMasterJob * ) ), SLOT( slotAuthError( Smb4KQueryMasterJob * ) ) );

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
  else
  {
    // Do nothing
  }
}


void Smb4KScanner::slotAuthError( Smb4KLookupDomainMembersJob *job )
{
  Smb4KHost *master = findHost( job->workgroup()->masterBrowserName(), job->workgroup()->workgroupName() );
  
  if ( master )
  {
    Smb4KAuthInfo authInfo( master );
   
    if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, job->parentWidget() ) )
    {
      lookupDomainMembers( job->workgroup(), job->parentWidget() );
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


void Smb4KScanner::slotAuthError( Smb4KLookupSharesJob *job )
{
  Smb4KHost *host = findHost( job->host()->hostName(), job->host()->workgroupName() );
  
  if ( host )
  {
    Smb4KAuthInfo authInfo( host );
    
    if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, job->parentWidget() ) )
    {
      lookupShares( job->host(), job->parentWidget() );
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


void Smb4KScanner::slotAboutToStartDomainsLookup()
{
  Smb4KBasicNetworkItem item;
  emit aboutToStart( &item, LookupDomains );
}


void Smb4KScanner::slotDomainsLookupFinished()
{
  Smb4KBasicNetworkItem item;
  emit finished( &item, LookupDomains );
  m_scanning_allowed = true;
}


void Smb4KScanner::slotAboutToStartHostsLookup( Smb4KWorkgroup *workgroup )
{
  emit aboutToStart( workgroup, LookupDomainMembers );
}


void Smb4KScanner::slotHostsLookupFinished( Smb4KWorkgroup *workgroup )
{
  emit finished( workgroup, LookupDomainMembers );
  m_scanning_allowed = true;
}


void Smb4KScanner::slotAboutToStartSharesLookup( Smb4KHost *host )
{
  emit aboutToStart( host, LookupShares );
}


void Smb4KScanner::slotSharesLookupFinished( Smb4KHost *host )
{
  emit finished( host, LookupShares );
  m_scanning_allowed = true;
}


void Smb4KScanner::slotAboutToStartInfoLookup( Smb4KHost *host )
{
  emit aboutToStart( host, LookupInfo );
}


void Smb4KScanner::slotInfoLookupFinished( Smb4KHost *host )
{
  emit finished( host, LookupInfo );
}


void Smb4KScanner::slotWorkgroups( const QList<Smb4KWorkgroup> &workgroups_list )
{
  // The new workgroup list will be used as global workgroup list.
  // We do some checks and adjustments now, so that the host list 
  // is also correctly updated.
  if ( !workgroups_list.isEmpty() )
  {
    for ( int i = 0; i < workgroups_list.size(); ++i )
    {
      Smb4KWorkgroup *workgroup = findWorkgroup( workgroups_list.at( i ).workgroupName() );

      // Check if the master browser changed.
      if ( workgroup )
      {
        if ( QString::compare( workgroups_list.at( i ).masterBrowserName(), workgroup->masterBrowserName(), Qt::CaseInsensitive ) != 0 )
        {
          // Get the old master browser and reset the master browser flag.
          Smb4KHost *old_master_browser = findHost( workgroup->masterBrowserName(), workgroup->workgroupName() );

          if ( old_master_browser )
          {
            old_master_browser->setIsMasterBrowser( false );
          }
          else
          {
            // Do nothing
          }

          // Lookup new master browser and either set the master browser flag
          // or insert it if it does not exit yet.
          Smb4KHost *new_master_browser = findHost( workgroups_list.at( i ).masterBrowserName(), workgroups_list.at( i ).workgroupName() );

          if ( new_master_browser )
          {
            if ( workgroups_list.at( i ).hasMasterBrowserIP() )
            {
              new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
            }
            else
            {
              // Do nothing
            }

            new_master_browser->setIsMasterBrowser( true );
          }
          else
          {
            new_master_browser = new Smb4KHost();
            new_master_browser->setHostName( workgroups_list.at( i ).masterBrowserName() );

            if ( workgroups_list.at( i ).hasMasterBrowserIP() )
            {
              new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
            }
            else
            {
              // Do nothing
            }

            new_master_browser->setWorkgroupName( workgroups_list.at( i ).workgroupName() );
            new_master_browser->setIsMasterBrowser( true );

            addHost( new_master_browser );
          }
        }
        else
        {
          // Do nothing
        }

        removeWorkgroup( workgroup );
      }
      else
      {
        // Check if the master browser of the new workgroup list is by chance
        // already in the list of hosts. If it exists, set the master browser
        // flag, else insert it.
        Smb4KHost *new_master_browser = findHost( workgroups_list.at( i ).masterBrowserName(), workgroups_list.at( i ).workgroupName() );

        if ( new_master_browser )
        {
          if ( workgroups_list.at( i ).hasMasterBrowserIP() )
          {
            new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
          }
          else
          {
            // Do nothing
          }

          new_master_browser->setIsMasterBrowser( true );
        }
        else
        {
          new_master_browser = new Smb4KHost();
          new_master_browser->setHostName( workgroups_list.at( i ).masterBrowserName() );

          if ( workgroups_list.at( i ).hasMasterBrowserIP() )
          {
            new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
          }
          else
          {
            // Do nothing
          }

          new_master_browser->setWorkgroupName( workgroups_list.at( i ).workgroupName() );
          new_master_browser->setIsMasterBrowser( true );

          addHost( new_master_browser );
        }
      }
    }
  }
  else
  {
    // Do nothing
  }

  // The global workgroup list only contains obsolete workgroups now.
  // Remove all hosts belonging to those obsolete workgroups from the
  // host list and then also the workgroups themselves.
  while ( !workgroupsList().isEmpty() )
  {
    Smb4KWorkgroup *workgroup = workgroupsList().first();
    QList<Smb4KHost *> obsolete_hosts = workgroupMembers( workgroup );
    QListIterator<Smb4KHost *> h( obsolete_hosts );
    
    while ( h.hasNext() )
    {
      Smb4KHost *host = h.next();
      removeHost( host );
    }

    removeWorkgroup( workgroup );
  }

  // Add a copy of all workgroups to the global list.
  for ( int i = 0; i < workgroups_list.size(); ++i )
  {
    addWorkgroup( new Smb4KWorkgroup( workgroups_list.at( i ) ) );
  }

  // Scan for IP addresses if necessary
  if ( !Smb4KSettings::scanBroadcastAreas() )
  {
    Smb4KIPAddressScanner::self()->lookup();
  }
  else
  {
    // Do nothing
  }

  emit workgroups( workgroupsList() );
  emit hostListChanged();  
}


void Smb4KScanner::slotHosts( const QList<Smb4KHost> &hosts_list )
{
  slotHosts( NULL, hosts_list );
}


void Smb4KScanner::slotHosts( Smb4KWorkgroup *workgroup, const QList<Smb4KHost> &hosts_list )
{
  QList<Smb4KHost> internal_hosts_list;
  
  if ( !hosts_list.isEmpty() )
  {
    // Copy any information we might need to the internal list and
    // remove the host from the global list. It will be added again
    // in an instant.
    for ( int i = 0; i < hosts_list.size(); ++i )
    {
      Smb4KHost new_host = hosts_list[i];
      Smb4KHost *host = findHost( new_host.hostName(), new_host.workgroupName() );

      if ( host )
      {
        // Set comment
        if ( new_host.comment().isEmpty() && !host->comment().isEmpty() )
        {
          new_host.setComment( host->comment() );
        }
        else
        {
          // Do nothing
        }

        // Set the additional information
        if ( !new_host.infoChecked() && host->infoChecked() )
        {
          new_host.setInfo( host->serverString(), host->osString() );
        }
        else
        {
          // Do nothing
        }

        // Set the IP addresses
        if ( !new_host.hasIP() && host->hasIP() )
        {
          new_host.setIP( host->ip() );
        }
        else
        {
          // Do nothing
        }

        removeHost( host );
      }
      else
      {
        // Do nothing
      }
      
      internal_hosts_list << new_host;
    }
  }
  else
  {
    // Do nothing
  }

  if ( workgroup )
  {
    // Now remove all (obsolete) hosts of the scanned workgroup from
    // the global list as well as their shares.
    QList<Smb4KHost *> obsolete_hosts = workgroupMembers( workgroup );
    QListIterator<Smb4KHost *> h( obsolete_hosts );
    
    while ( h.hasNext() )
    {
      Smb4KHost *host = h.next();
      
      QList<Smb4KShare *> obsolete_shares = sharedResources( host );
      QListIterator<Smb4KShare *> s( obsolete_shares );
      
      while ( s.hasNext() )
      {
        Smb4KShare *share = s.next();
        removeShare( share );
      }
      
      removeHost( host );
    }
  }
  else
  {
    // If no workgroup was passed it means that we are doing an IP scan
    // or at least members of more than one workgroup were looked up. In
    // this case the global hosts list is considered to carry obsolete
    // host entries at this point. Remove them as well as their shares.
    while ( !hostsList().isEmpty() )
    {
      Smb4KHost *host = hostsList().first();

      QList<Smb4KShare *> obsolete_shares = sharedResources( host );
      QListIterator<Smb4KShare *> s( obsolete_shares );
      
      while ( s.hasNext() )
      {
        Smb4KShare *share = s.next();
        removeShare( share );
      }
      
      removeHost( host );
    }
  }
  
  // Add a copy of all hosts to the global list.
  for ( int i = 0; i < internal_hosts_list.size(); ++i )
  {
    addHost( new Smb4KHost( internal_hosts_list.at( i ) ) );
  }

  // Scan for IP addresses if necessary
  if ( !internal_hosts_list.isEmpty() && !Smb4KSettings::scanBroadcastAreas() )
  {
    Smb4KIPAddressScanner::self()->lookup();
  }
  else
  {
    // Do nothing
  }
  
  if ( workgroup )
  {
    QList<Smb4KHost *> workgroup_members = workgroupMembers( workgroup );
    emit hosts( workgroup, workgroup_members );
  }
  else
  {
    emit hosts( workgroup, hostsList() );
  }
  emit hostListChanged();
  qDebug() << "slotHosts(): Finished";
}


void Smb4KScanner::slotShares( Smb4KHost *host, const QList<Smb4KShare> &shares_list )
{
  Q_ASSERT( host );
  
  QList<Smb4KShare> internal_shares_list;
  
  if ( !shares_list.isEmpty() )
  {
    // Copy some information before processing the shares further. 
    // Note, that the IP address and other information stemming from
    // the host were already entered by the lookup job.
    for ( int i = 0; i < shares_list.size(); ++i )
    {
      Smb4KShare new_share = shares_list[i];
      
      // Check if the share has already been mounted.
      QList<Smb4KShare *> mounted_shares = findShareByUNC( new_share.unc() );
      
      if ( !mounted_shares.isEmpty() )
      {
        // FIXME: We cannot honor Smb4KSettings::showAllShares() here, because 
        // in case the setting is changed, there will be no automatic rescan
        // (in case of an automatic or periodical rescan that would be the 
        // favorable method...
        //
        // For now, we prefer the share mounted by the user or use the first
        // occurrence if he/she did not mount it.
        Smb4KShare *mounted_share = mounted_shares.first();
        
        for ( int j = 0; j < mounted_shares.size(); ++j )
        {
          if ( !mounted_shares.at( i )->isForeign() )
          {
            mounted_share = mounted_shares[i];
            break;
          }
          else
          {
            continue;
          }
        }
        
        new_share.setMountData( mounted_share );
      }
      else
      {
        // Do nothing
      }
      
      // Now set some information that might have been collected
      // since the lookup started...
      Smb4KShare *share = findShare( new_share.shareName(), new_share.hostName(), new_share.workgroupName() );
        
      if ( share )
      {
        if ( !new_share.hasHostIP() && share->hasHostIP() )
        {
          new_share.setHostIP( share->hostIP() );
        }
        else
        {
          // Do nothing
        }
          
        removeShare( share );
      }
      else
      {
        // Do nothing
      }
        
      internal_shares_list << new_share;
    }
  }
  else
  {
    // Do nothing
  }
  
  // Copy authentication information
  Smb4KHost *known_host = findHost( host->hostName(), host->workgroupName() );
  
  if ( known_host )
  {
    known_host->setLogin( host->login() );
    known_host->setPassword( host->password() );
  }
  else
  {
    // Do nothing
  }
  
  // Now remove all (obsolete) shares of the scanned host from
  // the global list.
  QList<Smb4KShare *> obsolete_shares = sharedResources( host );
  QListIterator<Smb4KShare *> s( obsolete_shares );
    
  while ( s.hasNext() )
  {
    Smb4KShare *share = s.next();
    removeShare( share );
  }
  
  // Add a copy of all shares to the global list.
  for ( int i = 0; i < internal_shares_list.size(); ++i )
  {
    addShare( new Smb4KShare( internal_shares_list.at( i ) ) );
  }
  
  QList<Smb4KShare *> shared_resources = sharedResources( host );
  emit shares( host, shared_resources );
}


void Smb4KScanner::slotInfo( Smb4KHost *host )
{
  Q_ASSERT( host );
  
  Smb4KHost *known_host = NULL;
  
  if ( host->infoChecked() )
  {
    // Copy the information also to host in the global list, if present,
    // or copy 'host' to the global list.
    known_host = findHost( host->hostName(), host->workgroupName() );

    if ( known_host )
    {
      known_host->setInfo( host->serverString(), host->osString() );
    }
    else
    {
      known_host = new Smb4KHost( *host );
      addHost( known_host );
    }
  }
  else
  {
    // Do nothing
  }

  // Emit the host here.
  emit info( known_host );
}


#include "smb4kscanner.moc"
