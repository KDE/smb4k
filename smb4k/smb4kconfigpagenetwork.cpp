/***************************************************************************
    smb4knetworkoptions  -  The configuration page for the network
    settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2019 by Alexander Reinholdt
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
#include "smb4kconfigpagenetwork.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>


Smb4KConfigPageNetwork::Smb4KConfigPageNetwork(QWidget *parent) : QTabWidget(parent)
{
  //
  // Basic Samba Settings
  // 
  QWidget *commonSambaTab = new QWidget(this);
  QVBoxLayout *commonSambaTabLayout = new QVBoxLayout(commonSambaTab);
  
  //
  // Basic Settings group box
  // 
  QGroupBox *basicSettingsBox = new QGroupBox(i18n("Basic Settings"), commonSambaTab);
  QGridLayout *basicSettingsBoxLayout = new QGridLayout(basicSettingsBox);
  
  QLabel *nebiosNameLabel = new QLabel(Smb4KSettings::self()->netBIOSNameItem()->label(), basicSettingsBox);
  KLineEdit *netbiosName = new KLineEdit(basicSettingsBox);
  netbiosName->setObjectName("kcfg_NetBIOSName");
  netbiosName->setClearButtonEnabled(true);
  nebiosNameLabel->setBuddy(netbiosName);

  QLabel *domainLabel = new QLabel(Smb4KSettings::self()->domainNameItem()->label(), basicSettingsBox);
  KLineEdit *domain = new KLineEdit(basicSettingsBox);
  domain->setObjectName("kcfg_DomainName");
  domain->setClearButtonEnabled(true);
  domainLabel->setBuddy(domain);
  
  QCheckBox *useRemoteSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), basicSettingsBox);
  useRemoteSmbPort->setObjectName("kcfg_UseRemoteSmbPort");

  QSpinBox *remoteSmbPort = new QSpinBox(basicSettingsBox);
  remoteSmbPort->setObjectName("kcfg_RemoteSmbPort");
//   remoteSmbPort->setSliderEnabled(true);
  
  QCheckBox *largeNetworkNeighborhood = new QCheckBox(Smb4KSettings::self()->largeNetworkNeighborhoodItem()->label(), basicSettingsBox);
  largeNetworkNeighborhood->setObjectName("kcfg_LargeNetworkNeighborhood");

  basicSettingsBoxLayout->addWidget(nebiosNameLabel, 0, 0, 0);
  basicSettingsBoxLayout->addWidget(netbiosName, 0, 1, 0);
  basicSettingsBoxLayout->addWidget(domainLabel, 1, 0, 0);
  basicSettingsBoxLayout->addWidget(domain, 1, 1, 0);
  basicSettingsBoxLayout->addWidget(useRemoteSmbPort, 2, 0, 0);
  basicSettingsBoxLayout->addWidget(remoteSmbPort, 2, 1, 0);
  basicSettingsBoxLayout->addWidget(largeNetworkNeighborhood, 3, 0, 1, 2, 0);
  
  //
  // Behavior group box
  // 
  QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), commonSambaTab);
  QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);
  
  QCheckBox *detectPrinters = new QCheckBox(Smb4KSettings::self()->detectPrinterSharesItem()->label(), behaviorBox);
  detectPrinters->setObjectName("kcfg_DetectPrinterShares");

  QCheckBox *detectHiddenShares = new QCheckBox(Smb4KSettings::self()->detectHiddenSharesItem()->label(), behaviorBox);
  detectHiddenShares->setObjectName("kcfg_DetectHiddenShares");  
  
  QCheckBox *previewHiddenItems = new QCheckBox(Smb4KSettings::self()->previewHiddenItemsItem()->label(), behaviorBox);
  previewHiddenItems->setObjectName("kcfg_PreviewHiddenItems");
  
  behaviorBoxLayout->addWidget(detectPrinters, 0, 0, 0);
  behaviorBoxLayout->addWidget(detectHiddenShares, 0, 1, 0);
  behaviorBoxLayout->addWidget(previewHiddenItems, 1, 0, 0);
  
  commonSambaTabLayout->addWidget(basicSettingsBox, 0);
  commonSambaTabLayout->addWidget(behaviorBox, 0);
  commonSambaTabLayout->addStretch(100);
  
  addTab(commonSambaTab, i18n("Common Samba Settings"));
  
  //
  // Advanced Samba Settings
  // 
  QWidget *advancedSambaTab = new QWidget(this);
  QVBoxLayout *advancedSambaTabLayout = new QVBoxLayout(advancedSambaTab);
  
  //
  // Protocol group box
  // 
  QGroupBox *protocolsBox = new QGroupBox(i18n("Protocols"), basicSettingsBox);
  QVBoxLayout *protocolsBoxLayout = new QVBoxLayout(protocolsBox);
  
  QCheckBox *forceSMB1Protocol = new QCheckBox(Smb4KSettings::self()->forceSMB1ProtocolItem()->label(), protocolsBox);
  forceSMB1Protocol->setObjectName("kcfg_ForceSMB1Protocol");
  
  protocolsBoxLayout->addWidget(forceSMB1Protocol, 0);
  
  //
  // Authentication group box
  // 
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), advancedSambaTab);
  QGridLayout *authenticationBoxLayout = new QGridLayout(authenticationBox);

  QCheckBox *masterBrowsersRequireAuth = new QCheckBox(Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(), authenticationBox);
  masterBrowsersRequireAuth->setObjectName("kcfg_MasterBrowsersRequireAuth");
  
  QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
  useKerberos->setObjectName("kcfg_UseKerberos");

  QCheckBox *useCCache = new QCheckBox(Smb4KSettings::self()->useWinbindCCacheItem()->label(), authenticationBox);
  useCCache->setObjectName("kcfg_UseWinbindCCache");

  authenticationBoxLayout->addWidget(masterBrowsersRequireAuth, 0, 0, 0);
  authenticationBoxLayout->addWidget(useKerberos, 0, 1, 0);
  authenticationBoxLayout->addWidget(useCCache, 1, 0, 0);
  
  //
  // Security group box
  // 
  QGroupBox *securityBox = new QGroupBox(i18n("Security"), advancedSambaTab);
  QGridLayout *securityBoxLayout = new QGridLayout(securityBox);
  securityBoxLayout->setSpacing(5);
  
  // Encryption level
  QCheckBox *useEncryptionLevel = new QCheckBox(Smb4KSettings::self()->useEncryptionLevelItem()->label(), securityBox);
  useEncryptionLevel->setObjectName("kcfg_UseEncryptionLevel");
  
  KComboBox *encryptionLevel = new KComboBox(securityBox);
  encryptionLevel->setObjectName("kcfg_EncryptionLevel");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> encryptionLevelChoices = Smb4KSettings::self()->encryptionLevelItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &c : encryptionLevelChoices)
  {
    encryptionLevel->addItem(c.label);
  }

  securityBoxLayout->addWidget(useEncryptionLevel, 0, 0, 0);
  securityBoxLayout->addWidget(encryptionLevel, 0, 1, 0);
  
  advancedSambaTabLayout->addWidget(protocolsBox, 0);
  advancedSambaTabLayout->addWidget(authenticationBox, 0);
  advancedSambaTabLayout->addWidget(securityBox, 0);
  advancedSambaTabLayout->addStretch(100);
  
  addTab(advancedSambaTab, i18n("Advanced Samba Settings"));
  
  //
  // Wake-On-LAN tab
  //
  QWidget *wakeOnLanTab = new QWidget(this);
  QVBoxLayout *wakeOnLanTabLayout = new QVBoxLayout(wakeOnLanTab);
  
  // 
  // Wake-On-LAN group box
  // 
  QGroupBox *wakeOnLanBox = new QGroupBox(i18n("Wake-On-LAN"), wakeOnLanTab);
  QGridLayout *wakeOnLanBoxLayout = new QGridLayout(wakeOnLanBox);
  
  QCheckBox *enableWakeOnLan = new QCheckBox(Smb4KSettings::self()->enableWakeOnLANItem()->label(), wakeOnLanBox);
  enableWakeOnLan->setObjectName("kcfg_EnableWakeOnLAN");
  
  QLabel *wakeOnLanWaitingTimeLabel = new QLabel(Smb4KSettings::self()->wakeOnLANWaitingTimeItem()->label(), wakeOnLanBox);
  wakeOnLanWaitingTimeLabel->setIndent(25);
  
  QSpinBox *wakeOnLanWaitingTime = new QSpinBox(wakeOnLanBox);
  wakeOnLanWaitingTime->setObjectName("kcfg_WakeOnLANWaitingTime");
  wakeOnLanWaitingTime->setSuffix(i18n(" s"));
  wakeOnLanWaitingTime->setSingleStep(1);
//   wakeOnLanWaitingTime->setSliderEnabled(true);
  
  wakeOnLanWaitingTimeLabel->setBuddy(wakeOnLanWaitingTime);
  
  QFrame *wakeOnLanNote = new QFrame(wakeOnLanBox);
  QGridLayout *wakeOnLanNoteLayout = new QGridLayout(wakeOnLanNote);
  wakeOnLanNoteLayout->setSpacing(10);
  wakeOnLanNoteLayout->setMargin(5);

  QLabel *importantPixmap = new QLabel(wakeOnLanNote);
  importantPixmap->setPixmap(KIconLoader::global()->loadIcon("emblem-important", KIconLoader::Desktop, KIconLoader::SizeMedium));
  importantPixmap->adjustSize();

  QLabel *message = new QLabel(wakeOnLanNote);
  message->setText(i18n("<qt>Define the hosts that should be woken up via the custom options dialog.</qt>"));
  message->setTextFormat(Qt::AutoText);
  message->setWordWrap(true);
  message->setAlignment(Qt::AlignJustify);

  wakeOnLanNoteLayout->addWidget(importantPixmap, 0, 0, Qt::AlignVCenter);
  wakeOnLanNoteLayout->addWidget(message, 0, 1, Qt::AlignVCenter);
  wakeOnLanNoteLayout->setColumnStretch(1, 1);
  
  wakeOnLanBoxLayout->addWidget(enableWakeOnLan, 0, 0, 1, 2, 0);
  wakeOnLanBoxLayout->addWidget(wakeOnLanWaitingTimeLabel, 1, 0, 0);
  wakeOnLanBoxLayout->addWidget(wakeOnLanWaitingTime, 1, 1, 0);
  wakeOnLanBoxLayout->addWidget(wakeOnLanNote, 2, 0, 1, 2, 0);
  
  wakeOnLanTabLayout->addWidget(wakeOnLanBox, 0);
  wakeOnLanTabLayout->addStretch(100);
  
  addTab(wakeOnLanTab, i18n("Wake-On-LAN"));
}


Smb4KConfigPageNetwork::~Smb4KConfigPageNetwork()
{
}

