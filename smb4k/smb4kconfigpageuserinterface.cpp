/*
    This configuration page takes care of all settings concerning the
    user interface

    SPDX-FileCopyrightText: 2006-2026 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpageuserinterface.h"
#include "core/smb4kautostartmanager.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

// KDE includes
#include <KComboBox>
#include <KLocalizedString>

Smb4KConfigPageUserInterface::Smb4KConfigPageUserInterface(QWidget *parent)
    : QTabWidget(parent)
{
    QWidget *userInterfaceTab = new QWidget(this);
    QVBoxLayout *userInterfaceTabLayout = new QVBoxLayout(userInterfaceTab);

    QGroupBox *mainWindowBox = new QGroupBox(i18n("Main Window"), userInterfaceTab);
    QGridLayout *mainWindowBoxLayout = new QGridLayout(mainWindowBox);

    QLabel *tabOrientationLabel = new QLabel(Smb4KSettings::self()->mainWindowTabOrientationItem()->label(), mainWindowBox);

    KComboBox *tabOrientation = new KComboBox(mainWindowBox);
    tabOrientation->setObjectName(QStringLiteral("kcfg_MainWindowTabOrientation"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> tabOrientationChoices = Smb4KSettings::self()->mainWindowTabOrientationItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &to : std::as_const(tabOrientationChoices)) {
        tabOrientation->addItem(to.label);
    }

    mainWindowBoxLayout->addWidget(tabOrientationLabel, 0, 0);
    mainWindowBoxLayout->addWidget(tabOrientation, 0, 1);

    QCheckBox *showBookmarkLabel = new QCheckBox(Smb4KSettings::self()->showCustomBookmarkLabelItem()->label(), this);
    showBookmarkLabel->setObjectName(QStringLiteral("kcfg_ShowCustomBookmarkLabel"));

    mainWindowBoxLayout->addWidget(showBookmarkLabel, 1, 0, 1, 2);

    userInterfaceTabLayout->addWidget(mainWindowBox);

    QGroupBox *networkNeighborhoodBox = new QGroupBox(i18n("Network Neighborhood"), userInterfaceTab);
    QGridLayout *networkNeighborhoodBoxLayout = new QGridLayout(networkNeighborhoodBox);

    QLabel *iconSizeNetworkNeighborhoodLabel = new QLabel(Smb4KSettings::self()->networkBrowserIconSizeItem()->label(), networkNeighborhoodBox);
    QSlider *iconSizeNetworkNeighborhood = new QSlider(Qt::Horizontal, networkNeighborhoodBox);
    iconSizeNetworkNeighborhood->setObjectName(QStringLiteral("kcfg_NetworkBrowserIconSize"));
    iconSizeNetworkNeighborhood->setPageStep(16);
    iconSizeNetworkNeighborhood->setSingleStep(16);
    iconSizeNetworkNeighborhood->setTickPosition(QSlider::TicksBelow);

    networkNeighborhoodBoxLayout->addWidget(iconSizeNetworkNeighborhoodLabel, 0, 0);
    networkNeighborhoodBoxLayout->addWidget(iconSizeNetworkNeighborhood, 0, 1);

    QCheckBox *autoExpand = new QCheckBox(Smb4KSettings::self()->autoExpandNetworkItemsItem()->label(), networkNeighborhoodBox);
    autoExpand->setObjectName(QStringLiteral("kcfg_AutoExpandNetworkItems"));

    networkNeighborhoodBoxLayout->addWidget(autoExpand, 1, 0);

    QCheckBox *showType = new QCheckBox(Smb4KSettings::self()->showTypeItem()->label(), networkNeighborhoodBox);
    showType->setObjectName(QStringLiteral("kcfg_ShowType"));

    networkNeighborhoodBoxLayout->addWidget(showType, 1, 1);

    QCheckBox *showIpAddress = new QCheckBox(Smb4KSettings::self()->showIPAddressItem()->label(), networkNeighborhoodBox);
    showIpAddress->setObjectName(QStringLiteral("kcfg_ShowIPAddress"));

    networkNeighborhoodBoxLayout->addWidget(showIpAddress, 2, 0);

    QCheckBox *showComment = new QCheckBox(Smb4KSettings::self()->showCommentItem()->label(), networkNeighborhoodBox);
    showComment->setObjectName(QStringLiteral("kcfg_ShowComment"));

    networkNeighborhoodBoxLayout->addWidget(showComment, 2, 1);

    QCheckBox *showNetworkTooltip = new QCheckBox(Smb4KSettings::self()->showNetworkItemToolTipItem()->label(), networkNeighborhoodBox);
    showNetworkTooltip->setObjectName(QStringLiteral("kcfg_ShowNetworkItemToolTip"));

    networkNeighborhoodBoxLayout->addWidget(showNetworkTooltip, 3, 0);

    userInterfaceTabLayout->addWidget(networkNeighborhoodBox);

    QGroupBox *sharesViewBox = new QGroupBox(i18n("Shares View"), userInterfaceTab);
    QGridLayout *sharesViewBoxLayout = new QGridLayout(sharesViewBox);

    QLabel *viewModeLabel = new QLabel(Smb4KSettings::self()->sharesViewModeItem()->label(), sharesViewBox);

    KComboBox *viewMode = new KComboBox(sharesViewBox);
    viewMode->setObjectName(QStringLiteral("kcfg_SharesViewMode"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> sharesViewModeChoices = Smb4KSettings::self()->sharesViewModeItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &vm : std::as_const(sharesViewModeChoices)) {
        viewMode->addItem(vm.label);
    }

    sharesViewBoxLayout->addWidget(viewModeLabel, 0, 0);
    sharesViewBoxLayout->addWidget(viewMode, 0, 1);

    QLabel *iconSizeSharesViewIconViewLabel = new QLabel(Smb4KSettings::self()->sharesViewIconSizeIconViewItem()->label(), sharesViewBox);
    QSlider *iconSizeSharesViewIconView = new QSlider(Qt::Horizontal, sharesViewBox);
    iconSizeSharesViewIconView->setObjectName(QStringLiteral("kcfg_SharesViewIconSizeIconView"));
    iconSizeSharesViewIconView->setPageStep(16);
    iconSizeSharesViewIconView->setSingleStep(16);
    iconSizeSharesViewIconView->setTickPosition(QSlider::TicksBelow);

    sharesViewBoxLayout->addWidget(iconSizeSharesViewIconViewLabel, 1, 0);
    sharesViewBoxLayout->addWidget(iconSizeSharesViewIconView, 1, 1);

    QLabel *iconSizeSharesViewListViewLabel = new QLabel(Smb4KSettings::self()->sharesViewIconSizeListViewItem()->label(), sharesViewBox);
    QSlider *iconSizeSharesViewListView = new QSlider(Qt::Horizontal, sharesViewBox);
    iconSizeSharesViewListView->setObjectName(QStringLiteral("kcfg_SharesViewIconSizeListView"));
    iconSizeSharesViewListView->setPageStep(16);
    iconSizeSharesViewListView->setSingleStep(16);
    iconSizeSharesViewListView->setTickPosition(QSlider::TicksBelow);

    sharesViewBoxLayout->addWidget(iconSizeSharesViewListViewLabel, 2, 0);
    sharesViewBoxLayout->addWidget(iconSizeSharesViewListView, 2, 1);

    QCheckBox *showShareTooltip = new QCheckBox(Smb4KSettings::self()->showShareToolTipItem()->label(), sharesViewBox);
    showShareTooltip->setObjectName(QStringLiteral("kcfg_ShowShareToolTip"));

    sharesViewBoxLayout->addWidget(showShareTooltip, 3, 0, 1, 2);

    userInterfaceTabLayout->addWidget(sharesViewBox);
    userInterfaceTabLayout->addStretch(100);

    addTab(userInterfaceTab, i18n("User Interface"));

    QWidget *launchTab = new QWidget(this);
    QVBoxLayout *launchTabLayout = new QVBoxLayout(launchTab);

    QGroupBox *programLaunchBox = new QGroupBox(i18n("Behavior"), launchTab);
    QVBoxLayout *programLaunchBoxLayout = new QVBoxLayout(programLaunchBox);

    QCheckBox *startAutomatically = new QCheckBox(Smb4KSettings::self()->autostartApplicationItem()->label(), programLaunchBox);
    startAutomatically->setObjectName(QStringLiteral("kcfg_AutostartApplication"));

    programLaunchBoxLayout->addWidget(startAutomatically);

    QCheckBox *startMainWindowDocked = new QCheckBox(Smb4KSettings::self()->startMainWindowDockedItem()->label(), programLaunchBox);
    startMainWindowDocked->setObjectName(QStringLiteral("kcfg_StartMainWindowDocked"));

    programLaunchBoxLayout->addWidget(startMainWindowDocked);

    launchTabLayout->addWidget(programLaunchBox);
    launchTabLayout->addStretch(100);

    addTab(launchTab, i18n("Program Launch"));
}

Smb4KConfigPageUserInterface::~Smb4KConfigPageUserInterface()
{
}

void Smb4KConfigPageUserInterface::saveAutostartSetting()
{
    QCheckBox *startAutomatically = findChild<QCheckBox *>(QStringLiteral("kcfg_AutostartApplication"));
    Smb4KAutoStartManager::self()->enableAutoStart(startAutomatically->isChecked());
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////
