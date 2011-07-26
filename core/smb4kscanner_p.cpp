/***************************************************************************
    smb4kscanner_p  -  Private helper classes for the scanner
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
#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <kstandarddirs.h>
#include <kshell.h>

// application specific includes
#include <smb4kscanner_p.h>
#include <smb4ksettings.h>
#include <smb4knotification.h>
#include <smb4kglobal.h>
#include <smb4kcustomoptionsmanager.h>
#include <smb4kcustomoptions.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kshare.h>
#include <smb4kwalletmanager.h>

using namespace Smb4KGlobal;


Smb4KLookupDomainsJob::Smb4KLookupDomainsJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_proc( NULL )
{
}


Smb4KLookupDomainsJob::~Smb4KLookupDomainsJob()
{
}


void Smb4KLookupDomainsJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartLookup() ) );
}


void Smb4KLookupDomainsJob::setupLookup( QWidget *parent )
{
  m_parent_widget = parent;
}


bool Smb4KLookupDomainsJob::doKill()
{
  if ( m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting) )
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }

  return KJob::doKill();
}


void Smb4KLookupDomainsJob::processWorkgroups()
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
          m_workgroups_list << workgroup;
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

  emit workgroups( m_workgroups_list );
}


void Smb4KLookupDomainsJob::slotStartLookup()
{
  // Find nmblookup program.
  QString nmblookup = KStandardDirs::findExe( "nmblookup" );

  if ( nmblookup.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "nmblookup" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Find grep program.
  QString grep = KStandardDirs::findExe( "grep" );

  if ( grep.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "grep" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Find awk program
  QString awk = KStandardDirs::findExe( "awk" );

  if ( awk.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "awk" );
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }

  // Find xargs program
  QString xargs = KStandardDirs::findExe( "xargs" );

  if ( xargs.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "xargs" );
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }

  // Global Samba options
  QMap<QString,QString> samba_options = globalSambaOptions();

  // Compile command
  QStringList arguments;

  // nmblookup
  arguments << nmblookup;

  // Domain
  if ( !Smb4KSettings::domainName().isEmpty() &&
       QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
  {
    arguments << QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
  }
  else
  {
    // Do nothing
  }

  // NetBIOS name
  if ( !Smb4KSettings::netBIOSName().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
  {
    arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
  }
  else
  {
    // Do nothing
  }

  // NetBIOS scope
  if ( !Smb4KSettings::netBIOSScope().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0 )
  {
    arguments << QString( "-i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) );
  }
  else
  {
    // Do nothing
  }

  // Socket options
  if ( !Smb4KSettings::socketOptions().isEmpty() &&
       QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0 )
  {
    arguments << QString( "-O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) );
  }
  else
  {
    // Do nothing
  }

  // Port 137
  if ( Smb4KSettings::usePort137() )
  {
    arguments << "-r";
  }
  else
  {
    // Do nothing
  }

  // Broadcast address
  QHostAddress address( Smb4KSettings::broadcastAddress() );

  if ( !Smb4KSettings::broadcastAddress().isEmpty() &&
       address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    arguments << QString( "-B %1" ).arg( Smb4KSettings::broadcastAddress() );
  }
  else
  {
    // Do nothing
  }

  arguments << "-M";
  arguments << "--";
  arguments << "-";
  arguments << "|";
  arguments << grep;
  arguments << "'<01>'";
  arguments << "|";
  arguments << awk;
  arguments << "'{print $1}'";
  arguments << "|";

  if ( !winsServer().isEmpty() )
  {
    arguments << xargs;
    arguments << "-Iips";
    arguments << nmblookup;
    arguments << "-R";
    arguments << QString( "-U %1" ).arg( KShell::quoteArg( winsServer() ) );
    arguments << "-A ips";
  }
  else
  {
    arguments << xargs;
    arguments << "-Iips";
    arguments << nmblookup;
    arguments << "-A ips";
  }

  // Domain
  if ( !Smb4KSettings::domainName().isEmpty() &&
       QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
  {
    arguments << QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
  }
  else
  {
    // Do nothing
  }

  // NetBIOS name
  if ( !Smb4KSettings::netBIOSName().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
  {
    arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
  }
  else
  {
    // Do nothing
  }

  // NetBIOS scope
  if ( !Smb4KSettings::netBIOSScope().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0 )
  {
    arguments << QString( "-i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) );
  }
  else
  {
    // Do nothing
  }

  // Socket options
  if ( !Smb4KSettings::socketOptions().isEmpty() &&
       QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0 )
  {
    arguments << QString( "-O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) );
  }
  else
  {
    // Do nothing
  }

  // Port 137
  if ( Smb4KSettings::usePort137() )
  {
    arguments << "-r";
  }
  else
  {
    // Do nothing
  }

  m_proc = new Smb4KProcess( this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setShellCommand( arguments.join( " " ) );

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotReadStandardError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  emit aboutToStart();

  m_proc->start();  
}


void Smb4KLookupDomainsJob::slotReadStandardError()
{
  // Read from stderr and decide what to do.
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->retrievingDomainsFailed( stderr );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KLookupDomainsJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      processWorkgroups();
      break;
    }
  }

  emitResult();
  emit finished();
}



Smb4KQueryMasterJob::Smb4KQueryMasterJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_proc( NULL )
{
}


Smb4KQueryMasterJob::~Smb4KQueryMasterJob()
{
}


void Smb4KQueryMasterJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartLookup() ) );
}


void Smb4KQueryMasterJob::setupLookup( const QString &master, QWidget *parent )
{
  m_master_browser = master;
  m_parent_widget = parent;
}


bool Smb4KQueryMasterJob::doKill()
{
  if ( m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting) )
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KQueryMasterJob::processWorkgroups()
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

        m_workgroups_list << workgroup;

        workgroup = Smb4KWorkgroup();
        continue;
      }
    }
  }
  else
  {
    // Do nothing
  }

  emit workgroups( m_workgroups_list );
}


void Smb4KQueryMasterJob::slotStartLookup()
{
  // Find net program
  QString net = KStandardDirs::findExe( "net" );

  if ( net.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "net" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Find xargs program
  QString xargs = KStandardDirs::findExe( "xargs" );

  if ( xargs.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "xargs" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Global Samba options
  QMap<QString,QString> samba_options = globalSambaOptions();

  // Workgroup
  Smb4KWorkgroup workgroup;

  if ( !Smb4KSettings::domainName().isEmpty() )
  {
    workgroup.setWorkgroupName( Smb4KSettings::domainName() );
  }
  else
  {
    workgroup.setWorkgroupName( samba_options["workgroup"] );
  }

  Smb4KCustomOptions *options = NULL;
  QStringList arguments;

  if ( !m_master_browser.isEmpty() )
  {
    Smb4KHost host;
    // We do not need to set the domain here, because neither
    // Smb4KCustomOptionsMangager nor Smb4KWalletManager need 
    // the domain entry to return correct data.    
    if ( QHostAddress( m_master_browser ).protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
    {
      host.setHostName( m_master_browser );
    }
    else
    {
      host.setIP( m_master_browser );
    }

    // Acquire the custom options for the master browser
    options = Smb4KCustomOptionsManager::self()->findOptions( &host );

    // Get authentication information for the host if needed
    if ( Smb4KSettings::masterBrowsersRequireAuth() )
    {
      m_auth_info.setHost( &host );
      Smb4KWalletManager::self()->readAuthInfo( &m_auth_info );
      
    }
    else
    {
      // Do nothing
    }
    
    // Custom master lookup
    arguments << net;
    arguments << "lookup";
    arguments << "host";
    arguments << KShell::quoteArg( host.hostName() );
  }
  else
  {
    // Get authentication information for the host if needed
    if ( Smb4KSettings::masterBrowsersRequireAuth() && Smb4KSettings::useDefaultLogin() )
    {
      m_auth_info.useDefaultAuthInfo();
      Smb4KWalletManager::self()->readAuthInfo( &m_auth_info );
    }
    else
    {
      // Do nothing
    }
    
    // Master lookup
    arguments << net;
    arguments << "lookup";
    arguments << "master";
    arguments << KShell::quoteArg( workgroup.workgroupName() );
  }

  // The user's workgroup/domain name
  if ( !Smb4KSettings::domainName().isEmpty() &&
       QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
  {
    arguments << QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
  }
  else
  {
    // Do nothing
  }

  // The user's NetBIOS name
  if ( !Smb4KSettings::netBIOSName().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
  {
    arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
  }
  else
  {
    // Do nothing
  }

  // Machine account
  if ( Smb4KSettings::machineAccount() )
  {
    arguments << "-P";
  }
  else
  {
    // Do nothing
  }

  // Port
  if ( options && options->smbPort() != Smb4KSettings::remoteSMBPort() )
  {
    arguments << QString( "-p %1" ).arg( options->smbPort() );
  }
  else
  {
    arguments << QString( "-p %1" ).arg( Smb4KSettings::remoteSMBPort() );
  }

  // User name and password if needed
  if ( Smb4KSettings::masterBrowsersRequireAuth() )
  {
    arguments << QString( "-U %1%" ).arg( m_auth_info.login() );
  }
  else
  {
    arguments << "-U %";
  }
  
  arguments << "|";
  
  // Domain lookup (via xargs)
  arguments << xargs;
  arguments << "-Iip";
  arguments << net;

  // Protocol & command. Since the domain lookup only works with the RAP
  // protocol, there is no point in using the 'Automatic' feature.
  arguments << "rap";
  arguments << "domain";

  // The user's workgroup/domain name
  if ( !Smb4KSettings::domainName().isEmpty() &&
       QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
  {
    arguments << QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
  }
  else
  {
    // Do nothing
  }

  // The user's NetBIOS name
  if ( !Smb4KSettings::netBIOSName().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
  {
    arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
  }
  else
  {
    // Do nothing
  }

  // Machine account
  if ( Smb4KSettings::machineAccount() )
  {
    arguments << "-P";
  }
  else
  {
    // Do nothing
  }

  // Port
  if ( options && options->smbPort() != Smb4KSettings::remoteSMBPort() )
  {
    arguments << QString( "-p %1" ).arg( options->smbPort() );
  }
  else
  {
    arguments << QString( "-p %1" ).arg( Smb4KSettings::remoteSMBPort() );
  }

  // User name and password if needed
  if ( Smb4KSettings::masterBrowsersRequireAuth() )
  {
    arguments << QString( "-U %1%" ).arg( m_auth_info.login() );
  }
  else
  {
    arguments << "-U %";
  }

  // IP address (discovered by by previous net command)
  arguments << "-I ip";

  // Server name if available
  if ( !m_master_browser.isEmpty() )
  {
    arguments << QString( "-S %1" ).arg( KShell::quoteArg( m_master_browser ) );
  }
  else
  {
    // Do nothing
  }

  // Add debug level to get the IP of the master browser that we 
  // are connecting to.
  arguments << "-d3";

  m_proc = new Smb4KProcess( this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setShellCommand( arguments.join( " " ) );

  if ( Smb4KSettings::masterBrowsersRequireAuth() )
  {
    m_proc->setEnv( "PASSWD", m_auth_info.password() );
  }
  else
  {
    // Do nothing
  }

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotReadStandardError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  emit aboutToStart();

  m_proc->start();  
}


void Smb4KQueryMasterJob::slotReadStandardError()
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
      if ( m_master_browser.isEmpty() )
      {
        // Figure out the current master browser's IP address.
        QStringList stderr_list = stderr.split( "\n", QString::SkipEmptyParts );

        foreach ( const QString &line, stderr_list )
        {
          if ( line.contains( "Connecting to host=" ) )
          {
            m_master_browser = line.section( "=", 1, 1 ).trimmed();
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

      emit authError( this );
    }
    else
    {
      // Avoid reporting the debug output as error.
      if ( stderr.contains( "NT_STATUS" ) )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->retrievingDomainsFailed( stderr );
      }
      else
      {
        // Debug output. Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KQueryMasterJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      processWorkgroups();
      break;
    }
  }

  emitResult();
  emit finished();
}



Smb4KScanBAreasJob::Smb4KScanBAreasJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_proc( NULL )
{
}


Smb4KScanBAreasJob::~Smb4KScanBAreasJob()
{
}


void Smb4KScanBAreasJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartScan() ) );
}


void Smb4KScanBAreasJob::setupScan( QWidget *parent )
{
  m_parent_widget = parent;
}


bool Smb4KScanBAreasJob::doKill()
{
  if ( m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting) )
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KScanBAreasJob::processScan()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).trimmed().split( "\n", QString::SkipEmptyParts );

  if ( !stdout.isEmpty() )
  {
    Smb4KWorkgroup workgroup;
    Smb4KHost host;
    bool skip = false;

    foreach ( const QString &line, stdout )
    {
      // Check if we have to skip this host entry.
      // A host entry is skipped if the IP address is invalid, i.e.
      // 0.0.0.0 is returned.
      if ( line.startsWith( "Looking up status of" ) )
      {
        QString ip_address = line.section( "of", 1, 1 ).trimmed();
        skip = (QString::compare( ip_address, "0.0.0.0" ) == 0);
      }
      else
      {
        // Do nothing
      }

      // Now process the output if everything is OK. Otherwise
      // skip lines until there is a valid host entry.
      if ( !skip )
      {
        if ( line.startsWith( "Looking up status of" ) )
        {
          // Set the IP address of the host.
          QString ip_address = line.section( "of", 1, 1 ).trimmed();

          if ( QString::compare( ip_address, "0.0.0.0" ) != 0 )
          {
            host.setIP( ip_address );
          }
          else
          {
            // Do nothing
          }
          continue;
        }
        else if ( line.contains( "MAC Address", Qt::CaseSensitive ) )
        {
          // Check that the workgroup object carries a workgroup
          // name and a master browser name.
          if ( !workgroup.workgroupName().isEmpty() && !workgroup.masterBrowserName().isEmpty() )
          {
            // Check whether the workgroup has already been entered
            // into the list.
            bool workgroup_found = false;

            for ( int i = 0; i < m_workgroups_list.size(); ++i )
            {
              if ( QString::compare( m_workgroups_list.at( i ).workgroupName(), workgroup.workgroupName(), Qt::CaseInsensitive ) == 0 )
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
              m_workgroups_list << workgroup;
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

          m_hosts_list << host;

          workgroup = Smb4KWorkgroup();
          host = Smb4KHost();
          continue;
        }
        else if ( line.contains( " <00> ", Qt::CaseSensitive ) )
        {
          // This is the name of the workgroup and/or host. Depending
          // on if the <GROUP> label is present, it is either a host name
          // or a workgroup name that we process here.
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
          // The __MSBROWSE__ label marks a host that offers a browse list.
          // If also the <01> label is present, this is a master browser.
          //
          // If the host is not a master browser but offers a browse list,
          // we use it temporarily as pseudo master browser for the workgroup
          // until the right master browser has been set.
          if ( line.contains( " <01> ", Qt::CaseSensitive ) )
          {
            workgroup.setMasterBrowserName( host.hostName() );
            workgroup.setMasterBrowserIP( host.ip() );
            workgroup.setHasPseudoMasterBrowser( false );
            host.setIsMasterBrowser( true );
          }
          else
          {
            if ( workgroup.masterBrowserName().isEmpty() )
            {
              workgroup.setMasterBrowserName( host.hostName() );
              workgroup.setMasterBrowserIP( host.ip() );
              workgroup.setHasPseudoMasterBrowser( true );
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

  // Emit the list of workgroups and the list of hosts.
  emit workgroups( m_workgroups_list );
  emit hosts( m_hosts_list );
}


void Smb4KScanBAreasJob::slotStartScan()
{
  // Find nmblookup program.
  QString nmblookup = KStandardDirs::findExe( "nmblookup" );

  if ( nmblookup.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "nmblookup" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Find awk program
  QString awk = KStandardDirs::findExe( "awk" );

  if ( awk.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "awk" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Find sed program
  QString sed = KStandardDirs::findExe( "sed" );

  if ( sed.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "sed" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Find xargs program
  QString xargs = KStandardDirs::findExe( "xargs" );

  if ( xargs.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "xargs" );
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }

  // Global Samba options
  QMap<QString,QString> samba_options = globalSambaOptions();

  // Broadcast areas/addresses

  // FIXME: Emit error message if the list is empty!!!
  QStringList addresses = Smb4KSettings::broadcastAreas().split( ",", QString::SkipEmptyParts );

  // Assemble the command
  QStringList arguments;
  
  for ( int i = 0; i < addresses.size(); ++i )
  {
    if ( !arguments.isEmpty() )
    {
      arguments << ";";
    }
    else
    {
      // Do nothing
    }
    
    arguments << nmblookup;

    // Domain
    if ( !Smb4KSettings::domainName().isEmpty() &&
         QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
    {
      arguments << QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
    }
    else
    {
      // Do nothing
    }

    // NetBIOS name
    if ( !Smb4KSettings::netBIOSName().isEmpty() &&
         QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
    {
      arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
    }
    else
    {
      // Do nothing
    }

    // NetBIOS scope
    if ( !Smb4KSettings::netBIOSScope().isEmpty() &&
         QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0 )
    {
      arguments << QString( "-i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) );
    }
    else
    {
      // Do nothing
    }

    // Socket options
    if ( !Smb4KSettings::socketOptions().isEmpty() &&
         QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0 )
    {
      arguments << QString( "-O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) );
    }
    else
    {
      // Do nothing
    }

    // Port 137
    if ( Smb4KSettings::usePort137() )
    {
      arguments << "-r";
    }
    else
    {
      // Do nothing
    }

    // We do not want the globally defined broadcast address here, because the
    // broadcast address option is needed for the IP scan.

    // FIXME: Emit error message if address is not IP4 or IP6!!!
    arguments << QString( "-B %1" ).arg( addresses.at( i ) );
    arguments << "--";
    arguments << QString( "%1" ).arg( KShell::quoteArg( "*" ) );
    arguments << sed;
    arguments << "-e /querying/d";
    arguments << "|";
    arguments << awk;
    arguments << QString( "%1" ).arg( KShell::quoteArg( "{print $1}" ) );
    arguments << "|";
    arguments << xargs;
    arguments << "-Iip";
    arguments << nmblookup;

    // Domain
    if ( !Smb4KSettings::domainName().isEmpty() &&
         QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
    {
      QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
    }
    else
    {
      // Do nothing
    }

    // NetBIOS name
    if ( !Smb4KSettings::netBIOSName().isEmpty() &&
         QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
    {
      arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
    }
    else
    {
      // Do nothing
    }

    // NetBIOS scope
    if ( !Smb4KSettings::netBIOSScope().isEmpty() &&
         QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0 )
    {
      arguments << QString( "-i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) );
    }
    else
    {
      // Do nothing
    }

    // Socket options
    if ( !Smb4KSettings::socketOptions().isEmpty() &&
         QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0 )
    {
      arguments << QString( "-O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) );
    }
    else
    {
      // Do nothing
    }

    // Port 137
    if ( Smb4KSettings::usePort137() )
    {
      arguments << "-r";
    }
    else
    {
      // Do nothing
    }

    // Broadcast address
    // Note: This time we want to have the global one!
    if ( !Smb4KSettings::broadcastAddress().isEmpty() &&
         QHostAddress( Smb4KSettings::broadcastAddress() ).protocol() != QAbstractSocket::UnknownNetworkLayerProtocol )
    {
      arguments << QString( "-B %1" ).arg( Smb4KSettings::broadcastAddress() );
    }
    else
    {
      // Do nothing
    }

    // Include the WINS server:
    if ( !winsServer().isEmpty() )
    {
      arguments << "-R";
      arguments << QString( "-U %1" ).arg( winsServer() );
    }
    else
    {
      // Do nothing
    }
      
    arguments << "-A ip";
  }

  m_proc = new Smb4KProcess( this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setShellCommand( arguments.join( " " ) );

  connect( m_proc, SIGNAL( readyReadStandardError() ), SLOT( slotReadStandardError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  emit aboutToStart();

  m_proc->start(); 
}


void Smb4KScanBAreasJob::slotReadStandardError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->scanningBroadcastAreaFailed( stderr );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KScanBAreasJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      processScan();
      break;
    }
  }

  emitResult();
  emit finished();
}



Smb4KLookupDomainMembersJob::Smb4KLookupDomainMembersJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_proc( NULL )
{
}


Smb4KLookupDomainMembersJob::~Smb4KLookupDomainMembersJob()
{
}


void Smb4KLookupDomainMembersJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartLookup() ) );
}


void Smb4KLookupDomainMembersJob::setupLookup( Smb4KWorkgroup *workgroup, QWidget *parent )
{
  Q_ASSERT( workgroup );
  m_workgroup = *workgroup;
  m_parent_widget = parent;
}


bool Smb4KLookupDomainMembersJob::doKill()
{
  if ( m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting) )
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }

  return KJob::doKill();
}


void Smb4KLookupDomainMembersJob::processHosts()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

  if ( !stdout.isEmpty() )
  {
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
        // Omit host names that contain spaces since QUrl cannot handle them.
        // And, they are wrong, anyway.
        if ( !line.section( "   ", 0, 0 ).trimmed().contains( " " ) )
        {
          host.setHostName( line.section( "   ", 0, 0 ).trimmed() );
          host.setWorkgroupName( m_workgroup.workgroupName() );
          host.setComment( line.section( "   ", 1, -1 ).trimmed() );
          
          if ( QString::compare( host.hostName(), m_workgroup.masterBrowserName() ) == 0 )
          {
            host.setAuthInfo( &m_auth_info );
            host.setIsMasterBrowser( true );

            if ( m_workgroup.hasMasterBrowserIP() )
            {
              host.setIP( m_workgroup.masterBrowserIP() );
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
          
          m_hosts_list << host;
        }
        else
        {
          qDebug() << "This host name contains a space. I cannot handle this...";
        }
        
        host = Smb4KHost();
        continue;
      }
    }
  }
  else
  {
    // Do nothing
  }

  emit hosts( &m_workgroup, m_hosts_list );
}


void Smb4KLookupDomainMembersJob::slotStartLookup()
{
  // Find net program
  QString net = KStandardDirs::findExe( "net" );

  if ( net.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "net" );
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }

  // Get the master browser of the defined workgroup, so that we
  // can connect to it.
  Smb4KHost *master = findHost( m_workgroup.masterBrowserName(), m_workgroup.workgroupName() );

  if ( master )
  {
    // If the master browsers need authentication, we read it now.
    m_auth_info.setHost( master );

    if ( Smb4KSettings::masterBrowsersRequireAuth() )
    {
      Smb4KWalletManager::self()->readAuthInfo( &m_auth_info );
      master->setAuthInfo( &m_auth_info );
    }
    else
    {
      // Do nothing
    }

    // Global Samba options
    QMap<QString,QString> samba_options = globalSambaOptions();

    // Custom options
    Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions( master );

    // Assemble the command.
    QStringList arguments;

    // net command
    arguments << net;

    // Protocol & command. Since the domain member lookup only works with
    // the RAP protocol, there is no point in using the 'Automatic' feature.
    arguments << "rap";
    arguments << "server";
    arguments << "domain";

    // The user's domain or workgroup
    if ( !Smb4KSettings::domainName().isEmpty() &&
         QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
    {
      arguments << QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
    }
    else
    {
      // Do nothing
    }

    // The NetBIOS name of the user's machine
    if ( !Smb4KSettings::netBIOSName().isEmpty() &&
         QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
    {
      arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
    }
    else
    {
      // Do nothing
    }

    // Machine account
    if ( Smb4KSettings::machineAccount() )
    {
      arguments << "-P";
    }
    else
    {
      // Do nothing
    }

    // Remote SMB port
    if ( options && options->smbPort() != Smb4KSettings::remoteSMBPort() )
    {
      arguments << QString( "-p %1" ).arg( options->smbPort() );
    }
    else
    {
      arguments << QString( "-p %1" ).arg( Smb4KSettings::remoteSMBPort() );
    }

    // IP address of the master browser
    if ( m_workgroup.hasMasterBrowserIP() )
    {
      arguments << QString( "-I %1" ).arg( m_workgroup.masterBrowserIP() );
    }
    else
    {
      // Do nothing
    }

    // Workgroup of the remote master browser that is to be 
    // queried for the workgroup/domain members.
    arguments << QString( "-w %1" ).arg( KShell::quoteArg( m_workgroup.workgroupName() ) );

    // Name of the remote master browser
    arguments << QString( "-S %1" ).arg( KShell::quoteArg( m_workgroup.masterBrowserName() ) );

    // Authentication, if needed
    if ( Smb4KSettings::masterBrowsersRequireAuth() )
    {
      if ( !m_auth_info.login().isEmpty() )
      {
        arguments << QString( "-U %1" ).arg( m_auth_info.login() );
        // Password will be set below.
      }
      else
      {
        arguments << "-U %";
      }
    }
    else
    {
      arguments << "-U %";
    }

    m_proc = new Smb4KProcess( this );
    m_proc->setShellCommand( arguments.join( " " ) );
    m_proc->setOutputChannelMode( KProcess::SeparateChannels );

    if ( Smb4KSettings::self()->masterBrowsersRequireAuth() )
    {
      m_proc->setEnv( "PASSWD", m_auth_info.password(), true );
    }
    else
    {
      // Do nothing
    }
  
    connect( m_proc, SIGNAL( readyReadStandardError() ), SLOT( slotReadStandardError() ) );
    connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

    emit aboutToStart( &m_workgroup );
    
    m_proc->start();
  }
  else
  {
    // The master browser could not be determined. End the
    // job here and emit an empty hosts list.
    emit hosts( &m_workgroup, m_hosts_list );
    emitResult();
  }
}


void Smb4KLookupDomainMembersJob::slotReadStandardError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    if ( stderr.contains( "The username or password was not correct." ) ||
         stderr.contains( "NT_STATUS_ACCOUNT_DISABLED" ) /* AD error */ ||
         stderr.contains( "NT_STATUS_ACCESS_DENIED" ) ||
         stderr.contains( "NT_STATUS_LOGON_FAILURE" ) )
    {
      emit authError( this );
    }
    else if ( stderr.contains( "tdb_transaction_recover:" ) )
    {
      // Suppress debug output/information sent to stderr
      qDebug() << stderr;
    }
    else
    {
      // Notify the user that an error occurred.
      Smb4KNotification *notification = new Smb4KNotification();
      notification->retrievingServersFailed( &m_workgroup, stderr );
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KLookupDomainMembersJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      processHosts();
      break;
    }
  }

  emitResult();
  emit finished( &m_workgroup );
}



