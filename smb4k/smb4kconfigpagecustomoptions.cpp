/***************************************************************************
    The configuration page for the custom options
                             -------------------
    begin                : Sa Jan 19 2013
    copyright            : (C) 2013-2019 by Alexander Reinholdt
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
#include "smb4kconfigpagecustomoptions.h"
#include "core/smb4ksettings.h"
#include "core/smb4kcustomoptions.h"
#include "core/smb4kglobal.h"

#if defined(Q_OS_LINUX)
#include "core/smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "core/smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;


Smb4KConfigPageCustomOptions::Smb4KConfigPageCustomOptions(QWidget *parent) : QWidget(parent)
{
  m_maybe_changed = false;
  
  //
  // Layout 
  // 
  QHBoxLayout *layout = new QHBoxLayout(this);
  setLayout(layout);
  
  //
  // Set up the list widget
  // 
  QListWidget *optionsListWidget = new QListWidget(this);
  optionsListWidget->setObjectName("OptionsListWidget");
  optionsListWidget->setSelectionMode(QListWidget::SingleSelection);
  optionsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  
  connect(optionsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(slotEditCustomItem(QListWidgetItem*)));
  connect(optionsListWidget, SIGNAL(itemSelectionChanged()), SLOT(slotItemSelectionChanged()));
  connect(optionsListWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotCustomContextMenuRequested(QPoint)));
  
  layout->addWidget(optionsListWidget, 0);
  
  QAction *editAction = new QAction(KDE::icon("edit-rename"), i18n("Edit"), optionsListWidget);
  editAction->setObjectName("edit_action");
  editAction->setEnabled(false);
  
  QAction  *removeAction = new QAction(KDE::icon("edit-delete"), i18n("Remove"), optionsListWidget);
  removeAction->setObjectName("remove_action");
  removeAction->setEnabled(false);
  
  QAction *clearAction = new QAction(KDE::icon("edit-clear-list"), i18n("Clear List"), optionsListWidget);
  clearAction->setObjectName("clear_action");
  clearAction->setEnabled(false);
  
  optionsListWidget->addAction(editAction);
  optionsListWidget->addAction(removeAction);
  optionsListWidget->addAction(clearAction);
  
  KActionMenu *actionMenu = new KActionMenu(optionsListWidget);
  actionMenu->setObjectName("ActionMenu");
  actionMenu->addAction(editAction);
  actionMenu->addAction(removeAction);
  actionMenu->addAction(clearAction);
  
  connect(editAction, SIGNAL(triggered(bool)), SLOT(slotEditActionTriggered(bool)));
  connect(removeAction, SIGNAL(triggered(bool)), SLOT(slotRemoveActionTriggered(bool)));
  connect(clearAction, SIGNAL(triggered(bool)), SLOT(slotClearActionTriggered(bool)));
  
  //
  // Set up the tab widget
  // 
  QTabWidget *tabWidget = new QTabWidget(this);
  tabWidget->setObjectName("TabWidget");
  layout->addWidget(tabWidget, 0);
  
  //
  // Network item tab
  //
  QWidget *itemTab = new QWidget(tabWidget);
  QVBoxLayout *itemTabLayout = new QVBoxLayout(itemTab);
  
  // Identification
  QGroupBox *identificationBox = new QGroupBox(i18n("Identification"), itemTab);
  QGridLayout *identificationBoxLayout = new QGridLayout(identificationBox);
  
  QLabel *workgroupLabel = new QLabel(i18n("Workgroup:"), identificationBox);
  KLineEdit *workgroup = new KLineEdit(identificationBox);
  workgroup->setObjectName("Workgroup");
//   workgroup->setClearButtonEnabled(true);
  workgroup->setReadOnly(true);
  workgroupLabel->setBuddy(workgroup);
  
  identificationBoxLayout->addWidget(workgroupLabel, 0, 0, 0);
  identificationBoxLayout->addWidget(workgroup, 0, 1, 0);
  
  QLabel *locationLabel = new QLabel(i18n("Location:"), identificationBox);
  KLineEdit *location = new KLineEdit(identificationBox);
  location->setObjectName("Location");
//   location->setClearButtonEnabled(true);
  location->setReadOnly(true);
  locationLabel->setBuddy(location);
  
  identificationBoxLayout->addWidget(locationLabel, 1, 0, 0);
  identificationBoxLayout->addWidget(location, 1, 1, 0);
  
  QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), identificationBox);
  KLineEdit *ipAddress = new KLineEdit(identificationBox);
  ipAddress->setObjectName("IPAddress");
  ipAddress->setClearButtonEnabled(true);
  ipAddressLabel->setBuddy(ipAddress);
  
  identificationBoxLayout->addWidget(ipAddressLabel, 2, 0, 0);
  identificationBoxLayout->addWidget(ipAddress, 2, 1, 0);
  
  itemTabLayout->addWidget(identificationBox, 0);
  itemTabLayout->addStretch(100);
  
  tabWidget->addTab(itemTab, i18n("Network Item"));
  
  //
  // Mounting tab
  // 
  setupMountingTab();
  
  //
  // Samba tab
  // 
  QWidget *sambaTab = new QWidget(tabWidget);
  QVBoxLayout *sambaTabLayout = new QVBoxLayout(sambaTab);
  
  // Common Options
  QGroupBox *commonSambaOptionsBox = new QGroupBox(i18n("Common Options"), sambaTab);
  QGridLayout *commonSambaOptionsBoxLayout = new QGridLayout(commonSambaOptionsBox);
  
  // SMB port
  QCheckBox *useSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonSambaOptionsBox);
  useSmbPort->setObjectName("UseSmbPort");
  
  QSpinBox *smbPort = new QSpinBox(commonSambaOptionsBox);
  smbPort->setObjectName("SmbPort");
  smbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
  smbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());
//   smbPort->setSliderEnabled(true);
  
  commonSambaOptionsBoxLayout->addWidget(useSmbPort, 0, 0, 0);
  commonSambaOptionsBoxLayout->addWidget(smbPort, 0, 1, 0);
  
  sambaTabLayout->addWidget(commonSambaOptionsBox, 0);
  
  // Authentication
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
  QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);
  
  // Kerberos
  QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
  useKerberos->setObjectName("UseKerberos");

  authenticationBoxLayout->addWidget(useKerberos, 0);
  
  sambaTabLayout->addWidget(authenticationBox, 0);
  sambaTabLayout->addStretch(100);  
  
  tabWidget->addTab(sambaTab, i18n("Samba"));
  
  //
  // Wake-On-LAN tab
  //
  // NOTE: If you change the texts here, also alter them in the custom
  // options dialog
  // 
  QWidget *wakeOnLanTab = new QWidget(tabWidget);
  QVBoxLayout *wakeOnLanTabLayout = new QVBoxLayout(wakeOnLanTab);
  
  // MAC address
  QGroupBox *macAddressBox = new QGroupBox(i18n("MAC Address"), wakeOnLanTab);
  QGridLayout *macAddressBoxLayout = new QGridLayout(macAddressBox);
  
  // MAC address
  QLabel *macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressBox);
  KLineEdit *macAddress = new KLineEdit(macAddressBox);
  macAddress->setObjectName("MACAddress");
  macAddress->setClearButtonEnabled(true);
  macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_"); // MAC address, see QLineEdit doc
  macAddressLabel->setBuddy(macAddress);
  
  macAddressBoxLayout->addWidget(macAddressLabel, 0, 0, 0);
  macAddressBoxLayout->addWidget(macAddress, 0, 1, 0);

  wakeOnLanTabLayout->addWidget(macAddressBox, 0);
  
  // Wake-On-LAN Actions
  QGroupBox *wakeOnLANActionsBox = new QGroupBox(i18n("Actions"), wakeOnLanTab);
  QVBoxLayout *wakeOnLANActionsBoxLayout = new QVBoxLayout(wakeOnLANActionsBox);
  
  // Send magic package before network scan
  QCheckBox *sendPackageBeforeScan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), wakeOnLANActionsBox);
  sendPackageBeforeScan->setObjectName("SendPackageBeforeScan");
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeScan, 0);
  
  // Send magic package before mount
  QCheckBox *sendPackageBeforeMount = new QCheckBox(i18n("Send magic package before mounting a share"), wakeOnLanTab);
  sendPackageBeforeMount->setObjectName("SendPackageBeforeMount");
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeMount, 0);
  
  wakeOnLanTabLayout->addWidget(wakeOnLANActionsBox, 0);
  wakeOnLanTabLayout->addStretch(100);

  tabWidget->addTab(wakeOnLanTab, i18n("Wake-On-LAN"));
  
  //
  // Clear the editor widgets, i.e. set them to the default empty values.
  // 
  clearEditors();  
}


Smb4KConfigPageCustomOptions::~Smb4KConfigPageCustomOptions()
{
}

#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KConfigPageCustomOptions::setupMountingTab()
{
  //
  // Get the tab widget
  // 
  QTabWidget *tabWidget = findChild<QTabWidget *>("TabWidget");
  
  //
  // Custom options for mounting
  // 
  QWidget *mountingTab = new QWidget(tabWidget);
  QVBoxLayout *mountingTabLayout = new QVBoxLayout(mountingTab);
  
  //
  // Common options
  //
  QGroupBox *commonBox = new QGroupBox(i18n("Common Options"), mountingTab);
  QGridLayout *commonBoxLayout = new QGridLayout(commonBox);
  
  QCheckBox *remountAlways = new QCheckBox(i18n("Always remount this share"), commonBox);
  remountAlways->setObjectName("RemountAlways");
  remountAlways->setEnabled(false);
  
  commonBoxLayout->addWidget(remountAlways, 0, 0, 1, 2, 0);
  
  // Write access
  QCheckBox *useWriteAccess = new QCheckBox(Smb4KMountSettings::self()->useWriteAccessItem()->label(), commonBox);
  useWriteAccess->setObjectName("UseWriteAccess");
  
  KComboBox *writeAccess = new KComboBox(commonBox);
  writeAccess->setObjectName("WriteAccess");
  
  QString readWriteText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label;
  QString readOnlyText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label;
  
  writeAccess->addItem(readWriteText);
  writeAccess->addItem(readOnlyText);
  
  commonBoxLayout->addWidget(useWriteAccess, 1, 0, 0);
  commonBoxLayout->addWidget(writeAccess, 1, 1, 0);
  
  // Remote file system port
  QCheckBox *useFilesystemPort = new QCheckBox(Smb4KMountSettings::self()->useRemoteFileSystemPortItem()->label(), commonBox);
  useFilesystemPort->setObjectName("UseFilesystemPort");
  
  QSpinBox *filesystemPort = new QSpinBox(commonBox);
  filesystemPort->setObjectName("FileSystemPort");
  filesystemPort->setMinimum(Smb4KMountSettings::self()->remoteFileSystemPortItem()->minValue().toInt());
  filesystemPort->setMaximum(Smb4KMountSettings::self()->remoteFileSystemPortItem()->maxValue().toInt());
//   filesystemPort->setSliderEnabled(true);

  commonBoxLayout->addWidget(useFilesystemPort, 2, 0, 0);
  commonBoxLayout->addWidget(filesystemPort, 2, 1, 0);
  
  mountingTabLayout->addWidget(commonBox, 0);
  
  //
  // CIFS Unix Extensions Support
  // 
  QGroupBox *extensionsSupportBox = new QGroupBox(i18n("CIFS Unix Extensions Support"), mountingTab);
  QGridLayout *extensionsSupportBoxLayout = new QGridLayout(extensionsSupportBox);
  
  QCheckBox *cifsExtensionsSupport = new QCheckBox(i18n("This server supports the CIFS Unix extensions"), extensionsSupportBox);
  cifsExtensionsSupport->setObjectName("CifsExtensionsSupport");
  
  extensionsSupportBoxLayout->addWidget(cifsExtensionsSupport, 0, 0, 1, 2, 0);
  
  // User Id
  QCheckBox *useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), extensionsSupportBox);
  useUserId->setObjectName("UseUserId");
  
  KComboBox *userId = new KComboBox(extensionsSupportBox);
  userId->setObjectName("UserId");
  
  QList<KUser> allUsers = KUser::allUsers();

  for (const KUser &u : allUsers)
  {
    userId->addItem(QString("%1 (%2)").arg(u.loginName()).arg(u.userId().nativeId()), QVariant::fromValue<K_GID>(u.groupId().nativeId()));
  }
  
  extensionsSupportBoxLayout->addWidget(useUserId, 1, 0, 0);
  extensionsSupportBoxLayout->addWidget(userId, 1, 1, 0);
  
  // Group Id
  QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), extensionsSupportBox);
  useGroupId->setObjectName("UseGroupId");
  
  KComboBox *groupId = new KComboBox(extensionsSupportBox);
  groupId->setObjectName("GroupId");
  
  QList<KUserGroup> allGroups = KUserGroup::allGroups();

  for (const KUserGroup &g : allGroups)
  {
    groupId->addItem(QString("%1 (%2)").arg(g.name()).arg(g.groupId().nativeId()), QVariant::fromValue<K_GID>(g.groupId().nativeId()));
  }
  
  extensionsSupportBoxLayout->addWidget(useGroupId, 2, 0, 0);
  extensionsSupportBoxLayout->addWidget(groupId, 2, 1, 0);
  
  // File mode
  QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), extensionsSupportBox);
  useFileMode->setObjectName("UseFileMode");
  
  KLineEdit *fileMode = new KLineEdit(extensionsSupportBox);
  fileMode->setObjectName("FileMode");
  fileMode->setClearButtonEnabled(true);
  fileMode->setAlignment(Qt::AlignRight);
  
  extensionsSupportBoxLayout->addWidget(useFileMode, 3, 0, 0);
  extensionsSupportBoxLayout->addWidget(fileMode, 3, 1, 0);
  
  // Directory mode
  QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), extensionsSupportBox);
  useDirectoryMode->setObjectName("UseDirectoryMode");
  
  KLineEdit *directoryMode = new KLineEdit(extensionsSupportBox);
  directoryMode->setObjectName("DirectoryMode");
  directoryMode->setClearButtonEnabled(true);
  directoryMode->setAlignment(Qt::AlignRight);
  
  extensionsSupportBoxLayout->addWidget(useDirectoryMode, 4, 0, 0);
  extensionsSupportBoxLayout->addWidget(directoryMode, 4, 1, 0);
  
  mountingTabLayout->addWidget(extensionsSupportBox, 0);
  
  //
  // Advanced options
  // 
  QGroupBox *advancedOptionsBox = new QGroupBox(i18n("Advanced Options"), mountingTab);
  QGridLayout *advancedOptionsBoxLayout = new QGridLayout(advancedOptionsBox);
  
  // Security mode
  QCheckBox *useSecurityMode = new QCheckBox(Smb4KMountSettings::self()->useSecurityModeItem()->label(), advancedOptionsBox);
  useSecurityMode->setObjectName("UseSecurityMode");
  
  KComboBox *securityMode = new KComboBox(advancedOptionsBox);
  securityMode->setObjectName("SecurityMode");
  
  QString noneText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::None).label;
  QString krb5Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5).label;
  QString krb5iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5i).label;
  QString ntlmText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlm).label;
  QString ntlmiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmi).label;
  QString ntlmv2Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2).label;
  QString ntlmv2iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i).label;
  QString ntlmsspText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmssp).label;
  QString ntlmsspiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi).label;
  
  securityMode->addItem(noneText);
  securityMode->addItem(krb5Text);
  securityMode->addItem(krb5iText);
  securityMode->addItem(ntlmText);
  securityMode->addItem(ntlmiText);
  securityMode->addItem(ntlmv2Text);
  securityMode->addItem(ntlmv2iText);
  securityMode->addItem(ntlmsspText);
  
  advancedOptionsBoxLayout->addWidget(useSecurityMode, 0, 0, 0);
  advancedOptionsBoxLayout->addWidget(securityMode, 0, 1, 0);
  
  mountingTabLayout->addWidget(advancedOptionsBox, 0);
  mountingTabLayout->addStretch(100);  
  
  tabWidget->addTab(mountingTab, i18n("Mounting"));
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KConfigPageCustomOptions::setupMountingTab()
{
  //
  // Get the tab widget
  // 
  QTabWidget *tabWidget = findChild<QTabWidget *>("TabWidget");
  
  //
  // Custom options for mounting
  // 
  QWidget *mountingTab = new QWidget(tabWidget);
  QVBoxLayout *mountingTabLayout = new QVBoxLayout(mountingTab);
  
  //
  // Common options
  //
  QGroupBox *commonBox = new QGroupBox(i18n("Common Options"), mountingTab);
  QGridLayout *commonBoxLayout = new QGridLayout(commonBox);
  
  QCheckBox *remountAlways = new QCheckBox(i18n("Always remount this share"), commonBox);
  remountAlways->setObjectName("RemountAlways");
  remountAlways->setEnabled(false);
  
  commonBoxLayout->addWidget(remountAlways, 0, 0, 1, 2, 0);
  
  // User Id
  QCheckBox *useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), commonBox);
  useUserId->setObjectName("UseUserId");
  
  KComboBox *userId = new KComboBox(commonBox);
  userId->setObjectName("UserId");
  
  QList<KUser> allUsers = KUser::allUsers();

  for (const KUser &u : allUsers)
  {
    userId->addItem(QString("%1 (%2)").arg(u.loginName()).arg(u.userId().nativeId()), QVariant::fromValue<K_GID>(u.groupId().nativeId()));
  }
  
  commonBoxLayout->addWidget(useUserId, 1, 0, 0);
  commonBoxLayout->addWidget(userId, 1, 1, 0);
  
  // Group Id
  QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), commonBox);
  useGroupId->setObjectName("UseGroupId");
  
  KComboBox *groupId = new KComboBox(commonBox);
  groupId->setObjectName("GroupId");
  
  QList<KUserGroup> allGroups = KUserGroup::allGroups();

  for (const KUserGroup &g : allGroups)
  {
    groupId->addItem(QString("%1 (%2)").arg(g.name()).arg(g.groupId().nativeId()), QVariant::fromValue<K_GID>(g.groupId().nativeId()));
  }
  
  commonBoxLayout->addWidget(useGroupId, 2, 0, 0);
  commonBoxLayout->addWidget(groupId, 2, 1, 0);
  
  // File mode
  QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), commonBox);
  useFileMode->setObjectName("UseFileMode");
  
  KLineEdit *fileMode = new KLineEdit(commonBox);
  fileMode->setObjectName("FileMode");
  fileMode->setClearButtonEnabled(true);
  fileMode->setAlignment(Qt::AlignRight);
  
  commonBoxLayout->addWidget(useFileMode, 3, 0, 0);
  commonBoxLayout->addWidget(fileMode, 3, 1, 0);
  
  // Directory mode
  QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), commonBox);
  useDirectoryMode->setObjectName("UseDirectoryMode");
  
  KLineEdit *directoryMode = new KLineEdit(commonBox);
  directoryMode->setObjectName("DirectoryMode");
  directoryMode->setClearButtonEnabled(true);
  directoryMode->setAlignment(Qt::AlignRight);
  
  commonBoxLayout->addWidget(useDirectoryMode, 4, 0, 0);
  commonBoxLayout->addWidget(directoryMode, 4, 1, 0);
  
  mountingTabLayout->addWidget(commonBox, 0);
  mountingTabLayout->addStretch(100);  
  
  tabWidget->addTab(mountingTab, i18n("Mounting"));
}
#else
//
// Generic (without mount options)
//
void Smb4KConfigPageCustomOptions::setupMountingTab()
{
  // The operating system is not support
}
#endif


void Smb4KConfigPageCustomOptions::insertCustomOptions(const QList<OptionsPtr> &list)
{
  //
  // If the global list of options has not been loaded, set it here
  // 
  if (m_optionsList.isEmpty())
  {
    m_optionsList = list;
  }
  
  //
  // Get the list widget and display the new options
  //
  QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");
  
  if (optionsListWidget)
  {
    // Clear the list widget
    while (optionsListWidget->count() != 0)
    {
      delete optionsListWidget->item(0);
    }
    
    // Display the new options
    for (const OptionsPtr &o : m_optionsList)
    {
      switch (o->type())
      {
        case Host:
        {
          QListWidgetItem *item = new QListWidgetItem(KDE::icon("network-server"), o->displayString(), optionsListWidget, Host);
          item->setData(Qt::UserRole, o->url().toDisplayString());
          break;
        }
        case Share:
        {
          QListWidgetItem *item = new QListWidgetItem(KDE::icon("folder-network"), o->displayString(), optionsListWidget, Share);
          item->setData(Qt::UserRole, o->url().toDisplayString());
          break;
        }
        default:
        {
          break;
        }
      }
    }

    optionsListWidget->sortItems(Qt::AscendingOrder);
  }
}


const QList<OptionsPtr> Smb4KConfigPageCustomOptions::getCustomOptions()
{
  return m_optionsList;
}


void Smb4KConfigPageCustomOptions::clearEditors()
{
  //
  // Clear current options
  //
  m_currentOptions.clear();
  
  //
  // Workgroup
  // 
  KLineEdit *workgroup = findChild<KLineEdit *>("Workgroup");
  
  if (workgroup)
  {
    workgroup->clear();
  }
  
  //
  // Location
  // 
  KLineEdit *location = findChild<KLineEdit *>("Location");
  
  if (location)
  {
    location->clear();
  }
  
  //
  // IP address
  // 
  KLineEdit *ipAddress = findChild<KLineEdit *>("IPAddress");
  
  if (ipAddress)
  {
    disconnect(ipAddress, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
    ipAddress->clear();
  }
  
  //
  // Remounting
  // 
  QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");
  
  if (remountAlways)
  {
    disconnect(remountAlways, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    remountAlways->setChecked(false);
  }
  
  //
  // User Id
  //
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    disconnect(useUserId, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useUserId->setChecked(false);
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    disconnect(userId, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
    KUser user((K_UID)Smb4KMountSettings::userId().toInt());
    userId->setCurrentText(QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()));
  }
  
  //
  // Group Id
  //
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    disconnect(useGroupId, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useGroupId->setChecked(false);
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    disconnect(groupId, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
    KUserGroup group((K_GID)Smb4KMountSettings::groupId().toInt());
    groupId->setCurrentText(QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()));
  }
  
  //
  // File mode
  // 
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    disconnect(useFileMode, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useFileMode->setChecked(false);
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    disconnect(fileMode, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
    fileMode->setText(Smb4KMountSettings::fileMode());
  }
  
  //
  // Directory mode
  // 
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    disconnect(useDirectoryMode, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useDirectoryMode->setChecked(false);
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    disconnect(directoryMode, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
    directoryMode->setText(Smb4KMountSettings::fileMode());
  }
  
#if defined(Q_OS_LINUX)
  //
  // Write access
  // 
  QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");
  
  if (useWriteAccess)
  {
    disconnect(useWriteAccess, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useWriteAccess->setChecked(false);
  }
  
  KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");
  
  if (writeAccess)
  {
    disconnect(writeAccess, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
    
    QString readWriteText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label;
    QString readOnlyText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label;

    switch (Smb4KMountSettings::writeAccess())
    {
      case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
      {
        writeAccess->setCurrentText(readWriteText);
        break;
      }
      case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
      {
        writeAccess->setCurrentText(readOnlyText);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  //
  // File system port
  // 
  QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");
  
  if (useFilesystemPort)
  {
    disconnect(useFilesystemPort, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useFilesystemPort->setChecked(false);
  }
  
  QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");
  
  if (filesystemPort)
  {
    disconnect(filesystemPort, SIGNAL(valueChanged(int)), this, SLOT(slotEntryChanged()));
    filesystemPort->setValue(Smb4KMountSettings::remoteFileSystemPort());
  }
  
  //
  // CIFS Unix Extensions Support
  //
  QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");
  
  if (cifsExtensionsSupport)
  {
    disconnect(cifsExtensionsSupport, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    disconnect(cifsExtensionsSupport, SIGNAL(toggled(bool)), this, SLOT(slotCifsUnixExtensionsSupport(bool)));
    cifsExtensionsSupport->setChecked(false);
  }
  
  //
  // Security mode
  // 
  QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");
  
  if (useSecurityMode)
  {
    disconnect(useSecurityMode, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useSecurityMode->setChecked(false);
  }
  
  KComboBox *securityMode = findChild<KComboBox *>("SecurityMode");
  
  if (securityMode)
  {
    disconnect(securityMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
    
    QString noneText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::None).label;
    QString krb5Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5).label;
    QString krb5iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5i).label;
    QString ntlmText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlm).label;
    QString ntlmiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmi).label;
    QString ntlmv2Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2).label;
    QString ntlmv2iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i).label;
    QString ntlmsspText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmssp).label;
    QString ntlmsspiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi).label;  
    
    switch (Smb4KMountSettings::securityMode())
    {
      case Smb4KMountSettings::EnumSecurityMode::None:
      {
        securityMode->setCurrentText(noneText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5:
      {
        securityMode->setCurrentText(krb5Text);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5i:
      {
        securityMode->setCurrentText(krb5iText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlm:
      {
        securityMode->setCurrentText(ntlmText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
      {
        securityMode->setCurrentText(ntlmiText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
      {
        securityMode->setCurrentText(ntlmv2Text);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
      {
        securityMode->setCurrentText(ntlmv2iText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
      {
        securityMode->setCurrentText(ntlmsspText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
      {
        securityMode->setCurrentText(ntlmsspiText);
        break;
      }
      default:
      {
        break;
      }
    }
  }
#endif
  
  //
  // SMB port
  // 
  QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");
  
  if (useSmbPort)
  {
    disconnect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useSmbPort->setChecked(false);
  }
  
  QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");
  
  if (smbPort)
  {
    disconnect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotEntryChanged()));
    smbPort->setValue(Smb4KSettings::remoteSmbPort());
  }
  
  //
  // Kerberos
  // 
  QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");
  
  if (useKerberos)
  {
    disconnect(useKerberos, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    useKerberos->setChecked(false);
  }
  
  //
  // MAC address
  // 
  KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");
  
  if (macAddress)
  {
    disconnect(macAddress, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
    disconnect(macAddress, SIGNAL(textEdited(QString)), this, SLOT(slotEnableWOLFeatures(QString)));
    macAddress->clear();
    macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_");
  }
  
  //
  // Wake-On-Lan: send package before scan
  // 
  QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
  
  if (sendPackageBeforeScan)
  {
    disconnect(sendPackageBeforeScan, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    sendPackageBeforeScan->setChecked(false);
  }
  
  //
  // Wake-On-Lan: Send package before mount
  // 
  QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
  
  if (sendPackageBeforeMount)
  {
    disconnect(sendPackageBeforeMount, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
    sendPackageBeforeMount->setChecked(false);
  }

  // 
  // Disable widgets
  //
  QTabWidget *tabWidget = findChild<QTabWidget *>("TabWidget");
  
  if (tabWidget)
  {
    tabWidget->setEnabled(false);
  }
}


void Smb4KConfigPageCustomOptions::setCurrentOptions(const QString& url)
{
  for (const OptionsPtr &o : m_optionsList)
  {
    if (url == o->url().toString())
    {
      m_currentOptions = o;
      break;
    }
  }
}


void Smb4KConfigPageCustomOptions::populateEditors()
{
  //
  // Workgroup
  // 
  KLineEdit *workgroup = findChild<KLineEdit *>("Workgroup");
  
  if (workgroup)
  {
    workgroup->setText(m_currentOptions->workgroupName());
  }
  
  //
  // Location
  // 
  KLineEdit *location = findChild<KLineEdit *>("Location");
  
  if (location)
  {
    location->setText(m_currentOptions->displayString());
  }
  
  //
  // IP address
  // 
  KLineEdit *ipAddress = findChild<KLineEdit *>("IPAddress");
  
  if (ipAddress)
  {
    ipAddress->setText(m_currentOptions->ipAddress());
    connect(ipAddress, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // Remounting
  // 
  QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");
  
  if (remountAlways)
  {
    remountAlways->setEnabled(m_currentOptions->type() == Share);
    
    if (m_currentOptions->remount() == Smb4KCustomOptions::RemountAlways)
    {
      remountAlways->setChecked(true);
    }
    else
    {
      remountAlways->setChecked(false);
    }
    
    connect(remountAlways, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // User Id
  //
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    useUserId->setChecked(m_currentOptions->useUser());
    connect(useUserId, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    userId->setCurrentText(QString("%1 (%2)").arg(m_currentOptions->user().loginName()).arg(m_currentOptions->user().userId().nativeId()));
    connect(userId, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // Group Id
  //
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    useGroupId->setChecked(m_currentOptions->useGroup());
    connect(useGroupId, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    groupId->setCurrentText(QString("%1 (%2)").arg(m_currentOptions->group().name()).arg(m_currentOptions->group().groupId().nativeId()));
    connect(groupId, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // File mode
  // 
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    useFileMode->setChecked(m_currentOptions->useFileMode());
    connect(useFileMode, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }

  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    fileMode->setText(m_currentOptions->fileMode());
    connect(fileMode, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // Directory mode
  // 
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    useDirectoryMode->setChecked(m_currentOptions->useFileMode());
    connect(useDirectoryMode, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    directoryMode->setText(m_currentOptions->directoryMode());
    connect(directoryMode, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
  }
  
#if defined(Q_OS_LINUX)
  //
  // Write access
  // 
  QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");
  
  if (useWriteAccess)
  {
    useWriteAccess->setChecked(m_currentOptions->useWriteAccess());
    connect(useWriteAccess, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");
  
  if (writeAccess)
  {
    QString readWriteText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label;
    QString readOnlyText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label;
    
    switch (m_currentOptions->writeAccess())
    {
      case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
      {
        writeAccess->setCurrentText(readWriteText);
        break;
      }
      case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
      {
        writeAccess->setCurrentText(readOnlyText);
        break;
      }
      default:
      {
        break;
      }
    }
    
    connect(writeAccess, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // File system port
  // 
  QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");
  
  if (useFilesystemPort)
  {
    useFilesystemPort->setChecked(m_currentOptions->useFileSystemPort());
    connect(useFilesystemPort, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");
  
  if (filesystemPort)
  {
    filesystemPort->setValue(m_currentOptions->fileSystemPort());
    connect(filesystemPort, SIGNAL(valueChanged(int)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // CIFS Unix Extensions Support
  // 
  QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");
  
  if (cifsExtensionsSupport)
  {
    connect(cifsExtensionsSupport, SIGNAL(toggled(bool)), this, SLOT(slotCifsUnixExtensionsSupport(bool)));
    cifsExtensionsSupport->setChecked(m_currentOptions->cifsUnixExtensionsSupport());
    connect(cifsExtensionsSupport, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // Security mode
  // 
  QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");
  
  if (useSecurityMode)
  {
    useSecurityMode->setChecked(m_currentOptions->useSecurityMode());
    connect(useSecurityMode, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  KComboBox *securityMode = findChild<KComboBox *>("SecurityMode");
  
  if (securityMode)
  {
    QString noneText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::None).label;
    QString krb5Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5).label;
    QString krb5iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5i).label;
    QString ntlmText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlm).label;
    QString ntlmiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmi).label;
    QString ntlmv2Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2).label;
    QString ntlmv2iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i).label;
    QString ntlmsspText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmssp).label;
    QString ntlmsspiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi).label;
    
    switch (m_currentOptions->securityMode())
    {
      case Smb4KMountSettings::EnumSecurityMode::None:
      {
        securityMode->setCurrentText(noneText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5:
      {
        securityMode->setCurrentText(krb5Text);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5i:
      {
        securityMode->setCurrentText(krb5iText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlm:
      {
        securityMode->setCurrentText(ntlmText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
      {
        securityMode->setCurrentText(ntlmiText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
      {
        securityMode->setCurrentText(ntlmv2Text);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
      {
        securityMode->setCurrentText(ntlmv2iText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
      {
        securityMode->setCurrentText(ntlmsspText);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
      {
        securityMode->setCurrentText(ntlmsspiText);
        break;
      }
      default:
      {
        break;
      }
    }
    
    connect(securityMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEntryChanged()));
  }
  
  slotCifsUnixExtensionsSupport(cifsExtensionsSupport->isChecked());
#endif

  //
  // SMB port
  // 
  QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");

  if (useSmbPort)
  {
    useSmbPort->setChecked(m_currentOptions->useSmbPort());
    connect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");
  
  if (smbPort)
  {
    smbPort->setValue(m_currentOptions->smbPort());
    connect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // Kerberos
  // 
  QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");
  
  if (useKerberos)
  {
    useKerberos->setChecked(m_currentOptions->useKerberos());
    connect(useKerberos, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // MAC address
  // 
  KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");
  
  if (macAddress)
  {
    macAddress->setText(m_currentOptions->macAddress());
    connect(macAddress, SIGNAL(textEdited(QString)), this, SLOT(slotEntryChanged()));
    connect(macAddress, SIGNAL(textEdited(QString)), this, SLOT(slotEnableWOLFeatures(QString)));
    slotEnableWOLFeatures(m_currentOptions->macAddress());
  }
  
  //
  // Wake-On-Lan: Send package before scan
  // 
  QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
  
  if (sendPackageBeforeScan)
  {
    sendPackageBeforeScan->setChecked(m_currentOptions->wolSendBeforeNetworkScan());
    connect(sendPackageBeforeScan, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  //
  // Wake-On-Lan: Send package before mount
  // 
  QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
  
  if (sendPackageBeforeMount)
  {
    sendPackageBeforeMount->setChecked(m_currentOptions->wolSendBeforeMount());
    connect(sendPackageBeforeMount, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
  }
  
  // 
  // Enable widgets
  //
  QTabWidget *tabWidget = findChild<QTabWidget *>("TabWidget");
  
  if (tabWidget)
  {
    // Enable the tab widget
    tabWidget->setEnabled(true);
    
    // Enable the Wake-On-Lan page
    int wolTabIndex = tabWidget->count() - 1;
    tabWidget->widget(wolTabIndex)->setEnabled(Smb4KSettings::enableWakeOnLAN());
  }
}


void Smb4KConfigPageCustomOptions::commitChanges()
{
  //
  // IP address
  // 
  KLineEdit *ipAddress = findChild<KLineEdit *>("IPAddress");
  
  if (ipAddress)
  {
    m_currentOptions->setIpAddress(ipAddress->text());
  }
  
  //
  // Remounting
  // 
  QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");
  
  if (remountAlways)
  {
    if (remountAlways->isChecked())
    {
      m_currentOptions->setRemount(Smb4KCustomOptions::RemountAlways);
    }
    else
    {
      m_currentOptions->setRemount(Smb4KCustomOptions::UndefinedRemount);
    }
  }
  
  //
  // User Id
  // 
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    m_currentOptions->setUseUser(useUserId->isChecked());
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    m_currentOptions->setUser(KUser(userId->itemData(userId->currentIndex()).toInt()));
  }
  
  //
  // Group Id
  //
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    m_currentOptions->setUseGroup(useGroupId->isChecked());
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    m_currentOptions->setGroup(KUserGroup(groupId->itemData(groupId->currentIndex()).toInt()));
  }
  
  //
  // File mode
  //
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    m_currentOptions->setUseFileMode(useFileMode->isChecked());
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    m_currentOptions->setFileMode(fileMode->text());
  }
  
  //
  // Directory mode
  //
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    m_currentOptions->setUseDirectoryMode(useDirectoryMode->isChecked());
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    m_currentOptions->setDirectoryMode(directoryMode->text());
  }
  
#if defined(Q_OS_LINUX)
  //
  // Write access
  // 
  QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");
  
  if (useWriteAccess)
  {
    m_currentOptions->setUseWriteAccess(useWriteAccess->isChecked());
  }
  
  KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");
  
  if (writeAccess)
  {
    QString readWriteText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label;
    QString readOnlyText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label;
    
    if (writeAccess->currentText() == readWriteText)
    {
      m_currentOptions->setWriteAccess(Smb4KMountSettings::EnumWriteAccess::ReadWrite);
    }
    else if (writeAccess->currentText() == readOnlyText)
    {
      m_currentOptions->setWriteAccess(Smb4KMountSettings::EnumWriteAccess::ReadOnly);
    }
  }
  
  //
  // File system port
  // 
  QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");
  
  if (useFilesystemPort)
  {
    m_currentOptions->setUseFileSystemPort(useFilesystemPort->isChecked());
  }
  
  QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");
  
  if (filesystemPort)
  {
    m_currentOptions->setFileSystemPort(filesystemPort->value());
  }
  
  //
  // CIFS Unix Extensions Support
  // 
  QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");
  
  if (cifsExtensionsSupport)
  {
    m_currentOptions->setCifsUnixExtensionsSupport(cifsExtensionsSupport->isChecked());
  }
  
  //
  // Security mode
  // 
  QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");
  
  if (useSecurityMode)
  {
    m_currentOptions->setUseSecurityMode(useSecurityMode->isChecked());
  }
  
  KComboBox *securityMode = findChild<KComboBox *>("SecurityMode");
  
  if (securityMode)
  {
    QString noneText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::None).label;
    QString krb5Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5).label;
    QString krb5iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Krb5i).label;
    QString ntlmText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlm).label;
    QString ntlmiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmi).label;
    QString ntlmv2Text = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2).label;
    QString ntlmv2iText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i).label;
    QString ntlmsspText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmssp).label;
    QString ntlmsspiText = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi).label;
    
    if (securityMode->currentText() == noneText)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::None);
    }
    else if (securityMode->currentText() == krb5Text)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Krb5);
    }
    else if (securityMode->currentText() == krb5iText)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Krb5i);
    }
    else if (securityMode->currentText() == ntlmText)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlm);
    }
    else if (securityMode->currentText() == ntlmiText)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmi);
    }
    else if (securityMode->currentText() == ntlmv2Text)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmv2);
    }
    else if (securityMode->currentText() == ntlmv2iText)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i);
    }
    else if (securityMode->currentText() == ntlmsspText)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmssp);
    }
    else if (securityMode->currentText() == ntlmsspiText)
    {
      m_currentOptions->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi);
    }
  }
#endif
  
  //
  // SMB port
  //
  QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");
  
  if (useSmbPort)
  {
    m_currentOptions->setUseSmbPort(useSmbPort->isChecked());
  }
  
  QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");
  
  if (smbPort)
  {
    m_currentOptions->setSmbPort(smbPort->value());
  }
  
  //
  // Kerberos
  // 
  QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");
  
  if (useKerberos)
  {
    m_currentOptions->setUseKerberos(useKerberos->isChecked());
  }
  
  //
  // MAC address
  // 
  KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");
  
  if (macAddress)
  {
    m_currentOptions->setMACAddress(macAddress->text());
  }
  
  //
  // Wake-On-Lan: Send package before scan
  // 
  QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
  
  if (sendPackageBeforeScan)
  {
    m_currentOptions->setWOLSendBeforeNetworkScan(sendPackageBeforeScan->isChecked());
  }
  
  //
  // Wake-On-Lan: Send package before mount
  // 
  QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
  
  if (sendPackageBeforeMount)
  {
    m_currentOptions->setWOLSendBeforeMount(sendPackageBeforeMount->isChecked());
  }
  
  //
  // In case the options are defined for a host, propagate them 
  // to the options of shares belonging to that host. Overwrite 
  // the settings
  // 
  if (m_currentOptions->type() == Host)
  {
    for (const OptionsPtr &o : m_optionsList)
    {
      if (o->type() == Share && o->hostName() == m_currentOptions->hostName() && o->workgroupName() == m_currentOptions->workgroupName())
      {
        o->setIpAddress(m_currentOptions->ipAddress());
        o->setUseUser(m_currentOptions->useUser());
        o->setUser(m_currentOptions->user());
        o->setUseGroup(m_currentOptions->useGroup());
        o->setGroup(m_currentOptions->group());
        o->setUseFileMode(m_currentOptions->useFileMode());
        o->setFileMode(m_currentOptions->fileMode());
        o->setUseDirectoryMode(m_currentOptions->useDirectoryMode());
        o->setDirectoryMode(m_currentOptions->directoryMode());
#if defined(Q_OS_LINUX)
        o->setCifsUnixExtensionsSupport(m_currentOptions->cifsUnixExtensionsSupport());
        o->setUseFileSystemPort(m_currentOptions->useFileSystemPort());
        o->setFileSystemPort(m_currentOptions->fileSystemPort());
        o->setUseSecurityMode(m_currentOptions->useSecurityMode());
        o->setSecurityMode(m_currentOptions->securityMode());
        o->setUseWriteAccess(m_currentOptions->useWriteAccess());
        o->setWriteAccess(m_currentOptions->writeAccess());
#endif
        o->setUseSmbPort(m_currentOptions->useSmbPort());
        o->setSmbPort(m_currentOptions->smbPort());
        o->setUseKerberos(m_currentOptions->useKerberos());
        o->setMACAddress(m_currentOptions->macAddress());
        o->setWOLSendBeforeNetworkScan(m_currentOptions->wolSendBeforeNetworkScan());
        o->setWOLSendBeforeMount(m_currentOptions->wolSendBeforeMount());
      }
    }
  }
  
  m_maybe_changed = true;
  emit customSettingsModified();
}


bool Smb4KConfigPageCustomOptions::eventFilter(QObject* obj, QEvent* e)
{
  QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");
  
  if (optionsListWidget)
  {
    if (obj == optionsListWidget->viewport())
    {
      if (e->type() == QEvent::MouseButtonPress)
      {
        QMouseEvent *mev = static_cast<QMouseEvent *>(e);
        QPoint pos = optionsListWidget->viewport()->mapFromGlobal(mev->globalPos());
        QListWidgetItem *item = optionsListWidget->itemAt(pos);
        
        if (!item)
        {
          clearEditors();
          optionsListWidget->clearSelection();
        }
      }
    }
  }
  
  return QObject::eventFilter(obj, e);
}



void Smb4KConfigPageCustomOptions::slotEditCustomItem(QListWidgetItem *item)
{
  setCurrentOptions(item->data(Qt::UserRole).toString());
  
  if (m_currentOptions)
  {
    populateEditors();
  }
  else
  {
    clearEditors();
  }
}


void Smb4KConfigPageCustomOptions::slotItemSelectionChanged()
{
  clearEditors();
}


void Smb4KConfigPageCustomOptions::slotCustomContextMenuRequested(const QPoint& pos)
{
  QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");
  
  if (optionsListWidget)
  {
    QListWidgetItem *item = optionsListWidget->itemAt(pos);
    
    for (QAction *a : optionsListWidget->actions())
    {
      if (a->objectName() == "edit_action")
      {
        a->setEnabled(item != 0);
      }
      else if (a->objectName() == "remove_action")
      {
        a->setEnabled(item != 0);
      }
      else if (a->objectName() == "clear_action")
      {
        a->setEnabled(optionsListWidget->count() != 0);
      }
    }
  
    KActionMenu *actionMenu = optionsListWidget->findChild<KActionMenu *>("ActionMenu");
    
    if (actionMenu)
    {
      actionMenu->menu()->popup(optionsListWidget->viewport()->mapToGlobal(pos));
    }
  }
}


void Smb4KConfigPageCustomOptions::slotEditActionTriggered(bool /*checked*/)
{
  QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");
  
  if (optionsListWidget)
  {
    slotEditCustomItem(optionsListWidget->currentItem());
  }
}


