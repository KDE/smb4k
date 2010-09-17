/***************************************************************************
    smb4ksynchronizer_p  -  This file contains private helper classes for
    the Smb4KSynchronizer class.
                             -------------------
    begin                : Fr Okt 24 2008
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

// KDE includes
#include <kdebug.h>

// application specific includes
#include <smb4ksynchronizer_p.h>
#include <smb4kcoremessage.h>
#include <smb4knotification.h>


SynchronizationThread::SynchronizationThread( QObject *parent )
: QThread( parent )
{
  m_proc = NULL;
  m_info = NULL;
}


SynchronizationThread::~SynchronizationThread()
{
}


void SynchronizationThread::synchronize( Smb4KSynchronizationInfo *info, const QString &command )
{
  Q_ASSERT( info );
  Q_ASSERT( !command.isEmpty() );

  m_info = info;

  // Create the process and set the command.
  m_proc = new Smb4KProcess( Smb4KProcess::Synchronize, this );

  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessOutput() ) );
  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->start();
}


void SynchronizationThread::slotProcessOutput()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

  for ( int i = 0; i < stdout.size(); ++i )
  {
    if ( stdout.at( i )[0].isSpace() )
    {
      // Get transfer rate
      if ( stdout.at( i ).contains( "/s ", Qt::CaseSensitive ) )
      {
        QString rate = stdout.at( i ).section( "/s ", 0, 0 ).section( " ", -1, -1 ).trimmed();
        rate.append( "/s" );
        rate.insert( rate.length() - 4, " " );
        m_info->setTransferRate( rate );
      }
      else
      {
        // No transfer rate available
      }

      // Get the transfer progress of the file
      if ( stdout.at( i ).contains( "% ", Qt::CaseSensitive ) )
      {
        QString progress = stdout.at( i ).section( "% ", 0, 0 ).section( " ", -1, -1 ).trimmed();
        m_info->setCurrentProgress( progress.toInt() );
      }
      else
      {
        // No transfer progress available
      }

      // Get the overall transfer progress
      if ( stdout.at( i ).contains( " to-check=" ) )
      {
        QString tmp = stdout.at( i ).section( " to-check=", 1, 1 ).section( ")", 0, 0 ).trimmed();

        double files_to_process = tmp.section( "/", 0, 0 ).trimmed().toInt();
        double total_files      = tmp.section( "/", 1, 1 ).trimmed().toInt();
        double progress         = ((total_files - files_to_process) * 100) / total_files;

        m_info->setTotalFileNumber( total_files );
        m_info->setTotalProgress( progress );
      }
      else
      {
        // No overall transfer progress can be determined
      }

      // Get the number of processed files
      if ( stdout.at( i ).contains( "xfer#", Qt::CaseSensitive ) )
      {
        int processed_files = stdout.at( i ).section( "xfer#", 1, 1 ).section( ",", 0, 0 ).trimmed().toInt();
        m_info->setProcessedFileNumber( processed_files );
      }
      else
      {
        // No number of processed files available
      }
    }
    else
    {
      // This is the file name or some other important information
      m_info->setText( stdout.at( i ).trimmed() );
    }

    emit progress( m_info );
  }
}


void SynchronizationThread::slotProcessError()
{
  m_stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  // Avoid reporting an error if the process was killed by calling the abort() function.
  if ( !m_proc->isAborted() && (m_stderr.contains( "rsync error:" ) && !m_stderr.contains( "(code 23)" )
       /*ignore "some files were not transferred" error*/) )
  {
    m_proc->abort();
    
    Smb4KNotification *notification = new Smb4KNotification();
    notification->synchronizationFailed( m_info, m_stderr );
  }
  else
  {
    // Go ahead
  }
}


void SynchronizationThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() && m_stderr.contains( "rsync error:" ) )
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
      break;
    }
  }

  exit( exitCode );
}


Smb4KSynchronizerPrivate::Smb4KSynchronizerPrivate()
{
}


Smb4KSynchronizerPrivate::~Smb4KSynchronizerPrivate()
{
}

#include "smb4ksynchronizer_p.moc"

