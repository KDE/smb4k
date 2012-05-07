/***************************************************************************
    smb4ksearchdialogitem  -  This class is an enhanced version of a list
    box item for Smb4K.
                             -------------------
    begin                : So Jun 3 2007
    copyright            : (C) 2007-2012 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

// KDE includes
#include <kiconloader.h>
#include <klocale.h>
#include <kicon.h>

// application specific includes
#include <smb4knetworksearchitem.h>
#include <smb4knetworksearch.h>
#include <core/smb4khost.h>


Smb4KNetworkSearchItem::Smb4KNetworkSearchItem( KListWidget *listWidget, Smb4KShare *share )
: QListWidgetItem( listWidget, Share )
{
  m_share = new Smb4KShare( *share );
  setupItem();
}


Smb4KNetworkSearchItem::Smb4KNetworkSearchItem( KListWidget *listWidget )
: QListWidgetItem( listWidget, Failure )
{
  m_share = NULL;
  setupItem();
}


Smb4KNetworkSearchItem::~Smb4KNetworkSearchItem()
{
  delete m_share;
}


void Smb4KNetworkSearchItem::setupItem()
{
  switch( type() )
  {
    case Share:
    {
      setText( m_share->unc() );

      if ( m_share->isMounted() )
      {
        QStringList overlays;
        overlays.append( "emblem-mounted" );
        setIcon( KIcon( "folder-remote", KIconLoader::global(), overlays ) );
      }
      else
      {
        setIcon( KIcon( "folder-remote" ) );
      }

      break;
    }
    case Failure:
    {
      setText( i18n( "The search returned no results." ) );
      setIcon( KIcon( "dialog-error" ) );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkSearchItem::setMounted( bool mounted )
{
  m_share->setIsMounted( mounted );
  setupItem();
}