void Smb4KConfigPageCustomOptions::slotRemoveActionTriggered(bool /*checked*/)
{
  QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");
  
  if (optionsListWidget)
  {
    QListWidgetItem *item = optionsListWidget->currentItem();
    
    if (item)
    {
      setCurrentOptions(item->data(Qt::UserRole).toString());
      
      int index = m_optionsList.indexOf(m_currentOptions);
      
      if (index != -1)
      {
        m_optionsList.takeAt(index).clear();
      }
      
      KLineEdit *location = findChild<KLineEdit *>("Location");
      
      if (location)
      {
        if (item->text() == location->text())
        {
          clearEditors();
        }
      }
      
      delete item;
      m_currentOptions.clear();
      
      m_maybe_changed = true;
      emit customSettingsModified();
    }
  }
}


void Smb4KConfigPageCustomOptions::slotClearActionTriggered(bool /*checked*/)
{
  clearEditors();
  
  QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");
  
  if (optionsListWidget)
  {
    while (optionsListWidget->count() != 0)
    {
      delete optionsListWidget->item(0);
    }
  }

  while (!m_optionsList.isEmpty())
  {
    m_optionsList.takeFirst().clear();
  }
  
  m_currentOptions.clear();

  m_maybe_changed = true;
  emit customSettingsModified();
}


