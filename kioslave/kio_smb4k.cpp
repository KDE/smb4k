/***************************************************************************
    kio_smb4k  -  Main file for the KIO slave Smb4K provides.
                             -------------------
    begin                : Tue 12 Mai 2009
    copyright            : (C) 2009 by Alexander Reinholdt
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
#include <core/smb4kglobal.h>



Smb4KSlave::Smb4KSlave::Smb4KSlave( const QByteArray &pool, const QByteArray &app )
: SlaveBase( "smb4k", pool, app )
{
  m_current_item = Smb4KBasicNetworkItem( Smb4KBasicNetworkItem::Unknown );
}


Smb4KSlave::~Smb4KSlave()
{
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

  KIO::UDSEntry entry;

  m_current_item = currentNetworkItem( u );

  switch ( m_current_item.itemType() )
  {
    case Smb4KBasicNetworkItem::Unknown:
    {
//       // The user called "smb4k:/". Get the workgroups/domains.
//       QList<Smb4KWorkgroup *> workgroups = Smb4KScanner::self()->lookupDomains();
//
//       for ( int i = 0; i < workgroups.size(); ++i )
//       {
//         entry.insert( KIO::UDSEntry::UDS_NAME, workgroups.at( i )->workgroupName() );
//         entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
//         entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
//         entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "application/x-smb-workgroup" ) );
//
//         listEntry( entry, false );
//         entry.clear();
//       }
      break;
    }
    case Smb4KBasicNetworkItem::Workgroup:
    {
//       // The URL is a domain. Get the domain members.
//       Smb4KWorkgroup *workgroup = findWorkgroup( url.host() );
//       QList<Smb4KHost *> hosts = Smb4KScanner::self()->lookupDomainMembers( workgroup );
//
//       for ( int i = 0; i < hosts.size(); ++i )
//       {
//         entry.insert( KIO::UDSEntry::UDS_NAME, hosts.at( i )->hostName() );
//         entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
//         entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
//         entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "application/x-smb-server" ) );
//
//         listEntry( entry, false );
//         entry.clear();
//       }
      break;
    }
    case Smb4KBasicNetworkItem::Host:
    {
//       // FIXME: Also use the workgroup to search for the host.
//       Smb4KHost *host = findHost( u.host() );
//       m_shares = Smb4KScanner::self()->lookupShares( host );
//
//       for ( int i = 0; i < m_shares.size(); ++i )
//       {
//         entry.insert( KIO::UDSEntry::UDS_NAME, m_shares.at( i )->shareName() );
//
//         // Discriminate between normal shares and printers.
//         if ( !m_shares.at( i )->isPrinter() )
//         {
//           entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFBLK );
//           entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
//           entry.insert( KIO::UDSEntry::UDS_ICON_NAME, "folder" );
//         }
//         else
//         {
//           entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFCHR );
//           entry.insert( KIO::UDSEntry::UDS_ACCESS, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH) );
//           entry.insert( KIO::UDSEntry::UDS_ICON_NAME, "printer" );
//         }
//
//         listEntry( entry, false );
//         entry.clear();
//       }
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

  listEntry( entry, true );
  finished();
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

  m_current_item = currentNetworkItem( u );

  switch ( m_current_item.itemType() )
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
      Smb4KShare *share = findShare( share_name, u.host() );

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


void Smb4KSlave::get( const KUrl &url )
{
//   QString share_name = url.path().section( "/", 1, 1 ).trimmed();
//   Smb4KShare *share = findShare( share_name, url.host() );
//
//   // Do NOT use mimeType() here. The slave will then try to
//   // open the shares by itself. This fails and it shows the
//   // Open with... dialog.
//
//   if ( share )
//   {
//     if ( share->isPrinter() )
//     {
//     }
//     else
//     {
//       // Mount or unmount the share.
//       QList<Smb4KShare *> mounted_shares = Smb4KGlobal::findShareByUNC( share->unc() );
//       bool found = false;
//
//       for ( int i = 0; i < mounted_shares.size(); ++i )
//       {
//         if ( !mounted_shares.at( i )->isForeign() )
//         {
//           Smb4KMounter::self()->unmountShare( mounted_shares.at( i ) );
//           found = true;
//           break;
//         }
//         else
//         {
//           continue;
//         }
//       }
//
//       if ( !found )
//       {
//         Smb4KMounter::self()->mountShare( share );
//       }
//       else
//       {
//         // Do nothing
//       }
//
//       // FIXME: Relist the directory so that the new icons can be
//       // shown.
//     }
//   }
//   else
//   {
//     messageBox( KIO::SlaveBase::Information, "No share found" );
//   }

  data( QByteArray() );
  finished();
}


void Smb4KSlave::special( const QByteArray &data )
{
  kDebug(2400) << "special()" << endl;
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
        Smb4KHost *host = findHost( u.path().mid( 1 ).toUpper(), u.host().toUpper() );

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
      Smb4KHost *host = findHost( url.host() );

      if ( host )
      {
        return *host;
      }
      else
      {
        Smb4KWorkgroup *workgroup = findWorkgroup( url.host() );

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
      Smb4KShare *share = findShare( share_name, url.host() );

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


Smb4KWorkgroup *Smb4KSlave::findWorkgroup( const QString &name )
{
  if ( Smb4KGlobal::workgroupsList()->isEmpty() )
  {
    Smb4KScanner::self()->lookupDomains();
  }
  else
  {
    // Do nothing
  }

  return Smb4KGlobal::findWorkgroup( name );
}


Smb4KHost *Smb4KSlave::findHost( const QString &name, const QString &workgroup )
{
  if ( Smb4KGlobal::workgroupsList()->isEmpty() )
  {
    Smb4KScanner::self()->lookupDomains();
  }
  else
  {
    // Do nothing
  }

  if ( Smb4KGlobal::hostsList()->isEmpty() )
  {
    Smb4KWorkgroup *w = findWorkgroup( workgroup );

    if ( w )
    {
      Smb4KScanner::self()->lookupDomainMembers( w );
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

  return Smb4KGlobal::findHost( name, workgroup );
}


Smb4KShare *Smb4KSlave::findShare( const QString &name, const QString &host, const QString &workgroup )
{
//   if ( m_shares.isEmpty() )
//   {
//     // Lookup the shares.
//     Smb4KHost *h = findHost( host, workgroup );
//
//     if ( h )
//     {
//       m_shares = Smb4KScanner::self()->lookupShares( h );
//     }
//     else
//     {
//       // Do nothing
//     }
//   }
//   else
//   {
//     // Do nothing
//   }

  Smb4KShare *share = NULL;

  for ( int i = 0; i < m_shares.size(); ++i )
  {
    if ( !workgroup.isEmpty() &&
         QString::compare( m_shares.at( i )->workgroupName(), workgroup, Qt::CaseInsensitive ) != 0 )
    {
      continue;
    }
    else
    {
      // Go ahead
    }

    if ( QString::compare( m_shares.at( i )->shareName(), name, Qt::CaseInsensitive ) == 0 &&
         QString::compare( m_shares.at( i )->hostName(), host, Qt::CaseInsensitive ) == 0 )
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
