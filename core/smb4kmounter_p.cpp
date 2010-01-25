/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
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
#include <kshell.h>
#include <kmessagebox.h>

// application specific includes
#include <smb4kmounter_p.h>
#include <smb4kauthinfo.h>
#include <smb4kcoremessage.h>


BasicMountThread::BasicMountThread( Type type, Smb4KShare *share, QObject *parent )
: QThread( parent ), m_type( type ), m_share( *share )
{
}


BasicMountThread::~BasicMountThread()
{
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

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::MergedChannels ); // see below
#ifndef __FreeBSD__
  m_proc->setEnv( "PASSWD", !authInfo->password().isEmpty() ? authInfo->password() : "", true );
#endif
  m_proc->start();
}


void MountThread::slotProcessError()
{
  // The Samba programs report their errors to stdout. So,
  // get the (merged) output from stdout and check if there
  // was an error.
  QString stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).trimmed();

  if ( !stdout.isEmpty() )
  {
#ifndef __FreeBSD__
    if ( stdout.contains( "mount error 13", Qt::CaseSensitive ) || stdout.contains( "mount error(13)" )
         /* authentication error */ )
    {
      emit authError( &m_share );
    }
    else if ( (stdout.contains( "mount error 6" ) || stdout.contains( "mount error(6)" )) /* bad share name */ &&
              m_share.shareName().contains( "_", Qt::CaseSensitive ) )
    {
      emit badShareName( &m_share );
    }
    else if ( stdout.contains( "mount error 101" ) || stdout.contains( "mount error(101)" ) /* network unreachable */ )
    {
      kDebug() << "Network unreachable ..." << endl;
    }
#else
    if ( stdout.contains( "Authentication error" ) )
    {
      emit authError( &m_share );
    }
#endif
    else
    {
      Smb4KCoreMessage::error( ERROR_MOUNTING_SHARE, m_share.unc(), stdout );
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

  connect( m_proc, SIGNAL( readyReadStandardError() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotProcessError() ) );
  connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

  m_proc->setShellCommand( command );
  m_proc->setOutputChannelMode( KProcess::MergedChannels );  // see below
  m_proc->start();
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


void CheckThread::check()
{
  struct statvfs vfs;

  if ( statvfs( m_share->canonicalPath(), &vfs ) == -1 )
  {
    m_share->setInaccessible( true );
    m_share->setFreeDiskSpace( -1 );
    m_share->setTotalDiskSpace( -1 );
  }
  else
  {
    m_share->setInaccessible( false );

    double kB_block = (double)(vfs.f_bsize / 1000);
    double total = (double)(vfs.f_blocks*kB_block);
    double free = (double)(vfs.f_bfree*kB_block);

    m_share->setFreeDiskSpace( free );
    m_share->setTotalDiskSpace( total );
  }

  // Determine the file system. Expect under Solaris, we need
  // to use statfs for that.
  struct statfs fs;

  if ( statfs( m_share->canonicalPath(), &fs ) == -1 )
  {
    m_share->setFileSystem( Smb4KShare::Unknown );
  }
  else
  {
#ifndef __FreeBSD__
    if ( (uint)fs.f_type == 0xFF534D42 )
    {
      m_share->setFileSystem( Smb4KShare::CIFS );
    }
    else if ( (uint)fs.f_type == 0x517B )
    {
      m_share->setFileSystem( Smb4KShare::SMBFS );
    }
    else
    {
      m_share->setFileSystem( Smb4KShare::Unknown );
    }
#else
    // FIXME: Can we also use f_type??
    if ( !strncmp( fs.f_fstypename, "smbfs", strlen( fs.f_fstypename ) ) )
    {
      m_share->setFileSystem( Smb4KShare::SMBFS );
    }
    else
    {
      m_share->setFileSystem( Smb4KShare::Unknown );
    }
#endif
  }

  // Determine the owner and the group of the m_share if
  // necessary.
  if ( !m_share->uidIsSet() || !m_share->gidIsSet() )
  {
    struct stat buf;

    if ( lstat( m_share->canonicalPath(), &buf ) == -1 )
    {
      m_share->setUID( (uid_t)-1 );
      m_share->setGID( (gid_t)-1 );
    }
    else
    {
      m_share->setUID( buf.st_uid );
      m_share->setGID( buf.st_gid );
    }
  }
  else
  {
    // Do nothing
  }

  quit();
}


Smb4KMounterPrivate::Smb4KMounterPrivate()
: m_quit( false ), m_hardware( false )
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


#include "smb4kmounter_p.moc"
