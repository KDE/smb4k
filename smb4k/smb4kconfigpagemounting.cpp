/*
    The configuration page for the mount options

    SPDX-FileCopyrightText: 2015-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagemounting.h"
#include "core/smb4kglobal.h"

#if defined(Q_OS_LINUX)
#include "core/smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "core/smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes
#include <KComboBox>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>

using namespace Smb4KGlobal;

Smb4KConfigPageMounting::Smb4KConfigPageMounting(QWidget *parent)
    : QTabWidget(parent)
{
    setupWidget();
}

Smb4KConfigPageMounting::~Smb4KConfigPageMounting()
{
}

bool Smb4KConfigPageMounting::checkSettings()
{
    if (!m_mountPrefix->url().isValid()) {
        m_mountPrefix->setFocus();
        return false;
    }

    if (m_useFileMode->isChecked() && m_fileMode->text().trimmed().isEmpty()) {
        m_fileMode->setFocus();
        return false;
    }

    if (m_useDirectoryMode->isChecked() && m_directoryMode->text().trimmed().isEmpty()) {
        m_directoryMode->setFocus();
        return false;
    }

    return true;
}

#if defined(Q_OS_LINUX)
//
// Linux
//
void Smb4KConfigPageMounting::setupWidget()
{
    //
    // Basic Settings tab
    //
    QWidget *basicTab = new QWidget(this);
    QVBoxLayout *basicTabLayout = new QVBoxLayout(basicTab);

    //
    // Directories
    //
    QGroupBox *directoryBox = new QGroupBox(i18n("Directories"), basicTab);
    QGridLayout *directoryBoxLayout = new QGridLayout(directoryBox);

    QLabel *mountPrefixLabel = new QLabel(Smb4KMountSettings::self()->mountPrefixItem()->label(), directoryBox);
    m_mountPrefix = new KUrlRequester(directoryBox);
    m_mountPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    m_mountPrefix->setObjectName(QStringLiteral("kcfg_MountPrefix"));

    mountPrefixLabel->setBuddy(m_mountPrefix);

    QCheckBox *lowercaseSubdirs = new QCheckBox(Smb4KMountSettings::self()->forceLowerCaseSubdirsItem()->label(), directoryBox);
    lowercaseSubdirs->setObjectName(QStringLiteral("kcfg_ForceLowerCaseSubdirs"));

    directoryBoxLayout->addWidget(mountPrefixLabel, 0, 0);
    directoryBoxLayout->addWidget(m_mountPrefix, 0, 1);
    directoryBoxLayout->addWidget(lowercaseSubdirs, 1, 0, 1, 2);

    basicTabLayout->addWidget(directoryBox, 0);

    //
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), this);
    QVBoxLayout *behaviorBoxLayout = new QVBoxLayout(behaviorBox);

    QCheckBox *remountShares = new QCheckBox(Smb4KMountSettings::self()->remountSharesItem()->label(), behaviorBox);
    remountShares->setObjectName(QStringLiteral("kcfg_RemountShares"));

    m_remountSettingsWidget = new QWidget(behaviorBox);
    QGridLayout *remountSettingsWidgetLayout = new QGridLayout(m_remountSettingsWidget);
    remountSettingsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *remountAttemptsLabel = new QLabel(Smb4KMountSettings::self()->remountAttemptsItem()->label(), m_remountSettingsWidget);
    remountAttemptsLabel->setObjectName(QStringLiteral("RemountAttemptsLabel"));
    remountAttemptsLabel->setIndent(25);

    QSpinBox *remountAttempts = new QSpinBox(m_remountSettingsWidget);
    remountAttempts->setObjectName(QStringLiteral("kcfg_RemountAttempts"));
    remountAttemptsLabel->setBuddy(remountAttempts);

    QLabel *remountIntervalLabel = new QLabel(Smb4KMountSettings::self()->remountIntervalItem()->label(), m_remountSettingsWidget);
    remountIntervalLabel->setObjectName(QStringLiteral("RemountIntervalLabel"));
    remountIntervalLabel->setIndent(25);

    QSpinBox *remountInterval = new QSpinBox(m_remountSettingsWidget);
    remountInterval->setObjectName(QStringLiteral("kcfg_RemountInterval"));
    remountInterval->setSuffix(i18n(" min"));
    remountIntervalLabel->setBuddy(remountInterval);

    remountSettingsWidgetLayout->addWidget(remountAttemptsLabel, 0, 0);
    remountSettingsWidgetLayout->addWidget(remountAttempts, 0, 1);
    remountSettingsWidgetLayout->addWidget(remountIntervalLabel, 1, 0);
    remountSettingsWidgetLayout->addWidget(remountInterval, 1, 1);

    QCheckBox *unmountAllShares = new QCheckBox(Smb4KMountSettings::self()->unmountSharesOnExitItem()->label(), behaviorBox);
    unmountAllShares->setObjectName(QStringLiteral("kcfg_UnmountSharesOnExit"));

    QCheckBox *unmountForeignShares = new QCheckBox(Smb4KMountSettings::self()->unmountForeignSharesItem()->label(), behaviorBox);
    unmountForeignShares->setObjectName(QStringLiteral("kcfg_UnmountForeignShares"));

    QCheckBox *unmountInaccessibleShares = new QCheckBox(Smb4KMountSettings::self()->forceUnmountInaccessibleItem()->label(), behaviorBox);
    unmountInaccessibleShares->setObjectName(QStringLiteral("kcfg_ForceUnmountInaccessible"));

    QCheckBox *detectAllShares = new QCheckBox(Smb4KMountSettings::self()->detectAllSharesItem()->label(), behaviorBox);
    detectAllShares->setObjectName(QStringLiteral("kcfg_DetectAllShares"));

    behaviorBoxLayout->addWidget(remountShares);
    behaviorBoxLayout->addWidget(m_remountSettingsWidget);
    behaviorBoxLayout->addWidget(unmountAllShares);
    behaviorBoxLayout->addWidget(unmountInaccessibleShares);
    behaviorBoxLayout->addWidget(unmountForeignShares);
    behaviorBoxLayout->addWidget(detectAllShares);

    basicTabLayout->addWidget(behaviorBox, 0);
    basicTabLayout->addStretch(100);

    addTab(basicTab, i18n("Basic Settings"));

    //
    // Common Mount Settings tab
    //
    QWidget *commonTab = new QWidget(this);
    QVBoxLayout *commonTabLayout = new QVBoxLayout(commonTab);

    //
    // Common options group box
    //
    QGroupBox *commonOptions = new QGroupBox(i18n("Common Options"), commonTab);
    QGridLayout *commonOptionsLayout = new QGridLayout(commonOptions);

    // Write access
    QCheckBox *useWriteAccess = new QCheckBox(Smb4KMountSettings::self()->useWriteAccessItem()->label(), commonOptions);
    useWriteAccess->setObjectName(QStringLiteral("kcfg_UseWriteAccess"));

    KComboBox *writeAccess = new KComboBox(commonOptions);
    writeAccess->setObjectName(QStringLiteral("kcfg_WriteAccess"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> writeAccessChoices = Smb4KMountSettings::self()->writeAccessItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &wa : qAsConst(writeAccessChoices)) {
        writeAccess->addItem(wa.label);
    }

    commonOptionsLayout->addWidget(useWriteAccess, 0, 0);
    commonOptionsLayout->addWidget(writeAccess, 0, 1);

    // Character set
    QCheckBox *useCharacterSet = new QCheckBox(Smb4KMountSettings::self()->useClientCharsetItem()->label(), commonOptions);
    useCharacterSet->setObjectName(QStringLiteral("kcfg_UseClientCharset"));

    KComboBox *characterSet = new KComboBox(commonOptions);
    characterSet->setObjectName(QStringLiteral("kcfg_ClientCharset"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> charsetChoices = Smb4KMountSettings::self()->clientCharsetItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(charsetChoices)) {
        characterSet->addItem(c.label);
    }

    commonOptionsLayout->addWidget(useCharacterSet, 1, 0);
    commonOptionsLayout->addWidget(characterSet, 1, 1);

    // Remote filesystem port
    QCheckBox *useFilesystemPort = new QCheckBox(Smb4KMountSettings::self()->useRemoteFileSystemPortItem()->label(), commonOptions);
    useFilesystemPort->setObjectName(QStringLiteral("kcfg_UseRemoteFileSystemPort"));

    QSpinBox *filesystemPort = new QSpinBox(commonOptions);
    filesystemPort->setObjectName(QStringLiteral("kcfg_RemoteFileSystemPort"));

    commonOptionsLayout->addWidget(useFilesystemPort, 2, 0);
    commonOptionsLayout->addWidget(filesystemPort, 2, 1);

    commonTabLayout->addWidget(commonOptions, 0);

    //
    // CIFS Unix Extensions Support group box
    //
    QGroupBox *cifsExtensionSupportBox = new QGroupBox(i18n("CIFS Unix Extensions Support"), commonTab);
    QGridLayout *cifsExtensionSupportLayout = new QGridLayout(cifsExtensionSupportBox);

    // CIFS Unix extensions support
    QCheckBox *cifsExtensionsSupport = new QCheckBox(Smb4KMountSettings::self()->cifsUnixExtensionsSupportItem()->label(), cifsExtensionSupportBox);
    cifsExtensionsSupport->setObjectName(QStringLiteral("kcfg_CifsUnixExtensionsSupport"));

    cifsExtensionSupportLayout->addWidget(cifsExtensionsSupport, 0, 0, 1, 2);

    // User information
    m_useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), cifsExtensionSupportBox);
    m_useUserId->setObjectName(QStringLiteral("kcfg_UseUserId"));

    m_userIdInputWidget = new QWidget(cifsExtensionSupportBox);
    m_userIdInputWidget->setObjectName(QStringLiteral("UserIdInputWidget"));

    QGridLayout *userLayout = new QGridLayout(m_userIdInputWidget);
    userLayout->setContentsMargins(0, 0, 0, 0);

    m_userId = new KLineEdit(m_userIdInputWidget);
    m_userId->setObjectName(QStringLiteral("kcfg_UserId"));
    m_userId->setAlignment(Qt::AlignRight);
    m_userId->setReadOnly(true);

    QToolButton *userChooser = new QToolButton(m_userIdInputWidget);
    userChooser->setIcon(KDE::icon(QStringLiteral("edit-find-user")));
    userChooser->setToolTip(i18n("Choose a different user"));
    userChooser->setPopupMode(QToolButton::InstantPopup);

    QMenu *userMenu = new QMenu(userChooser);
    userChooser->setMenu(userMenu);

    QList<KUser> allUsers = KUser::allUsers();

    for (const KUser &u : qAsConst(allUsers)) {
        QAction *userAction = userMenu->addAction(u.loginName() + QStringLiteral(" (") + u.userId().toString() + QStringLiteral(")"));
        userAction->setData(u.userId().nativeId());
    }

    userLayout->addWidget(m_userId, 0, 0);
    userLayout->addWidget(userChooser, 0, 1, Qt::AlignCenter);

    cifsExtensionSupportLayout->addWidget(m_useUserId, 1, 0);
    cifsExtensionSupportLayout->addWidget(m_userIdInputWidget, 1, 1);

    // Group information
    m_useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), cifsExtensionSupportBox);
    m_useGroupId->setObjectName(QStringLiteral("kcfg_UseGroupId"));

    m_groupIdInputWidget = new QWidget(cifsExtensionSupportBox);
    m_groupIdInputWidget->setObjectName(QStringLiteral("GroupIdInputWidget"));

    QGridLayout *groupLayout = new QGridLayout(m_groupIdInputWidget);
    groupLayout->setContentsMargins(0, 0, 0, 0);

    m_groupId = new KLineEdit(m_groupIdInputWidget);
    m_groupId->setObjectName(QStringLiteral("kcfg_GroupId"));
    m_groupId->setAlignment(Qt::AlignRight);
    m_groupId->setReadOnly(true);

    QToolButton *groupChooser = new QToolButton(m_groupIdInputWidget);
    groupChooser->setIcon(KDE::icon(QStringLiteral("edit-find-user")));
    groupChooser->setToolTip(i18n("Choose a different group"));
    groupChooser->setPopupMode(QToolButton::InstantPopup);

    QMenu *groupMenu = new QMenu(groupChooser);
    groupChooser->setMenu(groupMenu);

    QList<KUserGroup> groupList = KUserGroup::allGroups();

    for (const KUserGroup &g : qAsConst(groupList)) {
        QAction *groupAction = groupMenu->addAction(g.name() + QStringLiteral(" (") + g.groupId().toString() + QStringLiteral(")"));
        groupAction->setData(g.groupId().nativeId());
    }

    groupLayout->addWidget(m_groupId, 0, 0);
    groupLayout->addWidget(groupChooser, 0, 1, Qt::AlignCenter);

    cifsExtensionSupportLayout->addWidget(m_useGroupId, 2, 0);
    cifsExtensionSupportLayout->addWidget(m_groupIdInputWidget, 2, 1);

    // File mask
    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), cifsExtensionSupportBox);
    m_useFileMode->setObjectName(QStringLiteral("kcfg_UseFileMode"));

    m_fileMode = new KLineEdit(cifsExtensionSupportBox);
    m_fileMode->setObjectName(QStringLiteral("kcfg_FileMode"));
    m_fileMode->setClearButtonEnabled(true);
    m_fileMode->setAlignment(Qt::AlignRight);

    cifsExtensionSupportLayout->addWidget(m_useFileMode, 3, 0);
    cifsExtensionSupportLayout->addWidget(m_fileMode, 3, 1);

    // Directory mask
    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), cifsExtensionSupportBox);
    m_useDirectoryMode->setObjectName(QStringLiteral("kcfg_UseDirectoryMode"));

    m_directoryMode = new KLineEdit(cifsExtensionSupportBox);
    m_directoryMode->setObjectName(QStringLiteral("kcfg_DirectoryMode"));
    m_directoryMode->setClearButtonEnabled(true);
    m_directoryMode->setAlignment(Qt::AlignRight);

    cifsExtensionSupportLayout->addWidget(m_useDirectoryMode, 4, 0);
    cifsExtensionSupportLayout->addWidget(m_directoryMode, 4, 1);

    commonTabLayout->addWidget(cifsExtensionSupportBox, 1);
    commonTabLayout->addStretch(100);

    addTab(commonTab, i18n("Common Mount Settings"));

    //
    // Advanced Mount Settings tab
    //
    QWidget *advancedTab = new QWidget(this);
    QVBoxLayout *advancedTabLayout = new QVBoxLayout(advancedTab);

    QGroupBox *advancedOptions = new QGroupBox(i18n("Advanced Options"), advancedTab);
    QGridLayout *advancedOptionsLayout = new QGridLayout(advancedOptions);

    // Force Uid
    QCheckBox *forceUid = new QCheckBox(Smb4KMountSettings::self()->forceUIDItem()->label(), advancedOptions);
    forceUid->setObjectName(QStringLiteral("kcfg_ForceUID"));

    advancedOptionsLayout->addWidget(forceUid, 0, 0);

    // Force Gid
    QCheckBox *forceGid = new QCheckBox(Smb4KMountSettings::self()->forceGIDItem()->label(), advancedOptions);
    forceGid->setObjectName(QStringLiteral("kcfg_ForceGID"));

    advancedOptionsLayout->addWidget(forceGid, 0, 1);

    // Permission checks
    QCheckBox *permissionChecks = new QCheckBox(Smb4KMountSettings::self()->permissionChecksItem()->label(), advancedOptions);
    permissionChecks->setObjectName(QStringLiteral("kcfg_PermissionChecks"));

    advancedOptionsLayout->addWidget(permissionChecks, 1, 0);

    // Client controls Ids
    QCheckBox *clientControlsIds = new QCheckBox(Smb4KMountSettings::self()->clientControlsIDsItem()->label(), advancedOptions);
    clientControlsIds->setObjectName(QStringLiteral("kcfg_ClientControlsIDs"));

    advancedOptionsLayout->addWidget(clientControlsIds, 1, 1);

    // Use server inode numbers
    QCheckBox *useServerInodes = new QCheckBox(Smb4KMountSettings::self()->serverInodeNumbersItem()->label(), advancedOptions);
    useServerInodes->setObjectName(QStringLiteral("kcfg_ServerInodeNumbers"));

    advancedOptionsLayout->addWidget(useServerInodes, 2, 0);

    // Translate reserved characters
    QCheckBox *translateReservedCharacters = new QCheckBox(Smb4KMountSettings::self()->translateReservedCharsItem()->label(), advancedOptions);
    translateReservedCharacters->setObjectName(QStringLiteral("kcfg_TranslateReservedChars"));

    advancedOptionsLayout->addWidget(translateReservedCharacters, 2, 1);

    // No locking
    QCheckBox *no_locking = new QCheckBox(Smb4KMountSettings::self()->noLockingItem()->label(), advancedOptions);
    no_locking->setObjectName(QStringLiteral("kcfg_NoLocking"));

    advancedOptionsLayout->addWidget(no_locking, 3, 0);

    // Extra widget for the rest of the options
    QWidget *advancedOptionsExtraWidget = new QWidget(advancedOptions);
    QGridLayout *advancedOptionsExtraWidgetLayout = new QGridLayout(advancedOptionsExtraWidget);
    advancedOptionsExtraWidgetLayout->setContentsMargins(0, 0, 0, 0);

    // SMB protocol version
    QCheckBox *useSmbProtocol = new QCheckBox(Smb4KMountSettings::self()->useSmbProtocolVersionItem()->label(), advancedOptionsExtraWidget);
    useSmbProtocol->setObjectName(QStringLiteral("kcfg_UseSmbProtocolVersion"));

    KComboBox *smbProtocol = new KComboBox(advancedOptionsExtraWidget);
    smbProtocol->setObjectName(QStringLiteral("kcfg_SmbProtocolVersion"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> smbProtocolChoices = Smb4KMountSettings::self()->smbProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(smbProtocolChoices)) {
        smbProtocol->addItem(c.label);
    }

    advancedOptionsExtraWidgetLayout->addWidget(useSmbProtocol, 0, 0);
    advancedOptionsExtraWidgetLayout->addWidget(smbProtocol, 0, 1);

    // Cache mode
    QCheckBox *useCacheMode = new QCheckBox(Smb4KMountSettings::self()->useCacheModeItem()->label(), advancedOptionsExtraWidget);
    useCacheMode->setObjectName(QStringLiteral("kcfg_UseCacheMode"));

    KComboBox *cacheMode = new KComboBox(advancedOptionsExtraWidget);
    cacheMode->setObjectName(QStringLiteral("kcfg_CacheMode"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> cacheModeChoices = Smb4KMountSettings::self()->cacheModeItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(cacheModeChoices)) {
        cacheMode->addItem(c.label);
    }

    advancedOptionsExtraWidgetLayout->addWidget(useCacheMode, 1, 0);
    advancedOptionsExtraWidgetLayout->addWidget(cacheMode, 1, 1);

    // Security mode
    QCheckBox *useSecurityMode = new QCheckBox(Smb4KMountSettings::self()->useSecurityModeItem()->label(), advancedOptionsExtraWidget);
    useSecurityMode->setObjectName(QStringLiteral("kcfg_UseSecurityMode"));

    KComboBox *securityMode = new KComboBox(advancedOptionsExtraWidget);
    securityMode->setObjectName(QStringLiteral("kcfg_SecurityMode"));

    QList<KConfigSkeleton::ItemEnum::Choice> securityModeChoices = Smb4KMountSettings::self()->securityModeItem()->choices();

    for (const KConfigSkeleton::ItemEnum::Choice &c : qAsConst(securityModeChoices)) {
        securityMode->addItem(c.label);
    }

    advancedOptionsExtraWidgetLayout->addWidget(useSecurityMode, 2, 0);
    advancedOptionsExtraWidgetLayout->addWidget(securityMode, 2, 1);

    // Additional options
    QCheckBox *useAdditionalCifsOptions = new QCheckBox(Smb4KMountSettings::self()->useCustomCifsOptionsItem()->label(), advancedOptionsExtraWidget);
    useAdditionalCifsOptions->setObjectName(QStringLiteral("kcfg_UseCustomCifsOptions"));

    QWidget *additionalOptionsWidget = new QWidget(advancedOptionsExtraWidget);
    QHBoxLayout *additionalOptionsWidgetLayout = new QHBoxLayout(additionalOptionsWidget);
    additionalOptionsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_additionalCifsOptions = new KLineEdit(additionalOptionsWidget);
    m_additionalCifsOptions->setObjectName(QStringLiteral("kcfg_CustomCIFSOptions"));
    m_additionalCifsOptions->setReadOnly(true);
    m_additionalCifsOptions->setClearButtonEnabled(true);

    QToolButton *additionalOptionsEdit = new QToolButton(advancedOptionsExtraWidget);
    additionalOptionsEdit->setIcon(KDE::icon(QStringLiteral("document-edit")));
    additionalOptionsEdit->setToolTip(i18n("Edit the additional CIFS options."));

    additionalOptionsWidgetLayout->addWidget(m_additionalCifsOptions, 0);
    additionalOptionsWidgetLayout->addWidget(additionalOptionsEdit, 0);

    advancedOptionsExtraWidgetLayout->addWidget(useAdditionalCifsOptions, 3, 0);
    advancedOptionsExtraWidgetLayout->addWidget(additionalOptionsWidget, 3, 1);

    advancedOptionsLayout->addWidget(advancedOptionsExtraWidget, 4, 0, 1, 2);

    advancedTabLayout->addWidget(advancedOptions, 0);
    advancedTabLayout->addStretch(100);

    addTab(advancedTab, i18n("Advanced Mount Settings"));

    //
    // Adjust widgets
    //
    slotCIFSUnixExtensionsSupport(Smb4KMountSettings::cifsUnixExtensionsSupport());
    slotRemountSharesToggled(Smb4KMountSettings::remountShares());

    //
    // Connections
    //
    connect(userMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotNewUserTriggered(QAction *)));
    connect(groupMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotNewGroupTriggered(QAction *)));
    connect(cifsExtensionsSupport, SIGNAL(toggled(bool)), this, SLOT(slotCIFSUnixExtensionsSupport(bool)));
    connect(additionalOptionsEdit, SIGNAL(clicked(bool)), this, SLOT(slotAdditionalCIFSOptions()));
    connect(remountShares, SIGNAL(toggled(bool)), this, SLOT(slotRemountSharesToggled(bool)));
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD
//
void Smb4KConfigPageMounting::setupWidget()
{
    //
    // Basic Settings tab
    //
    QWidget *basicTab = new QWidget(this);
    QVBoxLayout *basicTabLayout = new QVBoxLayout(basicTab);

    //
    // Directories
    //
    QGroupBox *directoryBox = new QGroupBox(i18n("Directories"), basicTab);
    QGridLayout *directoryBoxLayout = new QGridLayout(directoryBox);

    QLabel *mountPrefixLabel = new QLabel(Smb4KMountSettings::self()->mountPrefixItem()->label(), directoryBox);
    m_mountPrefix = new KUrlRequester(directoryBox);
    m_mountPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    m_mountPrefix->setObjectName(QStringLiteral("kcfg_MountPrefix"));

    mountPrefixLabel->setBuddy(m_mountPrefix);

    QCheckBox *lowercaseSubdirs = new QCheckBox(Smb4KMountSettings::self()->forceLowerCaseSubdirsItem()->label(), directoryBox);
    lowercaseSubdirs->setObjectName(QStringLiteral("kcfg_ForceLowerCaseSubdirs"));

    directoryBoxLayout->addWidget(prefixLabel, 0, 0);
    directoryBoxLayout->addWidget(prefix, 0, 1);
    directoryBoxLayout->addWidget(lowercaseSubdirs, 1, 0, 1, 2);

    basicTabLayout->addWidget(directoryBox, 0);

    //
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), this);
    QVBoxLayout *behaviorBoxLayout = new QVBoxLayout(behaviorBox);

    QCheckBox *remountShares = new QCheckBox(Smb4KMountSettings::self()->remountSharesItem()->label(), behaviorBox);
    remountShares->setObjectName(QStringLiteral("kcfg_RemountShares"));

    m_remountSettingsWidget = new QWidget(behaviorBox);
    QGridLayout *remountSettingsWidgetLayout = new QGridLayout(m_remountSettingsWidget);
    remountSettingsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *remountAttemptsLabel = new QLabel(Smb4KMountSettings::self()->remountAttemptsItem()->label(), m_remountSettingsWidget);
    remountAttemptsLabel->setObjectName(QStringLiteral("RemountAttemptsLabel"));
    remountAttemptsLabel->setIndent(25);

    QSpinBox *remountAttempts = new QSpinBox(m_remountSettingsWidget);
    remountAttempts->setObjectName(QStringLiteral("kcfg_RemountAttempts"));
    remountAttemptsLabel->setBuddy(remountAttempts);

    QLabel *remountIntervalLabel = new QLabel(Smb4KMountSettings::self()->remountIntervalItem()->label(), m_remountSettingsWidget);
    remountIntervalLabel->setObjectName(QStringLiteral("RemountIntervalLabel"));
    remountIntervalLabel->setIndent(25);

    QSpinBox *remountInterval = new QSpinBox(m_remountSettingsWidget);
    remountInterval->setObjectName(QStringLiteral("kcfg_RemountInterval"));
    remountInterval->setSuffix(i18n(" min"));
    remountIntervalLabel->setBuddy(remountInterval);

    remountSettingsWidgetLayout->addWidget(remountAttemptsLabel, 0, 0);
    remountSettingsWidgetLayout->addWidget(remountAttempts, 0, 1);
    remountSettingsWidgetLayout->addWidget(remountIntervalLabel, 1, 0);
    remountSettingsWidgetLayout->addWidget(remountInterval, 1, 1);

    QCheckBox *unmountAllShares = new QCheckBox(Smb4KMountSettings::self()->unmountSharesOnExitItem()->label(), behaviorBox);
    unmountAllShares->setObjectName(QStringLiteral("kcfg_UnmountSharesOnExit"));

    QCheckBox *unmountForeignShares = new QCheckBox(Smb4KMountSettings::self()->unmountForeignSharesItem()->label(), behaviorBox);
    unmountForeignShares->setObjectName(QStringLiteral("kcfg_UnmountForeignShares"));

    QCheckBox *detectAllShares = new QCheckBox(Smb4KMountSettings::self()->detectAllSharesItem()->label(), behaviorBox);
    detectAllShares->setObjectName(QStringLiteral("kcfg_DetectAllShares"));

    behaviorBoxLayout->addWidget(remountShares);
    behaviorBoxLayout->addWidget(m_remountSettingsWidget);
    behaviorBoxLayout->addWidget(unmountAllShares);
    behaviorBoxLayout->addWidget(unmountForeignShares);
    behaviorBoxLayout->addWidget(detectAllShares);

    basicTabLayout->addWidget(behaviorBox, 0);
    basicTabLayout->addStretch(100);

    addTab(basicTab, i18n("Basic Settings"));

    //
    // Mount Settings tab
    //
    QWidget *mountTab = new QWidget(this);
    QVBoxLayout *mountTabLayout = new QVBoxLayout(mountTab);

    //
    // Common Options
    //
    QGroupBox *commonOptionsBox = new QGroupBox(i18n("Common Options"), mountTab);
    QGridLayout *commonOptionsBoxLayout = new QGridLayout(commonOptionsBox);

    // User information
    QCheckBox *useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), commonOptionsBox);
    useUserId->setObjectName(QStringLiteral("kcfg_UseUserId"));

    QWidget *userIdInputWidget = new QWidget(commonOptionsBox);
    userIdInputWidget->setObjectName(QStringLiteral("UserIdInputWidget"));

    QGridLayout *userLayout = new QGridLayout(userIdInputWidget);
    userLayout->setContentsMargins(0, 0, 0, 0);

    m_userId = new KLineEdit(userIdInputWidget);
    m_userId->setObjectName(QStringLiteral("kcfg_UserId"));
    m_userId->setAlignment(Qt::AlignRight);
    m_userId->setReadOnly(true);

    QToolButton *userChooser = new QToolButton(userIdInputWidget);
    userChooser->setIcon(KDE::icon(QStringLiteral("edit-find-user")));
    userChooser->setToolTip(i18n("Choose a different user"));
    userChooser->setPopupMode(QToolButton::InstantPopup);

    QMenu *userMenu = new QMenu(userChooser);
    userChooser->setMenu(userMenu);

    QList<KUser> allUsers = KUser::allUsers();

    for (const KUser &u : allUsers) {
        QAction *userAction = userMenu->addAction(u.loginName() + QStringLiteral(" (") + u.userId().toString() + QStringLiteral(")"));
        userAction->setData(u.userId().nativeId());
    }

    userLayout->addWidget(m_userId, 0, 0);
    userLayout->addWidget(userChooser, 0, 1, Qt::AlignCenter);

    commonOptionsBoxLayout->addWidget(useUserId, 0, 0);
    commonOptionsBoxLayout->addWidget(userIdInputWidget, 0, 1);

    // Group information
    QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), commonOptionsBox);
    useGroupId->setObjectName(QStringLiteral("kcfg_UseGroupId"));

    QWidget *groupIdInputWidget = new QWidget(commonOptionsBox);
    groupIdInputWidget->setObjectName(QStringLiteral("GroupIdInputWidget"));

    QGridLayout *groupLayout = new QGridLayout(groupIdInputWidget);
    groupLayout->setContentsMargins(0, 0, 0, 0);

    m_groupId = new KLineEdit(groupIdInputWidget);
    m_groupId->setObjectName(QStringLiteral("kcfg_GroupId"));
    m_groupId->setAlignment(Qt::AlignRight);
    m_groupId->setReadOnly(true);

    QToolButton *groupChooser = new QToolButton(groupIdInputWidget);
    groupChooser->setIcon(KDE::icon(QStringLiteral("edit-find-user")));
    groupChooser->setToolTip(i18n("Choose a different group"));
    groupChooser->setPopupMode(QToolButton::InstantPopup);

    QMenu *groupMenu = new QMenu(groupChooser);
    groupChooser->setMenu(groupMenu);

    QList<KUserGroup> groupList = KUserGroup::allGroups();

    for (const KUserGroup &g : groupList) {
        QAction *groupAction = groupMenu->addAction(g.name() + QStringLiteral(" (") + g.groupId().toString() + QStringLiteral(")"));
        groupAction->setData(g.groupId().nativeId());
    }

    groupLayout->addWidget(m_groupId, 0, 0);
    groupLayout->addWidget(groupChooser, 0, 1, Qt::AlignCenter);

    commonOptionsBoxLayout->addWidget(useGroupId, 1, 0);
    commonOptionsBoxLayout->addWidget(groupIdInputWidget, 1, 1);

    // File mask
    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), commonOptionsBox);
    m_useFileMode->setObjectName(QStringLiteral("kcfg_UseFileMode"));

    m_fileMode = new KLineEdit(commonOptionsBox);
    m_fileMode->setObjectName(QStringLiteral("kcfg_FileMode"));
    m_fileMode->setClearButtonEnabled(true);
    m_fileMode->setAlignment(Qt::AlignRight);

    commonOptionsBoxLayout->addWidget(m_useFileMode, 2, 0);
    commonOptionsBoxLayout->addWidget(m_fileMode, 2, 1);

    // Directory mask
    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), commonOptionsBox);
    m_useDirectoryMode->setObjectName(QStringLiteral("kcfg_UseDirectoryMode"));

    m_directoryMode = new KLineEdit(commonOptionsBox);
    m_directoryMode->setObjectName(QStringLiteral("kcfg_DirectoryMode"));
    m_directoryMode->setClearButtonEnabled(true);
    m_directoryMode->setAlignment(Qt::AlignRight);

    commonOptionsBoxLayout->addWidget(m_useDirectoryMode, 3, 0);
    commonOptionsBoxLayout->addWidget(m_directoryMode, 3, 1);

    //
    // Character sets
    //
    QGroupBox *characterSetsBox = new QGroupBox(i18n("Character Sets"), mountTab);
    QVBoxLayout *characterSetsBoxLayout = new QVBoxLayout(characterSetsBox);

    // Client character set
    QCheckBox *useCharacterSets = new QCheckBox(Smb4KMountSettings::self()->useCharacterSetsItem()->label(), characterSetsBox);
    useCharacterSets->setObjectName(QStringLiteral("kcfg_UseCharacterSets"));

    m_characterSetSettingsWigdet = new QWidget(characterSetsBox);
    QGridLayout *characterSetSettingsWidgetLayout = new QGridLayout(m_characterSetSettingsWigdet);
    characterSetSettingsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *clientCharacterSetLabel = new QLabel(Smb4KMountSettings::self()->clientCharsetItem()->label(), m_characterSetSettingsWigdet);
    clientCharacterSetLabel->setIndent(25);

    KComboBox *clientCharacterSet = new KComboBox(m_characterSetSettingsWigdet);
    clientCharacterSet->setObjectName(QStringLiteral("kcfg_ClientCharset"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> charsetChoices = Smb4KMountSettings::self()->clientCharsetItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : charsetChoices) {
        clientCharacterSet->addItem(c.label);
    }

    clientCharacterSetLabel->setBuddy(clientCharacterSet);

    // Server character set
    QLabel *serverCharacterSetLabel = new QLabel(Smb4KMountSettings::self()->serverCodepageItem()->label(), m_characterSetSettingsWigdet);
    serverCharacterSetLabel->setIndent(25);
    serverCharacterSetLabel->setObjectName(QStringLiteral("ServerCodepageLabel"));

    KComboBox *serverCharacterSet = new KComboBox(m_characterSetSettingsWigdet);
    serverCharacterSet->setObjectName(QStringLiteral("kcfg_ServerCodepage"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> codepageChoices = Smb4KMountSettings::self()->serverCodepageItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : codepageChoices) {
        serverCharacterSet->addItem(c.label);
    }

    serverCharacterSetLabel->setBuddy(serverCharacterSet);

    characterSetSettingsWidgetLayout->addWidget(clientCharacterSetLabel, 0, 0);
    characterSetSettingsWidgetLayout->addWidget(clientCharacterSet, 0, 1);
    characterSetSettingsWidgetLayout->addWidget(serverCharacterSetLabel, 1, 0);
    characterSetSettingsWidgetLayout->addWidget(serverCharacterSet, 1, 1);

    characterSetsBoxLayout->addWidget(useCharacterSets);
    characterSetsBoxLayout->addWidget(m_characterSetSettingsWigdet);

    mountTabLayout->addWidget(commonOptionsBox, 0);
    mountTabLayout->addWidget(characterSetsBox, 0);
    mountTabLayout->addStretch(100);

    addTab(mountTab, i18n("Mount Settings"));

    //
    // Connections
    //
    connect(userMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotNewUserTriggered(QAction *)));
    connect(groupMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotNewGroupTriggered(QAction *)));
    connect(useCharacterSets, SIGNAL(toggled(bool)), this, SLOT(slotCharacterSets(bool)));
    connect(remountShares, SIGNAL(toggled(bool)), this, SLOT(slotRemountSharesToggled(bool)));

    //
    // Enable / disable widgets
    //
    slotCharacterSets(Smb4KMountSettings::useCharacterSets());
    slotRemountSharesToggled(Smb4KMountSettings::remountShares());
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
    m_userId->setText(action->data().toString());
}

void Smb4KConfigPageMounting::slotNewGroupTriggered(QAction *action)
{
    m_groupId->setText(action->data().toString());
}

#if defined(Q_OS_LINUX)
void Smb4KConfigPageMounting::slotCIFSUnixExtensionsSupport(bool checked)
{
    m_useUserId->setEnabled(!checked);
    m_userIdInputWidget->setEnabled(!checked);

    m_useGroupId->setEnabled(!checked);
    m_groupIdInputWidget->setEnabled(!checked);

    m_useFileMode->setEnabled(!checked);
    m_fileMode->setEnabled(!checked);

    m_useDirectoryMode->setEnabled(!checked);
    m_directoryMode->setEnabled(!checked);
}

void Smb4KConfigPageMounting::slotAdditionalCIFSOptions()
{
    QString options = m_additionalCifsOptions->originalText();

    bool ok = false;
    options = QInputDialog::getText(this,
                                    i18n("Additional CIFS Options"),
                                    i18n("<qt>Enter the desired options as a comma separated list:</qt>"),
                                    QLineEdit::Normal,
                                    options,
                                    &ok);

    if (ok) {
        if (!options.trimmed().isEmpty()) {
            // SECURITY: Only pass those arguments to mount.cifs that do not pose
            // a potential security risk and that have not already been defined.
            //
            // This is, among others, the proper fix to the security issue reported
            // by Heiner Markert (aka CVE-2014-2581).
            QStringList allowedArgs = allowedMountArguments();
            QStringList deniedArgs;
            QStringList list = options.split(QStringLiteral(","), Qt::SkipEmptyParts);
            QMutableStringListIterator it(list);

            while (it.hasNext()) {
                QString arg = it.next().section(QStringLiteral("="), 0, 0);

                if (!allowedArgs.contains(arg)) {
                    deniedArgs << arg;
                    it.remove();
                }
            }

            if (!deniedArgs.isEmpty()) {
                QString msg = i18np(
                    "The following entry is going to be removed from the additional options:<br>%2.<br>Please read the handbook for details.",
                    "The following %1 entries are going to be removed from the additional options:<br>%2.<br>Please read the handbook for details.",
                    deniedArgs.size(),
                    deniedArgs.join(QStringLiteral(", ")));
                KMessageBox::information(this, msg);
            }

            m_additionalCifsOptions->setText(list.join(QStringLiteral(",")).trimmed());
        } else {
            m_additionalCifsOptions->clear();
        }
    }
}
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
void Smb4KConfigPageMounting::slotCharacterSets(bool on)
{
    m_charsetSettingsWigdet->setEnabled(on);
}
#endif

void Smb4KConfigPageMounting::slotRemountSharesToggled(bool on)
{
    m_remountSettingsWidget->setEnabled(on);
}
