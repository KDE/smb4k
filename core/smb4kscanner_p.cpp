/***************************************************************************
    smb4kscanner_p  -  This is a private helper class for Smb4KScanner.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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

// KDE includes
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include <smb4kscanner_p.h>
#include <smb4kcoremessage.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kshare.h>


BasicScanThread::BasicScanThread( Type type, Smb4KBasicNetworkItem *item, QObject *parent )
: QThread( parent ), m_item( item ), m_type( type ), m_auth_error( false )
{
  m_proc = NULL;
}


BasicScanThread::~BasicScanThread()
{
}



LookupDomainsThread::LookupDomainsThread( Mode mode, QObject* parent )
: BasicScanThread( BasicScanThread::LookupDomainThread, NULL, parent ), m_mode( mode )
{
  Q_ASSERT( mode != Unknown );
}


LookupDomainsThread::~LookupDomainsThread()
{
  m_workgroups.clear();
  m_hosts.clear();
}


void LookupDomainsThread::lookup( const QString &command )
{
  Q_ASSERT( !command.isEmpty() );

  m_proc = new Smb4KProcess( Smb4KProcess::LookupDomains, this );

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->start();
}


void LookupDomainsThread::processLookupDomains()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

  if ( !stdout.isEmpty() )
  {
    Smb4KWorkgroup workgroup;

    foreach ( const QString &line, stdout )
    {
      if ( line.startsWith( "Looking up status of" ) )
      {
        // Get the IP address of the master browser.
        workgroup.setMasterBrowserIP( line.section( "of", 1, 1 ).trimmed() );
        continue;
      }
      else if ( line.contains( "MAC Address", Qt::CaseSensitive ) )
      {
        // Add workgroup to the list. Ignore misconfigured master browsers,
        // that do not belong to a workgroup/domain, i.e. where the workgroup
        // name is empty.
        if ( !workgroup.workgroupName().isEmpty() && !workgroup.masterBrowserName().isEmpty() )
        {
          m_workgroups.append( workgroup );
        }
        else
        {
          // Do nothing
        }

        workgroup = Smb4KWorkgroup();
        continue;
      }
      else if ( line.contains( " <00> ", Qt::CaseSensitive ) )
      {
        // Set the name of the workgroup/host.
        if ( line.contains( " <GROUP> ", Qt::CaseSensitive ) )
        {
          // Avoid setting the workgroup name twice.
          if ( workgroup.workgroupName().isEmpty() )
          {
            workgroup.setWorkgroupName( line.section( "<00>", 0, 0 ).trimmed() );
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Avoid setting the name of the master browser twice.
          if ( workgroup.masterBrowserName().isEmpty() )
          {
            workgroup.setMasterBrowserName( line.section( "<00>", 0, 0 ).trimmed() );
          }
          else
          {
            // Do nothing
          }
        }

        continue;
      }
      else if ( line.contains( " <1d> ", Qt::CaseSensitive ) )
      {
        // Get the workgroup name.
        if ( workgroup.workgroupName().isEmpty() )
        {
          workgroup.setWorkgroupName( line.section( "<1d>", 0, 0 ).trimmed() );
        }
        else
        {
          // Do nothing
        }

        continue;
      }
      else if ( line.contains( "__MSBROWSE__", Qt::CaseSensitive ) &&
                line.contains( " <01> ", Qt::CaseSensitive ) )
      {
        // The host is a master browser.
        workgroup.setHasPseudoMasterBrowser( false );
        continue;
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

  emit workgroups( m_workgroups );
}


void LookupDomainsThread::processQueryMaster()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

  if ( !stdout.isEmpty() )
  {
    Smb4KWorkgroup workgroup;

    foreach ( const QString &line, stdout )
    {
      if ( line.trimmed().startsWith( "Enumerating" ) )
      {
        continue;
      }
      else if ( line.trimmed().startsWith( "Domain name" ) )
      {
        continue;
      }
      else if ( line.trimmed().startsWith( "-------------" ) )
      {
        continue;
      }
      else if ( line.trimmed().isEmpty() )
      {
        continue;
      }
      else
      {
        // This is the workgroup and master entry. Process it.
        workgroup.setWorkgroupName( line.section( "   ", 0, 0 ).trimmed() );
        workgroup.setMasterBrowserName( line.section( "   ", 1, -1 ).trimmed() );
        workgroup.setHasPseudoMasterBrowser( false );

        m_workgroups.append( workgroup );

        workgroup = Smb4KWorkgroup();
        continue;
      }
    }
  }
  else
  {
    // Do nothing
  }

  emit workgroups( m_workgroups );
}


void LookupDomainsThread::processScanBroadcastAreas()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).trimmed().split( "\n", QString::SkipEmptyParts );

  Smb4KWorkgroup workgroup;
  Smb4KHost host;

  foreach ( const QString &line, stdout )
  {
    if ( line.startsWith( "Looking up status of" ) )
    {
      // Get the IP address of the host.
      host.setIP( line.section( "of", 1, 1 ).trimmed() );
      continue;
    }
    else if ( line.contains( "MAC Address", Qt::CaseSensitive ) )
    {
      // Check that the workgroup object carries a workgroup
      // and a master browser.
      if ( !workgroup.workgroupName().isEmpty() && !workgroup.masterBrowserName().isEmpty() )
      {
        bool workgroup_found = false;

        for ( int i = 0; i < m_workgroups.size(); ++i )
        {
          if ( QString::compare( m_workgroups.at( i ).workgroupName(), workgroup.workgroupName(), Qt::CaseInsensitive ) == 0 )
          {
            workgroup_found = true;
            break;
          }
          else
          {
            continue;
          }
        }

        if ( !workgroup_found )
        {
          m_workgroups.append( workgroup );
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

      m_hosts.append( host );

      workgroup = Smb4KWorkgroup();
      host = Smb4KHost();
      continue;
    }
    else if ( line.contains( " <00> ", Qt::CaseSensitive ) )
    {
      // Set the name of the workgroup/host.
      if ( line.contains( " <GROUP> ", Qt::CaseSensitive ) )
      {
        workgroup.setWorkgroupName( line.section( "<00>", 0, 0 ).trimmed() );
        host.setWorkgroupName( line.section( "<00>", 0, 0 ).trimmed() );
      }
      else
      {
        host.setHostName( line.section( "<00>", 0, 0 ).trimmed() );
      }

      continue;
    }
    else if ( line.contains( "__MSBROWSE__", Qt::CaseSensitive ) )
    {
      if ( line.contains( " <01> ", Qt::CaseSensitive ) )
      {
        // The host is a master browser.
        workgroup.setMasterBrowser( host.hostName(), host.ip(), false );
        host.setIsMasterBrowser( true );
      }
      else
      {
        if ( workgroup.masterBrowserName().isEmpty() )
        {
          // Set this server as pseudo master browser as long as we do not
          // have the correct one.
          workgroup.setMasterBrowser( host.hostName(), host.ip(), true );
        }
        else
        {
          // Do nothing
        }
      }
      continue;
    }
    else
    {
      continue;
    }
  }

  emit workgroups( m_workgroups );
  emit hosts( static_cast<Smb4KWorkgroup *>( m_item ) /* NULL */, m_hosts );
}


