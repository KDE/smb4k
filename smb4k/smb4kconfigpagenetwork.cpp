/*
    The configuration page for the network settings of Smb4K

    SPDX-FileCopyrightText: 2003-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagenetwork.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

// KDE includes
#include <KComboBox>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageWidget>

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
    useWsDiscovery->setObjectName(QStringLiteral("kcfg_UseWsDiscovery"));

    browseSettingsBoxLayout->addWidget(useWsDiscovery);
#endif

    // Use DNS-SD
    QCheckBox *useDnsServiceDiscovery = new QCheckBox(Smb4KSettings::self()->useDnsServiceDiscoveryItem()->label(), browseSettingsBox);
    useDnsServiceDiscovery->setObjectName(QStringLiteral("kcfg_UseDnsServiceDiscovery"));

    browseSettingsBoxLayout->addWidget(useDnsServiceDiscovery);

    // Force SMBv1 protocol for browsing
    QCheckBox *forceSmb1Protocol = new QCheckBox(Smb4KSettings::self()->forceSmb1ProtocolItem()->label(), browseSettingsBox);
    forceSmb1Protocol->setObjectName(QStringLiteral("kcfg_ForceSmb1Protocol"));

    browseSettingsBoxLayout->addWidget(forceSmb1Protocol);

    // Set client protocol versions
    QCheckBox *useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), browseSettingsBox);
    useClientProtocolVersions->setObjectName(QStringLiteral("kcfg_UseClientProtocolVersions"));

    browseSettingsBoxLayout->addWidget(useClientProtocolVersions);

    m_protocolVersionsWidget = new QWidget(browseSettingsBox);
    m_protocolVersionsWidget->setEnabled(false);
    QGridLayout *protocolVersionsWidgetLayout = new QGridLayout(m_protocolVersionsWidget);
    protocolVersionsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *minimalProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), m_protocolVersionsWidget);
    minimalProtocolVersionLabel->setIndent(25);

    KComboBox *minimalProtocolVersion = new KComboBox(m_protocolVersionsWidget);
    minimalProtocolVersion->setObjectName(QStringLiteral("kcfg_MinimalClientProtocolVersion"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(minimalProtocolVersionChoices)) {
        minimalProtocolVersion->addItem(c.label);
    }

    protocolVersionsWidgetLayout->addWidget(minimalProtocolVersionLabel, 0, 0);
    protocolVersionsWidgetLayout->addWidget(minimalProtocolVersion, 0, 1);

    QLabel *maximalProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), m_protocolVersionsWidget);
    maximalProtocolVersionLabel->setIndent(25);

    KComboBox *maximalProtocolVersion = new KComboBox(m_protocolVersionsWidget);
    maximalProtocolVersion->setObjectName(QStringLiteral("kcfg_MaximalClientProtocolVersion"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(maximalProtocolVersionChoices)) {
        maximalProtocolVersion->addItem(c.label);
    }

    protocolVersionsWidgetLayout->addWidget(maximalProtocolVersionLabel, 1, 0);
    protocolVersionsWidgetLayout->addWidget(maximalProtocolVersion, 1, 1);

    browseSettingsBoxLayout->addWidget(m_protocolVersionsWidget);

    basicTabLayout->addWidget(browseSettingsBox);

    //
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), basicTab);
    QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);

    QCheckBox *detectPrinters = new QCheckBox(Smb4KSettings::self()->detectPrinterSharesItem()->label(), behaviorBox);
    detectPrinters->setObjectName(QStringLiteral("kcfg_DetectPrinterShares"));

    behaviorBoxLayout->addWidget(detectPrinters, 0, 0);

    QCheckBox *detectHiddenShares = new QCheckBox(Smb4KSettings::self()->detectHiddenSharesItem()->label(), behaviorBox);
    detectHiddenShares->setObjectName(QStringLiteral("kcfg_DetectHiddenShares"));

    behaviorBoxLayout->addWidget(detectHiddenShares, 0, 1);

    QCheckBox *previewHiddenItems = new QCheckBox(Smb4KSettings::self()->previewHiddenItemsItem()->label(), behaviorBox);
    previewHiddenItems->setObjectName(QStringLiteral("kcfg_PreviewHiddenItems"));

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
    useRemoteSmbPort->setObjectName(QStringLiteral("kcfg_UseRemoteSmbPort"));

    sambaBoxLayout->addWidget(useRemoteSmbPort, 0, 0);

    QSpinBox *remoteSmbPort = new QSpinBox(sambaBox);
    remoteSmbPort->setObjectName(QStringLiteral("kcfg_RemoteSmbPort"));
    //   remoteSmbPort->setSliderEnabled(true);

    sambaBoxLayout->addWidget(remoteSmbPort, 0, 1);

    QCheckBox *useEncryptionLevel = new QCheckBox(Smb4KSettings::self()->useEncryptionLevelItem()->label(), sambaBox);
    useEncryptionLevel->setObjectName(QStringLiteral("kcfg_UseEncryptionLevel"));

    sambaBoxLayout->addWidget(useEncryptionLevel, 1, 0);

    KComboBox *encryptionLevel = new KComboBox(sambaBox);
    encryptionLevel->setObjectName(QStringLiteral("kcfg_EncryptionLevel"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> encryptionLevelChoices = Smb4KSettings::self()->encryptionLevelItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(encryptionLevelChoices)) {
        encryptionLevel->addItem(c.label);
    }

    sambaBoxLayout->addWidget(encryptionLevel, 1, 1);

    QCheckBox *largeNetworkNeighborhood = new QCheckBox(Smb4KSettings::self()->largeNetworkNeighborhoodItem()->label(), sambaBox);
    largeNetworkNeighborhood->setObjectName(QStringLiteral("kcfg_LargeNetworkNeighborhood"));

    sambaBoxLayout->addWidget(largeNetworkNeighborhood, 2, 0, 1, 2);

    QCheckBox *masterBrowsersRequireAuth = new QCheckBox(Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(), sambaBox);
    masterBrowsersRequireAuth->setObjectName(QStringLiteral("kcfg_MasterBrowsersRequireAuth"));

    sambaBoxLayout->addWidget(masterBrowsersRequireAuth, 3, 0, 1, 2);

    QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), sambaBox);
    useKerberos->setObjectName(QStringLiteral("kcfg_UseKerberos"));

    sambaBoxLayout->addWidget(useKerberos, 4, 0, 1, 2);

    QCheckBox *useCCache = new QCheckBox(Smb4KSettings::self()->useWinbindCCacheItem()->label(), sambaBox);
    useCCache->setObjectName(QStringLiteral("kcfg_UseWinbindCCache"));

    sambaBoxLayout->addWidget(useCCache, 5, 0, 1, 2);

    advancedTabLayout->addWidget(sambaBox);

    QGroupBox *wakeOnLanBox = new QGroupBox(i18n("Wake-On-LAN"), advancedTab);
    QVBoxLayout *wakeOnLanBoxLayout = new QVBoxLayout(wakeOnLanBox);

    QCheckBox *enableWakeOnLan = new QCheckBox(Smb4KSettings::self()->enableWakeOnLANItem()->label(), wakeOnLanBox);
    enableWakeOnLan->setObjectName(QStringLiteral("kcfg_EnableWakeOnLAN"));

    wakeOnLanBoxLayout->addWidget(enableWakeOnLan);

    m_waitingTimeWidget = new QWidget(wakeOnLanBox);
    m_waitingTimeWidget->setEnabled(false);
    QHBoxLayout *waitingTimeWidgetLayout = new QHBoxLayout(m_waitingTimeWidget);
    waitingTimeWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *wakeOnLanWaitingTimeLabel = new QLabel(Smb4KSettings::self()->wakeOnLANWaitingTimeItem()->label(), m_waitingTimeWidget);
    wakeOnLanWaitingTimeLabel->setIndent(25);

    waitingTimeWidgetLayout->addWidget(wakeOnLanWaitingTimeLabel);

    QSpinBox *wakeOnLanWaitingTime = new QSpinBox(m_waitingTimeWidget);
    wakeOnLanWaitingTime->setObjectName(QStringLiteral("kcfg_WakeOnLANWaitingTime"));
    wakeOnLanWaitingTime->setSuffix(i18n(" s"));
    wakeOnLanWaitingTime->setSingleStep(1);
    //   wakeOnLanWaitingTime->setSliderEnabled(true);

    waitingTimeWidgetLayout->addWidget(wakeOnLanWaitingTime);

    wakeOnLanBoxLayout->addWidget(m_waitingTimeWidget);

    KMessageWidget *wakeOnLanNote = new KMessageWidget(wakeOnLanBox);
    wakeOnLanNote->setText(i18n("Define the hosts that should be woken up via the custom settings editor."));
    wakeOnLanNote->setMessageType(KMessageWidget::Information);
    wakeOnLanNote->setCloseButtonVisible(false);
    wakeOnLanNote->setIcon(KDE::icon(QStringLiteral("emblem-information")));

    wakeOnLanBoxLayout->addWidget(wakeOnLanNote);

    advancedTabLayout->addWidget(wakeOnLanBox);
    advancedTabLayout->addStretch(100);

    addTab(advancedTab, i18n("Advanced Settings"));

    connect(useClientProtocolVersions, &QCheckBox::toggled, this, &Smb4KConfigPageNetwork::slotSetProtocolVersionsToggled);
    connect(enableWakeOnLan, &QCheckBox::toggled, this, &Smb4KConfigPageNetwork::slotEnableWakeOnLanFeatureToggled);
}

Smb4KConfigPageNetwork::~Smb4KConfigPageNetwork()
{
}

void Smb4KConfigPageNetwork::slotSetProtocolVersionsToggled(bool on)
{
    m_protocolVersionsWidget->setEnabled(on);
}

void Smb4KConfigPageNetwork::slotEnableWakeOnLanFeatureToggled(bool on)
{
    m_waitingTimeWidget->setEnabled(on);
}
