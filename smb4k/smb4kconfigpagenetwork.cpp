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
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>


Smb4KConfigPageNetwork::Smb4KConfigPageNetwork(QWidget *parent) : QWidget(parent)
{
  //
  // Layout
  // 
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  layout->setMargin(0);

  // 
  // Authentication group box
  // 
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), this);
  QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);
  authenticationBoxLayout->setSpacing(5);

  QCheckBox *masterBrowsersRequireAuth = new QCheckBox(Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(), authenticationBox);
  masterBrowsersRequireAuth->setObjectName("kcfg_MasterBrowsersRequireAuth");

  authenticationBoxLayout->addWidget(masterBrowsersRequireAuth, 0);

  layout->addWidget(authenticationBox, 0);
  
  // 
  // Behavior group box
  // 
  QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), this);
  QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);
  behaviorBoxLayout->setSpacing(5);
  
  QCheckBox *detectPrinters = new QCheckBox(Smb4KSettings::self()->detectPrinterSharesItem()->label(), behaviorBox);
  detectPrinters->setObjectName("kcfg_DetectPrinterShares");

  QCheckBox *detectHiddenShares = new QCheckBox(Smb4KSettings::self()->detectHiddenSharesItem()->label(), behaviorBox);
  detectHiddenShares->setObjectName("kcfg_DetectHiddenShares");  
  
  QCheckBox *previewHiddenItems = new QCheckBox(Smb4KSettings::self()->previewHiddenItemsItem()->label(), behaviorBox);
  previewHiddenItems->setObjectName("kcfg_PreviewHiddenItems");
  
  behaviorBoxLayout->addWidget(detectPrinters, 0, 0, 0);
  behaviorBoxLayout->addWidget(detectHiddenShares, 0, 1, 0);
  behaviorBoxLayout->addWidget(previewHiddenItems, 1, 0, 0);
  
  layout->addWidget(behaviorBox, 0);
  
  // 
  // Wake-On-LAN group box
  // 
  QGroupBox *wakeOnLanBox = new QGroupBox(i18n("Wake-On-LAN"), this);
  QGridLayout *wakeOnLanBoxLayout = new QGridLayout(wakeOnLanBox);
  wakeOnLanBoxLayout->setSpacing(5);
  
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
  
  layout->addWidget(wakeOnLanBox, 0);
  layout->addStretch(100);
}


Smb4KConfigPageNetwork::~Smb4KConfigPageNetwork()
{
}

