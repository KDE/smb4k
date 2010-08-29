/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
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

// Qt includes
#include <QFileInfo>

// KDE includes
#include <kshell.h>
#include <kmessagebox.h>
#include <kdiskfreespaceinfo.h>

// application specific includes
#include <smb4kmounter_p.h>
#include <smb4kauthinfo.h>
#include <smb4kcoremessage.h>
#include <smb4knotification.h>


BasicMountThread::BasicMountThread( Type type, Smb4KShare *share, QObject *parent )
: QThread( parent ), m_type( type ), m_share( *share ), m_auth_error( false ),
  m_bad_name_error( false ), m_start_detached( false )
{
}


BasicMountThread::~BasicMountThread()
{
}


void BasicMountThread::setStartDetached( bool on )
{
  m_start_detached = on;
}



MountThread::MountThread( Smb4KShare *share, QObject *parent )
: BasicMountThread( BasicMountThread::MountThread, share, parent )
{
}


MountThread::~MountThread()
{
}


void MountThread::mount( Smb4KAuthInfo *authInfo, const QString &command )
{
  Q_ASSERT( authInfo );
  Q_ASSERT( !command.isEmpty() );

  m_proc = new Smb4KProcess( Smb4KProcess::Mount, this );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::MergedChannels ); // see below
#ifndef Q_OS_FREEBSD
  m_proc->setEnv( "PASSWD", !authInfo->password().isEmpty() ? authInfo->password() : "", true );
#endif
  if ( !m_start_detached )
  {
    connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
    connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessError() ) );
    connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );
    
    m_proc->start();    
  }
  else
  {
    m_proc->startDetached();
    quit();
  }
}


void MountThread::slotProcessError()
{
  // The Samba programs report their errors to stdout. So,
  // get the (merged) output from stdout and check if there
  // was an error.
  QString stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).trimmed();

  if ( !stdout.isEmpty() )
  {
#ifndef Q_OS_FREEBSD
    if ( stdout.contains( "mount error 13", Qt::CaseSensitive ) || stdout.contains( "mount error(13)" )
         /* authentication error */ )
    {
      m_auth_error = true;
    }
    else if ( (stdout.contains( "mount error 6" ) || stdout.contains( "mount error(6)" )) /* bad share name */ &&
              m_share.shareName().contains( "_", Qt::CaseSensitive ) )
    {
      m_bad_name_error = true;
    }
    else if ( stdout.contains( "mount error 101" ) || stdout.contains( "mount error(101)" ) /* network unreachable */ )
    {
      qDebug() << "Network unreachable ..." << endl;
    }
#else
    if ( stdout.contains( "Authentication error" ) )
    {
      m_auth_error = true;
    }
#endif
    else
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->mountingFailed( &m_share, stdout ); 
    }
  }
  else
  {
    // Do nothing
  }
}


void MountThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
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
      emit mounted( &m_share );
      break;
    }
  }
  
  exit( exitCode );
}


UnmountThread::UnmountThread( Smb4KShare *share, QObject *parent )
: BasicMountThread( BasicMountThread::UnmountThread, share, parent )
{
}


UnmountThread::~UnmountThread()
{
}


void UnmountThread::unmount( const QString &command )
{
  Q_ASSERT( !command.isEmpty() );

  m_proc = new Smb4KProcess( Smb4KProcess::Unmount, this );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::MergedChannels );  // see below
  
  if ( !m_start_detached )
  {
    connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
    connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessError() ) );
    connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );
    
    m_proc->start();
  }
  else
  {
    m_proc->startDetached();
    quit();
  }
}


void UnmountThread::slotProcessError()
{
  // The Samba programs report their errors to stdout. So,
  // get the (merged) output from stdout and check if there
  // was an error.
  QString stderr = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).trimmed();

  if ( !stderr.isEmpty() )
  {
    // Filter out everything we do not consider an error.
    QStringList before = stderr.split( "\n" );
    QStringList after;

    foreach ( const QString &str, before )
    {
      if ( !str.trimmed().startsWith( "smb4k_umount(" ) &&
           !str.trimmed().startsWith( "sudo: unable to resolve host" ) )
      {
        // This is neither a sudo problem nor debug information. We are
        // indeed facing a problem.
        after += str;
      }
      else
      {
        // Do nothing
      }
    }

    stderr = after.join( "\n" );

    if ( !stderr.trimmed().isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_UNMOUNTING_SHARE, m_share.unc(), stderr );
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


void UnmountThread::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
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
      emit unmounted( &m_share );
      break;
    }
  }
  
  exit( exitCode );
}


CheckThread::CheckThread( Smb4KShare *share, QObject *parent )
: QThread( parent ), m_share( share )
{
  Q_ASSERT( m_share );
}


CheckThread::~CheckThread()
{
  // Do not delete the m_share here, because it's not
  // owned by this class!
}


void CheckThread::run()
{
  // Get the info about the usage, etc.
  KDiskFreeSpaceInfo space_info = KDiskFreeSpaceInfo::freeSpaceInfo( m_share->canonicalPath() );
  
  if ( space_info.isValid() )
  {
    m_share->setInaccessible( false );
    m_share->setFreeDiskSpace( space_info.available() );
    m_share->setTotalDiskSpace( space_info.size() );
    m_share->setUsedDiskSpace( space_info.used() );
  }
  else
  {
    m_share->setInaccessible( true );
    m_share->setFreeDiskSpace( 0 );
    m_share->setTotalDiskSpace( 0 );
    m_share->setUsedDiskSpace( 0 );
  }
  
  // Get the owner and group. Check also if we can
  // really access the mount point.
  QFileInfo file_info( m_share->canonicalPath() );
  file_info.setCaching( false );
  
  if ( file_info.exists() )
  {
    m_share->setUID( (K_UID)file_info.ownerId() );
    m_share->setGID( (K_GID)file_info.groupId() );
    
    if ( !m_share->isInaccessible() )
    {
      m_share->setInaccessible( !(file_info.isDir() && file_info.isExecutable()) );
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    m_share->setUID( (K_UID)-1 );
    m_share->setGID( (K_GID)-1 );
    m_share->setInaccessible( true );
  }
  
  // NOTE: We do not need to check the file system here, because
  // it has either already been checked by the import() function or 
  // a proper file system was used by the mountShare() function.
}


Smb4KMounterPrivate::Smb4KMounterPrivate()
: m_quit( false ), m_hardware( false ), m_pending_remounts( 0 ), m_initial_remounts( 0 ),
  m_pending_unmounts( 0 ), m_initial_unmounts( 0 )
{
}


Smb4KMounterPrivate::~Smb4KMounterPrivate()
{
}


void Smb4KMounterPrivate::setAboutToQuit()
{
  m_quit = true;
}


void Smb4KMounterPrivate::setHardwareReason( bool hardware )
{
  m_hardware = hardware;
}


void Smb4KMounterPrivate::addRemount()
{
  m_initial_remounts++;
  m_pending_remounts++;
}


void Smb4KMounterPrivate::removeRemount()
{
  m_pending_remounts--;
}


void Smb4KMounterPrivate::clearRemounts()
{
  m_initial_remounts = 0;
  m_pending_remounts = 0;
}


void Smb4KMounterPrivate::addUnmount()
{
  m_initial_unmounts++;
  m_pending_unmounts++;
}


void Smb4KMounterPrivate::removeUnmount()
{
  m_pending_unmounts--;
}


void Smb4KMounterPrivate::clearUnmounts()
{
  m_initial_unmounts = 0;
  m_pending_unmounts = 0;
}

#include "smb4kmounter_p.moc"
