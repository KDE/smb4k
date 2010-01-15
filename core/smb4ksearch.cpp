/***************************************************************************
    smb4ksearch  -  This class searches for custom search strings.
                             -------------------
    begin                : So Apr 27 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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
#include <QHostAddress>
#include <QDesktopWidget>
#include <QCoreApplication>

// KDE includes
#include <kshell.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kstandarddirs.h>

// application specific includes
#include <smb4ksearch.h>
#include <smb4kdefs.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4ksambaoptionsinfo.h>
#include <smb4kglobal.h>
#include <smb4kauthinfo.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4kcoremessage.h>
#include <smb4kipaddressscanner.h>
#include <smb4khomesshareshandler.h>
#include <smb4ksearch_p.h>
#include <smb4kwalletmanager.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KSearchPrivate, priv );


Smb4KSearch::Smb4KSearch() : QObject()
{
  m_state = SEARCH_STOP;

  connect( kapp,   SIGNAL( aboutToQuit() ),
           this,   SLOT( slotAboutToQuit() ) );
}


Smb4KSearch::~Smb4KSearch()
{
}


Smb4KSearch *Smb4KSearch::self()
{
  return &priv->instance;
}


void Smb4KSearch::search( const QString &string )
{
  if ( string.isEmpty() )
  {
    return;
  }
  else
  {
    // Go ahead
  }

  // Check the string if it is a IP address and compile the command accordingly.
  QHostAddress address( string.trimmed() );
  Smb4KAuthInfo authInfo;
  QString command;

  if ( address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // Find smbtree program.
    QString smbtree = KStandardDirs::findExe( "smbtree" );

    if ( smbtree.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smbtree" );
      return;
    }
    else
    {
      // Go ahead
    }
    
    // Lookup the workgroup the user is in.
    Smb4KWorkgroup *workgroup = findWorkgroup( Smb4KSettings::domainName() );
    Smb4KHost *master_browser = findHost( workgroup->masterBrowserName(), workgroup->workgroupName() );
    Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( master_browser );
    
    // This is not an IP address. Use the smbtree program.
    command += smbtree;
    command += " -d2";
    
    // Kerberos
    if ( info )
    {
      switch ( info->useKerberos() )
      {
        case Smb4KSambaOptionsInfo::UseKerberos:
        {
          command += " -k";
          break;
        }
        case Smb4KSambaOptionsInfo::NoKerberos:
        {
          // No kerberos 
          break;
        }
        case Smb4KSambaOptionsInfo::UndefinedKerberos:
        {
          command += Smb4KSettings::useKerberos() ? " -k" : "";
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
      command += Smb4KSettings::useKerberos() ? " -k" : "";
    }
    
    // Machine account
    command += Smb4KSettings::machineAccount() ? " -P" : "";
    
    // Signing state
    switch ( Smb4KSettings::signingState() )
    {
      case Smb4KSettings::EnumSigningState::None:
      {
        break;
      }
      case Smb4KSettings::EnumSigningState::On:
      {
        command += " -S on";
        break;
      }
      case Smb4KSettings::EnumSigningState::Off:
      {
        command += " -S off";
        break;
      }
      case Smb4KSettings::EnumSigningState::Required:
      {
        command += " -S required";
        break;
      }
      default:
      {
        break;
      }
    }
    
    // Send broadcasts
    command += Smb4KSettings::smbtreeSendBroadcasts() ? " -b" : "";

    // Authentication information
    if ( master_browser )
    {
      authInfo = Smb4KAuthInfo( master_browser );

      if ( Smb4KSettings::masterBrowsersRequireAuth() )
      {
        Smb4KWalletManager::self()->readAuthInfo( &authInfo );
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

    if ( !authInfo.login().isEmpty() )
    {
      command += " -U "+KShell::quoteArg( authInfo.login() );
    }
    else
    {
      command += " -U %";
    }
  }
  else
  {
    // This is an IP address. Use the nmblookup program.
    
    // Find nmblookup program.
    QString nmblookup = KStandardDirs::findExe( "nmblookup" );

    if ( nmblookup.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "nmblookup" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Find grep program.
    QString grep = KStandardDirs::findExe( "grep" );

    if ( grep.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "grep" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Find sed program.
    QString sed = KStandardDirs::findExe( "sed" );

    if ( sed.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sed" );
      return;
    }
    else
    {
      // Go ahead
    }
    
    // Global Samba options
    QMap<QString,QString> samba_options = Smb4KSambaOptionsHandler::self()->globalSambaOptions();

    // Assemble the command
    command += nmblookup;

    // Domain
    command += (!Smb4KSettings::domainName().isEmpty() &&
               QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0) ?
               QString( " -W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) ) : "";
    
    // NetBIOS name
    command += (!Smb4KSettings::netBIOSName().isEmpty() &&
               QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0) ?
               QString( " -n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) ) : "";
               
    // NetBIOS scope
    command += (!Smb4KSettings::netBIOSScope().isEmpty() &&
               QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0) ?
               QString( " -i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) ) : "";
               
    // Socket options
    command += (!Smb4KSettings::socketOptions().isEmpty() &&
               QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0) ?
               QString( " -O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) ) : "";
               
    // Port 137
    command += Smb4KSettings::usePort137() ? " -r" : "";
    
    // Broadcast address
    QHostAddress address( Smb4KSettings::broadcastAddress() );
    
    command += (!Smb4KSettings::broadcastAddress().isEmpty() &&
               address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) ?
               QString( " -B %1" ).arg( Smb4KSettings::broadcastAddress() ) : "";
    
    // WINS server
    if ( !Smb4KSambaOptionsHandler::self()->winsServer().isEmpty() )
    {
      command += " -R -U "+KShell::quoteArg( Smb4KSambaOptionsHandler::self()->winsServer() );
      command += " "+KShell::quoteArg( string );
      command += " -A | ";
      command += grep+" '<00>' | ";
      command += sed+" -e 's/<00>.*//'";
    }
    else
    {
      command += " "+KShell::quoteArg( string );
      command += " -A | ";
      command += grep+" '<00>' | ";
      command += sed+" -e 's/<00>.*//'";
    }
  }

  if ( m_cache.size() == 0 )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    m_state = SEARCH_START;
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( string );

  SearchThread *thread = new SearchThread( this );
  m_cache.insert( string, thread );

  connect( thread, SIGNAL( finished() ), this, SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( authError( const QString & ) ), this, SLOT( slotAuthError( const QString & ) ) );
  connect( thread, SIGNAL( result( Smb4KBasicNetworkItem * ) ), this, SLOT( slotProcessSearchResult( Smb4KBasicNetworkItem * ) ) );

  thread->start();
  thread->search( string, &authInfo, command );
}


