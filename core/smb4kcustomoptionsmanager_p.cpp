/*
    Private helper classes for Smb4KCustomOptionsManagerPrivate class
    -------------------
    begin                : Fr 29 Apr 2011
    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kcustomoptionsmanager_p.h"
#include "smb4ksettings.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KCompletion/KComboBox>
#include <KCompletion/KLineEdit>
#include <KConfigGui/KWindowConfig>
#include <KCoreAddons/KUser>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog(const OptionsPtr &options, QWidget *parent)
    : QDialog(parent)
    , m_options(options)
{
    //
    // Set the title
    //
    setWindowTitle(i18n("Custom Options"));

    //
    // Set up the layout
    //
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    // Header
    QWidget *header = new QWidget(this);

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *pixmap = new QLabel(header);
    QPixmap preferencesPixmap = KDE::icon("preferences-system-network").pixmap(KIconLoader::SizeHuge);
    pixmap->setPixmap(preferencesPixmap);
    pixmap->setAlignment(Qt::AlignCenter);

    QLabel *description = 0;

    switch (m_options->type()) {
    case Host: {
        description = new QLabel(i18n("<p>Define custom options for host <b>%1</b> and all the shares it provides.</p>", m_options->displayString()), header);
        break;
    }
    case Share: {
        description = new QLabel(i18n("<p>Define custom options for share <b>%1</b>.</p>", m_options->displayString()), header);
        break;
    }
    default: {
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
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
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

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
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
    // Tab "Common Mount Settings"
    //
    QWidget *commonMountSettingsTab = new QWidget(tabWidget);
    QVBoxLayout *commonMountSettingsTabLayout = new QVBoxLayout(commonMountSettingsTab);

    //
    // Common options
    //
    QGroupBox *commonBox = new QGroupBox(i18n("Common Options"), commonMountSettingsTab);
    QGridLayout *commonBoxLayout = new QGridLayout(commonBox);

    QCheckBox *remountAlways = new QCheckBox(i18n("Always remount this share"), commonBox);
    remountAlways->setObjectName("RemountAlways");
    remountAlways->setEnabled(m_options->type() == Share);
    connect(remountAlways, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(remountAlways, 0, 0, 1, 2);

    // Write access
    QCheckBox *useWriteAccess = new QCheckBox(Smb4KMountSettings::self()->useWriteAccessItem()->label(), commonBox);
    useWriteAccess->setObjectName("UseWriteAccess");

    KComboBox *writeAccess = new KComboBox(commonBox);
    writeAccess->setObjectName("WriteAccess");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> writeAccessChoices = Smb4KMountSettings::self()->writeAccessItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &wa : qAsConst(writeAccessChoices)) {
        writeAccess->addItem(wa.label);
    }

    connect(useWriteAccess, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(writeAccess, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(useWriteAccess, 1, 0);
    commonBoxLayout->addWidget(writeAccess, 1, 1);

    // Remote file system port
    QCheckBox *useFilesystemPort = new QCheckBox(Smb4KMountSettings::self()->useRemoteFileSystemPortItem()->label(), commonBox);
    useFilesystemPort->setObjectName("UseFilesystemPort");

    QSpinBox *filesystemPort = new QSpinBox(commonBox);
    filesystemPort->setObjectName("FileSystemPort");
    filesystemPort->setMinimum(Smb4KMountSettings::self()->remoteFileSystemPortItem()->minValue().toInt());
    filesystemPort->setMaximum(Smb4KMountSettings::self()->remoteFileSystemPortItem()->maxValue().toInt());

    connect(useFilesystemPort, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(filesystemPort, SIGNAL(valueChanged(int)), SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(useFilesystemPort, 2, 0);
    commonBoxLayout->addWidget(filesystemPort, 2, 1);

    commonMountSettingsTabLayout->addWidget(commonBox, 0);

    //
    // CIFS Unix Extensions Support
    //
    QGroupBox *extensionsSupportBox = new QGroupBox(i18n("CIFS Unix Extensions Support"), commonMountSettingsTab);
    QGridLayout *extensionsSupportBoxLayout = new QGridLayout(extensionsSupportBox);

    QCheckBox *cifsExtensionsSupport = new QCheckBox(i18n("This server supports the CIFS Unix extensions"), extensionsSupportBox);
    cifsExtensionsSupport->setObjectName("CifsExtensionsSupport");

    connect(cifsExtensionsSupport, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(cifsExtensionsSupport, SIGNAL(toggled(bool)), SLOT(slotCifsExtensionsSupport(bool)));

    extensionsSupportBoxLayout->addWidget(cifsExtensionsSupport, 0, 0, 1, 4);

    // User Id
    QCheckBox *useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), extensionsSupportBox);
    useUserId->setObjectName("UseUserId");

    KComboBox *userId = new KComboBox(extensionsSupportBox);
    userId->setObjectName("UserId");

    QList<KUser> allUsers = KUser::allUsers();

    for (const KUser &u : qAsConst(allUsers)) {
        userId->addItem(QString("%1 (%2)").arg(u.loginName(), u.userId().toString()));
    }

    connect(useUserId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(userId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

    extensionsSupportBoxLayout->addWidget(useUserId, 1, 0);
    extensionsSupportBoxLayout->addWidget(userId, 1, 1);

    // Group Id
    QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), extensionsSupportBox);
    useGroupId->setObjectName("UseGroupId");

    KComboBox *groupId = new KComboBox(extensionsSupportBox);
    groupId->setObjectName("GroupId");

    QList<KUserGroup> allGroups = KUserGroup::allGroups();

    for (const KUserGroup &g : qAsConst(allGroups)) {
        groupId->addItem(QString("%1 (%2)").arg(g.name(), g.groupId().toString()));
    }

    connect(useGroupId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(groupId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

    extensionsSupportBoxLayout->addWidget(useGroupId, 2, 0);
    extensionsSupportBoxLayout->addWidget(groupId, 2, 1);

    // File mode
    QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), extensionsSupportBox);
    useFileMode->setObjectName("UseFileMode");

    KLineEdit *fileMode = new KLineEdit(extensionsSupportBox);
    fileMode->setObjectName("FileMode");
    fileMode->setClearButtonEnabled(true);
    fileMode->setAlignment(Qt::AlignRight);

    connect(useFileMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(fileMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));

    extensionsSupportBoxLayout->addWidget(useFileMode, 3, 0);
    extensionsSupportBoxLayout->addWidget(fileMode, 3, 1);

    // Directory mode
    QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), extensionsSupportBox);
    useDirectoryMode->setObjectName("UseDirectoryMode");

    KLineEdit *directoryMode = new KLineEdit(extensionsSupportBox);
    directoryMode->setObjectName("DirectoryMode");
    directoryMode->setClearButtonEnabled(true);
    directoryMode->setAlignment(Qt::AlignRight);

    connect(useDirectoryMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(directoryMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));

    extensionsSupportBoxLayout->addWidget(useDirectoryMode, 4, 0);
    extensionsSupportBoxLayout->addWidget(directoryMode, 4, 1);

    commonMountSettingsTabLayout->addWidget(extensionsSupportBox, 0);
    commonMountSettingsTabLayout->addStretch(100);

    tabWidget->addTab(commonMountSettingsTab, i18n("Common Mount Settings"));

    //
    // Tab "Advanced Mount Settings"
    //
    QWidget *advancedMountSettingsTab = new QWidget(tabWidget);
    QVBoxLayout *advancedMountSettingsTabLayout = new QVBoxLayout(advancedMountSettingsTab);

    //
    // Advanced options
    //
    QGroupBox *advancedOptionsBox = new QGroupBox(i18n("Advanced Options"), advancedMountSettingsTab);
    QGridLayout *advancedOptionsBoxLayout = new QGridLayout(advancedOptionsBox);

    // SMB protocol version
    QCheckBox *useMountProtocol = new QCheckBox(Smb4KMountSettings::self()->useSmbProtocolVersionItem()->label(), advancedOptionsBox);
    useMountProtocol->setObjectName("UseMountProtocolVersion");

    KComboBox *mountProtocol = new KComboBox(advancedOptionsBox);
    mountProtocol->setObjectName("MountProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> smbProtocolChoices = Smb4KMountSettings::self()->smbProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(smbProtocolChoices)) {
        mountProtocol->addItem(c.label);
    }

    connect(useMountProtocol, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(mountProtocol, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

    advancedOptionsBoxLayout->addWidget(useMountProtocol, 0, 0);
    advancedOptionsBoxLayout->addWidget(mountProtocol, 0, 1);

    // Security mode
    QCheckBox *useSecurityMode = new QCheckBox(Smb4KMountSettings::self()->useSecurityModeItem()->label(), advancedOptionsBox);
    useSecurityMode->setObjectName("UseSecurityMode");

    KComboBox *securityMode = new KComboBox(advancedOptionsBox);
    securityMode->setObjectName("SecurityMode");

    QList<KConfigSkeleton::ItemEnum::Choice> securityModeChoices = Smb4KMountSettings::self()->securityModeItem()->choices();

    for (const KConfigSkeleton::ItemEnum::Choice &c : qAsConst(securityModeChoices)) {
        securityMode->addItem(c.label);
    }

    connect(useSecurityMode, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(securityMode, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

    advancedOptionsBoxLayout->addWidget(useSecurityMode, 1, 0);
    advancedOptionsBoxLayout->addWidget(securityMode, 1, 1);

    advancedMountSettingsTabLayout->addWidget(advancedOptionsBox, 0);
    advancedMountSettingsTabLayout->addStretch(100);

    tabWidget->addTab(advancedMountSettingsTab, i18n("Advanced Mount Settings"));

    //
    // Custom options for Browsing
    //
    QWidget *sambaTab = new QWidget(tabWidget);
    QVBoxLayout *sambaTabLayout = new QVBoxLayout(sambaTab);

    //
    // Common Options
    //
    QGroupBox *commonSambaOptionsBox = new QGroupBox(i18n("Common Options"), sambaTab);
    QGridLayout *commonSambaOptionsBoxLayout = new QGridLayout(commonSambaOptionsBox);

    // Minimal and maximal client protocol versions
    QCheckBox *useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), commonMountSettingsTab);
    useClientProtocolVersions->setObjectName("UseClientProtocolVersions");

    QLabel *minimalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), commonMountSettingsTab);
    minimalClientProtocolVersionLabel->setIndent(25);
    minimalClientProtocolVersionLabel->setObjectName("MinimalClientProtocolVersionLabel");

    KComboBox *minimalClientProtocolVersion = new KComboBox(commonSambaOptionsBox);
    minimalClientProtocolVersion->setObjectName("MinimalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalClientProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(minimalClientProtocolVersionChoices)) {
        minimalClientProtocolVersion->addItem(c.label);
    }

    QLabel *maximalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), commonMountSettingsTab);
    maximalClientProtocolVersionLabel->setIndent(25);
    maximalClientProtocolVersionLabel->setObjectName("MaximalClientProtocolVersionLabel");

    KComboBox *maximalClientProtocolVersion = new KComboBox(commonMountSettingsTab);
    maximalClientProtocolVersion->setObjectName("MaximalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalClientProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(maximalClientProtocolVersionChoices)) {
        maximalClientProtocolVersion->addItem(c.label);
    }

    minimalClientProtocolVersionLabel->setBuddy(minimalClientProtocolVersion);
    maximalClientProtocolVersionLabel->setBuddy(maximalClientProtocolVersion);

    connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotUseClientProtocolVersions(bool)));
    connect(minimalClientProtocolVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCheckValues()));
    connect(maximalClientProtocolVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCheckValues()));

    commonSambaOptionsBoxLayout->addWidget(useClientProtocolVersions, 0, 0, 1, 2);
    commonSambaOptionsBoxLayout->addWidget(minimalClientProtocolVersionLabel, 1, 0);
    commonSambaOptionsBoxLayout->addWidget(minimalClientProtocolVersion, 1, 1);
    commonSambaOptionsBoxLayout->addWidget(maximalClientProtocolVersionLabel, 2, 0);
    commonSambaOptionsBoxLayout->addWidget(maximalClientProtocolVersion, 2, 1);

    // SMB port
    QCheckBox *useSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonSambaOptionsBox);
    useSmbPort->setObjectName("UseSmbPort");

    QSpinBox *smbPort = new QSpinBox(commonSambaOptionsBox);
    smbPort->setObjectName("SmbPort");
    smbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
    smbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());

    connect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotCheckValues()));

    commonSambaOptionsBoxLayout->addWidget(useSmbPort, 3, 0);
    commonSambaOptionsBoxLayout->addWidget(smbPort, 3, 1);

    sambaTabLayout->addWidget(commonSambaOptionsBox, 0);

    //
    // Authentication
    //
    QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
    QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);

    // Kerberos
    QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
    useKerberos->setObjectName("UseKerberos");

    connect(useKerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));

    authenticationBoxLayout->addWidget(useKerberos, 0);

    sambaTabLayout->addWidget(authenticationBox, 0);
    sambaTabLayout->addStretch(100);

    tabWidget->addTab(sambaTab, i18n("Browse Settings"));

    //
    // Custom options for Wake-On-LAN
    //
    // NOTE: If you change the texts here, also alter them in the respective
    // config page.
    //
    QWidget *wakeOnLanTab = new QWidget(tabWidget);
    QVBoxLayout *wakeOnLanTabLayout = new QVBoxLayout(wakeOnLanTab);

    //
    // MAC address
    //
    QGroupBox *macAddressBox = new QGroupBox(i18n("MAC Address"), wakeOnLanTab);
    QGridLayout *macAddressBoxLayout = new QGridLayout(macAddressBox);

    // MAC address
    QLabel *macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressBox);
    KLineEdit *macAddress = new KLineEdit(macAddressBox);
    macAddress->setObjectName("MACAddress");
    macAddress->setClearButtonEnabled(true);
    macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_"); // MAC address, see QLineEdit doc
    macAddressLabel->setBuddy(macAddress);

    connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotCheckValues()));
    connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotEnableWOLFeatures(QString)));

    macAddressBoxLayout->addWidget(macAddressLabel, 0, 0);
    macAddressBoxLayout->addWidget(macAddress, 0, 1);

    wakeOnLanTabLayout->addWidget(macAddressBox, 0);

    //
    // Wake-On-LAN Actions
    //
    QGroupBox *wakeOnLANActionsBox = new QGroupBox(i18n("Actions"), wakeOnLanTab);
    QVBoxLayout *wakeOnLANActionsBoxLayout = new QVBoxLayout(wakeOnLANActionsBox);

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

    tabWidget->addTab(wakeOnLanTab, i18n("Wake-On-LAN Settings"));

    //
    // Load settings
    //
    if (m_options->hasOptions()) {
        // Remounting
        if (m_options->type() == Share) {
            remountAlways->setChecked((m_options->remount() == Smb4KCustomOptions::RemountAlways));
        }

        // CIFS Unix extensions support
        cifsExtensionsSupport->setChecked(m_options->cifsUnixExtensionsSupport());

        // User information
        useUserId->setChecked(m_options->useUser());
        userId->setCurrentText(QString("%1 (%2)").arg(m_options->user().loginName(), m_options->user().userId().toString()));

        // Group information
        useGroupId->setChecked(m_options->useGroup());
        groupId->setCurrentText(QString("%1 (%2)").arg(m_options->group().name(), m_options->group().groupId().toString()));

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

        QString writeAccessText = Smb4KMountSettings::self()->writeAccessItem()->choices().value(m_options->writeAccess()).label;
        writeAccess->setCurrentText(writeAccessText);

        // SMB protocol version for mounting
        useMountProtocol->setChecked(m_options->useMountProtocolVersion());

        QString mountProtocolVersionString = Smb4KMountSettings::self()->smbProtocolVersionItem()->choices().value(m_options->mountProtocolVersion()).label;
        mountProtocol->setCurrentText(mountProtocolVersionString);

        // Security mode
        useSecurityMode->setChecked(m_options->useSecurityMode());

        QString securityModeText = Smb4KMountSettings::self()->securityModeItem()->choices().value(m_options->securityMode()).label;
        securityMode->setCurrentText(securityModeText);

        // Client protocol versions
        useClientProtocolVersions->setChecked(m_options->useClientProtocolVersions());

        QString minimalClientProtocolVersionString =
            Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices().value(m_options->minimalClientProtocolVersion()).label;
        minimalClientProtocolVersion->setCurrentText(minimalClientProtocolVersionString);

        QString maximalClientProtocolVersionString =
            Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices().value(m_options->maximalClientProtocolVersion()).label;
        maximalClientProtocolVersion->setCurrentText(maximalClientProtocolVersionString);

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
    } else {
        setDefaultValues();
    }

    //
    // Enable/disable features
    //
    wakeOnLanTab->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
    slotEnableWOLFeatures(macAddress->text());
    slotCifsExtensionsSupport(cifsExtensionsSupport->isChecked());
    slotUseClientProtocolVersions(useClientProtocolVersions->isChecked());
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

    //
    // Common options
    //
    QGroupBox *commonBox = new QGroupBox(i18n("Common Options"), mountingTab);
    QGridLayout *commonBoxLayout = new QGridLayout(commonBox);

    QCheckBox *remountAlways = new QCheckBox(i18n("Always remount this share"), commonBox);
    remountAlways->setObjectName("RemountAlways");
    remountAlways->setEnabled(m_options->type() == Share);
    connect(remountAlways, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(remountAlways, 0, 0, 1, 2);

    // User Id
    QCheckBox *useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), commonBox);
    useUserId->setObjectName("UseUserId");

    KComboBox *userId = new KComboBox(commonBox);
    userId->setObjectName("UserId");

    QList<KUser> allUsers = KUser::allUsers();

    for (const KUser &u : allUsers) {
        userId->addItem(QString("%1 (%2)").arg(u.loginName(), u.userId().toString()));
    }

    connect(useUserId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(userId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(useUserId, 1, 0);
    commonBoxLayout->addWidget(userId, 1, 1);

    // Group Id
    QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), commonBox);
    useGroupId->setObjectName("UseGroupId");

    KComboBox *groupId = new KComboBox(commonBox);
    groupId->setObjectName("GroupId");

    QList<KUserGroup> allGroups = KUserGroup::allGroups();

    for (const KUserGroup &g : allGroups) {
        groupId->addItem(QString("%1 (%2)").arg(g.name(), g.groupId().toString()));
    }

    connect(useGroupId, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));
    connect(groupId, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(useGroupId, 2, 0);
    commonBoxLayout->addWidget(groupId, 2, 1);

    // File mode
    QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), commonBox);
    useFileMode->setObjectName("UseFileMode");

    KLineEdit *fileMode = new KLineEdit(commonBox);
    fileMode->setObjectName("FileMode");
    fileMode->setClearButtonEnabled(true);
    fileMode->setAlignment(Qt::AlignRight);

    connect(useFileMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(fileMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(useFileMode, 3, 0);
    commonBoxLayout->addWidget(fileMode, 3, 1);

    // Directory mode
    QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), commonBox);
    useDirectoryMode->setObjectName("UseDirectoryMode");

    KLineEdit *directoryMode = new KLineEdit(commonBox);
    directoryMode->setObjectName("DirectoryMode");
    directoryMode->setClearButtonEnabled(true);
    directoryMode->setAlignment(Qt::AlignRight);

    connect(useDirectoryMode, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(directoryMode, SIGNAL(textEdited(QString)), this, SLOT(slotCheckValues()));

    commonBoxLayout->addWidget(useDirectoryMode, 4, 0);
    commonBoxLayout->addWidget(directoryMode, 4, 1);

    mountingTabLayout->addWidget(commonBox, 0);
    mountingTabLayout->addStretch(100);

    tabWidget->addTab(mountingTab, i18n("Mount Settings"));

    //
    // Custom options for Browsing
    //
    QWidget *sambaTab = new QWidget(tabWidget);
    QVBoxLayout *sambaTabLayout = new QVBoxLayout(sambaTab);

    //
    // Common Options
    //
    QGroupBox *commonSambaOptionsBox = new QGroupBox(i18n("Common Options"), sambaTab);
    QGridLayout *commonSambaOptionsBoxLayout = new QGridLayout(commonSambaOptionsBox);

    // Minimal and maximal client protocol versions
    QCheckBox *useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), commonSambaOptionsBox);
    useClientProtocolVersions->setObjectName("UseClientProtocolVersions");

    QLabel *minimalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), commonSambaOptionsBox);
    minimalClientProtocolVersionLabel->setIndent(25);
    minimalClientProtocolVersionLabel->setObjectName("MinimalProtocolVersionLabel");

    KComboBox *minimalClientProtocolVersion = new KComboBox(commonSambaOptionsBox);
    minimalClientProtocolVersion->setObjectName("MinimalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalClientProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : minimalClientProtocolVersionChoices) {
        minimalClientProtocolVersion->addItem(c.label);
    }

    QLabel *maximalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), commonSambaOptionsBox);
    maximalClientProtocolVersionLabel->setIndent(25);
    maximalClientProtocolVersionLabel->setObjectName("MaximalProtocolVersionLabel");

    KComboBox *maximalClientProtocolVersion = new KComboBox(commonSambaOptionsBox);
    maximalClientProtocolVersion->setObjectName("MaximalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalClientProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : maximalClientProtocolVersionChoices) {
        maximalClientProtocolVersion->addItem(c.label);
    }

    minimalClientProtocolVersionLabel->setBuddy(minimalClientProtocolVersion);
    maximalClientProtocolVersionLabel->setBuddy(maximalClientProtocolVersion);

    connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotUseClientProtocolVersions(bool)));
    connect(minimalClientProtocolVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCheckValues()));
    connect(maximalClientProtocolVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCheckValues()));

    commonSambaOptionsBoxLayout->addWidget(useClientProtocolVersions, 0, 0, 1, 2);
    commonSambaOptionsBoxLayout->addWidget(minimalClientProtocolVersionLabel, 1, 0);
    commonSambaOptionsBoxLayout->addWidget(minimalClientProtocolVersion, 1, 1);
    commonSambaOptionsBoxLayout->addWidget(maximalClientProtocolVersionLabel, 2, 0);
    commonSambaOptionsBoxLayout->addWidget(maximalClientProtocolVersion, 2, 1);

    // SMB port
    QCheckBox *useSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonSambaOptionsBox);
    useSmbPort->setObjectName("UseSmbPort");

    QSpinBox *smbPort = new QSpinBox(commonSambaOptionsBox);
    smbPort->setObjectName("SmbPort");
    smbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
    smbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());

    connect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotCheckValues()));

    commonSambaOptionsBoxLayout->addWidget(useSmbPort, 3, 0);
    commonSambaOptionsBoxLayout->addWidget(smbPort, 3, 1);

    sambaTabLayout->addWidget(commonSambaOptionsBox, 0);

    //
    // Authentication
    //
    QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
    QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);

    // Kerberos
    QCheckBox *useKerberos = new QCheckBox(Smb4KSettings::self()->useKerberosItem()->label(), authenticationBox);
    useKerberos->setObjectName("UseKerberos");

    connect(useKerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()));

    authenticationBoxLayout->addWidget(useKerberos, 0);

    sambaTabLayout->addWidget(authenticationBox, 0);
    sambaTabLayout->addStretch(100);

    tabWidget->addTab(sambaTab, i18n("Browse Settings"));

    //
    // Custom options for Wake-On-LAN
    //
    // NOTE: If you change the texts here, also alter them in the respective
    // config page.
    //
    QWidget *wakeOnLanTab = new QWidget(tabWidget);
    QVBoxLayout *wakeOnLanTabLayout = new QVBoxLayout(wakeOnLanTab);

    //
    // MAC address
    //
    QGroupBox *macAddressBox = new QGroupBox(i18n("MAC Address"), wakeOnLanTab);
    QGridLayout *macAddressBoxLayout = new QGridLayout(macAddressBox);

    // MAC address
    QLabel *macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressBox);
    KLineEdit *macAddress = new KLineEdit(macAddressBox);
    macAddress->setObjectName("MACAddress");
    macAddress->setClearButtonEnabled(true);
    macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_"); // MAC address, see QLineEdit doc
    macAddressLabel->setBuddy(macAddress);

    connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotCheckValues()));
    connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotEnableWOLFeatures(QString)));

    macAddressBoxLayout->addWidget(macAddressLabel, 0, 0);
    macAddressBoxLayout->addWidget(macAddress, 0, 1);

    wakeOnLanTabLayout->addWidget(macAddressBox, 0);

    //
    // Wake-On-LAN Actions
    //
    QGroupBox *wakeOnLANActionsBox = new QGroupBox(i18n("Actions"), wakeOnLanTab);
    QVBoxLayout *wakeOnLANActionsBoxLayout = new QVBoxLayout(wakeOnLANActionsBox);

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

    tabWidget->addTab(wakeOnLanTab, i18n("Wake-On-LAN Settings"));

    //
    // Load settings
    //
    if (m_options->hasOptions()) {
        if (m_options->type() == Share) {
            remountAlways->setChecked((m_options->remount() == Smb4KCustomOptions::RemountAlways));
        }

        // User information
        useUserId->setChecked(m_options->useUser());
        userId->setCurrentText(QString("%1 (%2)").arg(m_options->user().loginName(), m_options->user().userId().toString()));

        // Group information
        useGroupId->setChecked(m_options->useGroup());
        groupId->setCurrentText(QString("%1 (%2)").arg(m_options->group().name(), m_options->group().groupId().toString()));

        // File mode
        useFileMode->setChecked(m_options->useFileMode());
        fileMode->setText(m_options->fileMode());

        // Directory mode
        useDirectoryMode->setChecked(m_options->useDirectoryMode());
        directoryMode->setText(m_options->directoryMode());

        // Client protocol versions
        useClientProtocolVersions->setChecked(m_options->useClientProtocolVersions());

        QString minimalClientProtocolVersionString =
            Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices().value(m_options->minimalClientProtocolVersion()).label;
        minimalClientProtocolVersion->setCurrentText(minimalClientProtocolVersionString);

        QString maximalClientProtocolVersionString =
            Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices().value(m_options->maximalClientProtocolVersion()).label;
        maximalClientProtocolVersion->setCurrentText(maximalClientProtocolVersionString);

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
    } else {
        setDefaultValues();
    }

    //
    // Enable/disable features
    //
    wakeOnLanTab->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
    slotEnableWOLFeatures(macAddress->text());
    slotUseClientProtocolVersions(useClientProtocolVersions->isChecked());
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

    //
    // Common Options
    //
    QGroupBox *commonSambaOptionsBox = new QGroupBox(i18n("Common Options"), sambaTab);
    QGridLayout *commonSambaOptionsBoxLayout = new QGridLayout(commonSambaOptionsBox);

    // Minimal and maximal client protocol versions
    QCheckBox *useClientProtocolVersions = new QCheckBox(Smb4KSettings::self()->useClientProtocolVersionsItem()->label(), commonMountSettingsTab);
    useClientProtocolVersions->setObjectName("UseClientProtocolVersions");

    QLabel *minimalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->minimalClientProtocolVersionItem()->label(), commonMountSettingsTab);
    minimalClientProtocolVersionLabel->setIndent(25);
    minimalClientProtocolVersionLabel->setObjectName("MinimalProtocolVersionLabel");

    KComboBox *minimalClientProtocolVersion = new KComboBox(commonSambaOptionsBox);
    minimalClientProtocolVersion->setObjectName("MinimalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalClientProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : minimalClientProtocolVersionChoices) {
        minimalClientProtocolVersion->addItem(c.label);
    }

    QLabel *maximalClientProtocolVersionLabel = new QLabel(Smb4KSettings::self()->maximalClientProtocolVersionItem()->label(), commonMountSettingsTab);
    maximalClientProtocolVersionLabel->setIndent(25);
    maximalClientProtocolVersionLabel->setObjectName("MaximalProtocolVersionLabel");

    KComboBox *maximalClientProtocolVersion = new KComboBox(commonMountSettingsTab);
    maximalClientProtocolVersion->setObjectName("MaximalClientProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalClientProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : maximalClientProtocolVersionChoices) {
        maximalClientProtocolVersion->addItem(c.label);
    }

    minimalClientProtocolVersionLabel->setBuddy(minimalClientProtocolVersion);
    maximalClientProtocolVersionLabel->setBuddy(maximalClientProtocolVersion);

    connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(useClientProtocolVersions, SIGNAL(toggled(bool)), this, SLOT(slotUseClientProtocolVersions(bool)));
    connect(minimalClientProtocolVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCheckValues()));
    connect(maximalClientProtocolVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCheckValues()));

    commonSambaOptionsBoxLayout->addWidget(useClientProtocolVersions, 0, 0, 1, 2);
    commonSambaOptionsBoxLayout->addWidget(minimalClientProtocolVersionLabel, 1, 0);
    commonSambaOptionsBoxLayout->addWidget(minimalClientProtocolVersion, 1, 1);
    commonSambaOptionsBoxLayout->addWidget(maximalClientProtocolVersionLabel, 2, 0);
    commonSambaOptionsBoxLayout->addWidget(maximalClientProtocolVersion, 2, 1);

    // SMB port
    QCheckBox *useSmbPort = new QCheckBox(Smb4KSettings::self()->useRemoteSmbPortItem()->label(), commonSambaOptionsBox);
    useSmbPort->setObjectName("UseSmbPort");

    QSpinBox *smbPort = new QSpinBox(commonSambaOptionsBox);
    smbPort->setObjectName("SmbPort");
    smbPort->setMinimum(Smb4KSettings::self()->remoteSmbPortItem()->minValue().toInt());
    smbPort->setMaximum(Smb4KSettings::self()->remoteSmbPortItem()->maxValue().toInt());

    connect(useSmbPort, SIGNAL(toggled(bool)), this, SLOT(slotCheckValues()));
    connect(smbPort, SIGNAL(valueChanged(int)), this, SLOT(slotCheckValues()));

    commonSambaOptionsBoxLayout->addWidget(useSmbPort, 3, 0);
    commonSambaOptionsBoxLayout->addWidget(smbPort, 3, 1);

    sambaTabLayout->addWidget(commonSambaOptionsBox, 0);

    //
    // Authentication
    //
    QGroupBox *authenticationBox = new QGroupBox(i18n("Authentication"), sambaTab);
    QVBoxLayout *authenticationBoxLayout = new QVBoxLayout(authenticationBox);

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

    //
    // MAC address
    //
    QGroupBox *macAddressBox = new QGroupBox(i18n("MAC Address"), wakeOnLanTab);
    QGridLayout *macAddressBoxLayout = new QGridLayout(macAddressBox);

    // MAC address
    QLabel *macAddressLabel = new QLabel(i18n("MAC Address:"), macAddressBox);
    KLineEdit *macAddress = new KLineEdit(macAddressBox);
    macAddress->setObjectName("MACAddress");
    macAddress->setClearButtonEnabled(true);
    macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_"); // MAC address, see QLineEdit doc
    macAddressLabel->setBuddy(macAddress);

    connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotCheckValues()));
    connect(macAddress, SIGNAL(textEdited(QString)), SLOT(slotEnableWOLFeatures(QString)));

    macAddressBoxLayout->addWidget(macAddressLabel, 0, 0);
    macAddressBoxLayout->addWidget(macAddress, 0, 1);

    wakeOnLanTabLayout->addWidget(macAddressBox, 0);

    //
    // Wake-On-LAN Actions
    //
    QGroupBox *wakeOnLANActionsBox = new QGroupBox(i18n("Actions"), wakeOnLanTab);
    QVBoxLayout *wakeOnLANActionsBoxLayout = new QVBoxLayout(wakeOnLANActionsBox);

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
    if (m_options->hasOptions()) {
        // Client protocol versions
        useClientProtocolVersions->setChecked(m_options->useClientProtocolVersions());

        QString minimalClientProtocolVersionString =
            Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices().value(m_options->minimalClientProtocolVersion()).label;
        minimalClientProtocolVersion->setCurrentText(minimalClientProtocolVersionString);

        QString maximalClientProtocolVersionString =
            Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices().value(m_options->maximalClientProtocolVersion()).label;
        maximalClientProtocolVersion->setCurrentText(maximalClientProtocolVersionString);

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
    } else {
        setDefaultValues();
    }

    //
    // Enable/disable features
    //
    wakeOnLanTab->setEnabled((m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()));
    slotEnableWOLFeatures(macAddress->text());
    slotUseClientProtocolVersions(useClientProtocolVersions->isChecked());
}
#endif

bool Smb4KCustomOptionsDialog::checkDefaultValues()
{
    //
    // Always remount the share
    //
    if (m_options->type() == Share) {
        QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");

        if (remountAlways) {
            if (remountAlways->isChecked()) {
                return false;
            }
        }
    }

    //
    // User Id
    //
    QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");

    if (useUserId) {
        if (useUserId->isChecked() != Smb4KMountSettings::useUserId()) {
            return false;
        }
    }

    KComboBox *userId = findChild<KComboBox *>("UserId");

    if (userId) {
        bool ok = false;
        K_UID uid = (K_UID)Smb4KMountSettings::userId().toInt(&ok);

        if (ok) {
            KUser defaultUserId(uid);

            if (defaultUserId.isValid()) {
                QString defaultUserIdString = QString("%1 (%2)").arg(defaultUserId.loginName(), defaultUserId.userId().toString());

                if (userId->currentText() != defaultUserIdString) {
                    return false;
                }
            }
        }
    }

    //
    // Group Id
    //
    QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");

    if (useGroupId) {
        if (useGroupId->isChecked() != Smb4KMountSettings::useGroupId()) {
            return false;
        }
    }

    KComboBox *groupId = findChild<KComboBox *>("GroupId");

    if (groupId) {
        bool ok = false;
        K_GID gid = (K_GID)Smb4KMountSettings::groupId().toInt(&ok);

        if (ok) {
            KUserGroup defaultGroupId(gid);

            if (defaultGroupId.isValid()) {
                QString defaultGroupIdString = QString("%1 (%2)").arg(defaultGroupId.name(), defaultGroupId.groupId().toString());

                if (groupId->currentText() != defaultGroupIdString) {
                    return false;
                }
            }
        }
    }

    //
    // File mode
    //
    QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");

    if (useFileMode) {
        if (useFileMode->isChecked() != Smb4KMountSettings::useFileMode()) {
            return false;
        }
    }

    KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");

    if (fileMode) {
        if (fileMode->text() != Smb4KMountSettings::fileMode()) {
            return false;
        }
    }

    //
    // Directory mode
    //
    QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");

    if (useDirectoryMode) {
        if (useDirectoryMode->isChecked() != Smb4KMountSettings::useDirectoryMode()) {
            return false;
        }
    }

    KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");

    if (directoryMode) {
        if (directoryMode->text() != Smb4KMountSettings::directoryMode()) {
            return false;
        }
    }

#if defined(Q_OS_LINUX)
    //
    // CIFS Unix extensions support
    //
    QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");

    if (cifsExtensionsSupport) {
        if (cifsExtensionsSupport->isChecked() != Smb4KMountSettings::cifsUnixExtensionsSupport()) {
            return false;
        }
    }

    //
    // Filesystem port
    //
    QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");

    if (useFilesystemPort) {
        if (useFilesystemPort->isChecked() != Smb4KMountSettings::useRemoteFileSystemPort()) {
            return false;
        }
    }

    QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");

    if (filesystemPort) {
        if (filesystemPort->value() != Smb4KMountSettings::remoteFileSystemPort()) {
            return false;
        }
    }

    //
    // Write access
    //
    QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");

    if (useWriteAccess) {
        if (useWriteAccess->isChecked() != Smb4KMountSettings::useWriteAccess()) {
            return false;
        }
    }

    KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");

    if (writeAccess) {
        if (writeAccess->currentText() != Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::writeAccess()).label) {
            return false;
        }
    }

    //
    // SMB mount protocol version
    //
    QCheckBox *useMountProtocol = findChild<QCheckBox *>("UseMountProtocolVersion");

    if (useMountProtocol) {
        if (useMountProtocol->isChecked() != Smb4KMountSettings::useSmbProtocolVersion()) {
            return false;
        }
    }

    KComboBox *mountProtocol = findChild<KComboBox *>("MountProtocolVersion");

    if (mountProtocol) {
        if (mountProtocol->currentText()
            != Smb4KMountSettings::self()->smbProtocolVersionItem()->choices().value(Smb4KMountSettings::smbProtocolVersion()).label) {
            return false;
        }
    }

    //
    // Security mode
    //
    QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");

    if (useSecurityMode) {
        if (useSecurityMode->isChecked() != Smb4KMountSettings::useSecurityMode()) {
            return false;
        }
    }

    KComboBox *securityMode = findChild<KComboBox *>("SecurityMode");

    if (securityMode) {
        if (securityMode->currentText() != Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::securityMode()).label) {
            return false;
        }
    }
#endif

    //
    // Client protocol versions
    //
    QCheckBox *useClientProtocolVersions = findChild<QCheckBox *>("UseClientProtocolVersions");

    if (useClientProtocolVersions) {
        if (useClientProtocolVersions->isChecked() != Smb4KSettings::useClientProtocolVersions()) {
            return false;
        }
    }

    KComboBox *minimalClientProtocolVersion = findChild<KComboBox *>("MinimalClientProtocolVersion");

    if (minimalClientProtocolVersion) {
        if (minimalClientProtocolVersion->currentText()
            != Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices().value(Smb4KSettings::minimalClientProtocolVersion()).label) {
            return false;
        }
    }

    KComboBox *maximalClientProtocolVersion = findChild<KComboBox *>("MaximalClientProtocolVersion");

    if (maximalClientProtocolVersion) {
        if (maximalClientProtocolVersion->currentText()
            != Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices().value(Smb4KSettings::maximalClientProtocolVersion()).label) {
            return false;
        }
    }

    //
    // SMB port
    //
    QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");

    if (useSmbPort) {
        if (useSmbPort->isChecked() != Smb4KSettings::useRemoteSmbPort()) {
            return false;
        }
    }

    QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");

    if (smbPort) {
        if (smbPort->value() != Smb4KSettings::remoteSmbPort()) {
            return false;
        }
    }

    //
    // Kerberos
    //
    QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");

    if (useKerberos) {
        if (useKerberos->isChecked() != Smb4KSettings::useKerberos()) {
            return false;
        }
    }

    //
    // MAC address & Wake-On-LAN features
    //
    if (m_options->type() == Host && Smb4KSettings::enableWakeOnLAN()) {
        KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");

        if (macAddress) {
            QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");

            if (exp.exactMatch(macAddress->text())) {
                return false;
            }
        }

        QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");

        if (sendPackageBeforeScan) {
            if (sendPackageBeforeScan->isChecked()) {
                return false;
            }
        }

        QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");

        if (sendPackageBeforeMount) {
            if (sendPackageBeforeMount->isChecked()) {
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
    if (m_options->type() == Share) {
        QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");

        if (remountAlways) {
            remountAlways->setChecked(false);
        }
    }

    //
    // User Id
    //
    QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");

    if (useUserId) {
        useUserId->setChecked(Smb4KMountSettings::useUserId());
    }

    KComboBox *userId = findChild<KComboBox *>("UserId");

    if (userId) {
        bool ok = false;
        K_UID uid = (K_UID)Smb4KMountSettings::userId().toInt(&ok);

        if (ok) {
            KUser defaultUserId(uid);

            if (defaultUserId.isValid()) {
                userId->setCurrentText(QString("%1 (%2)").arg(defaultUserId.loginName(), defaultUserId.userId().toString()));
            }
        }
    }

    //
    // Group Id
    //
    QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");

    if (useGroupId) {
        useGroupId->setChecked(Smb4KMountSettings::useGroupId());
    }

    KComboBox *groupId = findChild<KComboBox *>("GroupId");

    if (groupId) {
        bool ok = false;
        K_GID gid = (K_GID)Smb4KMountSettings::groupId().toInt(&ok);

        if (ok) {
            KUserGroup defaultGroupId(gid);

            if (defaultGroupId.isValid()) {
                groupId->setCurrentText(QString("%1 (%2)").arg(defaultGroupId.name(), defaultGroupId.groupId().toString()));
            }
        }
    }

    //
    // File mask
    //
    QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");

    if (useFileMode) {
        useFileMode->setChecked(Smb4KMountSettings::useFileMode());
    }

    KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");

    if (fileMode) {
        fileMode->setText(Smb4KMountSettings::fileMode());
    }

    //
    // Directory mode
    //
    QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");

    if (useDirectoryMode) {
        useDirectoryMode->setChecked(Smb4KMountSettings::useDirectoryMode());
    }

    KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");

    if (directoryMode) {
        directoryMode->setText(Smb4KMountSettings::directoryMode());
    }

#if defined(Q_OS_LINUX)
    //
    // CIFS Unix extensions support
    //
    QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");

    if (cifsExtensionsSupport) {
        cifsExtensionsSupport->setChecked(Smb4KMountSettings::cifsUnixExtensionsSupport());
    }

    //
    // Filesystem port
    //
    QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");

    if (useFilesystemPort) {
        useFilesystemPort->setChecked(Smb4KMountSettings::useRemoteFileSystemPort());
    }

    QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");

    if (filesystemPort) {
        filesystemPort->setValue(Smb4KMountSettings::remoteFileSystemPort());
    }

    //
    // Write access
    //
    QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");

    if (useWriteAccess) {
        useWriteAccess->setChecked(Smb4KMountSettings::useWriteAccess());
    }

    KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");

    if (writeAccess) {
        QString writeAccessString = Smb4KMountSettings::self()->writeAccessItem()->choices().value(Smb4KMountSettings::writeAccess()).label;
        writeAccess->setCurrentText(writeAccessString);
    }

    //
    // SMB mount protocol version
    //
    QCheckBox *useMountProtocol = findChild<QCheckBox *>("UseMountProtocolVersion");

    if (useMountProtocol) {
        useMountProtocol->setChecked(Smb4KMountSettings::useSmbProtocolVersion());
    }

    KComboBox *mountProtocol = findChild<KComboBox *>("MountProtocolVersion");

    if (mountProtocol) {
        QString mountProtocolVersionString =
            Smb4KMountSettings::self()->smbProtocolVersionItem()->choices().value(Smb4KMountSettings::smbProtocolVersion()).label;
        mountProtocol->setCurrentText(mountProtocolVersionString);
    }

    //
    // Security mode
    //
    QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");

    if (useSecurityMode) {
        useSecurityMode->setChecked(Smb4KMountSettings::useSecurityMode());
    }

    KComboBox *securityMode = findChild<KComboBox *>("SecurityMode");

    if (securityMode) {
        QString securityModeString = Smb4KMountSettings::self()->securityModeItem()->choices().value(Smb4KMountSettings::securityMode()).label;
        securityMode->setCurrentText(securityModeString);
    }
#endif

    //
    // Client protocol versions
    //
    QCheckBox *useClientProtocolVersions = findChild<QCheckBox *>("UseClientProtocolVersions");

    if (useClientProtocolVersions) {
        useClientProtocolVersions->setChecked(Smb4KSettings::useClientProtocolVersions());
    }

    KComboBox *minimalClientProtocolVersion = findChild<KComboBox *>("MinimalClientProtocolVersion");

    if (minimalClientProtocolVersion) {
        QString minimalClientProtocolVersionString =
            Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices().value(Smb4KSettings::minimalClientProtocolVersion()).label;
        minimalClientProtocolVersion->setCurrentText(minimalClientProtocolVersionString);
    }

    KComboBox *maximalClientProtocolVersion = findChild<KComboBox *>("MaximalClientProtocolVersion");

    if (maximalClientProtocolVersion) {
        QString maximalClientProtocolVersionString =
            Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices().value(Smb4KSettings::maximalClientProtocolVersion()).label;
        maximalClientProtocolVersion->setCurrentText(maximalClientProtocolVersionString);
    }

    //
    // SMB port
    //
    QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");

    if (useSmbPort) {
        useSmbPort->setChecked(Smb4KSettings::useRemoteSmbPort());
    }

    QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");

    if (smbPort) {
        smbPort->setValue(Smb4KSettings::remoteSmbPort());
    }

    //
    // Kerberos
    //
    QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");

    if (useKerberos) {
        useKerberos->setChecked(Smb4KSettings::useKerberos());
    }

    //
    // MAC address & Wake-On-LAN features
    //
    if (m_options->type() == Host) {
        KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");

        if (macAddress) {
            macAddress->clear();
            macAddress->setInputMask("HH:HH:HH:HH:HH:HH;_");
        }

        QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");

        if (sendPackageBeforeScan) {
            sendPackageBeforeScan->setChecked(false);
        }

        QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");

        if (sendPackageBeforeMount) {
            sendPackageBeforeMount->setChecked(false);
        }
    }
}

void Smb4KCustomOptionsDialog::saveValues()
{
    //
    // Always remount the share
    //
    if (m_options->type() == Share) {
        QCheckBox *remountAlways = findChild<QCheckBox *>("RemountAlways");

        if (remountAlways) {
            if (remountAlways->isChecked()) {
                m_options->setRemount(Smb4KCustomOptions::RemountAlways);
            } else {
                m_options->setRemount(Smb4KCustomOptions::UndefinedRemount);
            }
        }
    }

    //
    // User Id
    //
    QCheckBox *useUserId = findChild<QCheckBox *>("UseUserId");

    if (useUserId) {
        m_options->setUseUser(useUserId->isChecked());
    }

    KComboBox *userId = findChild<KComboBox *>("UserId");

    if (userId) {
        QString selectedUserIdString = userId->currentText().section("(", 1, 1).section(")", 0, 0).trimmed();

        bool ok = false;
        K_UID uid = (K_UID)selectedUserIdString.toInt(&ok);

        if (ok) {
            KUser selectedUserId(uid);

            if (selectedUserId.isValid()) {
                m_options->setUser(selectedUserId);
            }
        }
    }

    //
    // Group Id
    //
    QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");

    if (useGroupId) {
        m_options->setUseGroup(useGroupId->isChecked());
    }

    KComboBox *groupId = findChild<KComboBox *>("GroupId");

    if (groupId) {
        QString selectedGroupIdString = groupId->currentText().section("(", 1, 1).section(")", 0, 0).trimmed();

        bool ok = false;
        K_GID gid = (K_GID)selectedGroupIdString.toInt(&ok);

        if (ok) {
            KUserGroup selectedGroupId(gid);

            if (selectedGroupId.isValid()) {
                m_options->setGroup(selectedGroupId);
            }
        }
    }

    //
    // File mode
    //
    QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");

    if (useFileMode) {
        m_options->setUseFileMode(useFileMode->isChecked());
    }

    KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");

    if (fileMode) {
        m_options->setFileMode(fileMode->text());
    }

    //
    // Directory mode
    //
    QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");

    if (useDirectoryMode) {
        m_options->setUseDirectoryMode(useDirectoryMode->isChecked());
    }

    KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");

    if (directoryMode) {
        m_options->setDirectoryMode(directoryMode->text());
    }

#if defined(Q_OS_LINUX)
    //
    // CIFS Unix extensions support
    //
    QCheckBox *cifsExtensionsSupport = findChild<QCheckBox *>("CifsExtensionsSupport");

    if (cifsExtensionsSupport) {
        m_options->setCifsUnixExtensionsSupport(cifsExtensionsSupport->isChecked());
    }

    //
    // Filesystem port
    //
    QCheckBox *useFilesystemPort = findChild<QCheckBox *>("UseFilesystemPort");

    if (useFilesystemPort) {
        m_options->setUseFileSystemPort(useFilesystemPort->isChecked());
    }

    QSpinBox *filesystemPort = findChild<QSpinBox *>("FileSystemPort");

    if (filesystemPort) {
        m_options->setFileSystemPort(filesystemPort->value());
    }

    //
    // Write access
    //
    QCheckBox *useWriteAccess = findChild<QCheckBox *>("UseWriteAccess");

    if (useWriteAccess) {
        m_options->setUseWriteAccess(useWriteAccess->isChecked());
    }

    KComboBox *writeAccess = findChild<KComboBox *>("WriteAccess");

    if (writeAccess) {
        QList<KCoreConfigSkeleton::ItemEnum::Choice> writeAccessChoices = Smb4KMountSettings::self()->writeAccessItem()->choices();

        for (int i = 0; i < writeAccessChoices.size(); i++) {
            if (writeAccess->currentText() == writeAccessChoices.at(i).label) {
                m_options->setWriteAccess(i);
                break;
            }
        }
    }

    //
    // SMB mount protocol version
    //
    QCheckBox *useMountProtocol = findChild<QCheckBox *>("UseMountProtocolVersion");

    if (useMountProtocol) {
        m_options->setUseMountProtocolVersion(useMountProtocol->isChecked());
    }

    KComboBox *mountProtocol = findChild<KComboBox *>("MountProtocolVersion");

    if (mountProtocol) {
        QList<KCoreConfigSkeleton::ItemEnum::Choice> smbProtocolVersionChoices = Smb4KMountSettings::self()->smbProtocolVersionItem()->choices();

        for (int i = 0; i < smbProtocolVersionChoices.size(); i++) {
            if (mountProtocol->currentText() == smbProtocolVersionChoices.at(i).label) {
                m_options->setMountProtocolVersion(i);
                break;
            }
        }
    }

    //
    // Security mode
    //
    QCheckBox *useSecurityMode = findChild<QCheckBox *>("UseSecurityMode");

    if (useSecurityMode) {
        m_options->setUseSecurityMode(useSecurityMode->isChecked());
    }

    KComboBox *securityMode = findChild<KComboBox *>("SecurityMode");

    if (securityMode) {
        QList<KCoreConfigSkeleton::ItemEnum::Choice> securityModeChoices = Smb4KMountSettings::self()->securityModeItem()->choices();

        for (int i = 0; i < securityModeChoices.size(); i++) {
            if (securityMode->currentText() == securityModeChoices.at(i).label) {
                m_options->setSecurityMode(i);
                break;
            }
        }
    }
#endif

    //
    // Client protocol versions
    //
    QCheckBox *useClientProtocolVersions = findChild<QCheckBox *>("UseClientProtocolVersions");

    if (useClientProtocolVersions) {
        m_options->setUseClientProtocolVersions(useClientProtocolVersions->isChecked());
    }

    KComboBox *minimalClientProtocolVersion = findChild<KComboBox *>("MinimalClientProtocolVersion");

    if (minimalClientProtocolVersion) {
        QList<KCoreConfigSkeleton::ItemEnum::Choice> minimalClientProtocolVersionChoices = Smb4KSettings::self()->minimalClientProtocolVersionItem()->choices();

        for (int i = 0; i < minimalClientProtocolVersionChoices.size(); i++) {
            if (minimalClientProtocolVersion->currentText() == minimalClientProtocolVersionChoices.at(i).label) {
                m_options->setMinimalClientProtocolVersion(i);
                break;
            }
        }
    }

    KComboBox *maximalClientProtocolVersion = findChild<KComboBox *>("MaximalClientProtocolVersion");

    if (maximalClientProtocolVersion) {
        QList<KCoreConfigSkeleton::ItemEnum::Choice> maximalClientProtocolVersionChoices = Smb4KSettings::self()->maximalClientProtocolVersionItem()->choices();

        for (int i = 0; i < maximalClientProtocolVersionChoices.size(); i++) {
            if (maximalClientProtocolVersion->currentText() == maximalClientProtocolVersionChoices.at(i).label) {
                m_options->setMaximalClientProtocolVersion(i);
                break;
            }
        }
    }

    //
    // SMB port
    //
    QCheckBox *useSmbPort = findChild<QCheckBox *>("UseSmbPort");

    if (useSmbPort) {
        m_options->setUseSmbPort(useSmbPort->isChecked());
    }

    QSpinBox *smbPort = findChild<QSpinBox *>("SmbPort");

    if (smbPort) {
        m_options->setSmbPort(smbPort->value());
    }

    // Kerberos
    QCheckBox *useKerberos = findChild<QCheckBox *>("UseKerberos");

    if (useKerberos) {
        m_options->setUseKerberos(useKerberos->isChecked());
    }

    // MAC address & Wake-On-LAN features
    if (m_options->type() == Host) {
        KLineEdit *macAddress = findChild<KLineEdit *>("MACAddress");

        if (macAddress) {
            m_options->setMACAddress(macAddress->text());
        }

        QCheckBox *sendPackageBeforeScan = findChild<QCheckBox *>("SendPackageBeforeScan");

        if (sendPackageBeforeScan) {
            m_options->setWOLSendBeforeNetworkScan(sendPackageBeforeScan->isChecked());
        }

        QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");

        if (sendPackageBeforeMount) {
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

    if (buttonBox) {
        QList<QAbstractButton *> buttons = buttonBox->buttons();

        for (QAbstractButton *b : qAsConst(buttons)) {
            if (buttonBox->buttonRole(b) == QDialogButtonBox::ResetRole) {
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

    if (sendPackageBeforeScan) {
        sendPackageBeforeScan->setEnabled(m_options->type() == Host && exp.exactMatch(mac));
    }

    QCheckBox *sendPackageBeforeMount = findChild<QCheckBox *>("SendPackageBeforeMount");

    if (sendPackageBeforeMount) {
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

    if (useUserId) {
        useUserId->setEnabled(!support);
    }

    KComboBox *userId = findChild<KComboBox *>("UserId");

    if (userId) {
        userId->setEnabled(!support);
    }

    //
    // Group id
    //
    QCheckBox *useGroupId = findChild<QCheckBox *>("UseGroupId");

    if (useGroupId) {
        useGroupId->setEnabled(!support);
    }

    KComboBox *groupId = findChild<KComboBox *>("GroupId");

    if (groupId) {
        groupId->setEnabled(!support);
    }

    //
    // File mode
    //
    QCheckBox *useFileMode = findChild<QCheckBox *>("UseFileMode");

    if (useFileMode) {
        useFileMode->setEnabled(!support);
    }

    KLineEdit *fileMode = findChild<KLineEdit *>("FileMode");

    if (fileMode) {
        fileMode->setEnabled(!support);
    }

    //
    // Directory mode
    //
    QCheckBox *useDirectoryMode = findChild<QCheckBox *>("UseDirectoryMode");

    if (useDirectoryMode) {
        useDirectoryMode->setEnabled(!support);
    }

    KLineEdit *directoryMode = findChild<KLineEdit *>("DirectoryMode");

    if (directoryMode) {
        directoryMode->setEnabled(!support);
    }
#else
    Q_UNUSED(support);
#endif
}

void Smb4KCustomOptionsDialog::slotUseClientProtocolVersions(bool use)
{
    //
    // Minimal client protocol version
    //
    QLabel *minimalClientProtocolVersionLabel = findChild<QLabel *>("MinimalClientProtocolVersionLabel");

    if (minimalClientProtocolVersionLabel) {
        minimalClientProtocolVersionLabel->setEnabled(use);
    }

    KComboBox *minimalClientProtocolVersion = findChild<KComboBox *>("MinimalClientProtocolVersion");

    if (minimalClientProtocolVersion) {
        minimalClientProtocolVersion->setEnabled(use);
    }

    //
    // Maximal client protocol version
    //
    QLabel *maximalClientProtocolVersionLabel = findChild<QLabel *>("MaximalClientProtocolVersionLabel");

    if (maximalClientProtocolVersionLabel) {
        maximalClientProtocolVersionLabel->setEnabled(use);
    }

    KComboBox *maximalClientProtocolVersion = findChild<KComboBox *>("MaximalClientProtocolVersion");

    if (maximalClientProtocolVersion) {
        maximalClientProtocolVersion->setEnabled(use);
    }
}
