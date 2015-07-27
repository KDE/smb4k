/***************************************************************************
    smb4kshareslistview  -  This is the shares list view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
    copyright            : (C) 2007-2015 by Alexander Reinholdt
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
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QDrag>
#include <QtCore/QUrl>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>


Smb4KSharesListView::Smb4KSharesListView(QWidget *parent)
: QTreeWidget(parent)
{
  setAllColumnsShowFocus(false);
  setMouseTracking(true);
  setRootIsDecorated(false);
  setSelectionMode(ExtendedSelection);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  setContextMenuPolicy(Qt::CustomContextMenu);

  m_tooltip_item = 0;
  m_mouse_inside = false;

  QStringList header_labels;
  header_labels.append(i18n("Item"));
#if defined(Q_OS_LINUX)
  header_labels.append(i18n("Login"));
#endif
  header_labels.append(i18n("File System"));
  header_labels.append(i18n("Owner"));
  header_labels.append(i18n("Free"));
  header_labels.append(i18n("Used"));
  header_labels.append(i18n("Total"));
  header_labels.append(i18n("Usage"));
  setHeaderLabels(header_labels);

  header()->setStretchLastSection(false);
  header()->setResizeMode(QHeaderView::ResizeToContents);
  header()->setResizeMode(Item, QHeaderView::Stretch);

  // Connections:
  connect(this, SIGNAL(itemEntered(QTreeWidgetItem*,int)),
          this, SLOT(slotItemEntered(QTreeWidgetItem*,int)));
  
  connect(this, SIGNAL(viewportEntered()),
          this, SLOT(slotViewportEntered()));
}


Smb4KSharesListView::~Smb4KSharesListView()
{
}


bool Smb4KSharesListView::event(QEvent *e)
{
  switch (e->type())
  {
    case QEvent::ToolTip:
    {
      // Intercept the tool tip event and show our own tool tip.
      QPoint pos = viewport()->mapFromGlobal(cursor().pos());
      Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(itemAt(pos));
      
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

  return QTreeWidget::event(e);
}


void Smb4KSharesListView::mouseMoveEvent(QMouseEvent *e)
{
  QPoint pos = viewport()->mapFromGlobal(cursor().pos());

  // Find the item over which the user moved the mouse:
  Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(itemAt(pos));

  if (item)
  {
    emit itemEntered(item, columnAt(pos.x()));
  }
  else
  {
    // Do nothing
  }

  QTreeWidget::mouseMoveEvent(e);
}


void Smb4KSharesListView::leaveEvent(QEvent *e)
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
  QTreeWidget::leaveEvent(e);
}


void Smb4KSharesListView::enterEvent(QEvent *e)
{
  m_mouse_inside = true;
  QTreeWidget::enterEvent(e);
}


void Smb4KSharesListView::mousePressEvent(QMouseEvent *e)
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
  QTreeWidgetItem *item = itemAt(e->pos());

  if (!item && !selectedItems().isEmpty())
  {
    clearSelection();
    setCurrentItem(0);
    emit itemPressed(currentItem(), -1);
  }
  else
  {
    // Do nothing
  }

  QTreeWidget::mousePressEvent(e);
}


void Smb4KSharesListView::focusOutEvent(QFocusEvent *e)
{
  QTreeWidget::focusOutEvent(e);
}


void Smb4KSharesListView::wheelEvent(QWheelEvent *e)
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
  
  QTreeWidget::wheelEvent(e);
}


void Smb4KSharesListView::dragEnterEvent(QDragEnterEvent *e)
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


void Smb4KSharesListView::dragMoveEvent(QDragMoveEvent *e)
{
  // Let the QAbstractItemView do the highlighting of the item, etc.
  QAbstractItemView::dragMoveEvent(e);

  // Now we do our thing.
  Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(itemAt(e->pos()));

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


void Smb4KSharesListView::dropEvent(QDropEvent *e)
{
  Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(itemAt(e->pos()));

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


Qt::DropActions Smb4KSharesListView::supportedDropActions() const
{
  // Only allow copying and linking.
  return Qt::CopyAction | Qt::LinkAction;
}


QMimeData *Smb4KSharesListView::mimeData(const QList<QTreeWidgetItem *> list) const
{
  QMimeData *mimeData = new QMimeData();
  QList<QUrl> urls;

  for (int i = 0; i < list.count(); ++i)
  {
    Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(list.at(i));
    urls.append(QUrl(item->shareItem()->path()));
  }

  mimeData->setUrls(urls);

  return mimeData;
}


void Smb4KSharesListView::startDrag(Qt::DropActions supported)
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

  QList<QTreeWidgetItem *> list = selectedItems();

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
      Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(list.first());
      pixmap = item->shareItem()->icon().pixmap(KIconLoader::SizeMedium);
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

void Smb4KSharesListView::slotItemEntered(QTreeWidgetItem *item, int /*column*/)
{
  Smb4KSharesListViewItem *share_item = static_cast<Smb4KSharesListViewItem *>(item);
  
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


void Smb4KSharesListView::slotViewportEntered()
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

