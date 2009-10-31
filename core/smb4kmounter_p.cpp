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

// application specific includes
#include <smb4kmounter_p.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4ksambaoptionsinfo.h>
#include <smb4ksettings.h>
#include <smb4kdefs.h>


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
