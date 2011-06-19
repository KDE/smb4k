/***************************************************************************
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.
                             -------------------
    begin                : Mo Jan 8 2007
    copyright            : (C) 2007-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QApplication>
#include <QBrush>

// KDE includes
#include <kiconloader.h>
#include <kdebug.h>

// application specific includes
#include <smb4knetworkbrowseritem.h>


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem( QTreeWidget *parent, Smb4KWorkgroup *workgroup )
: QTreeWidgetItem( parent, Workgroup ), m_workgroup( *workgroup )
{
  setText( Network, m_workgroup.workgroupName() );
  setIcon( Network, m_workgroup.icon() );
}


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem( QTreeWidgetItem *parent, Smb4KHost *host )
: QTreeWidgetItem( parent, Host ), m_host( *host )
{
  setText( Network, m_host.hostName() );
  setText( IP, m_host.ip() );
  setText( Comment, m_host.comment() );

  if ( m_host.isMasterBrowser() )
  {
    for ( int i = 0; i < columnCount(); ++i )
    {
      QBrush brush( Qt::darkBlue );
      setForeground( i, brush );
    }
  }
  else
  {
    // Do nothing
  }

  setIcon( Network, m_host.icon() );
}


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem( QTreeWidgetItem *parent, Smb4KShare *share )
: QTreeWidgetItem( parent, Share ), m_share( *share )
{
  setText( Network, m_share.shareName() );
  setText( Type, m_share.translatedTypeString() );
  setText( Comment, m_share.comment() );

  if ( !m_share.isPrinter() && m_share.isMounted() )
  {
    for ( int i = 0; i < columnCount(); ++i )
    {
      QFont f = font( i );
      f.setItalic( true );
      setFont( i, f );
    }
  }
  else
  {
    // Do nothing
  }
  
  setIcon( Network, m_share.icon() );
}


Smb4KNetworkBrowserItem::~Smb4KNetworkBrowserItem()
{
}


Smb4KWorkgroup *Smb4KNetworkBrowserItem::workgroupItem()
{
  return (type() == Workgroup ? &m_workgroup : NULL);
}


Smb4KHost *Smb4KNetworkBrowserItem::hostItem()
{
  return (type() == Host ? &m_host : NULL);
}


Smb4KShare *Smb4KNetworkBrowserItem::shareItem()
{
  return (type() == Share ? &m_share : NULL);
}


Smb4KBasicNetworkItem* Smb4KNetworkBrowserItem::networkItem()
{
  switch ( type() )
  {
    case Workgroup:
    {
      return &m_workgroup;
    }
    case Host:
    {
      return &m_host;
    }
    case Share:
    {
      return &m_share;
    }
    default:
    {
      break;
    }
  }
  
  return NULL;
}


void Smb4KNetworkBrowserItem::update( Smb4KBasicNetworkItem *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KBasicNetworkItem::Workgroup:
      {
        if ( type() != Workgroup )
        {
          return;
        }
        else
        {
          // Do nothing
        }
        
        m_workgroup = *(static_cast<Smb4KWorkgroup *>( item ));
        break;
      }
      case Smb4KBasicNetworkItem::Host:
      {
        if ( type() != Host )
        {
          return;
        }
        else
        {
          // Do nothing
        }
        
        m_host = *(static_cast<Smb4KHost *>( item ));
        
        // Adjust the item's color.
        if ( m_host.isMasterBrowser() )
        {
          for ( int i = 0; i < columnCount(); ++i )
          {
            QBrush brush( Qt::darkBlue );
            setForeground( i, brush );
          }
        }
        else
        {
          for ( int i = 0; i < columnCount(); ++i )
          {
            QBrush brush = QApplication::palette().text();
            setForeground( i, brush );
          }          
        }
        
        // Set the IP address
        setText( IP, m_host.ip() );

        // Set the comment 
        setText( Comment, m_host.comment() );
        break;
      }
      case Smb4KBasicNetworkItem::Share:
      {
        if ( type() != Share )
        {
          return;
        }
        else
        {
          // Do nothing
        }
        
        m_share = *(static_cast<Smb4KShare *>( item ));

        // Set the comment.
        setText( Comment, m_share.comment() );
    
        // Set the icon
        setIcon( Network, m_share.icon() );
            
        // Set the font
        for ( int i = 0; i < columnCount(); ++i )
        {
          QFont f = font( i );
          f.setItalic( m_share.isMounted() );
          setFont( i, f );
        }
        
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
}

