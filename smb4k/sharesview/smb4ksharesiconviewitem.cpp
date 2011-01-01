/***************************************************************************
    smb4ksharesiconviewitem  -  The items for Smb4K's shares icon view.
                             -------------------
    begin                : Di Dez 5 2006
    copyright            : (C) 2006-2010 by Alexander Reinholdt
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

// KDE includes
#include <kiconeffect.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

// application specific includes
#include <smb4ksharesiconviewitem.h>
#include <smb4ksharesiconview.h>

Smb4KSharesIconViewItem::Smb4KSharesIconViewItem( Smb4KSharesIconView *parent, Smb4KShare *share, bool mountpoint )
: QListWidgetItem( parent ), m_share( *share ), m_mountpoint( mountpoint )
{
  setFlags( flags() | Qt::ItemIsDropEnabled );

  if ( !m_mountpoint )
  {
    setText( m_share.unc() );
  }
  else
  {
    setText( m_share.path() );
  }

  setIcon( m_share.icon() );
}


Smb4KSharesIconViewItem::~Smb4KSharesIconViewItem()
{
  // Do not touch the Smb4KShare object!
}


void Smb4KSharesIconViewItem::setShowMountPoint( bool show )
{
  m_mountpoint = show;
  update( &m_share );
}


void Smb4KSharesIconViewItem::update( Smb4KShare *share )
{
  m_share = *share;
  
  if ( !m_mountpoint )
  {
    setText( m_share.unc() );
  }
  else
  {
    setText( m_share.path() );
  }

  setIcon( m_share.icon() );
}
