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


Smb4KConfigPageSamba::Smb4KConfigPageSamba(QWidget *parent) : QTabWidget(parent)
{
  //
  // Common
  //
  QWidget *general_tab = new QWidget(this);

  QVBoxLayout *general_layout = new QVBoxLayout(general_tab);
  general_layout->setSpacing(5);
  general_layout->setMargin(0);

  // Common options
  QGroupBox *general_box = new QGroupBox(i18n("Common Options"), general_tab);

  QGridLayout *gen_opt_layout = new QGridLayout(general_box);
  gen_opt_layout->setSpacing(5);

  QLabel *netbios_name_label = new QLabel(Smb4KSettings::self()->netBIOSNameItem()->label(), general_box);
  KLineEdit *netbios_name = new KLineEdit(general_box);
  netbios_name->setObjectName("kcfg_NetBIOSName");
  netbios_name_label->setBuddy(netbios_name);

  QLabel *domain_label = new QLabel(Smb4KSettings::self()->domainNameItem()->label(), general_box);
  KLineEdit *domain = new KLineEdit(general_box);
  domain->setObjectName("kcfg_DomainName");
  domain_label->setBuddy(domain);
  
  QLabel *socket_options_label = new QLabel(Smb4KSettings::self()->socketOptionsItem()->label(), general_box);
  KLineEdit *socket_options = new KLineEdit(general_box);
  socket_options->setObjectName("kcfg_SocketOptions");
  socket_options_label->setBuddy(socket_options);
  
  QLabel *netbios_scope_label = new QLabel(Smb4KSettings::self()->netBIOSScopeItem()->label(), general_box);
  KLineEdit *netbios_scope = new KLineEdit(general_box);
  netbios_scope->setObjectName("kcfg_NetBIOSScope");
  netbios_scope_label->setBuddy(netbios_scope);
  
  QCheckBox *useRemoteSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), general_box);
  useRemoteSmbPort->setObjectName("kcfg_UseRemoteSmbPort");

  QSpinBox *remote_smb_port = new QSpinBox(general_box);
  remote_smb_port->setObjectName("kcfg_RemoteSmbPort");
