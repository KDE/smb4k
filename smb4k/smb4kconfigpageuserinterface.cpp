/*
    This configuration page takes care of all settings concerning the
    user interface

    SPDX-FileCopyrightText: 2006-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpageuserinterface.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QVBoxLayout>

// KDE includes
#include <KCompletion/KComboBox>
#include <KI18n/KLocalizedString>

Smb4KConfigPageUserInterface::Smb4KConfigPageUserInterface(QWidget *parent)
    : QWidget(parent)
{
    //
    // Layout
    //
    QVBoxLayout *layout = new QVBoxLayout(this);

    //
    // Main Window settings
    //
    QGroupBox *mainWindowBox = new QGroupBox(i18n("Main Window"), this);
    QGridLayout *mainWindowBoxLayout = new QGridLayout(mainWindowBox);

    QLabel *tabOrientationLabel = new QLabel(Smb4KSettings::self()->mainWindowTabOrientationItem()->label(), mainWindowBox);

    KComboBox *tabOrientation = new KComboBox(mainWindowBox);
    tabOrientation->setObjectName("kcfg_MainWindowTabOrientation");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> tabOrientationChoices = Smb4KSettings::self()->mainWindowTabOrientationItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &to : qAsConst(tabOrientationChoices)) {
        tabOrientation->addItem(to.label);
    }

    mainWindowBoxLayout->addWidget(tabOrientationLabel, 0, 0);
    mainWindowBoxLayout->addWidget(tabOrientation, 0, 1);

    QCheckBox *showBookmarkLabel = new QCheckBox(Smb4KSettings::self()->showCustomBookmarkLabelItem()->label(), this);
    showBookmarkLabel->setObjectName("kcfg_ShowCustomBookmarkLabel");

    mainWindowBoxLayout->addWidget(showBookmarkLabel, 1, 0, 1, 2);

    layout->addWidget(mainWindowBox);

    //
    // Network Neighborhood settings
    //
    QGroupBox *networkNeighborhoodBox = new QGroupBox(i18n("Network Neighborhood"), this);
    QGridLayout *networkNeighborhoodBoxLayout = new QGridLayout(networkNeighborhoodBox);

    QLabel *iconSizeNetworkNeighborhoodLabel = new QLabel(Smb4KSettings::self()->networkBrowserIconSizeItem()->label(), networkNeighborhoodBox);
    QSlider *iconSizeNetworkNeighborhood = new QSlider(Qt::Horizontal, networkNeighborhoodBox);
    iconSizeNetworkNeighborhood->setObjectName("kcfg_NetworkBrowserIconSize");
    iconSizeNetworkNeighborhood->setPageStep(16);
    iconSizeNetworkNeighborhood->setSingleStep(16);
    iconSizeNetworkNeighborhood->setTickPosition(QSlider::TicksBelow);

    networkNeighborhoodBoxLayout->addWidget(iconSizeNetworkNeighborhoodLabel, 0, 0);
    networkNeighborhoodBoxLayout->addWidget(iconSizeNetworkNeighborhood, 0, 1);

    QCheckBox *autoExpand = new QCheckBox(Smb4KSettings::self()->autoExpandNetworkItemsItem()->label(), networkNeighborhoodBox);
    autoExpand->setObjectName("kcfg_AutoExpandNetworkItems");

    networkNeighborhoodBoxLayout->addWidget(autoExpand, 1, 0);

    QCheckBox *showType = new QCheckBox(Smb4KSettings::self()->showTypeItem()->label(), networkNeighborhoodBox);
    showType->setObjectName("kcfg_ShowType");

    networkNeighborhoodBoxLayout->addWidget(showType, 1, 1);

    QCheckBox *showIpAddress = new QCheckBox(Smb4KSettings::self()->showIPAddressItem()->label(), networkNeighborhoodBox);
    showIpAddress->setObjectName("kcfg_ShowIPAddress");

    networkNeighborhoodBoxLayout->addWidget(showIpAddress, 2, 0);

    QCheckBox *showComment = new QCheckBox(Smb4KSettings::self()->showCommentItem()->label(), networkNeighborhoodBox);
    showComment->setObjectName("kcfg_ShowComment");

    networkNeighborhoodBoxLayout->addWidget(showComment, 2, 1);

    QCheckBox *showNetworkTooltip = new QCheckBox(Smb4KSettings::self()->showNetworkItemToolTipItem()->label(), networkNeighborhoodBox);
    showNetworkTooltip->setObjectName("kcfg_ShowNetworkItemToolTip");

    networkNeighborhoodBoxLayout->addWidget(showNetworkTooltip, 3, 0);

    layout->addWidget(networkNeighborhoodBox);

    //
    // Shares View settings
    //
    QGroupBox *sharesViewBox = new QGroupBox(i18n("Shares View"), this);
    QGridLayout *sharesViewBoxLayout = new QGridLayout(sharesViewBox);

    QLabel *viewModeLabel = new QLabel(Smb4KSettings::self()->sharesViewModeItem()->label(), sharesViewBox);

    KComboBox *viewMode = new KComboBox(sharesViewBox);
    viewMode->setObjectName("kcfg_SharesViewMode");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> sharesViewModeChoices = Smb4KSettings::self()->sharesViewModeItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &vm : qAsConst(sharesViewModeChoices)) {
        viewMode->addItem(vm.label);
    }

    sharesViewBoxLayout->addWidget(viewModeLabel, 0, 0);
    sharesViewBoxLayout->addWidget(viewMode, 0, 1);

    QLabel *iconSizeSharesViewIconViewLabel = new QLabel(Smb4KSettings::self()->sharesViewIconSizeIconViewItem()->label(), sharesViewBox);
    QSlider *iconSizeSharesViewIconView = new QSlider(Qt::Horizontal, sharesViewBox);
    iconSizeSharesViewIconView->setObjectName("kcfg_SharesViewIconSizeIconView");
    iconSizeSharesViewIconView->setPageStep(16);
    iconSizeSharesViewIconView->setSingleStep(16);
    iconSizeSharesViewIconView->setTickPosition(QSlider::TicksBelow);

    sharesViewBoxLayout->addWidget(iconSizeSharesViewIconViewLabel, 1, 0);
    sharesViewBoxLayout->addWidget(iconSizeSharesViewIconView, 1, 1);

    QLabel *iconSizeSharesViewListViewLabel = new QLabel(Smb4KSettings::self()->sharesViewIconSizeListViewItem()->label(), sharesViewBox);
    QSlider *iconSizeSharesViewListView = new QSlider(Qt::Horizontal, sharesViewBox);
    iconSizeSharesViewListView->setObjectName("kcfg_SharesViewIconSizeListView");
    iconSizeSharesViewListView->setPageStep(16);
    iconSizeSharesViewListView->setSingleStep(16);
    iconSizeSharesViewListView->setTickPosition(QSlider::TicksBelow);

    sharesViewBoxLayout->addWidget(iconSizeSharesViewListViewLabel, 2, 0);
    sharesViewBoxLayout->addWidget(iconSizeSharesViewListView, 2, 1);

    QCheckBox *showShareTooltip = new QCheckBox(Smb4KSettings::self()->showShareToolTipItem()->label(), sharesViewBox);
    showShareTooltip->setObjectName("kcfg_ShowShareToolTip");

    sharesViewBoxLayout->addWidget(showShareTooltip, 3, 0, 1, 2);

    layout->addWidget(sharesViewBox);
    layout->addStretch(100);
}

Smb4KConfigPageUserInterface::~Smb4KConfigPageUserInterface()
{
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////
