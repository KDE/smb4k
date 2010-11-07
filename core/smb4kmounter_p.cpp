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
#include <kdiskfreespaceinfo.h>

// application specific includes
#include <smb4kmounter_p.h>


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