void LookupDomainsThread::slotProcessError()
{
  // Read from stderr and decide what to do.
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    switch ( m_mode )
    {
      case LookupDomainsThread::LookupDomains:
      case LookupDomainsThread::QueryMaster:
      {
        Smb4KCoreMessage::error( ERROR_GETTING_WORKGROUPS, QString(), stderr );
        break;
      }
      case LookupDomainsThread::ScanBroadcastAreas:
      {
        Smb4KCoreMessage::error( ERROR_PERFORMING_IPSCAN, QString(), stderr );
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
    // Do nothing
  }
}


void LookupDomainsThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      switch ( m_mode )
      {
        case LookupDomains:
        {
          processLookupDomains();
          break;
        }
        case QueryMaster:
        {
          processQueryMaster();
          break;
        }
        case ScanBroadcastAreas:
        {
          processScanBroadcastAreas();
          break;
        }
        default:
        {
          break;
        }
      }
      break;
    }
  }

  exit( exitCode );
}


LookupMembersThread::LookupMembersThread( Smb4KWorkgroup *workgroup, QObject *parent )
: BasicScanThread( BasicScanThread::LookupMembersThread, workgroup, parent )
{
  Q_ASSERT( workgroup );
  m_auth_info = Smb4KAuthInfo();
}


