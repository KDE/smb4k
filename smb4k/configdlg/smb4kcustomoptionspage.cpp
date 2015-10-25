/***************************************************************************
    smb4kcustomoptionspage  -  The configuration page for the custom 
    options
                             -------------------
    begin                : Sa Jan 19 2013
    copyright            : (C) 2013-2015 by Alexander Reinholdt
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
#include "smb4kcustomoptionspage.h"
#include "core/smb4ksettings.h"
#include "core/smb4kcustomoptions.h"
#include "core/smb4kglobal.h"

#if defined(Q_OS_LINUX)
#include "core/smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "core/smb4kmountsettings_freebsd.h"
#endif

// Qt includes
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtGui/QMouseEvent>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QAbstractSocket>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;


Smb4KCustomOptionsPage::Smb4KCustomOptionsPage(QWidget *parent) : QWidget(parent)
{
  m_collection = new KActionCollection(this);
  m_maybe_changed = false;
  m_removed = false;
  m_current_options = NULL;
  
  setupWidget();
}


Smb4KCustomOptionsPage::~Smb4KCustomOptionsPage()
{
  while (!m_options_list.isEmpty())
  {
    delete m_options_list.takeFirst();
  }
}

#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KCustomOptionsPage::setupWidget()
{
  QHBoxLayout *custom_layout = new QHBoxLayout(this);
  custom_layout->setSpacing(5);
  custom_layout->setMargin(0);
  
  //
  // The list widget
  //  
  m_custom_options = new QListWidget(this);
  m_custom_options->setObjectName("CustomOptionsList");
  m_custom_options->viewport()->installEventFilter(this);
  m_custom_options->setSelectionMode(QListWidget::SingleSelection);
  m_custom_options->setContextMenuPolicy(Qt::CustomContextMenu);

  m_menu = new KActionMenu(m_custom_options);

  QAction *edit_action = new QAction(KDE::icon("edit-rename"), i18n("Edit"), m_collection);
  edit_action->setEnabled(false);

  QAction *remove_action = new QAction(KDE::icon("edit-delete"), i18n("Remove"), m_collection);
  remove_action->setEnabled(false);
  
  QAction *clear_action = new QAction(KDE::icon("edit-clear-list"), i18n("Clear List"), m_collection);
  clear_action->setEnabled(false);
  
  QAction *undo_action = new QAction(KDE::icon("edit-undo"), i18n("Undo"), m_collection);
  undo_action->setEnabled(false);
  
  m_collection->addAction("edit_action", edit_action);
  m_collection->addAction("remove_action", remove_action);
  m_collection->addAction("clear_action", clear_action);
  m_collection->addAction("undo_action", undo_action);

  m_menu->addAction(edit_action);
  m_menu->addAction(remove_action);
  m_menu->addAction(clear_action);
  m_menu->addAction(undo_action);
  
  //
  // The editors
  //  
  QWidget *editors = new QWidget(this);
  
  QVBoxLayout *editors_layout = new QVBoxLayout(editors);
  editors_layout->setSpacing(5);
  editors_layout->setMargin(0);
  
  m_general_editors = new QGroupBox(i18n("General"), editors);
  
  QGridLayout *general_editor_layout = new QGridLayout(m_general_editors);
  general_editor_layout->setSpacing(5);
  general_editor_layout->setContentsMargins(general_editor_layout->margin(),
                                            general_editor_layout->margin() + 10,
                                            general_editor_layout->margin(),
                                            general_editor_layout->margin());

  QLabel *unc_label = new QLabel(i18n("UNC Address:"), m_general_editors);
  
  m_unc_address = new KLineEdit(m_general_editors);
  m_unc_address->setReadOnly(true);

  unc_label->setBuddy(m_unc_address);
  
  QLabel *ip_label = new QLabel(i18n("IP Address:"), m_general_editors);
  
  m_ip_address = new KLineEdit(m_general_editors);
  m_ip_address->setClearButtonShown(true);
  
  ip_label->setBuddy(m_ip_address);
  
  m_remount_share = new QCheckBox(i18n("Always remount this share"), m_general_editors);
  
  general_editor_layout->addWidget(unc_label, 0, 0, 0);
  general_editor_layout->addWidget(m_unc_address, 0, 1, 0);
  general_editor_layout->addWidget(ip_label, 1, 0, 0);
  general_editor_layout->addWidget(m_ip_address, 1, 1, 0);
  general_editor_layout->addWidget(m_remount_share, 2, 0, 1, 2, 0);
  
  m_tab_widget = new QTabWidget(editors);
  
  QWidget *samba_tab = new QWidget(m_tab_widget);
  
  QVBoxLayout *samba_tab_layout = new QVBoxLayout(samba_tab);
  samba_tab_layout->setSpacing(5);
  samba_tab_layout->setMargin(0);
  
  QGroupBox *samba_editors = new QGroupBox(samba_tab);
  samba_editors->setFlat(true);
  
  QGridLayout *samba_editor_layout = new QGridLayout(samba_editors);
  samba_editor_layout->setSpacing(5);
  samba_editor_layout->setContentsMargins(samba_editor_layout->margin(),
                                          samba_editor_layout->margin() + 10,
                                          samba_editor_layout->margin(),
                                          samba_editor_layout->margin());
  
  QLabel *smb_port_label = new QLabel("SMB Port:", samba_editors);
  
  m_smb_port = new QSpinBox(samba_editors);
  m_smb_port->setRange(Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt());
//   m_smb_port->(true);

  smb_port_label->setBuddy(m_smb_port);

  QLabel *fs_port_label = new QLabel(i18n("Filesystem Port:"), samba_editors);
  
  m_fs_port = new QSpinBox(samba_editors);
  m_fs_port->setRange(Smb4KMountSettings::self()->remoteFileSystemPortItem()->minValue().toInt(),
                      Smb4KMountSettings::self()->remoteFileSystemPortItem()->maxValue().toInt());
//   m_fs_port->setSliderEnabled(true);

  fs_port_label->setBuddy(m_fs_port);
  
  QLabel *rw_label = new QLabel(i18n("Write Access:"), samba_editors);
  
  m_write_access = new KComboBox(samba_editors);
  m_write_access->insertItem(0, Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label, 
                             QVariant::fromValue<int>(Smb4KCustomOptions::ReadWrite));
  m_write_access->insertItem(1, Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label, 
                             QVariant::fromValue<int>(Smb4KCustomOptions::ReadOnly));

  rw_label->setBuddy(m_write_access);

  QLabel *security_label = new QLabel(i18n("Security Mode:"), samba_editors);

  m_security_mode = new KComboBox(samba_editors);

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
  
  security_label->setBuddy(m_security_mode);
  
  QLabel *uid_label = new QLabel(i18n("User ID:"), samba_editors);
  m_user_id = new KComboBox(samba_editors);
  
  QList<KUser> all_users = KUser::allUsers();
  
  for (int i = 0; i < all_users.size(); ++i)
  {
    KUser user = all_users.at(i);
    m_user_id->insertItem(i, QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()), QVariant::fromValue<K_UID>(user.userId().nativeId()));
  }

  uid_label->setBuddy(m_user_id);
  
  QLabel *gid_label = new QLabel(i18n("Group ID:"), samba_editors);
  m_group_id = new KComboBox(samba_editors);
  
  QList<KUserGroup> all_groups = KUserGroup::allGroups();
  
  for (int i = 0; i < all_groups.size(); ++i)
  {
    KUserGroup group = all_groups.at(i);
    m_group_id->insertItem(i, QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()), QVariant::fromValue<K_GID>(group.groupId().nativeId()));
  }

  gid_label->setBuddy(m_group_id);
  
  m_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), samba_editors);

  samba_editor_layout->addWidget(smb_port_label, 0, 0, 0);
  samba_editor_layout->addWidget(m_smb_port, 0, 1, 0);
  samba_editor_layout->addWidget(fs_port_label, 1, 0, 0);
  samba_editor_layout->addWidget(m_fs_port, 1, 1, 0);
  samba_editor_layout->addWidget(rw_label, 2, 0, 0);
  samba_editor_layout->addWidget(m_write_access, 2, 1, 0);
  samba_editor_layout->addWidget(security_label, 3, 0, 0);
  samba_editor_layout->addWidget(m_security_mode, 3, 1, 0);
  samba_editor_layout->addWidget(uid_label, 4, 0, 0);
  samba_editor_layout->addWidget(m_user_id, 4, 1, 0);
  samba_editor_layout->addWidget(gid_label, 5, 0, 0);
  samba_editor_layout->addWidget(m_group_id, 5, 1, 0);
  samba_editor_layout->addWidget(m_kerberos, 6, 0, 1, 2, 0);
  
  samba_tab_layout->addWidget(samba_editors);
  samba_tab_layout->addStretch(100);

  QWidget *wol_tab = new QWidget(m_tab_widget);
  
  QVBoxLayout *wol_tab_layout = new QVBoxLayout(wol_tab);
  wol_tab_layout->setSpacing(5);
  wol_tab_layout->setMargin(0);  
  
  QGroupBox *mac_editors = new QGroupBox(wol_tab);
  mac_editors->setFlat(true);
  
  QGridLayout *mac_editor_layout = new QGridLayout(mac_editors);
  mac_editor_layout->setSpacing(5);
  mac_editor_layout->setContentsMargins(mac_editor_layout->margin(),
                                        mac_editor_layout->margin() + 10,
                                        mac_editor_layout->margin(),
                                        mac_editor_layout->margin());
  
  QLabel *mac_label = new QLabel(i18n("MAC address:"), mac_editors);
  
  m_mac_address = new KLineEdit(mac_editors);
  m_mac_address->setClearButtonShown(true);
  
  mac_label->setBuddy(m_mac_address);
  
  // If you change the texts here please also alter them in the custom 
  // options dialog.
  m_send_before_scan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), mac_editors);
  m_send_before_scan->setEnabled(false);
  m_send_before_mount = new QCheckBox(i18n("Send magic package before mounting a share"), mac_editors);
  m_send_before_mount->setEnabled(false);
  
  mac_editor_layout->addWidget(mac_label, 0, 0, 0);
  mac_editor_layout->addWidget(m_mac_address, 0, 1, 0);
  mac_editor_layout->addWidget(m_send_before_scan, 1, 0, 1, 2, 0);
  mac_editor_layout->addWidget(m_send_before_mount, 2, 0, 1, 2, 0);
  
  wol_tab_layout->addWidget(mac_editors);
  wol_tab_layout->addStretch(100);
  
  (void)m_tab_widget->insertTab(SambaTab, samba_tab, i18n("Samba"));
  (void)m_tab_widget->insertTab(WolTab, wol_tab, i18n("Wake-On-LAN"));
  
  editors_layout->addWidget(m_general_editors);
  editors_layout->addWidget(m_tab_widget);
                                 
  custom_layout->addWidget(m_custom_options);
  custom_layout->addWidget(editors);

  m_general_editors->setEnabled(false);
  m_tab_widget->setEnabled(false);

  clearEditors();
  
  //
  // Connections
  //
  connect(m_custom_options, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
          this,             SLOT(slotEditCustomItem(QListWidgetItem*)));
  connect(m_custom_options, SIGNAL(itemSelectionChanged()),
          this,             SLOT(slotItemSelectionChanged()));
  connect(m_custom_options, SIGNAL(customContextMenuRequested(QPoint)),
          this,             SLOT(slotCustomContextMenuRequested(QPoint)));
  connect(edit_action,      SIGNAL(triggered(bool)),
          this,             SLOT(slotEditActionTriggered(bool)));
  connect(remove_action,    SIGNAL(triggered(bool)),
          this,             SLOT(slotRemoveActionTriggered(bool)));
  connect(clear_action,     SIGNAL(triggered(bool)),
          this,             SLOT(slotClearActionTriggered(bool)));
  connect(undo_action,      SIGNAL(triggered(bool)),
          this,             SLOT(slotUndoActionTriggered(bool)));
  connect(m_ip_address,     SIGNAL(textChanged(QString)),
          this,             SLOT(slotEntryChanged()));
  connect(m_remount_share,  SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_smb_port,       SIGNAL(valueChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_fs_port,        SIGNAL(valueChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_write_access,   SIGNAL(currentIndexChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_security_mode,  SIGNAL(currentIndexChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_user_id,        SIGNAL(currentIndexChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_group_id,       SIGNAL(currentIndexChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_kerberos,       SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_mac_address,    SIGNAL(textChanged(QString)),
          this,             SLOT(slotEntryChanged()));
  connect(m_send_before_scan,   SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_send_before_mount,  SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_mac_address,    SIGNAL(textChanged(QString)),
          this,             SLOT(slotEnableWOLFeatures(QString)));
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KCustomOptionsPage::setupWidget()
{
  QHBoxLayout *custom_layout = new QHBoxLayout(this);
  custom_layout->setSpacing(5);
  custom_layout->setMargin(0);
  
  //
  // The list widget
  //  
  m_custom_options = new QListWidget(this);
  m_custom_options->setObjectName("CustomOptionsList");
  m_custom_options->viewport()->installEventFilter(this);
  m_custom_options->setSelectionMode(QListWidget::SingleSelection);
  m_custom_options->setContextMenuPolicy(Qt::CustomContextMenu);

  m_menu = new KActionMenu(m_custom_options);

  QAction *edit_action = new QAction(KDE::icon("edit-rename"), i18n("Edit"), m_collection);
  edit_action->setEnabled(false);

  QAction *remove_action = new QAction(KDE::icon("edit-delete"), i18n("Remove"), m_collection);
  remove_action->setEnabled(false);
  
  QAction *clear_action = new QAction(KDE::icon("edit-clear-list"), i18n("Clear List"), m_collection);
  clear_action->setEnabled(false);
  
  QAction *undo_action = new QAction(KDE::icon("edit-undo"), i18n("Undo"), m_collection);
  undo_action->setEnabled(false);
  
  m_collection->addAction("edit_action", edit_action);
  m_collection->addAction("remove_action", remove_action);
  m_collection->addAction("clear_action", clear_action);
  m_collection->addAction("undo_action", undo_action);

  m_menu->addAction(edit_action);
  m_menu->addAction(remove_action);
  m_menu->addAction(clear_action);
  m_menu->addAction(undo_action);
  
  //
  // The editors
  //  
  QWidget *editors = new QWidget(this);
  
  QVBoxLayout *editors_layout = new QVBoxLayout(editors);
  editors_layout->setSpacing(5);
  editors_layout->setMargin(0);
  
  m_general_editors = new QGroupBox(i18n("General"), editors);
  
  QGridLayout *general_editor_layout = new QGridLayout(m_general_editors);
  general_editor_layout->setSpacing(5);
  general_editor_layout->setContentsMargins(general_editor_layout->margin(),
                                            general_editor_layout->margin() + 10,
                                            general_editor_layout->margin(),
                                            general_editor_layout->margin());

  QLabel *unc_label = new QLabel(i18n("UNC Address:"), m_general_editors);
  
  m_unc_address = new KLineEdit(m_general_editors);
  m_unc_address->setReadOnly(true);

  unc_label->setBuddy(m_unc_address);
  
  QLabel *ip_label = new QLabel(i18n("IP Address:"), m_general_editors);
  
  m_ip_address = new KLineEdit(m_general_editors);
  m_ip_address->setClearButtonShown(true);
  
  ip_label->setBuddy(m_ip_address);
  
  m_remount_share = new QCheckBox(i18n("Always remount this share"), m_general_editors);
  
  general_editor_layout->addWidget(unc_label, 0, 0, 0);
  general_editor_layout->addWidget(m_unc_address, 0, 1, 0);
  general_editor_layout->addWidget(ip_label, 1, 0, 0);
  general_editor_layout->addWidget(m_ip_address, 1, 1, 0);
  general_editor_layout->addWidget(m_remount_share, 2, 0, 1, 2, 0);
  
  m_tab_widget = new QTabWidget(editors);
  
  QWidget *samba_tab = new QWidget(m_tab_widget);
  
  QVBoxLayout *samba_tab_layout = new QVBoxLayout(samba_tab);
  samba_tab_layout->setSpacing(5);
  samba_tab_layout->setMargin(0);
  
  QGroupBox *samba_editors = new QGroupBox(samba_tab);
  samba_editors->setFlat(true);
  
  QGridLayout *samba_editor_layout = new QGridLayout(samba_editors);
  samba_editor_layout->setSpacing(5);
  samba_editor_layout->setContentsMargins(samba_editor_layout->margin(),
                                          samba_editor_layout->margin() + 10,
                                          samba_editor_layout->margin(),
                                          samba_editor_layout->margin());
  
  QLabel *smb_port_label = new QLabel("SMB Port:", samba_editors);
  
  m_smb_port = new QSpinBox(samba_editors);
  m_smb_port->setRange(Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt());
//   m_smb_port->setSliderEnabled(true);

  smb_port_label->setBuddy(m_smb_port);

  QLabel *uid_label = new QLabel(i18n("User ID:"), samba_editors);
  m_user_id = new KComboBox(samba_editors);
  
  QList<KUser> all_users = KUser::allUsers();
  
  for (int i = 0; i < all_users.size(); ++i)
  {
    KUser user = all_users.at(i);
    m_user_id->insertItem(i, QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()), QVariant::fromValue<K_UID>(user.userId().nativeId()));
  }

  uid_label->setBuddy(m_user_id);
  
  QLabel *gid_label = new QLabel(i18n("Group ID:"), samba_editors);
  m_group_id = new KComboBox(samba_editors);
  
  QList<KUserGroup> all_groups = KUserGroup::allGroups();
  
  for (int i = 0; i < all_groups.size(); ++i)
  {
    KUserGroup group = all_groups.at(i);
    m_group_id->insertItem(i, QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()), QVariant::fromValue<K_GID>(group.groupId().nativeId()));
  }

  gid_label->setBuddy(m_group_id);
  
  m_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), samba_editors);
  
  samba_editor_layout->addWidget(smb_port_label, 0, 0, 0);
  samba_editor_layout->addWidget(m_smb_port, 0, 1, 0);
  samba_editor_layout->addWidget(uid_label, 1, 0, 0);
  samba_editor_layout->addWidget(m_user_id, 1, 1, 0);
  samba_editor_layout->addWidget(gid_label, 2, 0, 0);
  samba_editor_layout->addWidget(m_group_id, 2, 1, 0);
  samba_editor_layout->addWidget(m_kerberos, 3, 0, 1, 2, 0);
  
  samba_tab_layout->addWidget(samba_editors);
  samba_tab_layout->addStretch(100);

  QWidget *wol_tab = new QWidget(m_tab_widget);
  
  QVBoxLayout *wol_tab_layout = new QVBoxLayout(wol_tab);
  wol_tab_layout->setSpacing(5);
  wol_tab_layout->setMargin(0);  
  
  QGroupBox *mac_editors = new QGroupBox(wol_tab);
  mac_editors->setFlat(true);
  
  QGridLayout *mac_editor_layout = new QGridLayout(mac_editors);
  mac_editor_layout->setSpacing(5);
  mac_editor_layout->setContentsMargins(mac_editor_layout->margin(),
                                        mac_editor_layout->margin() + 10,
                                        mac_editor_layout->margin(),
                                        mac_editor_layout->margin());
  
  QLabel *mac_label = new QLabel(i18n("MAC address:"), mac_editors);
  
  m_mac_address = new KLineEdit(mac_editors);
  m_mac_address->setClearButtonShown(true);
  
  mac_label->setBuddy(m_mac_address);
  
  // If you change the texts here please also alter them in the custom 
  // options dialog.
  m_send_before_scan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), mac_editors);
  m_send_before_scan->setEnabled(false);
  m_send_before_mount = new QCheckBox(i18n("Send magic package before mounting a share"), mac_editors);
  m_send_before_mount->setEnabled(false);
  
  mac_editor_layout->addWidget(mac_label, 0, 0, 0);
  mac_editor_layout->addWidget(m_mac_address, 0, 1, 0);
  mac_editor_layout->addWidget(m_send_before_scan, 1, 0, 1, 2, 0);
  mac_editor_layout->addWidget(m_send_before_mount, 2, 0, 1, 2, 0);
  
  wol_tab_layout->addWidget(mac_editors);
  wol_tab_layout->addStretch(100);
  
  (void)m_tab_widget->insertTab(SambaTab, samba_tab, i18n("Samba"));
  (void)m_tab_widget->insertTab(WolTab, wol_tab, i18n("Wake-On-LAN"));
  
  editors_layout->addWidget(m_general_editors);
  editors_layout->addWidget(m_tab_widget);
                                 
  custom_layout->addWidget(m_custom_options);
  custom_layout->addWidget(editors);

  m_general_editors->setEnabled(false);
  m_tab_widget->setEnabled(false);

  clearEditors();
  
  //
  // Connections
  //
  connect(m_custom_options, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
          this,             SLOT(slotEditCustomItem(QListWidgetItem*)));
  connect(m_custom_options, SIGNAL(itemSelectionChanged()),
          this,             SLOT(slotItemSelectionChanged()));
  connect(m_custom_options, SIGNAL(customContextMenuRequested(QPoint)),
          this,             SLOT(slotCustomContextMenuRequested(QPoint)));
  connect(edit_action,      SIGNAL(triggered(bool)),
          this,             SLOT(slotEditActionTriggered(bool)));
  connect(remove_action,    SIGNAL(triggered(bool)),
          this,             SLOT(slotRemoveActionTriggered(bool)));
  connect(clear_action,     SIGNAL(triggered(bool)),
          this,             SLOT(slotClearActionTriggered(bool)));
  connect(undo_action,      SIGNAL(triggered(bool)),
          this,             SLOT(slotUndoActionTriggered(bool)));
  connect(m_ip_address,     SIGNAL(textChanged(QString)),
          this,             SLOT(slotEntryChanged()));
  connect(m_remount_share,  SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_smb_port,       SIGNAL(valueChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_user_id,        SIGNAL(currentIndexChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_group_id,       SIGNAL(currentIndexChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_kerberos,       SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_mac_address,    SIGNAL(textChanged(QString)),
          this,             SLOT(slotEntryChanged()));
  connect(m_send_before_scan,   SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_send_before_mount,  SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_mac_address,    SIGNAL(textChanged(QString)),
          this,             SLOT(slotEnableWOLFeatures(QString)));
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsPage::setupWidget()
{
  QHBoxLayout *custom_layout = new QHBoxLayout(this);
  custom_layout->setSpacing(5);
  custom_layout->setMargin(0);
  
  //
  // The list widget
  //  
  m_custom_options = new QListWidget(this);
  m_custom_options->setObjectName("CustomOptionsList");
  m_custom_options->viewport()->installEventFilter(this);
  m_custom_options->setSelectionMode(QListWidget::SingleSelection);
  m_custom_options->setContextMenuPolicy(Qt::CustomContextMenu);

  m_menu = new KActionMenu(m_custom_options);

  QAction *edit_action = new QAction(KDE::icon("edit-rename"), i18n("Edit"), m_collection);
  edit_action->setEnabled(false);

  QAction *remove_action = new QAction(KDE::icon("edit-delete"), i18n("Remove"), m_collection);
  remove_action->setEnabled(false);
  
  QAction *clear_action = new QAction(KDE::icon("edit-clear-list"), i18n("Clear List"), m_collection);
  clear_action->setEnabled(false);
  
  QAction *undo_action = new QAction(KDE::icon("edit-undo"), i18n("Undo"), m_collection);
  undo_action->setEnabled(false);
  
  m_collection->addAction("edit_action", edit_action);
  m_collection->addAction("remove_action", remove_action);
  m_collection->addAction("clear_action", clear_action);
  m_collection->addAction("undo_action", undo_action);

  m_menu->addAction(edit_action);
  m_menu->addAction(remove_action);
  m_menu->addAction(clear_action);
  m_menu->addAction(undo_action);
  
  //
  // The editors
  //  
  QWidget *editors = new QWidget(this);
  
  QVBoxLayout *editors_layout = new QVBoxLayout(editors);
  editors_layout->setSpacing(5);
  editors_layout->setMargin(0);
  
  m_general_editors = new QGroupBox(i18n("General"), editors);
  
  QGridLayout *general_editor_layout = new QGridLayout(m_general_editors);
  general_editor_layout->setSpacing(5);
  general_editor_layout->setContentsMargins(general_editor_layout->margin(),
                                            general_editor_layout->margin() + 10,
                                            general_editor_layout->margin(),
                                            general_editor_layout->margin());

  QLabel *unc_label = new QLabel(i18n("UNC Address:"), m_general_editors);
  
  m_unc_address = new KLineEdit(m_general_editors);
  m_unc_address->setReadOnly(true);

  unc_label->setBuddy(m_unc_address);
  
  QLabel *ip_label = new QLabel(i18n("IP Address:"), m_general_editors);
  
  m_ip_address = new KLineEdit(m_general_editors);
  m_ip_address->setClearButtonShown(true);
  
  ip_label->setBuddy(m_ip_address);
  
  general_editor_layout->addWidget(unc_label, 0, 0, 0);
  general_editor_layout->addWidget(m_unc_address, 0, 1, 0);
  general_editor_layout->addWidget(ip_label, 1, 0, 0);
  general_editor_layout->addWidget(m_ip_address, 1, 1, 0);
  
  m_tab_widget = new QTabWidget(editors);
  
  QWidget *samba_tab = new QWidget(m_tab_widget);
  
  QVBoxLayout *samba_tab_layout = new QVBoxLayout(samba_tab);
  samba_tab_layout->setSpacing(5);
  samba_tab_layout->setMargin(0);
  
  QGroupBox *samba_editors = new QGroupBox(samba_tab);
  samba_editors->setFlat(true);
  
  QGridLayout *samba_editor_layout = new QGridLayout(samba_editors);
  samba_editor_layout->setSpacing(5);
  samba_editor_layout->setContentsMargins(samba_editor_layout->margin(),
                                          samba_editor_layout->margin() + 10,
                                          samba_editor_layout->margin(),
                                          samba_editor_layout->margin());
  
  QLabel *smb_port_label = new QLabel("SMB Port:", samba_editors);
  
  m_smb_port = new QSpinBox(samba_editors);
  m_smb_port->setRange(Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt());
//   m_smb_port->setSliderEnabled(true);

  smb_port_label->setBuddy(m_smb_port);

  m_kerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), samba_editors);

  samba_editor_layout->addWidget(smb_port_label, 0, 0, 0);
  samba_editor_layout->addWidget(m_smb_port, 0, 1, 0);
  samba_editor_layout->addWidget(m_kerberos, 1, 0, 1, 2, 0);
  
  samba_tab_layout->addWidget(samba_editors);
  samba_tab_layout->addStretch(100);

  QWidget *wol_tab = new QWidget(m_tab_widget);
  
  QVBoxLayout *wol_tab_layout = new QVBoxLayout(wol_tab);
  wol_tab_layout->setSpacing(5);
  wol_tab_layout->setMargin(0);  
  
  QGroupBox *mac_editors = new QGroupBox(wol_tab);
  mac_editors->setFlat(true);
  
  QGridLayout *mac_editor_layout = new QGridLayout(mac_editors);
  mac_editor_layout->setSpacing(5);
  mac_editor_layout->setContentsMargins(mac_editor_layout->margin(),
                                        mac_editor_layout->margin() + 10,
                                        mac_editor_layout->margin(),
                                        mac_editor_layout->margin());
  
  QLabel *mac_label = new QLabel(i18n("MAC address:"), mac_editors);
  
  m_mac_address = new KLineEdit(mac_editors);
  m_mac_address->setClearButtonShown(true);
  
  mac_label->setBuddy(m_mac_address);
  
  // If you change the texts here please also alter them in the custom 
  // options dialog.
  m_send_before_scan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), mac_editors);
  m_send_before_scan->setEnabled(false);
  m_send_before_mount = new QCheckBox(i18n("Send magic package before mounting a share"), mac_editors);
  m_send_before_mount->setEnabled(false);
  
  mac_editor_layout->addWidget(mac_label, 0, 0, 0);
  mac_editor_layout->addWidget(m_mac_address, 0, 1, 0);
  mac_editor_layout->addWidget(m_send_before_scan, 1, 0, 1, 2, 0);
  mac_editor_layout->addWidget(m_send_before_mount, 2, 0, 1, 2, 0);
  
  wol_tab_layout->addWidget(mac_editors);
  wol_tab_layout->addStretch(100);
  
  (void)m_tab_widget->insertTab(SambaTab, samba_tab, i18n("Samba"));
  (void)m_tab_widget->insertTab(WolTab, wol_tab, i18n("Wake-On-LAN"));
  
  editors_layout->addWidget(m_general_editors);
  editors_layout->addWidget(m_tab_widget);
                                 
  custom_layout->addWidget(m_custom_options);
  custom_layout->addWidget(editors);

  m_general_editors->setEnabled(false);
  m_tab_widget->setEnabled(false);

  clearEditors();
  
  //
  // Connections
  //
  connect(m_custom_options, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
          this,             SLOT(slotEditCustomItem(QListWidgetItem*)));
  connect(m_custom_options, SIGNAL(itemSelectionChanged()),
          this,             SLOT(slotItemSelectionChanged()));
  connect(m_custom_options, SIGNAL(customContextMenuRequested(QPoint)),
          this,             SLOT(slotCustomContextMenuRequested(QPoint)));
  connect(edit_action,      SIGNAL(triggered(bool)),
          this,             SLOT(slotEditActionTriggered(bool)));
  connect(remove_action,    SIGNAL(triggered(bool)),
          this,             SLOT(slotRemoveActionTriggered(bool)));
  connect(clear_action,     SIGNAL(triggered(bool)),
          this,             SLOT(slotClearActionTriggered(bool)));
  connect(undo_action,      SIGNAL(triggered(bool)),
          this,             SLOT(slotUndoActionTriggered(bool)));
  connect(m_ip_address,     SIGNAL(textChanged(QString)),
          this,             SLOT(slotEntryChanged()));
  connect(m_smb_port,       SIGNAL(valueChanged(int)),
          this,             SLOT(slotEntryChanged()));
  connect(m_kerberos,       SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_mac_address,    SIGNAL(textChanged(QString)),
          this,             SLOT(slotEntryChanged()));
  connect(m_send_before_scan,   SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_send_before_mount,  SIGNAL(toggled(bool)),
          this,             SLOT(slotEntryChanged()));
  connect(m_mac_address,    SIGNAL(textChanged(QString)),
          this,             SLOT(slotEnableWOLFeatures(QString)));
}
#endif


void Smb4KCustomOptionsPage::insertCustomOptions(const QList<Smb4KCustomOptions*> &list)
{
  // Insert those options that are not there.
  for (int i = 0; i < list.size(); ++i)
  {
    Smb4KCustomOptions *options = findOptions(list.at(i)->url().toDisplayString());
    
    if (!options)
    {
      m_options_list << new Smb4KCustomOptions(*list[i]);
    }
    else
    {
      // Do nothing
    }
  }
  
  // Clear the list widget before (re)displaying the list
  while (m_custom_options->count() != 0)
  {
    delete m_custom_options->item(0);
  }
  
  // Display the list.
  if (m_custom_options)
  {
    for (int i = 0; i < m_options_list.size(); ++i)
    {
      switch (m_options_list.at(i)->type())
      {
        case Host:
        {
          QListWidgetItem *item = new QListWidgetItem(KDE::icon("network-server"), 
                                      m_options_list.at(i)->unc(),
                                      m_custom_options, Host);
          item->setData(Qt::UserRole, m_options_list.at(i)->url().toDisplayString());
          break;
        }
        case Share:
        {
          QListWidgetItem *item = new QListWidgetItem(KDE::icon("folder-remote"), 
                                      m_options_list.at(i)->unc(),
                                      m_custom_options, Share);
          item->setData(Qt::UserRole, m_options_list.at(i)->url().toDisplayString());
          break;
        }
        default:
        {
          break;
        }
      }
    }

    m_custom_options->sortItems(Qt::AscendingOrder);
  }
  else
  {
    // Do nothing
  }
  
  m_removed = false;
}


const QList< Smb4KCustomOptions* > Smb4KCustomOptionsPage::getCustomOptions()
{
  return m_options_list;
}


#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KCustomOptionsPage::clearEditors()
{
  // Do not reset the current custom options object here,
  // so that we can undo the last changes!
  
  // Clearing the editors means to reset them to their initial/default values.
  m_unc_address->clear();
  m_ip_address->clear();
  m_remount_share->setChecked(false);
  m_smb_port->setValue(Smb4KSettings::remoteSMBPort());
  m_fs_port->setValue(Smb4KMountSettings::remoteFileSystemPort());

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

  KUser user(KUser::UseRealUserID);
  m_user_id->setCurrentItem(QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()));
  KUserGroup group(KUser::UseRealUserID);
  m_group_id->setCurrentItem(QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()));
  m_kerberos->setChecked(false);
  m_mac_address->clear();
  m_send_before_scan->setChecked(false);
  m_send_before_mount->setChecked(false);
  
  // Disable widgets
  m_general_editors->setEnabled(false);
  m_tab_widget->setEnabled(false);
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KCustomOptionsPage::clearEditors()
{
  // Do not reset the current custom options object here,
  // so that we can undo the last changes!
  
  // Clearing the editors means to reset them to their initial/default values.
  m_unc_address->clear();
  m_ip_address->clear();
  m_remount_share->setChecked(false);
  m_smb_port->setValue(Smb4KSettings::remoteSMBPort());

  KUser user(KUser::UseRealUserID);
  m_user_id->setCurrentItem(QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()));
  KUserGroup group(KUser::UseRealUserID);
  m_group_id->setCurrentItem(QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()));
  m_kerberos->setChecked(false);
  m_mac_address->clear();
  m_send_before_scan->setChecked(false);
  m_send_before_mount->setChecked(false);
  
  // Disable widgets
  m_general_editors->setEnabled(false);
  m_tab_widget->setEnabled(false);
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsPage::clearEditors()
{
  // Do not reset the current custom options object here,
  // so that we can undo the last changes!
  
  // Clearing the editors means to reset them to their initial/default values.
  m_unc_address->clear();
  m_ip_address->clear();
  m_smb_port->setValue(Smb4KSettings::remoteSMBPort());

  m_kerberos->setChecked(false);
  m_mac_address->clear();
  m_send_before_scan->setChecked(false);
  m_send_before_mount->setChecked(false);
  
  // Disable widgets
  m_general_editors->setEnabled(false);
  m_tab_widget->setEnabled(false);
}
#endif


Smb4KCustomOptions* Smb4KCustomOptionsPage::findOptions(const QString& url)
{
  Smb4KCustomOptions *options = NULL;
  
  for (int i = 0; i < m_options_list.size(); ++i)
  {
    if (QString::compare(url, m_options_list.at(i)->url().toDisplayString(), Qt::CaseInsensitive) == 0)
    {
      options = m_options_list[i];
      break;
    }
    else
    {
      continue;
    }
  }
  
  return options;
}


#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KCustomOptionsPage::populateEditors(Smb4KCustomOptions* options)
{
  // Commit changes
  commitChanges();
  
  // Copy custom options object
  m_current_options = new Smb4KCustomOptions(*options);
  
  // Populate the editors with the stored values.
  m_unc_address->setText(m_current_options->unc());
  
  if (!m_current_options->ip().isEmpty())
  {
    m_ip_address->setText(m_current_options->ip());
  }
  else
  {
    // Do nothing
  }
  
  if (m_current_options->remount() == Smb4KCustomOptions::RemountAlways)
  {
    m_remount_share->setChecked(true);
  }
  else
  {
    m_remount_share->setChecked(false);
  }
  
  if (m_current_options->smbPort() != -1)
  {
    m_smb_port->setValue(m_current_options->smbPort());
  }
  else
  {
    m_smb_port->setValue(Smb4KSettings::remoteSMBPort());
  }
  
  if (m_current_options->fileSystemPort() != -1)
  {
    m_fs_port->setValue(m_current_options->fileSystemPort());
  }
  else
  {
    m_fs_port->setValue(Smb4KMountSettings::remoteFileSystemPort());
  }
  
  if (m_current_options->writeAccess() == Smb4KCustomOptions::UndefinedWriteAccess)
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
    switch (m_current_options->writeAccess())
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

  if (m_current_options->securityMode() == Smb4KCustomOptions::UndefinedSecurityMode)
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
    switch (m_current_options->securityMode())
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

  m_user_id->setCurrentItem(QString("%1 (%2)").arg(m_current_options->user().loginName()).arg(m_current_options->user().userId().nativeId()));
  m_group_id->setCurrentItem(QString("%1 (%2)").arg(m_current_options->group().name()).arg(m_current_options->group().groupId().nativeId()));
  
  if (m_current_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos)
  {
    m_kerberos->setChecked(Smb4KSettings::useKerberos());
  }
  else
  {
    switch (m_current_options->useKerberos())
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
  
  m_mac_address->setText(m_current_options->macAddress());
  m_send_before_scan->setChecked(m_current_options->wolSendBeforeNetworkScan());
  m_send_before_mount->setChecked(m_current_options->wolSendBeforeMount());
  
  // Enable widget
  m_general_editors->setEnabled(true);
  m_tab_widget->setEnabled(true);
  m_tab_widget->widget(SambaTab)->setEnabled(true);
  
  if (m_current_options->type() == Host)
  {
    m_tab_widget->widget(WolTab)->setEnabled(Smb4KSettings::enableWakeOnLAN());
    m_remount_share->setEnabled(false);
  }
  else
  {
    m_tab_widget->widget(WolTab)->setEnabled(false);
    m_remount_share->setEnabled(true);
  }
  
  slotEnableWOLFeatures(m_mac_address->text());
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KCustomOptionsPage::populateEditors(Smb4KCustomOptions* options)
{
  // Commit changes
  commitChanges();
  
  // Copy custom options object
  m_current_options = new Smb4KCustomOptions(*options);
  
  // Populate the editors with the stored values.
  m_unc_address->setText(m_current_options->unc());
  
  if (!m_current_options->ip().isEmpty())
  {
    m_ip_address->setText(m_current_options->ip());
  }
  else
  {
    // Do nothing
  }
  
  if (m_current_options->remount() == Smb4KCustomOptions::RemountAlways)
  {
    m_remount_share->setChecked(true);
  }
  else
  {
    m_remount_share->setChecked(false);
  }
  
  if (m_current_options->smbPort() != -1)
  {
    m_smb_port->setValue(m_current_options->smbPort());
  }
  else
  {
    m_smb_port->setValue(Smb4KSettings::remoteSMBPort());
  }
  
  m_user_id->setCurrentItem(QString("%1 (%2)").arg(m_current_options->user().loginName()).arg(m_current_options->user().userId().nativeId()));
  m_group_id->setCurrentItem(QString("%1 (%2)").arg(m_current_options->group().name()).arg(m_current_options->group().groupId().nativeId()));
  
  if (m_current_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos)
  {
    m_kerberos->setChecked(Smb4KSettings::useKerberos());
  }
  else
  {
    switch (m_current_options->useKerberos())
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
  
  m_mac_address->setText(m_current_options->macAddress());
  m_send_before_scan->setChecked(m_current_options->wolSendBeforeNetworkScan());
  m_send_before_mount->setChecked(m_current_options->wolSendBeforeMount());
  
  // Enable widget
  m_general_editors->setEnabled(true);
  m_tab_widget->setEnabled(true);
  m_tab_widget->widget(SambaTab)->setEnabled(true);
  
  if (m_current_options->type() == Host)
  {
    m_tab_widget->widget(WolTab)->setEnabled(Smb4KSettings::enableWakeOnLAN());
    m_remount_share->setEnabled(false);
  }
  else
  {
    m_tab_widget->widget(WolTab)->setEnabled(false);
    m_remount_share->setEnabled(true);
  }
  
  slotEnableWOLFeatures(m_mac_address->text());
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsPage::populateEditors(Smb4KCustomOptions *options)
{
  // Commit changes
  commitChanges();
  
  // Copy custom options object
  m_current_options = new Smb4KCustomOptions(*options);
  
  // Populate the editors with the stored values.
  m_unc_address->setText(m_current_options->unc());
  
  if (!m_current_options->ip().isEmpty())
  {
    m_ip_address->setText(m_current_options->ip());
  }
  else
  {
    // Do nothing
  }
  
  if (m_current_options->smbPort() != -1)
  {
    m_smb_port->setValue(m_current_options->smbPort());
  }
  else
  {
    m_smb_port->setValue(Smb4KSettings::remoteSMBPort());
  }
  
  if (m_current_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos)
  {
    m_kerberos->setChecked(Smb4KSettings::useKerberos());
  }
  else
  {
    switch (m_current_options->useKerberos())
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
  
  m_mac_address->setText(m_current_options->macAddress());
  m_send_before_scan->setChecked(m_current_options->wolSendBeforeNetworkScan());
  m_send_before_mount->setChecked(m_current_options->wolSendBeforeMount());
  
  // Enable widget
  m_general_editors->setEnabled(true);
  m_tab_widget->setEnabled(true);
  m_tab_widget->widget(SambaTab)->setEnabled(true);
  
  if (m_current_options->type() == Host)
  {
    m_tab_widget->widget(WolTab)->setEnabled(Smb4KSettings::enableWakeOnLAN());
  }
  else
  {
    m_tab_widget->widget(WolTab)->setEnabled(false);
  }
  
  slotEnableWOLFeatures(m_mac_address->text());
}
#endif

#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KCustomOptionsPage::commitChanges()
{
  if (m_current_options && !m_options_list.isEmpty() &&
      QString::compare(m_current_options->unc(), m_unc_address->text()) == 0)
  {
    Smb4KCustomOptions *options = findOptions(m_current_options->url().toDisplayString());
    
    QHostAddress addr(m_ip_address->text());
    
    if (addr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      options->setIP(m_ip_address->text());
    }
    else
    {
      // Do nothing
    }
    
    if (m_remount_share->isChecked())
    {
      options->setRemount(Smb4KCustomOptions::RemountAlways);
    }
    else
    {
      options->setRemount(Smb4KCustomOptions::RemountNever);
    }
    
    options->setSMBPort(m_smb_port->value());
    options->setFileSystemPort(m_fs_port->value());
    options->setWriteAccess((Smb4KCustomOptions::WriteAccess)m_write_access->itemData(m_write_access->currentIndex()).toInt());
    options->setSecurityMode((Smb4KCustomOptions::SecurityMode)m_security_mode->itemData(m_security_mode->currentIndex()).toInt());
    options->setUser(KUser(m_user_id->itemData(m_user_id->currentIndex()).toInt()));
    options->setGroup(KUserGroup(m_group_id->itemData(m_group_id->currentIndex()).toInt()));

    if (m_kerberos->isChecked())
    {
      options->setUseKerberos(Smb4KCustomOptions::UseKerberos);
    }
    else
    {
      options->setUseKerberos(Smb4KCustomOptions::NoKerberos);
    }
    
    QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
    
    if (exp.exactMatch(m_mac_address->text()))
    {
      options->setMACAddress(m_mac_address->text());
    }
    else
    {
      // Do nothing
    }
    
    options->setWOLSendBeforeNetworkScan(m_send_before_scan->isChecked());
    options->setWOLSendBeforeMount(m_send_before_mount->isChecked());
    
    // In case of a host, propagate the changes to its shares.
    if (options->type() == Host)
    {
      for (int i = 0; i < m_options_list.size(); ++i)
      {
        if (m_options_list.at(i)->type() == Share &&
            QString::compare(m_options_list.at(i)->hostName(), options->hostName(), Qt::CaseInsensitive) == 0 &&
            QString::compare(m_options_list.at(i)->workgroupName(), options->workgroupName(), Qt::CaseInsensitive) == 0)
        {
          // Propagate the options to the shared resources of the host.
          // They overwrite the ones defined for the shares.
          m_options_list[i]->setSMBPort(options->smbPort());
          m_options_list[i]->setFileSystemPort(options->fileSystemPort());
          m_options_list[i]->setWriteAccess(options->writeAccess());
          m_options_list[i]->setSecurityMode(options->securityMode());
          m_options_list[i]->setUser(options->user());
          m_options_list[i]->setGroup(options->group());
          m_options_list[i]->setUseKerberos(options->useKerberos());
          m_options_list[i]->setMACAddress(options->macAddress());
          m_options_list[i]->setWOLSendBeforeNetworkScan(options->wolSendBeforeNetworkScan());
          m_options_list[i]->setWOLSendBeforeMount(options->wolSendBeforeMount());
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // Do nothing
    }
    
    m_maybe_changed = true;
    emit customSettingsModified();
  }
  else
  {
    // Do nothing
  }
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KCustomOptionsPage::commitChanges()
{
  if (m_current_options && !m_options_list.isEmpty() &&
      QString::compare(m_current_options->unc(), m_unc_address->text()) == 0)
  {
    Smb4KCustomOptions *options = findOptions(m_current_options->url().toDisplayString());
    
    QHostAddress addr(m_ip_address->text());
    
    if (addr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      options->setIP(m_ip_address->text());
    }
    else
    {
      // Do nothing
    }
    
    if (m_remount_share->isChecked())
    {
      options->setRemount(Smb4KCustomOptions::RemountAlways);
    }
    else
    {
      options->setRemount(Smb4KCustomOptions::RemountNever);
    }
    
    options->setSMBPort(m_smb_port->value());
    options->setUser(KUser(m_user_id->itemData(m_user_id->currentIndex()).toInt()));
    options->setGroup(KUserGroup(m_group_id->itemData(m_group_id->currentIndex()).toInt()));

    if (m_kerberos->isChecked())
    {
      options->setUseKerberos(Smb4KCustomOptions::UseKerberos);
    }
    else
    {
      options->setUseKerberos(Smb4KCustomOptions::NoKerberos);
    }
    
    QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
    
    if (exp.exactMatch(m_mac_address->text()))
    {
      options->setMACAddress(m_mac_address->text());
    }
    else
    {
      // Do nothing
    }
    
    options->setWOLSendBeforeNetworkScan(m_send_before_scan->isChecked());
    options->setWOLSendBeforeMount(m_send_before_mount->isChecked());
    
    // In case of a host, propagate the changes to its shares.
    if (options->type() == Host)
    {
      for (int i = 0; i < m_options_list.size(); ++i)
      {
        if (m_options_list.at(i)->type() == Share &&
            QString::compare(m_options_list.at(i)->hostName() , options->hostName(), Qt::CaseInsensitive) == 0 &&
            QString::compare(m_options_list.at(i)->workgroupName() , options->workgroupName(), Qt::CaseInsensitive) == 0)
        {
          // Propagate the options to the shared resources of the host.
          // They overwrite the ones defined for the shares.
          m_options_list[i]->setSMBPort(options->smbPort());
          m_options_list[i]->setUser(options->user());
          m_options_list[i]->setGroup(options->group());
          m_options_list[i]->setUseKerberos(options->useKerberos());
          m_options_list[i]->setMACAddress(options->macAddress());
          m_options_list[i]->setWOLSendBeforeNetworkScan(options->wolSendBeforeNetworkScan());
          m_options_list[i]->setWOLSendBeforeMount(options->wolSendBeforeMount());
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // Do nothing
    }
    
    m_maybe_changed = true;
    emit customSettingsModified();
  }
  else
  {
    // Do nothing
  }
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsPage::commitChanges()
{
  if (m_current_options && !m_options_list.isEmpty() &&
      QString::compare(m_current_options->unc(), m_unc_address->text()) == 0)
  {
    Smb4KCustomOptions *options = findOptions(m_current_options->url().toDisplayString());
    
    QHostAddress addr(m_ip_address->text());
    
    if (addr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      options->setIP(m_ip_address->text());
    }
    else
    {
      // Do nothing
    }
    
    options->setSMBPort(m_smb_port->value());

    if (m_kerberos->isChecked())
    {
      options->setUseKerberos(Smb4KCustomOptions::UseKerberos);
    }
    else
    {
      options->setUseKerberos(Smb4KCustomOptions::NoKerberos);
    }
    
    QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
    
    if (exp.exactMatch(m_mac_address->text()))
    {
      options->setMACAddress(m_mac_address->text());
    }
    else
    {
      // Do nothing
    }
    
    options->setWOLSendBeforeNetworkScan(m_send_before_scan->isChecked());
    options->setWOLSendBeforeMount(m_send_before_mount->isChecked());
    
    // In case of a host, propagate the changes to its shares.
    if (options->type() == Host)
    {
      for (int i = 0; i < m_options_list.size(); ++i)
      {
        if (m_options_list.at(i)->type() == Share &&
            QString::compare(m_options_list.at(i)->hostName(), options->hostName(), Qt::CaseInsensitive) == 0 &&
            QString::compare(m_options_list.at(i)->workgroupName(), options->workgroupName(), Qt::CaseInsensitive) == 0)
        {
          // Propagate the options to the shared resources of the host.
          // They overwrite the ones defined for the shares.
          m_options_list[i]->setSMBPort(options->smbPort());
          m_options_list[i]->setUseKerberos(options->useKerberos());
          m_options_list[i]->setMACAddress(options->macAddress());
          m_options_list[i]->setWOLSendBeforeNetworkScan(options->wolSendBeforeNetworkScan());
          m_options_list[i]->setWOLSendBeforeMount(options->wolSendBeforeMount());
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // Do nothing
    }
    
    m_maybe_changed = true;
    emit customSettingsModified();
  }
  else
  {
    // Do nothing
  }
}
#endif


bool Smb4KCustomOptionsPage::eventFilter(QObject* obj, QEvent* e)
{
  if (obj == m_custom_options->viewport())
  {
    if (e->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *mev = static_cast<QMouseEvent *>(e);
      QPoint pos = m_custom_options->viewport()->mapFromGlobal(mev->globalPos());
      QListWidgetItem *item = m_custom_options->itemAt(pos);
      
      if (!item)
      {
        clearEditors();
        m_custom_options->clearSelection();
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
  }
  else
  {
    // Do nothing
  }
  
  return QObject::eventFilter(obj, e);
}



void Smb4KCustomOptionsPage::slotEditCustomItem(QListWidgetItem *item)
{
  Smb4KCustomOptions *options = findOptions(item->data(Qt::UserRole).toString());
  
  if (options)
  {
    populateEditors(options);
  }
  else
  {
    clearEditors();
  }
}


void Smb4KCustomOptionsPage::slotItemSelectionChanged()
{
  clearEditors();
}


void Smb4KCustomOptionsPage::slotCustomContextMenuRequested(const QPoint& pos)
{
  QListWidgetItem *item = m_custom_options->itemAt(pos);
  
  if (item)
  {
    m_collection->action("edit_action")->setEnabled(true);
    m_collection->action("remove_action")->setEnabled(true);
  }
  else
  {
    m_collection->action("edit_action")->setEnabled(false);
    m_collection->action("remove_action")->setEnabled(false);
  }
  
  m_collection->action("clear_action")->setEnabled(m_custom_options->count() != 0);
  m_collection->action("undo_action")->setEnabled(m_current_options || m_removed);
  
  m_menu->menu()->popup(m_custom_options->viewport()->mapToGlobal(pos));
}


void Smb4KCustomOptionsPage::slotEditActionTriggered(bool /*checked*/)
{
  slotEditCustomItem(m_custom_options->currentItem());
}


