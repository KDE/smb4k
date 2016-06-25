/***************************************************************************
    smb4ksharesiconview  -  This is the shares icon view of Smb4K.
                             -------------------
    begin                : Mo Dez 4 2006
    copyright            : (C) 2006-2016 by Alexander Reinholdt
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
#include "smb4ksharesview.h"
#include "smb4ksharesviewitem.h"
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


Smb4KSharesView::Smb4KSharesView(QWidget *parent)
: QListWidget(parent)
{
  setMouseTracking(true);
  setSelectionMode(ExtendedSelection);
  setResizeMode(Adjust);
  setSortingEnabled(true);
  setWordWrap(true);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setUniformItemSizes(true);
  setWrapping(true);
  
  setContextMenuPolicy(Qt::CustomContextMenu);

  m_tooltipItem = 0;
  m_mouseInside = false;

  // Connections:
  connect(this, SIGNAL(itemEntered(QListWidgetItem*)),
          this, SLOT(slotItemEntered(QListWidgetItem*)));

  connect(this, SIGNAL(viewportEntered()),
          this, SLOT(slotViewportEntered()));
}


Smb4KSharesView::~Smb4KSharesView()
{
}


bool Smb4KSharesView::event(QEvent *e)
{
  switch (e->type())
  {
    case QEvent::ToolTip:
    {
      // Intercept the tool tip event and show our own tool tip.
      QPoint pos = viewport()->mapFromGlobal(cursor().pos());
      Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(itemAt(pos));
      
      if (item)
      {
        if (Smb4KSettings::showShareToolTip())
        {
          m_tooltipItem = item;
          emit aboutToShowToolTip(m_tooltipItem);
          m_tooltipItem->tooltip()->show(cursor().pos());
        }
        else
        {
          if (m_tooltipItem)
          {
            emit aboutToHideToolTip(m_tooltipItem);
            m_tooltipItem->tooltip()->hide();
            m_tooltipItem = 0;
          }
          else
          {
            // Do nothing
          }
        }
      }
      else
      {
        if (m_tooltipItem)
        {
          emit aboutToHideToolTip(m_tooltipItem);
          m_tooltipItem->tooltip()->hide();
          m_tooltipItem = 0;
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


void Smb4KSharesView::leaveEvent(QEvent *e)
{
  if (m_tooltipItem)
  {
    emit aboutToHideToolTip(m_tooltipItem);
    m_tooltipItem->tooltip()->hide();
    m_tooltipItem = 0;
  }
  else
  {
    // Do nothing
  }
  
  m_mouseInside = false;
  QListWidget::leaveEvent(e);
}


void Smb4KSharesView::enterEvent(QEvent *e)
{
  m_mouseInside = true;
  QListWidget::enterEvent(e);
}


void Smb4KSharesView::mousePressEvent(QMouseEvent *e)
{
  // Hide the current tool tip so that it is not in the way.
  if (m_tooltipItem)
  {
    emit aboutToHideToolTip(m_tooltipItem);
    m_tooltipItem->tooltip()->hide();
    m_tooltipItem = 0;
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


void Smb4KSharesView::focusOutEvent(QFocusEvent *e)
{
  QListWidget::focusOutEvent(e);
}


void Smb4KSharesView::wheelEvent(QWheelEvent *e)
{
  if (m_tooltipItem)
  {
    emit aboutToHideToolTip(m_tooltipItem);
    m_tooltipItem->tooltip()->hide();
    m_tooltipItem = 0;
  }
  else
  {
    // Do nothing
  }
  
  QListWidget::wheelEvent(e);
}


void Smb4KSharesView::dragEnterEvent(QDragEnterEvent *e)
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


void Smb4KSharesView::dragMoveEvent(QDragMoveEvent *e)
{
  // Let the QAbstractItemView do the highlighting of the item, etc.
  QAbstractItemView::dragMoveEvent(e);

  // Now we do our thing.
  Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(itemAt(e->pos()));

  if (item && (item->flags() & Qt::ItemIsDropEnabled) && (e->proposedAction() & (Qt::CopyAction | Qt::MoveAction)))
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


void Smb4KSharesView::dropEvent(QDropEvent *e)
{
  Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(itemAt(e->pos()));

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


Qt::DropActions Smb4KSharesView::supportedDropActions() const
{
  // Only allow copying and linking.
  return Qt::CopyAction | Qt::LinkAction;
}


QMimeData *Smb4KSharesView::mimeData(const QList<QListWidgetItem *> list) const
{
  QMimeData *mimeData = new QMimeData();
  QList<QUrl> urls;

  for (int i = 0; i < list.count(); ++i)
  {
    Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(list.at(i));
    urls.append(QUrl(item->shareItem()->path()));
  }

  mimeData->setUrls(urls);

  return mimeData;
}


void Smb4KSharesView::startDrag(Qt::DropActions supported)
{
  if (m_tooltipItem)
  {
    emit aboutToHideToolTip(m_tooltipItem);
    m_tooltipItem->tooltip()->hide();
    m_tooltipItem = 0;
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
      Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(list.first());
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

void Smb4KSharesView::slotItemEntered(QListWidgetItem *item)
{
  Smb4KSharesViewItem *share_item = static_cast<Smb4KSharesViewItem *>(item);
  
  if (m_tooltipItem && m_tooltipItem != share_item)
  {
    emit aboutToHideToolTip(m_tooltipItem);
    m_tooltipItem->tooltip()->hide();
    m_tooltipItem = 0;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesView::slotViewportEntered()
{
  // Hide the tool tip.
  if (m_tooltipItem)
  {
    emit aboutToHideToolTip(m_tooltipItem);
    m_tooltipItem->tooltip()->hide();
    m_tooltipItem = 0;
  }
  else
  {
    // Do nothing
  }
}

