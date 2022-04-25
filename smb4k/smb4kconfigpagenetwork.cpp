/*
    smb4knetworkoptions  -  The configuration page for the network
    settings of Smb4K

    SPDX-FileCopyrightText: 2003-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagenetwork.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QFormLayout>
#include <QGridLayout>

// KDE includes
#include <KCompletion/KComboBox>
#include <KCompletion/KLineEdit>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

Smb4KConfigPageNetwork::Smb4KConfigPageNetwork(QWidget *parent)
    : QTabWidget(parent)
{
    //
    // Basic Settings
    //
    QWidget *basicTab = new QWidget(this);
    QFormLayout *basicTabLayout = new QFormLayout(basicTab);

    //
    // Browse Settings
    //
#ifdef USE_WS_DISCOVERY
    // Use WS-Discovery
    QCheckBox *useWsDiscovery = new QCheckBox(Smb4KSettings::self()->useWsDiscoveryItem()->label(), basicTab);
    useWsDiscovery->setObjectName("kcfg_UseWsDiscovery");

    basicTabLayout->addRow(i18n("Browse Settings:"), useWsDiscovery);
#endif

    // Use DNS-SD
    QCheckBox *useDnsServiceDiscovery = new QCheckBox(Smb4KSettings::self()->useDnsServiceDiscoveryItem()->label(), basicTab);
    useDnsServiceDiscovery->setObjectName("kcfg_UseDnsServiceDiscovery");

#ifdef USE_WS_DISCOVERY
    basicTabLayout->addRow(QString(), useDnsServiceDiscovery);
#else
    basicTabLayout->addRow(i18n("Browse Settings:"), useDnsServiceDiscovery);
#endif

    // Force SMBv1 protocol for browsing
    QCheckBox *forceSmb1Protocol = new QCheckBox(Smb4KSettings::self()->forceSmb1ProtocolItem()->label(), basicTab);
    forceSmb1Protocol->setObjectName("kcfg_ForceSmb1Protocol");

    basicTabLayout->addRow(QString(), forceSmb1Protocol);

    // Set client protocol versions
    QCheckBox *useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), basicTab);
    useClientProtocolVersions->setObjectName("kcfg_UseClientProtocolVersions");

    basicTabLayout->addRow(QString(), useClientProtocolVersions);

    QWidget *versionsWidget = new QWidget(basicTab);
    QGridLayout *versionsLayout = new QGridLayout(versionsWidget);
    versionsLayout->setMargin(0);

    QLabel *minimalProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), versionsWidget);
    minimalProtocolVersionLabel->setIndent(25);
    minimalProtocolVersionLabel->setObjectName("MinimalProtocolVersionLabel");

    KComboBox *minimalProtocolVersion = new KComboBox(versionsWidget);
    minimalProtocolVersion->setObjectName("kcfg_MinimalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(minimalProtocolVersionChoices)) {
        minimalProtocolVersion->addItem(c.label);
    }

    versionsLayout->addWidget(minimalProtocolVersionLabel, 0, 0);
    versionsLayout->addWidget(minimalProtocolVersion, 0, 1);

    QLabel *maximalProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), versionsWidget);
    maximalProtocolVersionLabel->setIndent(25);
    maximalProtocolVersionLabel->setObjectName("MaximalProtocolVersionLabel");

    KComboBox *maximalProtocolVersion = new KComboBox(versionsWidget);
    maximalProtocolVersion->setObjectName("kcfg_MaximalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(maximalProtocolVersionChoices)) {
        maximalProtocolVersion->addItem(c.label);
    }

    versionsLayout->addWidget(maximalProtocolVersionLabel, 1, 0);
    versionsLayout->addWidget(maximalProtocolVersion, 1, 1);

    basicTabLayout->addRow(QString(), versionsWidget);

    //
    // Behavior
    //
    QCheckBox *detectPrinters = new QCheckBox(Smb4KSettings::self()->detectPrinterSharesItem()->label(), basicTab);
    detectPrinters->setObjectName("kcfg_DetectPrinterShares");

    basicTabLayout->addRow(i18n("Behavior:"), detectPrinters);

    QCheckBox *detectHiddenShares = new QCheckBox(Smb4KSettings::self()->detectHiddenSharesItem()->label(), basicTab);
    detectHiddenShares->setObjectName("kcfg_DetectHiddenShares");

    basicTabLayout->addRow(QString(), detectHiddenShares);

    QCheckBox *previewHiddenItems = new QCheckBox(Smb4KSettings::self()->previewHiddenItemsItem()->label(), basicTab);
    previewHiddenItems->setObjectName("kcfg_PreviewHiddenItems");

    basicTabLayout->addRow(QString(), previewHiddenItems);

    addTab(basicTab, i18n("Basic Settings"));

    //
    // Advanced Settings
    //
    QWidget *advancedTab = new QWidget(this);
    QFormLayout *advancedTabLayout = new QFormLayout(advancedTab);
    
    // Samba    
    QWidget *remoteSmbPortWidget = new QWidget(advancedTab);
    QHBoxLayout *remoteSmbPortWidgetLayout = new QHBoxLayout(remoteSmbPortWidget);
    remoteSmbPortWidgetLayout->setMargin(0);

    QCheckBox *useRemoteSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), advancedTab);
    useRemoteSmbPort->setObjectName("kcfg_UseRemoteSmbPort");

    QSpinBox *remoteSmbPort = new QSpinBox(advancedTab);
    remoteSmbPort->setObjectName("kcfg_RemoteSmbPort");
    //   remoteSmbPort->setSliderEnabled(true);

    remoteSmbPortWidgetLayout->addWidget(useRemoteSmbPort);
    remoteSmbPortWidgetLayout->addWidget(remoteSmbPort);

    advancedTabLayout->addRow(i18n("Samba:"), remoteSmbPortWidget);

    QCheckBox *largeNetworkNeighborhood = new QCheckBox(Smb4KSettings::self()->largeNetworkNeighborhoodItem()->label(), advancedTab);
    largeNetworkNeighborhood->setObjectName("kcfg_LargeNetworkNeighborhood");

    advancedTabLayout->addRow(QString(), largeNetworkNeighborhood);

    QCheckBox *masterBrowsersRequireAuth = new QCheckBox(Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(), advancedTab);
    masterBrowsersRequireAuth->setObjectName("kcfg_MasterBrowsersRequireAuth");

    advancedTabLayout->addRow(QString(), masterBrowsersRequireAuth);

    QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), advancedTab);
    useKerberos->setObjectName("kcfg_UseKerberos");

    advancedTabLayout->addRow(QString(), useKerberos);

    QCheckBox *useCCache = new QCheckBox(Smb4KSettings::self()->useWinbindCCacheItem()->label(), advancedTab);
    useCCache->setObjectName("kcfg_UseWinbindCCache");

    advancedTabLayout->addRow(QString(), useCCache);

    QWidget *encryptionWidget = new QWidget(advancedTab);
    QHBoxLayout *encryptionWidgetLayout = new QHBoxLayout(encryptionWidget);
    encryptionWidgetLayout->setMargin(0);

    QCheckBox *useEncryptionLevel = new QCheckBox(Smb4KSettings::self()->useEncryptionLevelItem()->label(), advancedTab);
    useEncryptionLevel->setObjectName("kcfg_UseEncryptionLevel");

    KComboBox *encryptionLevel = new KComboBox(advancedTab);
    encryptionLevel->setObjectName("kcfg_EncryptionLevel");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> encryptionLevelChoices = Smb4KSettings::self()->encryptionLevelItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(encryptionLevelChoices)) {
        encryptionLevel->addItem(c.label);
    }

    encryptionWidgetLayout->addWidget(useEncryptionLevel);
    encryptionWidgetLayout->addWidget(encryptionLevel);

    advancedTabLayout->addRow(QString(), encryptionWidget);
    
    QCheckBox *enableWakeOnLan = new QCheckBox(Smb4KSettings::self()->enableWakeOnLANItem()->label(), advancedTab);
    enableWakeOnLan->setObjectName("kcfg_EnableWakeOnLAN");

    advancedTabLayout->addRow(i18n("Wake-On-LAN:"), enableWakeOnLan);
    
    QWidget *waitingTimeWidget = new QWidget(advancedTab);
    QGridLayout *waitingTimeWidgetLayout = new QGridLayout(waitingTimeWidget);
    waitingTimeWidgetLayout->setMargin(0);
    
    QLabel *wakeOnLanWaitingTimeLabel = new QLabel(Smb4KSettings::self()->wakeOnLANWaitingTimeItem()->label(), advancedTab);
    wakeOnLanWaitingTimeLabel->setIndent(25);
    wakeOnLanWaitingTimeLabel->setObjectName("WakeOnLanWaitingTimeLabel");
    
    QSpinBox *wakeOnLanWaitingTime = new QSpinBox(advancedTab);
    wakeOnLanWaitingTime->setObjectName("kcfg_WakeOnLANWaitingTime");
    wakeOnLanWaitingTime->setSuffix(i18n(" s"));
    wakeOnLanWaitingTime->setSingleStep(1);
    //   wakeOnLanWaitingTime->setSliderEnabled(true);
    
    waitingTimeWidgetLayout->addWidget(wakeOnLanWaitingTimeLabel, 0, 0);
    waitingTimeWidgetLayout->addWidget(wakeOnLanWaitingTime, 0, 1);

    advancedTabLayout->addRow(QString(), waitingTimeWidget);
    
    QFrame *wakeOnLanNote = new QFrame(advancedTab);
    QGridLayout *wakeOnLanNoteLayout = new QGridLayout(wakeOnLanNote);
    wakeOnLanNoteLayout->setContentsMargins(5, 5, 5, 5);
    wakeOnLanNoteLayout->setMargin(0);

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
    
    advancedTabLayout->addRow(QString(), wakeOnLanNote);
    
    addTab(advancedTab, i18n("Advanced Settings"));

    //
    // Connections
    //
    connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotSetProtocolVersionsToggled(bool)));
    connect(enableWakeOnLan, SIGNAL(toggled(bool)), this, SLOT(slotEnableWakeOnLanFeatureToggled(bool)));

    //
    // Set the correct states to the widgets
    //
    slotSetProtocolVersionsToggled(Smb4KSettings::useClientProtocolVersions());
    slotEnableWakeOnLanFeatureToggled(Smb4KSettings::enableWakeOnLAN());
}

Smb4KConfigPageNetwork::~Smb4KConfigPageNetwork()
{
}

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