Smb4KLookupSharesJob::Smb4KLookupSharesJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_proc( NULL )
{
}


Smb4KLookupSharesJob::~Smb4KLookupSharesJob()
{
}


void Smb4KLookupSharesJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartLookup() ) );
}


void Smb4KLookupSharesJob::setupLookup( Smb4KHost *host, QWidget *parent )
{
  Q_ASSERT( host );
  m_host = *host;
  m_parent_widget = parent;
}


bool Smb4KLookupSharesJob::doKill()
{
  if ( m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting) )
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KLookupSharesJob::processShares()
{
  // Additional authentication error handling.
  if ( m_proc->exitCode() == 104 /* access denied in W2k3 domain */ ||
       m_proc->exitCode() == 235 /* wrong password in W2k3 domain */ )
  {
    emit authError( this );
    // We can just return here, because this function is invoked
    // in slotProcessFinished() and emitResult() will be emitted 
    // at its end.
    return;
  }
  else
  {
    // Do nothing
  }

  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );
  
  if ( !stdout.isEmpty() )
  {
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
            
        share.setHostName( m_host.hostName() );
        share.setWorkgroupName( m_host.workgroupName() );
        share.setTypeString( "Disk" );
        share.setAuthInfo( &m_auth_info );

        if ( m_host.hasIP() )
        {
          share.setHostIP( m_host.ip() );
        }
        else
        {
          // Do nothing
        }

        m_shares_list << share;
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
            
        share.setHostName( m_host.hostName() );
        share.setWorkgroupName( m_host.workgroupName() );
        share.setTypeString( "IPC" );
        share.setAuthInfo( &m_auth_info );

        if ( m_host.hasIP() )
        {
          share.setHostIP( m_host.ip() );
        }
        else
        {
          // Do nothing
        }

        m_shares_list << share;
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
            
        share.setHostName( m_host.hostName() );
        share.setWorkgroupName( m_host.workgroupName() );
        share.setTypeString( "Printer" );
        share.setAuthInfo( &m_auth_info );

        if ( m_host.hasIP() )
        {
          share.setHostIP( m_host.ip() );
        }
        else
        {
          // Do nothing
        }

        m_shares_list << share;
        share = Smb4KShare();
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
  
  emit shares( &m_host, m_shares_list );
}


