/***************************************************************************
    smb4ksharesviewitemdata  -  This class is a container for the widget
    independed data needed by the various parts of the shares view.
                             -------------------
    begin                : Di Jun 3 2008
    copyright            : (C) 2008 by Alexander Reinholdt
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
#include <smb4ksharesviewitemdata.h>

Smb4KSharesViewItemData::Smb4KSharesViewItemData()
: m_share( Smb4KShare() ), m_show_mountpoint( false )
{
}


Smb4KSharesViewItemData::~Smb4KSharesViewItemData()
{
}


void Smb4KSharesViewItemData::setShare( Smb4KShare *share )
{
  m_share = *share;
}


void Smb4KSharesViewItemData::setShowMountPoint( bool show )
{
  m_show_mountpoint = show;
}


void Smb4KSharesViewItemData::setIcon( const QIcon &icon, QIcon::Mode mode, QIcon::State state )
{
  m_icon = icon;
  m_mode = mode;
  m_state = state;
}


QPixmap Smb4KSharesViewItemData::pixmap( int size ) const
{
  return m_icon.pixmap( size, m_mode, m_state );
}


QPixmap Smb4KSharesViewItemData::pixmap( const QSize &size ) const
{
  return m_icon.pixmap( size, m_mode, m_state );
}