void Smb4KConfigPageCustomOptions::slotEntryChanged()
{
  commitChanges();
}


void Smb4KConfigPageCustomOptions::slotEnableWOLFeatures(const QString &mac_address)
{
  QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
  
  //
  // Wake-On_lan: send package before scan
  // 
  QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
  
  if (sendPackageBeforeScan)
  {
    sendPackageBeforeScan->setEnabled(exp.exactMatch(mac_address));
  }
  
  //
  // Wake-On-Lan: send package before mount
  // 
  QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
  
  if (sendPackageBeforeMount)
  {
    sendPackageBeforeMount->setEnabled(exp.exactMatch(mac_address));
  }
}


void Smb4KConfigPageCustomOptions::slotCifsUnixExtensionsSupport(bool on)
{
  //
  // User Id
  // 
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    useUserId->setEnabled(!on);
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    userId->setEnabled(!on);
  }
  
  //
  // Group Id
  //
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    useGroupId->setEnabled(!on);
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    groupId->setEnabled(!on);
  }
  
  //
  // File mode
  // 
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    useFileMode->setEnabled(!on);
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    fileMode->setEnabled(!on);
  }
  
  //
  // Directory mode
  // 
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    useDirectoryMode->setEnabled(!on);
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    directoryMode->setEnabled(!on);
  }
}


