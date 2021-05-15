/***************************************************************************
    smb4knetworkoptions  -  The configuration page for the network
    settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2020 by Alexander Reinholdt
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
  QWidget *basicTab = new QWidget(this);
  QVBoxLayout *basicTabLayout = new QVBoxLayout(basicTab);
  
  //
  // Browse Settings group box
  // 
  QGroupBox *browseSettingsBox = new QGroupBox(i18n("Browse Settings"), basicTab);
  QGridLayout *browseSettingsBoxLayout = new QGridLayout(browseSettingsBox);

#ifdef USE_WS_DISCOVERY
  // Use WS-Discovery
  QCheckBox *useWsDiscovery = new QCheckBox(Smb4KSettings::self()->useWsDiscoveryItem()->label(), browseSettingsBox);
  useWsDiscovery->setObjectName("kcfg_UseWsDiscovery");
#endif
  
  // Use DNS-SD
  QCheckBox *useDnsServiceDiscovery = new QCheckBox(Smb4KSettings::self()->useDnsServiceDiscoveryItem()->label(), browseSettingsBox);
  useDnsServiceDiscovery->setObjectName("kcfg_UseDnsServiceDiscovery");
  
#ifdef USE_SMBC_PROTOCOL
  // Force SMBv1 protocol for browsing
  QCheckBox *forceSmb1Protocol = new QCheckBox(Smb4KSettings::self()->forceSmb1ProtocolItem()->label(), browseSettingsBox);
  forceSmb1Protocol->setObjectName("kcfg_ForceSmb1Protocol");
  
  // Set client protocol versions
  QCheckBox *useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), browseSettingsBox);
  useClientProtocolVersions->setObjectName("kcfg_UseClientProtocolVersions");
  
  QLabel *minimalProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), browseSettingsBox);
  minimalProtocolVersionLabel->setIndent(25);
  minimalProtocolVersionLabel->setObjectName("MinimalProtocolVersionLabel");
  
  KComboBox *minimalProtocolVersion = new KComboBox(browseSettingsBox);
  minimalProtocolVersion->setObjectName("kcfg_MinimalClientProtocolVersion");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &c : minimalProtocolVersionChoices)
  {
    minimalProtocolVersion->addItem(c.label);
  }
  
  QLabel *maximalProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), browseSettingsBox);
  maximalProtocolVersionLabel->setIndent(25);
  maximalProtocolVersionLabel->setObjectName("MaximalProtocolVersionLabel");
  
  KComboBox *maximalProtocolVersion = new KComboBox(browseSettingsBox);
  maximalProtocolVersion->setObjectName("kcfg_MaximalClientProtocolVersion");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &c : maximalProtocolVersionChoices)
  {
    maximalProtocolVersion->addItem(c.label);
  }

  minimalProtocolVersionLabel->setBuddy(minimalProtocolVersion);
  maximalProtocolVersionLabel->setBuddy(maximalProtocolVersion);
#endif
  
#ifdef USE_WS_DISCOVERY
  browseSettingsBoxLayout->addWidget(useWsDiscovery, 0, 0, 1, 2);
  browseSettingsBoxLayout->addWidget(useDnsServiceDiscovery, 1, 0, 1, 2);
#ifdef USE_SMBC_PROTOCOL
  browseSettingsBoxLayout->addWidget(forceSmb1Protocol, 2, 0, 1, 2);
  browseSettingsBoxLayout->addWidget(useClientProtocolVersions, 3, 0, 1, 2);
  browseSettingsBoxLayout->addWidget(minimalProtocolVersionLabel, 4, 0);
  browseSettingsBoxLayout->addWidget(minimalProtocolVersion, 4, 1);
  browseSettingsBoxLayout->addWidget(maximalProtocolVersionLabel, 5, 0);
  browseSettingsBoxLayout->addWidget(maximalProtocolVersion, 5, 1);
#endif
#else
  browseSettingsBoxLayout->addWidget(useDnsServiceDiscovery, 0, 0, 1, 2);
#ifdef USE_SMBC_PROTOCOL
  browseSettingsBoxLayout->addWidget(forceSmb1Protocol, 1, 0, 1, 2);
  browseSettingsBoxLayout->addWidget(useClientProtocolVersions, 2, 0, 1, 2);
  browseSettingsBoxLayout->addWidget(minimalProtocolVersionLabel, 3, 0);
  browseSettingsBoxLayout->addWidget(minimalProtocolVersion, 3, 1);
  browseSettingsBoxLayout->addWidget(maximalProtocolVersionLabel, 4, 0);
  browseSettingsBoxLayout->addWidget(maximalProtocolVersion, 4, 1);
#endif
#endif
  
  //
  // Behavior group box
  // 
  QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), basicTab);
  QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);
  
  QCheckBox *detectPrinters = new QCheckBox(Smb4KSettings::self()->detectPrinterSharesItem()->label(), behaviorBox);
  detectPrinters->setObjectName("kcfg_DetectPrinterShares");

  QCheckBox *detectHiddenShares = new QCheckBox(Smb4KSettings::self()->detectHiddenSharesItem()->label(), behaviorBox);
  detectHiddenShares->setObjectName("kcfg_DetectHiddenShares");  
  
  QCheckBox *previewHiddenItems = new QCheckBox(Smb4KSettings::self()->previewHiddenItemsItem()->label(), behaviorBox);
  previewHiddenItems->setObjectName("kcfg_PreviewHiddenItems");
  
  behaviorBoxLayout->addWidget(detectPrinters, 0, 0);
  behaviorBoxLayout->addWidget(detectHiddenShares, 0, 1);
  behaviorBoxLayout->addWidget(previewHiddenItems, 1, 0);
  
  basicTabLayout->addWidget(browseSettingsBox, 0);
  basicTabLayout->addWidget(behaviorBox, 0);
  basicTabLayout->addStretch(100);
  
  addTab(basicTab, i18n("Basic Settings"));
  
  //
  // Samba Settings
  // 
  QWidget *sambaTab = new QWidget(this);
  QVBoxLayout *sambaTabLayout = new QVBoxLayout(sambaTab);
  
  //
  // Basic Settings group box
  // 
  QGroupBox *basicSettingsBox = new QGroupBox(i18n("Common Settings"), sambaTab);
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

  basicSettingsBoxLayout->addWidget(nebiosNameLabel, 0, 0);
  basicSettingsBoxLayout->addWidget(netbiosName, 0, 1);
  basicSettingsBoxLayout->addWidget(domainLabel, 1, 0);
  basicSettingsBoxLayout->addWidget(domain, 1, 1);
  basicSettingsBoxLayout->addWidget(useRemoteSmbPort, 2, 0);
  basicSettingsBoxLayout->addWidget(remoteSmbPort, 2, 1);
  basicSettingsBoxLayout->addWidget(largeNetworkNeighborhood, 3, 0, 1, 2);
  
  //
  // Authentication group box
  // 
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
  QGridLayout *authenticationBoxLayout = new QGridLayout(authenticationBox);

  QCheckBox *masterBrowsersRequireAuth = new QCheckBox(Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(), authenticationBox);
  masterBrowsersRequireAuth->setObjectName("kcfg_MasterBrowsersRequireAuth");
  
  QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
  useKerberos->setObjectName("kcfg_UseKerberos");

  QCheckBox *useCCache = new QCheckBox(Smb4KSettings::self()->useWinbindCCacheItem()->label(), authenticationBox);
  useCCache->setObjectName("kcfg_UseWinbindCCache");

  authenticationBoxLayout->addWidget(masterBrowsersRequireAuth, 0, 0);
  authenticationBoxLayout->addWidget(useKerberos, 0, 1);
  authenticationBoxLayout->addWidget(useCCache, 1, 0);
  
  //
  // Security group box
  // 
  QGroupBox *securityBox = new QGroupBox(i18n("Security"), sambaTab);
  QGridLayout *securityBoxLayout = new QGridLayout(securityBox);
  
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

  securityBoxLayout->addWidget(useEncryptionLevel, 0, 0);
  securityBoxLayout->addWidget(encryptionLevel, 0, 1);
  
  sambaTabLayout->addWidget(basicSettingsBox, 0);
  sambaTabLayout->addWidget(authenticationBox, 0);
  sambaTabLayout->addWidget(securityBox, 0);
  sambaTabLayout->addStretch(100);
  
  addTab(sambaTab, i18n("Samba Settings"));
  
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
  wakeOnLanWaitingTimeLabel->setObjectName("WakeOnLanWaitingTimeLabel");
  
  QSpinBox *wakeOnLanWaitingTime = new QSpinBox(wakeOnLanBox);
  wakeOnLanWaitingTime->setObjectName("kcfg_WakeOnLANWaitingTime");
  wakeOnLanWaitingTime->setSuffix(i18n(" s"));
  wakeOnLanWaitingTime->setSingleStep(1);
//   wakeOnLanWaitingTime->setSliderEnabled(true);
  
  wakeOnLanWaitingTimeLabel->setBuddy(wakeOnLanWaitingTime);
  
  QFrame *wakeOnLanNote = new QFrame(wakeOnLanBox);
  QGridLayout *wakeOnLanNoteLayout = new QGridLayout(wakeOnLanNote);
  wakeOnLanNoteLayout->setContentsMargins(5, 5, 5, 5);

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
  
  wakeOnLanBoxLayout->addWidget(enableWakeOnLan, 0, 0, 1, 2);
  wakeOnLanBoxLayout->addWidget(wakeOnLanWaitingTimeLabel, 1, 0);
  wakeOnLanBoxLayout->addWidget(wakeOnLanWaitingTime, 1, 1);
  wakeOnLanBoxLayout->addWidget(wakeOnLanNote, 2, 0, 1, 2);
  
  wakeOnLanTabLayout->addWidget(wakeOnLanBox, 0);
  wakeOnLanTabLayout->addStretch(100);
  
  addTab(wakeOnLanTab, i18n("Wake-On-LAN Settings"));

  //
  // Connections
  // 
#ifdef USE_SMBC_PROTOCOL
  connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotSetProtocolVersionsToggled(bool)));
#endif
  connect(enableWakeOnLan, SIGNAL(toggled(bool)), this, SLOT(slotEnableWakeOnLanFeatureToggled(bool)));
  
  //
  // Set the correct states to the widgets
  // 
#ifdef USE_SMBC_PROTOCOL
  slotSetProtocolVersionsToggled(Smb4KSettings::useClientProtocolVersions());
#endif
  slotEnableWakeOnLanFeatureToggled(Smb4KSettings::enableWakeOnLAN());
}


Smb4KConfigPageNetwork::~Smb4KConfigPageNetwork()
{
}


#ifdef USE_SMBC_PROTOCOL
void Smb4KConfigPageNetwork::slotSetProtocolVersionsToggled(bool on)
{
  //
  // Get the widgets
  // 
  QLabel *minimalProtocolVersionLabel = findChild<QLabel *>("MinimalProtocolVersionLabel");
  KComboBox *minimalProtocolVersion = findChild<KComboBox *>("kcfg_MinimalClientProtocolVersion");
  QLabel *maximalProtocolVersionLabel = findChild<QLabel *>("MaximalProtocolVersionLabel");
  KComboBox *maximalProtocolVersion = findChild<KComboBox *>("kcfg_MaximalClientProtocolVersion");
  
  //
  // Enable / disable widgets
  // 
  minimalProtocolVersionLabel->setEnabled(on);
  minimalProtocolVersion->setEnabled(on);
  maximalProtocolVersionLabel->setEnabled(on);
  maximalProtocolVersion->setEnabled(on);
}
#endif


void Smb4KConfigPageNetwork::slotEnableWakeOnLanFeatureToggled(bool on)
{
  //
  // Get the widgets
  // 
  QLabel *wakeOnLanWaitingTimeLabel = findChild<QLabel *>("WakeOnLanWaitingTimeLabel");
  QSpinBox *wakeOnLanWaitingTime = findChild<QSpinBox *>("kcfg_WakeOnLANWaitingTime");
  
  //
  // Enable / disable widgets
  // 
  wakeOnLanWaitingTimeLabel->setEnabled(on);
  wakeOnLanWaitingTime->setEnabled(on);
}



