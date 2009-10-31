/***************************************************************************
    smb4kprint_p  -  This file contains private helpers for the
    Smb4KPrint class
                             -------------------
    begin                : Fr Okt 31 2008
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
#include <smb4kprint_p.h>
#include <smb4kprocess.h>
#include <smb4kcoremessage.h>
#include <smb4kauthinfo.h>


PrintThread::PrintThread( QObject *parent )
: QThread( parent )
{
  m_proc = NULL;
  m_info = NULL;
}


PrintThread::~PrintThread()
{
}


void PrintThread::print( Smb4KPrintInfo *info, Smb4KAuthInfo *authInfo, const QString &command )
{
  Q_ASSERT( info );
  Q_ASSERT( !command.isEmpty() );

  m_info = info;

  m_proc = new Smb4KProcess( Smb4KProcess::Print, this );
  m_proc->setShellCommand( command );
  m_proc->setEnv( "DEVICE_URI", info->deviceURI(), true );
  m_proc->setEnv( "PASSWD", !authInfo->password().isEmpty() ? authInfo->password() : "", true );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );

  switch ( m_proc->execute() )
  {
    case -2:
    {
      Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, m_proc->error() );
      break;
    }
    case -1:
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
      QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 );

      if ( stderr.contains( "NT_STATUS_ACCESS_DENIED", Qt::CaseSensitive ) ||
           stderr.contains( "NT_STATUS_LOGON_FAILURE", Qt::CaseSensitive ) )
      {
        // An authentication error occurred.
        emit authError( m_info );
      }
      else
      {
        QStringList stderr_list = stderr.split( "\n", QString::SkipEmptyParts );

        // Remove unwanted lines.
        for ( int i = 0; i < stderr_list.size(); ++i )
        {
          if ( stderr_list.at( i ).contains( "DEBUG" ) )
          {
            // Debug info from smbspool.
            stderr_list[i].clear();
            continue;
          }
          else
          {
            continue;
          }
        }

        (void) stderr_list.removeAll( QString() );

        if ( !stderr_list.isEmpty() )
        {
          Smb4KCoreMessage::error( ERROR_PRINTING, info->filePath(), stderr );
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
  }

  exit( m_proc->exitCode() );
}


void PrintThread::setTempFilePath( const QString &path )
{
  m_temp = path;
}


Smb4KPrintPrivate::Smb4KPrintPrivate()
{
}


Smb4KPrintPrivate::~Smb4KPrintPrivate()
{
}

#include "smb4kprint_p.moc"

