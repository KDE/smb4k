/***************************************************************************
    smb4kipaddressscanner  -  This class scans for IP addresses.
                             -------------------
    begin                : Fri Mar 18 2011
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>

// KDE includes
#include <kglobal.h>

// application specific includes
#include <smb4kipaddressscanner.h>
#include <smb4kipaddressscanner_p.h>
#include <smb4kglobal.h>

using namespace Smb4KGlobal;


K_GLOBAL_STATIC( Smb4KIPAddressScannerPrivate, p );


Smb4KIPAddressScanner::Smb4KIPAddressScanner() : KCompositeJob( 0 )
{
  connect( QCoreApplication::instance(), SIGNAL( aboutToQuit() ), SLOT( slotAboutToQuit() ) );
}


Smb4KIPAddressScanner::~Smb4KIPAddressScanner()
{
}


Smb4KIPAddressScanner *Smb4KIPAddressScanner::self()
{
  return &p->instance;
}


void Smb4KIPAddressScanner::lookup( bool force, QWidget *parent )
{
  if ( !hostsList().isEmpty() )
  {
    for ( int i = 0; i < hostsList().size(); i++ )
    {
      // We do not need to lookup the IP address for a
      // host that already has got one except 'force' is 
      // true.
      if ( !hostsList().at( i )->ipChecked() || force )
      {
        // Create a new job and add it to the subjobs
        Smb4KIPLookupJob *job = new Smb4KIPLookupJob( this );
        job->setObjectName( QString( "IPLookupJob_%1" ).arg( hostsList().at( i )->unc() ) );
        job->setupLookup( hostsList()[i], parent );

        connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
         connect( job, SIGNAL( ipAddress( Smb4KHost * ) ), SLOT( slotProcessIPAddress( Smb4KHost * ) ) );
        connect( job, SIGNAL( aboutToStart( Smb4KHost * ) ), SIGNAL( aboutToStart( Smb4KHost * ) ) );
        connect( job, SIGNAL( finished( Smb4KHost * ) ), SIGNAL( finished( Smb4KHost * ) ) );

        addSubjob( job );

        job->start();
      }
      else
      {
        emit ipAddress( hostsList()[i] );
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KIPAddressScanner::getIPAddress( Smb4KWorkgroup *workgroup )
{
  for ( int i = 0; i < hostsList().size(); i++ )
  {
    if ( !hostsList().at( i )->workgroupName().isEmpty() )
    {
      if ( QString::compare( hostsList().at( i )->workgroupName(),
           workgroup->workgroupName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( hostsList().at( i )->hostName(),
           workgroup->masterBrowserName(), Qt::CaseInsensitive ) == 0 )
      {
        // Only set the IP address, if there is one. We can avoid erasing
        // already existing IP addresses.
        if ( hostsList().at( i )->hasIP() )
        {
          workgroup->setMasterBrowserIP( hostsList().at( i )->ip() );
        }
        else
        {
          continue;
        }
        break;
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      if ( QString::compare( hostsList().at( i )->hostName(),
           workgroup->masterBrowserName(), Qt::CaseInsensitive ) == 0 )
      {
        // Only set the IP address, if there is one. We can avoid erasing
        // already existing IP addresses.
        if ( hostsList().at( i )->hasIP() )
        {
          workgroup->setMasterBrowserIP( hostsList().at( i )->ip() );
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
}


void Smb4KIPAddressScanner::getIPAddress( Smb4KHost *host )
{
  for ( int i = 0; i < hostsList().size(); i++ )
  {
    if ( !hostsList().at( i )->workgroupName().isEmpty() && host->workgroupName().isEmpty() )
    {
      if ( QString::compare( hostsList().at( i )->workgroupName(),
           host->workgroupName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( hostsList().at( i )->hostName(),
           host->hostName(), Qt::CaseInsensitive ) == 0 )
      {
        // Only set the IP address, if there is one. We can avoid erasing
        // already existing IP addresses.
        if ( hostsList().at( i )->hasIP() )
        {
          host->setIP( hostsList().at( i )->ip() );
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
    else
    {
      if ( QString::compare( hostsList().at( i )->hostName(),
           host->hostName(), Qt::CaseInsensitive ) == 0 )
      {
        // Only set the IP address, if there is one. We can avoid erasing
        // already existing IP addresses.
        if ( hostsList().at( i )->hasIP() )
        {
          host->setIP( hostsList().at( i )->ip() );
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
}


bool Smb4KIPAddressScanner::isRunning()
{
  return !subjobs().isEmpty();
}


void Smb4KIPAddressScanner::abortAll()
{
  for ( int i = 0; i < subjobs().size(); i++ )
  {
    subjobs().at( i )->kill( KJob::EmitResult );
  }
}


void Smb4KIPAddressScanner::start()
{
  QTimer::singleShot( 0, this, SLOT( slotStartJobs() ) );
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KIPAddressScanner::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KIPAddressScanner::slotProcessIPAddress( Smb4KHost *host )
{
  if ( host->isMasterBrowser() )
  {
    Smb4KWorkgroup *workgroup = findWorkgroup( host->workgroupName() );

    if ( workgroup )
    {
      workgroup->setMasterBrowserIP( host->ip() );
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

  emit ipAddress( host );
}


void Smb4KIPAddressScanner::slotJobFinished( KJob *job )
{
  removeSubjob( job );
}


void Smb4KIPAddressScanner::slotAboutToQuit()
{
  abortAll();
}

#include "smb4kipaddressscanner.moc"
