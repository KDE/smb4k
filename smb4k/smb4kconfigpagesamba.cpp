/***************************************************************************
    This is the configuration page for the Samba settings of Smb4K
                             -------------------
    begin                : Mo Jan 26 2004
    copyright            : (C) 2004-2017 by Alexander Reinholdt
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
#include "smb4kconfigpagesamba.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>


Smb4KConfigPageSamba::Smb4KConfigPageSamba(QWidget *parent) : QWidget(parent)
{
  //
  // The layout
  //
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  layout->setMargin(0);

  // 
  // Common options group box
  // 
  QGroupBox *commonBox = new QGroupBox(i18n("Common Options"), this);

  QGridLayout *commonBoxLayout = new QGridLayout(commonBox);
  commonBoxLayout->setSpacing(5);

  QLabel *nebiosNameLabel = new QLabel(Smb4KSettings::self()->netBIOSNameItem()->label(), commonBox);
  KLineEdit *netbiosName = new KLineEdit(commonBox);
  netbiosName->setObjectName("kcfg_NetBIOSName");
  nebiosNameLabel->setBuddy(netbiosName);

  QLabel *domainLabel = new QLabel(Smb4KSettings::self()->domainNameItem()->label(), commonBox);
  KLineEdit *domain = new KLineEdit(commonBox);
  domain->setObjectName("kcfg_DomainName");
  domainLabel->setBuddy(domain);
  
  QCheckBox *useRemoteSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonBox);
  useRemoteSmbPort->setObjectName("kcfg_UseRemoteSmbPort");

  QSpinBox *remoteSmbPort = new QSpinBox(commonBox);
  remoteSmbPort->setObjectName("kcfg_RemoteSmbPort");
//   remoteSmbPort->setSliderEnabled(true); 

  commonBoxLayout->addWidget(nebiosNameLabel, 0, 0, 0);
  commonBoxLayout->addWidget(netbiosName, 0, 1, 0);
  commonBoxLayout->addWidget(domainLabel, 2, 0, 0);
  commonBoxLayout->addWidget(domain, 2, 1, 0);
  commonBoxLayout->addWidget(useRemoteSmbPort, 3, 0, 0);
  commonBoxLayout->addWidget(remoteSmbPort, 3, 1, 0);

  // 
  // Authentication group box
  // 
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), this);

  QGridLayout *authenticationBoxLayout = new QGridLayout(authenticationBox);
  authenticationBoxLayout->setSpacing(5);

  QCheckBox *auth_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
  auth_kerberos->setObjectName("kcfg_UseKerberos");

  QCheckBox *use_ccache = new QCheckBox(Smb4KSettings::self()->useWinbindCCacheItem()->label(), authenticationBox);
  use_ccache->setObjectName("kcfg_UseWinbindCCache");

  authenticationBoxLayout->addWidget(auth_kerberos, 0, 0, 0);
  authenticationBoxLayout->addWidget(use_ccache, 0, 1, 0);
  
  //
  // Security group box
  // 
  QGroupBox *securityBox = new QGroupBox(i18n("Security"), this);
  QGridLayout *securityBoxLayout = new QGridLayout(securityBox);
  securityBoxLayout->setSpacing(5);
  
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

  securityBoxLayout->addWidget(useEncryptionLevel, 0, 0, 0);
  securityBoxLayout->addWidget(encryptionLevel, 0, 1, 0);

  layout->addWidget(commonBox);
  layout->addWidget(authenticationBox);
  layout->addWidget(securityBox);
  layout->addStretch(100);
}


Smb4KConfigPageSamba::~Smb4KConfigPageSamba()
{
}