//   remote_smb_port->setSliderEnabled(true); 

  gen_opt_layout->addWidget(netbios_name_label, 0, 0, 0);
  gen_opt_layout->addWidget(netbios_name, 0, 1, 0);
  gen_opt_layout->addWidget(domain_label, 2, 0, 0);
  gen_opt_layout->addWidget(domain, 2, 1, 0);
  gen_opt_layout->addWidget(socket_options_label, 3, 0, 0);
  gen_opt_layout->addWidget(socket_options, 3, 1, 0);
  gen_opt_layout->addWidget(netbios_scope_label, 4, 0, 0);
  gen_opt_layout->addWidget(netbios_scope, 4, 1, 0);
  gen_opt_layout->addWidget(useRemoteSmbPort, 5, 0, 0);
  gen_opt_layout->addWidget(remote_smb_port, 5, 1, 0);

  // Common client options
  QGroupBox *auth_box = new QGroupBox(i18n("Authentication"), general_tab);

  QGridLayout *auth_layout = new QGridLayout(auth_box);
  auth_layout->setSpacing(5);

  QCheckBox *auth_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), auth_box);
  auth_kerberos->setObjectName("kcfg_UseKerberos");

  QCheckBox *auth_machine_acc = new QCheckBox(Smb4KSettings::self()->machineAccountItem()->label(), auth_box);
  auth_machine_acc->setObjectName("kcfg_MachineAccount");

  QCheckBox *use_ccache = new QCheckBox(Smb4KSettings::self()->useWinbindCCacheItem()->label(), auth_box);
  use_ccache->setObjectName("kcfg_UseWinbindCCache");

  auth_layout->addWidget(auth_kerberos, 0, 0, 0);
  auth_layout->addWidget(auth_machine_acc, 0, 1, 0);
  auth_layout->addWidget(use_ccache, 1, 0, 0);
  
  //
  // Security group box
  // 
  QGroupBox *securityBox = new QGroupBox(i18n("Security"), general_tab);
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

  // Signing state
  QLabel *signing_state_label = new QLabel(Smb4KSettings::self()->signingStateItem()->label(), securityBox);
  KComboBox *signing_state = new KComboBox(securityBox);
  signing_state->setObjectName("kcfg_SigningState");
  signing_state->insertItem(Smb4KSettings::EnumSigningState::None,
                             Smb4KSettings::self()->signingStateItem()->choices().value(Smb4KSettings::EnumSigningState::None).label);
  signing_state->insertItem(Smb4KSettings::EnumSigningState::On,
                             Smb4KSettings::self()->signingStateItem()->choices().value(Smb4KSettings::EnumSigningState::On).label);
  signing_state->insertItem(Smb4KSettings::EnumSigningState::Off,
                             Smb4KSettings::self()->signingStateItem()->choices().value(Smb4KSettings::EnumSigningState::Off).label);
  signing_state->insertItem(Smb4KSettings::EnumSigningState::Required,
                             Smb4KSettings::self()->signingStateItem()->choices().value(Smb4KSettings::EnumSigningState::Required).label);
  signing_state_label->setBuddy(signing_state);

  // Encrypt SMB transport
  QCheckBox *encrypt_transport = new QCheckBox(Smb4KSettings::self()->encryptSMBTransportItem()->label(), securityBox);
  encrypt_transport->setObjectName("kcfg_EncryptSMBTransport");

  securityBoxLayout->addWidget(useEncryptionLevel, 0, 0, 0);
  securityBoxLayout->addWidget(encryptionLevel, 0, 1, 0);
  securityBoxLayout->addWidget(signing_state_label, 1, 0, 0);
  securityBoxLayout->addWidget(signing_state, 1, 1, 0);
  securityBoxLayout->addWidget(encrypt_transport, 2, 0, 1, 2, 0);

  general_layout->addWidget(general_box);
  general_layout->addWidget(auth_box);
  general_layout->addWidget(securityBox);
  general_layout->addStretch(100);

  insertTab(GeneralTab, general_tab, i18n("Common Settings"));

  //
  // Options for the client programs
  //
  QWidget *clients_tab = new QWidget(this);

  QVBoxLayout *client_layout = new QVBoxLayout(clients_tab);
  client_layout->setSpacing(5);
  client_layout->setMargin(0);

  // 'smbclient' program
  QGroupBox *smbclient_box = new QGroupBox(i18n("smbclient"), clients_tab);

  QGridLayout *smbclient_layout = new QGridLayout(smbclient_box);
  smbclient_layout->setSpacing(5);

  QLabel *name_resolve_label = new QLabel(Smb4KSettings::self()->nameResolveOrderItem()->label(), smbclient_box);

  KLineEdit *name_resolve = new KLineEdit(smbclient_box);
  name_resolve->setObjectName("kcfg_NameResolveOrder");

  name_resolve_label->setBuddy(name_resolve);

  QLabel *buffer_size_label = new QLabel(Smb4KSettings::self()->bufferSizeItem()->label(), smbclient_box);

  QSpinBox *buffer_size = new QSpinBox(smbclient_box);
  buffer_size->setObjectName("kcfg_BufferSize");
  buffer_size->setSuffix(i18n(" Bytes"));
//   buffer_size->setSliderEnabled(true);

  buffer_size_label->setBuddy(buffer_size);

  smbclient_layout->addWidget(name_resolve_label, 0, 0, 0);
  smbclient_layout->addWidget(name_resolve, 0, 1, 0);
  smbclient_layout->addWidget(buffer_size_label, 2, 0, 0);
  smbclient_layout->addWidget(buffer_size, 2, 1, 0);

  // 'smbtree' program
  QGroupBox *smbtree_box = new QGroupBox(i18n("smbtree"), clients_tab);

  QGridLayout *smbtree_layout = new QGridLayout(smbtree_box);
  smbtree_layout->setSpacing(5);

  QCheckBox *smbtree_bcasts = new QCheckBox(Smb4KSettings::self()->smbtreeSendBroadcastsItem()->label(), smbtree_box);
  smbtree_bcasts->setObjectName("kcfg_SmbtreeSendBroadcasts");

  smbtree_layout->addWidget(smbtree_bcasts, 0, 0, 0);

  client_layout->addWidget(smbclient_box);
  client_layout->addWidget(smbtree_box);
  client_layout->addStretch(100);

  insertTab(ClientProgramsTab, clients_tab, i18n("Utility Programs"));
}


Smb4KConfigPageSamba::~Smb4KConfigPageSamba()
{
}

