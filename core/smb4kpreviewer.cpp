/***************************************************************************
    smb4kpreviewer  -  This class queries a remote share for a preview
                             -------------------
    begin                : Sa Mär 05 2011
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

// KDE includes
#include <kglobal.h>
#include <kapplication.h>

// application specific includes
#include <smb4kpreviewer.h>
#include <smb4kpreviewer_p.h>
#include <smb4kwalletmanager.h>
#include <smb4kshare.h>
#include <smb4kauthinfo.h>
#include <smb4khomesshareshandler.h>

K_GLOBAL_STATIC( Smb4KPreviewerPrivate, p );


Smb4KPreviewer::Smb4KPreviewer() : KCompositeJob( 0 )
{
  connect( kapp, SIGNAL( aboutToQuit() ), SLOT( slotAboutToQuit() ) );
}


Smb4KPreviewer::~Smb4KPreviewer()
{
}


Smb4KPreviewer *Smb4KPreviewer::self()
{
  return &p->instance;
}


void Smb4KPreviewer::preview( Smb4KShare *share, QWidget *parent )
{
  if ( share->isPrinter() )
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  // Process homes shares.
  if( share->isHomesShare() )
  {
    if ( !Smb4KHomesSharesHandler::self()->specifyUser( share, parent ) )
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
    // Do nothing
  }

  // Check if a preview dialog has already been set up 
  // for this share and reuse it, if appropriate.
  Smb4KPreviewDialog *dlg = NULL;
  
  for ( int i = 0; i < m_dialogs.size(); i++ )
  {
    if ( share == m_dialogs.at( i )->share() )
    {
      dlg = m_dialogs.at( i );
    }
    else
    {
      // Do nothing
    }
  }

  if ( !dlg )
  {
    // Create the preview dialog..
    dlg = new Smb4KPreviewDialog( share, parent );
    connect( dlg,  SIGNAL( aboutToClose( Smb4KPreviewDialog * ) ),
             this, SLOT( slotDialogClosed( Smb4KPreviewDialog * ) ) );
    connect( dlg,  SIGNAL( requestPreview( Smb4KShare *, const QUrl &, QWidget * ) ),
             this, SLOT( slotAcquirePreview( Smb4KShare *, const QUrl &, QWidget * ) ) );
    connect( this, SIGNAL( aboutToStart( Smb4KShare *, const QUrl & ) ),
             dlg,  SLOT( slotAboutToStart( Smb4KShare *, const QUrl & ) ) );
    connect( this, SIGNAL( finished( Smb4KShare *, const QUrl & ) ),
             dlg,  SLOT( slotFinished( Smb4KShare *, const QUrl & ) ) );
    connect( dlg,  SIGNAL( abortPreview( Smb4KShare * ) ),
             this, SLOT( slotAbortPreview( Smb4KShare* ) ) );
    m_dialogs.append( dlg );
  }
  else
  {
    // Do nothing
  }
  
  if ( !dlg->isVisible() )
  {
    dlg->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KPreviewer::isRunning()
{
  return !subjobs().isEmpty();
}


bool Smb4KPreviewer::isRunning( Smb4KShare *share )
{
  bool running = false;

  for ( int i = 0; i < subjobs().size(); i++ )
  {
    if ( QString::compare( QString( "PreviewJob_%1" ).arg( share->unc() ), subjobs().at( i )->objectName() ) == 0 )
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


void Smb4KPreviewer::abortAll()
{
  for ( int i = 0; i < subjobs().size(); i++ )
  {
    subjobs().at( i )->kill( KJob::EmitResult );
  }
}


void Smb4KPreviewer::abort( Smb4KShare *share )
{
  for ( int i = 0; i < subjobs().size(); i++ )
  {
    if ( QString::compare( QString( "PreviewJob_%1" ).arg( share->unc() ), subjobs().at( i )->objectName() ) == 0 )
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


void Smb4KPreviewer::start()
{
  QTimer::singleShot( 0, this, SLOT( slotStartJobs() ) );
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPreviewer::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KPreviewer::slotJobFinished( KJob *job )
{
  disconnect( job );
  removeSubjob( job );
}


void Smb4KPreviewer::slotAuthError( Smb4KPreviewJob *job )
{
  Smb4KAuthInfo authInfo( job->share() );

  if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, job->parentWidget() ) )
  {
    slotAcquirePreview( job->share(), job->location(), job->parentWidget() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewer::slotDialogClosed( Smb4KPreviewDialog *dialog )
{
  if ( dialog )
  {
    // Find the dialog in the list and take it from the list.
    // It will automatically be deleted on close, so there is
    // no need to delete the dialog here.
    int i = m_dialogs.indexOf( dialog );
    m_dialogs.takeAt( i );
  }
  else
  {
    qDebug() << "Dialog already gone.";
  }
}


void Smb4KPreviewer::slotAcquirePreview( Smb4KShare *share, const QUrl &url, QWidget *parent )
{
  // Get the authentication information
  Smb4KAuthInfo authInfo( share );
  Smb4KWalletManager::self()->readAuthInfo( &authInfo );
  share->setAuthInfo( &authInfo );
  
  // Create a new job and add it to the subjobs
  Smb4KPreviewJob *job = new Smb4KPreviewJob( this );
  job->setObjectName( QString( "PreviewJob_%1" ).arg( share->unc() ) );
  job->setupPreview( share, url, parent );

  connect( job,  SIGNAL( result( KJob * ) ),
           this, SLOT( slotJobFinished( KJob * ) ) );
  connect( job,  SIGNAL( authError( Smb4KPreviewJob * ) ),
           this, SLOT( slotAuthError( Smb4KPreviewJob * ) ) );
  connect( job,  SIGNAL( aboutToStart( Smb4KShare *, const QUrl & ) ),
           this, SIGNAL( aboutToStart( Smb4KShare *, const QUrl & ) ) );
  connect( job,  SIGNAL( finished( Smb4KShare *, const QUrl & ) ),
           this, SIGNAL( finished( Smb4KShare *, const QUrl & ) ) );

  // Get the preview dialog, so that the result of the query
  // can be sent.
  Smb4KPreviewDialog *dlg = NULL;

  for ( int i = 0; i < m_dialogs.size(); i++ )
  {
    if ( m_dialogs.at( i ) && m_dialogs.at( i )->share() == share )
    {
      dlg = m_dialogs[i];
      break;
    }
    else
    {
      continue;
    }
  }
  
  if ( dlg )
  {
    connect( job, SIGNAL( preview( const QUrl &, const QList<Item> & ) ),
             dlg, SLOT( slotDisplayPreview( const QUrl &, const QList<Item> & ) ) );
  }
  else
  {
    // Do nothing
  }

  addSubjob( job );

  job->start();
}


void Smb4KPreviewer::slotAbortPreview( Smb4KShare *share )
{
  abort( share );
}


void Smb4KPreviewer::slotAboutToQuit()
{
  abortAll();
}

#include "smb4kpreviewer.moc"
