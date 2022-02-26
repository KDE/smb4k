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
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QFormLayout>

// KDE includes
#include <KCompletion/KComboBox>
#include <KI18n/KLocalizedString>

Smb4KConfigPageUserInterface::Smb4KConfigPageUserInterface(QWidget *parent)
    : QWidget(parent)
{
    //
    // Layout
    //
    QFormLayout *layout = new QFormLayout(this);

    //
    // Main Window settings
    //
    QLabel *mainWindowSection = new QLabel(i18n("Main Window"), this);
    layout->addRow(mainWindowSection);

    QLabel *tabOrientationLabel = new QLabel(Smb4KSettings::self()->mainWindowTabOrientationItem()->label(), this);

    KComboBox *tabOrientation = new KComboBox(this);
    tabOrientation->setObjectName("kcfg_MainWindowTabOrientation");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> tabOrientationChoices = Smb4KSettings::self()->mainWindowTabOrientationItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &to : qAsConst(tabOrientationChoices)) {
        tabOrientation->addItem(to.label);
    }

    layout->addRow(tabOrientationLabel, tabOrientation);

    QCheckBox *showBookmarkLabel = new QCheckBox(Smb4KSettings::self()->showCustomBookmarkLabelItem()->label(), this);
    showBookmarkLabel->setObjectName("kcfg_ShowCustomBookmarkLabel");

    layout->addRow(i18n("Behavior:"), showBookmarkLabel);

    //
    // Network Neighborhood settings
    //
    QLabel *networkNeighborhoodSection = new QLabel(i18n("Network Neighborhood"), this);
    layout->addRow(networkNeighborhoodSection);

    QLabel *iconSizeNetworkNeighborhoodLabel = new QLabel(Smb4KSettings::self()->networkBrowserIconSizeItem()->label(), this);
    QSlider *iconSizeNetworkNeighborhood = new QSlider(Qt::Horizontal, this);
    iconSizeNetworkNeighborhood->setObjectName("kcfg_NetworkBrowserIconSize");
    iconSizeNetworkNeighborhood->setPageStep(16);
    iconSizeNetworkNeighborhood->setSingleStep(16);
    iconSizeNetworkNeighborhood->setTickPosition(QSlider::TicksBelow);

    layout->addRow(iconSizeNetworkNeighborhoodLabel, iconSizeNetworkNeighborhood);

    QCheckBox *autoExpand = new QCheckBox(Smb4KSettings::self()->autoExpandNetworkItemsItem()->label(), this);
    autoExpand->setObjectName("kcfg_AutoExpandNetworkItems");

    layout->addRow(i18n("Behavior:"), autoExpand);

    QCheckBox *showType = new QCheckBox(Smb4KSettings::self()->showTypeItem()->label(), this);
    showType->setObjectName("kcfg_ShowType");

    layout->addRow(QString(), showType);

    QCheckBox *showIpAddress = new QCheckBox(Smb4KSettings::self()->showIPAddressItem()->label(), this);
    showIpAddress->setObjectName("kcfg_ShowIPAddress");

    layout->addRow(QString(), showIpAddress);

    QCheckBox *showComment = new QCheckBox(Smb4KSettings::self()->showCommentItem()->label(), this);
    showComment->setObjectName("kcfg_ShowComment");

    layout->addRow(QString(), showComment);

    QCheckBox *showNetworkTooltip = new QCheckBox(Smb4KSettings::self()->showNetworkItemToolTipItem()->label(), this);
    showNetworkTooltip->setObjectName("kcfg_ShowNetworkItemToolTip");

    layout->addRow(QString(), showNetworkTooltip);

    //
    // Shares View settings
    //
    QLabel *sharesViewSection = new QLabel(i18n("Shares View"), this);
    layout->addRow(sharesViewSection);

    QLabel *viewModeLabel = new QLabel(Smb4KSettings::self()->sharesViewModeItem()->label(), this);

    KComboBox *viewMode = new KComboBox(this);
    viewMode->setObjectName("kcfg_SharesViewMode");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> sharesViewModeChoices = Smb4KSettings::self()->sharesViewModeItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &vm : qAsConst(sharesViewModeChoices)) {
        viewMode->addItem(vm.label);
    }

    layout->addRow(viewModeLabel, viewMode);

    QLabel *iconSizeSharesViewIconViewLabel = new QLabel(Smb4KSettings::self()->sharesViewIconSizeIconViewItem()->label(), this);
    QSlider *iconSizeSharesViewIconView = new QSlider(Qt::Horizontal, this);
    iconSizeSharesViewIconView->setObjectName("kcfg_SharesViewIconSizeIconView");
    iconSizeSharesViewIconView->setPageStep(16);
    iconSizeSharesViewIconView->setSingleStep(16);
    iconSizeSharesViewIconView->setTickPosition(QSlider::TicksBelow);

    layout->addRow(iconSizeSharesViewIconViewLabel, iconSizeSharesViewIconView);

    QLabel *iconSizeSharesViewListViewLabel = new QLabel(Smb4KSettings::self()->sharesViewIconSizeListViewItem()->label(), this);
    QSlider *iconSizeSharesViewListView = new QSlider(Qt::Horizontal, this);
    iconSizeSharesViewListView->setObjectName("kcfg_SharesViewIconSizeListView");
    iconSizeSharesViewListView->setPageStep(16);
    iconSizeSharesViewListView->setSingleStep(16);
    iconSizeSharesViewListView->setTickPosition(QSlider::TicksBelow);

    layout->addRow(iconSizeSharesViewListViewLabel, iconSizeSharesViewListView);

    QCheckBox *showShareTooltip = new QCheckBox(Smb4KSettings::self()->showShareToolTipItem()->label(), this);
    showShareTooltip->setObjectName("kcfg_ShowShareToolTip");

    layout->addRow(i18n("Behavior:"), showShareTooltip);
}

Smb4KConfigPageUserInterface::~Smb4KConfigPageUserInterface()
{
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////
