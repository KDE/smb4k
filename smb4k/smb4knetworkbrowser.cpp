/***************************************************************************
    smb4knetworkbrowser  -  The network browser widget of Smb4K.
                             -------------------
    begin                : Mo Jan 8 2007
    copyright            : (C) 2007-2020 by Alexander Reinholdt
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
#include "smb4knetworkbrowser.h"
#include "smb4knetworkbrowseritem.h"
#include "smb4ktooltip.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QTimer>
#include <QMouseEvent>
#include <QHeaderView>
#include <QApplication>

#include <QLayout>

// KDE includes
#include <KI18n/KLocalizedString>

using namespace Smb4KGlobal;

Smb4KNetworkBrowser::Smb4KNetworkBrowser(QWidget *parent)
: QTreeWidget(parent)
{
  setRootIsDecorated(true);
  setAllColumnsShowFocus(false);
  setMouseTracking(true);
  setSelectionMode(ExtendedSelection);
  
  setContextMenuPolicy(Qt::CustomContextMenu);
  
  m_toolTip = new Smb4KToolTip(this);

  QStringList header_labels;
  header_labels.append(i18n("Network"));
  header_labels.append(i18n("Type"));
  header_labels.append(i18n("IP Address"));
  header_labels.append(i18n("Comment"));
  setHeaderLabels(header_labels);

  header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // 
  // Connections
  // 
  connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
}


Smb4KNetworkBrowser::~Smb4KNetworkBrowser()
{
}


Smb4KToolTip *Smb4KNetworkBrowser::toolTip()
{
  return m_toolTip;
}



bool Smb4KNetworkBrowser::event(QEvent *e)
{
  switch (e->type())
  {
    case QEvent::ToolTip:
    {
      // 
      // Intercept the tool tip event and show our own tool tip
      // 
      QPoint pos = viewport()->mapFromGlobal(cursor().pos());
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(itemAt(pos));
      
      if (item)
      {
        if (Smb4KSettings::showNetworkItemToolTip())
        {
          int ind = 0;

          switch (item->type())
          {
            case Host:
            {
              ind = 2;
              break;
            }
            case Share:
            {
              ind = 3;
              break;
            }
            default:
            {
              ind = 1;
              break;
            }
          }
          
          //
          // Show the tooltip 
          // 
          if (pos.x() > ind * indentation())
          {
            //
            // Set up the tooltip
            // 
            m_toolTip->setupToolTip(Smb4KToolTip::NetworkItem, item->networkItem());
            
            //
            // Show the tooltip
            // 
            m_toolTip->show(cursor().pos(), nativeParentWidget()->windowHandle());
          }
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


void Smb4KNetworkBrowser::mousePressEvent(QMouseEvent *e)
{
  //
  // Hide the tooltip
  // 
  if (m_toolTip->isVisible())
  {
    m_toolTip->hide();
  }
  
  // 
  // Get the item that is under the mouse. If there is no
  // item, unselect the current item.
  // 
  QTreeWidgetItem *item = itemAt(e->pos());

  if (!item && currentItem())
  {
    currentItem()->setSelected(false);
    setCurrentItem(0);
  }

  QTreeWidget::mousePressEvent(e);
}


void Smb4KNetworkBrowser::mouseMoveEvent(QMouseEvent* e)
{
  //
  // Hide the tooltip
  // 
  if (m_toolTip->isVisible())
  {
    m_toolTip->hide();
  }
  
  QTreeWidget::mouseMoveEvent(e);
}



/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KNetworkBrowser::slotItemActivated(QTreeWidgetItem *item, int /*column*/)
{
  // Only do something if there are no keyboard modifiers pressed
  // and there is only one item selected.
  if (QApplication::keyboardModifiers() == Qt::NoModifier && selectedItems().size() == 1)
  {
    if (item)
    {
      switch (item->type())
      {
        case Workgroup:
        case Host:
        {
          if (!item->isExpanded())
          {
            expandItem(item);
          }
          else
          {
            collapseItem(item);
          }

          break;
        }
        default:
        {
          break;
        }
      }
    }
  }
}


void Smb4KNetworkBrowser::slotItemSelectionChanged()
{
  if (selectedItems().size() > 1)
  {
    // If multiple items are selected, only allow shares
    // to stay selected.
    for (int i = 0; i < selectedItems().size(); ++i)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItems()[i]);
      
      if (item)
      {
        switch (item->networkItem()->type())
        {
          case Workgroup:
          case Host:
          {
            item->setSelected(false);
            break;
          }
          case Share:
          {
            if (item->shareItem()->isPrinter())
            {
              item->setSelected(false);
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
    }
  }
}