void Smb4KSearch::abort( const QString &string )
{
  Q_ASSERT( !string.isEmpty() );

  SearchThread *thread = m_cache.object( string );

  if ( thread && thread->process() && (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
  {
    thread->process()->abort();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KSearch::isAborted( const QString &string )
{
  Q_ASSERT( !string.isEmpty() );

  SearchThread *thread = m_cache.object( string );
  return (thread && thread->process() && thread->process()->isAborted());
}


void Smb4KSearch::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QStringList keys = m_cache.keys();

    foreach ( const QString &key, keys )
    {
      SearchThread *thread = m_cache.object( key );

      if ( thread->process() && (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
      {
        thread->process()->abort();
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // priv has already been deleted
  }
}


bool Smb4KSearch::isRunning( const QString &string )
{
  Q_ASSERT( !string.isEmpty() );

  SearchThread *thread = m_cache.object( string );
  return (thread && thread->process() && thread->process()->state() == KProcess::Running);
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSearch::slotAboutToQuit()
{
  abortAll();
}


void Smb4KSearch::slotAuthError( const QString &string )
{
  Smb4KAuthInfo authInfo;

  // We need to authenticate to the master browser of the domain
  // the user's computer is in. Get the authentication information.
  Smb4KWorkgroup *workgroup = findWorkgroup( Smb4KSettings::domainName() );

  if ( workgroup )
  {
    authInfo.setWorkgroupName( workgroup->workgroupName() );
    authInfo.setUNC( "//"+workgroup->masterBrowserName() );

    if ( Smb4KSettings::masterBrowsersRequireAuth() &&
         Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
    {
      // Retry the search.
      search( string );
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


void Smb4KSearch::slotProcessSearchResult( Smb4KBasicNetworkItem *item )
{
  switch ( item->type() )
  {
    case Smb4KBasicNetworkItem::Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( item );
      Smb4KHost *known_host = findHost( host->hostName(), host->workgroupName() );

      // Get the IP address if necessary.
      if ( !host->ipChecked() )
      {
        if ( known_host && !known_host->ip().trimmed().isEmpty() )
        {
          host->setIP( known_host->ip() );
        }
        else
        {
          Smb4KIPAddressScanner::self()->lookup( host );
        }
      }
      else
      {
        // Do nothing
      }

      // Add the just discovered host if necessary.
      if ( !known_host )
      {
        addHost( host );
      }
      else
      {
        // Do nothing
      }

      emit result( host, (known_host) );
      break;
    }
    case Smb4KBasicNetworkItem::Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>( item );
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

      // In case this is a 'homes' share, set also the user names.
      if ( QString::compare( share->shareName(), "homes" ) == 0 )
      {
        Smb4KHomesSharesHandler::self()->setHomesUsers( share );
      }
      else
      {
        // Do nothing
      }

      emit result( share, share->isMounted() );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSearch::slotThreadFinished()
{
  QStringList keys = m_cache.keys();

  foreach ( const QString &key, keys )
  {
    SearchThread *thread = m_cache.object( key );

    if ( thread->isFinished() )
    {
      (void) m_cache.take( key );
      emit finished( thread->searchItem() );
      delete thread;
    }
    else
    {
      // Do not touch the thread
    }
  }

  if ( m_cache.size() == 0 )
  {
    m_state = SEARCH_STOP;
    emit stateChanged();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Still running
  }
}


#include "smb4ksearch.moc"
