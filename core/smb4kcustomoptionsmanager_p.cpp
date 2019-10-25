/***************************************************************************
    Private helper classes for Smb4KCustomOptionsManagerPrivate class
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2019 by Alexander Reinholdt
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
#include "smb4kcustomoptionsmanager_p.h"
#include "smb4ksettings.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QList>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KCoreAddons/KUser>
#include <KConfigGui/KWindowConfig>
#include <KIconThemes/KIconLoader>
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog(const OptionsPtr &options, QWidget *parent)
: QDialog(parent), m_options(options)
{
  //
  // Set the title
  // 
  setWindowTitle(i18n("Custom Options"));
  
  //
  // Set up the layout
  //
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  setLayout(layout);
  
  // Header
  QWidget *header = new QWidget(this);
  
  QHBoxLayout *headerLayout = new QHBoxLayout(header);
  headerLayout->setSpacing(5);
  headerLayout->setMargin(0);

  QLabel *pixmap = new QLabel(header);
  QPixmap preferencesPixmap = KDE::icon("preferences-system-network").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(preferencesPixmap);
  pixmap->setAlignment(Qt::AlignCenter);

  QLabel *description = 0;

  switch (m_options->type())
  {
    case Host:
    {
      description = new QLabel(i18n("<p>Define custom options for host <b>%1</b> and all the shares it provides.</p>", m_options->displayString()), header);
      break;
    }
    case Share:
    {
      description = new QLabel(i18n("<p>Define custom options for share <b>%1</b>.</p>", m_options->displayString()), header);
      break;
    }
    default:
    {
      description = new QLabel();
      break;
    }
  }

  description->setWordWrap(true);
  description->setAlignment(Qt::AlignVCenter);

  headerLayout->addWidget(pixmap, 0);
  headerLayout->addWidget(description, Qt::AlignVCenter);
  
  layout->addWidget(header, 0);
  
  //
  // Set up the operating system dependent stuff
  // 
  setupView();
  
  //
  // Finish the layout
  // 
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  
  QPushButton *restoreButton = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
  
  QPushButton *okButton = buttonBox->addButton(QDialogButtonBox::Ok);
  okButton->setShortcut(Qt::CTRL|Qt::Key_Return);
  okButton->setDefault(true);
  
  QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
  cancelButton->setShortcut(Qt::Key_Escape);
  
  layout->addWidget(buttonBox, 0);
  
  //
  // Connections
  // 
  connect(restoreButton, SIGNAL(clicked()), SLOT(slotSetDefaultValues()));
  connect(okButton, SIGNAL(clicked()), SLOT(slotOKClicked()));
  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));

  //
  // Set the dialog size
  // 
  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
  QSize dialogSize;
  
  if (group.exists())
  {
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    dialogSize = windowHandle()->size();
  }
  else
  {
    dialogSize = sizeHint();
  }
  
  resize(dialogSize); // workaround for QTBUG-40584
  
  //
  // Enable/disable buttons
  // 
  restoreButton->setEnabled(!checkDefaultValues());
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
  //
  // Tab widget with settings
  //
  QTabWidget *tabWidget = new QTabWidget(this);
  
  QVBoxLayout *dialogLayout = qobject_cast<QVBoxLayout *>(layout());
  dialogLayout->addWidget(tabWidget, 0);
  
  //
  // Custom options for mounting
  // 
  QWidget *mountingTab = new QWidget(tabWidget);
  
  QVBoxLayout *mountingTabLayout = new QVBoxLayout(mountingTab);
  mountingTabLayout->setSpacing(5);
  
  //
  // Common options
  //
  QGroupBox *commonBox = new QGroupBox(i18n("Common Options"), mountingTab);
  QGridLayout *commonBoxLayout = new QGridLayout(commonBox);
  commonBoxLayout->setSpacing(5);
  
  QCheckBox *remountAlways = new QCheckBox(i18n("Always remount this share"), commonBox);
  remountAlways->setObjectName("RemountAlways");
  remountAlways->setEnabled(m_options->type() == Share);
  connect(remountAlways, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
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
  
  connect(useWriteAccess, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(writeAccess, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  
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
  
  connect(useFilesystemPort, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(filesystemPort, SIGNAL(valueChanged(int)), SLOT(slotCheckValues()));

  commonBoxLayout->addWidget(useFilesystemPort, 2, 0, 0);
  commonBoxLayout->addWidget(filesystemPort, 2, 1, 0);
  
  mountingTabLayout->addWidget(commonBox, 0);
  
  //
  // CIFS Unix Extensions Support
  // 
  QGroupBox *extensionsSupportBox = new QGroupBox(i18n("CIFS Unix Extensions Support"), mountingTab);
  QGridLayout *extensionsSupportBoxLayout = new QGridLayout(extensionsSupportBox);
  extensionsSupportBoxLayout->setSpacing(5);
  
  QCheckBox *cifsExtensionsSupport = new QCheckBox(i18n("This server supports the CIFS Unix extensions"), extensionsSupportBox);
  cifsExtensionsSupport->setObjectName("CifsExtensionsSupport");
  
  connect(cifsExtensionsSupport, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(cifsExtensionsSupport, SIGNAL(toggled(bool)), SLOT(slotCifsExtensionsSupport(bool)));
  
  extensionsSupportBoxLayout->addWidget(cifsExtensionsSupport, 0, 0, 1, 4, 0);
  
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
  
  connect(useUserId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(userId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

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
  
  connect(useGroupId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(groupId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  
  extensionsSupportBoxLayout->addWidget(useGroupId, 2, 0, 0);
  extensionsSupportBoxLayout->addWidget(groupId, 2, 1, 0);
  
  // File mode
  QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), extensionsSupportBox);
  useFileMode->setObjectName("UseFileMode");
  
  KLineEdit *fileMode = new KLineEdit(extensionsSupportBox);
  fileMode->setObjectName("FileMode");
  fileMode->setClearButtonEnabled(true);
  fileMode->setAlignment(Qt::AlignRight);
  
  connect(useFileMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
  connect(fileMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));
  
  extensionsSupportBoxLayout->addWidget(useFileMode, 3, 0, 0);
  extensionsSupportBoxLayout->addWidget(fileMode, 3, 1, 0);
  
  // Directory mode
  QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), extensionsSupportBox);
  useDirectoryMode->setObjectName("UseDirectoryMode");
  
  KLineEdit *directoryMode = new KLineEdit(extensionsSupportBox);
  directoryMode->setObjectName("DirectoryMode");
  directoryMode->setClearButtonEnabled(true);
  directoryMode->setAlignment(Qt::AlignRight);
  
  connect(useDirectoryMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
  connect(directoryMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));
  
  extensionsSupportBoxLayout->addWidget(useDirectoryMode, 4, 0, 0);
  extensionsSupportBoxLayout->addWidget(directoryMode, 4, 1, 0);
  
  mountingTabLayout->addWidget(extensionsSupportBox, 0);
  
  //
  // Advanced options
  // 
  QGroupBox *advancedOptionsBox = new QGroupBox(i18n("Advanced Options"), mountingTab);
  QGridLayout *advancedOptionsBoxLayout = new QGridLayout(advancedOptionsBox);
  advancedOptionsBoxLayout->setSpacing(5);
  
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
  
  connect(useSecurityMode, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(securityMode, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

  advancedOptionsBoxLayout->addWidget(useSecurityMode, 0, 0, 0);
  advancedOptionsBoxLayout->addWidget(securityMode, 0, 1, 0);
  
  mountingTabLayout->addWidget(advancedOptionsBox, 0);
  mountingTabLayout->addStretch(100);
  
  tabWidget->addTab(mountingTab, i18n("Mounting"));
  
  //
  // Custom options for Samba
  //
  QWidget *sambaTab = new QWidget(tabWidget);
  QVBoxLayout *sambaTabLayout = new QVBoxLayout(sambaTab);
  sambaTabLayout->setSpacing(5);

  //
  // Common Options
  //
  QGroupBox *commonSambaOptionsBox = new QGroupBox(i18n("Common Options"), sambaTab);
  QGridLayout *commonSambaOptionsBoxLayout = new QGridLayout(commonSambaOptionsBox);
  commonSambaOptionsBoxLayout->setSpacing(5);
  
  // SMB port
  QCheckBox *useSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonSambaOptionsBox);
  useSmbPort->setObjectName("UseSmbPort");
  
  QSpinBox *smbPort = new QSpinBox(commonSambaOptionsBox);
  smbPort->setObjectName("SmbPort");
  smbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
  smbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());
//   smbPort->setSliderEnabled(true);
  
  connect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
  connect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotCheckValues()));
  
  commonSambaOptionsBoxLayout->addWidget(useSmbPort, 0, 0, 0);
  commonSambaOptionsBoxLayout->addWidget(smbPort, 0, 1, 0);
  
  sambaTabLayout->addWidget(commonSambaOptionsBox, 0);
  
  //
  // Authentication
  // 
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
  QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);
  authenticationBoxLayout->setSpacing(5);
  
  // Kerberos
  QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
  useKerberos->setObjectName("UseKerberos");

  connect(useKerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  authenticationBoxLayout->addWidget(useKerberos, 0);
  
  sambaTabLayout->addWidget(authenticationBox, 0);
  sambaTabLayout->addStretch(100);

  tabWidget->addTab(sambaTab, i18n("Samba"));

  //
  // Custom options for Wake-On-LAN
  //
  // NOTE: If you change the texts here, also alter them in the respective
  // config page.
  // 
  QWidget *wakeOnLanTab = new QWidget(tabWidget);
  QVBoxLayout *wakeOnLanTabLayout = new QVBoxLayout(wakeOnLanTab);
  wakeOnLanTabLayout->setSpacing(5);
  
  // 
  // MAC address
  // 
  QGroupBox *macAddressBox = new QGroupBox(i18n("MAC Address"), wakeOnLanTab);
  QGridLayout *macAddressBoxLayout = new QGridLayout(macAddressBox);
  macAddressBoxLayout->setSpacing(5);
  
  // MAC address
  QLabel *macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressBox);
  KLineEdit *macAddress = new KLineEdit(macAddressBox);
  macAddress->setObjectName("MACAddress");
  macAddress->setClearButtonEnabled(true);
  macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_"); // MAC address, see QLineEdit doc
  macAddressLabel->setBuddy(macAddress);
  
  connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotCheckValues()));
  connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotEnableWOLFeatures(QString)));
  
  macAddressBoxLayout->addWidget(macAddressLabel, 0, 0, 0);
  macAddressBoxLayout->addWidget(macAddress, 0, 1, 0);

  wakeOnLanTabLayout->addWidget(macAddressBox, 0);
  
  //
  // Wake-On-LAN Actions
  // 
  QGroupBox *wakeOnLANActionsBox = new QGroupBox(i18n("Actions"), wakeOnLanTab);
  QVBoxLayout *wakeOnLANActionsBoxLayout = new QVBoxLayout(wakeOnLANActionsBox);
  wakeOnLANActionsBoxLayout->setSpacing(5);
  
  // Send magic package before network scan
  QCheckBox *sendPackageBeforeScan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), wakeOnLANActionsBox);
  sendPackageBeforeScan->setObjectName("SendPackageBeforeScan");
  
  connect(sendPackageBeforeScan, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeScan, 0);
  
  // Send magic package before mount
  QCheckBox *sendPackageBeforeMount = new QCheckBox(i18n("Send magic package before mounting a share"), wakeOnLanTab);
  sendPackageBeforeMount->setObjectName("SendPackageBeforeMount");
  
  connect(sendPackageBeforeMount, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeMount, 0);
  
  wakeOnLanTabLayout->addWidget(wakeOnLANActionsBox, 0);
  wakeOnLanTabLayout->addStretch(100);

  tabWidget->addTab(wakeOnLanTab, i18n("Wake-On-LAN"));
  
  //
  // Load settings
  // 
  if (m_options->hasOptions())
  {
    if (m_options->type() == Share)
    {
      remountAlways->setChecked((m_options->remount() == Smb4KCustomOptions::RemountAlways));
    }
    
    // CIFS Unix extensions support
    cifsExtensionsSupport->setChecked(m_options->cifsUnixExtensionsSupport());
    
    // User information
    useUserId->setChecked(m_options->useUser());
    userId->setCurrentText(QString("%1 (%2)").arg(m_options->user().loginName()).arg(m_options->user().userId().nativeId()));
    
    // Group information
    useGroupId->setChecked(m_options->useGroup());
    groupId->setCurrentText(QString("%1 (%2)").arg(m_options->group().name()).arg(m_options->group().groupId().nativeId()));
    
    // File mode
    useFileMode->setChecked(m_options->useFileMode());
    fileMode->setText(m_options->fileMode());
    
    // Directory mode
    useDirectoryMode->setChecked(m_options->useDirectoryMode());
    directoryMode->setText(m_options->directoryMode());
    
    // Remote file system port
    useFilesystemPort->setChecked(m_options->useFileSystemPort());
    filesystemPort->setValue(m_options->fileSystemPort());
    
    // Write access
    useWriteAccess->setChecked(m_options->useWriteAccess());

    switch (m_options->writeAccess())
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
    
    // Security mode
    useSecurityMode->setChecked(m_options->useSecurityMode());

    switch (m_options->securityMode())
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
    
    // Remote SMB port
    useSmbPort->setChecked(m_options->useSmbPort());
    smbPort->setValue(m_options->smbPort());

    // Kerberos
    useKerberos->setChecked(m_options->useKerberos());
    
    // MAC address
    macAddress->setText(m_options->macAddress());
    
    // Send magic package before scan
    sendPackageBeforeScan->setChecked(m_options->wolSendBeforeNetworkScan());
    
    // Send magic package before mount
    sendPackageBeforeMount->setChecked(m_options->wolSendBeforeMount());
  }
  else
  {
    setDefaultValues();
  }
  
  //
  // Enable/disable features
  // 
  wakeOnLanTab->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
  slotEnableWOLFeatures(macAddress->text());
  slotCifsExtensionsSupport(cifsExtensionsSupport->isChecked());
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KCustomOptionsDialog::setupView()
{
  //
  // Tab widget with settings
  //
  QTabWidget *tabWidget = new QTabWidget(this);
  
  QVBoxLayout *dialogLayout = qobject_cast<QVBoxLayout *>(layout());
  dialogLayout->addWidget(tabWidget, 0);
  
  //
  // Custom options for mounting
  // 
  QWidget *mountingTab = new QWidget(tabWidget);
  
  QVBoxLayout *mountingTabLayout = new QVBoxLayout(mountingTab);
  mountingTabLayout->setSpacing(5);
  
  //
  // Common options
  //
  QGroupBox *commonBox = new QGroupBox(i18n("Common Options"), mountingTab);
  QGridLayout *commonBoxLayout = new QGridLayout(commonBox);
  commonBoxLayout->setSpacing(5);
  
  QCheckBox *remountAlways = new QCheckBox(i18n("Always remount this share"), commonBox);
  remountAlways->setObjectName("RemountAlways");
  remountAlways->setEnabled(m_options->type() == Share);
  connect(remountAlways, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
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
  
  connect(useUserId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(userId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

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
  
  connect(useGroupId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  connect(groupId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));
  
  commonBoxLayout->addWidget(useGroupId, 2, 0, 0);
  commonBoxLayout->addWidget(groupId, 2, 1, 0);
  
  // File mode
  QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), commonBox);
  useFileMode->setObjectName("UseFileMode");
  
  KLineEdit *fileMode = new KLineEdit(commonBox);
  fileMode->setObjectName("FileMode");
  fileMode->setClearButtonEnabled(true);
  fileMode->setAlignment(Qt::AlignRight);
  
  connect(useFileMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
  connect(fileMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));
  
  commonBoxLayout->addWidget(useFileMode, 3, 0, 0);
  commonBoxLayout->addWidget(fileMode, 3, 1, 0);
  
  // Directory mode
  QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), commonBox);
  useDirectoryMode->setObjectName("UseDirectoryMode");
  
  KLineEdit *directoryMode = new KLineEdit(commonBox);
  directoryMode->setObjectName("DirectoryMode");
  directoryMode->setClearButtonEnabled(true);
  directoryMode->setAlignment(Qt::AlignRight);
  
  connect(useDirectoryMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
  connect(directoryMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));
  
  commonBoxLayout->addWidget(useDirectoryMode, 4, 0, 0);
  commonBoxLayout->addWidget(directoryMode, 4, 1, 0);
  
  mountingTabLayout->addWidget(commonBox, 0);
  mountingTabLayout->addStretch(100);
  
  tabWidget->addTab(mountingTab, i18n("Mounting"));
  
  //
  // Custom options for Samba
  //
  QWidget *sambaTab = new QWidget(tabWidget);
  QVBoxLayout *sambaTabLayout = new QVBoxLayout(sambaTab);
  sambaTabLayout->setSpacing(5);

  //
  // Common Options
  //
  QGroupBox *commonSambaOptionsBox = new QGroupBox(i18n("Common Options"), sambaTab);
  QGridLayout *commonSambaOptionsBoxLayout = new QGridLayout(commonSambaOptionsBox);
  commonSambaOptionsBoxLayout->setSpacing(5);
  
  // SMB port
  QCheckBox *useSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonSambaOptionsBox);
  useSmbPort->setObjectName("UseSmbPort");
  
  QSpinBox *smbPort = new QSpinBox(commonSambaOptionsBox);
  smbPort->setObjectName("SmbPort");
  smbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
  smbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());
//   smbPort->setSliderEnabled(true);
  
  connect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
  connect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotCheckValues()));
  
  commonSambaOptionsBoxLayout->addWidget(useSmbPort, 0, 0, 0);
  commonSambaOptionsBoxLayout->addWidget(smbPort, 0, 1, 0);
  
  sambaTabLayout->addWidget(commonSambaOptionsBox, 0);
  
  //
  // Authentication
  // 
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
  QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);
  authenticationBoxLayout->setSpacing(5);
  
  // Kerberos
  QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
  useKerberos->setObjectName("UseKerberos");

  connect(useKerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  authenticationBoxLayout->addWidget(useKerberos, 0);
  
  sambaTabLayout->addWidget(authenticationBox, 0);
  sambaTabLayout->addStretch(100);

  tabWidget->addTab(sambaTab, i18n("Samba"));

  //
  // Custom options for Wake-On-LAN
  //
  // NOTE: If you change the texts here, also alter them in the respective
  // config page.
  // 
  QWidget *wakeOnLanTab = new QWidget(tabWidget);
  QVBoxLayout *wakeOnLanTabLayout = new QVBoxLayout(wakeOnLanTab);
  wakeOnLanTabLayout->setSpacing(5);
  
  // 
  // MAC address
  // 
  QGroupBox *macAddressBox = new QGroupBox(i18n("MAC Address"), wakeOnLanTab);
  QGridLayout *macAddressBoxLayout = new QGridLayout(macAddressBox);
  macAddressBoxLayout->setSpacing(5);
  
  // MAC address
  QLabel *macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressBox);
  KLineEdit *macAddress = new KLineEdit(macAddressBox);
  macAddress->setObjectName("MACAddress");
  macAddress->setClearButtonEnabled(true);
  macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_"); // MAC address, see QLineEdit doc
  macAddressLabel->setBuddy(macAddress);
  
  connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotCheckValues()));
  connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotEnableWOLFeatures(QString)));
  
  macAddressBoxLayout->addWidget(macAddressLabel, 0, 0, 0);
  macAddressBoxLayout->addWidget(macAddress, 0, 1, 0);

  wakeOnLanTabLayout->addWidget(macAddressBox, 0);
  
  //
  // Wake-On-LAN Actions
  // 
  QGroupBox *wakeOnLANActionsBox = new QGroupBox(i18n("Actions"), wakeOnLanTab);
  QVBoxLayout *wakeOnLANActionsBoxLayout = new QVBoxLayout(wakeOnLANActionsBox);
  wakeOnLANActionsBoxLayout->setSpacing(5);
  
  // Send magic package before network scan
  QCheckBox *sendPackageBeforeScan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), wakeOnLANActionsBox);
  sendPackageBeforeScan->setObjectName("SendPackageBeforeScan");
  
  connect(sendPackageBeforeScan, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeScan, 0);
  
  // Send magic package before mount
  QCheckBox *sendPackageBeforeMount = new QCheckBox(i18n("Send magic package before mounting a share"), wakeOnLanTab);
  sendPackageBeforeMount->setObjectName("SendPackageBeforeMount");
  
  connect(sendPackageBeforeMount, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeMount, 0);
  
  wakeOnLanTabLayout->addWidget(wakeOnLANActionsBox, 0);
  wakeOnLanTabLayout->addStretch(100);

  tabWidget->addTab(wakeOnLanTab, i18n("Wake-On-LAN"));
  
  //
  // Load settings
  // 
  if (m_options->hasOptions())
  {
    if (m_options->type() == Share)
    {
      remountAlways->setChecked((m_options->remount() == Smb4KCustomOptions::RemountAlways));
    }
    
    // User information
    useUserId->setChecked(m_options->useUser());
    userId->setCurrentText(QString("%1 (%2)").arg(m_options->user().loginName()).arg(m_options->user().userId().nativeId()));
    
    // Group information
    useGroupId->setChecked(m_options->useGroup());
    groupId->setCurrentText(QString("%1 (%2)").arg(m_options->group().name()).arg(m_options->group().groupId().nativeId()));
    
    // File mode
    useFileMode->setChecked(m_options->useFileMode());
    fileMode->setText(m_options->fileMode());
    
    // Directory mode
    useDirectoryMode->setChecked(m_options->useDirectoryMode());
    directoryMode->setText(m_options->directoryMode());
    
    // Remote SMB port
    useSmbPort->setChecked(m_options->useSmbPort());
    smbPort->setValue(m_options->smbPort());

    // Kerberos
    useKerberos->setChecked(m_options->useKerberos());
    
    // MAC address
    macAddress->setText(m_options->macAddress());
    
    // Send magic package before scan
    sendPackageBeforeScan->setChecked(m_options->wolSendBeforeNetworkScan());
    
    // Send magic package before mount
    sendPackageBeforeMount->setChecked(m_options->wolSendBeforeMount());
  }
  else
  {
    setDefaultValues();
  }
  
  //
  // Enable/disable features
  // 
  wakeOnLanTab->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
  slotEnableWOLFeatures(macAddress->text());
}
#else
//
// Generic (without mount options)
//
void Smb4KCustomOptionsDialog::setupView()
{
  //
  // Tab widget with settings
  //
  QTabWidget *tabWidget = new QTabWidget(this);
  
  QVBoxLayout *dialogLayout = qobject_cast<QVBoxLayout *>(layout());
  dialogLayout->addWidget(tabWidget, 0);
  
  //
  // Custom options for Samba
  //
  QWidget *sambaTab = new QWidget(tabWidget);
  QVBoxLayout *sambaTabLayout = new QVBoxLayout(sambaTab);
  sambaTabLayout->setSpacing(5);

  //
  // Common Options
  //
  QGroupBox *commonSambaOptionsBox = new QGroupBox(i18n("Common Options"), sambaTab);
  QGridLayout *commonSambaOptionsBoxLayout = new QGridLayout(commonSambaOptionsBox);
  commonSambaOptionsBoxLayout->setSpacing(5);
  
  // SMB port
  QCheckBox *useSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonSambaOptionsBox);
  useSmbPort->setObjectName("UseSmbPort");
  
  QSpinBox *smbPort = new QSpinBox(commonSambaOptionsBox);
  smbPort->setObjectName("SmbPort");
  smbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
  smbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());
//   smbPort->setSliderEnabled(true);
  
  connect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
  connect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotCheckValues()));
  
  commonSambaOptionsBoxLayout->addWidget(useSmbPort, 0, 0, 0);
  commonSambaOptionsBoxLayout->addWidget(smbPort, 0, 1, 0);
  
  sambaTabLayout->addWidget(commonSambaOptionsBox, 0);
  
  //
  // Authentication
  // 
  QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
  QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);
  authenticationBoxLayout->setSpacing(5);
  
  // Kerberos
  QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
  useKerberos->setObjectName("UseKerberos");

  connect(useKerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  authenticationBoxLayout->addWidget(useKerberos, 0);
  
  sambaTabLayout->addWidget(authenticationBox, 0);
  sambaTabLayout->addStretch(100);

  tabWidget->addTab(sambaTab, i18n("Samba"));

  //
  // Custom options for Wake-On-LAN
  //
  // NOTE: If you change the texts here, also alter them in the respective
  // config page.
  // 
  QWidget *wakeOnLanTab = new QWidget(tabWidget);
  QVBoxLayout *wakeOnLanTabLayout = new QVBoxLayout(wakeOnLanTab);
  wakeOnLanTabLayout->setSpacing(5);
  
  // 
  // MAC address
  // 
  QGroupBox *macAddressBox = new QGroupBox(i18n("MAC Address"), wakeOnLanTab);
  QGridLayout *macAddressBoxLayout = new QGridLayout(macAddressBox);
  macAddressBoxLayout->setSpacing(5);
  
  // MAC address
  QLabel *macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressBox);
  KLineEdit *macAddress = new KLineEdit(macAddressBox);
  macAddress->setObjectName("MACAddress");
  macAddress->setClearButtonEnabled(true);
  macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_"); // MAC address, see QLineEdit doc
  macAddressLabel->setBuddy(macAddress);
  
  connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotCheckValues()));
  connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotEnableWOLFeatures(QString)));
  
  macAddressBoxLayout->addWidget(macAddressLabel, 0, 0, 0);
  macAddressBoxLayout->addWidget(macAddress, 0, 1, 0);

  wakeOnLanTabLayout->addWidget(macAddressBox, 0);
  
  //
  // Wake-On-LAN Actions
  // 
  QGroupBox *wakeOnLANActionsBox = new QGroupBox(i18n("Actions"), wakeOnLanTab);
  QVBoxLayout *wakeOnLANActionsBoxLayout = new QVBoxLayout(wakeOnLANActionsBox);
  wakeOnLANActionsBoxLayout->setSpacing(5);
  
  // Send magic package before network scan
  QCheckBox *sendPackageBeforeScan = new QCheckBox(i18n("Send magic package before scanning the network neighborhood"), wakeOnLANActionsBox);
  sendPackageBeforeScan->setObjectName("SendPackageBeforeScan");
  
  connect(sendPackageBeforeScan, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeScan, 0);
  
  // Send magic package before mount
  QCheckBox *sendPackageBeforeMount = new QCheckBox(i18n("Send magic package before mounting a share"), wakeOnLanTab);
  sendPackageBeforeMount->setObjectName("SendPackageBeforeMount");
  
  connect(sendPackageBeforeMount, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
  
  wakeOnLANActionsBoxLayout->addWidget(sendPackageBeforeMount, 0);
  
  wakeOnLanTabLayout->addWidget(wakeOnLANActionsBox, 0);
  wakeOnLanTabLayout->addStretch(100);

  tabWidget->addTab(wakeOnLanTab, i18n("Wake-On-LAN"));
  
  //
  // Load settings
  // 
  if (m_options->hasOptions())
  {
    // Remote SMB port
    useSmbPort->setChecked(m_options->useSmbPort());
    smbPort->setValue(m_options->smbPort());

    // Kerberos
    useKerberos->setChecked(m_options->useKerberos());
    
    // MAC address
    macAddress->setText(m_options->macAddress());
    
    // Send magic package before scan
    sendPackageBeforeScan->setChecked(m_options->wolSendBeforeNetworkScan());
    
    // Send magic package before mount
    sendPackageBeforeMount->setChecked(m_options->wolSendBeforeMount());
  }
  else
  {
    setDefaultValues();
  }
  
  //
  // Enable/disable features
  // 
  wakeOnLanTab->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
  slotEnableWOLFeatures(macAddress->text());
  
  
}
#endif


bool Smb4KCustomOptionsDialog::checkDefaultValues()
{
  // 
  // Always remount the share
  // 
  if (m_options->type() == Share)
  {
    QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");
    
    if (remountAlways)
    {
      if (remountAlways->isChecked())
      {
        return false;
      }
    }
  }
  
  // 
  // User Id
  // 
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    if (useUserId->isChecked() != Smb4KMountSettings::useUserId())
    {
      return false;
    }
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    K_UID uid = (K_UID)userId->itemData(userId->currentIndex()).toInt();

    if (uid != (K_UID)Smb4KMountSettings::userId().toInt())
    {
      return false;
    }
  }
  
  // 
  // Group Id
  // 
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    if (useGroupId->isChecked() != Smb4KMountSettings::useGroupId())
    {
      return false;
    }
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    K_GID gid = (K_GID)groupId->itemData(groupId->currentIndex()).toInt();

    if (gid != (K_GID)Smb4KMountSettings::groupId().toInt())
    {
      return false;
    }
  }
  
  //
  // File mode
  //
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    if (useFileMode->isChecked() != Smb4KMountSettings::useFileMode())
    {
      return false;
    }
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    if (fileMode->text() != Smb4KMountSettings::fileMode())
    {
      return false;
    }
  }
  
  //
  // Directory mode
  // 
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    if (useDirectoryMode->isChecked() != Smb4KMountSettings::useDirectoryMode())
    {
      return false;
    }
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    if (directoryMode->text() != Smb4KMountSettings::directoryMode())
    {
      return false;
    }
  }

#if defined(Q_OS_LINUX)
  // 
  // CIFS Unix extensions support
  // 
  QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");
  
  if (cifsExtensionsSupport)
  {
    if (cifsExtensionsSupport->isChecked() != Smb4KMountSettings::cifsUnixExtensionsSupport())
    {
      return false;
    }
  }
  
  // 
  // Filesystem port
  // 
  QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");
  
  if (useFilesystemPort)
  {
    if (useFilesystemPort->isChecked() != Smb4KMountSettings::useRemoteFileSystemPort())
    {
      return false;
    }
  }
  
  QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");
  
  if (filesystemPort)
  {
    if (filesystemPort->value() != Smb4KMountSettings::remoteFileSystemPort())
    {
      return false;
    }
  }
  
  // 
  // Write access
  // 
  QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");
  
  if (useWriteAccess)
  {
    if (useWriteAccess->isChecked() != Smb4KMountSettings::useWriteAccess())
    {
      return false;
    }
  }
  
  KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");
  
  if (writeAccess)
  {  
    if (writeAccess->currentText() != Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::self()->writeAccess()).label)
    {
      return false;
    }
  }
  
  // 
  // Security mode
  // 
  QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");
  
  if (useSecurityMode)
  {
    if (useSecurityMode->isChecked() != Smb4KMountSettings::useSecurityMode())
    {
      return false;
    }
  }
  
  KComboBox *securityMode = findChild<KComboBox *>("SecurityMode");
  
  if (securityMode)
  {  
    if (securityMode->currentText() != Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::self()->securityMode()).label)
    {
      return false;
    }
  }
#endif
  
  // 
  // SMB port
  // 
  QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");
  
  if (useSmbPort)
  {
    if (useSmbPort->isChecked() != Smb4KSettings::useRemoteSmbPort())
    {
      return false;
    }
  }
  
  QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");
  
  if (smbPort)
  {
    if (smbPort->value() != Smb4KSettings::remoteSmbPort())
    {
      return false;
    }
  }
  
  // 
  // Kerberos
  // 
  QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");
  
  if (useKerberos)
  {
    if (useKerberos->isChecked() != Smb4KSettings::useKerberos())
    {
      return false;
    }
  }
  
  // 
  // MAC address & Wake-On-LAN features
  // 
  if (m_options->type() == Host && Smb4KSettings::enableWakeOnLAN())
  {
    KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");
    
    if (macAddress)
    {
      QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
        
      if (exp.exactMatch(macAddress->text()))
      {
        return false;
      }
    }
    
    QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
    
    if (sendPackageBeforeScan)
    {
      if (sendPackageBeforeScan->isChecked())
      {
        return false;
      }
    }
    
    QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
    
    if (sendPackageBeforeMount)
    {
      if (sendPackageBeforeMount->isChecked())
      {
        return false;
      }
    }
  }

  return true;
}


void Smb4KCustomOptionsDialog::setDefaultValues()
{
  // 
  // Always remount the share
  // 
  if (m_options->type() == Share)
  {
    QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");
    
    if (remountAlways)
    {
      remountAlways->setChecked(false);
    }
  }
  
  // 
  // User Id
  // 
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    useUserId->setChecked(Smb4KMountSettings::useUserId());
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    KUser user((K_UID)Smb4KMountSettings::userId().toInt());
    userId->setCurrentText(QString("%1 (%2)").arg(user.loginName()).arg(user.userId().nativeId()));
  }
  
  // 
  // Group Id
  // 
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    useGroupId->setChecked(Smb4KMountSettings::useGroupId());
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    KUserGroup group((K_GID)Smb4KMountSettings::groupId().toInt());
    groupId->setCurrentText(QString("%1 (%2)").arg(group.name()).arg(group.groupId().nativeId()));;
  }
  
  //
  // File mask
  // 
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    useFileMode->setChecked(Smb4KMountSettings::useFileMode());
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    fileMode->setText(Smb4KMountSettings::fileMode());
  }
  
  //
  // Directory mode
  // 
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    useDirectoryMode->setChecked(Smb4KMountSettings::useDirectoryMode());
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    directoryMode->setText(Smb4KMountSettings::directoryMode());
  }
  
#if defined(Q_OS_LINUX)
  // 
  // CIFS Unix extensions support
  // 
  QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");
  
  if (cifsExtensionsSupport)
  {
    cifsExtensionsSupport->setChecked(Smb4KMountSettings::cifsUnixExtensionsSupport());
  }
  
  // 
  // Filesystem port
  // 
  QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");
  
  if (useFilesystemPort)
  {
    useFilesystemPort->setChecked(Smb4KMountSettings::useRemoteFileSystemPort());
  }
  
  QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");
  
  if (filesystemPort)
  {
    filesystemPort->setValue(Smb4KMountSettings::remoteFileSystemPort());
  }
  
  // 
  // Write access
  // 
  QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");
  
  if (useWriteAccess)
  {
    useWriteAccess->setChecked(Smb4KMountSettings::useWriteAccess());
  }
  
  KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");
  
  if (writeAccess)
  {
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
  // Security mode
  // 
  QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");
  
  if (useSecurityMode)
  {
    useSecurityMode->setChecked(Smb4KMountSettings::useSecurityMode());
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
    useSmbPort->setChecked(Smb4KSettings::useRemoteSmbPort());
  }
  
  QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");
  
  if (smbPort)
  {
    smbPort->setValue(Smb4KSettings::remoteSmbPort());
  }
  
  // 
  // Kerberos
  // 
  QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");
  
  if (useKerberos)
  {
    useKerberos->setChecked(Smb4KSettings::useKerberos());
  }
  
  // 
  // MAC address & Wake-On-LAN features
  // 
  if (m_options->type() == Host)
  {
    KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");
    
    if (macAddress)
    {
      macAddress->clear();
      macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_");
    }
    
    QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
    
    if (sendPackageBeforeScan)
    {
      sendPackageBeforeScan->setChecked(false);
    }
    
    QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
    
    if (sendPackageBeforeMount)
    {
      sendPackageBeforeMount->setChecked(false);
    }
  }
}


void Smb4KCustomOptionsDialog::saveValues()
{
  //
  // Always remount the share
  // 
  if (m_options->type() == Share)
  {
    QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");
    
    if (remountAlways)
    {
      if (remountAlways->isChecked())
      {
        m_options->setRemount(Smb4KCustomOptions::RemountAlways);
      }
      else
      {
        m_options->setRemount(Smb4KCustomOptions::UndefinedRemount);
      }
    }
  }
  
  // 
  // User Id
  // 
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    m_options->setUseUser(useUserId->isChecked());
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    m_options->setUser(KUser(userId->itemData(userId->currentIndex()).toInt()));
  }
  
  // 
  // Group Id
  // 
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    m_options->setUseGroup(useGroupId->isChecked());
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    m_options->setGroup(KUserGroup(groupId->itemData(groupId->currentIndex()).toInt()));
  }
  
  //
  // File mode
  // 
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    m_options->setUseFileMode(useFileMode->isChecked());
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    m_options->setFileMode(fileMode->text());
  }
  
  //
  // Directory mode
  // 
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    m_options->setUseDirectoryMode(useDirectoryMode->isChecked());
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    m_options->setDirectoryMode(directoryMode->text());
  }

#if defined(Q_OS_LINUX)
  // 
  // CIFS Unix extensions support
  // 
  QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");
  
  if (cifsExtensionsSupport)
  {
    m_options->setCifsUnixExtensionsSupport(cifsExtensionsSupport->isChecked());
  }
  
  // 
  // Filesystem port
  // 
  QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");
  
  if (useFilesystemPort)
  {
    m_options->setUseFileSystemPort(useFilesystemPort->isChecked());
  }
  
  QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");
  
  if (filesystemPort)
  {
    m_options->setFileSystemPort(filesystemPort->value());
  }
  
  // 
  // Write access
  // 
  QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");
  
  if (useWriteAccess)
  {
    m_options->setUseWriteAccess(useWriteAccess->isChecked());
  }
  
  KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");
  
  if (writeAccess)
  {
    QString readWriteText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadWrite).label;
    QString readOnlyText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::EnumWriteAccess::ReadOnly).label;
    
    if (writeAccess->currentText() == readWriteText)
    {
      m_options->setWriteAccess(Smb4KMountSettings::EnumWriteAccess::ReadWrite);
    }
    else if (writeAccess->currentText() == readOnlyText)
    {
      m_options->setWriteAccess(Smb4KMountSettings::EnumWriteAccess::ReadOnly);
    }
  }
  
  // 
  // Security mode
  // 
  QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");
  
  if (useSecurityMode)
  {
    m_options->setUseSecurityMode(useSecurityMode->isChecked());
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
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::None);
    }
    else if (securityMode->currentText() == krb5Text)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Krb5);
    }
    else if (securityMode->currentText() == krb5iText)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Krb5i);
    }
    else if (securityMode->currentText() == ntlmText)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlm);
    }
    else if (securityMode->currentText() == ntlmiText)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmi);
    }
    else if (securityMode->currentText() == ntlmv2Text)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmv2);
    }
    else if (securityMode->currentText() == ntlmv2iText)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i);
    }
    else if (securityMode->currentText() == ntlmsspText)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmssp);
    }
    else if (securityMode->currentText() == ntlmsspiText)
    {
      m_options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi);
    }
  }
#endif
  
  // 
  // SMB port
  // 
  QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");
  
  if (useSmbPort)
  {
    m_options->setUseSmbPort(useSmbPort->isChecked());
  }
  
  QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");
  
  if (smbPort)
  {
    m_options->setSmbPort(smbPort->value());
  }
  
  // Kerberos
  QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");
  
  if (useKerberos)
  {
    m_options->setUseKerberos(useKerberos->isChecked());
  }
  
  // MAC address & Wake-On-LAN features
  if (m_options->type() == Host)
  {
    KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");
    
    if (macAddress)
    {
      m_options->setMACAddress(macAddress->text());
    }
    
    QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
    
    if (sendPackageBeforeScan)
    {
      m_options->setWOLSendBeforeNetworkScan(sendPackageBeforeScan->isChecked());
    }
    
    QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
    
    if (sendPackageBeforeMount)
    {
      m_options->setWOLSendBeforeMount(sendPackageBeforeMount->isChecked());
    }
  }

  KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
}


void Smb4KCustomOptionsDialog::slotSetDefaultValues()
{
  setDefaultValues();
}


void Smb4KCustomOptionsDialog::slotCheckValues()
{
  QDialogButtonBox *buttonBox = findChild<QDialogButtonBox *>();
  
  if (buttonBox)
  {
    for (QAbstractButton *b : buttonBox->buttons())
    {
      if (buttonBox->buttonRole(b) == QDialogButtonBox::ResetRole)
      {
        b->setEnabled(!checkDefaultValues());
        break;
      }
    }
  }
}


void Smb4KCustomOptionsDialog::slotOKClicked()
{
  saveValues();
  accept();
}


void Smb4KCustomOptionsDialog::slotEnableWOLFeatures(const QString &mac)
{
  QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
  
  QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");
  
  if (sendPackageBeforeScan)
  {
    sendPackageBeforeScan->setEnabled(m_options->type() == Host && exp.exactMatch(mac));
  }
  
  QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");
  
  if (sendPackageBeforeMount)
  {
    sendPackageBeforeMount->setEnabled(m_options->type() == Host && exp.exactMatch(mac));
  }
}


void Smb4KCustomOptionsDialog::slotCifsExtensionsSupport(bool support)
{
#if defined(Q_OS_LINUX)
  // 
  // User id
  // 
  QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");
  
  if (useUserId)
  {
    useUserId->setEnabled(!support);
  }
  
  KComboBox *userId = findChild<KComboBox *>("UserId");
  
  if (userId)
  {
    userId->setEnabled(!support);
  }
  
  // 
  // Group id
  // 
  QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");
  
  if (useGroupId)
  {
    useGroupId->setEnabled(!support);
  }
  
  KComboBox *groupId = findChild<KComboBox *>("GroupId");
  
  if (groupId)
  {
    groupId->setEnabled(!support);
  }
  
  //
  // File mode
  // 
  QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");
  
  if (useFileMode)
  {
    useFileMode->setEnabled(!support);
  }
  
  KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");
  
  if (fileMode)
  {
    fileMode->setEnabled(!support);
  }
  
  //
  // Directory mode
  //
  QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");
  
  if (useDirectoryMode)
  {
    useDirectoryMode->setEnabled(!support);
  }
  
  KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");
  
  if (directoryMode)
  {
    directoryMode->setEnabled(!support);
  }
#endif
}


