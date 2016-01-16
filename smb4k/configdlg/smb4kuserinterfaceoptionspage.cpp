/***************************************************************************
    smb4kuserinterfaceoptions  -  This configuration page takes care
    of all settings concerning the user interface of Smb4K
                             -------------------
    begin                : Mi Aug 30 2006
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
#include "smb4kuserinterfaceoptionspage.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>

// KDE includes
#include <KI18n/KLocalizedString>


Smb4KUserInterfaceOptionsPage::Smb4KUserInterfaceOptionsPage(QWidget *parent)
: QTabWidget(parent)
{
  //
  // Network browser
  //
  QWidget *network_browser_tab = new QWidget(this);

  QVBoxLayout *net_browser_layout = new QVBoxLayout(network_browser_tab);
  net_browser_layout->setSpacing(5);
  net_browser_layout->setMargin(0);
  
  // Behavior
  QGroupBox *behavior_box = new QGroupBox(i18n("Behavior"), network_browser_tab);
  
  QGridLayout *behavior_layout = new QGridLayout(behavior_box);
  behavior_layout->setSpacing(5);
  
  QCheckBox *auto_open = new QCheckBox(Smb4KSettings::self()->autoExpandNetworkItemsItem()->label(), behavior_box);
  auto_open->setObjectName("kcfg_AutoExpandNetworkItems");
  
  behavior_layout->addWidget(auto_open, 0, 0, 0);

  // Columns
  QGroupBox *columns_box = new QGroupBox(i18n("Columns"), network_browser_tab);

  QGridLayout *columns_layout = new QGridLayout(columns_box);
  columns_layout->setSpacing(5);

  QCheckBox *show_type = new QCheckBox(Smb4KSettings::self()->showTypeItem()->label(), columns_box);
  show_type->setObjectName("kcfg_ShowType");

  QCheckBox *show_ip_address = new QCheckBox(Smb4KSettings::self()->showIPAddressItem()->label(), columns_box);
  show_ip_address->setObjectName("kcfg_ShowIPAddress");

  QCheckBox *show_comment = new QCheckBox(Smb4KSettings::self()->showCommentItem()->label(), columns_box);
  show_comment->setObjectName("kcfg_ShowComment");

  columns_layout->addWidget(show_type, 0, 0, 0);
  columns_layout->addWidget(show_ip_address, 0, 1, 0);
  columns_layout->addWidget(show_comment, 1, 0, 0);

  // Network item tooltips
  QGroupBox *network_tooltips_box = new QGroupBox(i18n("Tooltips"), network_browser_tab);

  QGridLayout *n_tooltips_layout = new QGridLayout(network_tooltips_box);
  n_tooltips_layout->setSpacing(5);

  QCheckBox *network_tooltip = new QCheckBox(Smb4KSettings::self()->showNetworkItemToolTipItem()->label(), network_tooltips_box);
  network_tooltip->setObjectName("kcfg_ShowNetworkItemToolTip");

  n_tooltips_layout->addWidget(network_tooltip, 0, 0, 0);

  net_browser_layout->addWidget(behavior_box);
  net_browser_layout->addWidget(columns_box);
  net_browser_layout->addWidget(network_tooltips_box);
  net_browser_layout->addStretch(100);

  addTab(network_browser_tab, i18n("Network Neighborhood"));

  //
  // Shares view
  //
  QWidget *shares_view_tab = new QWidget(this);

  QGridLayout *shares_view_layout = new QGridLayout(shares_view_tab);
  shares_view_layout->setSpacing(5);
  shares_view_layout->setMargin(0);

  // Share tooltips
  QGroupBox *share_tooltips_box = new QGroupBox(i18n("Tooltips"), shares_view_tab);

  QGridLayout *s_tooltips_layout = new QGridLayout(share_tooltips_box);
  s_tooltips_layout->setSpacing(5);

  QCheckBox *show_share_tooltip = new QCheckBox(Smb4KSettings::self()->showShareToolTipItem()->label(), share_tooltips_box);
  show_share_tooltip->setObjectName("kcfg_ShowShareToolTip");

  s_tooltips_layout->addWidget(show_share_tooltip, 0, 0, 0);

  QSpacerItem *spacer4 = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

  shares_view_layout->addWidget(share_tooltips_box, 0, 0, 0);
  shares_view_layout->addItem(spacer4, 1, 0);

  addTab(shares_view_tab, i18n("Mounted Shares"));
  
  //
  // Miscellaneous
  //
  QWidget *misc_tab = new QWidget(this);

  QVBoxLayout *misc_layout = new QVBoxLayout(misc_tab);
  misc_layout->setSpacing(5);
  misc_layout->setMargin(0);
  
  // Bookmarks
  QGroupBox *bookmarks_box = new QGroupBox(i18n("Bookmarks"), misc_tab);

  QGridLayout *bookmarks_layout = new QGridLayout(bookmarks_box);
  bookmarks_layout->setSpacing(5);

  QCheckBox *show_bookmark_label = new QCheckBox(Smb4KSettings::self()->showCustomBookmarkLabelItem()->label(), bookmarks_box);
  show_bookmark_label->setObjectName("kcfg_ShowCustomBookmarkLabel");

  bookmarks_layout->addWidget(show_bookmark_label, 0, 0, 0);

  misc_layout->addWidget(bookmarks_box);
  misc_layout->addStretch(100);

  addTab(misc_tab, i18n("Miscellaneous Settings"));
}


Smb4KUserInterfaceOptionsPage::~Smb4KUserInterfaceOptionsPage()
{
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


