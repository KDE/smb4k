/***************************************************************************
    smb4ktooltip  -  Provides tooltips for Smb4K
                             -------------------
    begin                : Sa Dez 23 2010
    copyright            : (C) 2010 by Alexander Reinholdt
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

// application specific includes
#include <core/smb4kbasicnetworkitem.h>
#include <core/smb4kworkgroup.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>
#include "smb4ktooltip.h"

Smb4KToolTip::Smb4KToolTip( QWidget* parent )
: QWidget( parent, Qt::ToolTip|Qt::BypassGraphicsProxyWidget|Qt::FramelessWindowHint ),
  m_item( NULL ), m_tip_layout( NULL ), m_info_layout( NULL ), m_text_layout( NULL )
{
  setAttribute( Qt::WA_TranslucentBackground );
  
  // Copied from QToolTip
  setForegroundRole( QPalette::ToolTipText );
  setBackgroundRole( QPalette::ToolTipBase );
  setPalette( QToolTip::palette() );
  ensurePolished();
  setWindowOpacity( style()->styleHint( QStyle::SH_ToolTipLabel_Opacity, 0, this ) / 255.0 );
  
  // Check which of the Smb4K GUIs we have got as parent.
  if ( qstrcmp( parent->metaObject()->className(), "Smb4KNetworkBrowser" ) == 0 )
  {
    m_parent = NetworkBrowser;
  }
  else if ( qstrcmp( parent->metaObject()->className(), "Smb4KSharesIconView" ) == 0 )
  {
    m_parent = SharesView;
  }
  else if ( qstrcmp( parent->metaObject()->className(), "Smb4KSharesListView" ) == 0 )
  {
    m_parent = SharesView;
  }
  else
  {
    qDebug() << "UnknownParent";
    m_parent = UnknownParent;
  }
} 


Smb4KToolTip::~Smb4KToolTip()
{
  // Never delete m_item here. We only have a pointer to 
  // somewhere outside of this class.
}

 
void Smb4KToolTip::show( Smb4KBasicNetworkItem *item, const QPoint &pos )
{
  // Let the internal Smb4KBasicNetworkItem object point to the
  // one for that we will show information.
  m_item = item;
  
  // Ensure that the tooltip is setup correctly.
  if ( isVisible() )
  {
    hide();
  }
  else
  {
    // Do nothing
  }
  
  // Setup the tooltip.
  switch ( m_parent )
  {
    case NetworkBrowser:
    {
      setupNetworkBrowserToolTip( item );
      break;
    }
    case SharesView:
    {
      break;
    }
    default:
    {
      return;
    }
  }
  
  // Emit the aboutToShow() signal.
  emit aboutToShow( m_item );
  
  // Show the tooltip.
  QPoint p = static_cast<QAbstractScrollArea *>( parentWidget() )->viewport()->mapToGlobal( pos );

  QDesktopWidget *d = QApplication::desktop();

  if ( p.x() + width() > d->width() )
  {
    p.setX( p.x() - width() - 5 );
  }
  else
  {
    p.setX( p.x() + 5 );
  }

  if ( p.y() + height() > d->height() )
  {
    p.setY( p.y() - height() - 5 );
  }
  else
  {
    p.setY( p.y() + 5 );
  }

  move( p );
  setVisible( true );
  
  QTimer::singleShot( 10000, this, SLOT( slotHideToolTip() ) );
}


void Smb4KToolTip::hide()
{
  slotHideToolTip();
}


void Smb4KToolTip::setupNetworkBrowserToolTip( Smb4KBasicNetworkItem* item )
{
  // NOTE: If you change the layout here, adjust also the update function!
  
  m_tip_layout = new QHBoxLayout( this );
  m_tip_layout->setAlignment( Qt::AlignTop );
  m_info_layout = new QVBoxLayout();
  m_info_layout->setAlignment( Qt::AlignTop );

  // Set the icon
  QLabel *icon_label = new QLabel( this );
  icon_label->setPixmap( item->icon().pixmap( KIconLoader::SizeEnormous ) );
  
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
  
  switch ( item->type() )
  {
    case Smb4KBasicNetworkItem::Workgroup:
    {
      Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( item );
      
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
        m_text_layout->addWidget( new QLabel( workgroup->masterBrowserName()
                                  +" ("+workgroup->masterBrowserIP()+")", this ), 1, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( workgroup->masterBrowserName(), this ), 1, 1, 0 );
      }
      
      m_info_layout->addLayout( m_text_layout );
      m_info_layout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
      break;
    }
    case Smb4KBasicNetworkItem::Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( item );
      
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
        m_text_layout->addWidget( new QLabel( host->comment(), this ), 1, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( "-", this ), 1, 1, 0 );
      }
      
      QLabel *srv_label = new QLabel( i18n( "Server" ), this );
      srv_label->setPalette( p );
      
      m_text_layout->addWidget( srv_label, 2, 0, Qt::AlignRight );
      
      if ( !host->serverString().isEmpty() )
      {
        m_text_layout->addWidget( new QLabel( host->serverString(), this ), 2, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( "-", this ), 2, 1, 0 );
      }
      
      QLabel *os_label = new QLabel( i18n( "Operating system" ), this );
      os_label->setPalette( p );
      
      m_text_layout->addWidget( os_label, 3, 0, Qt::AlignRight );
      
      if ( !host->osString().isEmpty() )
      {
        m_text_layout->addWidget( new QLabel( host->osString(), this ), 3, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( "-", this ), 3, 1, 0 );
      }
      
      QLabel *ip_label = new QLabel( i18n( "IP Address" ), this );
      ip_label->setPalette( p );
      
      m_text_layout->addWidget( ip_label, 4, 0, Qt::AlignRight );
      
      if ( !host->ip().isEmpty() )
      {
        m_text_layout->addWidget( new QLabel( host->ip(), this ), 4, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( "-", this ), 4, 1, 0 );
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
      Smb4KShare *share = static_cast<Smb4KShare *>( item );
      
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
      m_text_layout->addWidget( new QLabel( i18n( "Share (%1)" ).arg( share->translatedTypeString() ), this ), 0, 1, 0 );
      
      QLabel *co_label = new QLabel( i18n( "Comment" ), this );
      co_label->setPalette( p );
      
      m_text_layout->addWidget( co_label, 1, 0, Qt::AlignRight );
      
      if ( !share->comment().isEmpty() )
      {
        m_text_layout->addWidget( new QLabel( share->comment(), this ), 1, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( "-", this ), 1, 1, 0 );
      }
     
      QLabel *mnt_label = new QLabel( i18n( "Mounted" ), this );
      mnt_label->setPalette( p );
      
      m_text_layout->addWidget( mnt_label, 2, 0, Qt::AlignRight );
      
      if ( !share->isPrinter() )
      {
        if ( share->isMounted() )
        {
          m_text_layout->addWidget( new QLabel( i18n( "yes" ), this ), 2, 1, 0 );
        }
        else
        {
          m_text_layout->addWidget( new QLabel( i18n( "no" ), this ), 2, 1, 0 );
        }
      }
      else
      {
        m_text_layout->addWidget( new QLabel( "-", this ), 2, 1, 0 );
      }
      
      QLabel *ip_label = new QLabel( i18n( "IP Address" ), this );
      ip_label->setPalette( p );
      
      m_text_layout->addWidget( ip_label, 3, 0, Qt::AlignRight );
      
      if ( !share->hostIP().isEmpty() )
      {
        m_text_layout->addWidget( new QLabel( share->hostIP(), this ), 3, 1, 0 );
      }
      else
      {
        m_text_layout->addWidget( new QLabel( "-", this ), 3, 1, 0 );
      }
      
      QLabel *h_label = new QLabel( i18n( "Host" ), this );
      h_label->setPalette( p );
      
      m_text_layout->addWidget( h_label, 4, 0, Qt::AlignRight );
      m_text_layout->addWidget( new QLabel( share->hostName() ), 4, 1, 0 );

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


void Smb4KToolTip::update()
{
  switch ( m_parent )
  {
    case NetworkBrowser:
    {
      updateNetworkBrowserToolTip();
      break;
    }
    case SharesView:
    {
      // FIXME
      break;
    }
    default:
    {
      break;
    }
  }
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
          if ( !workgroup->masterBrowserIP().isEmpty() )
          {
            mb_label->setText( workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIP()+")" );
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


void Smb4KToolTip::paintEvent( QPaintEvent *e )
{
  // Copied from QToolTip
  QStylePainter p( this );
  QStyleOptionFrame opt;
  opt.init( this );
  p.drawPrimitive( QStyle::PE_PanelTipLabel, opt );
  p.end();
    
  QWidget::paintEvent(e);
}


//
// SLOTS
//

void Smb4KToolTip::slotHideToolTip()
{
  emit aboutToHide( m_item );
  
  setVisible( false );
  
  // Delete all the layout stuff
  QLayoutItem *child;
  
  if ( m_text_layout )
  {
    while ( (child = m_text_layout->takeAt( 0 )) != 0 )
    {
      delete child;
    }
  }
  
  if ( m_info_layout )
  {
    while ( (child = m_info_layout->takeAt( 0 )) != 0 )
    {
      delete child;
    }
  }

  if ( m_tip_layout )
  {
    while ( (child = m_tip_layout->takeAt( 0 )) != 0 )
    {
      delete child;
    }
  }
  
  // Delete all children
  while ( !children().isEmpty() )
  {
    delete children().first();
  }

  // For the above checks to work, assign the NULL pointer
  // to the layout objects.
  m_text_layout = NULL;
  m_info_layout = NULL;
  m_tip_layout = NULL;
}


#include "smb4ktooltip.moc"
