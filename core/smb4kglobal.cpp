/***************************************************************************
    smb4kglobal  -  This is the global namespace for Smb4K.
                             -------------------
    begin                : Sa Apr 2 2005
    copyright            : (C) 2005-2011 by Alexander Reinholdt
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
#include <QMutex>

// KDE includes
#include <kconfig.h>
#include <kdebug.h>
#include <kurl.h>
#include <krun.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kshell.h>

// application specific includes
#include <smb4kglobal.h>
#include <smb4kglobal_p.h>
#include <smb4kdefs.h>
#include <smb4knotification.h>


static Smb4KGlobalPrivate p;
QMutex mutex( QMutex::Recursive /* needed to avoid dead-locks */ );


int Smb4KGlobal::timerInterval()
{
  return TIMER_INTERVAL;
}


const QList<Smb4KWorkgroup *> &Smb4KGlobal::workgroupsList()
{
  return p.workgroupsList;
}


Smb4KWorkgroup *Smb4KGlobal::findWorkgroup( const QString &name )
{
  Smb4KWorkgroup *workgroup = NULL;

  mutex.lock();

  for ( int i = 0; i < p.workgroupsList.size(); ++i )
  {
    if ( QString::compare( p.workgroupsList.at( i )->workgroupName(),
         name, Qt::CaseInsensitive ) == 0 )
    {
      workgroup = p.workgroupsList.at( i );
      break;
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return workgroup;
}


bool Smb4KGlobal::addWorkgroup( Smb4KWorkgroup *workgroup )
{
  Q_ASSERT( workgroup );

  bool added = false;

  mutex.lock();

  if ( !findWorkgroup( workgroup->workgroupName() ) )
  {
    p.workgroupsList.append( workgroup );
    added = true;
  }
  else
  {
    // Do nothing
  }

  mutex.unlock();

  return added;
}


bool Smb4KGlobal::removeWorkgroup( Smb4KWorkgroup *workgroup )
{
  Q_ASSERT( workgroup );

  bool removed = false;

  mutex.lock();

  int index = p.workgroupsList.indexOf( workgroup );

  if ( index != -1 )
  {
    // The workgroup was found. Remove it.
    delete p.workgroupsList.takeAt( index );
    removed = true;
  }
  else
  {
    // Try harder to find the workgroup.
    Smb4KWorkgroup *wg = findWorkgroup( workgroup->workgroupName() );

    if ( wg )
    {
      index = p.workgroupsList.indexOf( wg );

      if ( index != -1 )
      {
        delete p.workgroupsList.takeAt( index );
        removed = true;
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

    delete workgroup;
  }

  mutex.unlock();

  return removed;
}


void Smb4KGlobal::clearWorkgroupsList()
{
  mutex.lock();

  while ( !p.workgroupsList.isEmpty() )
  {
    delete p.workgroupsList.takeFirst();
  }

  mutex.unlock();
}


const QList<Smb4KHost *> &Smb4KGlobal::hostsList()
{
  return p.hostsList;
}


Smb4KHost *Smb4KGlobal::findHost( const QString &name, const QString &workgroup )
{
  Smb4KHost *host = NULL;

  mutex.lock();

  for ( int i = 0; i < p.hostsList.size(); ++i )
  {
    if ( (workgroup.isEmpty() ||
         QString::compare( p.hostsList.at( i )->workgroupName(),
         workgroup, Qt::CaseInsensitive ) == 0) &&
         QString::compare( p.hostsList.at( i )->hostName(), name,
         Qt::CaseInsensitive ) == 0 )
    {
      host = p.hostsList.at( i );
      break;
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return host;
}


bool Smb4KGlobal::addHost( Smb4KHost *host )
{
  Q_ASSERT( host );

  bool added = false;

  mutex.lock();

  if ( !findHost( host->hostName(), host->workgroupName() ) )
  {
    p.hostsList.append( host );
    added = true;
  }
  else
  {
    // Do nothing
  }

  mutex.unlock();

  return added;
}


bool Smb4KGlobal::removeHost( Smb4KHost *host )
{
  Q_ASSERT( host );

  bool removed = false;

  mutex.lock();

  int index = p.hostsList.indexOf( host );

  if ( index != -1 )
  {
    // The host was found. Remove it.
    delete p.hostsList.takeAt( index );
    removed = true;
  }
  else
  {
    // Try harder to find the host.
    Smb4KHost *h = findHost( host->hostName(), host->workgroupName() );

    if ( h )
    {
      index = p.hostsList.indexOf( h );

      if ( index != -1 )
      {
        delete p.hostsList.takeAt( index );
        removed = true;
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

    delete host;
  }

  mutex.unlock();

  return removed;
}


void Smb4KGlobal::clearHostsList()
{
  mutex.lock();

  while ( !p.hostsList.isEmpty() )
  {
    delete p.hostsList.takeFirst();
  }

  mutex.unlock();
}


QList<Smb4KHost *> Smb4KGlobal::workgroupMembers( Smb4KWorkgroup *workgroup )
{
  QList<Smb4KHost *> hosts;

  mutex.lock();

  for ( int i = 0; i < p.hostsList.size(); ++i )
  {
    if ( QString::compare( p.hostsList.at( i )->workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive ) == 0 )
    {
      hosts += p.hostsList.at( i );
      continue;
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return hosts;
}


const QList<Smb4KShare *> &Smb4KGlobal::sharesList()
{
  return p.sharesList;
}


Smb4KShare *Smb4KGlobal::findShare( const QString &name, const QString &host, const QString &workgroup )
{
  Smb4KShare *share = NULL;

  mutex.lock();

  for ( int i = 0; i < p.sharesList.size(); ++i )
  {
    if ( (workgroup.isEmpty() ||
         QString::compare( p.sharesList.at( i )->workgroupName(), workgroup, Qt::CaseInsensitive ) == 0) &&
         QString::compare( p.sharesList.at( i )->hostName(), host, Qt::CaseInsensitive ) == 0 &&
         QString::compare( p.sharesList.at( i )->shareName(), name, Qt::CaseInsensitive ) == 0 )
    {
      share = p.sharesList.at( i );
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return share;
}


bool Smb4KGlobal::addShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  bool added = false;

  mutex.lock();

  if ( !findShare( share->shareName(), share->hostName(), share->workgroupName() ) )
  {
    p.sharesList.append( share );
    added = true;
  }
  else
  {
    // Do nothing
  }

  mutex.unlock();

  return added;
}


bool Smb4KGlobal::removeShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  bool removed = false;

  mutex.lock();

  int index = p.sharesList.indexOf( share );

  if ( index != -1 )
  {
    // The share was found. Remove it.
    delete p.sharesList.takeAt( index );
    removed = true;
  }
  else
  {
    // Try harder to find the share.
    Smb4KShare *s = findShare( share->shareName(), share->hostName(), share->workgroupName() );

    if ( s )
    {
      index = p.sharesList.indexOf( s );

      if ( index != -1 )
      {
        delete p.sharesList.takeAt( index );
        removed = true;
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

    delete share;
  }

  mutex.unlock();

  return removed;
}


void Smb4KGlobal::clearSharesList()
{
  mutex.lock();

  while ( !p.sharesList.isEmpty() )
  {
    delete p.sharesList.takeFirst();
  }

  mutex.unlock();
}


QList<Smb4KShare *> Smb4KGlobal::sharedResources( Smb4KHost *host )
{
  QList<Smb4KShare *> shares;

  mutex.lock();

  for ( int i = 0; i < p.sharesList.size(); ++i )
  {
    if ( QString::compare( p.sharesList.at( i )->hostName(), host->hostName(), Qt::CaseInsensitive ) == 0 &&
         QString::compare( p.sharesList.at( i )->workgroupName(), host->workgroupName(), Qt::CaseInsensitive ) == 0 )
    {
      shares += p.sharesList.at( i );
      continue;
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return shares;
}


const QList<Smb4KShare *> &Smb4KGlobal::mountedSharesList()
{
  return p.mountedSharesList;
}


Smb4KShare* Smb4KGlobal::findShareByPath( const QByteArray &path )
{
  if ( path.isEmpty() || p.mountedSharesList.isEmpty() )
  {
    return NULL;
  }

  Smb4KShare *share = NULL;

  mutex.lock();

  for ( int i = 0; i < p.mountedSharesList.size(); ++i )
  {
    if ( path.toUpper() == p.mountedSharesList.at( i )->path().toUpper() ||
         path.toUpper() == p.mountedSharesList.at( i )->canonicalPath().toUpper() )
    {
      share = p.mountedSharesList.at( i );
      break;
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return share;
}


QList<Smb4KShare *> Smb4KGlobal::findShareByUNC( const QString &unc )
{
  QList<Smb4KShare *> list;

  if ( unc.isEmpty() || p.mountedSharesList.isEmpty() )
  {
    return list;  // is empty
  }

  QUrl u( unc );

  mutex.lock();

  for ( int i = 0; i < p.mountedSharesList.size(); ++i )
  {
    if ( QString::compare( u.toString( QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort ),
                           p.mountedSharesList.at( i )->unc( QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort ),
                           Qt::CaseInsensitive ) == 0 ||
         QString::compare( u.toString( QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort ).replace( " ", "_" ),
                           p.mountedSharesList.at( i )->unc( QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort ),
                           Qt::CaseInsensitive ) == 0  )
    {
      list.append( p.mountedSharesList.at( i ) );
      continue;
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return list;
}


QList<Smb4KShare*> Smb4KGlobal::findInaccessibleShares()
{
  QList<Smb4KShare *> inaccessible_shares;

  mutex.lock();

  for ( int i = 0; i < p.mountedSharesList.size(); ++i )
  {
    if ( p.mountedSharesList.at( i )->isInaccessible() )
    {
      inaccessible_shares.append( p.mountedSharesList.at( i ) );

      continue;
    }
    else
    {
      continue;
    }
  }

  mutex.unlock();

  return inaccessible_shares;
}


bool Smb4KGlobal::addMountedShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  bool added = false;

  mutex.lock();

  if ( !findShareByPath( share->path() ) )
  {
    p.mountedSharesList.append( share );
    added = true;
  }
  else
  {
    // Do nothing
  }

  mutex.unlock();

  return added;
}


bool Smb4KGlobal::removeMountedShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  bool removed = false;

  mutex.lock();

  int index = p.mountedSharesList.indexOf( share );

  if ( index != -1 )
  {
    // The share was found. Remove it.
    delete p.mountedSharesList.takeAt( index );
    removed = true;
  }
  else
  {
    // Try harder to find the mounted share.
    Smb4KShare *s = findShareByPath( share->path() );

    if ( s )
    {
      index = p.mountedSharesList.indexOf( s );

      if ( index != -1 )
      {
        delete p.mountedSharesList.takeAt( index );
        removed = true;
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

    delete share;
  }

  mutex.unlock();

  return removed;
}


void Smb4KGlobal::open( Smb4KShare *share, OpenWith openWith )
{
  if ( !share || share->isInaccessible() )
  {
    return;
  }

  switch ( openWith )
  {
    case FileManager:
    {
      KUrl url;
      url.setPath( share->canonicalPath() );

      (void) new KRun( url, 0, 0, true );

      break;
    }
    case Konsole:
    {
      QString konsole = KGlobal::dirs()->findResource( "exe", "konsole" );

      if ( konsole.isEmpty() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->commandNotFound( "konsole" );
      }
      else
      {
        KRun::runCommand( konsole+" --workdir "+KShell::quoteArg( share->canonicalPath() ), 0 );
      }

      break;
    }
    default:
    {
      break;
    }
  }
}


KUiServerJobTracker *Smb4KGlobal::jobTracker()
{
  return &p.jobTracker;
}


