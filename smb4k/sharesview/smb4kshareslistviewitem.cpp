/***************************************************************************
    smb4kshareslistviewitem  -  The shares list view item class of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
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
#include <QPixmap>
#include <QBrush>

// KDE includes
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>

// application specific includes
#include <smb4kshareslistviewitem.h>
#include <smb4kshareslistview.h>


Smb4KSharesListViewItem::Smb4KSharesListViewItem( Smb4KSharesListView *parent, Smb4KShare *share, bool mountpoint )
: QTreeWidgetItem( parent ), m_share( *share ), m_mountpoint( mountpoint )
{
  setFlags( flags() | Qt::ItemIsDropEnabled );
  
  // Set up the text.
  if ( !m_mountpoint )
  {
    setText( Item, m_share.unc() );
  }
  else
  {
    setText( Item, m_share.path() );
  }

  setText( Owner, QString( "%1 - %2" ).arg( m_share.owner() ).arg( m_share.group() ) );

#ifndef Q_OS_FREEBSD
  switch ( m_share.fileSystem() )
  {
    case Smb4KShare::CIFS:
    {
      if ( !m_share.login().isEmpty() )
      {
        setText( Login, m_share.login() );
      }
      else
      {
        setText( Login, i18n( "unknown" ) );
      }
      break;
    }
    default:
    {
      setText( Login, "-" );
      break;
    }
  }
#endif

  setText( FileSystem, m_share.fileSystemString().toUpper() );
  setText( Used, m_share.usedDiskSpaceString() );
  setText( Free, m_share.freeDiskSpaceString() );
  setText( Total, m_share.totalDiskSpaceString() );
  setText( Usage, m_share.diskUsageString() );

  // Alignment
  setTextAlignment( Used, Qt::AlignRight|Qt::AlignVCenter );
  setTextAlignment( Free, Qt::AlignRight|Qt::AlignVCenter );
  setTextAlignment( Total, Qt::AlignRight|Qt::AlignVCenter );
  setTextAlignment( Usage, Qt::AlignRight|Qt::AlignVCenter );

  setIcon( Item, m_share.icon() );
}


Smb4KSharesListViewItem::~Smb4KSharesListViewItem()
{
  // Do not touch the Smb4KShare object!
}


// void Smb4KSharesListViewItem::setupItem( Smb4KShare *share, bool mountpoint )
// {
// 
//   // Set up the text.
//   setText( Item, (mountpoint ? m_data.share()->path() : m_data.share()->unc()) );
//   setText( Owner, QString( "%1 - %2" ).arg( m_data.share()->owner() ).arg( m_data.share()->group() ) );
// 
// #ifndef __FreeBSD__
//   setText( Login, (m_data.share()->fileSystem() == Smb4KShare::CIFS) ?
//            m_data.share()->login() :
//            QString() );
// #endif
// 
//   setText( FileSystem, m_data.share()->fileSystemString().toUpper() );
//   setText( Used, m_data.share()->usedDiskSpaceString() );
//   setText( Free, m_data.share()->freeDiskSpaceString() );
//   setText( Total, m_data.share()->totalDiskSpaceString() );
//   setText( Usage, m_data.share()->diskUsageString() );
// 
//   // Alignment
//   setTextAlignment( Used, Qt::AlignRight|Qt::AlignVCenter );
//   setTextAlignment( Free, Qt::AlignRight|Qt::AlignVCenter );
//   setTextAlignment( Total, Qt::AlignRight|Qt::AlignVCenter );
//   setTextAlignment( Usage, Qt::AlignRight|Qt::AlignVCenter );
// 
//   m_data.setShare( share );
//   m_data.setShowMountPoint( mountpoint );
// }


void Smb4KSharesListViewItem::setShowMountPoint( bool show )
{
  m_mountpoint = show;
  update( &m_share );
}


void Smb4KSharesListViewItem::update( Smb4KShare *share )
{
  m_share = *share;
  
  // Set up the text.
  if ( !m_mountpoint )
  {
    setText( Item, m_share.unc() );
  }
  else
  {
    setText( Item, m_share.path() );
  }

  setText( Owner, QString( "%1 - %2" ).arg( m_share.owner() ).arg( m_share.group() ) );

#ifndef __FreeBSD__
  switch ( m_share.fileSystem() )
  {
    case Smb4KShare::CIFS:
    {
      if ( !m_share.login().isEmpty() )
      {
        setText( Login, m_share.login() );
      }
      else
      {
        setText( Login, i18n( "unknown" ) );
      }
      break;
    }
    default:
    {
      setText( Login, "-" );
      break;
    }
  }
#endif

  setText( FileSystem, m_share.fileSystemString().toUpper() );
  setText( Used, m_share.usedDiskSpaceString() );
  setText( Free, m_share.freeDiskSpaceString() );
  setText( Total, m_share.totalDiskSpaceString() );
  setText( Usage, m_share.diskUsageString() );

  setIcon( Item, m_share.icon() );
}

