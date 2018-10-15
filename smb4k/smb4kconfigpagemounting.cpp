/***************************************************************************
    The configuration page for the mount options
                             -------------------
    begin                : So Mär 22 2015
    copyright            : (C) 2015-2018 by Alexander Reinholdt
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
  // Common Settings tab
  //
  QWidget *commonTab = new QWidget(this);
  QVBoxLayout *commonTabLayout = new QVBoxLayout(commonTab);
  commonTabLayout->setSpacing(5);
  commonTabLayout->setMargin(0);

  // 
  // Common options group box
  // 
  QGroupBox *commonOptions = new QGroupBox(i18n("Common Options"), commonTab);

  QGridLayout *commonOptionsLayout = new QGridLayout(commonOptions);
  commonOptionsLayout->setSpacing(5);

  // Write access
  QCheckBox *useWriteAccess = new QCheckBox(Smb4KMountSettings::self()->useWriteAccessItem()->label(), commonOptions);
  useWriteAccess->setObjectName("kcfg_UseWriteAccess");

  KComboBox *writeAccess = new KComboBox(commonOptions);
  writeAccess->setObjectName("kcfg_WriteAccess");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> writeAccessChoices = Smb4KMountSettings::self()->writeAccessItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &wa : writeAccessChoices)
  {
    writeAccess->addItem(wa.label);
  }
  
  commonOptionsLayout->addWidget(useWriteAccess, 0, 0, 0);
  commonOptionsLayout->addWidget(writeAccess, 0, 1, 0);

  // Character set
  QCheckBox *useCharacterSet = new QCheckBox(Smb4KMountSettings::self()->useClientCharsetItem()->label(), commonOptions);
  useCharacterSet->setObjectName("kcfg_UseClientCharset");

  KComboBox *characterSet = new KComboBox(commonOptions);
  characterSet->setObjectName("kcfg_ClientCharset");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> charsetChoices = Smb4KMountSettings::self()->clientCharsetItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &c : charsetChoices)
  {
    characterSet->addItem(c.label);
  }
  
  commonOptionsLayout->addWidget(useCharacterSet, 1, 0, 0);
  commonOptionsLayout->addWidget(characterSet, 1, 1, 0);

  // Remote filesystem port
  QCheckBox *useFilesystemPort = new QCheckBox(Smb4KMountSettings::self()->useRemoteFileSystemPortItem()->label(), commonOptions);
  useFilesystemPort->setObjectName("kcfg_UseRemoteFileSystemPort");
  
  QSpinBox *filesystemPort = new QSpinBox(commonOptions);
  filesystemPort->setObjectName("kcfg_RemoteFileSystemPort");

  commonOptionsLayout->addWidget(useFilesystemPort, 2, 0, 0);
  commonOptionsLayout->addWidget(filesystemPort, 2, 1, 0);
  
  commonTabLayout->addWidget(commonOptions, 0, 0);
  
  //
  // CIFS Unix Extensions Support group box
  // 
  QGroupBox *cifsExtensionSupportBox= new QGroupBox(i18n("CIFS Unix Extensions Support"), commonTab);
  
  QGridLayout *cifsExtensionSupportLayout = new QGridLayout(cifsExtensionSupportBox);
  cifsExtensionSupportLayout->setSpacing(5);
  
  // CIFS Unix extensions support
  QCheckBox *cifsExtensionsSupport = new QCheckBox(Smb4KMountSettings::self()->cifsUnixExtensionsSupportItem()->label(), cifsExtensionSupportBox);
  cifsExtensionsSupport->setObjectName("kcfg_CifsUnixExtensionsSupport");
  
  cifsExtensionSupportLayout->addWidget(cifsExtensionsSupport, 0, 0, 1, 2, 0);
  
  // User information
  QCheckBox *useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), cifsExtensionSupportBox);
  useUserId->setObjectName("kcfg_UseUserId");

  QWidget *userIdInputWidget = new QWidget(cifsExtensionSupportBox);
  userIdInputWidget->setObjectName("UserIdInputWidget");

  QGridLayout *userLayout = new QGridLayout(userIdInputWidget);
  userLayout->setSpacing(5);
  userLayout->setMargin(0);

  KLineEdit *userId = new KLineEdit(userIdInputWidget);
  userId->setObjectName("kcfg_UserId");
  userId->setAlignment(Qt::AlignRight);
  userId->setReadOnly(true);

  QToolButton *userChooser = new QToolButton(userIdInputWidget);
  userChooser->setIcon(KDE::icon("edit-find-user"));
  userChooser->setToolTip(i18n("Choose a different user"));
  userChooser->setPopupMode(QToolButton::InstantPopup);

  QMenu *userMenu = new QMenu(userChooser);
  userChooser->setMenu(userMenu);
  
  QList<KUser> allUsers = KUser::allUsers();

  for (const KUser &u : allUsers)
  {
    QAction *userAction = userMenu->addAction(QString("%1 (%2)").arg(u.loginName()).arg(u.userId().nativeId()));
    userAction->setData(u.userId().nativeId());
  }

  userLayout->addWidget(userId, 0, 0, 0);
  userLayout->addWidget(userChooser, 0, 1, Qt::AlignCenter);
  
  cifsExtensionSupportLayout->addWidget(useUserId, 1, 0, 0);
  cifsExtensionSupportLayout->addWidget(userIdInputWidget, 1, 1, 0);

  // Group information
  QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), cifsExtensionSupportBox);
  useGroupId->setObjectName("kcfg_UseGroupId");

  QWidget *groupIdInputWidget = new QWidget(cifsExtensionSupportBox);
  groupIdInputWidget->setObjectName("GroupIdInputWidget");

  QGridLayout *groupLayout = new QGridLayout(groupIdInputWidget);
  groupLayout->setSpacing(5);
  groupLayout->setMargin(0);

  KLineEdit *groupId = new KLineEdit(groupIdInputWidget);
  groupId->setObjectName("kcfg_GroupId");
  groupId->setAlignment(Qt::AlignRight);
  groupId->setReadOnly(true);
  
  QToolButton *groupChooser = new QToolButton(groupIdInputWidget);
  groupChooser->setIcon(KDE::icon("edit-find-user"));
  groupChooser->setToolTip(i18n("Choose a different group"));
  groupChooser->setPopupMode(QToolButton::InstantPopup);

  QMenu *groupMenu = new QMenu(groupChooser);
  groupChooser->setMenu(groupMenu);

  QList<KUserGroup> groupList = KUserGroup::allGroups();
  
  for (const KUserGroup &g : groupList)
  {
    QAction *groupAction = groupMenu->addAction(QString("%1 (%2)").arg(g.name()).arg(g.groupId().nativeId()));
    groupAction->setData(g.groupId().nativeId());
  }

  groupLayout->addWidget(groupId, 0, 0, 0);
  groupLayout->addWidget(groupChooser, 0, 1, Qt::AlignCenter);
  
  cifsExtensionSupportLayout->addWidget(useGroupId, 2, 0, 0);
  cifsExtensionSupportLayout->addWidget(groupIdInputWidget, 2, 1, 0);

  // File mask
  QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), cifsExtensionSupportBox);
  useFileMode->setObjectName("kcfg_UseFileMode");

  KLineEdit *fileMode = new KLineEdit(cifsExtensionSupportBox);
  fileMode->setObjectName("kcfg_FileMode");
  fileMode->setClearButtonEnabled(true);
  fileMode->setAlignment(Qt::AlignRight);
  
  cifsExtensionSupportLayout->addWidget(useFileMode, 3, 0, 0);
  cifsExtensionSupportLayout->addWidget(fileMode, 3, 1, 0);

  // Directory mask
  QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), cifsExtensionSupportBox);
  useDirectoryMode->setObjectName("kcfg_UseDirectoryMode");

  KLineEdit *directoryMode = new KLineEdit(cifsExtensionSupportBox);
  directoryMode->setObjectName("kcfg_DirectoryMode");
  directoryMode->setClearButtonEnabled(true);
  directoryMode->setAlignment(Qt::AlignRight);

  cifsExtensionSupportLayout->addWidget(useDirectoryMode, 4, 0, 0);
  cifsExtensionSupportLayout->addWidget(directoryMode, 4, 1, 0);

  commonTabLayout->addWidget(cifsExtensionSupportBox, 1, 0);
  commonTabLayout->addStretch(100);
  
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
  
  // Force Uid
  QCheckBox *forceUid = new QCheckBox(Smb4KMountSettings::self()->forceUIDItem()->label(), advancedOptions);
  forceUid->setObjectName("kcfg_ForceUID");
  
  advancedOptionsLayout->addWidget(forceUid, 0, 0, 0);
  
  // Force Gid
  QCheckBox *forceGid = new QCheckBox(Smb4KMountSettings::self()->forceGIDItem()->label(), advancedOptions);
  forceGid->setObjectName("kcfg_ForceGID");
  
  advancedOptionsLayout->addWidget(forceGid, 0, 1, 0);

  // Permission checks
  QCheckBox *permissionChecks = new QCheckBox(Smb4KMountSettings::self()->permissionChecksItem()->label(), advancedOptions);
  permissionChecks->setObjectName("kcfg_PermissionChecks");
  
  advancedOptionsLayout->addWidget(permissionChecks, 1, 0, 0);
  
  // Client controls Ids
  QCheckBox *clientControlsIds = new QCheckBox(Smb4KMountSettings::self()->clientControlsIDsItem()->label(), advancedOptions);
  clientControlsIds->setObjectName("kcfg_ClientControlsIDs");
  
  advancedOptionsLayout->addWidget(clientControlsIds, 1, 1, 0);

  // Use server inode numbers
  QCheckBox *useServerInodes = new QCheckBox(Smb4KMountSettings::self()->serverInodeNumbersItem()->label(), advancedOptions);
  useServerInodes->setObjectName("kcfg_ServerInodeNumbers");
  
  advancedOptionsLayout->addWidget(useServerInodes, 2, 0, 0);

  // Translate reserved characters
  QCheckBox *translateReservedCharacters = new QCheckBox(Smb4KMountSettings::self()->translateReservedCharsItem()->label(), advancedOptions);
  translateReservedCharacters->setObjectName("kcfg_TranslateReservedChars");
  
  advancedOptionsLayout->addWidget(translateReservedCharacters, 2, 1, 0);
  
  // No locking
  QCheckBox *no_locking = new QCheckBox(Smb4KMountSettings::self()->noLockingItem()->label(), advancedOptions);
  no_locking->setObjectName("kcfg_NoLocking");
  
  advancedOptionsLayout->addWidget(no_locking, 3, 0, 0);
  
  // Extra widget for the rest of the options
  QWidget *advancedOptionsExtraWidget = new QWidget(advancedOptions);

  QGridLayout *advancedOptionsExtraWidgetLayout = new QGridLayout(advancedOptionsExtraWidget);
  advancedOptionsExtraWidgetLayout->setSpacing(5);
  advancedOptionsExtraWidgetLayout->setMargin(0);
  
  // SMB protocol version
  QCheckBox *useSmbProtocol = new QCheckBox(Smb4KMountSettings::self()->useSmbProtocolVersionItem()->label(), advancedOptionsExtraWidget);
  useSmbProtocol->setObjectName("kcfg_UseSmbProtocolVersion");

  KComboBox *smbProtocol = new KComboBox(advancedOptionsExtraWidget);
  smbProtocol->setObjectName("kcfg_SmbProtocolVersion");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> smbProtocolChoices = Smb4KMountSettings::self()->smbProtocolVersionItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &c : smbProtocolChoices)
  {
    smbProtocol->addItem(c.label);
  }
  
  advancedOptionsExtraWidgetLayout->addWidget(useSmbProtocol, 0, 0, 0);
  advancedOptionsExtraWidgetLayout->addWidget(smbProtocol, 0, 1, 0);
  
  // Cache mode
  QCheckBox *useCacheMode = new QCheckBox(Smb4KMountSettings::self()->useCacheModeItem()->label(), advancedOptionsExtraWidget);
  useCacheMode->setObjectName("kcfg_UseCacheMode");
  
  KComboBox *cacheMode = new KComboBox(advancedOptionsExtraWidget);
  cacheMode->setObjectName("kcfg_CacheMode");
  
  QList<KCoreConfigSkeleton::ItemEnum::Choice> cacheModeChoices = Smb4KMountSettings::self()->cacheModeItem()->choices();
  
  for (const KCoreConfigSkeleton::ItemEnum::Choice &c : cacheModeChoices)
  {
    cacheMode->addItem(c.label);
  }
  
  advancedOptionsExtraWidgetLayout->addWidget(useCacheMode, 1, 0, 0);
  advancedOptionsExtraWidgetLayout->addWidget(cacheMode, 1, 1, 0);
  
  // Security mode
  QCheckBox *useSecurityMode = new QCheckBox(Smb4KMountSettings::self()->useSecurityModeItem()->label(), advancedOptionsExtraWidget);
  useSecurityMode->setObjectName("kcfg_UseSecurityMode");
                                 
  KComboBox *securityMode = new KComboBox(advancedOptionsExtraWidget);
  securityMode->setObjectName("kcfg_SecurityMode");
  
  QList<KConfigSkeleton::ItemEnum::Choice> securityModeChoices = Smb4KMountSettings::self()->securityModeItem()->choices();
  
  for (const KConfigSkeleton::ItemEnum::Choice &c : securityModeChoices)
  {
    securityMode->addItem(c.label);
  }
  
  advancedOptionsExtraWidgetLayout->addWidget(useSecurityMode, 2, 0, 0);
  advancedOptionsExtraWidgetLayout->addWidget(securityMode, 2, 1, 0);

  // Additional options
  QLabel *additionalOptionsLabel = new QLabel(Smb4KMountSettings::self()->customCIFSOptionsItem()->label(), advancedOptionsExtraWidget);
  
  QWidget *additionalOptionsWidget = new QWidget(advancedOptionsExtraWidget);
  
  QHBoxLayout *additionalOptionsWidgetLayout = new QHBoxLayout(additionalOptionsWidget);
  additionalOptionsWidgetLayout->setSpacing(5);
  additionalOptionsWidgetLayout->setMargin(0);

  KLineEdit *additionalOptions = new KLineEdit(additionalOptionsWidget);
  additionalOptions->setObjectName("kcfg_CustomCIFSOptions");
  additionalOptions->setReadOnly(true);
  additionalOptions->setClearButtonEnabled(true);
  additionalOptionsLabel->setBuddy(additionalOptions);
  
  QToolButton *additionalOptionsEdit = new QToolButton(advancedOptionsExtraWidget);
  additionalOptionsEdit->setIcon(KDE::icon("document-edit"));
  additionalOptionsEdit->setToolTip(i18n("Edit the additional CIFS options."));
  
  additionalOptionsWidgetLayout->addWidget(additionalOptions, 0);
  additionalOptionsWidgetLayout->addWidget(additionalOptionsEdit, 0);

  advancedOptionsExtraWidgetLayout->addWidget(additionalOptionsLabel, 3, 0, 0);
  advancedOptionsExtraWidgetLayout->addWidget(additionalOptionsWidget, 3, 1, 0);

  advancedOptionsLayout->addWidget(advancedOptionsExtraWidget, 4, 0, 1, 2, 0);
  
  advancedTabLayout->addWidget(advancedOptions, 0);
  advancedTabLayout->addStretch(100);
  
  addTab(advancedTab, i18n("Advanced Settings"));
  
  //
  // Adjust widgets
  // 
  slotCIFSUnixExtensionsSupport(Smb4KMountSettings::cifsUnixExtensionsSupport());
  
  //
  // Connections
  //
  connect(userMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotNewUserTriggered(QAction*)));
  connect(groupMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotNewGroupTriggered(QAction*)));
  connect(cifsExtensionsSupport, SIGNAL(clicked(bool)), this, SLOT(slotCIFSUnixExtensionsSupport(bool)));
  connect(additionalOptionsEdit, SIGNAL(clicked(bool)), this, SLOT(slotAdditionalCIFSOptions()));  
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
  user_id->setObjectName("kcfg_UserId");
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
  group_id->setObjectName("kcfg_GroupId");
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

  QLabel *fmask_label = new QLabel(Smb4KMountSettings::self()->fileModeItem()->label(), commonOptions);

  KLineEdit *fmask = new KLineEdit(commonOptions);
  fmask->setObjectName("kcfg_FileMode");
  fmask->setAlignment(Qt::AlignRight);

  fmask_label->setBuddy(fmask);

  QLabel *dmask_label = new QLabel(Smb4KMountSettings::self()->directoryModeItem()->label(), commonOptions);

  KLineEdit *dmask = new KLineEdit(commonOptions);
  dmask->setObjectName("kcfg_DirectoryMode");
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
  KLineEdit *userId = findChild<KLineEdit *>("kcfg_UserId");

  if (userId)
  {
    userId->setText(action->data().toString());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigPageMounting::slotNewGroupTriggered(QAction *action)
{
  KLineEdit *groupId = findChild<KLineEdit *>("kcfg_GroupId");

  if (groupId)
  {
    groupId->setText(action->data().toString());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigPageMounting::slotCIFSUnixExtensionsSupport(bool checked)
{
  QCheckBox *useUserId = findChild<QCheckBox *>("kcfg_UseUserId");
  
  if (useUserId)
  {
    useUserId->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
  
  QWidget *userIdInputWidget = findChild<QWidget *>("UserIdInputWidget");
  
  if (userIdInputWidget)
  {
    userIdInputWidget->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *useGroupId = findChild<QCheckBox *>("kcfg_UseGroupId");
  
  if (useGroupId)
  {
    useGroupId->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
  
  QWidget *groupIdInputWidget = findChild<QWidget *>("GroupIdInputWidget");
  
  if (groupIdInputWidget)
  {
    groupIdInputWidget->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *useFileMode = findChild<QCheckBox *>("kcfg_UseFileMode");
  
  if (useFileMode)
  {
    useFileMode->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("kcfg_FileMode");
  
  if (fileMode)
  {
    fileMode->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("kcfg_UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    useDirectoryMode->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("kcfg_DirectoryMode");
  
  if (directoryMode)
  {
    directoryMode->setEnabled(!checked);
  }
  else
  {
    // Do nothing
  }
}



void Smb4KConfigPageMounting::slotAdditionalCIFSOptions()
{
  KLineEdit *cifsOptions = findChild<KLineEdit *>("kcfg_CustomCIFSOptions");
  
  if (cifsOptions)
  {
    QString options = cifsOptions->originalText();
    
    bool ok = false;
    options = QInputDialog::getText(this, i18n("Additional CIFS Options"), i18n("<qt>Enter the desired options as a comma separated list:</qt>"), QLineEdit::Normal, options, &ok);
    
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
        QStringList deniedArgs;
        QStringList list = options.split(',', QString::SkipEmptyParts);
        QMutableStringListIterator it(list);
        
        while (it.hasNext())
        {
          QString arg = it.next().section("=", 0, 0);
          
          if (!whitelist.contains(arg))
          {
            deniedArgs << arg;
            it.remove();
          }
          else
          {
            // Do nothing
          }
        }
        
        if (!deniedArgs.isEmpty())
        {
          QString msg = i18np("<qt>The following entry is going to be removed from the additional options: %2. Please read the handbook for details.</qt>", "<qt>The following %1 entries are going to be removed from the additional options: %2. Please read the handbook for details.</qt>", deniedArgs.size(), deniedArgs.join(", "));
          KMessageBox::sorry(this, msg);
        }
        else
        {
          // Do nothing
        }
        
        cifsOptions->setText(list.join(",").trimmed());
      }
      else
      {
        cifsOptions->clear();
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
}

