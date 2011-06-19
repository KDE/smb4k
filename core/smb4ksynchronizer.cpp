/***************************************************************************
    smb4ksynchronizer  -  This is the new synchronizer of Smb4K.
                             -------------------
    begin                : Fr Feb 04 2011
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
#include <QFile>
#include <QDir>
#include <QCoreApplication>

// KDE includes
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kshell.h>

// application specific includes
#include <smb4ksynchronizer.h>
#include <smb4ksynchronizer_p.h>
#include <smb4knotification.h>
#include <smb4kglobal.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KSynchronizerPrivate, p );


Smb4KSynchronizer::Smb4KSynchronizer() : KCompositeJob( 0 )
{
  connect( QCoreApplication::instance(), SIGNAL( aboutToQuit() ), SLOT( slotAboutToQuit() ) );
}


Smb4KSynchronizer::~Smb4KSynchronizer()
{
}


Smb4KSynchronizer *Smb4KSynchronizer::self()
{
  return &p->instance;
}


void Smb4KSynchronizer::synchronize( Smb4KShare *share, QWidget *parent )
{
  if ( !isRunning( share ) )
  {
    // Create a new job, add it to the subjobs and register it
    // with the job tracker.
    Smb4KSyncJob *job = new Smb4KSyncJob( this );
    job->setObjectName( QString( "SyncJob_%1" ).arg( QString::fromUtf8( share->canonicalPath() ) ) );
    job->setupSynchronization( share, parent );
    
    connect( job, SIGNAL( result( KJob * ) ), SLOT( slotJobFinished( KJob * ) ) );
    connect( job, SIGNAL( aboutToStart( const QString & ) ), SIGNAL( aboutToStart( const QString & ) ) );
    connect( job, SIGNAL( finished( const QString & ) ), SIGNAL( finished( const QString & ) ) );

    addSubjob( job );
    
    job->start();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KSynchronizer::isRunning()
{
  return !subjobs().isEmpty();
}


bool Smb4KSynchronizer::isRunning( Smb4KShare *share )
{
  bool running = false;

  for ( int i = 0; i < subjobs().size(); ++i )
  {
    if ( QString::compare( QString( "SyncJob_%1" ).arg( QString::fromUtf8( share->canonicalPath() ) ), subjobs().at( i )->objectName() ) == 0 )
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


void Smb4KSynchronizer::abortAll()
{
  for ( int i = 0; i < subjobs().size(); ++i )
  {
    subjobs().at( i )->kill( KJob::EmitResult );
  }
}


void Smb4KSynchronizer::abort( Smb4KShare *share )
{
  for ( int i = 0; i < subjobs().size(); ++i )
  {
    if ( QString::compare( QString( "SyncJob_%1" ).arg( QString::fromUtf8( share->canonicalPath() ) ), subjobs().at( i )->objectName() ) == 0 )
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


void Smb4KSynchronizer::start()
{
  QTimer::singleShot( 0, this, SLOT( slotStartJobs() ) );
}

/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSynchronizer::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KSynchronizer::slotJobFinished( KJob *job )
{
  // Remove the job.
  removeSubjob( job );
}


void Smb4KSynchronizer::slotAboutToQuit()
{
  abortAll();
}


#include "smb4ksynchronizer.moc"
