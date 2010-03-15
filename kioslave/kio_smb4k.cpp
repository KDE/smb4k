/***************************************************************************
    kio_smb4k  -  Main file for the KIO slave Smb4K provides.
                             -------------------
    begin                : Tue 12 Mai 2009
    copyright            : (C) 2009-2010 by Alexander Reinholdt
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
#include <QDir>

// KDE includes
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/udsentry.h>

// application specific includes
#include <kio_smb4k.h>
#include <core/smb4kscanner.h>
#include <core/smb4kmounter.h>
#include <core/smb4kworkgroup.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>


class Smb4KSlavePrivate
{
  public:
    Smb4KSlavePrivate( QObject *parent ) : eventLoop( parent ) {}
    ~Smb4KSlavePrivate() {}
    Smb4KSlave *slave;
    QEventLoop eventLoop;
    
    Smb4KBasicNetworkItem currentItem;
    
    void addWorkgroup( Smb4KWorkgroup *workgroup );
    void addHost( Smb4KHost *host );
    void addShare( Smb4KShare *share );
    
    // FIXME: Add clear functions!?
    
    Smb4KWorkgroup *findWorkgroup( const QString &name );
    Smb4KHost *findHost( const QString &name, const QString &workgroup = QString() );
    Smb4KShare *findShare( const QString &name, const QString &host, const QString &workgroup = QString() );
    
    void slotWorkgroups( const QList<Smb4KWorkgroup *> &list );
    void slotHosts( Smb4KWorkgroup *workgroup, const QList<Smb4KHost *> &list );
    void slotShares( Smb4KHost *host, const QList<Smb4KShare *> &list );
    void slotScannerFinished( Smb4KBasicNetworkItem *item, int process );
    
  private:
    QList<Smb4KWorkgroup *> m_workgroups;
    QList<Smb4KHost *> m_hosts;
    QList<Smb4KShare *> m_shares;
};


void Smb4KSlavePrivate::addWorkgroup( Smb4KWorkgroup *workgroup )
{
  if ( !m_workgroups.isEmpty() )
  {
    for ( int i = 0; i < m_workgroups.size(); ++i )
    {
      if ( QString::compare( m_workgroups.at( i )->workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive ) == 0 )
      {
        delete m_workgroups.takeAt( i );
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
  
  m_workgroups += workgroup;
}


void Smb4KSlavePrivate::addHost( Smb4KHost *host )
{
  if ( !m_hosts.isEmpty() )
  {
    for ( int i = 0; i < m_hosts.size(); ++i )
    {
      if ( QString::compare( m_hosts.at( i )->hostName(), host->hostName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( m_hosts.at( i )->workgroupName(), host->workgroupName(), Qt::CaseInsensitive ) == 0 )
      {
        delete m_hosts.takeAt( i );
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
  
  m_hosts += host;
}


void Smb4KSlavePrivate::addShare( Smb4KShare *share )
{
  if ( !m_shares.isEmpty() )
  {
    for ( int i = 0; i < m_shares.size(); ++i )
    {
      if ( QString::compare( m_shares.at( i )->shareName(), share->shareName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( m_shares.at( i )->hostName(), share->hostName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( m_shares.at( i )->workgroupName(), share->workgroupName(), Qt::CaseInsensitive ) == 0 )
      {
        delete m_shares.takeAt( i );
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
  
  m_shares += share;
}


Smb4KWorkgroup *Smb4KSlavePrivate::findWorkgroup( const QString &name )
{
  Smb4KWorkgroup *workgroup = NULL;
  
  for ( int i = 0; i < m_workgroups.size(); ++i )
  {
    if ( QString::compare( m_workgroups.at( i )->workgroupName(), name, Qt::CaseInsensitive ) == 0 )
    {
      workgroup = m_workgroups[i];
      break;
    }
    else
    {
      continue;
    }
  }
  
  return workgroup;
}


Smb4KHost *Smb4KSlavePrivate::findHost( const QString &name, const QString &workgroup )
{
  Smb4KHost *host = NULL;
  
  for ( int i = 0; i < m_hosts.size(); ++i )
  {
    if ( QString::compare( m_hosts.at( i )->hostName(), name, Qt::CaseInsensitive ) == 0 &&
         (workgroup.isEmpty() || 
         QString::compare( m_hosts.at( i )->workgroupName(), workgroup, Qt::CaseInsensitive ) == 0) )
    {
      host = m_hosts[i];
      break;
    }
    else
    {
      continue;
    }
  }
  
  return host;
}


Smb4KShare *Smb4KSlavePrivate::findShare( const QString &name, const QString &host, const QString &workgroup )
{
  Smb4KShare *share = NULL;
  
  for ( int i = 0; i < m_shares.size(); ++i )
  {
    if ( QString::compare( m_shares.at( i )->shareName(), name, Qt::CaseInsensitive ) == 0 &&
         QString::compare( m_shares.at( i )->hostName(), host, Qt::CaseInsensitive ) == 0 &&
         (workgroup.isEmpty() ||
         QString::compare( m_shares.at( i )->workgroupName(), workgroup, Qt::CaseInsensitive ) == 0) )
    {
      share = m_shares[i];
      break;
    }
    else
    {
      continue;
    }
  }
  
  return share;
}


void Smb4KSlavePrivate::slotWorkgroups( const QList<Smb4KWorkgroup *> &list )
{
  KIO::UDSEntry entry;
  
  for ( int i = 0; i < list.size(); ++i )
  {
    entry.insert( KIO::UDSEntry::UDS_NAME, list.at( i )->workgroupName() );
    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
    entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "application/x-smb-workgroup" ) );

    slave->listEntry( entry, false );
    entry.clear();
  }
  
  slave->listEntry( entry, true );
  slave->finished();
}


void Smb4KSlavePrivate::slotHosts( Smb4KWorkgroup */*workgroup*/, const QList<Smb4KHost *> &list )
{
  KIO::UDSEntry entry;
  
  for ( int i = 0; i < list.size(); ++i )
  {
    entry.insert( KIO::UDSEntry::UDS_NAME, list.at( i )->hostName() );
    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
    entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "application/x-smb-server" ) );

    slave->listEntry( entry, false );
    entry.clear();
  }
  
  slave->listEntry( entry, true );
  slave->finished();
}


