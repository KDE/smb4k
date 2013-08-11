/***************************************************************************
    smb4ktooltip  -  Provides tooltips for Smb4K
                             -------------------
    begin                : Sa Dez 23 2010
    copyright            : (C) 2010-2013 by Alexander Reinholdt
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
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4ktooltip.h"
#include "core/smb4kbasicnetworkitem.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QLabel>
#include <QToolTip>
#include <QStylePainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QDesktopWidget>
#include <QPalette>

// KDE includes
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kwindowsystem.h>
#include <kcolorscheme.h>

Smb4KToolTip::Smb4KToolTip( QWidget* parent )
: QWidget( parent, Qt::ToolTip|Qt::BypassGraphicsProxyWidget|Qt::FramelessWindowHint ),
  m_item( NULL ), m_tip_layout( NULL ), m_info_layout( NULL ), m_text_layout( NULL )
{
  m_master_browser_label = NULL;
  m_comment_label        = NULL;
  m_server_label         = NULL;
  m_os_label             = NULL;
  m_ip_label             = NULL;
  m_mounted_label        = NULL;
  m_size_label           = NULL;
  
  setAttribute( Qt::WA_TranslucentBackground );
  
  // Copied from QToolTip
  setForegroundRole( QPalette::ToolTipText );
  setBackgroundRole( QPalette::ToolTipBase );
  setPalette( QToolTip::palette() );
  ensurePolished();
  setWindowOpacity( style()->styleHint( QStyle::SH_ToolTipLabel_Opacity, 0, this ) / 255.0 );
} 


Smb4KToolTip::~Smb4KToolTip()
{
  // Never delete m_item here. We only have a pointer to 
  // somewhere outside of this class.
}


void Smb4KToolTip::setup(Smb4KToolTip::Parent parent, Smb4KBasicNetworkItem* item)
{
  if ( item )
  {
    m_item = item;
  
    // Set up tool tip.
    switch ( parent )
    {
      case NetworkBrowser:
      {
        setupNetworkBrowserToolTip();
        break;
      }
      case SharesView:
      {
        setupSharesViewToolTip();
        break;
      }
      default:
      {
        return;
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KToolTip::update(Smb4KToolTip::Parent parent, Smb4KBasicNetworkItem* item)
{
  if ( item )
  {
    m_item = item;
    
    switch ( parent )
    {
      case NetworkBrowser:
      {
        switch ( m_item->type() )
        {
          case Smb4KBasicNetworkItem::Workgroup:
          {
            Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( item );
            
            if ( !workgroup->masterBrowserIP().isEmpty() )
            {
              m_master_browser_label->setText( workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIP()+')' );
            }
            else
            {
              m_master_browser_label->setText( workgroup->masterBrowserName() );
            }
            break;
          }
          case Smb4KBasicNetworkItem::Host:
          {
            Smb4KHost *host = static_cast<Smb4KHost *>( item );
            
            if ( !host->comment().isEmpty() )
            {
              m_comment_label->setText( host->comment() );
            }
            else
            {
              m_comment_label->setText( "-" );
            }
            
            if ( !host->serverString().isEmpty() )
            {
              m_server_label->setText( host->serverString() );
            }
            else
            {
              m_server_label->setText( "-" );
            }
            
            if ( !host->osString().isEmpty() )
            {
              m_os_label->setText( host->osString() );
            }
            else
            {
              m_os_label->setText( "-" );
            }
            
            if ( !host->ip().isEmpty() )
            {
              m_ip_label->setText( host->ip() );
            }
            else
            {
              m_ip_label->setText( "-" );
            }
            break;
          }
          case Smb4KBasicNetworkItem::Share:
          {
            Smb4KShare *share = static_cast<Smb4KShare *>( item );
            
            if ( !share->comment().isEmpty() )
            {
              m_comment_label->setText( share->comment() );
            }
            else
            {
              m_comment_label->setText( "-" );
            }
            
            if ( !share->isPrinter() )
            {
              if ( share->isMounted() )
              {
                m_mounted_label->setText( i18n( "yes" ) );
              }
              else
              {
                m_mounted_label->setText( i18n( "no" ) );
              }
            }
            else
            {
              m_mounted_label->setText( "-" );
            }
            
            if ( !share->hostIP().isEmpty() )
            {
              m_ip_label->setText( share->hostIP() );
            }
            else
            {
              m_ip_label->setText( "-" );
            }
            
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
      case SharesView:
      {
        Smb4KShare *share = static_cast<Smb4KShare *>( item );
        
        if ( share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0 )
        {
          m_size_label->setText( i18n( "%1 free of %2 (%3 used)",
                                 share->freeDiskSpaceString(),
                                 share->totalDiskSpaceString(),
                                 share->diskUsageString() ) );
        }
        else
        {
          m_size_label->setText( i18n( "unknown" ) );
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

 
void Smb4KToolTip::show(const QPoint &pos)
{
  QPoint p;
  
  QDesktopWidget *d = QApplication::desktop();

  if ( pos.x() + width() > d->width() )
  {
    p.setX( pos.x() - width() - 5 );
  }
  else
  {
    p.setX( pos.x() + 5 );
  }

  if ( pos.y() + height() > d->height() )
  {
    p.setY( pos.y() - height() - 5 );
  }
  else
  {
    p.setY( pos.y() + 5 );
  }

  move( p );
  setVisible( true );
  
  QTimer::singleShot( 10000, this, SLOT(slotHideToolTip()) );
}
 
 
void Smb4KToolTip::hide()
{
  setVisible( false );
}


void Smb4KToolTip::setupNetworkBrowserToolTip()
{
  // NOTE: If you change the layout here, adjust also the update function!
  
  m_tip_layout = new QHBoxLayout( this );
  m_tip_layout->setAlignment( Qt::AlignTop );
  m_info_layout = new QVBoxLayout();
  m_info_layout->setAlignment( Qt::AlignTop );

  // Set the icon
  QLabel *icon_label = new QLabel( this );
  icon_label->setPixmap( m_item->icon().pixmap( KIconLoader::SizeEnormous ) );
  
  m_tip_layout->addWidget( icon_label, Qt::AlignHCenter );
  m_tip_layout->addLayout( m_info_layout );
  
  // Use a brighter color for the left label. This was copied from
  // KFileMetaDataWidget class.
  QPalette p = palette();
  const QPalette::ColorRole role = foregroundRole();
  QColor textColor = p.color( role );
  textColor.setAlpha( 128 );
  p.setColor( role, textColor );
  
  // FIXME: Use smaller font for the information. Get the current 
  // point size of the window system with QFontInfo::pointSize().
  
  switch ( m_item->type() )
  {
    case Smb4KBasicNetworkItem::Workgroup:
    {
      Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( m_item );
      
      QLabel *caption = new QLabel( workgroup->workgroupName(), this );
      caption->setAlignment( Qt::AlignHCenter );
      QFont caption_font = caption->font();
      caption_font.setBold( true );
      caption->setFont( caption_font );
      
      m_info_layout->addWidget( caption );
      m_info_layout->addWidget( new KSeparator( this ), Qt::AlignHCenter );
      
      m_text_layout = new QGridLayout();
      
      QLabel *type_label = new QLabel( i18n( "Type" ), this );
      type_label->setPalette( p );
      
      m_text_layout->addWidget( type_label, 0, 0, Qt::AlignRight );
      m_text_layout->addWidget( new QLabel( i18n( "Workgroup" ), this ), 0, 1, 0 );
      
      QLabel *mb_label = new QLabel( i18n( "Master browser" ), this );
      mb_label->setPalette( p );
      
      m_text_layout->addWidget( mb_label, 1, 0, Qt::AlignRight );
      
      if ( !workgroup->masterBrowserIP().isEmpty() )
      {
        m_master_browser_label = new QLabel( workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIP()+')', this );
        m_text_layout->addWidget( m_master_browser_label, 1, 1, 0 );
      }
      else
      {
        m_master_browser_label = new QLabel( workgroup->masterBrowserName(), this );
        m_text_layout->addWidget( m_master_browser_label, 1, 1, 0 );
      }
      
      m_info_layout->addLayout( m_text_layout );
      m_info_layout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
      break;
    }
    case Smb4KBasicNetworkItem::Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( m_item );
      
      QLabel *caption = new QLabel( host->hostName(), this );
      caption->setAlignment( Qt::AlignHCenter );
      QFont caption_font = caption->font();
      caption_font.setBold( true );
      caption->setFont( caption_font );
      
      m_info_layout->addWidget( caption, Qt::AlignHCenter );
      m_info_layout->addWidget( new KSeparator( this ), Qt::AlignHCenter );

      m_text_layout = new QGridLayout();
      
      QLabel *type_label = new QLabel( i18n( "Type" ), this );
      type_label->setPalette( p );
      
      m_text_layout->addWidget( type_label, 0, 0, Qt::AlignRight );
      m_text_layout->addWidget( new QLabel( i18n( "Host" ), this ), 0, 1, 0 );
      
      QLabel *co_label = new QLabel( i18n( "Comment" ), this );
      co_label->setPalette( p );
      
      m_text_layout->addWidget( co_label, 1, 0, Qt::AlignRight );
      
      if ( !host->comment().isEmpty() )
      {
        m_comment_label = new QLabel( host->comment(), this );
        m_text_layout->addWidget( m_comment_label, 1, 1, 0 );
      }
      else
      {
        m_comment_label = new QLabel( "-", this );
        m_text_layout->addWidget( m_comment_label, 1, 1, 0 );
      }
      
      QLabel *srv_label = new QLabel( i18n( "Server" ), this );
      srv_label->setPalette( p );
      
      m_text_layout->addWidget( srv_label, 2, 0, Qt::AlignRight );
      
      if ( !host->serverString().isEmpty() )
      {
        m_server_label = new QLabel( host->serverString(), this );
        m_text_layout->addWidget( m_server_label, 2, 1, 0 );
      }
      else
      {
        m_server_label = new QLabel( "-", this );
        m_text_layout->addWidget( m_server_label, 2, 1, 0 );
      }
      
      QLabel *os_label = new QLabel( i18n( "Operating system" ), this );
      os_label->setPalette( p );
      
      m_text_layout->addWidget( os_label, 3, 0, Qt::AlignRight );
      
      if ( !host->osString().isEmpty() )
      {
        m_os_label = new QLabel( host->osString(), this );
        m_text_layout->addWidget( m_os_label, 3, 1, 0 );
      }
      else
      {
        m_os_label = new QLabel( "-", this );
        m_text_layout->addWidget( m_os_label, 3, 1, 0 );
      }
      
      QLabel *ip_label = new QLabel( i18n( "IP Address" ), this );
      ip_label->setPalette( p );
      
      m_text_layout->addWidget( ip_label, 4, 0, Qt::AlignRight );
      
      if ( !host->ip().isEmpty() )
      {
        m_ip_label = new QLabel( host->ip(), this );
        m_text_layout->addWidget( m_ip_label, 4, 1, 0 );
      }
      else
      {
        m_ip_label = new QLabel( "-", this );
        m_text_layout->addWidget( m_ip_label, 4, 1, 0 );
      }
      
      QLabel *wg_label = new QLabel( i18n( "Workgroup" ), this );
      wg_label->setPalette( p );
      
      m_text_layout->addWidget( wg_label, 5, 0, Qt::AlignRight );
      m_text_layout->addWidget( new QLabel( host->workgroupName(), this ), 5, 1, 0 );
      
      m_info_layout->addLayout( m_text_layout );
      m_info_layout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
      break;
    }
    case Smb4KBasicNetworkItem::Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>( m_item );
      
      QLabel *caption = new QLabel( share->shareName(), this );
      caption->setAlignment( Qt::AlignHCenter );
      QFont caption_font = caption->font();
      caption_font.setBold( true );
      caption->setFont( caption_font );
      
      m_info_layout->addWidget( caption );
      m_info_layout->addWidget( new KSeparator( this ), Qt::AlignHCenter );
      
      m_text_layout = new QGridLayout();
      
      QLabel *type_label = new QLabel( i18n( "Type" ), this );
      type_label->setPalette( p );
      
      m_text_layout->addWidget( type_label, 0, 0, Qt::AlignRight );
      m_text_layout->addWidget( new QLabel( i18n( "Share (%1)", share->translatedTypeString() ), this ), 0, 1, 0 );
      
      QLabel *co_label = new QLabel( i18n( "Comment" ), this );
      co_label->setPalette( p );
      
      m_text_layout->addWidget( co_label, 1, 0, Qt::AlignRight );
      
      if ( !share->comment().isEmpty() )
      {
        m_comment_label = new QLabel( share->comment(), this );
        m_text_layout->addWidget( m_comment_label, 1, 1, 0 );
      }
      else
      {
        m_comment_label = new QLabel( "-", this );
        m_text_layout->addWidget( m_comment_label, 1, 1, 0 );
      }
     
      QLabel *mnt_label = new QLabel( i18n( "Mounted" ), this );
      mnt_label->setPalette( p );
      
      m_text_layout->addWidget( mnt_label, 2, 0, Qt::AlignRight );
      
      if ( !share->isPrinter() )
      {
        if ( share->isMounted() )
        {
          m_mounted_label = new QLabel( i18n( "yes" ), this );
          m_text_layout->addWidget( m_mounted_label, 2, 1, 0 );
        }
        else
        {
          m_mounted_label = new QLabel( i18n( "no" ), this );
          m_text_layout->addWidget( m_mounted_label, 2, 1, 0 );
        }
      }
      else
      {
        m_mounted_label = new QLabel( "-", this );
        m_text_layout->addWidget( m_mounted_label, 2, 1, 0 );
      }
      
      QLabel *h_label = new QLabel( i18n( "Host" ), this );
      h_label->setPalette( p );
      
      m_text_layout->addWidget( h_label, 3, 0, Qt::AlignRight );
      m_text_layout->addWidget( new QLabel( share->hostName() ), 3, 1, 0 );

      QLabel *ip_label = new QLabel( i18n( "IP Address" ), this );
      ip_label->setPalette( p );
      
      m_text_layout->addWidget( ip_label, 4, 0, Qt::AlignRight );
      
      if ( !share->hostIP().isEmpty() )
      {
        m_ip_label = new QLabel( share->hostIP(), this );
        m_text_layout->addWidget( m_ip_label, 4, 1, 0 );
      }
      else
      {
        m_ip_label = new QLabel( "-", this );
        m_text_layout->addWidget( m_ip_label, 4, 1, 0 );
      }
      
      QLabel *unc_label = new QLabel( i18n( "UNC" ), this );
      unc_label->setPalette( p );
      
      m_text_layout->addWidget( unc_label, 5, 0, Qt::AlignRight );
      m_text_layout->addWidget( new QLabel( share->unc(), this ), 5, 1, 0 );
      
      m_info_layout->addLayout( m_text_layout );
      m_info_layout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
      break;
    }
    default:
    {
      return;
    }
  }
  
  adjustSize();
}


void Smb4KToolTip::setupSharesViewToolTip()
{
  // NOTE: If you change the layout here, adjust also the update function!
  
  Smb4KShare *share = static_cast<Smb4KShare *>( m_item );
  
  m_tip_layout = new QHBoxLayout( this );
  m_tip_layout->setAlignment( Qt::AlignTop );
  m_info_layout = new QVBoxLayout();
  m_info_layout->setAlignment( Qt::AlignTop );

  // Set the icon
  QLabel *icon_label = new QLabel( this );
  icon_label->setPixmap( share->icon().pixmap( KIconLoader::SizeEnormous ) );
  
  m_tip_layout->addWidget( icon_label, Qt::AlignHCenter );
  m_tip_layout->addLayout( m_info_layout );
  
  // Use a brighter color for the left label. This was copied from
  // KFileMetaDataWidget class.
  QPalette p = palette();
  const QPalette::ColorRole role = foregroundRole();
  QColor textColor = p.color( role );
  textColor.setAlpha( 128 );
  p.setColor( role, textColor );
  
  // FIXME: Use smaller font for the information. Get the current 
  // point size of the window system with QFontInfo::pointSize().
      
  QLabel *caption = new QLabel( share->shareName(), this );
  caption->setAlignment( Qt::AlignHCenter );
  QFont caption_font = caption->font();
  caption_font.setBold( true );
  caption->setFont( caption_font );
      
  m_info_layout->addWidget( caption );
  m_info_layout->addWidget( new KSeparator( this ), Qt::AlignHCenter );
      
  m_text_layout = new QGridLayout();
      
  QLabel *unc_label = new QLabel( i18n( "UNC" ), this );
  unc_label->setPalette( p );
      
  m_text_layout->addWidget( unc_label, 0, 0, Qt::AlignRight );
  m_text_layout->addWidget( new QLabel( share->unc(), this ), 0, 1, 0 );
      
  QLabel *mp_label = new QLabel( i18n( "Mountpoint" ), this );
  mp_label->setPalette( p );
      
  m_text_layout->addWidget( mp_label, 1, 0, Qt::AlignRight );
  m_text_layout->addWidget( new QLabel( share->path(), this ), 1, 1, 0 );

  QLabel *log_label = new QLabel( i18n( "Login" ), this );
  log_label->setPalette( p );
      
  m_text_layout->addWidget( log_label, 2, 0, Qt::AlignRight );
      
  switch ( share->fileSystem() )
  {
    case Smb4KShare::CIFS:
    case Smb4KShare::SMBFS:
    {
      if ( !share->login().isEmpty() )
      {
        m_text_layout->addWidget( new QLabel( share->login(), this ), 2, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( i18n( "unknown" ), this ), 2, 1, 0 );
      }
      break;
    }
    default:
    {
      m_text_layout->addWidget( new QLabel( "-", this ), 2, 1, 0 );
      break;
    }
  }
  
  QLabel *own_label = new QLabel( i18n( "Owner" ), this );
  own_label->setPalette( p );
      
  m_text_layout->addWidget( own_label, 3, 0, Qt::AlignRight );
  
  QString owner = (!share->owner().isEmpty() ? share->owner() : i18n( "unknown" ));
  QString group = (!share->group().isEmpty() ? share->group() : i18n( "unknown" ));
  
  m_text_layout->addWidget( new QLabel( QString( "%1 - %2" )
                            .arg( owner ).arg( group ), this ), 3, 1, 0 );
      
  QLabel *fs_label = new QLabel( i18n( "File system" ), this );
  fs_label->setPalette( p );
      
  m_text_layout->addWidget( fs_label, 4, 0, Qt::AlignRight );
  m_text_layout->addWidget( new QLabel( share->fileSystemString().toUpper() ), 4, 1, 0 );
  
  QLabel *s_label = new QLabel( i18n( "Size" ), this );
  s_label->setPalette( p );
  
  m_text_layout->addWidget( s_label, 5, 0, Qt::AlignRight );
  
  if ( share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0 )
  {
    m_size_label = new QLabel( i18n( "%1 free of %2 (%3 used)",
                               share->freeDiskSpaceString(),
                               share->totalDiskSpaceString(),
                               share->diskUsageString() ) );
    m_text_layout->addWidget( m_size_label, 5, 1, 0 );
  }
  else
  {
    m_size_label = new QLabel( i18n( "unknown" ) );
    m_text_layout->addWidget( m_size_label, 5, 1, 0 );
  }
      
  m_info_layout->addLayout( m_text_layout );
  m_info_layout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}


void Smb4KToolTip::updateNetworkBrowserToolTip()
{
  if ( m_item && m_text_layout && m_tip_layout )
  {
    switch ( m_item->type() )
    {
      case Smb4KBasicNetworkItem::Workgroup:
      {
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( m_item );
        
        // Master browser name and IP address
        QLayoutItem *mb_item = m_text_layout->itemAtPosition( 1, 1 );
        QLabel *mb_label = static_cast<QLabel *>( mb_item->widget() );
        
        if ( mb_label )
        {
          if ( !workgroup->hasMasterBrowserIP() )
          {
            mb_label->setText( workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIP()+')' );
          }
          else
          {
            mb_label->setText( workgroup->masterBrowserName() );
          }
        }
        else
        {
          // Do nothing
        }
        
        break;
      }
      case Smb4KBasicNetworkItem::Host:
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( m_item );

        // Comment
        QLayoutItem *co_item = m_text_layout->itemAtPosition( 1, 1 );
        QLabel *co_label = static_cast<QLabel *>( co_item->widget() );
        
        if ( co_label )
        {
          if ( !host->comment().isEmpty() )
          {
            co_label->setText( host->comment() );
          }
          else
          {
            co_label->setText( "-" );
          }
        }
        else
        {
          // Do nothing
        }
      
        // Server
        QLayoutItem *srv_item = m_text_layout->itemAtPosition( 2, 1 );
        QLabel *srv_label = static_cast<QLabel *>( srv_item->widget() );
        
        if ( srv_label )
        {
          if ( !host->serverString().isEmpty() )
          {
            srv_label->setText( host->serverString() );
          }
          else
          {
            srv_label->setText( "-" );
          }
        }
        else
        {
          // Do nothing
        }
        
        // Operating system
        QLayoutItem *os_item = m_text_layout->itemAtPosition( 3, 1 );
        QLabel *os_label = static_cast<QLabel *>( os_item->widget() );
        
        if ( os_label )
        {        
          if ( !host->osString().isEmpty() )
          {
            os_label->setText( host->osString() );
          }
          else
          {
            os_label->setText( "-" );
          }
        }
        else
        {
          // Do nothing
        }
        
        // IP address
        QLayoutItem *ip_item = m_text_layout->itemAtPosition( 4, 1 );
        QLabel *ip_label = static_cast<QLabel *>( ip_item->widget() );
        
        if ( ip_label )
        {
          if ( !host->ip().isEmpty() )
          {
            ip_label->setText( host->ip() );
          }
          else
          {
            ip_label->setText( "-" );
          }
        }
        else
        {
          // Do nothing
        }
      
        break;
      }
      case Smb4KBasicNetworkItem::Share:
      {
        Smb4KShare *share = static_cast<Smb4KShare *>( m_item );
        
        // Icon
        QLayoutItem *icon_item = m_tip_layout->itemAt( 0 );
        QLabel *icon_label = static_cast<QLabel *>( icon_item->widget() );
        icon_label->setPixmap( m_item->icon().pixmap( KIconLoader::SizeEnormous ) );

        // Comment
        QLayoutItem *co_item = m_text_layout->itemAtPosition( 1, 1 );
        QLabel *co_label = static_cast<QLabel *>( co_item->widget() );

        if ( co_label )
        {
          if ( !share->comment().isEmpty() )
          {
            co_label->setText( share->comment() );
          }
          else
          {
            co_label->setText( "-" );
          }
        }
        else
        {
          // Do nothing
        }
     
        // Mounted indicator
        QLayoutItem *mnt_item = m_text_layout->itemAtPosition( 2, 1 );
        QLabel *mnt_label = static_cast<QLabel *>( mnt_item->widget() );
        
        if ( mnt_label )
        {
          if ( !share->isPrinter() )
          {
            if ( share->isMounted() )
            {
              mnt_label->setText( i18n( "yes" ) );
            }
            else
            {
              mnt_label->setText( i18n( "no" ) );
            }
          }
          else
          {
            mnt_label->setText( "-" );
          }
        }
        else
        {
          // Do nothing
        }
        
        // The rest won't change while the tool tip is shown.
      
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


void Smb4KToolTip::updateSharesViewToolTip()
{
  if ( m_item && m_text_layout && m_tip_layout )
  {
    Smb4KShare *share = static_cast<Smb4KShare *>( m_item );
    
    // Set the icon
    QLayoutItem *icon_item = m_tip_layout->itemAt( 0 );
    QLabel *icon_label = static_cast<QLabel *>( icon_item->widget() );
    icon_label->setPixmap( m_item->icon().pixmap( KIconLoader::SizeEnormous ) );
    
    QLayoutItem *log_item = m_text_layout->itemAtPosition( 2, 1 );
    QLabel *log_label = static_cast<QLabel *>( log_item->widget() );
    
    switch ( share->fileSystem() )
    {
      case Smb4KShare::CIFS:
      case Smb4KShare::SMBFS:
      {
        if ( !share->login().isEmpty() )
        {
          log_label->setText( share->login() );
        }
        else
        {
          log_label->setText( i18n( "unknown" ) );
        }
        break;
      }
      default:
      {
        log_label->setText( "-" );
        break;
      }
    }
    
    QLayoutItem *s_item = m_text_layout->itemAtPosition( 5, 1 );
    QLabel *s_label = static_cast<QLabel *>( s_item->widget() );
  
    if ( share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0 )
    {
      s_label->setText( i18n( "%1 free of %2 (%3 used)", 
                              share->freeDiskSpaceString(),
                              share->totalDiskSpaceString(),
                              share->diskUsageString() ) );
    }
    else
    {
      s_label->setText( i18n( "unknown" ) );
    }
  }
  else
  {
    // Do nothing
  }
  
  // The rest won't change while the tool tip is shown.
}


void Smb4KToolTip::paintEvent( QPaintEvent *e )
{
  // Copied from Dolphin's meta data tool tips.
  Q_UNUSED( e );

  QPainter painter( this );

  QColor toColor = palette().brush( QPalette::ToolTipBase).color();
  QColor fromColor = KColorScheme::shade(toColor, KColorScheme::LightShade, 0.2);

  const bool haveAlphaChannel = KWindowSystem::compositingActive();
  
  if ( haveAlphaChannel )
  {
    painter.setRenderHint( QPainter::Antialiasing );
    painter.translate( 0.5, 0.5 );
    toColor.setAlpha( 220 );
    fromColor.setAlpha( 220 );
  }
  else
  {
    // Do nothing
  }

  QLinearGradient gradient( QPointF( 0.0, 0.0 ), QPointF( 0.0, height() ) );
  gradient.setColorAt( 0.0, fromColor );
  gradient.setColorAt( 1.0, toColor );
  painter.setPen( Qt::NoPen );
  painter.setBrush( gradient );

  const QRect rect( 0, 0, width(), height() );
    
  if (haveAlphaChannel) 
  {
    const qreal radius = 5.0;

    QPainterPath path;
    path.moveTo( rect.left(), rect.top() + radius );
    arc( path, rect.left() + radius, rect.top() + radius, radius, 180, -90 );
    arc( path, rect.right() - radius, rect.top() + radius, radius, 90, -90 );
    arc( path, rect.right() - radius, rect.bottom() - radius, radius, 0, -90);
    arc( path, rect.left() + radius, rect.bottom() - radius, radius, 270, -90);
    path.closeSubpath();

    painter.drawPath( path );
  } 
  else 
  {
    painter.drawRect( rect );
  }
}


void Smb4KToolTip::arc(QPainterPath& path,
                              qreal cx, qreal cy,
                              qreal radius, qreal angle,
                              qreal sweepLength)
{
  // Copied from Dolphin's meta data tool tips.
  path.arcTo( cx-radius, cy-radius, radius * 2, radius * 2, angle, sweepLength );
}


// void Smb4KToolTip::clear()
// {
//   // Delete all the layout stuff
//   QLayoutItem *child;
//   
//   if ( m_text_layout )
//   {
//     while ( (child = m_text_layout->takeAt( 0 )) != 0 )
//     {
//       delete child;
//     }
//   }
//   
//   if ( m_info_layout )
//   {
//     while ( (child = m_info_layout->takeAt( 0 )) != 0 )
//     {
//       delete child;
//     }
//   }
// 
//   if ( m_tip_layout )
//   {
//     while ( (child = m_tip_layout->takeAt( 0 )) != 0 )
//     {
//       delete child;
//     }
//   }
//   
//   // Delete all children
//   while ( !children().isEmpty() )
//   {
//     delete children().first();
//   }
// 
//   // For the above checks to work, assign the NULL pointer
//   // to the layout objects.
//   m_text_layout = NULL;
//   m_info_layout = NULL;
//   m_tip_layout = NULL; 
// }


//
// SLOTS
//

void Smb4KToolTip::slotHideToolTip()
{
  hide();
}

#include "smb4ktooltip.moc"
