/***************************************************************************
    This configuration page takes care of all settings concerning the 
    user interface
                             -------------------
    begin                : Mi Aug 30 2006
    copyright            : (C) 2006-2019 by Alexander Reinholdt
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
#include "smb4kconfigpageuserinterface.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KCompletion/KComboBox>


Smb4KConfigPageUserInterface::Smb4KConfigPageUserInterface(QWidget *parent)
: QWidget(parent)
{
  //
  // Layout
  // 
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  layout->setMargin(0);
  
  // 
  // Main Window settings
  // 
  QGroupBox *mainWindowBox = new QGroupBox(i18n("Main Window"), this);
  QGridLayout *mainWindowBoxLayout = new QGridLayout(mainWindowBox);
  mainWindowBoxLayout->setSpacing(5);
  
  QLabel *tabOrientationLabel = new QLabel(Smb4KSettings::self()->mainWindowTabOrientationItem()->label(), mainWindowBox);
  
  KComboBox *tabOrientation = new KComboBox(mainWindowBox);
  tabOrientation->setObjectName("kcfg_MainWindowTabOrientation");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> tabOrientationChoices = Smb4KSettings::self()->mainWindowTabOrientationItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &to : tabOrientationChoices)
  {
    tabOrientation->addItem(to.label);
  }
  
  tabOrientationLabel->setBuddy(tabOrientation);
  
  mainWindowBoxLayout->addWidget(tabOrientationLabel, 0, 0, 0);
  mainWindowBoxLayout->addWidget(tabOrientation, 0, 1, 0);
  
  QCheckBox *showBookmarkLabel = new QCheckBox(Smb4KSettings::self()->showCustomBookmarkLabelItem()->label(), mainWindowBox);
  showBookmarkLabel->setObjectName("kcfg_ShowCustomBookmarkLabel");
  
  mainWindowBoxLayout->addWidget(showBookmarkLabel, 1, 0, 1, 2, 0);

  layout->addWidget(mainWindowBox, 0);
  
  //
  // Network Neighborhood settings
  // 
  QGroupBox *networkNeighhoodBox = new QGroupBox(i18n("Network Neighborhood"), this);
  QGridLayout *networkNeighhoodBoxLayout = new QGridLayout(networkNeighhoodBox);
  networkNeighhoodBoxLayout->setSpacing(5);
  
  QCheckBox *autoExpand = new QCheckBox(Smb4KSettings::self()->autoExpandNetworkItemsItem()->label(), networkNeighhoodBox);
  autoExpand->setObjectName("kcfg_AutoExpandNetworkItems");
  
  networkNeighhoodBoxLayout->addWidget(autoExpand, 0, 0, 0);
  
  QCheckBox *showType = new QCheckBox(Smb4KSettings::self()->showTypeItem()->label(), networkNeighhoodBox);
  showType->setObjectName("kcfg_ShowType");
  
  networkNeighhoodBoxLayout->addWidget(showType, 0, 1, 0);
  
  QCheckBox *showIpAddress = new QCheckBox(Smb4KSettings::self()->showIPAddressItem()->label(), networkNeighhoodBox);
  showIpAddress->setObjectName("kcfg_ShowIPAddress");
  
  networkNeighhoodBoxLayout->addWidget(showIpAddress, 1, 0, 0);
  
  QCheckBox *showComment = new QCheckBox(Smb4KSettings::self()->showCommentItem()->label(), networkNeighhoodBox);
  showComment->setObjectName("kcfg_ShowComment");
  
  networkNeighhoodBoxLayout->addWidget(showComment, 1, 1, 0);
  
  QCheckBox *showNetworkTooltip = new QCheckBox(Smb4KSettings::self()->showNetworkItemToolTipItem()->label(), networkNeighhoodBox);
  showNetworkTooltip->setObjectName("kcfg_ShowNetworkItemToolTip");
  
  networkNeighhoodBoxLayout->addWidget(showNetworkTooltip, 2, 0, 1, 2, 0);
  
  layout->addWidget(networkNeighhoodBox, 0);
  
  //
  // Shares View settings
  // 
  QGroupBox *sharesViewBox = new QGroupBox(i18n("Shares View"), this);
  QGridLayout *sharesViewBoxLayout = new QGridLayout(sharesViewBox);
  sharesViewBoxLayout->setSpacing(5);
  
  QLabel *viewModeLabel = new QLabel(Smb4KSettings::self()->sharesViewModeItem()->label(), sharesViewBox);
  
  KComboBox *viewMode = new KComboBox(sharesViewBox);
  viewMode->setObjectName("kcfg_SharesViewMode");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> sharesViewModeChoices = Smb4KSettings::self()->sharesViewModeItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &vm : sharesViewModeChoices)
  {
    viewMode->addItem(vm.label);
  }

  viewModeLabel->setBuddy(viewMode);

  sharesViewBoxLayout->addWidget(viewModeLabel, 0, 0, 0);
  sharesViewBoxLayout->addWidget(viewMode, 0, 1, 0);
  
  QCheckBox *showShareTooltip = new QCheckBox(Smb4KSettings::self()->showShareToolTipItem()->label(), sharesViewBox);
  showShareTooltip->setObjectName("kcfg_ShowShareToolTip");
  
  sharesViewBoxLayout->addWidget(showShareTooltip, 1, 0, 1, 2, 0);
  
  layout->addWidget(sharesViewBox, 0);
  layout->addStretch(100);
  


//   // Network item tooltips
//   QGroupBox *network_tooltips_box = new QGroupBox(i18n("Tooltips"), networkNeighborhoodTab);
// 
//   QGridLayout *n_tooltips_layout = new QGridLayout(network_tooltips_box);
//   n_tooltips_layout->setSpacing(5);
// 

// 
//   n_tooltips_layout->addWidget(network_tooltip, 0, 0, 0);
// 
//   networkNeighborhoodTabLayout->addWidget(behavior_box);
//   networkNeighborhoodTabLayout->addWidget(columns_box);
//   networkNeighborhoodTabLayout->addWidget(network_tooltips_box);
//   networkNeighborhoodTabLayout->addStretch(100);
// 
//   addTab(networkNeighborhoodTab, i18n("Network Neighborhood"));
// 
//   //
//   // Shares view
//   //
//   QWidget *shares_view_tab = new QWidget(this);
// 
//   QVBoxLayout *shares_view_layout = new QVBoxLayout(shares_view_tab);
//   shares_view_layout->setSpacing(5);
//   shares_view_layout->setMargin(0);
//   
//   // View
//   QGroupBox *viewBox = new QGroupBox(i18n("View Mode"), shares_view_tab);
//   QHBoxLayout *viewBoxLayout = new QHBoxLayout(viewBox);
//   viewBoxLayout->setSpacing(5);
//   
//   QLabel *viewModeLabel = new QLabel(Smb4KSettings::self()->sharesViewModeItem()->label(), viewBox);
//   KComboBox *viewMode = new KComboBox(viewBox);
//   viewMode->setObjectName("kcfg_SharesViewMode");
//   viewMode->insertItem(Smb4KSettings::EnumSharesViewMode::IconView,
//                        Smb4KSettings::self()->sharesViewModeItem()->choices().value(Smb4KSettings::EnumSharesViewMode::IconView).label);
//   viewMode->insertItem(Smb4KSettings::EnumSharesViewMode::ListView,
//                        Smb4KSettings::self()->sharesViewModeItem()->choices().value(Smb4KSettings::EnumSharesViewMode::ListView).label);  
//   viewModeLabel->setBuddy(viewMode);
//   
//   viewBoxLayout->addWidget(viewModeLabel);
//   viewBoxLayout->addWidget(viewMode);
// 
//   // Share tooltips
//   QGroupBox *share_tooltips_box = new QGroupBox(i18n("Tooltips"), shares_view_tab);
// 
//   QGridLayout *s_tooltips_layout = new QGridLayout(share_tooltips_box);
//   s_tooltips_layout->setSpacing(5);
// 
//   QCheckBox *show_share_tooltip = new QCheckBox(Smb4KSettings::self()->showShareToolTipItem()->label(), share_tooltips_box);
//   show_share_tooltip->setObjectName("kcfg_ShowShareToolTip");
// 
//   s_tooltips_layout->addWidget(show_share_tooltip, 0, 0, 0);
// 
//   QSpacerItem *spacer4 = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
// 
//   shares_view_layout->addWidget(viewBox);
//   shares_view_layout->addWidget(share_tooltips_box);
//   shares_view_layout->addItem(spacer4);
// 
//   addTab(shares_view_tab, i18n("Mounted Shares"));
}


Smb4KConfigPageUserInterface::~Smb4KConfigPageUserInterface()
{
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