LookupMembersThread::~LookupMembersThread()
{
}


void LookupMembersThread::lookup( bool auth_required, Smb4KAuthInfo *authInfo, const QString &command )
{
  Q_ASSERT( !command.isEmpty() );

  m_auth_info = *authInfo;

  m_proc = new Smb4KProcess( Smb4KProcess::LookupDomainMembers, this );

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setEnv( "PASSWD", (auth_required && !authInfo->password().isEmpty()) ? authInfo->password() : "", true );
  m_proc->start();
}


void LookupMembersThread::slotProcessError()
{
  // Read from stderr and decide what to do.
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    if ( stderr.contains( "The username or password was not correct." ) ||
         stderr.contains( "NT_STATUS_ACCOUNT_DISABLED" ) /* AD error */ ||
         stderr.contains( "NT_STATUS_ACCESS_DENIED" ) ||
         stderr.contains( "NT_STATUS_LOGON_FAILURE" ) )
    {
      m_auth_error = true;
    }
    else
    {
      // Notify the user that an error occurred.
      Smb4KCoreMessage::error( ERROR_GETTING_MEMBERS, QString(), stderr );
    }
  }
  else
  {
    // Do nothing
  }
}


void LookupMembersThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  switch( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );
      Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( m_item );
      Smb4KHost host;

      foreach ( const QString &line, stdout )
      {
        if ( line.trimmed().startsWith( "Enumerating" ) )
        {
          continue;
        }
        else if ( line.trimmed().startsWith( "Server name" ) )
        {
          continue;
        }
        else if ( line.trimmed().startsWith( "-------------" ) )
        {
          continue;
        }
        else
        {
          // FIXME: Work around QUrl problems with hosts that contain spaces.
          // If you try to set a host name containing a space, you'll end up
          // with an empty host name in the QUrl object.
          if ( !line.section( "   ", 0, 0 ).trimmed().contains( " " ) )
          {
            host.setHostName( line.section( "   ", 0, 0 ).trimmed() );
            host.setWorkgroupName( workgroup->workgroupName() );
            host.setComment( line.section( "   ", 1, -1 ).trimmed() );

            if ( QString::compare( host.hostName(), workgroup->masterBrowserName() ) == 0 )
            {
              host.setLogin( m_auth_info.login() );
              host.setIsMasterBrowser( true );

              if ( workgroup->hasMasterBrowserIP() )
              {
                host.setIP( workgroup->masterBrowserIP() );
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              host.setIsMasterBrowser( false );
            }

            m_hosts.append( host );
          }
          else
          {
            qDebug() << "Cannot handle host names containing spaces. Omitting.";
          }
          host = Smb4KHost();
          continue;
        }
      }

      emit hosts( workgroup, m_hosts );

      break;
    }
  }

  exit( exitCode );
}



LookupSharesThread::LookupSharesThread( Smb4KHost *host, QObject *parent )
: BasicScanThread ( BasicScanThread::LookupSharesThread, host, parent )
{
  Q_ASSERT( host );
  m_auth_info = Smb4KAuthInfo();
}



LookupSharesThread::~LookupSharesThread()
{
}


