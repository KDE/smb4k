/***************************************************************************
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.
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
#include "smb4knetworkbrowseritem.h"
#include "core/smb4kglobal.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QDebug>
#include <QBrush>
#include <QApplication>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KSeparator>
#include <KI18n/KLocalizedString>

using namespace Smb4KGlobal;


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem(QTreeWidget *parent, const NetworkItemPtr &item)
: QTreeWidgetItem(parent, item->type()), m_item(item)
{
  switch (m_item->type())
  {
    case Workgroup:
    {
      WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
      setText(Network, workgroup->workgroupName());
      setIcon(Network, workgroup->icon());
      break;
    }
    case Host:
    {
      HostPtr host = m_item.staticCast<Smb4KHost>();
      setText(Network, host->hostName());
      setText(IP, host->ipAddress());
      setText(Comment, host->comment());
      
      if (host->isMasterBrowser())
      {
        for (int i = 0; i < columnCount(); ++i)
        {
          QBrush brush(Qt::darkBlue);
          setForeground(i, brush);
        }
      }

      setIcon(Network, host->icon());
      break;
    }
    case Share:
    {
      SharePtr share = m_item.staticCast<Smb4KShare>();
      setText(Network, share->shareName());
      setText(Type, share->shareTypeString());
      setText(Comment, share->comment());

      if (!share->isPrinter() && share->isMounted())
      {
        for (int i = 0; i < columnCount(); ++i)
        {
          QFont f = font(i);
          f.setItalic(true);
          setFont(i, f);
        }
      }

      setIcon(Network, share->icon());
      break;
    }
    default:
    {
      break;
    }
  }
  
  setupToolTipContentsWidget();
}


Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem(QTreeWidgetItem *parent, const NetworkItemPtr &item)
: QTreeWidgetItem(parent, item->type()), m_item(item)
{
  switch (m_item->type())
  {
    case Workgroup:
    {
      WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
      setText(Network, workgroup->workgroupName());
      setIcon(Network, workgroup->icon());
      break;
    }
    case Host:
    {
      HostPtr host = m_item.staticCast<Smb4KHost>();
      setText(Network, host->hostName());
      setText(IP, host->ipAddress());
      setText(Comment, host->comment());
      
      if (host->isMasterBrowser())
      {
        for (int i = 0; i < columnCount(); ++i)
        {
          QBrush brush(Qt::darkBlue);
          setForeground(i, brush);
        }
      }

      setIcon(Network, host->icon());
      break;
    }
    case Share:
    {
      SharePtr share = m_item.staticCast<Smb4KShare>();
      setText(Network, share->shareName());
      setText(Type, share->shareTypeString());
      setText(Comment, share->comment());

      if (!share->isPrinter() && share->isMounted())
      {
        for (int i = 0; i < columnCount(); ++i)
        {
          QFont f = font(i);
          f.setItalic(true);
          setFont(i, f);
        }
      }
      
      setIcon(Network, share->icon());
      break;
    }
    default:
    {
      break;
    }
  }
  
  setupToolTipContentsWidget();
}



Smb4KNetworkBrowserItem::~Smb4KNetworkBrowserItem()
{
  m_toolTipContentsWidget.clear();
}


WorkgroupPtr Smb4KNetworkBrowserItem::workgroupItem()
{
  if (!m_item || (m_item && m_item->type() != Workgroup))
  {
    return WorkgroupPtr();
  }
  
  return m_item.staticCast<Smb4KWorkgroup>();
}


HostPtr Smb4KNetworkBrowserItem::hostItem()
{
  if (!m_item || (m_item && m_item->type() != Host))
  {
    return HostPtr();
  }
  
  return m_item.staticCast<Smb4KHost>();
}


SharePtr Smb4KNetworkBrowserItem::shareItem()
{
  if (!m_item || (m_item && m_item->type() != Share))
  {
    return SharePtr();
  }
  
  return m_item.staticCast<Smb4KShare>();
}


const NetworkItemPtr &Smb4KNetworkBrowserItem::networkItem()
{
  return m_item;
}


void Smb4KNetworkBrowserItem::update()
{
  switch (m_item->type())
  {
    case Host:
    {
      HostPtr host = m_item.staticCast<Smb4KHost>();
      
      // Adjust the item's color.
      if (host->isMasterBrowser())
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
      setText(IP, host->ipAddress());

      // Set the comment 
      setText(Comment, host->comment());
      break;
    }
    case Share:
    {
      SharePtr share = m_item.staticCast<Smb4KShare>();
      
      // Set the comment
      setText(Comment, share->comment());
    
      // Set the icon
      setIcon(Network, share->icon());
            
      // Set the font
      for (int i = 0; i < columnCount(); ++i)
      {
        QFont f = font(i);
        f.setItalic(share->isMounted());
        setFont(i, f);
      }
        
      break;
    }
    default:
    {
      break;
    }
  }
    
  setupToolTipContentsWidget();
}


QWidget *Smb4KNetworkBrowserItem::toolTipContentsWidget()
{
  return m_toolTipContentsWidget;
}



void Smb4KNetworkBrowserItem::setupToolTipContentsWidget()
{
  //
  // The layout
  // 
  QHBoxLayout *mainLayout = nullptr;
  
  // 
  // Make sure the widget exists
  // 
  if (!m_toolTipContentsWidget)
  {
    m_toolTipContentsWidget = new QWidget(treeWidget());
    mainLayout = new QHBoxLayout(m_toolTipContentsWidget);
  }
  else
  {
    mainLayout = qobject_cast<QHBoxLayout *>(m_toolTipContentsWidget->layout());
  }
  
  //
  // Update the contents, if possible
  // 
  if (!m_toolTipContentsWidget->layout()->isEmpty())
  {
    switch (m_item->type())
    {
      case Workgroup:
      {
        WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
        QLabel *masterBrowserName = m_toolTipContentsWidget->findChild<QLabel *>("MasterBrowserName");
            
        if (workgroup->hasMasterBrowserIpAddress())
        {
          masterBrowserName->setText(workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIpAddress()+')');
        }
        else
        {
          masterBrowserName->setText(workgroup->masterBrowserName());
        }
        break;
      }
      case Host:
      {
        HostPtr host = m_item.staticCast<Smb4KHost>();
        m_toolTipContentsWidget->findChild<QLabel *>("CommentString")->setText(!host->comment().isEmpty() ? host->comment() : "-");
        m_toolTipContentsWidget->findChild<QLabel *>("IPAddressString")->setText(host->hasIpAddress() ? host->ipAddress() : "-");
        break;
      }
      case Share:
      {
        SharePtr share = m_item.staticCast<Smb4KShare>();
        
        m_toolTipContentsWidget->findChild<QLabel *>("CommentString")->setText(!share->comment().isEmpty() ? share->comment() : "-");
        m_toolTipContentsWidget->findChild<QLabel *>("IPAddressString")->setText(share->hasHostIpAddress() ? share->hostIpAddress() : "-");
        
        QLabel *mountedState = m_toolTipContentsWidget->findChild<QLabel *>("MountedState");
            
        if (!share->isPrinter())
        {
          mountedState->setText(share->isMounted() ? i18n("yes") : i18n("no"));
        }
        else
        {
          mountedState->setText("-");
        }
        break;
      }
      default:
      {
        break;
      }
    }
    
    return;
  }
  
  //
  // Set up the widget
  // 
  
  // Icon
  QLabel *iconLabel = new QLabel(m_toolTipContentsWidget);
  iconLabel->setPixmap(m_item->icon().pixmap(KIconLoader::SizeEnormous));
  mainLayout->addWidget(iconLabel, Qt::AlignHCenter);
  
  // Header
  QGridLayout *descriptionLayout = new QGridLayout();
  mainLayout->addLayout(descriptionLayout);
  
  QLabel *caption = new QLabel(m_toolTipContentsWidget);
  caption->setForegroundRole(QPalette::ToolTipText);
  caption->setBackgroundRole(QPalette::AlternateBase);
      
  QFont captionFont = caption->font();
  captionFont.setBold(true);
  caption->setFont(captionFont);

  descriptionLayout->addWidget(caption, 0, 0, 1, 2, Qt::AlignHCenter);
  
  KSeparator *separator = new KSeparator(Qt::Horizontal, m_toolTipContentsWidget);
  separator->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(separator, 1, 0, 1, 2, 0);
  
  // Type
  QLabel *typeCaption = new QLabel(i18n("Type:"), m_toolTipContentsWidget);
  typeCaption->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(typeCaption, 2, 0, Qt::AlignRight);
  
  QLabel *typeName = new QLabel(m_toolTipContentsWidget);
  typeName->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(typeName, 2, 1, 0);
  
  switch (m_item->type())
  {
    case Workgroup:
    {
      WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
     
      caption->setText(workgroup->workgroupName());
      typeName->setText(i18n("Workgroup"));
      
      // Master browser
      QLabel *masterBrowserLabel = new QLabel(i18n("Master Browser:"), m_toolTipContentsWidget);
      masterBrowserLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(masterBrowserLabel, 3, 0, Qt::AlignRight);
      
      QLabel *masterBrowserName = new QLabel(m_toolTipContentsWidget);
      masterBrowserName->setObjectName("MasterBrowserName");
      masterBrowserName->setForegroundRole(QPalette::ToolTipText);
      
      if (workgroup->hasMasterBrowserIpAddress())
      {
        masterBrowserName->setText(QString("%1 (%2)").arg(workgroup->masterBrowserName()).arg(workgroup->masterBrowserIpAddress()));
      }
      else
      {
        masterBrowserName->setText(workgroup->masterBrowserName());
      }
      
      descriptionLayout->addWidget(masterBrowserName, 3, 1, 0);
      
      descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 4, 0, 2, 1, 0);
      
      break;
    }
    case Host:
    {
      HostPtr host = m_item.staticCast<Smb4KHost>();
      caption->setText(host->hostName());
      typeName->setText(i18n("Host"));
      
      // Comment
      QLabel *commentLabel = new QLabel(i18n("Comment:"), m_toolTipContentsWidget);
      commentLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(commentLabel, 3, 0, Qt::AlignRight);
      
      QLabel *commentString = new QLabel(!host->comment().isEmpty() ? host->comment() : "-", m_toolTipContentsWidget);
      commentString->setObjectName("CommentString");
      commentString->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(commentString, 3, 1, 0);
      
      // IP address
      QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), m_toolTipContentsWidget);
      ipAddressLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(ipAddressLabel, 4, 0, Qt::AlignRight);
      
      QLabel *ipAddress = new QLabel(host->hasIpAddress() ? host->ipAddress() : "-", m_toolTipContentsWidget);
      ipAddress->setObjectName("IPAddressString");
      ipAddress->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(ipAddress, 4, 1, 0);
      
      // Workgroup
      QLabel *workgroupLabel = new QLabel(i18n("Workgroup:"), m_toolTipContentsWidget);
      workgroupLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(workgroupLabel, 5, 0, Qt::AlignRight);
      
      QLabel *workgroupName = new QLabel(host->workgroupName(), m_toolTipContentsWidget);
      workgroupName->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(workgroupName, 5, 1, 0);
      
      descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 6, 0, 2, 1, 0);
      
      break;
    }
    case Share:
    {
      SharePtr share = m_item.staticCast<Smb4KShare>();
      caption->setText(share->shareName());
      typeName->setText(i18n("Share (%1)", share->shareTypeString()));
      
      // Comment
      QLabel *commentLabel = new QLabel(i18n("Comment:"), m_toolTipContentsWidget);
      commentLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(commentLabel, 3, 0, Qt::AlignRight);
      
      QLabel *commentString = new QLabel(!share->comment().isEmpty() ? share->comment() : "-", m_toolTipContentsWidget);
      commentString->setObjectName("CommentString");
      commentString->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(commentString, 3, 1, 0);
      
      // State (mounted/not mounted)
      QLabel *mountedLabel = new QLabel(i18n("Mounted:"), m_toolTipContentsWidget);
      mountedLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(mountedLabel, 4, 0, Qt::AlignRight);
      
      QLabel *mountedState = nullptr;
      
      if (!share->isPrinter())
      {
        mountedState = new QLabel(share->isMounted() ? i18n("yes") : i18n("no"), m_toolTipContentsWidget);
      }
      else
      {
        mountedState = new QLabel("-", m_toolTipContentsWidget);
      }      
      
      mountedState->setObjectName("MountedState");
      mountedState->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(mountedState, 4, 1, 0);
      
      // Host
      QLabel *hostLabel = new QLabel(i18n("Host:"), m_toolTipContentsWidget);
      hostLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(hostLabel, 5, 0, Qt::AlignRight);
      
      QLabel *hostName = new QLabel(share->hostName(), m_toolTipContentsWidget);
      hostName->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(hostName, 5, 1, 0);
      
      // IP address
      QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), m_toolTipContentsWidget);
      ipAddressLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(ipAddressLabel, 6, 0, Qt::AlignRight);
      
      QLabel *ipAddressString = new QLabel(share->hasHostIpAddress() ? share->hostIpAddress() : "-", m_toolTipContentsWidget);
      ipAddressString->setObjectName("IPAddressString");
      ipAddressString->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(ipAddressString, 6, 1, 0);
      
      // Location
      QLabel *locationLabel = new QLabel(i18n("Location:"), m_toolTipContentsWidget);
      locationLabel->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(locationLabel, 7, 0, Qt::AlignRight);
      
      QLabel *locationString = new QLabel(share->displayString(), m_toolTipContentsWidget);
      locationString->setForegroundRole(QPalette::ToolTipText);
      
      descriptionLayout->addWidget(locationString, 7, 1, 0);
      
      descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 8, 0, 2, 1, 0);
      
      break;
    }
    default:
    {
      break;
    }
  }
  
  m_toolTipContentsWidget->adjustSize();
}



