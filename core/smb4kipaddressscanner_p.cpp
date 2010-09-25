/***************************************************************************
    smb4kipaddressscanner_p  -  Private classes for the IP address scanner
    of Smb4K.
                             -------------------
    begin                : Mi Jan 28 2009
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

// KDE includes
#include <kdebug.h>

// application specific includes
#include <smb4kipaddressscanner_p.h>
#include <smb4khost.h>
#include <smb4knotification.h>


IPScanThread::IPScanThread( Smb4KHost *host, QObject *parent )
: QThread( parent ), m_host( host )
{
  Q_ASSERT( m_host );
  m_proc = NULL;
}


IPScanThread::~IPScanThread()
{
}


void IPScanThread::lookup( const QString &command )
{
  Q_ASSERT( !command.isEmpty() );

  m_proc = new Smb4KProcess( Smb4KProcess::LookupIP, this );

  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessOutput() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->start();
}


void IPScanThread::slotProcessOutput()
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


void IPScanThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
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
      break;
    }
  }
  
  exit( exitCode );
}


Smb4KIPAddressScannerPrivate::Smb4KIPAddressScannerPrivate()
{
}


Smb4KIPAddressScannerPrivate::~Smb4KIPAddressScannerPrivate()
{
}

#include "smb4kipaddressscanner_p.moc"