void LookupSharesThread::lookup( Smb4KAuthInfo *authInfo, const QString &command )
{
  Q_ASSERT( !command.isEmpty() );

  m_auth_info = *authInfo;

  m_proc = new Smb4KProcess( Smb4KProcess::LookupShares, this );

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  // Set the password if required.
  m_proc->setEnv( "PASSWD", !authInfo->password().isEmpty() ? authInfo->password() : "", true );
  m_proc->start();
}


void LookupSharesThread::slotProcessError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    if ( stderr.contains( "The username or password was not correct." ) ||
         stderr.contains( "NT_STATUS_ACCOUNT_DISABLED" ) /* AD error */ ||
         stderr.contains( "NT_STATUS_ACCESS_DENIED" ) ||
         stderr.contains( "NT_STATUS_LOGON_FAILURE" ) )
    {
      m_auth_error = true;
    }
    else if ( stderr.contains( "could not obtain sid for domain", Qt::CaseSensitive ) )
    {
      // FIXME
      qDebug() << "FIXME: Wrong protocol used for host " << static_cast<Smb4KHost *>( m_item )->hostName() << "..." << endl;
    }
    else
    {
      if ( !stderr.contains( "creating lame", Qt::CaseSensitive ) )
      {
        Smb4KCoreMessage::error( ERROR_GETTING_SHARES, QString(), stderr );
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
}


void LookupSharesThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  switch( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // Additional authentication error handling.
      if ( exitCode == 104 /* access denied in W2k3 domain */ ||
           exitCode == 235 /* wrong password in W2k3 domain */ )
      {
        m_auth_error = true;
      }
      else
      {
        QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );
        Smb4KHost *host = static_cast<Smb4KHost *>( m_item );
        Smb4KShare share;

        foreach ( const QString &line, stdout )
        {
          if ( line.trimmed().startsWith( "Enumerating" ) )
          {
            continue;
          }
          else if ( line.trimmed().startsWith( "Share name" ) )
          {
            continue;
          }
          else if ( line.trimmed().startsWith( "----------" ) )
          {
            continue;
          }
          else if ( line.contains( " Disk     ", Qt::CaseSensitive ) /* line has comment */ ||
                    (!line.contains( " Disk     ", Qt::CaseSensitive ) &&
                     line.trimmed().endsWith( " Disk", Qt::CaseSensitive ) /* line has no comment */) )
          {
            if ( !line.trimmed().endsWith( " Disk", Qt::CaseSensitive ) )
            {
              share.setShareName( line.section( " Disk     ", 0, 0 ).trimmed() );
              share.setComment( line.section( " Disk     ", 1, 1 ).trimmed() );
            }
            else
            {
              share.setShareName( line.section( " Disk", 0, 0 ).trimmed() );
              share.setComment( "" );
            }
            
            share.setHostName( host->hostName() );
            share.setWorkgroupName( host->workgroupName() );
            share.setTypeString( "Disk" );
            share.setLogin( m_auth_info.login() );

            if ( host->ipChecked() && host->hasIP() )
            {
              share.setHostIP( host->ip() );
            }
            else
            {
              // Do nothing
            }

            m_shares.append( share );
            share = Smb4KShare();
            continue;
          }
          else if ( line.contains( " IPC      ", Qt::CaseSensitive ) /* line has comment */ ||
                    (!line.contains( " IPC      ", Qt::CaseSensitive ) &&
                     line.trimmed().endsWith( " IPC", Qt::CaseSensitive ) /* line has no comment */) )
          {
            if ( !line.trimmed().endsWith( " IPC", Qt::CaseSensitive ) )
            {
              share.setShareName( line.section( " IPC      ", 0, 0 ).trimmed() );
              share.setComment( line.section( " IPC      ", 1, 1 ).trimmed() );
            }
            else
            {
              share.setShareName( line.section( " IPC", 0, 0 ).trimmed() );
              share.setComment( "" );
            }
            
            share.setHostName( host->hostName() );
            share.setWorkgroupName( host->workgroupName() );
            share.setTypeString( "IPC" );
            share.setLogin( m_auth_info.login() );

            if ( host->ipChecked() && host->hasIP() )
            {
              share.setHostIP( host->ip() );
            }
            else
            {
              // Do nothing
            }

            m_shares.append( share );
            share = Smb4KShare();
            continue;
          }
          else if ( line.contains( " Print    ", Qt::CaseSensitive ) /* line has comment */ ||
                    (!line.contains( " Print    ", Qt::CaseSensitive ) &&
                     line.trimmed().endsWith( " Print", Qt::CaseSensitive ) /* line has no comment */) )
          {
            if ( !line.trimmed().endsWith( " Print", Qt::CaseSensitive ) )
            {
              share.setShareName( line.section( " Print    ", 0, 0 ).trimmed() );
              share.setComment( line.section( " Print    ", 1, 1 ).trimmed() );
            }
            else
            {
              share.setShareName( line.section( " Print", 0, 0 ).trimmed() );
              share.setComment( "" );
            }
            
            share.setHostName( host->hostName() );
            share.setWorkgroupName( host->workgroupName() );
            share.setTypeString( "Printer" );
            share.setLogin( m_auth_info.login() );

            if ( host->ipChecked() && host->hasIP() )
            {
              share.setHostIP( host->ip() );
            }
            else
            {
              // Do nothing
            }

            m_shares.append( share );
            share = Smb4KShare();
            continue;
          }
          else
          {
            continue;
          }
        }

        emit shares( host, m_shares );
      }
      break;
    }
  }

  exit( exitCode );
}



