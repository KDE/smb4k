/***************************************************************************
    smb4ksearch_p  -  Private helper classes for Smb4KSearch class.
                             -------------------
    begin                : Mo Dez 22 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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
#include <QTimer>
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <kstandarddirs.h>

// application specific inludes
#include <smb4ksearch_p.h>
#include <smb4knotification.h>
#include <smb4kglobal.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4ksambaoptionsinfo.h>
#include <smb4ksettings.h>
#include <smb4kauthinfo.h>

using namespace Smb4KGlobal;


Smb4KSearchJob::Smb4KSearchJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_proc( NULL )
{
  setCapabilities( KJob::Killable );
}


Smb4KSearchJob::~Smb4KSearchJob()
{
}


void Smb4KSearchJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartSearch() ) );
}


void Smb4KSearchJob::setupSearch( const QString &string, Smb4KHost *master, QWidget *parentWidget )
{
  Q_ASSERT( !string.trimmed().isEmpty() );
  m_string = string;
  m_master = *master;
  m_parent_widget = parentWidget;
}


bool Smb4KSearchJob::doKill()
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


void Smb4KSearchJob::slotStartSearch()
{
  // Check the string if it is a IP address and compile the command accordingly.
  QHostAddress address( m_string.trimmed() );
  QStringList arguments;

  emit aboutToStart( m_string );

  if ( address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // Find smbtree program.
    QString smbtree = KStandardDirs::findExe( "smbtree" );

    if ( smbtree.isEmpty() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "smbtree" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Lookup the custom options that are defined for the master browser.
    Smb4KWorkgroup *workgroup = findWorkgroup( Smb4KSettings::domainName() );
    Smb4KHost *master_browser = NULL;
    Smb4KSambaOptionsInfo *info = NULL;
    Smb4KAuthInfo authInfo;

    if ( workgroup )
    {
      master_browser = findHost( workgroup->masterBrowserName(), workgroup->workgroupName() );
    }
    else
    {
      // Do nothing
    }

    if ( master_browser )
    {
      info = Smb4KSambaOptionsHandler::self()->findItem( master_browser );
    }
    else
    {
      // Do nothing
    }

    // Compile the command
    arguments << smbtree;
    arguments << "-d2";

    // Kerberos
    if ( info )
    {
      switch ( info->useKerberos() )
      {
        case Smb4KSambaOptionsInfo::UseKerberos:
        {
          arguments << "-k";
          break;
        }
        case Smb4KSambaOptionsInfo::NoKerberos:
        {
          // No kerberos
          break;
        }
        case Smb4KSambaOptionsInfo::UndefinedKerberos:
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

    // Send broadcasts
    if ( Smb4KSettings::smbtreeSendBroadcasts() )
    {
      arguments << "-b";
    }
    else
    {
      // Do nothing
    }

    if ( !authInfo.login().isEmpty() )
    {
      arguments << QString( "-U %1" ).arg( m_master.login() );
    }
    else
    {
      arguments << "-U %";
    }
  }
  else
  {
    // Find nmblookup program.
    QString nmblookup = KStandardDirs::findExe( "nmblookup" );

    if ( nmblookup.isEmpty() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "nmblookup" );
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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "grep" );
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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->commandNotFound( "sed" );
      return;
    }
    else
    {
      // Go ahead
    }

    QMap<QString,QString> samba_options = Smb4KSambaOptionsHandler::self()->globalSambaOptions();

    // Compile the command
    arguments << nmblookup;

    // Domain
    if ( !Smb4KSettings::domainName().isEmpty() &&
         QString::compare( Smb4KSettings::domainName(), samba_options["workgroup"] ) != 0 )
    {
      arguments << QString( "-W '%1'" ).arg( Smb4KSettings::domainName() );
    }
    else
    {
      // Do nothing
    }

    // NetBIOS name
    if ( !Smb4KSettings::netBIOSName().isEmpty() &&
         QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
    {
      arguments << QString( "-n '%1'" ).arg( Smb4KSettings::netBIOSName() );
    }
    else
    {
      // Do nothing
    }

    // NetBIOS scope
    if ( !Smb4KSettings::netBIOSScope().isEmpty() &&
         QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0 )
    {
      arguments << QString( "-i '%1'" ).arg( Smb4KSettings::netBIOSScope() );
    }
    else
    {
      // Do nothing
    }

    // Socket options
    if ( !Smb4KSettings::socketOptions().isEmpty() &&
         QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0 )
    {
      arguments << QString( "-O '%1'" ).arg( Smb4KSettings::socketOptions() );
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

    // WINS server
    if ( !Smb4KSambaOptionsHandler::self()->winsServer().isEmpty() )
    {
      arguments << "-R";
      arguments << QString( "-U '%1'" ).arg( Smb4KSambaOptionsHandler::self()->winsServer() );
      arguments << QString( "'%1'" ).arg( m_string );
      arguments << "-A";
      arguments << "|";
      arguments << grep;
      arguments << "'<00>'";
      arguments << "|";
      arguments << sed;
      arguments << "-e 's/<00>.*//'";
    }
    else
    {
      arguments << QString( "'%1'" ).arg( m_string );
      arguments << "-A";
      arguments << "|";
      arguments << grep;
      arguments << "'<00>'";
      arguments << "|";
      arguments << sed;
      arguments << "-e 's/<00>.*//'";
    }
  }

  m_proc = new Smb4KProcess( Smb4KProcess::Search, this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setEnv( "PASSWD", !m_master.password().isEmpty() ? m_master.password() : "", true );
  m_proc->setProgram( arguments );
  
  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotReadStandardOutput() ) );
  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotReadStandardError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );
  
  m_proc->start();
}