void Smb4KLookupSharesJob::slotStartLookup()
{
  // Find net program
  QString net = KStandardDirs::findExe( "net" );

  if ( net.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "net" );
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }
  
  // Authentication information.
  m_auth_info.setHost( &m_host );
  Smb4KWalletManager::self()->readAuthInfo( &m_auth_info );
  m_host.setAuthInfo( &m_auth_info );
  
  // Global Samba and custom options
  QMap<QString,QString> samba_options = globalSambaOptions();
  Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions( &m_host );
  
  // Assemble the command.
  QStringList arguments;
  
  // net program
  arguments << net;
  
  // Protocol hint & command.
  if ( options && options->protocolHint() != Smb4KCustomOptions::UndefinedProtocolHint )
  {
    switch ( options->protocolHint() )
    {
      case Smb4KCustomOptions::RPC:
      {
        arguments << "rpc";
        arguments << "share";
        arguments << "list";
        break;
      }
      case Smb4KCustomOptions::RAP:
      {
        arguments << "rap";
        arguments << "share";
        break;
      }
      default:
      {
        // Auto-detection. This only work with 'net share list' and 
        // *NOT* with 'net share'.
        arguments << "share";
        arguments << "list";
        break;
      }
    }
  }
  else
  {
    switch ( Smb4KSettings::protocolHint() )
    {
      case Smb4KSettings::EnumProtocolHint::RPC:
      {
        arguments << "rpc";
        arguments << "share";
        arguments << "list";
        break;
      }
      case Smb4KSettings::EnumProtocolHint::RAP:
      {
        arguments << "rap";
        arguments << "share";
        break;
      }
      default:
      {
        // Auto-detection. This only work with 'net share list' and 
        // *NOT* with 'net share'.
        arguments << "share";
        arguments << "list";
        break;
      }
    }
  }
  
  // Long output. We need this, because we want to know the type and
  // the comment, too.
  arguments << "-l";
  
  // The user's domain or workgroup
  if ( !Smb4KSettings::domainName().isEmpty() &&
       QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
  {
    arguments << QString( "-W %1" ).arg( KShell::quoteArg( Smb4KSettings::domainName() ) );
  }
  else
  {
    // Do nothing
  }
               
  // The user's NetBIOS name
  if ( !Smb4KSettings::netBIOSName().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
  {
    arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
  }
  else
  {
    // Do nothing
  }
  
  // Machine account
  if ( Smb4KSettings::machineAccount() )
  {
    arguments << "-P";
  }
  else
  {
    // Do nothing
  }
  
  // Port
  // If a port was defined for the host via Smb4KHost::port(), it will 
  // overwrite the other options.
  if ( m_host.port() != -1 )
  {
    arguments << QString( "-p %1" ).arg( m_host.port() );
  }
  else
  {
    if ( options && options->smbPort() != Smb4KSettings::remoteSMBPort() )
    {
      arguments << QString( "-p %1" ).arg( options->smbPort() );
    }
    else
    {
      arguments << QString( "-p %1" ).arg( Smb4KSettings::remoteSMBPort() );
    }
  }
  
  // Remote domain/workgroup name
  arguments << QString( "-w %1" ).arg( KShell::quoteArg( m_host.workgroupName() ) );
  
  // Remote host name
  arguments << QString( "-S %1" ).arg( KShell::quoteArg( m_host.hostName() ) );

  // IP address
  if ( m_host.hasIP() )
  {
    arguments << QString( "-I %1" ).arg( m_host.ip() );
  }
  else
  {
    // Do nothing
  }
  
  // Authentication data
  if ( !m_auth_info.login().isEmpty() )
  {
    arguments << QString( "-U %1" ).arg( m_auth_info.login() );
  }
  else
  {
    // Under some circumstances you need under Windows the 'guest'
    // account to be able to retrieve the list of shared resources.
    arguments << "-U guest%";
  }
 
 
  m_proc = new Smb4KProcess( this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setShellCommand( arguments.join( " " ) );
  m_proc->setEnv( "PASSWD", m_auth_info.password(), true );

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotReadStandardError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  emit aboutToStart( &m_host );

  m_proc->start();
}


void Smb4KLookupSharesJob::slotReadStandardError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    if ( stderr.contains( "The username or password was not correct." ) ||
         stderr.contains( "NT_STATUS_ACCOUNT_DISABLED" ) /* AD error */ ||
         stderr.contains( "NT_STATUS_ACCESS_DENIED" ) ||
         stderr.contains( "NT_STATUS_LOGON_FAILURE" ) )
    {
      emit authError( this );
    }
    else if ( stderr.contains( "could not obtain sid for domain", Qt::CaseSensitive ) )
    {
      // FIXME
      qDebug() << "FIXME: Wrong protocol used for host " << m_host.hostName();
    }
    else
    {
      if ( !stderr.contains( "creating lame", Qt::CaseSensitive ) )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->retrievingSharesFailed( &m_host, stderr );
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


void Smb4KLookupSharesJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      processShares();
      break;
    }
  }

  emitResult();
  emit finished( &m_host );
}



