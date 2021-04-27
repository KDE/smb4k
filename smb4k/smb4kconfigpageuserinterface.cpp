/***************************************************************************
    This configuration page takes care of all settings concerning the
    user interface
                             -------------------
    begin                : Mi Aug 30 2006
    copyright            : (C) 2006-2021 by Alexander Reinholdt
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
#include "smb4kconfigpageuserinterface.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
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

    tabOrientationLabel->setBuddy(tabOrientation);

    mainWindowBoxLayout->addWidget(tabOrientationLabel, 0, 0);
    mainWindowBoxLayout->addWidget(tabOrientation, 0, 1);

    QCheckBox *showBookmarkLabel = new QCheckBox(Smb4KSettings::self()->showCustomBookmarkLabelItem()->label(), mainWindowBox);
    showBookmarkLabel->setObjectName("kcfg_ShowCustomBookmarkLabel");

    mainWindowBoxLayout->addWidget(showBookmarkLabel, 1, 0, 1, 2);

    layout->addWidget(mainWindowBox, 0);

    //
    // Network Neighborhood settings
    //
    QGroupBox *networkNeighhoodBox = new QGroupBox(i18n("Network Neighborhood"), this);
    QGridLayout *networkNeighhoodBoxLayout = new QGridLayout(networkNeighhoodBox);

    QCheckBox *autoExpand = new QCheckBox(Smb4KSettings::self()->autoExpandNetworkItemsItem()->label(), networkNeighhoodBox);
    autoExpand->setObjectName("kcfg_AutoExpandNetworkItems");

    networkNeighhoodBoxLayout->addWidget(autoExpand, 0, 0);

    QCheckBox *showType = new QCheckBox(Smb4KSettings::self()->showTypeItem()->label(), networkNeighhoodBox);
    showType->setObjectName("kcfg_ShowType");

    networkNeighhoodBoxLayout->addWidget(showType, 0, 1);

    QCheckBox *showIpAddress = new QCheckBox(Smb4KSettings::self()->showIPAddressItem()->label(), networkNeighhoodBox);
    showIpAddress->setObjectName("kcfg_ShowIPAddress");

    networkNeighhoodBoxLayout->addWidget(showIpAddress, 1, 0);

    QCheckBox *showComment = new QCheckBox(Smb4KSettings::self()->showCommentItem()->label(), networkNeighhoodBox);
    showComment->setObjectName("kcfg_ShowComment");

    networkNeighhoodBoxLayout->addWidget(showComment, 1, 1);

    QCheckBox *showNetworkTooltip = new QCheckBox(Smb4KSettings::self()->showNetworkItemToolTipItem()->label(), networkNeighhoodBox);
    showNetworkTooltip->setObjectName("kcfg_ShowNetworkItemToolTip");

    networkNeighhoodBoxLayout->addWidget(showNetworkTooltip, 2, 0, 1, 2);

    layout->addWidget(networkNeighhoodBox, 0);

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

    viewModeLabel->setBuddy(viewMode);

    sharesViewBoxLayout->addWidget(viewModeLabel, 0, 0);
    sharesViewBoxLayout->addWidget(viewMode, 0, 1);

    QCheckBox *showShareTooltip = new QCheckBox(Smb4KSettings::self()->showShareToolTipItem()->label(), sharesViewBox);
    showShareTooltip->setObjectName("kcfg_ShowShareToolTip");

    sharesViewBoxLayout->addWidget(showShareTooltip, 1, 0, 1, 2);

    layout->addWidget(sharesViewBox, 0);
    layout->addStretch(100);
}

Smb4KConfigPageUserInterface::~Smb4KConfigPageUserInterface()
{
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////
