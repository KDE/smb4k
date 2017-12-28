/***************************************************************************
    The configuration page for the mount options
                             -------------------
    begin                : So Mär 22 2015
    copyright            : (C) 2015-2017 by Alexander Reinholdt
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
#include "smb4kconfigpagemounting.h"
#include "core/smb4kglobal.h"

#if defined(Q_OS_LINUX)
#include "core/smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "core/smb4kmountsettings_freebsd.h"
#endif

// Qt includes
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QCheckBox>
#include <QInputDialog>
#include <QSpinBox>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KMessageBox>

using namespace Smb4KGlobal;


Smb4KConfigPageMounting::Smb4KConfigPageMounting(QWidget* parent): QTabWidget(parent)
{
  setupWidget();
}


Smb4KConfigPageMounting::~Smb4KConfigPageMounting()
{
}


#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KConfigPageMounting::setupWidget()
{
  //
  // Common options
  //
  QWidget *commonTab = new QWidget(this);
  QVBoxLayout *commonTabLayout = new QVBoxLayout(commonTab);
  commonTabLayout->setSpacing(5);
  commonTabLayout->setMargin(0);

  QGroupBox *commonOptions = new QGroupBox(i18n("Common Options"), commonTab);

  QGridLayout *commonOptionsLayout = new QGridLayout(commonOptions);
  commonOptionsLayout->setSpacing(5);

  QLabel *user_id_label = new QLabel(Smb4KMountSettings::self()->userIDItem()->label(), commonOptions);

  QWidget *user_widget = new QWidget(commonOptions);

  QGridLayout *user_layout = new QGridLayout(user_widget);
  user_layout->setSpacing(5);
  user_layout->setMargin(0);

  KLineEdit *user_id = new KLineEdit(user_widget);
  user_id->setObjectName("kcfg_UserID");
  user_id->setAlignment(Qt::AlignRight);
  user_id->setReadOnly(true);

  QToolButton *user_chooser = new QToolButton(user_widget);
  user_chooser->setIcon(KDE::icon("edit-find-user"));
  user_chooser->setToolTip(i18n("Choose a different user"));
  user_chooser->setPopupMode(QToolButton::InstantPopup);

  user_id_label->setBuddy(user_chooser);
  
  QMenu *user_menu = new QMenu(user_chooser);
  user_chooser->setMenu(user_menu);

  QList<KUser> user_list = KUser::allUsers();
  QMap<QString,QString> users;

  for (int i = 0; i < user_list.size(); ++i)
  {
    users.insert(QString("%1 (%2)").arg(user_list.at(i).loginName()).arg(user_list.at(i).userId().nativeId()),
                 QString("%1").arg(user_list.at(i).userId().nativeId()));
  }

  QMap<QString,QString>::const_iterator u_it = users.constBegin();

  while (u_it != users.constEnd())
  {
    QAction *user_action = user_menu->addAction(u_it.key());
    user_action->setData(u_it.value());
    ++u_it;
  }

  user_layout->addWidget(user_id, 0, 0, 0);
  user_layout->addWidget(user_chooser, 0, 1, Qt::AlignCenter);

  QLabel *group_id_label = new QLabel(Smb4KMountSettings::self()->groupIDItem()->label(), commonOptions);

  QWidget *group_widget = new QWidget(commonOptions);

  QGridLayout *group_layout = new QGridLayout(group_widget);
  group_layout->setSpacing(5);
  group_layout->setMargin(0);

  KLineEdit *group_id = new KLineEdit(group_widget);
  group_id->setObjectName("kcfg_GroupID");
  group_id->setAlignment(Qt::AlignRight);
  group_id->setReadOnly(true);
  
  QToolButton *group_chooser = new QToolButton(group_widget);
  group_chooser->setIcon(KDE::icon("edit-find-user"));
  group_chooser->setToolTip(i18n("Choose a different group"));
  group_chooser->setPopupMode(QToolButton::InstantPopup);

  group_id_label->setBuddy(group_chooser);

  QMenu *group_menu = new QMenu(group_chooser);
  group_chooser->setMenu(group_menu);

  QList<KUserGroup> group_list = KUserGroup::allGroups();
  QMap<QString,QString> groups;

  for (int i = 0; i < group_list.size(); ++i)
  {
    groups.insert(QString("%1 (%2)").arg(group_list.at(i).name()).arg(group_list.at(i).groupId().nativeId()),
                  QString("%1").arg(group_list.at(i).groupId().nativeId()));
  }

  QMap<QString,QString>::const_iterator g_it = groups.constBegin();

  while (g_it != groups.constEnd())
  {
    QAction *group_action = group_menu->addAction(g_it.key());
    group_action->setData(g_it.value());
    ++g_it;
  }

  group_layout->addWidget(group_id, 0, 0, 0);
  group_layout->addWidget(group_chooser, 0, 1, Qt::AlignCenter);

  QLabel *fmask_label = new QLabel(Smb4KMountSettings::self()->fileMaskItem()->label(), commonOptions);

  KLineEdit *fmask = new KLineEdit(commonOptions);
  fmask->setObjectName("kcfg_FileMask");
  fmask->setAlignment(Qt::AlignRight);

  fmask_label->setBuddy(fmask);

  QLabel *dmask_label = new QLabel(Smb4KMountSettings::self()->directoryMaskItem()->label(), commonOptions);

  KLineEdit *dmask = new KLineEdit(commonOptions);
  dmask->setObjectName("kcfg_DirectoryMask");
  dmask->setAlignment(Qt::AlignRight);

  dmask_label->setBuddy(dmask);
  
  QLabel *write_access_label = new QLabel(Smb4KMountSettings::self()->writeAccessItem()->label(), commonOptions);

  KComboBox *write_access      = new KComboBox(commonOptions);
  write_access->setObjectName("kcfg_WriteAccess");
  write_access->insertItem(Smb4KMountSettings::EnumWriteAccess::ReadWrite,
                           Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label);
  write_access->insertItem(Smb4KMountSettings::EnumWriteAccess::ReadOnly,
                           Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label);

  write_access_label->setBuddy(write_access);

  QLabel *charset_label = new QLabel(Smb4KMountSettings::self()->clientCharsetItem()->label(), commonOptions);

  KComboBox *charset = new KComboBox(commonOptions);
  charset->setObjectName("kcfg_ClientCharset");
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::default_charset,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::default_charset).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_1,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_1).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_2,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_2).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_3,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_3).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_4,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_4).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_5,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_5).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_6,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_6).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_7,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_7).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_8,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset:: iso8859_8).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_9,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_9).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_13,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_13).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_14,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_14).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_15,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_15).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::utf8,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::utf8).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::koi8_r,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::koi8_r).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::koi8_u,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::koi8_u).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::koi8_ru,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::koi8_ru).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::cp1251,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::cp1251).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::gb2312,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::gb2312).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::big5,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::big5).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::euc_jp,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::euc_jp).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::euc_kr,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::euc_kr).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::tis_620,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::tis_620).label);
  charset_label->setBuddy(charset);
  
  QLabel *remote_fs_port_label = new QLabel(Smb4KMountSettings::self()->remoteFileSystemPortItem()->label(), commonOptions);

  QSpinBox *remote_fs_port = new QSpinBox(commonOptions);
  remote_fs_port->setObjectName("kcfg_RemoteFileSystemPort");
//   remote_fs_port->setSliderEnabled(true);

  commonOptionsLayout->addWidget(user_id_label, 0, 0, 0);
  commonOptionsLayout->addWidget(user_widget, 0, 1, 0);
  commonOptionsLayout->addWidget(group_id_label, 1, 0, 0);
  commonOptionsLayout->addWidget(group_widget, 1, 1, 0);
  commonOptionsLayout->addWidget(fmask_label, 2, 0, 0);
  commonOptionsLayout->addWidget(fmask, 2, 1, 0);
  commonOptionsLayout->addWidget(dmask_label, 3, 0, 0);
  commonOptionsLayout->addWidget(dmask, 3, 1, 0);
  commonOptionsLayout->addWidget(write_access_label, 4, 0, 0);
  commonOptionsLayout->addWidget(write_access, 4, 1, 0);
  commonOptionsLayout->addWidget(charset_label, 5, 0, 0);
  commonOptionsLayout->addWidget(charset, 5, 1, 0);
  commonOptionsLayout->addWidget(remote_fs_port_label, 6, 0, 0);
  commonOptionsLayout->addWidget(remote_fs_port, 6, 1, 0);
  
  commonTabLayout->addWidget(commonOptions, 0, 0);
  commonTabLayout->addStretch(1000);
  
  addTab(commonTab, i18n("Common Settings"));

  // 
  // Advanced options
  // 
  QWidget *advancedTab = new QWidget(this);
  QVBoxLayout *advancedTabLayout = new QVBoxLayout(advancedTab);
  advancedTabLayout->setSpacing(5);
  advancedTabLayout->setMargin(0);
  
  QGroupBox *advancedOptions = new QGroupBox(i18n("Advanced Options"), advancedTab);

  QGridLayout *advancedOptionsLayout = new QGridLayout(advancedOptions);
  advancedOptionsLayout->setSpacing(5);
  
  QCheckBox *force_uid = new QCheckBox(Smb4KMountSettings::self()->forceUIDItem()->label(), advancedOptions);
  force_uid->setObjectName("kcfg_ForceUID");
  
  QCheckBox *force_gid = new QCheckBox(Smb4KMountSettings::self()->forceGIDItem()->label(), advancedOptions);
  force_gid->setObjectName("kcfg_ForceGID");

  QCheckBox *permission_checks = new QCheckBox(Smb4KMountSettings::self()->permissionChecksItem()->label(), advancedOptions);
  permission_checks->setObjectName("kcfg_PermissionChecks");

  QCheckBox *client_controls = new QCheckBox(Smb4KMountSettings::self()->clientControlsIDsItem()->label(), advancedOptions);
  client_controls->setObjectName("kcfg_ClientControlsIDs");

  QCheckBox *server_inodes = new QCheckBox(Smb4KMountSettings::self()->serverInodeNumbersItem()->label(), advancedOptions);
  server_inodes->setObjectName("kcfg_ServerInodeNumbers");

  QCheckBox *reserved_chars = new QCheckBox(Smb4KMountSettings::self()->translateReservedCharsItem()->label(), advancedOptions);
  reserved_chars->setObjectName("kcfg_TranslateReservedChars");

  QCheckBox *no_locking = new QCheckBox(Smb4KMountSettings::self()->noLockingItem()->label(), advancedOptions);
  no_locking->setObjectName("kcfg_NoLocking");

  QWidget *c_extra_widget = new QWidget(advancedOptions);

  QGridLayout *c_extra_layout = new QGridLayout(c_extra_widget);
  c_extra_layout->setSpacing(5);
  c_extra_layout->setMargin(0);
  
  QLabel *smbProtocol_label = new QLabel(Smb4KMountSettings::self()->smbProtocolVersionItem()->label(), c_extra_widget);
  
  KComboBox *smbProtocol_box = new KComboBox(c_extra_widget);
  smbProtocol_box->setObjectName("kcfg_SmbProtocolVersion");
  smbProtocol_box->insertItem(Smb4KMountSettings::EnumSmbProtocolVersion::OnePointZero,
                              Smb4KMountSettings::self()->smbProtocolVersionItem()->choices().value(Smb4KMountSettings::EnumSmbProtocolVersion::OnePointZero).label);
  smbProtocol_box->insertItem(Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointZero,
                              Smb4KMountSettings::self()->smbProtocolVersionItem()->choices().value(Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointZero).label);
  smbProtocol_box->insertItem(Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointOne,
                              Smb4KMountSettings::self()->smbProtocolVersionItem()->choices().value(Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointOne).label);
  smbProtocol_box->insertItem(Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointZero,
                              Smb4KMountSettings::self()->smbProtocolVersionItem()->choices().value(Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointZero).label);
  
  QLabel *cache_label = new QLabel(Smb4KMountSettings::self()->cacheModeItem()->label(), c_extra_widget);
  
  KComboBox *cache_box = new KComboBox(c_extra_widget);
  cache_box->setObjectName("kcfg_CacheMode");
  cache_box->insertItem(Smb4KMountSettings::EnumCacheMode::None,
                        Smb4KMountSettings::self()->cacheModeItem()->choices().value(Smb4KMountSettings::EnumCacheMode::None).label);
  cache_box->insertItem(Smb4KMountSettings::EnumCacheMode::Strict,
                        Smb4KMountSettings::self()->cacheModeItem()->choices().value(Smb4KMountSettings::EnumCacheMode::Strict).label);
  cache_box->insertItem(Smb4KMountSettings::EnumCacheMode::Loose,
                        Smb4KMountSettings::self()->cacheModeItem()->choices().value(Smb4KMountSettings::EnumCacheMode::Loose).label);
  cache_label->setBuddy(cache_box);
  
  QLabel *security_label = new QLabel(Smb4KMountSettings::self()->securityModeItem()->label(), c_extra_widget);
                                 
  KComboBox *security_box = new KComboBox(c_extra_widget);
  security_box->setObjectName("kcfg_SecurityMode");
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::None,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::None).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Krb5,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Krb5i,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5i).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Ntlm,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlm).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Ntlmi,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmi).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Ntlmv2,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Ntlmssp,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmssp).label);
  security_box->insertItem(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi,
                           Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi).label);
  security_label->setBuddy(security_box);

  QLabel *add_options_label = new QLabel(Smb4KMountSettings::self()->customCIFSOptionsItem()->label(), c_extra_widget);

  KLineEdit *additional_opts = new KLineEdit(c_extra_widget);
  additional_opts->setObjectName("kcfg_CustomCIFSOptions");
  additional_opts->setReadOnly(true);
  additional_opts->setClearButtonShown(true);
  add_options_label->setBuddy(additional_opts);
  
  QToolButton *additional_opts_edit = new QToolButton(c_extra_widget);
  additional_opts_edit->setIcon(KDE::icon("document-edit"));
  additional_opts_edit->setToolTip(i18n("Edit the additional CIFS options."));

  c_extra_layout->addWidget(smbProtocol_label, 0, 0, 0);
  c_extra_layout->addWidget(smbProtocol_box, 0, 1, 1, 2, 0);
  c_extra_layout->addWidget(cache_label, 1, 0, 0);
  c_extra_layout->addWidget(cache_box, 1, 1, 1, 2, 0);
  c_extra_layout->addWidget(security_label, 2, 0, 0);
  c_extra_layout->addWidget(security_box, 2, 1, 1, 2, 0);
  c_extra_layout->addWidget(add_options_label, 3, 0, 0);
  c_extra_layout->addWidget(additional_opts, 3, 1, 0);
  c_extra_layout->addWidget(additional_opts_edit, 3, 2, 0);

  advancedOptionsLayout->addWidget(force_uid, 0, 0, 0);
  advancedOptionsLayout->addWidget(force_gid, 0, 1, 0);
  advancedOptionsLayout->addWidget(permission_checks, 1, 0, 0);
  advancedOptionsLayout->addWidget(client_controls, 1, 1, 0);
  advancedOptionsLayout->addWidget(server_inodes, 2, 0, 0);
  advancedOptionsLayout->addWidget(reserved_chars, 2, 1, 0);
  advancedOptionsLayout->addWidget(no_locking, 3, 0, 0);
  advancedOptionsLayout->addWidget(c_extra_widget, 4, 0, 1, 2, 0);
  
  advancedTabLayout->addWidget(advancedOptions, 0);
  advancedTabLayout->addStretch(100);
  
  addTab(advancedTab, i18n("Advanced Settings"));
  
  //
  // Connections
  //
  connect(user_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotNewUserTriggered(QAction*)));
  connect(group_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotNewGroupTriggered(QAction*)));
  connect(additional_opts_edit, SIGNAL(clicked(bool)), this, SLOT(slotAdditionalCIFSOptions()));
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KConfigPageMounting::setupWidget()
{
  QVBoxLayout *mount_layout = new QVBoxLayout(this);
  mount_layout->setSpacing(5);
  mount_layout->setMargin(0);

  // Common Options
  QGroupBox *commonOptions = new QGroupBox(i18n("Common Options"), this);

  QGridLayout *commonOptionsLayout = new QGridLayout(commonOptions);
  commonOptionsLayout->setSpacing(5);

  QLabel *user_id_label = new QLabel(Smb4KMountSettings::self()->userIDItem()->label(), commonOptions);

  QWidget *user_widget = new QWidget(commonOptions);

  QGridLayout *user_layout = new QGridLayout(user_widget);
  user_layout->setSpacing(5);
  user_layout->setMargin(0);

  KLineEdit *user_id = new KLineEdit(user_widget);
  user_id->setObjectName("kcfg_UserID");
  user_id->setAlignment(Qt::AlignRight);
  user_id->setReadOnly(true);

  QToolButton *user_chooser = new QToolButton(user_widget);
  user_chooser->setIcon(KDE::icon("edit-find-user"));
  user_chooser->setToolTip(i18n("Choose a different user"));
  user_chooser->setPopupMode(QToolButton::InstantPopup);

  user_id_label->setBuddy(user_chooser);
  
  QMenu *user_menu = new QMenu(user_chooser);
  user_chooser->setMenu(user_menu);

  QList<KUser> user_list = KUser::allUsers();
  QMap<QString,QString> users;

  for (int i = 0; i < user_list.size(); ++i)
  {
    users.insert(QString("%1 (%2)").arg(user_list.at(i).loginName()).arg(user_list.at(i).userId().nativeId()),
                 QString("%1").arg(user_list.at(i).userId().nativeId()));
  }

  QMap<QString,QString>::const_iterator u_it = users.constBegin();

  while (u_it != users.constEnd())
  {
    QAction *user_action = user_menu->addAction(u_it.key());
    user_action->setData(u_it.value());
    ++u_it;
  }

  user_layout->addWidget(user_id, 0, 0, 0);
  user_layout->addWidget(user_chooser, 0, 1, Qt::AlignCenter);

  QLabel *group_id_label = new QLabel(Smb4KMountSettings::self()->groupIDItem()->label(), commonOptions);

  QWidget *group_widget = new QWidget(commonOptions);

  QGridLayout *group_layout = new QGridLayout(group_widget);
  group_layout->setSpacing(5);
  group_layout->setMargin(0);

  KLineEdit *group_id = new KLineEdit(group_widget);
  group_id->setObjectName("kcfg_GroupID");
  group_id->setAlignment(Qt::AlignRight);
  group_id->setReadOnly(true);
  
  QToolButton *group_chooser = new QToolButton(group_widget);
  group_chooser->setIcon(KDE::icon("edit-find-user"));
  group_chooser->setToolTip(i18n("Choose a different group"));
  group_chooser->setPopupMode(QToolButton::InstantPopup);

  group_id_label->setBuddy(group_chooser);

  QMenu *group_menu = new QMenu(group_chooser);
  group_chooser->setMenu(group_menu);

  QList<KUserGroup> group_list = KUserGroup::allGroups();
  QMap<QString,QString> groups;

  for (int i = 0; i < group_list.size(); ++i)
  {
    groups.insert(QString("%1 (%2)").arg(group_list.at(i).name()).arg(group_list.at(i).groupId().nativeId()),
                  QString("%1").arg(group_list.at(i).groupId().nativeId()));
  }

  QMap<QString,QString>::const_iterator g_it = groups.constBegin();

  while (g_it != groups.constEnd())
  {
    QAction *group_action = group_menu->addAction(g_it.key());
    group_action->setData(g_it.value());
    ++g_it;
  }

  group_layout->addWidget(group_id, 0, 0, 0);
  group_layout->addWidget(group_chooser, 0, 1, Qt::AlignCenter);

  QLabel *fmask_label = new QLabel(Smb4KMountSettings::self()->fileMaskItem()->label(), commonOptions);

  KLineEdit *fmask = new KLineEdit(commonOptions);
  fmask->setObjectName("kcfg_FileMask");
  fmask->setAlignment(Qt::AlignRight);

  fmask_label->setBuddy(fmask);

  QLabel *dmask_label = new QLabel(Smb4KMountSettings::self()->directoryMaskItem()->label(), commonOptions);

  KLineEdit *dmask = new KLineEdit(commonOptions);
  dmask->setObjectName("kcfg_DirectoryMask");
  dmask->setAlignment(Qt::AlignRight);

  dmask_label->setBuddy(dmask);
  
  QLabel *charset_label = new QLabel(Smb4KMountSettings::self()->clientCharsetItem()->label(), commonOptions);

  KComboBox *charset = new KComboBox(commonOptions);
  charset->setObjectName("kcfg_ClientCharset");
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::default_charset,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::default_charset).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_1,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_1).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_2,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_2).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_3,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_3).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_4,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_4).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_5,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_5).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_6,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_6).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_7,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_7).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_8,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset:: iso8859_8).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_9,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_9).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_13,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_13).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_14,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_14).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::iso8859_15,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::iso8859_15).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::utf8,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::utf8).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::koi8_r,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::koi8_r).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::koi8_u,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::koi8_u).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::koi8_ru,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::koi8_ru).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::cp1251,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::cp1251).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::gb2312,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::gb2312).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::big5,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::big5).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::euc_jp,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::euc_jp).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::euc_kr,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::euc_kr).label);
  charset->insertItem(Smb4KMountSettings::EnumClientCharset::tis_620,
                      Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::EnumClientCharset::tis_620).label);

  charset_label->setBuddy(charset);

  QLabel *codepage_label = new QLabel(Smb4KMountSettings::self()->serverCodepageItem()->label(), commonOptions);
  codepage_label->setObjectName("CodepageLabel");

  KComboBox *codepage = new KComboBox(commonOptions);
  codepage->setObjectName("kcfg_ServerCodepage");
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::default_codepage,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::default_codepage).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp437,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp437).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp720,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp720).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp737,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp737).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp775,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp775).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp850,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp850).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp852,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp852).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp855,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp855).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp857,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp857).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp858,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp858).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp860,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp860).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp861,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp861).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp862,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp862).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp863,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp863).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp864,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp864).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp865,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp865).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp866,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp866).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp869,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp869).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp874,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp874).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp932,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp932).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp936,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp936).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp949,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp949).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp950,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp950).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1250,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1250).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1251,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1251).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1252,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1252).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1253,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1253).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1254,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1254).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1255,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1255).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1256,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1256).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1257,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1257).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::cp1258,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::cp1258).label);
  codepage->insertItem(Smb4KMountSettings::EnumServerCodepage::unicode,
                       Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::EnumServerCodepage::unicode).label);

  codepage_label->setBuddy(codepage);

  commonOptionsLayout->addWidget(user_id_label, 0, 0, 0);
  commonOptionsLayout->addWidget(user_widget, 0, 1, 0);
  commonOptionsLayout->addWidget(group_id_label, 1, 0, 0);
  commonOptionsLayout->addWidget(group_widget, 1, 1, 0);
  commonOptionsLayout->addWidget(fmask_label, 2, 0, 0);
  commonOptionsLayout->addWidget(fmask, 2, 1, 0);
  commonOptionsLayout->addWidget(dmask_label, 3, 0, 0);
  commonOptionsLayout->addWidget(dmask, 3, 1, 0);
  commonOptionsLayout->addWidget(charset_label, 4, 0, 0);
  commonOptionsLayout->addWidget(charset, 4, 1, 0);
  commonOptionsLayout->addWidget(codepage_label, 5, 0, 0);
  commonOptionsLayout->addWidget(codepage, 5, 1, 0);

  mount_layout->addWidget(commonOptions);
  mount_layout->addStretch(100);

  //
  // Connections
  //
  connect(user_menu, SIGNAL(triggered(QAction*)),
          this, SLOT(slotNewUserTriggered(QAction*)));
  connect(group_menu, SIGNAL(triggered(QAction*)),
          this, SLOT(slotNewGroupTriggered(QAction*)));
}
#else
//
// Dummy
//
void Smb4KConfigPageMounting::setupWidget()
{
}
#endif


void Smb4KConfigPageMounting::slotNewUserTriggered(QAction *action)
{
  KLineEdit *user_id = findChild<KLineEdit *>("kcfg_UserID");

  if (user_id)
  {
    user_id->setText(action->data().toString());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigPageMounting::slotNewGroupTriggered(QAction *action)
{
  KLineEdit *group_id = findChild<KLineEdit *>("kcfg_GroupID");

  if (group_id)
  {
    group_id->setText(action->data().toString());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigPageMounting::slotAdditionalCIFSOptions()
{
#if defined (Q_OS_LINUX)
  KLineEdit *cifs_opts = findChild<KLineEdit *>("kcfg_CustomCIFSOptions");
  
  if (cifs_opts)
  {
    QString options = cifs_opts->originalText();
    
    bool ok = false;
    options = QInputDialog::getText(this, i18n("Additional CIFS Options"), 
                                    i18n("<qt>Enter the desired options as a comma separated list:</qt>"), 
                                    QLineEdit::Normal, 
                                    options,
                                    &ok);
    
    if (ok)
    {
      if(!options.trimmed().isEmpty())
      {
        // SECURITY: Only pass those arguments to mount.cifs that do not pose
        // a potential security risk and that have not already been defined.
        //
        // This is, among others, the proper fix to the security issue reported
        // by Heiner Markert (aka CVE-2014-2581).
        QStringList whitelist = whitelistedMountArguments();
        QStringList denied_args;
        QStringList list = options.split(',', QString::SkipEmptyParts);
        QMutableStringListIterator it(list);
        
        while (it.hasNext())
        {
          QString arg = it.next().section("=", 0, 0);
          
          if (!whitelist.contains(arg))
          {
            denied_args << arg;
            it.remove();
          }
          else
          {
            // Do nothing
          }
        }
        
        if (!denied_args.isEmpty())
        {
          QString msg = i18np("<qt>The following entry is going to be removed from the additional options: %2. Please read the handbook for details.</qt>", "<qt>The following %1 entries are going to be removed from the additional options: %2. Please read the handbook for details.</qt>", denied_args.size(), denied_args.join(", "));
          KMessageBox::sorry(this, msg);
        }
        else
        {
          // Do nothing
        }
        
        cifs_opts->setText(list.join(",").trimmed());
      }
      else
      {
        cifs_opts->clear();
      }
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
#endif
}

