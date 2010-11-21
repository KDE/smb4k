/***************************************************************************
    smb4ksearchdialogitem  -  This class is an enhanced version of a list
    box item for Smb4K.
                             -------------------
    begin                : So Jun 3 2007
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

// KDE includes
#include <kiconloader.h>
#include <klocale.h>
#include <kicon.h>

// application specific includes
#include <smb4knetworksearchitem.h>
#include <smb4knetworksearch.h>
#include <core/smb4khost.h>


Smb4KNetworkSearchItem::Smb4KNetworkSearchItem( KListWidget *listWidget, Smb4KHost *host )
: QListWidgetItem( listWidget, Host ), m_host( *host ), m_share( 0 ), m_is_known( false ),
  m_is_mounted( false )
{
  setupItem();
}


Smb4KNetworkSearchItem::Smb4KNetworkSearchItem( KListWidget *listWidget, Smb4KShare *share )
: QListWidgetItem( listWidget, Share ), m_host( 0 ), m_share( *share ), m_is_known( false ),
  m_is_mounted( false )
{
  setupItem();
}


Smb4KNetworkSearchItem::Smb4KNetworkSearchItem( KListWidget *listWidget )
: QListWidgetItem( listWidget, Failure ), m_host( 0 ), m_share( 0 ), m_is_known( false ),
  m_is_mounted( false )
{
  setupItem();
}


Smb4KNetworkSearchItem::~Smb4KNetworkSearchItem()
{
}


void Smb4KNetworkSearchItem::setupItem()
{
  switch( type() )
  {
    case Host:
    {
      setText( m_host.hostName() );

      if ( m_is_known )
      {
        QStringList overlays;
        overlays.append( "dialog-ok-apply" );

        setIcon( KIcon( "network-server", KIconLoader::global(), overlays ) );
      }
      else
      {
        setIcon( KIcon( "network-server" ) );
      }

      break;
    }
    case Share:
    {
      setText( m_share.unc() );

      if ( m_is_mounted )
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


void Smb4KNetworkSearchItem::setKnown( bool known )
{
  m_is_known = known;
  setupItem();
}


void Smb4KNetworkSearchItem::setMounted( bool mounted )
{
  m_is_mounted = mounted;
  setupItem();
}

