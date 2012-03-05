/***************************************************************************
    smb4ksearch  -  This class does custom searches
                             -------------------
    begin                : Tue Mar 08 2011
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
#include <QHostAddress>
#include <QAbstractSocket>
#include <QApplication>

// KDE includes
#include <kglobal.h>

// application specific includes
#include <smb4ksearch.h>
#include <smb4ksearch_p.h>
#include <smb4kauthinfo.h>
#include <smb4ksettings.h>
#include <smb4kglobal.h>
#include <smb4kworkgroup.h>
#include <smb4kwalletmanager.h>
#include <smb4khomesshareshandler.h>
#include <smb4kipaddressscanner.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KSearchPrivate, p );


Smb4KSearch::Smb4KSearch() : KCompositeJob( 0 )
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
  
  connect( QCoreApplication::instance(), SIGNAL( aboutToQuit() ), SLOT( slotAboutToQuit() ) );
}


Smb4KSearch::~Smb4KSearch()
{
}


Smb4KSearch *Smb4KSearch::self()
{
  return &p->instance;
}


void Smb4KSearch::search( const QString &string, QWidget *parent )
{
  if ( string.trimmed().isEmpty() )
  {
    return;
  }
  else
  {
    // Do nothing
  }

  // Get authentication information.
  Smb4KHost master_browser;

  if ( Smb4KSettings::masterBrowsersRequireAuth() )
  {
    // smbtree will be used and the master browser requires authentication. 
    // Lookup the authentication information for the master browser.
    Smb4KWorkgroup *workgroup = findWorkgroup( Smb4KSettings::domainName() );
    Smb4KHost *host = NULL;

    if ( workgroup )
    {
      host = findHost( workgroup->masterBrowserName(), workgroup->workgroupName() );

      if ( host )
      {
        // Copy host item
        master_browser = *host;
        
        // Authentication information
        Smb4KWalletManager::self()->readAuthInfo( &master_browser );
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

  // Create a new job and add it to the subjobs
  Smb4KSearchJob *job = new Smb4KSearchJob( this );
  job->setObjectName( QString( "SearchJob_%1" ).arg( string ) );
  job->setupSearch( string, &master_browser, parent );

  connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
  connect( job, SIGNAL( authError( Smb4KSearchJob * ) ), SLOT( slotAuthError( Smb4KSearchJob * ) ) );
  connect( job, SIGNAL( result( Smb4KShare * ) ), SLOT( slotProcessSearchResult( Smb4KShare * ) ) );
  connect( job, SIGNAL( aboutToStart( const QString & ) ), SIGNAL( aboutToStart( const QString & ) ) );
  connect( job, SIGNAL( finished( const QString & ) ), SIGNAL( finished( const QString & ) ) );

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


bool Smb4KSearch::isRunning()
{
  return !subjobs().isEmpty();
}


bool Smb4KSearch::isRunning( const QString &string )
{
  bool running = false;

  for ( int i = 0; i < subjobs().size(); ++i )
  {
    if ( QString::compare( QString( "SearchJob_%1" ).arg( string ), subjobs().at( i )->objectName() ) == 0 )
    {
      running = true;
      break;
    }
    else
    {
      continue;
    }
  }

  return running;
}



void Smb4KSearch::abortAll()
{
  for ( int i = 0; i < subjobs().size(); ++i )
  {
    subjobs().at( i )->kill( KJob::EmitResult );
  }
}


void Smb4KSearch::abort( const QString &string )
{
  for ( int i = 0; i < subjobs().size(); ++i )
  {
    if ( QString::compare( QString( "SearchJob_%1" ).arg( string ), subjobs().at( i )->objectName() ) == 0 )
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


void Smb4KSearch::start()
{
  QTimer::singleShot( 0, this, SLOT( slotStartJobs() ) );
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSearch::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KSearch::slotJobFinished( KJob *job )
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


void Smb4KSearch::slotAuthError( Smb4KSearchJob *job )
{
  if ( Smb4KWalletManager::self()->showPasswordDialog( job->masterBrowser(), job->parentWidget() ) )
  {
    search( job->searchString(), job->parentWidget() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSearch::slotProcessSearchResult( Smb4KShare *share )
{
  Q_ASSERT( share );

  QList<Smb4KShare *> shares = findShareByUNC( share->unc() );

  foreach ( Smb4KShare *s, shares )
  {
    if ( (!s->isForeign() || Smb4KSettings::showAllShares()) && s->isMounted() )
    {
      share->setIsMounted( true );
      break;
    }
    else
    {
      continue;
    }
  }

  // The host of this share should already be known. Set the IP address.
  if ( share->hostIP().isEmpty() )
  {
    Smb4KHost *host = findHost( share->hostName(), share->workgroupName() );

    if ( host )
    {
      share->setHostIP( host->ip() );
    }
    else
    {
      // Should not occur. Do nothing.
    }
  }
  else
  {
    // Do nothing
  }

  emit result( share );
}


void Smb4KSearch::slotAboutToQuit()
{
  abortAll();
}


#include "smb4ksearch.moc"
