/***************************************************************************
    smb4kpreviewer_p  -  Private helper classes for Smb4KPreviewer class.
                             -------------------
    begin                : So Dez 21 2008
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

// KDE includes
#include <kdebug.h>

// application specific includes
#include <smb4kpreviewer_p.h>
#include <smb4kauthinfo.h>
#include <smb4knotification.h>


PreviewThread::PreviewThread( Smb4KPreviewItem *item, QObject *parent )
: QThread( parent ), m_item( item ), m_auth_error( false )
{
  m_proc = NULL;
}


PreviewThread::~PreviewThread()
{
}


void PreviewThread::preview( Smb4KAuthInfo *authInfo, const QString &command )
{
  Q_ASSERT( !command.isEmpty() );

  // Make sure there is no old contents left.
  m_item->clearContents();

  // Create the process and set the command.
  m_proc = new Smb4KProcess( Smb4KProcess::Preview, this );

  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessOutput() ) );
  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setEnv( "PASSWD", !authInfo->password().isEmpty() ? authInfo->password() : "", true );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->start();
}


void PreviewThread::slotProcessOutput()
{
  Q_ASSERT( m_item );

  QStringList list = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

  foreach ( const QString &line, list )
  {
    if ( line.contains( "blocks of size" ) || line.contains( "Domain=[" ) )
    {
      // FIXME: Do we need this at all?
      continue;
    }
    else
    {
      QString tmp = line.trimmed().section( "        ", 0, -2 ).trimmed();
      QString name = tmp.section( "  ", 0, -2 ).trimmed().isEmpty() ?
                     tmp : tmp.section( "  ", 0, -2 ).trimmed();

      QString dir_string = tmp.right( 3 ).trimmed();
      bool is_dir = (!dir_string.isEmpty() && dir_string.contains( "D" ));

      if ( !name.isEmpty() )
      {
        if ( is_dir )
        {
          if ( name.startsWith( "." ) &&
              (QString::compare( name, "." ) != 0 && QString::compare( name, ".." ) != 0) )
          {
            m_item->addContents( ContentsItem( Smb4KPreviewItem::HiddenDirectory, name ) );
          }
          else
          {
            m_item->addContents( ContentsItem( Smb4KPreviewItem::Directory, name ) );
          }
        }
        else
        {
          if ( name.startsWith( "." ) )
          {
            m_item->addContents( ContentsItem( Smb4KPreviewItem::HiddenFile, name ) );
          }
          else
          {
            m_item->addContents( ContentsItem( Smb4KPreviewItem::File, name ) );
          }
        }
      }
      else
      {
        continue;
      }
    }
  }

  emit result( m_item );
}


void PreviewThread::slotProcessError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    QStringList errors = stderr.split( "\n", QString::SkipEmptyParts );

    if ( errors.size() == 1 && (errors.first().trimmed().startsWith( "Domain" ) ||
         errors.first().trimmed().startsWith( "OS" )) )
    {
      // Ignore this "error" message and proceed with the code below. We
      // have to do this, because - unfortunately - smbclient reports the
      // domain name, the operating system and the server to stderr.
    }
    else
    {
      if ( stderr.contains( "NT_STATUS_ACCESS_DENIED", Qt::CaseSensitive ) ||
           stderr.contains( "NT_STATUS_LOGON_FAILURE", Qt::CaseSensitive ) )
      {
        // Authentication error.
        m_auth_error = true;
      }
      else
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->retrievingPreviewFailed( m_item->share(), stderr );
      }
    }
  }
  else
  {
    // No output.
  }

  // Clear the item's contents.
  m_item->clearContents();
}


void PreviewThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  switch ( exitStatus )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );;
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


Smb4KPreviewerPrivate::Smb4KPreviewerPrivate()
{
}


Smb4KPreviewerPrivate::~Smb4KPreviewerPrivate()
{
}

#include "smb4kpreviewer_p.moc"