void Smb4KSearchJob::slotReadStandardOutput()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts, Qt::CaseSensitive );
  QHostAddress address( m_string.trimmed() );

  if ( address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // Process output from smbtree.
    QString workgroup_name;

    foreach ( const QString &line, stdout )
    {
      if ( !line.contains( "added interface", Qt::CaseInsensitive ) &&
           !line.contains( "tdb(", Qt::CaseInsensitive ) &&
           !line.contains( "Got a positive", Qt::CaseInsensitive ) &&
           !line.contains( "error connecting", Qt::CaseInsensitive ) &&
           !line.isEmpty() )
      {
        if ( !line.contains( "\\" ) && !line.trimmed().isEmpty() )
        {
          workgroup_name = line.trimmed();
          continue;
        }
        else if ( line.count( "\\" ) == 2 )
        {
          // Get the host name and check if it matches the search string.
          QString host_name = line.trimmed().section( "\t", 0, 0 ).trimmed().remove( 0, 2 );
          QString comment = line.trimmed().section( "\t", 1, -1 ).trimmed();

          if ( host_name.contains( m_string, Qt::CaseInsensitive ) )
          {
            Smb4KHost host;
            host.setWorkgroupName( workgroup_name );
            host.setHostName( host_name );
            host.setComment( comment );

            emit result( &host );
          }
          else
          {
            // Do nothing
          }
          continue;
        }
        else if ( line.count( "\\" ) == 3 )
        {
          QString unc = line.trimmed().section( "\t", 0, 0 ).trimmed().replace( "\\", "/" );
          QString comment = line.trimmed().section( "\t", 1, -1 ).trimmed();

          if ( unc.contains( m_string, Qt::CaseInsensitive ) )
          {
            Smb4KShare share;
            share.setUNC( unc );
            share.setComment( comment );
            share.setWorkgroupName( workgroup_name );

            emit result( &share );
          }
          else
          {
            // Do nothing
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
    // Process output from nmblookup.
    QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

    if ( !stdout.isEmpty() )
    {
      Smb4KHost host( stdout.first().trimmed() );
      host.setWorkgroupName( stdout.last().trimmed() );
      host.setIP( m_string.trimmed() );

      emit result( &host );
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KSearchJob::slotReadStandardError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 );

  QHostAddress address( m_string.trimmed() );

  if ( address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // Process authentication errors:
    if ( stderr.contains( "The username or password was not correct." ) ||
         stderr.contains( "NT_STATUS_ACCOUNT_DISABLED" ) /* AD error */ ||
         stderr.contains( "NT_STATUS_ACCESS_DENIED" ) ||
         stderr.contains( "NT_STATUS_LOGON_FAILURE" ) )
    {
      m_proc->abort();
      emit authError( this );
    }
    else
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->searchingFailed( m_string, stderr );
    }
  }
  else
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->searchingFailed( m_string, stderr );
  }
}


void Smb4KSearchJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus status )
{
  switch ( status )
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
      break;
    }
  }
  
  emitResult();
  emit finished( m_string );
}



Smb4KSearchPrivate::Smb4KSearchPrivate()
{
}


Smb4KSearchPrivate::~Smb4KSearchPrivate()
{
}

#include "smb4ksearch_p.moc"