Smb4KLookupInfoJob::Smb4KLookupInfoJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_proc( NULL )
{
}


Smb4KLookupInfoJob::~Smb4KLookupInfoJob()
{
}


void Smb4KLookupInfoJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartLookup() ) );
}


void Smb4KLookupInfoJob::setupLookup( Smb4KHost *host, QWidget* parent )
{
  Q_ASSERT( host );
  m_host = *host;
  m_parent_widget = parent;
}


bool Smb4KLookupInfoJob::doKill()
{
  if ( m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting) )
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KLookupInfoJob::processInfo()
{
  // First evaluate stdout and if we cannot find the appropriate 
  // information also evaluate stderr.
  QString stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 );
  
  if ( stdout.contains( "OS=" ) || stdout.contains( "Server=" ) )
  {
    QStringList stdout_list = stdout.split( "\n", QString::SkipEmptyParts );
    
    foreach ( const QString &line, stdout_list )
    {
      if ( line.contains( "OS=" ) || line.contains( "Server=" ) )
      {
        QString server = line.section( "Server=[", 1, 1 ).section( "]", 0, 0 ).trimmed();
        QString os = line.section( "OS=[", 1, 1 ).section( "]", 0, 0 ).trimmed();
        m_host.setInfo( server, os );
        emit info( &m_host );
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
    QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 );
    
    if ( stderr.contains( "OS=" ) || stderr.contains( "Server=" ) )
    {
      QStringList stderr_list = stderr.split( "\n", QString::SkipEmptyParts );
      
      foreach ( const QString &line, stderr_list )
      {
        if ( line.contains( "OS=" ) || line.contains( "Server=" ) )
        {
          QString server = line.section( "Server=[", 1, 1 ).section( "]", 0, 0 ).trimmed();
          QString os = line.section( "OS=[", 1, 1 ).section( "]", 0, 0 ).trimmed();
          m_host.setInfo( server, os );
          emit info( &m_host );
          break;
        }
        else
        {
          continue;
        }
      }
    }
  }
}


