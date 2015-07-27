/***************************************************************************
    smb4ksharesiconview  -  This is the shares icon view of Smb4K.
                             -------------------
    begin                : Mo Dez 4 2006
    copyright            : (C) 2006-2015 by Alexander Reinholdt
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
#include "smb4ksharesiconview.h"
#include "smb4ksharesiconviewitem.h"
#include "../tooltips/smb4ktooltip.h"
#include "core/smb4kshare.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QDrag>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QApplication>

// KDE includes
#include <KIconThemes/KIconLoader>


Smb4KSharesIconView::Smb4KSharesIconView(QWidget *parent)
: QListWidget(parent)
{
  setViewMode(IconMode);
  setMouseTracking(true);
  setSelectionMode(ExtendedSelection);
  setResizeMode(Adjust);
  setSortingEnabled(true);
  setWordWrap(true);
  setSpacing(5);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  
  setContextMenuPolicy(Qt::CustomContextMenu);

  m_tooltip_item = 0;
//   m_auto_select_timer = new QTimer(this);
  m_mouse_inside = false;

  // Connections:
  connect(this, SIGNAL(itemEntered(QListWidgetItem*)),
          this, SLOT(slotItemEntered(QListWidgetItem*)));

  connect(this, SIGNAL(viewportEntered()),
          this, SLOT(slotViewportEntered()));
}


Smb4KSharesIconView::~Smb4KSharesIconView()
{
}


bool Smb4KSharesIconView::event(QEvent *e)
{
  switch (e->type())
  {
    case QEvent::ToolTip:
    {
      // Intercept the tool tip event and show our own tool tip.
      QPoint pos = viewport()->mapFromGlobal(cursor().pos());
      Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(itemAt(pos));
      
      if (item)
      {
        if (Smb4KSettings::showShareToolTip())
        {
          m_tooltip_item = item;
          emit aboutToShowToolTip(m_tooltip_item);
          m_tooltip_item->tooltip()->show(cursor().pos());
        }
        else
        {
          if (m_tooltip_item)
          {
            emit aboutToHideToolTip(m_tooltip_item);
            m_tooltip_item->tooltip()->hide();
            m_tooltip_item = 0;
          }
          else
          {
            // Do nothing
          }
        }
      }
      else
      {
        if (m_tooltip_item)
        {
          emit aboutToHideToolTip(m_tooltip_item);
          m_tooltip_item->tooltip()->hide();
          m_tooltip_item = 0;
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

  return QListWidget::event(e);
}


void Smb4KSharesIconView::leaveEvent(QEvent *e)
{
  if (m_tooltip_item)
  {
    emit aboutToHideToolTip(m_tooltip_item);
    m_tooltip_item->tooltip()->hide();
    m_tooltip_item = 0;
  }
  else
  {
    // Do nothing
  }
  
  m_mouse_inside = false;
  QListWidget::leaveEvent(e);
}


void Smb4KSharesIconView::enterEvent(QEvent *e)
{
  m_mouse_inside = true;
  QListWidget::enterEvent(e);
}


void Smb4KSharesIconView::mousePressEvent(QMouseEvent *e)
{
  // Hide the current tool tip so that it is not in the way.
  if (m_tooltip_item)
  {
    emit aboutToHideToolTip(m_tooltip_item);
    m_tooltip_item->tooltip()->hide();
    m_tooltip_item = 0;
  }
  else
  {
    // Do nothing
  }

  // Get the item that is under the mouse. If there is no
  // item, unselect the current item.
  QListWidgetItem *item = itemAt(e->pos());

  if (!item && !selectedItems().isEmpty())
  {
    clearSelection();
    setCurrentItem(0);
    emit itemPressed(currentItem());
  }
  else
  {
    // Do nothing
  }

  QListWidget::mousePressEvent(e);
}


void Smb4KSharesIconView::focusOutEvent(QFocusEvent *e)
{
  QListWidget::focusOutEvent(e);
}


void Smb4KSharesIconView::wheelEvent(QWheelEvent *e)
{
  if (m_tooltip_item)
  {
    emit aboutToHideToolTip(m_tooltip_item);
    m_tooltip_item->tooltip()->hide();
    m_tooltip_item = 0;
  }
  else
  {
    // Do nothing
  }
  
  QListWidget::wheelEvent(e);
}


void Smb4KSharesIconView::dragEnterEvent(QDragEnterEvent *e)
{
  if (e->mimeData()->hasUrls())
  {
    e->accept();
  }
  else
  {
    e->ignore();
  }
}


void Smb4KSharesIconView::dragMoveEvent(QDragMoveEvent *e)
{
  // Let the QAbstractItemView do the highlighting of the item, etc.
  QAbstractItemView::dragMoveEvent(e);

  // Now we do our thing.
  Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(itemAt(e->pos()));

  if (item && (item->flags() & Qt::ItemIsDropEnabled) &&
       (e->proposedAction() & (Qt::CopyAction | Qt::MoveAction)))
  {
    QUrl url(item->shareItem()->path());

    if (e->source() == this && e->mimeData()->urls().first() == url)
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


void Smb4KSharesIconView::dropEvent(QDropEvent *e)
{
  Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(itemAt(e->pos()));

  if (item && (e->proposedAction() & (Qt::CopyAction | Qt::MoveAction)))
  {
    QUrl url(item->shareItem()->path());

    if (e->source() == this && e->mimeData()->urls().first() == url)
    {
      e->ignore();
    }
    else
    {
      e->acceptProposedAction();

      emit acceptedDropEvent(item, e);
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


QMimeData *Smb4KSharesIconView::mimeData(const QList<QListWidgetItem *> list) const
{
  QMimeData *mimeData = new QMimeData();
  QList<QUrl> urls;

  for (int i = 0; i < list.count(); ++i)
  {
    Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(list.at(i));
    urls.append(QUrl(item->shareItem()->path()));
  }

  mimeData->setUrls(urls);

  return mimeData;
}


void Smb4KSharesIconView::startDrag(Qt::DropActions supported)
{
  if (m_tooltip_item)
  {
    emit aboutToHideToolTip(m_tooltip_item);
    m_tooltip_item->tooltip()->hide();
    m_tooltip_item = 0;
  }
  else
  {
    // Do nothing
  }

  QList<QListWidgetItem *> list = selectedItems();

  if (!list.isEmpty())
  {
    QMimeData *data = mimeData(list);

    if (!data)
    {
      return;
    }

    QDrag *drag = new QDrag(this);

    QPixmap pixmap;

    if (list.count() == 1)
    {
      Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(list.first());
      pixmap = item->icon().pixmap(KIconLoader::SizeMedium);
    }
    else
    {
      pixmap = KDE::icon("document-multiple").pixmap(KIconLoader::SizeMedium);
    }

    drag->setPixmap(pixmap);
    drag->setMimeData(data);

    drag->exec(supported, Qt::IgnoreAction);
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSharesIconView::slotItemEntered(QListWidgetItem *item)
{
  Smb4KSharesIconViewItem *share_item = static_cast<Smb4KSharesIconViewItem *>(item);
  
  if (m_tooltip_item && m_tooltip_item != share_item)
  {
    emit aboutToHideToolTip(m_tooltip_item);
    m_tooltip_item->tooltip()->hide();
    m_tooltip_item = 0;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesIconView::slotViewportEntered()
{
  // Hide the tool tip.
  if (m_tooltip_item)
  {
    emit aboutToHideToolTip(m_tooltip_item);
    m_tooltip_item->tooltip()->hide();
    m_tooltip_item = 0;
  }
  else
  {
    // Do nothing
  }
}

