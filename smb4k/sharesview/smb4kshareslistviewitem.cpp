/***************************************************************************
    smb4kshareslistviewitem  -  The shares list view item class of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
    copyright            : (C) 2007-2008 by Alexander Reinholdt
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
#include <QPixmap>
#include <QBrush>

// KDE includes
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

// application specific includes
#include <smb4kshareslistviewitem.h>
#include <smb4kshareslistview.h>


Smb4KSharesListViewItem::Smb4KSharesListViewItem( Smb4KShare *share, Smb4KSharesListView *parent )
: QTreeWidgetItem( parent )
{
  setFlags( flags() | Qt::ItemIsDropEnabled );

  m_data.setShare( share );

  setupItem( m_data.share(), m_data.showMountPoint() );
}


Smb4KSharesListViewItem::~Smb4KSharesListViewItem()
{
  // Do not touch the Smb4KShare object!
}


void Smb4KSharesListViewItem::setupItem( Smb4KShare *share, bool mountpoint )
{
  // Set up the icon.
  KIcon icon;

  QStringList overlays;
  overlays.append( "emblem-mounted" );

  if ( m_data.share()->isInaccessible() )
  {
    icon = KIcon( "folder-locked", KIconLoader::global(), overlays );
  }
  else
  {
    icon = KIcon( "folder-remote", KIconLoader::global(), overlays );
  }

  if ( m_data.share()->isForeign() )
  {
    m_data.setIcon( icon, QIcon::Disabled );
  }
  else
  {
    m_data.setIcon( icon );
  }

  setIcon( Item, KIcon( m_data.pixmap( KIconLoader::global()->currentSize( KIconLoader::Small ) ) ) );

  // Set up the text.
  setText( Item, (mountpoint ? m_data.share()->path() : m_data.share()->unc()) );
  setText( Owner, QString( "%1 - %2" ).arg( m_data.share()->owner() ).arg( m_data.share()->group() ) );

#ifndef __FreeBSD__
  setText( Login, (m_data.share()->fileSystem() == Smb4KShare::CIFS) ?
           m_data.share()->login() :
           QString() );
#endif

  setText( FileSystem, m_data.share()->fileSystemString().toUpper() );
  setText( Used, m_data.share()->usedDiskSpaceString() );
  setText( Free, m_data.share()->freeDiskSpaceString() );
  setText( Total, m_data.share()->totalDiskSpaceString() );
  setText( Usage, m_data.share()->diskUsageString() );

  // Alignment
  setTextAlignment( Used, Qt::AlignRight|Qt::AlignVCenter );
  setTextAlignment( Free, Qt::AlignRight|Qt::AlignVCenter );
  setTextAlignment( Total, Qt::AlignRight|Qt::AlignVCenter );
  setTextAlignment( Usage, Qt::AlignRight|Qt::AlignVCenter );

  m_data.setShare( share );
  m_data.setShowMountPoint( mountpoint );
}


void Smb4KSharesListViewItem::setShowMountPoint( bool show )
{
  setupItem( m_data.share(), show );
}


bool Smb4KSharesListViewItem::sameShareObject( Smb4KShare *share )
{
  Q_ASSERT( share );

  return m_data.share()->equals( share, Smb4KShare::LocalOnly );
}


void Smb4KSharesListViewItem::replaceShareObject( Smb4KShare *share )
{
  setupItem( share, m_data.showMountPoint() );
}
