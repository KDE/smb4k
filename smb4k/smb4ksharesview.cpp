/***************************************************************************
    This is the shares view of Smb4K.
                             -------------------
    begin                : Mo Dez 4 2006
    copyright            : (C) 2006-2020 by Alexander Reinholdt
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

// application specific includes
#include "smb4ksharesview.h"
#include "smb4ksharesviewitem.h"
#include "core/smb4kshare.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDrag>
#include <QDesktopWidget>
#include <QApplication>
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0) 
#include <QDesktopWidget>
#else
#include <QScreen>
#endif
#include <QLayout>

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
  
  m_toolTip = new KToolTipWidget(this);
  
  setContextMenuPolicy(Qt::CustomContextMenu);
}


Smb4KSharesView::~Smb4KSharesView()
{
}


void Smb4KSharesView::setViewMode(QListView::ViewMode mode, int iconSize)
{
  //
  // Set the view mode
  //
  QListWidget::setViewMode(mode);
  
  //
  // Make adjustments
  //
  switch(mode)
  {
    case IconMode:
    {
      setUniformItemSizes(true);
      setIconSize(QSize(iconSize, iconSize));
      setSpacing(5);
      break;
    }
    case ListMode:
    {
      setUniformItemSizes(false);
      setIconSize(QSize(iconSize, iconSize));
      setSpacing(0);
      break;
    }
    default:
    {
      break;
    }
  }
  
  //
  // Align the items 
  //
  for (int i = 0; i < count(); ++i)
  {
    Smb4KSharesViewItem *viewItem = static_cast<Smb4KSharesViewItem *>(item(i));
    viewItem->setItemAlignment(mode);
  }
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
          QPoint tooltipPos = cursor().pos();
            
          int testWidth = item->toolTipContentsWidget()->width() + cursor().pos().x() + m_toolTip->layout()->contentsMargins().left() + m_toolTip->layout()->contentsMargins().right();
            
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            if (QApplication::desktop()->screenGeometry(pos).width() < testWidth)
            {
              tooltipPos.setX(cursor().pos().x() - item->toolTipContentsWidget()->width() - m_toolTip->layout()->contentsMargins().left() - m_toolTip->layout()->contentsMargins().right());
            }
            
            int testHeight = item->toolTipContentsWidget()->height() + cursor().pos().y() + m_toolTip->layout()->contentsMargins().top() + m_toolTip->layout()->contentsMargins().bottom();
            
            if (QApplication::desktop()->screenGeometry(pos).height() < testHeight)
            {
              tooltipPos.setY(cursor().pos().y() - item->toolTipContentsWidget()->height() - m_toolTip->layout()->contentsMargins().top() - m_toolTip->layout()->contentsMargins().bottom());
            }
#else
            if (QApplication::screenAt(pos)->virtualSize().width() < testWidth)
            {
              tooltipPos.setX(cursor().pos().x() - item->toolTipContentsWidget()->width() - m_toolTip->layout()->contentsMargins().left() - m_toolTip->layout()->contentsMargins().right());
            }
            
            int testHeight = item->toolTipContentsWidget()->height() + cursor().pos().y() + m_toolTip->layout()->contentsMargins().top() + m_toolTip->layout()->contentsMargins().bottom();
            
            if (QApplication::screenAt(pos)->virtualSize().height() < testHeight)
            {
              tooltipPos.setY(cursor().pos().y() - item->toolTipContentsWidget()->height() - m_toolTip->layout()->contentsMargins().top() - m_toolTip->layout()->contentsMargins().bottom());
            }
#endif
            
          m_toolTip->showAt(tooltipPos, item->toolTipContentsWidget(), nativeParentWidget()->windowHandle());
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


void Smb4KSharesView::mousePressEvent(QMouseEvent *e)
{
  // Get the item that is under the mouse. If there is no
  // item, unselect the current item.
  QListWidgetItem *item = itemAt(e->pos());

  if (!item && !selectedItems().isEmpty())
  {
    clearSelection();
    setCurrentItem(0);
    emit itemPressed(currentItem());
  }

  QListWidget::mousePressEvent(e);
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

  if (item && !item->shareItem()->isInaccessible() && (item->flags() & Qt::ItemIsDropEnabled) && (e->proposedAction() & (Qt::CopyAction | Qt::MoveAction)))
  {
    QUrl url = QUrl::fromLocalFile(item->shareItem()->path());

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
  // Get the item and process the drop event
  Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(itemAt(e->pos()));

  if (item && !item->shareItem()->isInaccessible() && (e->proposedAction() & (Qt::CopyAction|Qt::MoveAction)))
  {
    QUrl url = QUrl::fromLocalFile(item->shareItem()->path());

    if (e->source() == this && e->mimeData()->urls().first() == url)
    {
      e->ignore();
    }
    else
    {
      e->acceptProposedAction();
      emit acceptedDropEvent(item, e);
      e->accept();
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
  return (Qt::CopyAction|Qt::LinkAction);
}


QMimeData *Smb4KSharesView::mimeData(const QList<QListWidgetItem *> list) const
{
  QMimeData *mimeData = new QMimeData();
  QList<QUrl> urls;

  for (int i = 0; i < list.count(); ++i)
  {
    Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(list.at(i));
    urls << QUrl::fromLocalFile(item->shareItem()->path());
  }

  mimeData->setUrls(urls);

  return mimeData;
}


void Smb4KSharesView::startDrag(Qt::DropActions supported)
{
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
}