void Smb4KSlavePrivate::slotShares( Smb4KHost */*host*/, const QList<Smb4KShare *> &list )
{
  // FIXME: Copy the list of shares to the private list used by
  // Smb4KSlave::findShare().
  
  KIO::UDSEntry entry;

  for ( int i = 0; i < list.size(); ++i )
  {
    entry.insert( KIO::UDSEntry::UDS_NAME, list.at( i )->shareName() );

    // Discriminate between normal shares and printers.
    if ( !list.at( i )->isPrinter() )
    {
      entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFBLK );
      entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
      entry.insert( KIO::UDSEntry::UDS_ICON_NAME, "folder" );
    }
    else
    {
      entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFCHR );
      entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH) );
      entry.insert( KIO::UDSEntry::UDS_ICON_NAME, "printer" );
    }

    slave->listEntry( entry, false );
    entry.clear();
  }
  
  slave->listEntry( entry, true );
  slave->finished();
}


void Smb4KSlavePrivate::slotScannerFinished( Smb4KBasicNetworkItem *item, int process )
{
  slave->messageBox( KIO::SlaveBase::Information, "Scanner finished" );
  
  eventLoop.exit();
}



Smb4KSlave::Smb4KSlave::Smb4KSlave( const QByteArray &pool, const QByteArray &app )
: QObject(), SlaveBase( "smb4k", pool, app ), m_priv( new Smb4KSlavePrivate( this ) )
{
  m_priv->slave = this;
  m_priv->currentItem = Smb4KBasicNetworkItem( Smb4KBasicNetworkItem::Unknown );
}


Smb4KSlave::~Smb4KSlave()
{
  delete m_priv;
}


