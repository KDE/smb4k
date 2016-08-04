/***************************************************************************
    The configuration page for the settings regarding share management
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2016 by Alexander Reinholdt
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

// applications specific includes
#include "smb4kconfigpageshares.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QSpinBox>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlRequester>


Smb4KConfigPageShares::Smb4KConfigPageShares(QWidget *parent)
: QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  layout->setMargin(0);

  // Directories
  QGroupBox *directory_box = new QGroupBox(i18n("Directories"), this);

  QGridLayout *directory_layout = new QGridLayout(directory_box);
  directory_layout->setSpacing(5);

  QLabel *prefix_label = new QLabel(Smb4KSettings::self()->mountPrefixItem()->label(), directory_box);
  KUrlRequester *prefix = new KUrlRequester(directory_box);
  prefix->setMode(KFile::Directory | KFile::LocalOnly);
  prefix->setObjectName("kcfg_MountPrefix");

  prefix_label->setBuddy(prefix);
  
  QCheckBox *lowercase_subdirs = new QCheckBox(Smb4KSettings::self()->forceLowerCaseSubdirsItem()->label(), directory_box);
  lowercase_subdirs->setObjectName("kcfg_ForceLowerCaseSubdirs");

  directory_layout->addWidget(prefix_label, 0, 0, 0);
  directory_layout->addWidget(prefix, 0, 1, 0);
  directory_layout->addWidget(lowercase_subdirs, 1, 0, 1, 2, 0);

  // Behavior
  QGroupBox *behavior_box = new QGroupBox(i18n("Behavior"), this);

  QGridLayout *behavior_layout = new QGridLayout(behavior_box);
  behavior_layout->setSpacing(5);
  
  QCheckBox *remount_shares = new QCheckBox(Smb4KSettings::self()->remountSharesItem()->label(), behavior_box);
  remount_shares->setObjectName("kcfg_RemountShares");
  
  QLabel *rem_attempts_label = new QLabel(Smb4KSettings::self()->remountAttemptsItem()->label(), behavior_box);
  rem_attempts_label->setIndent(25);
  
  QSpinBox *remount_attempts = new QSpinBox(behavior_box);
  remount_attempts->setObjectName("kcfg_RemountAttempts");
  rem_attempts_label->setBuddy(remount_attempts);
  
  QLabel *rem_interval_label = new QLabel(Smb4KSettings::self()->remountIntervalItem()->label(), behavior_box);
  rem_interval_label->setIndent(25);
  
  QSpinBox *remount_interval = new QSpinBox(behavior_box);
  remount_interval->setObjectName("kcfg_RemountInterval");
  remount_interval->setSuffix(" min.");
  rem_interval_label->setBuddy(remount_interval);

  QCheckBox *unmount_all_shares = new QCheckBox(Smb4KSettings::self()->unmountSharesOnExitItem()->label(), behavior_box);
  unmount_all_shares->setObjectName("kcfg_UnmountSharesOnExit");
  
  QCheckBox *allow_foreign = new QCheckBox(Smb4KSettings::self()->unmountForeignSharesItem()->label(), behavior_box);
  allow_foreign->setObjectName("kcfg_UnmountForeignShares");

#if defined(Q_OS_LINUX)
  QCheckBox *unmount_inaccessible = new QCheckBox(Smb4KSettings::self()->forceUnmountInaccessibleItem()->label(), behavior_box);
  unmount_inaccessible->setObjectName("kcfg_ForceUnmountInaccessible");
#endif
  
  QCheckBox *retrieve_all = new QCheckBox(Smb4KSettings::self()->detectAllSharesItem()->label(), behavior_box);
  retrieve_all->setObjectName("kcfg_DetectAllShares");

#if defined(Q_OS_LINUX)
  behavior_layout->addWidget(remount_shares, 0, 0, 1, 2, 0);
  behavior_layout->addWidget(rem_attempts_label, 1, 0, 0);
  behavior_layout->addWidget(remount_attempts, 1, 1, 0);
  behavior_layout->addWidget(rem_interval_label, 2, 0, 0);
  behavior_layout->addWidget(remount_interval, 2, 1, 0);
  behavior_layout->addWidget(unmount_all_shares, 3, 0, 1, 2, 0);
  behavior_layout->addWidget(unmount_inaccessible, 4, 0, 1, 2, 0);
  behavior_layout->addWidget(allow_foreign, 5, 0, 1, 2, 0);
  behavior_layout->addWidget(retrieve_all, 6, 0, 1, 2, 0);
#else
  behavior_layout->addWidget(remount_shares, 0, 0, 1, 2, 0);
  behavior_layout->addWidget(rem_attempts_label, 1, 0, 0);
  behavior_layout->addWidget(remount_attempts, 1, 1, 0);
  behavior_layout->addWidget(rem_interval_label, 2, 0, 0);
  behavior_layout->addWidget(remount_interval, 2, 1, 0);
  behavior_layout->addWidget(unmount_all_shares, 3, 0, 1, 2, 0);
  behavior_layout->addWidget(allow_foreign, 4, 0, 1, 2, 0);
  behavior_layout->addWidget(retrieve_all, 5, 0, 1, 2, 0);
#endif

  // Checks
  QGroupBox *checks_box = new QGroupBox(i18n("Checks"), this);

  QGridLayout *checks_layout = new QGridLayout(checks_box);
  checks_layout->setSpacing(5);

  QLabel *check_interval_label = new QLabel(Smb4KSettings::self()->checkIntervalItem()->label(), checks_box);
  QSpinBox *check_interval  = new QSpinBox(checks_box);
  check_interval->setObjectName("kcfg_CheckInterval");
  check_interval->setSuffix(" ms");
  // Set the step width. If you change this, also change the TIMEOUT definition
  // in Smb4KMounter!
  check_interval->setSingleStep(50);
//   check_interval->setSliderEnabled(true);

  check_interval_label->setBuddy(check_interval);
  
  checks_layout->addWidget(check_interval_label, 0, 0, 0);
  checks_layout->addWidget(check_interval, 0, 1, 0);

  layout->addWidget(directory_box, 0, 0);
  layout->addWidget(behavior_box, 1, 0);
  layout->addWidget(checks_box, 2, 0);
  layout->addStretch(100);
}

Smb4KConfigPageShares::~Smb4KConfigPageShares()
{
}

