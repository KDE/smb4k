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
#include <QHostAddress>

// KDE includes
#include <kdebug.h>

// application specific includes
#include <smb4ksearch_p.h>
#include <smb4kcoremessage.h>
#include <smb4khost.h>
#include <smb4kshare.h>
#include <smb4kbasicnetworkitem.h>
#include <smb4kauthinfo.h>


SearchThread::SearchThread( QObject *parent )
: QThread( parent ), m_auth_error( false )
{
  m_proc = NULL;
}


SearchThread::~SearchThread()
{
}


void SearchThread::search( const QString &searchItem, Smb4KAuthInfo *authInfo, const QString &command )
{
  Q_ASSERT( !searchItem.isEmpty() );
  Q_ASSERT( !command.isEmpty() );

  m_string = searchItem;

  m_proc = new Smb4KProcess( Smb4KProcess::Search, this );

  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessOutput() ) );
  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setEnv( "PASSWD", !authInfo->password().isEmpty() ? authInfo->password() : "", true );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->start();
}


void SearchThread::slotProcessOutput()
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


void SearchThread::slotProcessError()
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
      m_auth_error = true;
      // Abort the process.
      m_proc->abort();
    }
    else
    {
      Smb4KCoreMessage::error( ERROR_SEARCHING, QString(), stderr );
    }
  }
  else
  {
    Smb4KCoreMessage::error( ERROR_SEARCHING, QString(), stderr );
  }
}


void SearchThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
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
      break;
    }
  }

  exit( exitCode );
}


Smb4KSearchPrivate::Smb4KSearchPrivate()
{
}


Smb4KSearchPrivate::~Smb4KSearchPrivate()
{
}

#include "smb4ksearch_p.moc"

