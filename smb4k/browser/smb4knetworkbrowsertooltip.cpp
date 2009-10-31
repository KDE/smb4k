/***************************************************************************
    smb4knetworkbrowsertooltip  -  Tool tip for the network browser.
                             -------------------
    begin                : Sa Jan 20 2007
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
#include <qglobal.h>
#if QT_VERSION < 0x040400
#include <QToolTip>
#endif

// KDE includes
#include <klocale.h>
#include <kiconcache.h>
#include <kiconloader.h>
#include <kdebug.h>

// application specific includes
#include <smb4knetworkbrowsertooltip.h>
#include <smb4knetworkbrowseritem.h>
#include <core/smb4kworkgroup.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>


static bool first_time = true;

Smb4KNetworkBrowserToolTip::Smb4KNetworkBrowserToolTip( QWidget *parent )
: QLabel( parent, Qt::ToolTip ), m_item( NULL ), m_cleared( true )
{
#if QT_VERSION >= 0x040400
  setAutoFillBackground( true );
  setBackgroundRole( QPalette::ToolTipBase );
  setForegroundRole( QPalette::ToolTipText );
#else
  setPalette( QToolTip::palette() );
#endif
  setLineWidth( 1 );
  setMidLineWidth( 1 );
  setFrameShape( Box );
  setFrameShadow( Plain );
  setMouseTracking( true );

  m_layout = NULL;

  m_workgroup_label = NULL;
  m_workgroup = NULL;
  m_master_browser_label = NULL;
  m_master_browser = NULL;
  m_host_label = NULL;
  m_host = NULL;
  m_comment_label = NULL;
  m_comment = NULL;
  m_ip_label = NULL;
  m_ip = NULL;
  m_os_label = NULL;
  m_os = NULL;
  m_server_label = NULL;
  m_server = NULL;
  m_line = NULL;
  m_share_label = NULL;
  m_share = NULL;
  m_type_label = NULL;
  m_type = NULL;
  m_mounted_label = NULL;
  m_mounted = NULL;
  m_icon = NULL;
}


Smb4KNetworkBrowserToolTip::~Smb4KNetworkBrowserToolTip()
{
  // Never touch the Smb4KNetworkBrowserItem object here
}


void Smb4KNetworkBrowserToolTip::setupToolTip( Smb4KNetworkBrowserItem *item )
{
  if ( item )
  {
    if ( !m_cleared )
    {
      clearToolTip();
    }
    else
    {
      // Do nothing
    }

    m_item = item;

    m_layout = new QGridLayout( this );
    m_layout->setSpacing( 5 );

    switch ( m_item->type() )
    {
      case Smb4KNetworkBrowserItem::Workgroup:
      {
        m_workgroup_label =          new QLabel( i18n( "Workgroup:" ), this );
        m_workgroup =                new QLabel( m_item->workgroupItem()->workgroupName() , this );

        QString master_label_entry = m_item->workgroupItem()->hasPseudoMasterBrowser() ?
                                     i18n( "Pseudo master browser:" ) :
                                     i18n( "Master browser:" );

        m_master_browser_label =     new QLabel( master_label_entry, this );

        QString master_entry =       m_item->workgroupItem()->masterBrowserIP().isEmpty() ?
                                     (m_item->workgroupItem()->masterBrowserName().isEmpty() ?
                                     i18n( "unknown" ) :
                                     m_item->workgroupItem()->masterBrowserName()) :
                                     m_item->workgroupItem()->masterBrowserName() +
                                     " ("+m_item->workgroupItem()->masterBrowserIP()+")";
        m_master_browser =           new QLabel( master_entry, this );

        m_layout->addWidget( m_workgroup_label, 0, 1, 0 );
        m_layout->addWidget( m_workgroup, 0, 2, 0 );
        m_layout->addWidget( m_master_browser_label, 1, 1, 0 );
        m_layout->addWidget( m_master_browser, 1, 2, 0 );

        break;
      }
      case Smb4KNetworkBrowserItem::Host:
      {
        m_host_label =             new QLabel( i18n( "Host:" ), this );
        m_host =                   new QLabel( m_item->hostItem()->hostName(), this );

        m_comment_label =          new QLabel( i18n( "Comment:" ), this );
        m_comment =                new QLabel( m_item->hostItem()->comment(), this );

        m_ip_label =               new QLabel( i18n( "IP address:" ), this );
        QString ip_entry =         m_item->hostItem()->ip().isEmpty() ?
                                   i18n( "unknown" ) :
                                   m_item->hostItem()->ip();
        m_ip =                     new QLabel( ip_entry, this );

        m_os_label =               new QLabel( i18n( "Operating system:" ), this );
        m_os =                     new QLabel( m_item->hostItem()->osString().isEmpty() ?
                                   i18n( "unknown" ) :
                                   m_item->hostItem()->osString(), this );

        m_server_label =           new QLabel( i18n( "Server string:" ), this );
        m_server =                 new QLabel( m_item->hostItem()->serverString().isEmpty() ?
                                   i18n( "unknown" ) :
                                   m_item->hostItem()->serverString(), this );

        m_line = new QFrame( this );
        m_line->setLineWidth( 1 );
        m_line->setMidLineWidth( 0 );
        m_line->setFixedWidth( 100 );
        m_line->setFrameShape( QFrame::HLine );
        m_line->setFrameShadow( QFrame::Plain );

        m_workgroup_label =  new QLabel( i18n( "Workgroup:" ), this );
        m_workgroup =        new QLabel( m_item->hostItem()->workgroupName(), this );

        Smb4KWorkgroup *workgroupItem = static_cast<Smb4KNetworkBrowserItem *>( m_item->parent() )->workgroupItem();

        m_master_browser_label =   new QLabel( i18n( "Master browser:" ), this );
        m_master_browser =         new QLabel( (workgroupItem && !workgroupItem->masterBrowserName().isEmpty()) ?
                                   workgroupItem->masterBrowserName() :
                                   i18n( "unknown" ), this );

        m_layout->addWidget( m_host_label, 0, 1, 0 );
        m_layout->addWidget( m_host, 0, 2, 0 );
        m_layout->addWidget( m_comment_label, 1, 1, 0 );
        m_layout->addWidget( m_comment, 1, 2, 0 );
        m_layout->addWidget( m_ip_label, 2, 1, 0 );
        m_layout->addWidget( m_ip, 2, 2, 0 );
        m_layout->addWidget( m_os_label, 3, 1, 0 );
        m_layout->addWidget( m_os, 3, 2, 0 );
        m_layout->addWidget( m_server_label, 4, 1, 0 );
        m_layout->addWidget( m_server, 4, 2, 0 );
        m_layout->addWidget( m_line, 5, 1, 1, 2, Qt::AlignCenter );
        m_layout->addWidget( m_workgroup_label, 6, 1, 0 );
        m_layout->addWidget( m_workgroup, 6, 2, 0 );
        m_layout->addWidget( m_master_browser_label, 7, 1, 0 );
        m_layout->addWidget( m_master_browser, 7, 2, 0 );

        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        m_share_label =         new QLabel( i18n( "Share:" ), this );
        m_share =               new QLabel( m_item->shareItem()->shareName(), this );

        m_comment_label =       new QLabel( i18n( "Comment:" ), this );
        m_comment =             new QLabel( m_item->shareItem()->comment(), this );

        m_type_label =          new QLabel( i18n( "Type:" ), this );
        m_type =                new QLabel( m_item->shareItem()->translatedType(), this );

        if ( !m_item->shareItem()->isPrinter() )
        {
          m_mounted_label =     new QLabel( i18n( "Mounted:" ), this );

          m_mounted =           new QLabel( m_item->shareItem()->isMounted() ?
                                i18n( "yes" ) :
                                i18n( "no" ), this );
        }
        else
        {
          // Do nothing
        }

        m_line = new QFrame( this );
        m_line->setLineWidth( 1 );
        m_line->setMidLineWidth( 0 );
        m_line->setFixedWidth( 100 );
        m_line->setFrameShape( QFrame::HLine );
        m_line->setFrameShadow( QFrame::Plain );

        m_host_label =          new QLabel( i18n( "Host:" ), this );
        m_host =                new QLabel( m_item->shareItem()->hostName(), this );

        m_ip_label =            new QLabel( i18n( "IP address:" ), this );
        m_ip =                  new QLabel( !m_item->shareItem()->hostIP().isEmpty() ?
                                m_item->shareItem()->hostIP() :
                                i18n( "unknown" ), this );

        m_layout->addWidget( m_share_label, 0, 1, 0 );
        m_layout->addWidget( m_share, 0, 2, 0 );
        m_layout->addWidget( m_comment_label, 1, 1, 0 );
        m_layout->addWidget( m_comment, 1, 2, 0 );
        m_layout->addWidget( m_type_label, 2, 1, 0 );
        m_layout->addWidget( m_type, 2, 2, 0 );

        if ( !m_item->shareItem()->isPrinter() )
        {
          m_layout->addWidget( m_mounted_label, 3, 1, 0 );
          m_layout->addWidget( m_mounted, 3, 2, 0 );
          m_layout->addWidget( m_line, 4, 1, 1, 2, Qt::AlignCenter );
          m_layout->addWidget( m_host_label, 5, 1, 0 );
          m_layout->addWidget( m_host, 5, 2, 0 );
          m_layout->addWidget( m_ip_label, 6, 1, 0 );
          m_layout->addWidget( m_ip, 6, 2, 0 );
        }
        else
        {
          m_layout->addWidget( m_line, 3, 1, 1, 2, Qt::AlignCenter );
          m_layout->addWidget( m_host_label, 4, 1, 0 );
          m_layout->addWidget( m_host, 4, 2, 0 );
          m_layout->addWidget( m_ip_label, 5, 1, 0 );
          m_layout->addWidget( m_ip, 5, 2, 0 );
        }

        break;
      }
      default:
      {
        break;
      }
    }

    m_icon = new QLabel( this );
    m_icon->setPixmap( m_item->desktopIcon() );

    m_layout->addWidget( m_icon, 0, 0, m_layout->rowCount(), 1, Qt::AlignCenter );

    m_cleared = false;
  }
  else
  {
    return;
  }

  // The first time the tool tip is shown,
  // we should not run QGridLayout::activate()
  // because it *redoes* the layout.
  if ( !first_time )
  {
    m_layout->activate();
  }
  else
  {
    // Do nothing
  }

  adjustSize();
}


void Smb4KNetworkBrowserToolTip::clearToolTip()
{
  if ( m_item )
  {
    switch ( m_item->type() )
    {
      case Smb4KNetworkBrowserItem::Workgroup:
      {
        delete m_workgroup_label;
        m_workgroup_label = NULL;

        delete m_workgroup;
        m_workgroup = NULL;

        delete m_master_browser_label;
        m_master_browser_label = NULL;

        delete m_master_browser;
        m_master_browser = NULL;

        break;
      }
      case Smb4KNetworkBrowserItem::Host:
      {
        delete m_host_label;
        m_host_label = NULL;

        delete m_host;
        m_host = NULL;

        delete m_comment_label;
        m_comment_label = NULL;

        delete m_comment;
        m_comment = NULL;

        delete m_ip_label;
        m_ip_label = NULL;

        delete m_ip;
        m_ip = NULL;

        delete m_os_label;
        m_os_label = NULL;

        delete m_os;
        m_os = NULL;

        delete m_server_label;
        m_server_label = NULL;

        delete m_server;
        m_server = NULL;

        delete m_line;
        m_line = NULL;

        delete m_workgroup_label;
        m_workgroup_label = NULL;

        delete m_workgroup;
        m_workgroup = NULL;

        delete m_master_browser_label;
        m_master_browser_label = NULL;

        delete m_master_browser;
        m_master_browser = NULL;

        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        delete m_share_label;
        m_share_label = NULL;

        delete m_share;
        m_share = NULL;

        delete m_comment_label;
        m_comment_label = NULL;

        delete m_comment;
        m_comment = NULL;

        delete m_type_label;
        m_type_label = NULL;

        delete m_type;
        m_type = NULL;

        delete m_mounted_label;
        m_mounted_label = NULL;

        delete m_mounted;
        m_mounted = NULL;

        delete m_line;
        m_line = NULL;

        delete m_host_label;
        m_host_label = NULL;

        delete m_host;
        m_host = NULL;

        delete m_ip_label;
        m_ip_label = NULL;

        delete m_ip;
        m_ip = NULL;

        break;
      }
      default:
      {
        break;
      }
    }

    delete m_icon;
    m_icon = NULL;

    m_item = NULL;
  }
  else
  {
    // Do nothing
  }

  delete m_layout;
  m_layout = NULL;

  m_cleared = true;
}


void Smb4KNetworkBrowserToolTip::update()
{
  if ( !isVisible() || isCleared() )
  {
    return;
  }

  switch ( m_item->type() )
  {
    case Smb4KNetworkBrowserItem::Workgroup:
    {
      // Update the information about the master browser.
      if ( !m_item->workgroupItem()->masterBrowserIP().isEmpty() )
      {
        m_master_browser->setText( m_item->workgroupItem()->masterBrowserName() +
                                   " ("+m_item->workgroupItem()->masterBrowserIP()+")" );
      }
      else
      {
        m_master_browser->setText( m_item->workgroupItem()->masterBrowserName() );
      }

      break;
    }
    case Smb4KNetworkBrowserItem::Host:
    {
      // Update/set the information about the operating system.
      if ( !m_item->hostItem()->osString().isEmpty() )
      {
        m_os->setText( m_item->hostItem()->osString() );
      }
      else
      {
        m_os->setText( i18n( "unknown" ) );
      }

      // Update/set the information about the server.
      if ( !m_item->hostItem()->serverString().isEmpty() )
      {
        m_server->setText( m_item->hostItem()->serverString() );
      }
      else
      {
        m_server->setText( i18n( "unknown" ) );
      }

      // Update/set the information about the IP address.
      if ( !m_item->hostItem()->ip().isEmpty() )
      {
        m_ip->setText( m_item->hostItem()->ip() );
      }
      else
      {
        m_ip->setText( i18n( "unknown" ) );
      }

      break;
    }
    case Smb4KNetworkBrowserItem::Share:
    {
      // Update/set the information about the IP address.
      if ( !m_item->shareItem()->hostIP().isEmpty() )
      {
        m_ip->setText( m_item->shareItem()->hostIP() );
      }
      else
      {
        m_ip->setText( i18n( "unknown" ) );
      }

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkBrowserToolTip::mousePressEvent( QMouseEvent *e )
{
  setVisible( false );
  QLabel::mousePressEvent( e );
}


void Smb4KNetworkBrowserToolTip::leaveEvent( QEvent *e )
{
  setVisible( false );
  QLabel::leaveEvent( e );
}


void Smb4KNetworkBrowserToolTip::showEvent( QShowEvent *e )
{
  if ( first_time )
  {
    first_time = false;
  }
  else
  {
    // Do nothing
  }

  emit aboutToShow( m_item );
  QLabel::showEvent( e );
}


void Smb4KNetworkBrowserToolTip::hideEvent( QHideEvent *e )
{
  emit aboutToHide();
  QLabel::hideEvent( e );
}

#include "smb4knetworkbrowsertooltip.moc"