void Smb4KCustomOptionsPage::slotRemoveActionTriggered(bool /*checked*/)
{
  QListWidgetItem *item = m_custom_options->currentItem();
  Smb4KCustomOptions *options = findOptions(item->data(Qt::UserRole).toString());
  
  if (item && options)
  {
    if (m_current_options && m_current_options->url().matches(options->url(), QUrl::StripTrailingSlash))
    {
      delete m_current_options;
      m_current_options = 0;
    }
    else
    {
      // Do nothing
    }
    
    int index = m_options_list.indexOf(options);
    
    if (index != -1)
    {
      m_options_list.removeAt(index);
    }
    else
    {
      // Do nothing
    }
    
    if (QString::compare(item->text(), m_unc_address->text(), Qt::CaseInsensitive) == 0)
    {
      clearEditors();
    }
    else
    {
      // Do nothing
    }
    
    delete item;
    
    m_removed = true;
    m_maybe_changed = true;
    emit customSettingsModified();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KCustomOptionsPage::slotClearActionTriggered(bool /*checked*/)
{
  clearEditors();

  while (m_custom_options->count() != 0)
  {
    delete m_custom_options->item(0);
  }
  
  while (!m_options_list.isEmpty())
  {
    delete m_options_list.takeFirst();
  }
  
  delete m_current_options;
  m_current_options = NULL;

  m_removed = true;
  m_maybe_changed = true;
  emit customSettingsModified();
}


void Smb4KCustomOptionsPage::slotUndoActionTriggered(bool /*checked*/)
{
  if (m_removed)
  {
    emit reloadCustomSettings();
  }
  else
  {
    if (m_current_options)
    {
      if (QString::compare(m_custom_options->currentItem()->data(Qt::UserRole).toString(),
                            m_current_options->url().toDisplayString(), Qt::CaseInsensitive) == 0)
      {
        // Populate the editor with the original values and commit
        // the changes.
        populateEditors(m_current_options);
        commitChanges();
      }
      else
      {
        // Copy the original values to the appropriate options object
        // in the list.
        Smb4KCustomOptions *options = findOptions(m_current_options->url().toDisplayString());
        
        if (options)
        {
          options->setSMBPort(m_current_options->smbPort());
#ifdef Q_OS_LINUX
          options->setFileSystemPort(m_current_options->fileSystemPort());
          options->setWriteAccess(m_current_options->writeAccess());
          options->setSecurityMode(m_current_options->securityMode());
#endif
          options->setUser(m_current_options->user());
          options->setGroup(m_current_options->group());
          options->setUseKerberos(m_current_options->useKerberos());
          options->setMACAddress(m_current_options->macAddress());
          options->setWOLSendBeforeNetworkScan(m_current_options->wolSendBeforeNetworkScan());
          options->setWOLSendBeforeMount(m_current_options->wolSendBeforeMount());
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  
  m_maybe_changed = true;
  emit customSettingsModified();
}


void Smb4KCustomOptionsPage::slotEntryChanged()
{
  commitChanges();
}


void Smb4KCustomOptionsPage::slotEnableWOLFeatures(const QString &mac_address)
{
  QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
    
  m_send_before_scan->setEnabled(exp.exactMatch(mac_address));
  m_send_before_mount->setEnabled(exp.exactMatch(mac_address));
}

