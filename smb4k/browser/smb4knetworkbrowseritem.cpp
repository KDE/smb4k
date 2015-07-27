/***************************************************************************
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.
                             -------------------
    begin                : Mo Jan 8 2007
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
#include "smb4knetworkbrowseritem.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QtGui/QBrush>
#include <QtWidgets/QApplication>

// KDE includes



Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem(QTreeWidget *parent, Smb4KWorkgroup *workgroup)
: QTreeWidgetItem(parent, Workgroup)
{
  m_workgroup = new Smb4KWorkgroup(*workgroup);
  m_host      = 0;
  m_share     = 0;
  m_tooltip   = new Smb4KToolTip();
  m_tooltip->setup(Smb4KToolTip::NetworkBrowser, m_workgroup);
  setText(Network, m_workgroup->workgroupName());
  setIcon(Network, m_workgroup->icon());
}


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem(QTreeWidgetItem *parent, Smb4KHost *host)
: QTreeWidgetItem(parent, Host)
{
  m_workgroup = 0;
  m_host      = new Smb4KHost(*host);
  m_share     = 0;
  m_tooltip   = new Smb4KToolTip();
  m_tooltip->setup(Smb4KToolTip::NetworkBrowser, m_host);
  setText(Network, m_host->hostName());
  setText(IP, m_host->ip());
  setText(Comment, m_host->comment());

  if (m_host->isMasterBrowser())
  {
    for (int i = 0; i < columnCount(); ++i)
    {
      QBrush brush(Qt::darkBlue);
      setForeground(i, brush);
    }
  }
  else
  {
    // Do nothing
  }

  setIcon(Network, m_host->icon());
}


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem(QTreeWidgetItem *parent, Smb4KShare *share)
: QTreeWidgetItem(parent, Share)
{
  m_workgroup = 0;
  m_host      = 0;
  m_share     = new Smb4KShare(*share);
  m_tooltip   = new Smb4KToolTip();
  m_tooltip->setup(Smb4KToolTip::NetworkBrowser, m_share);
  setText(Network, m_share->shareName());
  setText(Type, m_share->translatedTypeString());
  setText(Comment, m_share->comment());

  if (!m_share->isPrinter() && m_share->isMounted())
  {
    for (int i = 0; i < columnCount(); ++i)
    {
      QFont f = font(i);
      f.setItalic(true);
      setFont(i, f);
    }
  }
  else
  {
    // Do nothing
  }
  
  setIcon(Network, m_share->icon());
}


Smb4KNetworkBrowserItem::~Smb4KNetworkBrowserItem()
{
  delete m_workgroup;
  delete m_host;
  delete m_share;
  delete m_tooltip;
}


Smb4KWorkgroup *Smb4KNetworkBrowserItem::workgroupItem()
{
  return m_workgroup;
}


Smb4KHost *Smb4KNetworkBrowserItem::hostItem()
{
  return m_host;
}


Smb4KShare *Smb4KNetworkBrowserItem::shareItem()
{
  return m_share;
}


Smb4KBasicNetworkItem* Smb4KNetworkBrowserItem::networkItem()
{
  switch (type())
  {
    case Workgroup:
    {
      return m_workgroup;
    }
    case Host:
    {
      return m_host;
    }
    case Share:
    {
      return m_share;
    }
    default:
    {
      break;
    }
  }
  
  return NULL;
}


void Smb4KNetworkBrowserItem::update(Smb4KBasicNetworkItem *item)
{
  if (item)
  {
    switch (item->type())
    {
      case Workgroup:
      {
        if (type() != Workgroup)
        {
          return;
        }
        else
        {
          // Do nothing
        }
        
        delete m_workgroup;
        m_workgroup = new Smb4KWorkgroup(*(static_cast<Smb4KWorkgroup *>(item)));
        
        m_tooltip->update(Smb4KToolTip::NetworkBrowser, m_workgroup);
        
        break;
      }
      case Host:
      {
        if (type() != Host)
        {
          return;
        }
        else
        {
          // Do nothing
        }
        
        delete m_host;
        m_host = new Smb4KHost(*(static_cast<Smb4KHost *>(item)));
        
        m_tooltip->update(Smb4KToolTip::NetworkBrowser, m_host);
        
        // Adjust the item's color.
        if (m_host->isMasterBrowser())
        {
          for (int i = 0; i < columnCount(); ++i)
          {
            QBrush brush(Qt::darkBlue);
            setForeground(i, brush);
          }
        }
        else
        {
          for (int i = 0; i < columnCount(); ++i)
          {
            QBrush brush = QApplication::palette().text();
            setForeground(i, brush);
          }          
        }
        
        // Set the IP address
        setText(IP, m_host->ip());

        // Set the comment 
        setText(Comment, m_host->comment());
        break;
      }
      case Share:
      {
        if (type() != Share)
        {
          return;
        }
        else
        {
          // Do nothing
        }
        
        delete m_share;
        m_share = new Smb4KShare(*(static_cast<Smb4KShare *>(item)));
        
        m_tooltip->update(Smb4KToolTip::NetworkBrowser, m_share);

        // Set the comment.
        setText(Comment, m_share->comment());
    
        // Set the icon
        setIcon(Network, m_share->icon());
            
        // Set the font
        for (int i = 0; i < columnCount(); ++i)
        {
          QFont f = font(i);
          f.setItalic(m_share->isMounted());
          setFont(i, f);
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


Smb4KToolTip* Smb4KNetworkBrowserItem::tooltip()
{
  return m_tooltip;
}

