/***************************************************************************
    smb4kshareslistview  -  This is the shares list view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
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
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kshareslistview.h"
#include "smb4kshareslistviewitem.h"
#include "../tooltips/smb4ktooltip.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QHeaderView>
#include <QDrag>

// KDE includes
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kicon.h>


Smb4KSharesListView::Smb4KSharesListView( QWidget *parent )
: QTreeWidget( parent )
{
  setAllColumnsShowFocus( false );
  setMouseTracking( true );
  setRootIsDecorated( false );
  setSelectionMode( ExtendedSelection );
  setAcceptDrops( true );
  setDragEnabled( true );
  setDropIndicatorShown( true );

  setContextMenuPolicy( Qt::CustomContextMenu );

  m_auto_select_timer = new QTimer( this );
  m_mouse_inside = false;
  m_tooltip = new Smb4KToolTip( this );

  QStringList header_labels;
  header_labels.append( i18n( "Item" ) );
#ifndef Q_OS_FREEBSD
  header_labels.append( i18n( "Login" ) );
#endif
  header_labels.append( i18n( "File System" ) );
  header_labels.append( i18n( "Owner" ) );
  header_labels.append( i18n( "Free" ) );
  header_labels.append( i18n( "Used" ) );
  header_labels.append( i18n( "Total" ) );
  header_labels.append( i18n( "Usage" ) );
  setHeaderLabels( header_labels );

  header()->setStretchLastSection( false );
  header()->setResizeMode( QHeaderView::ResizeToContents );
  header()->setResizeMode( Item, QHeaderView::Stretch );

  // Connections:
  connect( this, SIGNAL( itemEntered( QTreeWidgetItem *, int ) ),
           this, SLOT( slotItemEntered( QTreeWidgetItem *, int ) ) );

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


Smb4KSharesListView::~Smb4KSharesListView()
{
}


bool Smb4KSharesListView::event( QEvent *e )
{
  switch ( e->type() )
  {
    case QEvent::ToolTip:
    {
      // Intercept the tool tip event and show our own tool tip.
      QPoint pos = viewport()->mapFromGlobal( cursor().pos() );
      Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( itemAt( pos ) );
      
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
          if ( m_tooltip->isVisible() )
          {
            m_tooltip->hide();
          }
          else
          {
            // Do nothing
          }
        }
      }
      else
      {
        if ( m_tooltip->isVisible() )
        {
          m_tooltip->hide();
        }
        else
        {
          // Do nothing
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }

  return QTreeWidget::event( e );
}


void Smb4KSharesListView::mouseMoveEvent( QMouseEvent *e )
{
  QPoint pos = viewport()->mapFromGlobal( cursor().pos() );

  // Find the item over which the user moved the mouse:
  Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( itemAt( pos ) );

  if ( item )
  {
    emit itemEntered( item, columnAt( pos.x() ) );
  }
  else
  {
    // Do nothing
  }

  QTreeWidget::mouseMoveEvent( e );
}


void Smb4KSharesListView::leaveEvent( QEvent *e )
{
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }
  
  m_auto_select_timer->stop();
  m_mouse_inside = false;
  QTreeWidget::leaveEvent( e );
}


void Smb4KSharesListView::enterEvent( QEvent *e )
{
  m_mouse_inside = true;
  QTreeWidget::enterEvent( e );
}


void Smb4KSharesListView::mousePressEvent( QMouseEvent *e )
{
  // Hide the current tool tip so that it is not in the way.
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }

  // Get the item that is under the mouse. If there is no
  // item, unselect the current item.
  QTreeWidgetItem *item = itemAt( e->pos() );

  if ( !item && !selectedItems().isEmpty() )
  {
    clearSelection();
    setCurrentItem( NULL );
    emit itemPressed( currentItem(), -1 );
  }
  else
  {
    // Do nothing
  }

  QTreeWidget::mousePressEvent( e );
}


void Smb4KSharesListView::focusOutEvent( QFocusEvent *e )
{
  m_auto_select_timer->stop();
  QTreeWidget::focusOutEvent( e );
}


void Smb4KSharesListView::wheelEvent( QWheelEvent *e )
{
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }
  
  QTreeWidget::wheelEvent( e );
}


void Smb4KSharesListView::dragEnterEvent( QDragEnterEvent *e )
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


void Smb4KSharesListView::dragMoveEvent( QDragMoveEvent *e )
{
  // Let the QAbstractItemView do the highlighting of the item, etc.
  QAbstractItemView::dragMoveEvent( e );

  // Now we do our thing.
  Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( itemAt( e->pos() ) );

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


void Smb4KSharesListView::dropEvent( QDropEvent *e )
{
  Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( itemAt( e->pos() ) );

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


Qt::DropActions Smb4KSharesListView::supportedDropActions() const
{
  // Only allow copying and linking.
  return Qt::CopyAction | Qt::LinkAction;
}


QMimeData *Smb4KSharesListView::mimeData( const QList<QTreeWidgetItem *> list ) const
{
  QMimeData *mimeData = new QMimeData();
  QList<QUrl> urls;

  for ( int i = 0; i < list.count(); ++i )
  {
    Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( list.at( i ) );

    urls.append( QUrl::fromLocalFile( item->shareItem()->path() ) );
  }

  mimeData->setUrls( urls );

  return mimeData;
}


void Smb4KSharesListView::startDrag( Qt::DropActions supported )
{
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }

  QList<QTreeWidgetItem *> list = selectedItems();

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
      Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( list.first() );
      pixmap = item->shareItem()->icon().pixmap( KIconLoader::SizeMedium );
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

void Smb4KSharesListView::slotItemEntered( QTreeWidgetItem *item, int /*column*/ )
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
  
  if ( m_tooltip->isVisible() )
  {
    Smb4KSharesListViewItem *share_item = static_cast<Smb4KSharesListViewItem *>( item );
      
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
  else
  {
    // Do nothing
  }
}


void Smb4KSharesListView::slotViewportEntered()
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

  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesListView::slotKDESettingsChanged( int category )
{
  // Adjust to KDE's default mouse settings.
  if ( category != KGlobalSettings::SETTINGS_MOUSE )
  {
    return;
  }

  disconnect( this, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ) );
  disconnect( this, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ) );

  m_use_single_click        = KGlobalSettings::singleClick();
  m_change_cursor_over_icon = KGlobalSettings::changeCursorOverIcon();
  m_auto_select_delay       = KGlobalSettings::autoSelectDelay();

  if ( m_use_single_click )
  {
    connect( this, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
             this, SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ) );
  }
  else
  {
    connect( this, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ) );
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


void Smb4KSharesListView::slotAutoSelectItem()
{
  // Check that the item is still valid.
  QPoint pos = viewport()->mapFromGlobal( cursor().pos() );
  QTreeWidgetItem *item = itemAt( pos );

  if ( !m_auto_select_item || !item || m_auto_select_item != item )
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

  QTreeWidgetItem *previousItem = currentItem();
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

      QTreeWidgetItem *indexItem = down ? previousItem : m_auto_select_item;

      QTreeWidgetItemIterator it( indexItem );

      while ( *it )
      {
        if ( down && *it == m_auto_select_item )
        {
          m_auto_select_item->setSelected( select );

          break;
        }
        else
        {
          // Do nothing
        }

        if ( !down && *it == previousItem )
        {
          previousItem->setSelected( select );
          break;
        }
        else
        {
          // Do nothing
        }

        indexItem->setSelected( select );

        ++it;
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );

      emit itemSelectionChanged();

      if ( selectionMode() == QTreeWidget::SingleSelection )
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

#include "smb4kshareslistview.moc"
