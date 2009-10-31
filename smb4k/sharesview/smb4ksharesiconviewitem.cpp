/***************************************************************************
    smb4ksharesiconviewitem  -  The items for Smb4K's shares icon view.
                             -------------------
    begin                : Di Dez 5 2006
    copyright            : (C) 2006-2008 by Alexander Reinholdt
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

// KDE includes
#include <kiconeffect.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

// application specific includes
#include <smb4ksharesiconviewitem.h>
#include <smb4ksharesiconview.h>

Smb4KSharesIconViewItem::Smb4KSharesIconViewItem( Smb4KShare *share, Smb4KSharesIconView *parent )
: QListWidgetItem( parent )
{
  setFlags( flags() | Qt::ItemIsDropEnabled );
  m_data.setShare( share );
  setupItem( m_data.share(), m_data.showMountPoint() );
}


Smb4KSharesIconViewItem::~Smb4KSharesIconViewItem()
{
  // Do not touch the Smb4KShare object!
}


void Smb4KSharesIconViewItem::setupItem( Smb4KShare *share, bool mountpoint )
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

  setIcon( KIcon( m_data.pixmap( KIconLoader::SizeMedium ) ) );

  // Set up the text.
  setText( (mountpoint ? m_data.share()->path() : m_data.share()->unc()) );

  m_data.setShare( share );
  m_data.setShowMountPoint( mountpoint );
}


void Smb4KSharesIconViewItem::setShowMountPoint( bool show )
{
  setupItem( m_data.share(), show );
}


bool Smb4KSharesIconViewItem::sameShareObject( Smb4KShare *share )
{
  Q_ASSERT( share );

  return m_data.share()->equals( share, Smb4KShare::LocalOnly );
}


void Smb4KSharesIconViewItem::replaceShareObject( Smb4KShare *share )
{
  setupItem( share, m_data.showMountPoint() );
}