void Smb4KSlave::listDir( const KUrl &url )
{
  KUrl u = checkURL( url );

  if ( u != url )
  {
    redirection( u );
    finished();
    return;
  }
  else
  {
    // Go ahead
  }

  m_priv->currentItem = currentNetworkItem( u );

  switch ( m_priv->currentItem.type() )
  {
    case Smb4KBasicNetworkItem::Unknown:
    {
      Smb4KScanner::self()->lookupDomains();
      m_priv->slave->connect( Smb4KScanner::self(), SIGNAL( workgroups( const QList<Smb4KWorkgroup *> & ) ),
                              SLOT( slotWorkgroups( const QList<Smb4KWorkgroup *> & ) ) );
      m_priv->slave->connect( Smb4KScanner::self(), SIGNAL( finished( Smb4KBasicNetworkItem *, int ) ),
                              SLOT( slotScannerFinished( Smb4KBasicNetworkItem *, int ) ) );
      m_priv->eventLoop.exec();
      break;
    }
    case Smb4KBasicNetworkItem::Workgroup:
    {
      Smb4KWorkgroup *workgroup = m_priv->findWorkgroup( url.host() );
      Smb4KScanner::self()->lookupDomainMembers( workgroup );
      m_priv->slave->connect( Smb4KScanner::self(), SIGNAL( hosts( Smb4KWorkgroup *, const QList<Smb4KHost *> & ) ),
                              SLOT( slotHosts( Smb4KWorkgroup *, const QList<Smb4KHost *> & ) ) );
      m_priv->slave->connect( Smb4KScanner::self(), SIGNAL( finished( Smb4KBasicNetworkItem *, int ) ),
                              SLOT( slotScannerFinished( Smb4KBasicNetworkItem *, int ) ) );
      m_priv->eventLoop.exec();
      break;
    }
    case Smb4KBasicNetworkItem::Host:
    {
      // FIXME: Also use the workgroup to search for the host.
      Smb4KHost *host = m_priv->findHost( u.host() );
      Smb4KScanner::self()->lookupShares( host );
      m_priv->slave->connect( Smb4KScanner::self(), SIGNAL( shares( Smb4KHost *, const QList<Smb4KShare *> & ) ),
                              SLOT( slotShares( Smb4KHost *, const QList<Smb4KShare *> & ) ) );
      m_priv->slave->connect( Smb4KScanner::self(), SIGNAL( finished( Smb4KBasicNetworkItem *, int ) ),
                              SLOT( slotScannerFinished( Smb4KBasicNetworkItem *, int ) ) );
      break;
    }
    case Smb4KBasicNetworkItem::Share:
    {
      messageBox( KIO::SlaveBase::Information, "listDir(): share" );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSlave::stat( const KUrl &url )
{
  KUrl u = checkURL( url );

  if ( u != url )
  {
    redirection( u );
    finished();
    return;
  }
  else
  {
    // Go ahead
  }

  KIO::UDSEntry entry;

  m_priv->currentItem = currentNetworkItem( u );

  switch ( m_priv->currentItem.type() )
  {
    case Smb4KBasicNetworkItem::Unknown:
    case Smb4KBasicNetworkItem::Workgroup:
    case Smb4KBasicNetworkItem::Host:
    {
      // Entire network, workgroup/domain or server without prepended domain.
      entry.insert( KIO::UDSEntry::UDS_NAME, u.host() );
      entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
      entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
      break;
    }
    case Smb4KBasicNetworkItem::Share:
    {
      QString share_name = u.path().section( "/", 1, 1 );
      Smb4KShare *share = m_priv->findShare( share_name, u.host() );

      if ( share )
      {
        entry.insert( KIO::UDSEntry::UDS_NAME, share->shareName() );

        // Discriminate between normal shares and printers.
        if ( !share->isPrinter() )
        {
          entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFBLK );
          entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
        }
        else
        {
          entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFCHR );
          entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH) );
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

  statEntry( entry );
  finished();
}


KUrl Smb4KSlave::checkURL( const KUrl &url )
{
  // FIXME: If url.host() contains a space, redirect to the
  // network and show an error message, that the members of
  // this workgroup/host cannot be listed.

  QString smb4k_url = url.url();

  if ( smb4k_url.startsWith( "smb4k:/" ) )
  {
    if ( smb4k_url.length() != 7 && smb4k_url.at( 7 ) != '/' )
    {
      smb4k_url = "smb4k://"+smb4k_url.mid( 7 );
    }
    else
    {
      // Do nothing
    }

    KUrl u( smb4k_url );

    if ( !u.hasPath() )
    {
      smb4k_url = smb4k_url+"/";
    }
    else
    {
      if ( QString::compare( u.path(), "/" ) != 0 )
      {
        // If u.path() represents a host, replace the workgroup
        // entry (url.host()) with it.
        Smb4KHost *host = m_priv->findHost( u.path().mid( 1 ).toUpper(), u.host().toUpper() );

        if ( host )
        {
          smb4k_url = smb4k_url.replace( u.host(), host->hostName(), Qt::CaseInsensitive );
          smb4k_url = smb4k_url.section( "/", 0, 2 )+"/";
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

    // FIXME: Should we check for user and pass here?

    u.setUrl( smb4k_url );

    return u;
  }
  else
  {
    // Do nothing
  }

  // Something is not right. Redirect the user to his/her
  // home directory.
  return KUrl( QDir::homePath() );
}


Smb4KBasicNetworkItem Smb4KSlave::currentNetworkItem( const KUrl &url )
{
  // Check what the incoming URL represents.
  if ( !url.authority().isEmpty() )
  {
    if ( !url.hasPath() || QString::compare( url.path(), "/" ) == 0 )
    {
      // Check if the host entry is a host or a workgroup/domain.
      Smb4KHost *host = m_priv->findHost( url.host() );

      if ( host )
      {
        return *host;
      }
      else
      {
        Smb4KWorkgroup *workgroup = m_priv->findWorkgroup( url.host() );

        if ( workgroup )
        {
          return *workgroup;
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // FIXME: Discriminate between share and path.
      QString share_name = url.path().section( "/", 1, 1 ).trimmed();
      Smb4KShare *share = m_priv->findShare( share_name, url.host() );

      if ( share )
      {
        return *share;
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

  return Smb4KBasicNetworkItem( Smb4KBasicNetworkItem::Unknown );
}


extern "C"
{
  int KDE_EXPORT kdemain( int argc, char **argv )
  {
    KAboutData aboutData( "kio_smb4k", 0, ki18n( "Network Neighborhood" ), 0 );
    KCmdLineArgs::init( &aboutData );
    KApplication app;

    if ( argc != 4 )
    {
      kDebug() << "Usage: kio_smb4k protocol domain-socket1 domain-socket2" << endl;
      return -1;
    }
    else
    {
      // Go ahead
    }

    Smb4KSlave slave( argv[2], argv[3] );
    slave.dispatchLoop();

    return 0;
  }
}

#include "kio_smb4k.moc"
