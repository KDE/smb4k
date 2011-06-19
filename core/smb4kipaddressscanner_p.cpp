/***************************************************************************
    smb4kipaddressscanner_p  -  Private classes for the IP address scanner
    of Smb4K.
                             -------------------
    begin                : Mi Jan 28 2009
    copyright            : (C) 2009-2011 by Alexander Reinholdt
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
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kshell.h>

// application specific includes
#include <smb4kipaddressscanner_p.h>
#include <smb4khost.h>
#include <smb4knotification.h>
#include <smb4ksettings.h>
#include <smb4kglobal.h>

using namespace Smb4KGlobal;


Smb4KIPLookupJob::Smb4KIPLookupJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_host( NULL ), m_parent_widget( NULL )
{
  setCapabilities( KJob::Killable );
}


Smb4KIPLookupJob::~Smb4KIPLookupJob()
{
}


void Smb4KIPLookupJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartLookup() ) );
}


void Smb4KIPLookupJob::setupLookup( Smb4KHost *host, QWidget *parentWidget )
{
  Q_ASSERT( host );
  m_host = host;
  m_parent_widget = parentWidget;
}


bool Smb4KIPLookupJob::doKill()
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


void Smb4KIPLookupJob::slotStartLookup()
{
  emit aboutToStart( m_host );

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
    // Go ahead
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
    // Go ahead
  }

  // Find the awk program
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

  // Global Samba options
  QMap<QString,QString> samba_options = globalSambaOptions();

  // Compile the arguments
  QStringList arguments;
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

  // WINS server
  if ( !winsServer().isEmpty() )
  {
    arguments << "-R";
    arguments << QString( "-U %1" ).arg( KShell::quoteArg( winsServer() ) );
  }
  else
  {
    // Do nothing
  }

  arguments << "--";
  arguments << KShell::quoteArg( m_host->hostName() );
  arguments << "|";
  arguments << grep;
  arguments << "'<00>'";
  arguments << "|";
  arguments << awk;
  arguments << "{'print $1'}";

  m_proc = new Smb4KProcess( Smb4KProcess::LookupIP, this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setShellCommand( arguments.join( " " ) );

  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotReadStandardOutput() ) );
  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotReadStandardError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->start();
}


void Smb4KIPLookupJob::slotReadStandardOutput()
{
  // Normally, there should only be one IP address. However, there might
  // be more than one. So, split the incoming data and use the first entry
  // as IP address (it's most likely the correct one). If there is no data,
  // set the IP address to an empty string.
  QStringList ip_address = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

  if ( !ip_address.isEmpty() )
  {
    m_host->setIP( ip_address.first().trimmed() );
  }
  else
  {
    m_host->setIP( QString() );
  }

  emit ipAddress( m_host );
}


void Smb4KIPLookupJob::slotReadStandardError()
{
  qDebug() << m_proc->readAllStandardError();
}


void Smb4KIPLookupJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus status )
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
  emit finished( m_host );
}


Smb4KIPAddressScannerPrivate::Smb4KIPAddressScannerPrivate()
{
}


Smb4KIPAddressScannerPrivate::~Smb4KIPAddressScannerPrivate()
{
}

#include "smb4kipaddressscanner_p.moc"

