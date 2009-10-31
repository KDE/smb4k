/***************************************************************************
    smb4ksharesviewtooltip  -  Tool tip for the shares view.
                             -------------------
    begin                : So Jul 8 2007
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
#include <kdebug.h>
#include <kiconloader.h>

// application specific includes
#include <smb4ksharesviewtooltip.h>
#include <smb4kshareslistviewitem.h>


static bool first_time = true;

Smb4KSharesViewToolTip::Smb4KSharesViewToolTip( QWidget *parent )
: QLabel( parent, Qt::ToolTip ), m_data( NULL ), m_cleared( true )
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

  m_unc_label = NULL;
  m_unc = NULL;
  m_mount_point_label = NULL;
  m_mount_point = NULL;
  m_owner_label = NULL;
  m_owner = NULL;
  m_login_label = NULL;
  m_login = NULL;
  m_file_system_label = NULL;
  m_file_system = NULL;
  m_line = NULL;
  m_free_label = NULL;
  m_free = NULL;
  m_used_label = NULL;
  m_used = NULL;
  m_total_label = NULL;
  m_total = NULL;
  m_usage_label = NULL;
  m_usage = NULL;
  m_icon = NULL;
  m_inaccessible = NULL;
}


Smb4KSharesViewToolTip::~Smb4KSharesViewToolTip()
{
  // Never touch the Smb4KSharesListViewItem object here!
}


void Smb4KSharesViewToolTip::setupToolTip( Smb4KSharesViewItemData *data )
{
  if ( data )
  {
    if ( !m_cleared )
    {
      clearToolTip();
    }
    else
    {
      // Do nothing
    }

    m_data = data;

    m_layout = new QGridLayout( this );
    m_layout->setSpacing( 5 );

    // UNC
    m_unc_label         = new QLabel( i18n( "Share:" ), this );
    m_unc               = new QLabel( m_data->share()->unc(), this );

    m_layout->addWidget( m_unc_label, 0, 1, 0 );
    m_layout->addWidget( m_unc, 0, 2, 0 );

    // Mount point
    m_mount_point_label = new QLabel( i18n( "Mount point:" ), this );
    m_mount_point       = new QLabel( m_data->share()->path(), this );

    m_layout->addWidget( m_mount_point_label, 1, 1, 0 );
    m_layout->addWidget( m_mount_point, 1, 2, 0 );

    // Owner and group
    m_owner_label       = new QLabel( i18n( "Owner:" ), this );
    m_owner             = new QLabel( m_data->share()->owner()+" - "+m_data->share()->group(), this );

    m_layout->addWidget( m_owner_label, 2, 1, 0 );
    m_layout->addWidget( m_owner, 2, 2, 0 );

    // CIFS login
    m_login_label       = new QLabel( i18n( "Login:" ), this );
#ifndef __FreeBSD__
    m_login             = new QLabel( (m_data->share()->fileSystem() == Smb4KShare::CIFS) ?
                          m_data->share()->login() : "-", this );
#else
    m_login             = new QLabel( m_data->share()->login() );
#endif

    m_layout->addWidget( m_login_label, 3, 1, 0 );
    m_layout->addWidget( m_login, 3, 2, 0 );

    // File system
    m_file_system_label = new QLabel( i18n( "File system:" ), this );
    m_file_system       = new QLabel( m_data->share()->fileSystemString().toUpper(), this );

    m_layout->addWidget( m_file_system_label, 4, 1, 0 );
    m_layout->addWidget( m_file_system, 4, 2, 0 );

    // Line
    m_line = new QFrame( this );
    m_line->setLineWidth( 1 );
    m_line->setMidLineWidth( 0 );
    m_line->setFixedWidth( 100 );
    m_line->setFrameShape( QFrame::HLine );
    m_line->setFrameShadow( QFrame::Plain );

    m_layout->addWidget( m_line, 5, 1, 1, 2, Qt::AlignCenter );

    if ( !m_data->share()->isInaccessible() )
    {
      m_free_label        = new QLabel( i18n( "Free:" ), this );
      m_free              = new QLabel( m_data->share()->freeDiskSpaceString(), this );

      m_layout->addWidget( m_free_label, 6, 1, 0 );
      m_layout->addWidget( m_free, 6, 2, 0 );

      m_used_label        = new QLabel( i18n( "Used:" ), this );
      m_used              = new QLabel( m_data->share()->usedDiskSpaceString(), this );

      m_layout->addWidget( m_used_label, 7, 1, 0 );
      m_layout->addWidget( m_used, 7, 2, 0 );

      m_total_label       = new QLabel( i18n( "Total:" ), this );
      m_total             = new QLabel( m_data->share()->totalDiskSpaceString(), this );

      m_layout->addWidget( m_total_label, 8, 1, 0 );
      m_layout->addWidget( m_total, 8, 2, 0 );

      // The disk usage
      m_usage_label       = new QLabel( i18n( "Usage:" ), this );
      m_usage             = new QLabel( m_data->share()->diskUsageString(), this );

      m_layout->addWidget( m_usage_label, 9, 1, 0 );
      m_layout->addWidget( m_usage, 9, 2, 0 );
    }
    else
    {
      m_inaccessible      = new QLabel( i18n( "This share is inaccessible." ), this );

      QFont font;
      font.setItalic( true );

      m_inaccessible->setFont( font );

      m_layout->addWidget( m_inaccessible, 6, 1, 1, 2, Qt::AlignCenter );
    }

    // Pixmap
    m_icon                = new QLabel( this );
    m_icon->setPixmap( m_data->pixmap( KIconLoader::global()->currentSize( KIconLoader::Desktop ) ) );

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


void Smb4KSharesViewToolTip::clearToolTip()
{
  if ( m_data )
  {
    delete m_unc_label;
    m_unc_label = NULL;

    delete m_unc;
    m_unc = NULL;

    delete m_mount_point_label;
    m_mount_point_label = NULL;

    delete m_mount_point;
    m_mount_point = NULL;

    delete m_owner_label;
    m_owner_label = NULL;

    delete m_owner;
    m_owner = NULL;

    delete m_login_label;
    m_login_label = NULL;

    delete m_login;
    m_login = NULL;

    delete m_file_system_label;
    m_file_system_label = NULL;

    delete m_file_system;
    m_file_system = NULL;

    delete m_line;
    m_line = NULL;

    delete m_free_label;
    m_free_label = NULL;

    delete m_free;
    m_free = NULL;

    delete m_used_label;
    m_used_label = NULL;

    delete m_used;
    m_used = NULL;

    delete m_total_label;
    m_total_label = NULL;

    delete m_total;
    m_total = NULL;

    delete m_usage_label;
    m_usage_label = NULL;

    delete m_usage;
    m_usage = NULL;

    delete m_icon;
    m_icon = NULL;

    delete m_inaccessible;
    m_inaccessible = NULL;

    m_data = NULL;

    delete m_layout;
    m_layout = NULL;
  }
  else
  {
    // Do nothing
  }

  m_cleared = true;
}


void Smb4KSharesViewToolTip::update()
{
  if ( !isVisible() || isCleared() )
  {
    return;
  }

  if ( !m_data->share()->isInaccessible() )
  {
    m_free->setText( m_data->share()->freeDiskSpaceString() );
    m_used->setText( m_data->share()->usedDiskSpaceString() );
    m_total->setText( m_data->share()->totalDiskSpaceString() );
    m_usage->setText( m_data->share()->diskUsageString() );
  }
  else
  {
    delete m_free_label;
    m_free_label = NULL;

    delete m_free;
    m_free = NULL;

    delete m_used_label;
    m_used_label = NULL;

    delete m_used;
    m_used = NULL;

    delete m_total_label;
    m_total_label = NULL;

    delete m_total;
    m_total = NULL;

    delete m_usage_label;
    m_usage_label = NULL;

    delete m_usage;
    m_usage = NULL;

    if ( !m_inaccessible )
    {
      m_inaccessible      = new QLabel( i18n( "This share is inaccessible." ), this );

      QFont font;
      font.setItalic( true );

      m_inaccessible->setFont( font );

      m_layout->addWidget( m_inaccessible, 6, 1, 1, 2, Qt::AlignCenter );
    }
    else
    {
      // Do nothing
    }

    delete m_icon;

    m_icon                = new QLabel( this );
    m_icon->setPixmap( m_data->pixmap( KIconLoader::global()->currentSize( KIconLoader::Desktop ) ) );

    m_layout->addWidget( m_icon, 0, 0, m_layout->rowCount(), 1, Qt::AlignCenter );

    m_layout->activate();
    adjustSize();
  }
}


void Smb4KSharesViewToolTip::mousePressEvent( QMouseEvent *e )
{
  setVisible( false );
  QLabel::mousePressEvent( e );
}


void Smb4KSharesViewToolTip::leaveEvent( QEvent *e )
{
  setVisible( false );
  QLabel::leaveEvent( e );
}


void Smb4KSharesViewToolTip::showEvent( QShowEvent *e )
{
  if ( first_time )
  {
    first_time = false;
  }
  else
  {
    // Do nothing
  }

  emit aboutToShow( m_data );
  QLabel::showEvent( e );
}


void Smb4KSharesViewToolTip::hideEvent( QHideEvent *e )
{
  emit aboutToHide();
  QLabel::hideEvent( e );
}


#include "smb4ksharesviewtooltip.moc"
