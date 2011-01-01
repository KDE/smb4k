/***************************************************************************
    smb4ksharesiconview  -  This is the shares icon view of Smb4K.
                             -------------------
    begin                : Mo Dez 4 2006
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
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QUrl>
#include <QDrag>

// KDE includes
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kicon.h>

// application specific includes
#include <smb4ksharesiconview.h>
#include <smb4ksharesiconviewitem.h>
#include <../tooltips/smb4ktooltip.h>
#include <core/smb4kshare.h>
#include <core/smb4ksettings.h>
#include <core/smb4kcore.h>


Smb4KSharesIconView::Smb4KSharesIconView( QWidget *parent )
: QListWidget( parent )
{
  setViewMode( IconMode );
  setMouseTracking( true );
  setSelectionMode( ExtendedSelection );
  setResizeMode( Adjust );
  setSortingEnabled( true );
  setWordWrap( true );
  setSpacing( 5 );
  setAcceptDrops( true );
  setDragEnabled( true );
  setDropIndicatorShown( true );

  setContextMenuPolicy( Qt::CustomContextMenu );

  m_tooltip_timer = new QTimer( this );
  m_auto_select_timer = new QTimer( this );
  m_mouse_inside = false;
  m_tooltip = new Smb4KToolTip( this );

  // Connections:
  connect( this, SIGNAL( itemEntered( QListWidgetItem * ) ),
           this, SLOT( slotItemEntered( QListWidgetItem * ) ) );

  connect( this, SIGNAL( viewportEntered() ),
           this, SLOT( slotViewportEntered() ) );

  // We need to conform with KDE's settings (see also slotKDESettingsChanged(),
  // slotItemEntered() and slotViewportEntered()).
  slotKDESettingsChanged( KGlobalSettings::SETTINGS_MOUSE );

  connect( KGlobalSettings::self(), SIGNAL( settingsChanged( int ) ),
           this,                    SLOT( slotKDESettingsChanged( int ) ) );

  connect( m_auto_select_timer,     SIGNAL( timeout() ),
           this,                    SLOT( slotAutoSelectItem() ) );
}


Smb4KSharesIconView::~Smb4KSharesIconView()
{
}


bool Smb4KSharesIconView::event( QEvent *e )
{
  switch ( e->type() )
  {
    case QEvent::ToolTip:
    {
      // Intercept the tool tip event and show our own tool tip.
      QPoint pos = viewport()->mapFromGlobal( cursor().pos() );
      Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( itemAt( pos ) );
      
      if ( item )
      {
        if ( Smb4KSettings::showShareToolTip() )
        {
          if ( !m_tooltip->isVisible() || (m_tooltip->networkItem() && 
               QString::compare( item->shareItem()->key(), m_tooltip->networkItem()->key() ) != 0) )
          {
            m_tooltip->show( item->shareItem(), pos );
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          m_tooltip->hide();
        }
      }
      else
      {
        m_tooltip->hide();
      }

break;
    }
    default:
    {
      break;
    }
  }

  return QListWidget::event( e );
}


void Smb4KSharesIconView::leaveEvent( QEvent *e )
{
  m_tooltip->hide();
  m_auto_select_timer->stop();
  m_mouse_inside = false;
  QListWidget::leaveEvent( e );
}


void Smb4KSharesIconView::enterEvent( QEvent *e )
{
  m_mouse_inside = true;
  QListWidget::enterEvent( e );
}


void Smb4KSharesIconView::mousePressEvent( QMouseEvent *e )
{
  // Hide the current tool tip so that it is not in the way.
  m_tooltip->hide();

  // Get the item that is under the mouse. If there is no
  // item, unselect the current item.
  QListWidgetItem *item = itemAt( e->pos() );

  if ( !item && !selectedItems().isEmpty() )
  {
    clearSelection();
    setCurrentItem( NULL );
    emit itemPressed( currentItem() );
  }
  else
  {
    // Do nothing
  }

  QListWidget::mousePressEvent( e );
}


void Smb4KSharesIconView::focusOutEvent( QFocusEvent *e )
{
  m_auto_select_timer->stop();
  QListWidget::focusOutEvent( e );
}


void Smb4KSharesIconView::wheelEvent( QWheelEvent *e )
{
  m_tooltip->hide();
  QListWidget::wheelEvent( e );
}


void Smb4KSharesIconView::dragEnterEvent( QDragEnterEvent *e )
{
  if ( e->mimeData()->hasUrls() )
  {
    e->accept();
  }
  else
  {
    e->ignore();
  }
}


void Smb4KSharesIconView::dragMoveEvent( QDragMoveEvent *e )
{
  // Let the QAbstractItemView do the highlighting of the item, etc.
  QAbstractItemView::dragMoveEvent( e );

  // Now we do our thing.
  Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( itemAt( e->pos() ) );

  if ( item && (item->flags() & Qt::ItemIsDropEnabled) &&
       (e->proposedAction() & (Qt::CopyAction | Qt::MoveAction)) )
  {
    QUrl url = QUrl::fromLocalFile( item->shareItem()->path() );

    if ( e->source() == this && e->mimeData()->urls().first() == url )
    {
      e->ignore();
    }
    else
    {
      e->accept();
    }
  }
  else
  {
    e->ignore();
  }
}


void Smb4KSharesIconView::dropEvent( QDropEvent *e )
{
  Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( itemAt( e->pos() ) );

  if ( item && (e->proposedAction() & (Qt::CopyAction | Qt::MoveAction)) )
  {
    QUrl url = QUrl::fromLocalFile( item->shareItem()->path() );

    if ( e->source() == this && e->mimeData()->urls().first() == url )
    {
      e->ignore();
    }
    else
    {
      e->acceptProposedAction();

      emit acceptedDropEvent( item, e );
    }
  }
  else
  {
    e->ignore();
  }
}


Qt::DropActions Smb4KSharesIconView::supportedDropActions() const
{
  // Only allow copying and linking.
  return Qt::CopyAction | Qt::LinkAction;
}


QMimeData *Smb4KSharesIconView::mimeData( const QList<QListWidgetItem *> list ) const
{
  QMimeData *mimeData = new QMimeData();
  QList<QUrl> urls;

  for ( int i = 0; i < list.count(); ++i )
  {
    Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( list.at( i ) );

    urls.append( QUrl::fromLocalFile( item->shareItem()->path() ) );
  }

  mimeData->setUrls( urls );

  return mimeData;
}


void Smb4KSharesIconView::startDrag( Qt::DropActions supported )
{
  m_tooltip->hide();

  QList<QListWidgetItem *> list = selectedItems();

  if ( !list.isEmpty() )
  {
    QMimeData *data = mimeData( list );

    if ( !data )
    {
      return;
    }

    QDrag *drag = new QDrag( this );

    QPixmap pixmap;

    if ( list.count() == 1 )
    {
      Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( list.first() );
      pixmap = item->icon().pixmap( KIconLoader::SizeMedium );
    }
    else
    {
      pixmap = KIcon( "document-multiple" ).pixmap( KIconLoader::SizeMedium );
    }

    drag->setPixmap( pixmap );
    drag->setMimeData( data );

    drag->exec( supported, Qt::IgnoreAction );
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSharesIconView::slotItemEntered( QListWidgetItem *item )
{
  // Comply with KDE's settings.
  if ( item && m_use_single_click )
  {
    if ( m_change_cursor_over_icon )
    {
      viewport()->setCursor( QCursor( Qt::PointingHandCursor ) );
    }

    if ( m_auto_select_delay > -1 )
    {
      m_auto_select_item = item;
      m_auto_select_timer->setSingleShot( true );
      m_auto_select_timer->start( m_auto_select_delay );
    }
  }
  else
  {
    // Do nothing
  }

  Smb4KSharesIconViewItem *share_item = static_cast<Smb4KSharesIconViewItem *>( item );

  if ( share_item && m_tooltip->networkItem() &&
       QString::compare( share_item->shareItem()->key(), m_tooltip->networkItem()->key() ) != 0 )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesIconView::slotViewportEntered()
{
  // Comply with KDE's settings.
  if ( m_change_cursor_over_icon )
  {
    viewport()->unsetCursor();
  }
  else
  {
    // Do nothing
  }

  m_auto_select_timer->stop();
  m_auto_select_item = 0;

  // Hide the tool tip.
  m_tooltip->hide();  
}


void Smb4KSharesIconView::slotKDESettingsChanged( int category )
{
  // Adjust to KDE's default mouse settings.
  if ( category != KGlobalSettings::SETTINGS_MOUSE )
  {
    return;
  }

  disconnect( this, SIGNAL( itemClicked( QListWidgetItem * ) ) );
  disconnect( this, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ) );

  m_use_single_click        = KGlobalSettings::singleClick();
  m_change_cursor_over_icon = KGlobalSettings::changeCursorOverIcon();
  m_auto_select_delay       = KGlobalSettings::autoSelectDelay();

  if ( m_use_single_click )
  {
    connect( this, SIGNAL( itemClicked( QListWidgetItem * ) ),
             this, SIGNAL( itemExecuted( QListWidgetItem * ) ) );
  }
  else
  {
    connect( this, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
             this, SIGNAL( itemExecuted( QListWidgetItem * ) ) );
  }

  if ( !m_use_single_click || !m_change_cursor_over_icon )
  {
    viewport()->unsetCursor();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesIconView::slotAutoSelectItem()
{
  // Check that the item is still valid.
  QPoint pos = viewport()->mapFromGlobal( cursor().pos() );
  QListWidgetItem *shareItem = itemAt( pos );

  if ( !m_auto_select_item || !shareItem || m_auto_select_item != shareItem )
  {
    return;
  }
  else
  {
    // Do nothing. We are OK.
  }

  // Give the widget the keyboard focus.
  if ( !hasFocus() )
  {
    setFocus();
  }
  else
  {
    // Do nothing
  }

  // Now set up the auto selection. Most of this has been "stolen" from
  // the KListWidget code.
  Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

  QListWidgetItem *previousItem = currentItem();
  setCurrentItem( m_auto_select_item );

  if ( m_auto_select_item )
  {
    if ( (modifiers & Qt::ShiftModifier) )
    {
      bool block = signalsBlocked();
      blockSignals( true );

      // If no CTRL is pressed, clear before.
      if ( !(modifiers & Qt::ControlModifier ) )
      {
        clearSelection();
      }
      else
      {
        // Do nothing
      }

      bool select = !m_auto_select_item->isSelected();
      bool update = viewport()->updatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = indexFromItem( previousItem ).row() < indexFromItem( m_auto_select_item ).row();

      QListWidgetItem *indexItem = down ? previousItem : m_auto_select_item;

      for ( int i = row( indexItem ); i < count(); ++i )
      {
        if ( down && item( i ) == m_auto_select_item )
        {
          m_auto_select_item->setSelected( select );

          break;
        }
        else
        {
          // Do nothing
        }

        if ( !down && item( i ) == previousItem )
        {
          previousItem->setSelected( select );
          break;
        }
        else
        {
          // Do nothing
        }

        indexItem->setSelected( select );
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );

      emit itemSelectionChanged();

      if ( selectionMode() == QListWidget::SingleSelection )
      {
        emit itemSelectionChanged();
      }
      else
      {
        // Do nothing
      }
    }
    else if ( (modifiers & Qt::ControlModifier) )
    {
      m_auto_select_item->setSelected( !m_auto_select_item->isSelected() );
    }
    else
    {
      bool block = signalsBlocked();
      blockSignals( true );

      if ( !m_auto_select_item->isSelected() )
      {
        clearSelection();
      }
      else
      {
        // Do nothing
      }

      blockSignals( block );

      m_auto_select_item->setSelected( true );
    }
  }
  else
  {
    // Do nothing. This should never happen, however.
  }
}

#include "smb4ksharesiconview.moc"
