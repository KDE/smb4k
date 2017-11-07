/***************************************************************************
    smb4knetworkoptions  -  The configuration page for the network
    settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2017 by Alexander Reinholdt
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
#include "smb4kconfigpagenetwork.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>


Smb4KConfigPageNetwork::Smb4KConfigPageNetwork(QWidget *parent)
: QTabWidget(parent)
{
  //
  // General Settings tab
  //
  QWidget *tab1 = new QWidget(this);
  QVBoxLayout *tab1_layout = new QVBoxLayout(tab1);
  tab1_layout->setSpacing(5);
  tab1_layout->setMargin(0);

  // The browse list group box.
  QGroupBox *browse_list_box = new QGroupBox(i18n("Browse List"), tab1);
  QGridLayout *browse_box_layout = new QGridLayout(browse_list_box);
  browse_box_layout->setSpacing(5);

  QButtonGroup *browse_list_buttons = new QButtonGroup(browse_list_box);

  QRadioButton *lookup_workgroups = new QRadioButton(Smb4KSettings::self()->lookupDomainsItem()->label(), browse_list_box);
  lookup_workgroups->setObjectName("kcfg_LookupDomains");

  QRadioButton *query_current = new QRadioButton(Smb4KSettings::self()->queryCurrentMasterItem()->label(), browse_list_box);
  query_current->setObjectName("kcfg_QueryCurrentMaster");

  QRadioButton *query_custom = new QRadioButton(Smb4KSettings::self()->queryCustomMasterItem()->label(), browse_list_box);
  query_custom->setObjectName("kcfg_QueryCustomMaster");

  KLineEdit *custom_name = new KLineEdit(browse_list_box);
  custom_name->setObjectName("kcfg_CustomMasterBrowser");

  browse_list_buttons->addButton(lookup_workgroups);
  browse_list_buttons->addButton(query_current);
  browse_list_buttons->addButton(query_custom);

  browse_box_layout->addWidget(lookup_workgroups, 0, 0, 1, 3, 0);
  browse_box_layout->addWidget(query_current, 1, 0, 1, 3, 0);
  browse_box_layout->addWidget(query_custom, 2, 0, 0);
  browse_box_layout->addWidget(custom_name, 2, 1, 1, 2, 0);

  // The authentication group box.
  QGroupBox *auth_box = new QGroupBox(i18n("Authentication"), tab1);
  QVBoxLayout *auth_box_layout = new QVBoxLayout(auth_box);
  auth_box_layout->setSpacing(5);

  QCheckBox *master_browser_auth = new QCheckBox(Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(), auth_box);
  master_browser_auth->setObjectName("kcfg_MasterBrowsersRequireAuth");

  auth_box_layout->addWidget(master_browser_auth, 0);
  
  // Behavior group box.
  QGroupBox *behavior_box = new QGroupBox(i18n("Behavior"), tab1);
  QGridLayout *behavior_layout = new QGridLayout(behavior_box);
  behavior_layout->setSpacing(5);
  
  QLabel *lookup_ips_label = new QLabel(Smb4KSettings::self()->lookupIPsItem()->label(), behavior_box);
  KComboBox *lookup_ips = new KComboBox(behavior_box);
  lookup_ips->setObjectName("kcfg_LookupIPs");
  lookup_ips->insertItem(Smb4KSettings::EnumLookupIPs::nmblookup,
                         Smb4KSettings::self()->lookupIPsItem()->choices().value(Smb4KSettings::EnumLookupIPs::nmblookup).label);
  lookup_ips->insertItem(Smb4KSettings::EnumLookupIPs::net,
                         Smb4KSettings::self()->lookupIPsItem()->choices().value(Smb4KSettings::EnumLookupIPs::net).label);
  lookup_ips_label->setBuddy(lookup_ips);
  
  QCheckBox *detect_printers = new QCheckBox(Smb4KSettings::self()->detectPrinterSharesItem()->label(), behavior_box);
  detect_printers->setObjectName("kcfg_DetectPrinterShares");

  QCheckBox *detect_hidden = new QCheckBox(Smb4KSettings::self()->detectHiddenSharesItem()->label(), behavior_box);
  detect_hidden->setObjectName("kcfg_DetectHiddenShares");  
  
  QCheckBox *preview_hidden = new QCheckBox(Smb4KSettings::self()->previewHiddenItemsItem()->label(), behavior_box);
  preview_hidden->setObjectName("kcfg_PreviewHiddenItems");
  
  behavior_layout->addWidget(lookup_ips_label, 0, 0, 0);
  behavior_layout->addWidget(lookup_ips, 0, 1, 0);
  behavior_layout->addWidget(detect_printers, 1, 0, 1, 2, 0);
  behavior_layout->addWidget(detect_hidden, 2, 0, 1, 2, 0);
  behavior_layout->addWidget(preview_hidden, 3, 0, 1, 2, 0);
  
  tab1_layout->addWidget(browse_list_box, 0);
  tab1_layout->addWidget(auth_box, 0);
  tab1_layout->addWidget(behavior_box, 0);
  tab1_layout->addStretch(100);
  
  addTab(tab1, i18n("General Settings"));
  
  //
  // Advanced Settings tab
  //
  QWidget *tab2 = new QWidget(this);
  QVBoxLayout *tab2_layout = new QVBoxLayout(tab2);
  tab2_layout->setSpacing(5);
  tab2_layout->setMargin(0);
  
  // Periodic scanning
  QGroupBox *periodic_box = new QGroupBox(i18n("Periodic Scanning"), tab2);
  QGridLayout *periodic_layout = new QGridLayout(periodic_box);
  periodic_layout->setSpacing(5);
  
  QCheckBox *periodic_scanning = new QCheckBox(Smb4KSettings::self()->periodicScanningItem()->label(), periodic_box);
  periodic_scanning->setObjectName("kcfg_PeriodicScanning");
  
  QLabel *interval_label = new QLabel(Smb4KSettings::self()->scanIntervalItem()->label(), periodic_box);
  interval_label->setIndent(25);
  
  QSpinBox *scan_interval = new QSpinBox(periodic_box);
  scan_interval->setObjectName("kcfg_ScanInterval");
  scan_interval->setSuffix(i18n(" min"));
  scan_interval->setSingleStep(1);
//   scan_interval->setSliderEnabled(true);
  
  interval_label->setBuddy(scan_interval);
  
  periodic_layout->addWidget(periodic_scanning, 0, 0, 1, 2, 0);
  periodic_layout->addWidget(interval_label, 1, 0, 0);
  periodic_layout->addWidget(scan_interval, 1, 1, 0);  
  
  
  // Wake-On-LAN
  QGroupBox *wol_box = new QGroupBox(i18n("Wake-On-LAN"), tab2);
  QGridLayout *wol_layout = new QGridLayout(wol_box);
  wol_layout->setSpacing(5);
  
  QCheckBox *enable_wol = new QCheckBox(Smb4KSettings::self()->enableWakeOnLANItem()->label(), wol_box);
  enable_wol->setObjectName("kcfg_EnableWakeOnLAN");
  
  QLabel *waiting_label = new QLabel(Smb4KSettings::self()->wakeOnLANWaitingTimeItem()->label(), wol_box);
  waiting_label->setIndent(25);
  
  QSpinBox *waiting_time = new QSpinBox(wol_box);
  waiting_time->setObjectName("kcfg_WakeOnLANWaitingTime");
  waiting_time->setSuffix(i18n(" s"));
  waiting_time->setSingleStep(1);
//   waiting_time->setSliderEnabled(true);
  
  waiting_label->setBuddy(waiting_time);
  
  QFrame *note = new QFrame(wol_box);
  QGridLayout *note_layout = new QGridLayout(note);
  note_layout->setSpacing(10);
  note_layout->setMargin(5);

  QLabel *important_pix = new QLabel(note);
  important_pix->setPixmap(KIconLoader::global()->loadIcon("emblem-important", KIconLoader::Desktop, KIconLoader::SizeMedium));
  important_pix->adjustSize();

  QLabel *message = new QLabel(note);
  message->setText(i18n("<qt>Define the hosts that should be woken up via the custom options dialog.</qt>"));
  message->setTextFormat(Qt::AutoText);
  message->setWordWrap(true);
  message->setAlignment(Qt::AlignJustify);

  note_layout->addWidget(important_pix, 0, 0, Qt::AlignCenter);
  note_layout->addWidget(message, 0, 1, Qt::AlignVCenter);

  note_layout->setColumnStretch(1, 1);  
  
  wol_layout->addWidget(enable_wol, 0, 0, 1, 2, 0);
  wol_layout->addWidget(waiting_label, 1, 0, 0);
  wol_layout->addWidget(waiting_time, 1, 1, 0);
  wol_layout->addWidget(note, 2, 0, 1, 2, 0);
  
  tab2_layout->addWidget(periodic_box, 0);
  tab2_layout->addWidget(wol_box, 0);
  tab2_layout->addStretch(100);
  
  addTab(tab2, i18n("Advanced Settings"));
}


Smb4KConfigPageNetwork::~Smb4KConfigPageNetwork()
{
}

