/***************************************************************************
    Private helper classes for Smb4KCustomOptionsManager class
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2016 by Alexander Reinholdt
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
#include "smb4kcustomoptionsmanager_p.h"
#include "smb4ksettings.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_freebsd.h"
#endif

// Qt includes
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QDialogButtonBox>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KCoreAddons/KUser>
#include <KConfigGui/KWindowConfig>
#include <KIconThemes/KIconLoader>


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog(Smb4KCustomOptions *options, QWidget *parent)
: QDialog(parent), m_options(options)
{
  setWindowTitle(i18n("Custom Options"));

  setupView();

  KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
}


Smb4KCustomOptionsDialog::~Smb4KCustomOptionsDialog()
{
}


#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KCustomOptionsDialog::setupView()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);

  QWidget *description = new QWidget(this);

  QHBoxLayout *descriptionLayout = new QHBoxLayout(description);
  descriptionLayout->setSpacing(5);
  descriptionLayout->setMargin(0);

  QLabel *pixmap = new QLabel(description);
  QPixmap mountPixmap = KDE::icon("preferences-system-network").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(mountPixmap);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = 0;

  switch (m_options->type())
  {
    case Host:
    {
      label = new QLabel(i18n("<p>Define custom options for host <b>%1</b> and all the shares it provides.</p>",
                         m_options->hostName()), description);
      break;
    }
    case Share:
    {
      label = new QLabel(i18n("<p>Define custom options for share <b>%1</b> on host <b>%2</b>.</p>",
                         m_options->shareName(), m_options->hostName()), description);
      break;
    }
    default:
    {
      label = new QLabel();
      break;
    }
  }

  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  descriptionLayout->addWidget(pixmap, 0);
  descriptionLayout->addWidget(label, Qt::AlignBottom);

  QGroupBox *general = new QGroupBox(i18n("General"), this);

  QGridLayout *generalLayout = new QGridLayout(general);
  generalLayout->setSpacing(5);

  QLabel *uncLabel = new QLabel(i18n("UNC Address:"), general);
  KLineEdit *unc = new KLineEdit(m_options->unc(), general);
  unc->setReadOnly(true);
  
  QLabel *ipLabel = new QLabel(i18n("IP Address:"), general);
  KLineEdit *ip = new KLineEdit(m_options->ip(), general);
  ip->setReadOnly(true);
  
  generalLayout->addWidget(uncLabel, 0, 0, 0);
  generalLayout->addWidget(unc, 0, 1, 0);
  generalLayout->addWidget(ipLabel, 1, 0, 0);
  generalLayout->addWidget(ip, 1, 1, 0);
  
  if (m_options->type() == Share)
  {
    m_remount = new QCheckBox(i18n("Always remount this share"), general);
    m_remount->setChecked((m_options->remount() == Smb4KCustomOptions::RemountAlways));
    generalLayout->addWidget(m_remount, 2, 0, 1, 2, 0);
  }
  else
  {
    m_remount = 0;
  }

  //
  // Tab widget with settings
  //

  QTabWidget *tabWidget = new QTabWidget(this);

  //
  // Custom options for Samba
  //

  QWidget *sambaEditors = new QWidget(tabWidget);
  
  QGridLayout *sambaEditorsLayout = new QGridLayout(sambaEditors);
  sambaEditorsLayout->setSpacing(5);

  QLabel *smbLabel = new QLabel(i18n("SMB Port:"), sambaEditors);
  m_smb_port = new QSpinBox(sambaEditors);
  m_smb_port->setRange(Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt());
  m_smb_port->setValue(m_options->smbPort() != Smb4KSettings::remoteSMBPort() ?
                       m_options->smbPort() : Smb4KSettings::remoteSMBPort());
//   m_smb_port->setSliderEnabled(true);
  smbLabel->setBuddy(m_smb_port);

  QLabel *filesystemLabel = new QLabel(i18n("Filesystem Port:"), sambaEditors);
  m_fs_port = new QSpinBox(sambaEditors);
  m_fs_port->setRange(Smb4KMountSettings::self()->remoteFileSystemPortItem()->minValue().toInt(),
                      Smb4KMountSettings::self()->remoteFileSystemPortItem()->maxValue().toInt());
  m_fs_port->setValue(m_options->fileSystemPort() != Smb4KMountSettings::remoteFileSystemPort() ?
                      m_options->fileSystemPort() : Smb4KMountSettings::remoteFileSystemPort());
//   m_fs_port->setSliderEnabled(true);
  filesystemLabel->setBuddy(m_fs_port);

  QLabel *readWriteLabel = new QLabel(i18n("Write Access:"), sambaEditors);
  m_write_access   = new KComboBox(sambaEditors);
  m_write_access->insertItem(0, Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label,
                             QVariant::fromValue<int>(Smb4KCustomOptions::ReadWrite));
  m_write_access->insertItem(1, Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label,
                             QVariant::fromValue<int>(Smb4KCustomOptions::ReadOnly));
  readWriteLabel->setBuddy(m_write_access);

  if (m_options->writeAccess() == Smb4KCustomOptions::UndefinedWriteAccess)
  {
    switch (Smb4KMountSettings::writeAccess())
    {
      case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
      {
        m_write_access->setCurrentIndex(0);
        break;
      }
      case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
      {
        m_write_access->setCurrentIndex(1);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    switch (m_options->writeAccess())
    {
      case Smb4KCustomOptions::ReadWrite:
      {
        m_write_access->setCurrentIndex(0);
        break;
      }
      case Smb4KCustomOptions::ReadOnly:
      {
        m_write_access->setCurrentIndex(1);
        break;
      }
      default:
      {
        break;
      }
    }
  }

  QLabel *securityLabel = new QLabel(i18n("Security Mode:"), sambaEditors);
  
  m_security_mode        = new KComboBox(sambaEditors);
  m_security_mode->insertItem(0, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::None).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::NoSecurityMode));
  m_security_mode->insertItem(1, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Krb5));
  m_security_mode->insertItem(2, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5i).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Krb5i));
  m_security_mode->insertItem(3, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlm).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Ntlm));
  m_security_mode->insertItem(4, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmi).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Ntlmi));
  m_security_mode->insertItem(5, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Ntlmv2));
  m_security_mode->insertItem(6, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Ntlmv2i));
  m_security_mode->insertItem(7, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmssp).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Ntlmssp));
  m_security_mode->insertItem(8, Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi).label,
                              QVariant::fromValue<int>(Smb4KCustomOptions::Ntlmsspi));
  securityLabel->setBuddy(m_security_mode);

  if (m_options->securityMode() == Smb4KCustomOptions::UndefinedSecurityMode)
  {
    switch (Smb4KMountSettings::securityMode())
    {
      case Smb4KMountSettings::EnumSecurityMode::None:
      {
        m_security_mode->setCurrentIndex(0);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5:
      {
        m_security_mode->setCurrentIndex(1);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5i:
      {
        m_security_mode->setCurrentIndex(2);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlm:
      {
        m_security_mode->setCurrentIndex(3);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
      {
        m_security_mode->setCurrentIndex(4);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
      {
        m_security_mode->setCurrentIndex(5);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
      {
        m_security_mode->setCurrentIndex(6);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
      {
        m_security_mode->setCurrentIndex(7);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
      {
        m_security_mode->setCurrentIndex(8);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    switch (m_options->securityMode())
    {
      case Smb4KCustomOptions::NoSecurityMode:
      {
        m_security_mode->setCurrentIndex(0);
        break;
      }
      case Smb4KCustomOptions::Krb5:
      {
        m_security_mode->setCurrentIndex(1);
        break;
      }
      case Smb4KCustomOptions::Krb5i:
      {
        m_security_mode->setCurrentIndex(2);
        break;
      }
      case Smb4KCustomOptions::Ntlm:
      {
        m_security_mode->setCurrentIndex(3);
        break;
      }
      case Smb4KCustomOptions::Ntlmi:
      {
        m_security_mode->setCurrentIndex(4);
        break;
      }
      case Smb4KCustomOptions::Ntlmv2:
      {
        m_security_mode->setCurrentIndex(5);
        break;
      }
      case Smb4KCustomOptions::Ntlmv2i:
      {
        m_security_mode->setCurrentIndex(6);
        break;
      }
      case Smb4KCustomOptions::Ntlmssp:
      {
        m_security_mode->setCurrentIndex(7);
        break;
      }
      case Smb4KCustomOptions::Ntlmsspi:
      {
        m_security_mode->setCurrentIndex(8);
        break;
      }
      default:
      {
        break;
      }
    }
  }

  QLabel *uidLabel = new QLabel(i18n("User ID:"), sambaEditors);
  m_user_id = new KComboBox(sambaEditors);
  uidLabel->setBuddy(m_user_id);

  // To avoid weird crashes under FreeBSD, first copy KUser::allUsers().
  QList<KUser> allUsers = KUser::allUsers();

  for (int i = 0; i < allUsers.size(); ++i)
  {
    KUser user = allUsers.at(i);
    m_user_id->insertItem(i, QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()), QVariant::fromValue<K_UID>(user.userId().nativeId()));

    if (m_options->user().userId() == user.userId())
    {
      m_user_id->setCurrentIndex(i);
    }
    else
    {
      // Do nothing
    }
  }

  QLabel *gidLabel = new QLabel(i18n("Group ID:"), sambaEditors);
  m_group_id = new KComboBox(sambaEditors);
  gidLabel->setBuddy(m_group_id);

  // To avoid weird crashes under FreeBSD, first copy KUserGroup::allGroups().
  QList<KUserGroup> allGroups = KUserGroup::allGroups();

  for (int i = 0; i < allGroups.size(); ++i)
  {
    KUserGroup group = allGroups.at(i);
    m_group_id->insertItem(i, QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()), QVariant::fromValue<K_GID>(group.groupId().nativeId()));

    if (m_options->group().groupId() == group.groupId())
    {
      m_group_id->setCurrentIndex(i);
    }
    else
    {
      // Do nothing
    }
  }

  m_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), sambaEditors);

  if (m_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos)
  {
    m_kerberos->setChecked(Smb4KSettings::useKerberos());
  }
  else
  {
    switch (m_options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        m_kerberos->setChecked(true);
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        m_kerberos->setChecked(false);
        break;
      }
      default:
      {
        break;
      }
    }
  }

  sambaEditorsLayout->addWidget(smbLabel, 0, 0, 0);
  sambaEditorsLayout->addWidget(m_smb_port, 0, 1, 0);
  sambaEditorsLayout->addWidget(filesystemLabel, 1, 0, 0);
  sambaEditorsLayout->addWidget(m_fs_port, 1, 1, 0);
  sambaEditorsLayout->addWidget(readWriteLabel, 2, 0, 0);
  sambaEditorsLayout->addWidget(m_write_access, 2, 1, 0);
  sambaEditorsLayout->addWidget(securityLabel, 3, 0, 0);
  sambaEditorsLayout->addWidget(m_security_mode, 3, 1, 0);
  sambaEditorsLayout->addWidget(uidLabel, 4, 0, 0);
  sambaEditorsLayout->addWidget(m_user_id, 4, 1, 0);
  sambaEditorsLayout->addWidget(gidLabel, 5, 0, 0);
  sambaEditorsLayout->addWidget(m_group_id, 5, 1, 0);
  sambaEditorsLayout->addWidget(m_kerberos, 6, 0, 1, 2, 0);

  tabWidget->addTab(sambaEditors, i18n("Samba"));


  //
  // Custom options for Wake-On-LAN
  //

  QWidget *wolEditors = new QWidget(tabWidget);
  
  QGridLayout *wolEditorsLayout = new QGridLayout(wolEditors);
  wolEditorsLayout->setSpacing(5);
  
  QLabel *macLabel = new QLabel(i18n("MAC Address:"), wolEditors);
  m_mac_address = new KLineEdit(m_options->macAddress(), wolEditors);
  macLabel->setBuddy(m_mac_address);
  
  // If you change the texts here, please also alter them in the config
  // dialog.
  m_send_before_scan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), wolEditors);
  m_send_before_scan->setChecked(m_options->wolSendBeforeNetworkScan());
  m_send_before_scan->setEnabled((m_options->type() == Host));
  
  m_send_before_mount = new QCheckBox(i18n("Send magic package before mounting a share"), wolEditors);
  m_send_before_mount->setChecked(m_options->wolSendBeforeMount());
  m_send_before_mount->setEnabled((m_options->type() == Host));
  
  wolEditorsLayout->addWidget(macLabel, 0, 0, 0);
  wolEditorsLayout->addWidget(m_mac_address, 0, 1, 0);
  wolEditorsLayout->addWidget(m_send_before_scan, 1, 0, 1, 2, 0);
  wolEditorsLayout->addWidget(m_send_before_mount, 2, 0, 1, 2, 0);
  wolEditorsLayout->setRowStretch(3, 100);

  tabWidget->addTab(wolEditors, i18n("Wake-On-LAN"));
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_restore_button = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_ok_button->setDefault(true);

  layout->addWidget(description, 0);
  layout->addWidget(general, 0);
  layout->addWidget(tabWidget, 0);
  layout->addWidget(buttonBox, 0);

  connect(m_smb_port, SIGNAL(valueChanged(int)), this, SLOT(slotCheckValues()));
  connect(m_fs_port, SIGNAL(valueChanged(int)), SLOT(slotCheckValues()));
  connect(m_write_access, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  connect(m_security_mode, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  connect(m_user_id, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  connect(m_group_id, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  connect(m_kerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotCheckValues()));
  connect(m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotEnableWOLFeatures(QString)));
  connect(m_send_before_scan, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_send_before_mount, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_restore_button, SIGNAL(clicked()), SLOT(slotSetDefaultValues()));
  connect(m_ok_button, SIGNAL(clicked()), SLOT(slotOKClicked()));
  connect(m_cancel_button, SIGNAL(clicked()), SLOT(reject()));
  
  wolEditors->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
  
  m_restore_button->setEnabled(!checkDefaultValues());
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KCustomOptionsDialog::setupView()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);

  QWidget *description = new QWidget(this);

  QHBoxLayout *descriptionLayout = new QHBoxLayout(description);
  descriptionLayout->setSpacing(5);
  descriptionLayout->setMargin(0);

  QLabel *pixmap = new QLabel(description);
  QPixmap mountPixmap = KDE::icon("preferences-system-network").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(mountPixmap);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = 0;

  switch (m_options->type())
  {
    case Host:
    {
      label = new QLabel(i18n("<p>Define custom options for host <b>%1</b> and all the shares it provides.</p>",
                         m_options->hostName()), description);
      break;
    }
    case Share:
    {
      label = new QLabel(i18n("<p>Define custom options for share <b>%1</b> at host <b>%2</b>.</p>",
                         m_options->shareName(), m_options->hostName()), description);
      break;
    }
    default:
    {
      label = new QLabel();
      break;
    }
  }

  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  descriptionLayout->addWidget(pixmap, 0);
  descriptionLayout->addWidget(label, Qt::AlignBottom);

  QGroupBox *general = new QGroupBox(i18n("General"), this);

  QGridLayout *generalLayout = new QGridLayout(general);
  generalLayout->setSpacing(5);

  QLabel *uncLabel = new QLabel(i18n("UNC Address:"), general);
  KLineEdit *unc = new KLineEdit(m_options->unc(), general);
  unc->setReadOnly(true);
  
  QLabel *ipLabel = new QLabel(i18n("IP Address:"), general);
  KLineEdit *ip    = new KLineEdit(m_options->ip(), general);
  ip->setReadOnly(true);
  
  generalLayout->addWidget(uncLabel, 0, 0, 0);
  generalLayout->addWidget(unc, 0, 1, 0);
  generalLayout->addWidget(ipLabel, 1, 0, 0);
  generalLayout->addWidget(ip, 1, 1, 0);
  
  if (m_options->type() == Share)
  {
    m_remount = new QCheckBox(i18n("Always remount this share"), general);
    m_remount->setChecked((m_options->remount() == Smb4KCustomOptions::RemountAlways));
    generalLayout->addWidget(m_remount, 2, 0, 1, 2, 0);
  }
  else
  {
    m_remount = 0;
  }
  
  //
  // Tab widget with settings
  //

  QTabWidget *tabWidget = new QTabWidget(this);

  //
  // Custom options for Samba
  //

  QWidget *sambaEditors = new QWidget(tabWidget);
  
  QGridLayout *sambaEditorsLayout = new QGridLayout(sambaEditors);
  sambaEditorsLayout->setSpacing(5);

  QLabel *smbLabel = new QLabel(i18n("SMB Port:"), sambaEditors);
  m_smb_port = new QSpinBox(sambaEditors);
  m_smb_port->setValue(m_options->smbPort() != Smb4KSettings::remoteSMBPort() ?
                       m_options->smbPort() : Smb4KSettings::remoteSMBPort());
  m_smb_port->setRange(Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt());
//   m_smb_port->setSliderEnabled(true);
  smbLabel->setBuddy(m_smb_port);

  QLabel *uidLabel = new QLabel(i18n("User ID:"), sambaEditors);
  m_user_id = new KComboBox(sambaEditors);
  uidLabel->setBuddy(m_user_id);

  // To avoid weird crashes under FreeBSD, first copy KUser::allUsers().
  QList<KUser> allUsers = KUser::allUsers();

  for (int i = 0; i < allUsers.size(); ++i)
  {
    KUser user = allUsers.at(i);
    m_user_id->insertItem(i, QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()), QVariant::fromValue<K_UID>(user.userId().nativeId()));

    if (m_options->user().userId() == user.userId())
    {
      m_user_id->setCurrentIndex(i);
    }
    else
    {
      // Do nothing
    }
  }

  QLabel *gidLabel = new QLabel(i18n("Group ID:"), sambaEditors);
  m_group_id = new KComboBox(sambaEditors);
  gidLabel->setBuddy(m_group_id);

  // To avoid weird crashes under FreeBSD, first copy KUserGroup::allGroups().
  QList<KUserGroup> allGroups = KUserGroup::allGroups();

  for (int i = 0; i < allGroups.size(); ++i)
  {
    KUserGroup group = allGroups.at(i);
    m_group_id->insertItem(i, QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()), QVariant::fromValue<K_GID>(group.groupId().nativeId()));

    if (m_options->group().groupId() == group.groupId())
    {
      m_group_id->setCurrentIndex(i);
    }
    else
    {
      // Do nothing
    }
  }

  m_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), sambaEditors);

  if (m_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos)
  {
    m_kerberos->setChecked(Smb4KSettings::useKerberos());
  }
  else
  {
    switch (m_options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        m_kerberos->setChecked(true);
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        m_kerberos->setChecked(false);
        break;
      }
      default:
      {
        break;
      }
    }
  }

  sambaEditorsLayout->addWidget(smbLabel, 0, 0, 0);
  sambaEditorsLayout->addWidget(m_smb_port, 0, 1, 0);
  sambaEditorsLayout->addWidget(uidLabel, 1, 0, 0);
  sambaEditorsLayout->addWidget(m_user_id, 1, 1, 0);
  sambaEditorsLayout->addWidget(gidLabel, 2, 0, 0);
  sambaEditorsLayout->addWidget(m_group_id, 2, 1, 0);
  sambaEditorsLayout->addWidget(m_kerberos, 3, 0, 1, 2, 0);

  tabWidget->addTab(sambaEditors, i18n("Samba"));


  //
  // Custom options for Wake-On-LAN
  //

  QWidget *wolEditors = new QWidget(tabWidget);
  
  QGridLayout *wolEditorsLayout = new QGridLayout(wolEditors);
  wolEditorsLayout->setSpacing(5);
  
  QLabel *macLabel = new QLabel(i18n("MAC Address:"), wolEditors);
  m_mac_address = new KLineEdit(m_options->macAddress(), wolEditors);
  macLabel->setBuddy(m_mac_address);
  
  // If you change the texts here, please also alter them in the config
  // dialog.
  m_send_before_scan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), wolEditors);
  m_send_before_scan->setChecked(m_options->wolSendBeforeNetworkScan());
  m_send_before_scan->setEnabled((m_options->type() == Host));
  
  m_send_before_mount = new QCheckBox(i18n("Send magic package before mounting a share"), wolEditors);
  m_send_before_mount->setChecked(m_options->wolSendBeforeMount());
  m_send_before_mount->setEnabled((m_options->type() == Host));
  
  wolEditorsLayout->addWidget(macLabel, 0, 0, 0);
  wolEditorsLayout->addWidget(m_mac_address, 0, 1, 0);
  wolEditorsLayout->addWidget(m_send_before_scan, 1, 0, 1, 2, 0);
  wolEditorsLayout->addWidget(m_send_before_mount, 2, 0, 1, 2, 0);
  wolEditorsLayout->setRowStretch(3, 100);

  tabWidget->addTab(wolEditors, i18n("Wake-On-LAN"));
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_restore_button = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_ok_button->setDefault(true);

  layout->addWidget(description, 0);
  layout->addWidget(general, 0);
  layout->addWidget(tabWidget, 0);
  layout->addWidget(buttonBox, 0);

  connect(m_smb_port, SIGNAL(valueChanged(int)), SLOT(slotCheckValues()));
  connect(m_user_id, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  connect(m_group_id, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  connect(m_kerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotCheckValues()));
  connect(m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotEnableWOLFeatures(QString)));
  connect(m_send_before_scan, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_send_before_mount, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_restore_button, SIGNAL(clicked()), SLOT(slotSetDefaultValues()));
  connect(m_ok_button, SIGNAL(clicked()), SLOT(slotOKClicked()));
  connect(m_cancel_button, SIGNAL(clicked()), SLOT(reject()));
  
  wolEditors->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
  
  m_restore_button->setEnabled(!checkDefaultValues());
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsDialog::setupView()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);

  QWidget *description = new QWidget(this);

  QHBoxLayout *descriptionLayout = new QHBoxLayout(description);
  descriptionLayout->setSpacing(5);
  descriptionLayout->setMargin(0);

  QLabel *pixmap = new QLabel(description);
  QPixmap mountPixmap = KDE::icon("preferences-system-network").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(mountPixmap);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = 0;

  switch (m_options->type())
  {
    case Host:
    {
      label = new QLabel(i18n("<p>Define custom options for host <b>%1</b> and all the shares it provides.</p>",
                         m_options->hostName()), description);
      break;
    }
    case Share:
    {
      label = new QLabel(i18n("<p>Define custom options for share <b>%1</b> at host <b>%2</b>.</p>",
                         m_options->shareName(), m_options->hostName()), description);
      break;
    }
    default:
    {
      label = new QLabel();
      break;
    }
  }

  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  descriptionLayout->addWidget(pixmap, 0);
  descriptionLayout->addWidget(label, Qt::AlignBottom);

  QGroupBox *general = new QGroupBox(i18n("General"), this);

  QGridLayout *generalLayout = new QGridLayout(general);
  generalLayout->setSpacing(5);

  QLabel *uncLabel = new QLabel(i18n("UNC Address:"), general);
  KLineEdit *unc = new KLineEdit(m_options->unc(), general);
  unc->setReadOnly(true);
  
  QLabel *ipLabel = new QLabel(i18n("IP Address:"), general);
  KLineEdit *ip = new KLineEdit(m_options->ip(), general);
  ip->setReadOnly(true);
  
  generalLayout->addWidget(uncLabel, 0, 0, 0);
  generalLayout->addWidget(unc, 0, 1, 0);
  generalLayout->addWidget(ipLabel, 1, 0, 0);
  generalLayout->addWidget(ip, 1, 1, 0);
  
  //
  // Tab widget with settings
  //

  QTabWidget *tabWidget = new QTabWidget(this);

  //
  // Custom options for Samba
  //

  QWidget *sambaEditors = new QWidget(tabWidget);
  
  QGridLayout *sambaEditorsLayout = new QGridLayout(sambaEditors);
  sambaEditorsLayout->setSpacing(5);

  QLabel *smbLabel = new QLabel(i18n("SMB Port:"), sambaEditors);
  m_smb_port = new QSpinBox(sambaEditors);
  m_smb_port->setValue(m_options->smbPort() != Smb4KSettings::remoteSMBPort() ?
                       m_options->smbPort() : Smb4KSettings::remoteSMBPort());
  m_smb_port->setRange(Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt());
//   m_smb_port->setSliderEnabled(true);
  smbLabel->setBuddy(m_smb_port);

  m_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), sambaEditors);

  if (m_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos)
  {
    m_kerberos->setChecked(Smb4KSettings::useKerberos());
  }
  else
  {
    switch (m_options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        m_kerberos->setChecked(true);
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        m_kerberos->setChecked(false);
        break;
      }
      default:
      {
        break;
      }
    }
  }

  sambaEditorsLayout->addWidget(smbLabel, 0, 0, 0);
  sambaEditorsLayout->addWidget(m_smb_port, 0, 1, 0);
  sambaEditorsLayout->addWidget(m_kerberos, 1, 0, 1, 2, 0);

  tabWidget->addTab(sambaEditors, i18n("Samba"));


  //
  // Custom options for Wake-On-LAN
  //

  QWidget *wolEditors = new QWidget(tabWidget);
  
  QGridLayout *wolEditorsLayout = new QGridLayout(wolEditors);
  wolEditorsLayout->setSpacing(5);
  
  QLabel *macLabel = new QLabel(i18n("MAC Address:"), wolEditors);
  m_mac_address = new KLineEdit(m_options->macAddress(), wolEditors);
  macLabel->setBuddy(m_mac_address);
  
  // If you change the texts here, please also alter them in the config
  // dialog.
  m_send_before_scan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), wolEditors);
  m_send_before_scan->setChecked(m_options->wolSendBeforeNetworkScan());
  m_send_before_scan->setEnabled((m_options->type() == Host));
  
  m_send_before_mount = new QCheckBox(i18n("Send magic package before mounting a share"), wolEditors);
  m_send_before_mount->setChecked(m_options->wolSendBeforeMount());
  m_send_before_mount->setEnabled((m_options->type() == Host));
  
  wolEditorsLayout->addWidget(macLabel, 0, 0, 0);
  wolEditorsLayout->addWidget(m_mac_address, 0, 1, 0);
  wolEditorsLayout->addWidget(m_send_before_scan, 1, 0, 1, 2, 0);
  wolEditorsLayout->addWidget(m_send_before_mount, 2, 0, 1, 2, 0);
  wolEditorsLayout->setRowStretch(3, 100);

  tabWidget->addTab(wolEditors, i18n("Wake-On-LAN"));
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_restore_button = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_ok_button->setDefault(true);

  layout->addWidget(description, 0);
  layout->addWidget(general, 0);
  layout->addWidget(tabWidget, 0);
  layout->addWidget(buttonBox, 0);

  connect(m_smb_port, SIGNAL(valueChanged(int)), SLOT(slotCheckValues()));
  connect(m_kerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotCheckValues()));
  connect(m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotEnableWOLFeatures(QString)));
  connect(m_send_before_scan, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_send_before_mount, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(m_restore_button, SIGNAL(clicked()), SLOT(slotSetDefaultValues()));
  connect(m_ok_button, SIGNAL(clicked()), SLOT(slotOKClicked()));
  connect(m_cancel_button, SIGNAL(clicked()), SLOT(reject()));
  
  wolEditors->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
  
  m_restore_button->setEnabled(!checkDefaultValues());
}
#endif


#if defined(Q_OS_LINUX)
//
// Linux
//
bool Smb4KCustomOptionsDialog::checkDefaultValues()
{
  if (m_options->type() == Share)
  {
    if (m_remount->isChecked() != false)
    {
      return false;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }

  if (m_smb_port->value() != Smb4KSettings::remoteSMBPort())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (m_fs_port->value() != Smb4KMountSettings::remoteFileSystemPort())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (QString::compare(m_security_mode->currentText(),
      Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::self()->securityMode()).label,
      Qt::CaseInsensitive) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (QString::compare(m_write_access->currentText(),
      Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::self()->writeAccess()).label,
      Qt::CaseInsensitive) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  K_UID uid = (K_UID)m_user_id->itemData(m_user_id->currentIndex()).toInt();

  if (uid != (K_UID)Smb4KMountSettings::userID().toInt())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  K_GID gid = (K_GID)m_group_id->itemData(m_group_id->currentIndex()).toInt();

  if (gid != (K_GID)Smb4KMountSettings::groupID().toInt())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (m_kerberos->isChecked() != Smb4KSettings::useKerberos())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (m_options->type() == Host)
  {
    if (!m_mac_address->text().isEmpty())
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    if (m_send_before_scan->isChecked())
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    if (m_send_before_mount->isChecked())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }

  return true;
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD or NetBSD
//
bool Smb4KCustomOptionsDialog::checkDefaultValues()
{
  if (m_options->type() == Share)
  {
    if (m_remount->isChecked() != false)
    {
      return false;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }

  if (m_smb_port->value() != Smb4KSettings::remoteSMBPort())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  K_UID uid = (K_UID)m_user_id->itemData(m_user_id->currentIndex()).toInt();

  if (uid != (K_UID)Smb4KMountSettings::userID().toInt())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  K_GID gid = (K_GID)m_group_id->itemData(m_group_id->currentIndex()).toInt();

  if (gid != (K_GID)Smb4KMountSettings::groupID().toInt())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (m_kerberos->isChecked() != Smb4KSettings::useKerberos())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (m_options->type() == Host)
  {
    if (!m_mac_address->text().isEmpty())
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    if (m_send_before_scan->isChecked())
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    if (m_send_before_mount->isChecked())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }

  return true;
}
#else
//
// Generic (without mount options)
//
bool Smb4KCustomOptionsDialog::checkDefaultValues()
{
  if (m_smb_port->value() != Smb4KSettings::remoteSMBPort())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (m_kerberos->isChecked() != Smb4KSettings::useKerberos())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if (m_options->type() == Host)
  {
    if (!m_mac_address->text().isEmpty())
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    if (m_send_before_scan->isChecked())
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    if (m_send_before_mount->isChecked())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }

  return true;
}
#endif


#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KCustomOptionsDialog::setDefaultValues()
{
  if (m_options->type() == Share)
  {
    m_remount->setChecked(false);
  }
  else
  {
    // Do nothing
  }

  m_smb_port->setValue(Smb4KSettings::remoteSMBPort());
  m_fs_port->setValue(Smb4KMountSettings::remoteFileSystemPort());

  switch (Smb4KMountSettings::securityMode())
  {
    case Smb4KMountSettings::EnumSecurityMode::None:
    {
      m_security_mode->setCurrentIndex(0);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Krb5:
    {
      m_security_mode->setCurrentIndex(1);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Krb5i:
    {
      m_security_mode->setCurrentIndex(2);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Ntlm:
    {
      m_security_mode->setCurrentIndex(3);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
    {
      m_security_mode->setCurrentIndex(4);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
    {
      m_security_mode->setCurrentIndex(5);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
    {
      m_security_mode->setCurrentIndex(6);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
    {
      m_security_mode->setCurrentIndex(7);
      break;
    }
    case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
    {
      m_security_mode->setCurrentIndex(8);
      break;
    }
    default:
    {
      break;
    }
  }

  switch (Smb4KMountSettings::writeAccess())
  {
    case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
    {
      m_write_access->setCurrentIndex(0);
      break;
    }
    case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
    {
      m_write_access->setCurrentIndex(1);
      break;
    }
    default:
    {
      break;
    }
  }

  for (int i = 0; i < m_user_id->count(); ++i)
  {
    if (m_user_id->itemData(i).toInt() == Smb4KMountSettings::userID().toInt())
    {
      m_user_id->setCurrentIndex(i);
      break;
    }
    else
    {
      continue;
    }
  }

  for (int i = 0; i < m_group_id->count(); ++i)
  {
    if (m_group_id->itemData(i).toInt() == Smb4KMountSettings::groupID().toInt())
    {
      m_group_id->setCurrentIndex(i);
      break;
    }
    else
    {
      continue;
    }
  }

  m_kerberos->setChecked(Smb4KSettings::self()->useKerberos());

  if (m_options->type() == Host)
  {
    m_mac_address->clear();
    m_send_before_scan->setChecked(false);
    m_send_before_mount->setChecked(false);
  }
  else
  {
    // Do nothing, because with shares these widgets are
    // disabled.
  }
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD or NetBSD
//
void Smb4KCustomOptionsDialog::setDefaultValues()
{
  if (m_options->type() == Share)
  {
    m_remount->setChecked(false);
  }
  else
  {
    // Do nothing
  }

  m_smb_port->setValue(Smb4KSettings::remoteSMBPort());

  for (int i = 0; i < m_user_id->count(); ++i)
  {
    if (m_user_id->itemData(i).toInt() == Smb4KMountSettings::userID().toInt())
    {
      m_user_id->setCurrentIndex(i);
      break;
    }
    else
    {
      continue;
    }
  }

  for (int i = 0; i < m_group_id->count(); ++i)
  {
    if (m_group_id->itemData(i).toInt() == Smb4KMountSettings::groupID().toInt())
    {
      m_group_id->setCurrentIndex(i);
      break;
    }
    else
    {
      continue;
    }
  }

  m_kerberos->setChecked(Smb4KSettings::self()->useKerberos());

  if (m_options->type() == Host)
  {
    m_mac_address->clear();
    m_send_before_scan->setChecked(false);
    m_send_before_mount->setChecked(false);
  }
  else
  {
    // Do nothing, because with shares these widgets are
    // disabled.
  }
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsDialog::setDefaultValues()
{
  m_smb_port->setValue(Smb4KSettings::remoteSMBPort());

  m_kerberos->setChecked(Smb4KSettings::self()->useKerberos());

  if (m_options->type() == Host)
  {
    m_mac_address->clear();
    m_send_before_scan->setChecked(false);
    m_send_before_mount->setChecked(false);
  }
  else
  {
    // Do nothing, because with shares these widgets are
    // disabled.
  }
}
#endif


#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KCustomOptionsDialog::saveValues()
{
  if (m_options->type() == Share)
  {
    if (m_remount->isChecked())
    {
      m_options->setRemount(Smb4KCustomOptions::RemountAlways);
    }
    else
    {
      m_options->setRemount(Smb4KCustomOptions::RemountNever);
    }
  }
  else
  {
    // Do nothing
  }
  
  m_options->setSMBPort(m_smb_port->value());
  m_options->setFileSystemPort(m_fs_port->value());
  m_options->setWriteAccess((Smb4KCustomOptions::WriteAccess)m_write_access->itemData(m_write_access->currentIndex()).toInt());
  m_options->setSecurityMode((Smb4KCustomOptions::SecurityMode)m_security_mode->itemData(m_security_mode->currentIndex()).toInt());
  m_options->setUser(KUser(m_user_id->itemData(m_user_id->currentIndex()).toInt()));
  m_options->setGroup(KUserGroup(m_group_id->itemData(m_group_id->currentIndex()).toInt()));

  if (m_kerberos->isChecked())
  {
    m_options->setUseKerberos(Smb4KCustomOptions::UseKerberos);
  }
  else
  {
    m_options->setUseKerberos(Smb4KCustomOptions::NoKerberos);
  }
  
  m_options->setMACAddress(m_mac_address->text());
  m_options->setWOLSendBeforeNetworkScan(m_send_before_scan->isChecked());
  m_options->setWOLSendBeforeMount(m_send_before_mount->isChecked());

  KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD or NetBSD
//
void Smb4KCustomOptionsDialog::saveValues()
{
  if (m_options->type() == Share)
  {
    if (m_remount->isChecked())
    {
      m_options->setRemount(Smb4KCustomOptions::RemountAlways);
    }
    else
    {
      m_options->setRemount(Smb4KCustomOptions::RemountNever);
    }
  }
  else
  {
    // Do nothing
  }
  
  m_options->setSMBPort(m_smb_port->value());
  m_options->setUser(KUser(m_user_id->itemData(m_user_id->currentIndex()).toInt()));
  m_options->setGroup(KUserGroup(m_group_id->itemData(m_group_id->currentIndex()).toInt()));

  if (m_kerberos->isChecked())
  {
    m_options->setUseKerberos(Smb4KCustomOptions::UseKerberos);
  }
  else
  {
    m_options->setUseKerberos(Smb4KCustomOptions::NoKerberos);
  }
  
  m_options->setMACAddress(m_mac_address->text());
  m_options->setWOLSendBeforeNetworkScan(m_send_before_scan->isChecked());
  m_options->setWOLSendBeforeMount(m_send_before_mount->isChecked());

  KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsDialog::saveValues()
{
  m_options->setSMBPort(m_smb_port->value());

  if (m_kerberos->isChecked())
  {
    m_options->setUseKerberos(Smb4KCustomOptions::UseKerberos);
  }
  else
  {
    m_options->setUseKerberos(Smb4KCustomOptions::NoKerberos);
  }
  
  m_options->setMACAddress(m_mac_address->text());
  m_options->setWOLSendBeforeNetworkScan(m_send_before_scan->isChecked());
  m_options->setWOLSendBeforeMount(m_send_before_mount->isChecked());

  KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
}
#endif


void Smb4KCustomOptionsDialog::slotSetDefaultValues()
{
  setDefaultValues();
}


void Smb4KCustomOptionsDialog::slotCheckValues()
{
  m_restore_button->setEnabled(!checkDefaultValues());
}


void Smb4KCustomOptionsDialog::slotOKClicked()
{
  saveValues();
  accept();
}


void Smb4KCustomOptionsDialog::slotEnableWOLFeatures(const QString &mac)
{
  QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
    
  m_send_before_scan->setEnabled(exp.exactMatch(mac));
  m_send_before_mount->setEnabled(exp.exactMatch(mac));
}

