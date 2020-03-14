/***************************************************************************
    The item for Smb4K's shares view.
                             -------------------
    begin                : Di Dez 5 2006
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
#include "smb4ksharesviewitem.h"
#include "smb4ksharesview.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QHBoxLayout>
#include <QLabel>

// KDE includes
#include <KWidgetsAddons/KSeparator>
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>


Smb4KSharesViewItem::Smb4KSharesViewItem(Smb4KSharesView *parent, const SharePtr &share)
: QListWidgetItem(parent), m_share(share)
{
  setFlags(flags() | Qt::ItemIsDropEnabled);
  setItemAlignment(parent->viewMode());

  setText(m_share->displayString());
  setIcon(m_share->icon());
  
  setupToolTipContentsWidget();
}


Smb4KSharesViewItem::~Smb4KSharesViewItem()
{
  m_toolTipContentsWidget.clear();
}


void Smb4KSharesViewItem::update()
{
  setText(m_share->displayString());
  setIcon(m_share->icon());

  setupToolTipContentsWidget();
}


void Smb4KSharesViewItem::setItemAlignment(QListView::ViewMode mode)
{
  switch (mode)
  {
    case QListView::IconMode:
    {
      setTextAlignment(Qt::AlignHCenter|Qt::AlignTop);
      break;
    }
    case QListView::ListMode:
    {
      setTextAlignment(Qt::AlignAbsolute|Qt::AlignVCenter);
      break;
    }
    default:
    {
      break;
    }
  }
}


QWidget *Smb4KSharesViewItem::toolTipContentsWidget()
{
  return m_toolTipContentsWidget;
}


void Smb4KSharesViewItem::setupToolTipContentsWidget()
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
    m_toolTipContentsWidget = new QWidget(listWidget());
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
    m_toolTipContentsWidget->findChild<QLabel *>("IconLabel")->setPixmap(m_share->icon().pixmap(KIconLoader::SizeEnormous));
    m_toolTipContentsWidget->findChild<QLabel *>("LoginString")->setText(!m_share->login().isEmpty() ? m_share->login() : i18n("unknown"));
    
    QString sizeIndication;
    
    if (m_share->totalDiskSpace() != 0 && m_share->freeDiskSpace() != 0) 
    {
      sizeIndication = i18n("%1 free of %2 (%3 used)", m_share->freeDiskSpaceString(), m_share->totalDiskSpaceString(), m_share->diskUsageString());
    }
    else
    {
      sizeIndication = i18n("unknown");
    }
    
    m_toolTipContentsWidget->findChild<QLabel *>("SizeString")->setText(sizeIndication);
    
    return;
  }
  
  //
  // Set up the widget
  // 
  
  // Icon
  QLabel *iconLabel = new QLabel(m_toolTipContentsWidget);
  iconLabel->setPixmap(m_share->icon().pixmap(KIconLoader::SizeEnormous));
  iconLabel->setObjectName("IconLabel");
  mainLayout->addWidget(iconLabel, Qt::AlignHCenter);
  
  // Header
  QGridLayout *descriptionLayout = new QGridLayout();
  mainLayout->addLayout(descriptionLayout);
  
  QLabel *caption = new QLabel(m_share->shareName(), m_toolTipContentsWidget);
  caption->setForegroundRole(QPalette::ToolTipText);
  caption->setBackgroundRole(QPalette::AlternateBase);
      
  QFont captionFont = caption->font();
  captionFont.setBold(true);
  caption->setFont(captionFont);

  descriptionLayout->addWidget(caption, 0, 0, 1, 2, Qt::AlignHCenter);
  
  KSeparator *separator = new KSeparator(Qt::Horizontal, m_toolTipContentsWidget);
  separator->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(separator, 1, 0, 1, 2, 0);
  
  // Location
  QLabel *locationLabel = new QLabel(i18n("Location:"), m_toolTipContentsWidget);
  locationLabel->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(locationLabel, 2, 0, Qt::AlignRight);
  
  QLabel *locationString = new QLabel(m_share->displayString(), m_toolTipContentsWidget);
  locationString->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(locationString, 2, 1, 0);
  
  // Mount point
  QLabel *mountpointLabel = new QLabel(i18n("Mountpoint:"), m_toolTipContentsWidget);
  mountpointLabel->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(mountpointLabel, 3, 0, Qt::AlignRight);
  
  QLabel *mountpointString = new QLabel(m_share->path(), m_toolTipContentsWidget);
  mountpointString->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(mountpointString, 3, 1, 0);
  
  // Login
  QLabel *loginLabel = new QLabel(i18n("Login:"), m_toolTipContentsWidget);
  loginLabel->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(loginLabel, 4, 0, Qt::AlignRight);
  
  QLabel *loginString = new QLabel(!m_share->login().isEmpty() ? m_share->login() : i18n("unknown"), m_toolTipContentsWidget);
  loginString->setObjectName("LoginString");
  loginString->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(loginString, 4, 1, 0);
  
  // Owner
  QLabel *ownerLabel = new QLabel(i18n("Owner:"), m_toolTipContentsWidget);
  ownerLabel->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(ownerLabel, 5, 0, Qt::AlignRight);
  
  QString owner(!m_share->user().loginName().isEmpty() ? m_share->user().loginName() : i18n("unknown"));
  QString group(!m_share->group().name().isEmpty() ? m_share->group().name() : i18n("unknown"));
  
  QLabel *ownerString = new QLabel(QString("%1 - %2").arg(owner, group),m_toolTipContentsWidget);
  ownerString->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(ownerString, 5, 1, 0);
  
  // File system
  QLabel *fileSystemLabel = new QLabel(i18n("File system:"), m_toolTipContentsWidget);
  fileSystemLabel->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(fileSystemLabel, 6, 0, Qt::AlignRight);
  
  QLabel *fileSystemString = new QLabel(m_share->fileSystemString(), m_toolTipContentsWidget);
  fileSystemString->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(fileSystemString, 6, 1, 0);
  
  // Size
  QLabel *sizeLabel = new QLabel(i18n("Size:"), m_toolTipContentsWidget);
  sizeLabel->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(sizeLabel, 7, 0, Qt::AlignRight);
  
  QString sizeIndication;
  
  if (m_share->totalDiskSpace() != 0 && m_share->freeDiskSpace() != 0) 
  {
    sizeIndication = i18n("%1 free of %2 (%3 used)", m_share->freeDiskSpaceString(), m_share->totalDiskSpaceString(), m_share->diskUsageString());
  }
  else
  {
    sizeIndication = i18n("unknown");
  }
  
  QLabel *sizeString = new QLabel(sizeIndication, m_toolTipContentsWidget);
  sizeString->setObjectName("SizeString");
  sizeString->setForegroundRole(QPalette::ToolTipText);
  
  descriptionLayout->addWidget(sizeString, 7, 1, 0);

  descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 8, 0, 2, 1, 0);
  
  m_toolTipContentsWidget->adjustSize();
}



