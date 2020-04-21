/***************************************************************************
    smb4ktooltip  -  Provides tooltips for Smb4K
                             -------------------
    begin                : Sa Dez 23 2010
    copyright            : (C) 2010-2017 by Alexander Reinholdt
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
#include "smb4ktooltip.h"
#include "core/smb4kbasicnetworkitem.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QPalette>
#include <QLabel>
#include <QToolTip>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainterPath>
#include <QStylePainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QAbstractScrollArea>
#include <QScreen>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KSeparator>
#include <KConfigWidgets/KColorScheme>
#include <KWindowSystem/KWindowSystem>

using namespace Smb4KGlobal;


Smb4KToolTip::Smb4KToolTip(QWidget* parent)
: QWidget(parent, Qt::ToolTip|Qt::BypassGraphicsProxyWidget|Qt::FramelessWindowHint),
  m_item(NetworkItemPtr()), m_tip_layout(0), m_info_layout(0), m_text_layout(0)
{
  qDebug() << "Use KToolTipWidget for the tooltips";
  
  m_master_browser_label = 0;
  m_comment_label        = 0;
  m_ip_label             = 0;
  m_mounted_label        = 0;
  m_size_label           = 0;
  
  setAttribute(Qt::WA_TranslucentBackground);
  
  // Copied from QToolTip
  setForegroundRole(QPalette::ToolTipText);
  setBackgroundRole(QPalette::ToolTipBase);
  setPalette(QToolTip::palette());
  ensurePolished();
  setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
} 


Smb4KToolTip::~Smb4KToolTip()
{
  // Never delete m_item here. We only have a pointer to 
  // somewhere outside of this class.
}


void Smb4KToolTip::setup(Smb4KToolTip::Parent parent, const NetworkItemPtr &item)
{
  if (item)
  {
    m_item = item;
  
    // Set up tool tip.
    switch (parent)
    {
      case NetworkBrowser:
      {
        setupNetworkBrowserToolTip();
        break;
      }
      case SharesView:
      {
        setupSharesViewToolTip();
        break;
      }
      default:
      {
        return;
      }
    }
  }
}


void Smb4KToolTip::update(Smb4KToolTip::Parent parent, const NetworkItemPtr &item)
{
  if (item)
  {
    m_item = item;
    
    switch (parent)
    {
      case NetworkBrowser:
      {
        switch (m_item->type())
        {
          case Workgroup:
          {
            WorkgroupPtr workgroup = item.staticCast<Smb4KWorkgroup>();
            
            if (workgroup->hasMasterBrowserIpAddress())
            {
              m_master_browser_label->setText(workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIpAddress()+')');
            }
            else
            {
              m_master_browser_label->setText(workgroup->masterBrowserName());
            }
            break;
          }
          case Host:
          {
            HostPtr host = item.staticCast<Smb4KHost>();
            
            if (!host->comment().isEmpty())
            {
              m_comment_label->setText(host->comment());
            }
            else
            {
              m_comment_label->setText("-");
            }
            
            if (host->hasIpAddress())
            {
              m_ip_label->setText(host->ipAddress());
            }
            else
            {
              m_ip_label->setText("-");
            }
            break;
          }
          case Share:
          {
            SharePtr share = item.staticCast<Smb4KShare>();
            
            if (!share->comment().isEmpty())
            {
              m_comment_label->setText(share->comment());
            }
            else
            {
              m_comment_label->setText("-");
            }
            
            if (!share->isPrinter())
            {
              if (share->isMounted())
              {
                m_mounted_label->setText(i18n("yes"));
              }
              else
              {
                m_mounted_label->setText(i18n("no"));
              }
            }
            else
            {
              m_mounted_label->setText("-");
            }
            
            if (share->hasHostIpAddress())
            {
              m_ip_label->setText(share->hostIpAddress());
            }
            else
            {
              m_ip_label->setText("-");
            }
            
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
      case SharesView:
      {
        SharePtr share = item.staticCast<Smb4KShare>();
        
        if (share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0)
        {
          m_size_label->setText(i18n("%1 of %2 free (%3 used)",
                                 share->freeDiskSpaceString(),
                                 share->totalDiskSpaceString(),
                                 share->diskUsageString()));
        }
        else
        {
          m_size_label->setText(i18n("unknown"));
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

 
void Smb4KToolTip::show(const QPoint &pos)
{
  // Get the geometry of the screen where the cursor is
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  const QRect screenRect = QApplication::desktop()->screenGeometry(pos);
#else
  const QRect screenRect = QApplication::screenAt(pos)->geometry();
#endif
  
  // Adjust the size
  adjustSize();

  // The position where the tooltip is to be shown
  QPoint tooltipPos;  
  
  // Correct the position of the tooltip, so that it is completely 
  // shown.
  if (pos.x() + width() + 5 >= screenRect.x() + screenRect.width())
  {
    tooltipPos.setX(pos.x() - width() - 5);
  }
  else
  {
    tooltipPos.setX(pos.x() + 5);
  }
  
  if (pos.y() + height() + 5 >= screenRect.y() + screenRect.height())
  {
    tooltipPos.setY(pos.y() - height() - 5);
  }
  else
  {
    tooltipPos.setY(pos.y() + 5);
  }

  move(tooltipPos);
  setVisible(true);
  
  QTimer::singleShot(10000, this, SLOT(slotHideToolTip()));
}
 
 
void Smb4KToolTip::hide()
{
  setVisible(false);
}


void Smb4KToolTip::setupNetworkBrowserToolTip()
{
  // NOTE: If you change the layout here, adjust also the update function!
  
  m_tip_layout = new QHBoxLayout(this);
  m_tip_layout->setAlignment(Qt::AlignTop);
  m_info_layout = new QVBoxLayout();
  m_info_layout->setAlignment(Qt::AlignTop);

  // Set the icon
  QLabel *icon_label = new QLabel(this);
  icon_label->setPixmap(m_item->icon().pixmap(KIconLoader::SizeEnormous));
  
  m_tip_layout->addWidget(icon_label, Qt::AlignHCenter);
  m_tip_layout->addLayout(m_info_layout);
  
  // Use a brighter color for the left label. This was copied from
  // KFileMetaDataWidget class.
  QPalette p = palette();
  const QPalette::ColorRole role = foregroundRole();
  QColor textColor = p.color(role);
  textColor.setAlpha(128);
  p.setColor(role, textColor);
  
  // FIXME: Use smaller font for the information. Get the current 
  // point size of the window system with QFontInfo::pointSize().
  
  switch (m_item->type())
  {
    case Workgroup:
    {
      WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
      
      QLabel *caption = new QLabel(workgroup->workgroupName(), this);
      caption->setAlignment(Qt::AlignHCenter);
      QFont caption_font = caption->font();
      caption_font.setBold(true);
      caption->setFont(caption_font);
      
      m_info_layout->addWidget(caption);
      m_info_layout->addWidget(new KSeparator(this), Qt::AlignHCenter);
      
      m_text_layout = new QGridLayout();
      
      QLabel *type_label = new QLabel(i18n("Type"), this);
      type_label->setPalette(p);
      
      m_text_layout->addWidget(type_label, 0, 0, Qt::AlignRight);
      m_text_layout->addWidget(new QLabel(i18n("Workgroup"), this), 0, 1, 0);
      
      QLabel *mb_label = new QLabel(i18n("Master browser"), this);
      mb_label->setPalette(p);
      
      m_text_layout->addWidget(mb_label, 1, 0, Qt::AlignRight);
      
      if (workgroup->hasMasterBrowserIpAddress())
      {
        m_master_browser_label = new QLabel(workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIpAddress()+')', this);
        m_text_layout->addWidget(m_master_browser_label, 1, 1, 0);
      }
      else
      {
        m_master_browser_label = new QLabel(workgroup->masterBrowserName(), this);
        m_text_layout->addWidget(m_master_browser_label, 1, 1, 0);
      }
      
      m_info_layout->addLayout(m_text_layout);
      m_info_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
      break;
    }
    case Host:
    {
      HostPtr host = m_item.staticCast<Smb4KHost>();
      
      QLabel *caption = new QLabel(host->hostName(), this);
      caption->setAlignment(Qt::AlignHCenter);
      QFont caption_font = caption->font();
      caption_font.setBold(true);
      caption->setFont(caption_font);
      
      m_info_layout->addWidget(caption, Qt::AlignHCenter);
      m_info_layout->addWidget(new KSeparator(this), Qt::AlignHCenter);

      m_text_layout = new QGridLayout();
      
      QLabel *type_label = new QLabel(i18n("Type"), this);
      type_label->setPalette(p);
      
      m_text_layout->addWidget(type_label, 0, 0, Qt::AlignRight);
      m_text_layout->addWidget(new QLabel(i18n("Host"), this), 0, 1, 0);
      
      QLabel *co_label = new QLabel(i18n("Comment"), this);
      co_label->setPalette(p);
      
      m_text_layout->addWidget(co_label, 1, 0, Qt::AlignRight);
      
      if (!host->comment().isEmpty())
      {
        m_comment_label = new QLabel(host->comment(), this);
        m_text_layout->addWidget(m_comment_label, 1, 1, 0);
      }
      else
      {
        m_comment_label = new QLabel("-", this);
        m_text_layout->addWidget(m_comment_label, 1, 1, 0);
      }
      
      QLabel *ip_label = new QLabel(i18n("IP Address"), this);
      ip_label->setPalette(p);
      
      m_text_layout->addWidget(ip_label, 2, 0, Qt::AlignRight);
      
      if (host->hasIpAddress())
      {
        m_ip_label = new QLabel(host->ipAddress(), this);
        m_text_layout->addWidget(m_ip_label, 2, 1, 0);
      }
      else
      {
        m_ip_label = new QLabel("-", this);
        m_text_layout->addWidget(m_ip_label, 2, 1, 0);
      }
      
      QLabel *wg_label = new QLabel(i18n("Workgroup"), this);
      wg_label->setPalette(p);
      
      m_text_layout->addWidget(wg_label, 3, 0, Qt::AlignRight);
      m_text_layout->addWidget(new QLabel(host->workgroupName(), this), 3, 1, 0);
      
      m_info_layout->addLayout(m_text_layout);
      m_info_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
      break;
    }
    case Share:
    {
      SharePtr share = m_item.staticCast<Smb4KShare>();
      
      QLabel *caption = new QLabel(share->shareName(), this);
      caption->setAlignment(Qt::AlignHCenter);
      QFont caption_font = caption->font();
      caption_font.setBold(true);
      caption->setFont(caption_font);
      
      m_info_layout->addWidget(caption);
      m_info_layout->addWidget(new KSeparator(this), Qt::AlignHCenter);
      
      m_text_layout = new QGridLayout();
      
      QLabel *type_label = new QLabel(i18n("Type"), this);
      type_label->setPalette(p);
      
      m_text_layout->addWidget(type_label, 0, 0, Qt::AlignRight);
      m_text_layout->addWidget(new QLabel(i18n("Share (%1)", share->shareTypeString()), this), 0, 1, 0);
      
      QLabel *co_label = new QLabel(i18n("Comment"), this);
      co_label->setPalette(p);
      
      m_text_layout->addWidget(co_label, 1, 0, Qt::AlignRight);
      
      if (!share->comment().isEmpty())
      {
        m_comment_label = new QLabel(share->comment(), this);
        m_text_layout->addWidget(m_comment_label, 1, 1, 0);
      }
      else
      {
        m_comment_label = new QLabel("-", this);
        m_text_layout->addWidget(m_comment_label, 1, 1, 0);
      }
     
      QLabel *mnt_label = new QLabel(i18n("Mounted"), this);
      mnt_label->setPalette(p);
      
      m_text_layout->addWidget(mnt_label, 2, 0, Qt::AlignRight);
      
      if (!share->isPrinter())
      {
        if (share->isMounted())
        {
          m_mounted_label = new QLabel(i18n("yes"), this);
          m_text_layout->addWidget(m_mounted_label, 2, 1, 0);
        }
        else
        {
          m_mounted_label = new QLabel(i18n("no"), this);
          m_text_layout->addWidget(m_mounted_label, 2, 1, 0);
        }
      }
      else
      {
        m_mounted_label = new QLabel("-", this);
        m_text_layout->addWidget(m_mounted_label, 2, 1, 0);
      }
      
      QLabel *h_label = new QLabel(i18n("Host"), this);
      h_label->setPalette(p);
      
      m_text_layout->addWidget(h_label, 3, 0, Qt::AlignRight);
      m_text_layout->addWidget(new QLabel(share->hostName()), 3, 1, 0);

      QLabel *ip_label = new QLabel(i18n("IP Address"), this);
      ip_label->setPalette(p);
      
      m_text_layout->addWidget(ip_label, 4, 0, Qt::AlignRight);
      
      if (share->hasHostIpAddress())
      {
        m_ip_label = new QLabel(share->hostIpAddress(), this);
        m_text_layout->addWidget(m_ip_label, 4, 1, 0);
      }
      else
      {
        m_ip_label = new QLabel("-", this);
        m_text_layout->addWidget(m_ip_label, 4, 1, 0);
      }
      
      QLabel *unc_label = new QLabel(i18n("Location"), this);
      unc_label->setPalette(p);
      
      m_text_layout->addWidget(unc_label, 5, 0, Qt::AlignRight);
      m_text_layout->addWidget(new QLabel(share->displayString(), this), 5, 1, 0);
      
      m_info_layout->addLayout(m_text_layout);
      m_info_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
      break;
    }
    default:
    {
      return;
    }
  }
  
  adjustSize();
}


void Smb4KToolTip::setupSharesViewToolTip()
{
  // NOTE: If you change the layout here, adjust also the update function!
  
  SharePtr share = m_item.staticCast<Smb4KShare>();
  
  m_tip_layout = new QHBoxLayout(this);
  m_tip_layout->setAlignment(Qt::AlignTop);
  m_info_layout = new QVBoxLayout();
  m_info_layout->setAlignment(Qt::AlignTop);

  // Set the icon
  QLabel *icon_label = new QLabel(this);
  icon_label->setPixmap(share->icon().pixmap(KIconLoader::SizeEnormous));
  
  m_tip_layout->addWidget(icon_label, Qt::AlignHCenter);
  m_tip_layout->addLayout(m_info_layout);
  
  // Use a brighter color for the left label. This was copied from
  // KFileMetaDataWidget class.
  QPalette p = palette();
  const QPalette::ColorRole role = foregroundRole();
  QColor textColor = p.color(role);
  textColor.setAlpha(128);
  p.setColor(role, textColor);
  
  // FIXME: Use smaller font for the information. Get the current 
  // point size of the window system with QFontInfo::pointSize().
      
  QLabel *caption = new QLabel(share->shareName(), this);
  caption->setAlignment(Qt::AlignHCenter);
  QFont caption_font = caption->font();
  caption_font.setBold(true);
  caption->setFont(caption_font);
      
  m_info_layout->addWidget(caption);
  m_info_layout->addWidget(new KSeparator(this), Qt::AlignHCenter);
      
  m_text_layout = new QGridLayout();
      
  QLabel *unc_label = new QLabel(i18n("Location"), this);
  unc_label->setPalette(p);
      
  m_text_layout->addWidget(unc_label, 0, 0, Qt::AlignRight);
  m_text_layout->addWidget(new QLabel(share->displayString(), this), 0, 1, 0);
      
  QLabel *mp_label = new QLabel(i18n("Mountpoint"), this);
  mp_label->setPalette(p);
      
  m_text_layout->addWidget(mp_label, 1, 0, Qt::AlignRight);
  m_text_layout->addWidget(new QLabel(share->path(), this), 1, 1, 0);

  QLabel *log_label = new QLabel(i18n("Login"), this);
  log_label->setPalette(p);
      
  m_text_layout->addWidget(log_label, 2, 0, Qt::AlignRight);
      
  if (!share->login().isEmpty())
  {
    m_text_layout->addWidget(new QLabel(share->login(), this), 2, 1, 0);
  }
  else
  {
    m_text_layout->addWidget(new QLabel(i18n("unknown"), this), 2, 1, 0);
  }
  
  QLabel *own_label = new QLabel(i18n("Owner"), this);
  own_label->setPalette(p);
      
  m_text_layout->addWidget(own_label, 3, 0, Qt::AlignRight);
  
  QString owner = (!share->user().loginName().isEmpty() ? share->user().loginName() : i18n("unknown"));
  QString group = (!share->group().name().isEmpty() ? share->group().name() : i18n("unknown"));
  
  m_text_layout->addWidget(new QLabel(QString("%1 - %2")
                            .arg(owner).arg(group), this), 3, 1, 0);
      
  QLabel *fs_label = new QLabel(i18n("File system"), this);
  fs_label->setPalette(p);
  
  m_text_layout->addWidget(fs_label, 4, 0, Qt::AlignRight);
  m_text_layout->addWidget(new QLabel(share->fileSystemString()), 4, 1, 0);
  
  QLabel *s_label = new QLabel(i18n("Size"), this);
  s_label->setPalette(p);
  
  m_text_layout->addWidget(s_label, 5, 0, Qt::AlignRight);
  
  if (share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0)
  {
    m_size_label = new QLabel(i18n("%1 free of %2 (%3 used)",
                               share->freeDiskSpaceString(),
                               share->totalDiskSpaceString(),
                               share->diskUsageString()));
    m_text_layout->addWidget(m_size_label, 5, 1, 0);
  }
  else
  {
    m_size_label = new QLabel(i18n("unknown"));
    m_text_layout->addWidget(m_size_label, 5, 1, 0);
  }
      
  m_info_layout->addLayout(m_text_layout);
  m_info_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}


void Smb4KToolTip::updateNetworkBrowserToolTip()
{
  if (m_item && m_text_layout && m_tip_layout)
  {
    switch (m_item->type())
    {
      case Workgroup:
      {
        WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
        
        // Master browser name and IP address
        QLayoutItem *mb_item = m_text_layout->itemAtPosition(1, 1);
        QLabel *mb_label = static_cast<QLabel *>(mb_item->widget());
        
        if (mb_label)
        {
          if (!workgroup->hasMasterBrowserIpAddress())
          {
            mb_label->setText(workgroup->masterBrowserName()+" ("+workgroup->masterBrowserIpAddress()+')');
          }
          else
          {
            mb_label->setText(workgroup->masterBrowserName());
          }
        }
        
        break;
      }
      case Host:
      {
        HostPtr host = m_item.staticCast<Smb4KHost>();

        // Comment
        QLayoutItem *co_item = m_text_layout->itemAtPosition(1, 1);
        QLabel *co_label = static_cast<QLabel *>(co_item->widget());
        
        if (co_label)
        {
          if (!host->comment().isEmpty())
          {
            co_label->setText(host->comment());
          }
          else
          {
            co_label->setText("-");
          }
        }
      
        // IP address
        QLayoutItem *ip_item = m_text_layout->itemAtPosition(2, 1);
        QLabel *ip_label = static_cast<QLabel *>(ip_item->widget());
        
        if (ip_label)
        {
          if (host->hasIpAddress())
          {
            ip_label->setText(host->ipAddress());
          }
          else
          {
            ip_label->setText("-");
          }
        }
      
        break;
      }
      case Share:
      {
        SharePtr share = m_item.staticCast<Smb4KShare>();
        
        // Icon
        QLayoutItem *icon_item = m_tip_layout->itemAt(0);
        QLabel *icon_label = static_cast<QLabel *>(icon_item->widget());
        icon_label->setPixmap(m_item->icon().pixmap(KIconLoader::SizeEnormous));

        // Comment
        QLayoutItem *co_item = m_text_layout->itemAtPosition(1, 1);
        QLabel *co_label = static_cast<QLabel *>(co_item->widget());

        if (co_label)
        {
          if (!share->comment().isEmpty())
          {
            co_label->setText(share->comment());
          }
          else
          {
            co_label->setText("-");
          }
        }
     
        // Mounted indicator
        QLayoutItem *mnt_item = m_text_layout->itemAtPosition(2, 1);
        QLabel *mnt_label = static_cast<QLabel *>(mnt_item->widget());
        
        if (mnt_label)
        {
          if (!share->isPrinter())
          {
            if (share->isMounted())
            {
              mnt_label->setText(i18n("yes"));
            }
            else
            {
              mnt_label->setText(i18n("no"));
            }
          }
          else
          {
            mnt_label->setText("-");
          }
        }
        
        // The rest won't change while the tool tip is shown.
      
        break;
      }
      default:
      {
        break;
      }
    }
  }
}


void Smb4KToolTip::updateSharesViewToolTip()
{
  if (m_item && m_text_layout && m_tip_layout)
  {
    SharePtr share = m_item.staticCast<Smb4KShare>();
    
    // Set the icon
    QLayoutItem *icon_item = m_tip_layout->itemAt(0);
    QLabel *icon_label = static_cast<QLabel *>(icon_item->widget());
    icon_label->setPixmap(m_item->icon().pixmap(KIconLoader::SizeEnormous));
    
    QLayoutItem *log_item = m_text_layout->itemAtPosition(2, 1);
    QLabel *log_label = static_cast<QLabel *>(log_item->widget());
    
    if (!share->login().isEmpty())
    {
      log_label->setText(share->login());
    }
    else
    {
      log_label->setText(i18n("unknown"));
    }
    
    QLayoutItem *s_item = m_text_layout->itemAtPosition(5, 1);
    QLabel *s_label = static_cast<QLabel *>(s_item->widget());
  
    if (share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0)
    {
      s_label->setText(i18n("%1 free of %2 (%3 used)", 
                              share->freeDiskSpaceString(),
                              share->totalDiskSpaceString(),
                              share->diskUsageString()));
    }
    else
    {
      s_label->setText(i18n("unknown"));
    }
  }
  
  // The rest won't change while the tool tip is shown.
}


void Smb4KToolTip::paintEvent(QPaintEvent *e)
{
  // Copied from Dolphin's meta data tool tips.
  Q_UNUSED(e);

  QPainter painter(this);

  QColor toColor = palette().brush(QPalette::ToolTipBase).color();
  QColor fromColor = KColorScheme::shade(toColor, KColorScheme::LightShade, 0.2);

  const bool haveAlphaChannel = KWindowSystem::compositingActive();
  
  if (haveAlphaChannel)
  {
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(0.5, 0.5);
    toColor.setAlpha(220);
    fromColor.setAlpha(220);
  }

  QLinearGradient gradient(QPointF(0.0, 0.0), QPointF(0.0, height()));
  gradient.setColorAt(0.0, fromColor);
  gradient.setColorAt(1.0, toColor);
  painter.setPen(Qt::NoPen);
  painter.setBrush(gradient);

  const QRect rect(0, 0, width(), height());
    
  if (haveAlphaChannel) 
  {
    const qreal radius = 5.0;

    QPainterPath path;
    path.moveTo(rect.left(), rect.top() + radius);
    arc(path, rect.left() + radius, rect.top() + radius, radius, 180, -90);
    arc(path, rect.right() - radius, rect.top() + radius, radius, 90, -90);
    arc(path, rect.right() - radius, rect.bottom() - radius, radius, 0, -90);
    arc(path, rect.left() + radius, rect.bottom() - radius, radius, 270, -90);
    path.closeSubpath();

    painter.drawPath(path);
  } 
  else 
  {
    painter.drawRect(rect);
  }
}


void Smb4KToolTip::arc(QPainterPath& path,
                              qreal cx, qreal cy,
                              qreal radius, qreal angle,
                              qreal sweepLength)
{
  // Copied from Dolphin's meta data tool tips.
  path.arcTo(cx-radius, cy-radius, radius * 2, radius * 2, angle, sweepLength);
}


void Smb4KToolTip::slotHideToolTip()
{
  hide();
}