LookupInfoThread::LookupInfoThread( Smb4KHost *host, QObject *parent )
: BasicScanThread ( BasicScanThread::LookupInfoThread, host, parent )
{
}


LookupInfoThread::~LookupInfoThread()
{
}


void LookupInfoThread::lookup( const QString &command )
{
  Q_ASSERT( !command.isEmpty() );
  
  m_proc = new Smb4KProcess( Smb4KProcess::LookupInfo, this );

  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->start();
}


void LookupInfoThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  switch( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // First evaluate stdout and if we cannot find the appropriate 
      // information also evaluate stderr.
      QString info_line;
      QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );
      
      for ( int i = 0; i < stdout.size(); ++i )
      {
        if ( stdout.at( i ).contains( "OS=" ) || stdout.at( i ).contains( "Server=" ) )
        {
          info_line = stdout.at( i );
          
          Smb4KHost *host = static_cast<Smb4KHost *>( m_item );
          host->setInfo( info_line.section( "Server=[", 1, 1 ).section( "]", 0, 0 ).trimmed(),
                         info_line.section( "OS=[", 1, 1 ).section( "]", 0, 0 ).trimmed() );
          emit info( host );
          break;
        }
        else
        {
          // Do nothing
        }
      }
      
      if ( info_line.isEmpty() )
      {
        QStringList stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).split( "\n", QString::SkipEmptyParts );
        
        for ( int i = 0; i < stderr.size(); ++i )
        {
          if ( stderr.at( i ).contains( "OS=" ) || stderr.at( i ).contains( "Server=" ) )
          {
            info_line = stderr.at( i );
            
            Smb4KHost *host = static_cast<Smb4KHost *>( m_item );
            host->setInfo( info_line.section( "Server=[", 1, 1 ).section( "]", 0, 0 ).trimmed(),
                           info_line.section( "OS=[", 1, 1 ).section( "]", 0, 0 ).trimmed() );
            emit info( host );
            break;
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
      break;
    }
  }

  exit( exitCode );
}


Smb4KScannerPrivate::Smb4KScannerPrivate()
{
}


Smb4KScannerPrivate::~Smb4KScannerPrivate()
{
}

#include "smb4kscanner_p.moc"
