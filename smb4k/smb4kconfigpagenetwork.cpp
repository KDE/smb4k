/*
    The configuration page for the network settings of Smb4K

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
// #include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>
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
    QVBoxLayout *basicTabLayout = new QVBoxLayout(basicTab);

    //
    // Browse Settings
    //
    QGroupBox *browseSettingsBox = new QGroupBox(i18n("Browse Settings"), basicTab);
    QVBoxLayout *browseSettingsBoxLayout = new QVBoxLayout(browseSettingsBox);

#ifdef USE_WS_DISCOVERY
    // Use WS-Discovery
    QCheckBox *useWsDiscovery = new QCheckBox(Smb4KSettings::self()->useWsDiscoveryItem()->label(), browseSettingsBox);
    useWsDiscovery->setObjectName("kcfg_UseWsDiscovery");

    browseSettingsBoxLayout->addWidget(useWsDiscovery);
#endif

    // Use DNS-SD
    QCheckBox *useDnsServiceDiscovery = new QCheckBox(Smb4KSettings::self()->useDnsServiceDiscoveryItem()->label(), browseSettingsBox);
    useDnsServiceDiscovery->setObjectName("kcfg_UseDnsServiceDiscovery");

    browseSettingsBoxLayout->addWidget(useDnsServiceDiscovery);

    // Force SMBv1 protocol for browsing
    QCheckBox *forceSmb1Protocol = new QCheckBox(Smb4KSettings::self()->forceSmb1ProtocolItem()->label(), browseSettingsBox);
    forceSmb1Protocol->setObjectName("kcfg_ForceSmb1Protocol");

    browseSettingsBoxLayout->addWidget(forceSmb1Protocol);

    // Set client protocol versions
    QCheckBox *useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), browseSettingsBox);
    useClientProtocolVersions->setObjectName("kcfg_UseClientProtocolVersions");

    browseSettingsBoxLayout->addWidget(useClientProtocolVersions);

    QWidget *versionsWidget = new QWidget(browseSettingsBox);
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

    browseSettingsBoxLayout->addWidget(versionsWidget);

    basicTabLayout->addWidget(browseSettingsBox);

    //
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), basicTab);
    QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);

    QCheckBox *detectPrinters = new QCheckBox(Smb4KSettings::self()->detectPrinterSharesItem()->label(), behaviorBox);
    detectPrinters->setObjectName("kcfg_DetectPrinterShares");

    behaviorBoxLayout->addWidget(detectPrinters, 0, 0);

    QCheckBox *detectHiddenShares = new QCheckBox(Smb4KSettings::self()->detectHiddenSharesItem()->label(), behaviorBox);
    detectHiddenShares->setObjectName("kcfg_DetectHiddenShares");

    behaviorBoxLayout->addWidget(detectHiddenShares, 0, 1);

    QCheckBox *previewHiddenItems = new QCheckBox(Smb4KSettings::self()->previewHiddenItemsItem()->label(), behaviorBox);
    previewHiddenItems->setObjectName("kcfg_PreviewHiddenItems");

    behaviorBoxLayout->addWidget(previewHiddenItems, 1, 0);

    basicTabLayout->addWidget(behaviorBox);
    basicTabLayout->addStretch(100);

    addTab(basicTab, i18n("Basic Settings"));

    //
    // Advanced Settings
    //
    QWidget *advancedTab = new QWidget(this);
    QVBoxLayout *advancedTabLayout = new QVBoxLayout(advancedTab);

    //
    // Samba
    //
    QGroupBox *sambaBox = new QGroupBox(i18n("Samba"), advancedTab);
    QGridLayout *sambaBoxLayout = new QGridLayout(sambaBox);

    QCheckBox *useRemoteSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), sambaBox);
    useRemoteSmbPort->setObjectName("kcfg_UseRemoteSmbPort");

    sambaBoxLayout->addWidget(useRemoteSmbPort, 0, 0);

    QSpinBox *remoteSmbPort = new QSpinBox(sambaBox);
    remoteSmbPort->setObjectName("kcfg_RemoteSmbPort");
    //   remoteSmbPort->setSliderEnabled(true);

    sambaBoxLayout->addWidget(remoteSmbPort, 0, 1);

    QCheckBox *largeNetworkNeighborhood = new QCheckBox(Smb4KSettings::self()->largeNetworkNeighborhoodItem()->label(), sambaBox);
    largeNetworkNeighborhood->setObjectName("kcfg_LargeNetworkNeighborhood");

    sambaBoxLayout->addWidget(largeNetworkNeighborhood, 1, 0, 1, 2);

    QCheckBox *masterBrowsersRequireAuth = new QCheckBox(Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(), sambaBox);
    masterBrowsersRequireAuth->setObjectName("kcfg_MasterBrowsersRequireAuth");

    sambaBoxLayout->addWidget(masterBrowsersRequireAuth, 2, 0, 1, 2);
//
//     QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), advancedTab);
//     useKerberos->setObjectName("kcfg_UseKerberos");
//
//     advancedTabLayout->addRow(QString(), useKerberos);
//
//     QCheckBox *useCCache = new QCheckBox(Smb4KSettings::self()->useWinbindCCacheItem()->label(), advancedTab);
//     useCCache->setObjectName("kcfg_UseWinbindCCache");
//
//     advancedTabLayout->addRow(QString(), useCCache);
//
//     QWidget *encryptionWidget = new QWidget(advancedTab);
//     QHBoxLayout *encryptionWidgetLayout = new QHBoxLayout(encryptionWidget);
//     encryptionWidgetLayout->setMargin(0);
//
//     QCheckBox *useEncryptionLevel = new QCheckBox(Smb4KSettings::self()->useEncryptionLevelItem()->label(), advancedTab);
//     useEncryptionLevel->setObjectName("kcfg_UseEncryptionLevel");
//
//     KComboBox *encryptionLevel = new KComboBox(advancedTab);
//     encryptionLevel->setObjectName("kcfg_EncryptionLevel");
//
//     QList<KCoreConfigSkeleton::ItemEnum::Choice> encryptionLevelChoices = Smb4KSettings::self()->encryptionLevelItem()->choices();
//
//     for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(encryptionLevelChoices)) {
//         encryptionLevel->addItem(c.label);
//     }
//
//     encryptionWidgetLayout->addWidget(useEncryptionLevel);
//     encryptionWidgetLayout->addWidget(encryptionLevel);
//
//     advancedTabLayout->addRow(QString(), encryptionWidget);
//
//     QCheckBox *enableWakeOnLan = new QCheckBox(Smb4KSettings::self()->enableWakeOnLANItem()->label(), advancedTab);
//     enableWakeOnLan->setObjectName("kcfg_EnableWakeOnLAN");

    advancedTabLayout->addWidget(sambaBox);

//     advancedTabLayout->addRow(i18n("Wake-On-LAN:"), enableWakeOnLan);
//
//     QWidget *waitingTimeWidget = new QWidget(advancedTab);
//     QGridLayout *waitingTimeWidgetLayout = new QGridLayout(waitingTimeWidget);
//     waitingTimeWidgetLayout->setMargin(0);
//
//     QLabel *wakeOnLanWaitingTimeLabel = new QLabel(Smb4KSettings::self()->wakeOnLANWaitingTimeItem()->label(), advancedTab);
//     wakeOnLanWaitingTimeLabel->setIndent(25);
//     wakeOnLanWaitingTimeLabel->setObjectName("WakeOnLanWaitingTimeLabel");
//
//     QSpinBox *wakeOnLanWaitingTime = new QSpinBox(advancedTab);
//     wakeOnLanWaitingTime->setObjectName("kcfg_WakeOnLANWaitingTime");
//     wakeOnLanWaitingTime->setSuffix(i18n(" s"));
//     wakeOnLanWaitingTime->setSingleStep(1);
//     //   wakeOnLanWaitingTime->setSliderEnabled(true);
//
//     waitingTimeWidgetLayout->addWidget(wakeOnLanWaitingTimeLabel, 0, 0);
//     waitingTimeWidgetLayout->addWidget(wakeOnLanWaitingTime, 0, 1);
//
//     advancedTabLayout->addRow(QString(), waitingTimeWidget);
//
//     QFrame *wakeOnLanNote = new QFrame(advancedTab);
//     QGridLayout *wakeOnLanNoteLayout = new QGridLayout(wakeOnLanNote);
//     wakeOnLanNoteLayout->setContentsMargins(5, 5, 5, 5);
//     wakeOnLanNoteLayout->setMargin(0);
//
//     QLabel *importantPixmap = new QLabel(wakeOnLanNote);
//     importantPixmap->setPixmap(KIconLoader::global()->loadIcon("emblem-important", KIconLoader::Desktop, KIconLoader::SizeMedium));
//     importantPixmap->adjustSize();
//
//     QLabel *message = new QLabel(wakeOnLanNote);
//     message->setText(i18n("<qt>Define the hosts that should be woken up via the custom options dialog.</qt>"));
//     message->setTextFormat(Qt::AutoText);
//     message->setWordWrap(true);
//     message->setAlignment(Qt::AlignJustify);
//
//     wakeOnLanNoteLayout->addWidget(importantPixmap, 0, 0, Qt::AlignVCenter);
//     wakeOnLanNoteLayout->addWidget(message, 0, 1, Qt::AlignVCenter);
//     wakeOnLanNoteLayout->setColumnStretch(1, 1);
//
//     advancedTabLayout->addRow(QString(), wakeOnLanNote);

    advancedTabLayout->addStretch(100);

    addTab(advancedTab, i18n("Advanced Settings"));
//
//     //
//     // Connections
//     //
//     connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotSetProtocolVersionsToggled(bool)));
//     connect(enableWakeOnLan, SIGNAL(toggled(bool)), this, SLOT(slotEnableWakeOnLanFeatureToggled(bool)));
//
//     //
//     // Set the correct states to the widgets
//     //
//     slotSetProtocolVersionsToggled(Smb4KSettings::useClientProtocolVersions());
//     slotEnableWakeOnLanFeatureToggled(Smb4KSettings::enableWakeOnLAN());
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