void Smb4KLookupInfoJob::slotStartLookup()
{
  // Find the smbclient program
  QString smbclient = KStandardDirs::findExe( "smbclient" );

  if ( smbclient.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "smbclient" );
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }

  // Compile the command.
  QStringList arguments;
  arguments << smbclient;
  arguments << "-d1";
  arguments << "-N";
  arguments << QString( "-W %1" ).arg( KShell::quoteArg( m_host.workgroupName() ) );
  arguments << QString( "-L %1" ).arg( KShell::quoteArg( m_host.hostName() ) );

  if ( m_host.hasIP() )
  {
    arguments << QString( "-I %1" ).arg( m_host.ip() );
  }
  else
  {
    // Do nothing
  }

  // Machine account
  if ( Smb4KSettings::machineAccount() )
  {
    arguments << "-P";
  }
  else
  {
    // Do nothing
  }
  
  // Signing state
  switch ( Smb4KSettings::signingState() )
  {
    case Smb4KSettings::EnumSigningState::None:
    {
      break;
    }
    case Smb4KSettings::EnumSigningState::On:
    {
      arguments << "-S on";
      break;
    }
    case Smb4KSettings::EnumSigningState::Off:
    {
      arguments << "-S off";
      break;
    }
    case Smb4KSettings::EnumSigningState::Required:
    {
      arguments << "-S required";
      break;
    }
    default:
    {
      break;
    }
  }
  
  // Buffer size
  if ( Smb4KSettings::bufferSize() != 65520 )
  {
    arguments << QString( "-b %1" ).arg( Smb4KSettings::bufferSize() );
  }
  else
  {
    // Do nothing
  }
  
  // Get global Samba and custom options
  QMap<QString,QString> samba_options = globalSambaOptions();
  Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions( &m_host );
  
  // Port
  // If a port was defined for the host via Smb4KHost::port(), it will 
  // overwrite the other options.
  if ( m_host.port() != -1 )
  {
    arguments << QString( "-p %1" ).arg( m_host.port() );
  }
  else
  {
    if ( options && options->smbPort() != Smb4KSettings::remoteSMBPort() )
    {
      arguments << QString( "-p %1" ).arg( options->smbPort() );
    }
    else
    {
      arguments << QString( "-p %1" ).arg( Smb4KSettings::remoteSMBPort() );
    }
  }
  
  // Kerberos
  if ( options )
  {
    switch ( options->useKerberos() )
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        arguments << "-k";
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        // No kerberos 
        break;
      }
      case Smb4KCustomOptions::UndefinedKerberos:
      {
        if ( Smb4KSettings::useKerberos() )
        {
          arguments << "-k";
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
  }
  else
  {
    if ( Smb4KSettings::useKerberos() )
    {
      arguments << "-k";
    }
    else
    {
      // Do nothing
    }
  }
  
  // Resolve order
  if ( !Smb4KSettings::nameResolveOrder().isEmpty() &&
       QString::compare( Smb4KSettings::nameResolveOrder(), samba_options["name resolve order"] ) != 0 ) 
  {
    arguments << QString( "-R %1" ).arg( KShell::quoteArg( Smb4KSettings::nameResolveOrder() ) );
  }
  else
  {
    // Do nothing
  }
             
  // NetBIOS name
  if ( !Smb4KSettings::netBIOSName().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
  {
    arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
  }
  else
  {
    // Do nothing
  }
             
  // NetBIOS scope
  if ( !Smb4KSettings::netBIOSScope().isEmpty() &&
        QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0 )
  {
    arguments << QString( "-i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) );
  }
  else
  {
    // Do nothing
  }
  
  // Socket options
  if ( !Smb4KSettings::socketOptions().isEmpty() &&
       QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0 )
  {
    arguments << QString( "-O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) );
  }
  else
  {
    // Do nothing
  }
  
  m_proc = new Smb4KProcess( this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setShellCommand( arguments.join( " " ) );

  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  emit aboutToStart( &m_host );

  m_proc->start();  
}


void Smb4KLookupInfoJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      processInfo();
      break;
    }
  }

  emitResult();
  emit finished( &m_host );
}




Smb4KScannerPrivate::Smb4KScannerPrivate()
{
}


Smb4KScannerPrivate::~Smb4KScannerPrivate()
{
}


#include "smb4kscanner_p.moc"
