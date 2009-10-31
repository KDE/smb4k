/***************************************************************************
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.
                             -------------------
    begin                : Mo Jan 8 2007
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
  m_icon = KIcon( "network-workgroup" );
  setIcon( Network, m_icon );
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

  m_icon = KIcon( "network-server" );
  setIcon( Network, m_icon );
}


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem( QTreeWidgetItem *parent, Smb4KShare *share )
: QTreeWidgetItem( parent, Share ), m_share( *share )
{
  setText( Network, m_share.shareName() );
  setText( Type, m_share.translatedType() );
  setText( Comment, m_share.comment() );

  if ( !m_share.isPrinter() )
  {
    QStringList overlays;

    if ( m_share.isMounted() )
    {
      overlays.append( "emblem-mounted" );

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

    m_icon = KIcon( "folder-remote", KIconLoader::global(), overlays );
    setIcon( Network, m_icon );
  }
  else
  {
    m_icon = KIcon( "printer" );
    setIcon( Network, m_icon );
  }
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


void Smb4KNetworkBrowserItem::update( Smb4KWorkgroup *workgroup )
{
  if ( workgroup )
  {
    m_workgroup = *workgroup;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserItem::update( Smb4KHost *host )
{
  if ( host )
  {
    // Adjust the item's color if necessary.
    if ( host->isMasterBrowser() )
    {
      if ( !m_host.isMasterBrowser() )
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
    }
    else
    {
      if ( m_host.isMasterBrowser() )
      {
        for ( int i = 0; i < columnCount(); ++i )
        {
          QBrush brush = QApplication::palette().text();
          setForeground( i, brush );
        }
      }
      else
      {
        // Do nothing
      }
    }

    // Set the IP address if necessary.
    if ( !host->ip().isEmpty() )
    {
      if ( QString::compare( host->ip(), m_host.ip() ) != 0 )
      {
        setText( IP, host->ip() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      if ( !m_host.ip().isEmpty() )
      {
        setText( IP, host->ip() );
      }
      else
      {
        // Do nothing
      }
    }

    // Set the comment if necessary.
    if ( !host->comment().isEmpty() )
    {
      if ( QString::compare( host->comment(), m_host.comment() ) != 0 )
      {
        setText( Comment, host->comment() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      if ( !m_host.comment().isEmpty() )
      {
        setText( Comment, host->comment() );
      }
      else
      {
        // Do nothing
      }
    }

    m_host = *host;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserItem::update( Smb4KShare *share )
{
  if ( share )
  {
    // Adjust the item's font and its icon if necessary.
    setMounted( share );

    // Set the comment if necessary.
    if ( !share->comment().isEmpty() )
    {
      if ( QString::compare( share->comment(), m_share.comment() ) != 0 )
      {
        setText( Comment, share->comment() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      if ( !m_share.comment().isEmpty() )
      {
        setText( Comment, share->comment() );
      }
      else
      {
        // Do nothing
      }
    }

    m_share = *share;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserItem::setMounted( Smb4KShare *share, MountFlag flag )
{
  if ( share )
  {
    switch ( flag )
    {
      case Mounted:
      {
        // Set the font
        for ( int i = 0; i < columnCount(); ++i )
        {
          QFont f = font( i );
          f.setItalic( true );
          setFont( i, f );
        }

        // Set the icon
        QStringList overlays;
        overlays.append( "emblem-mounted" );
        m_icon = KIcon( "folder-remote", KIconLoader::global(), overlays );
        setIcon( Network, m_icon );

        // Set the mount releated data of the share.
        m_share.setMountData( share );

        break;
      }
      case NotMounted:
      {
        // Set the font
        for ( int i = 0; i < columnCount(); ++i )
        {
          QFont f = font( i );
          f.setItalic( false );
          setFont( i, f );
        }

        // Set the icon
        m_icon = KIcon( "folder-remote" );
        setIcon( Network, m_icon );

        // Reset the mount related data of the share.
        m_share.resetMountData();

        break;
      }
      default:
      {
        if ( share->isMounted() )
        {
          // Set the font
          for ( int i = 0; i < columnCount(); ++i )
          {
            QFont f = font( i );
            f.setItalic( true );
            setFont( i, f );
          }

          // Set the icon
          QStringList overlays;
          overlays.append( "emblem-mounted" );
          m_icon = KIcon( "folder-remote", KIconLoader::global(), overlays );
          setIcon( Network, m_icon );

          // Set the mount releated data of the share.
          m_share.setMountData( share );
        }
        else
        {
          // Set the font
          for ( int i = 0; i < columnCount(); ++i )
          {
            QFont f = font( i );
            f.setItalic( false );
            setFont( i, f );
          }

          // Set the icon
          m_icon = KIcon( "folder-remote" );
          setIcon( Network, m_icon );

          // Reset the mount related data of the share.
          m_share.resetMountData();
        }

        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
}


QPixmap Smb4KNetworkBrowserItem::desktopIcon() const
{
  return m_icon.pixmap( KIconLoader::global()->currentSize( KIconLoader::Desktop ) );
}
