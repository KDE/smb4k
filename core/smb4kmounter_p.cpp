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


CheckThread::CheckThread() : QThread()
{
}


CheckThread::~CheckThread()
{
  // Do not delete the share here, because it's not
  // owned by this class!
}


void CheckThread::check( Smb4KShare *share )
{
  Q_ASSERT( share );

  struct statvfs vfs;

  if ( statvfs( share->canonicalPath(), &vfs ) == -1 )
  {
    share->setInaccessible( true );
    share->setFreeDiskSpace( -1 );
    share->setTotalDiskSpace( -1 );
  }
  else
  {
    share->setInaccessible( false );

    double kB_block = (double)(vfs.f_bsize / 1000);
    double total = (double)(vfs.f_blocks*kB_block);
    double free = (double)(vfs.f_bfree*kB_block);

    share->setFreeDiskSpace( free );
    share->setTotalDiskSpace( total );
  }

  // Determine the file system. Expect under Solaris, we need
  // to use statfs for that.
  struct statfs fs;

  if ( statfs( share->canonicalPath(), &fs ) == -1 )
  {
    share->setFileSystem( Smb4KShare::Unknown );
  }
  else
  {
#ifndef __FreeBSD__
    if ( (uint)fs.f_type == 0xFF534D42 )
    {
      share->setFileSystem( Smb4KShare::CIFS );
    }
    else if ( (uint)fs.f_type == 0x517B )
    {
      share->setFileSystem( Smb4KShare::SMBFS );
    }
    else
    {
      share->setFileSystem( Smb4KShare::Unknown );
    }
#else
    // FIXME: Can we also use f_type??
    if ( !strncmp( fs.f_fstypename, "smbfs", strlen( fs.f_fstypename ) ) )
    {
      share->setFileSystem( Smb4KShare::SMBFS );
    }
    else
    {
      share->setFileSystem( Smb4KShare::Unknown );
    }
#endif
  }

  // Determine the owner and the group of the share if
  // necessary.
  if ( !share->uidIsSet() || !share->gidIsSet() )
  {
    struct stat buf;

    if ( lstat( share->canonicalPath(), &buf ) == -1 )
    {
      share->setUID( (uid_t)-1 );
      share->setGID( (gid_t)-1 );
    }
    else
    {
      share->setUID( buf.st_uid );
      share->setGID( buf.st_gid );
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
