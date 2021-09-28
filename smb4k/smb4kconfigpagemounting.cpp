/*
    The configuration page for the mount options
    -------------------
    begin                : So Mär 22 2015
    SPDX-FileCopyrightText: 2015-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
#include "smb4kconfigpagemounting.h"
#include "core/smb4kglobal.h"

#if defined(Q_OS_LINUX)
#include "core/smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "core/smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QCheckBox>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes
#include <KCompletion/KComboBox>
#include <KCompletion/KLineEdit>
#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlRequester>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KMessageBox>

using namespace Smb4KGlobal;

Smb4KConfigPageMounting::Smb4KConfigPageMounting(QWidget *parent)
    : QTabWidget(parent)
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
    // Basic Settings tab
    //
    QWidget *basicTab = new QWidget(this);
    QVBoxLayout *basicTabLayout = new QVBoxLayout(basicTab);

    //
    // Directories
    //
    QGroupBox *directoryBox = new QGroupBox(i18n("Directories"), basicTab);
    QGridLayout *directoryBoxLayout = new QGridLayout(directoryBox);

    QLabel *prefixLabel = new QLabel(Smb4KMountSettings::self()->mountPrefixItem()->label(), directoryBox);
    KUrlRequester *prefix = new KUrlRequester(directoryBox);
    prefix->setMode(KFile::Directory | KFile::LocalOnly);
    prefix->setObjectName("kcfg_MountPrefix");

    prefixLabel->setBuddy(prefix);

    QCheckBox *lowercaseSubdirs = new QCheckBox(Smb4KMountSettings::self()->forceLowerCaseSubdirsItem()->label(), directoryBox);
    lowercaseSubdirs->setObjectName("kcfg_ForceLowerCaseSubdirs");

    directoryBoxLayout->addWidget(prefixLabel, 0, 0);
    directoryBoxLayout->addWidget(prefix, 0, 1);
    directoryBoxLayout->addWidget(lowercaseSubdirs, 1, 0, 1, 2);

    basicTabLayout->addWidget(directoryBox, 0);

    //
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), this);
    QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);

    QCheckBox *remountShares = new QCheckBox(Smb4KMountSettings::self()->remountSharesItem()->label(), behaviorBox);
    remountShares->setObjectName("kcfg_RemountShares");

    QLabel *remountAttemptsLabel = new QLabel(Smb4KMountSettings::self()->remountAttemptsItem()->label(), behaviorBox);
    remountAttemptsLabel->setObjectName("RemountAttemptsLabel");
    remountAttemptsLabel->setIndent(25);

    QSpinBox *remountAttempts = new QSpinBox(behaviorBox);
    remountAttempts->setObjectName("kcfg_RemountAttempts");
    remountAttemptsLabel->setBuddy(remountAttempts);

    QLabel *remountIntervalLabel = new QLabel(Smb4KMountSettings::self()->remountIntervalItem()->label(), behaviorBox);
    remountIntervalLabel->setObjectName("RemountIntervalLabel");
    remountIntervalLabel->setIndent(25);

    QSpinBox *remountInterval = new QSpinBox(behaviorBox);
    remountInterval->setObjectName("kcfg_RemountInterval");
    remountInterval->setSuffix(i18n(" min"));
    remountIntervalLabel->setBuddy(remountInterval);

    QCheckBox *unmountAllShares = new QCheckBox(Smb4KMountSettings::self()->unmountSharesOnExitItem()->label(), behaviorBox);
    unmountAllShares->setObjectName("kcfg_UnmountSharesOnExit");

    QCheckBox *unmountForeignShares = new QCheckBox(Smb4KMountSettings::self()->unmountForeignSharesItem()->label(), behaviorBox);
    unmountForeignShares->setObjectName("kcfg_UnmountForeignShares");

    QCheckBox *unmountInaccessibleShares = new QCheckBox(Smb4KMountSettings::self()->forceUnmountInaccessibleItem()->label(), behaviorBox);
    unmountInaccessibleShares->setObjectName("kcfg_ForceUnmountInaccessible");

    QCheckBox *detectAllShares = new QCheckBox(Smb4KMountSettings::self()->detectAllSharesItem()->label(), behaviorBox);
    detectAllShares->setObjectName("kcfg_DetectAllShares");

    behaviorBoxLayout->addWidget(remountShares, 0, 0, 1, 2);
    behaviorBoxLayout->addWidget(remountAttemptsLabel, 1, 0);
    behaviorBoxLayout->addWidget(remountAttempts, 1, 1);
    behaviorBoxLayout->addWidget(remountIntervalLabel, 2, 0);
    behaviorBoxLayout->addWidget(remountInterval, 2, 1);
    behaviorBoxLayout->addWidget(unmountAllShares, 3, 0, 1, 2);
    behaviorBoxLayout->addWidget(unmountInaccessibleShares, 4, 0, 1, 2);
    behaviorBoxLayout->addWidget(unmountForeignShares, 5, 0, 1, 2);
    behaviorBoxLayout->addWidget(detectAllShares, 6, 0, 1, 2);

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
    useWriteAccess->setObjectName("kcfg_UseWriteAccess");

    KComboBox *writeAccess = new KComboBox(commonOptions);
    writeAccess->setObjectName("kcfg_WriteAccess");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> writeAccessChoices = Smb4KMountSettings::self()->writeAccessItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &wa : qAsConst(writeAccessChoices)) {
        writeAccess->addItem(wa.label);
    }

    commonOptionsLayout->addWidget(useWriteAccess, 0, 0);
    commonOptionsLayout->addWidget(writeAccess, 0, 1);

    // Character set
    QCheckBox *useCharacterSet = new QCheckBox(Smb4KMountSettings::self()->useClientCharsetItem()->label(), commonOptions);
    useCharacterSet->setObjectName("kcfg_UseClientCharset");

    KComboBox *characterSet = new KComboBox(commonOptions);
    characterSet->setObjectName("kcfg_ClientCharset");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> charsetChoices = Smb4KMountSettings::self()->clientCharsetItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(charsetChoices)) {
        characterSet->addItem(c.label);
    }

    commonOptionsLayout->addWidget(useCharacterSet, 1, 0);
    commonOptionsLayout->addWidget(characterSet, 1, 1);

    // Remote filesystem port
    QCheckBox *useFilesystemPort = new QCheckBox(Smb4KMountSettings::self()->useRemoteFileSystemPortItem()->label(), commonOptions);
    useFilesystemPort->setObjectName("kcfg_UseRemoteFileSystemPort");

    QSpinBox *filesystemPort = new QSpinBox(commonOptions);
    filesystemPort->setObjectName("kcfg_RemoteFileSystemPort");

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
    cifsExtensionsSupport->setObjectName("kcfg_CifsUnixExtensionsSupport");

    cifsExtensionSupportLayout->addWidget(cifsExtensionsSupport, 0, 0, 1, 2);

    // User information
    QCheckBox *useUserId = new QCheckBox(Smb4KMountSettings::self()->useUserIdItem()->label(), cifsExtensionSupportBox);
    useUserId->setObjectName("kcfg_UseUserId");

    QWidget *userIdInputWidget = new QWidget(cifsExtensionSupportBox);
    userIdInputWidget->setObjectName("UserIdInputWidget");

    QGridLayout *userLayout = new QGridLayout(userIdInputWidget);
    userLayout->setContentsMargins(0, 0, 0, 0);

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

    for (const KUser &u : qAsConst(allUsers)) {
        QAction *userAction = userMenu->addAction(QString("%1 (%2)").arg(u.loginName()).arg(u.userId().nativeId()));
        userAction->setData(u.userId().nativeId());
    }

    userLayout->addWidget(userId, 0, 0);
    userLayout->addWidget(userChooser, 0, 1, Qt::AlignCenter);

    cifsExtensionSupportLayout->addWidget(useUserId, 1, 0);
    cifsExtensionSupportLayout->addWidget(userIdInputWidget, 1, 1);

    // Group information
    QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), cifsExtensionSupportBox);
    useGroupId->setObjectName("kcfg_UseGroupId");

    QWidget *groupIdInputWidget = new QWidget(cifsExtensionSupportBox);
    groupIdInputWidget->setObjectName("GroupIdInputWidget");

    QGridLayout *groupLayout = new QGridLayout(groupIdInputWidget);
    groupLayout->setContentsMargins(0, 0, 0, 0);

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

    for (const KUserGroup &g : qAsConst(groupList)) {
        QAction *groupAction = groupMenu->addAction(QString("%1 (%2)").arg(g.name()).arg(g.groupId().nativeId()));
        groupAction->setData(g.groupId().nativeId());
    }

    groupLayout->addWidget(groupId, 0, 0);
    groupLayout->addWidget(groupChooser, 0, 1, Qt::AlignCenter);

    cifsExtensionSupportLayout->addWidget(useGroupId, 2, 0);
    cifsExtensionSupportLayout->addWidget(groupIdInputWidget, 2, 1);

    // File mask
    QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), cifsExtensionSupportBox);
    useFileMode->setObjectName("kcfg_UseFileMode");

    KLineEdit *fileMode = new KLineEdit(cifsExtensionSupportBox);
    fileMode->setObjectName("kcfg_FileMode");
    fileMode->setClearButtonEnabled(true);
    fileMode->setAlignment(Qt::AlignRight);

    cifsExtensionSupportLayout->addWidget(useFileMode, 3, 0);
    cifsExtensionSupportLayout->addWidget(fileMode, 3, 1);

    // Directory mask
    QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), cifsExtensionSupportBox);
    useDirectoryMode->setObjectName("kcfg_UseDirectoryMode");

    KLineEdit *directoryMode = new KLineEdit(cifsExtensionSupportBox);
    directoryMode->setObjectName("kcfg_DirectoryMode");
    directoryMode->setClearButtonEnabled(true);
    directoryMode->setAlignment(Qt::AlignRight);

    cifsExtensionSupportLayout->addWidget(useDirectoryMode, 4, 0);
    cifsExtensionSupportLayout->addWidget(directoryMode, 4, 1);

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
    forceUid->setObjectName("kcfg_ForceUID");

    advancedOptionsLayout->addWidget(forceUid, 0, 0);

    // Force Gid
    QCheckBox *forceGid = new QCheckBox(Smb4KMountSettings::self()->forceGIDItem()->label(), advancedOptions);
    forceGid->setObjectName("kcfg_ForceGID");

    advancedOptionsLayout->addWidget(forceGid, 0, 1);

    // Permission checks
    QCheckBox *permissionChecks = new QCheckBox(Smb4KMountSettings::self()->permissionChecksItem()->label(), advancedOptions);
    permissionChecks->setObjectName("kcfg_PermissionChecks");

    advancedOptionsLayout->addWidget(permissionChecks, 1, 0);

    // Client controls Ids
    QCheckBox *clientControlsIds = new QCheckBox(Smb4KMountSettings::self()->clientControlsIDsItem()->label(), advancedOptions);
    clientControlsIds->setObjectName("kcfg_ClientControlsIDs");

    advancedOptionsLayout->addWidget(clientControlsIds, 1, 1);

    // Use server inode numbers
    QCheckBox *useServerInodes = new QCheckBox(Smb4KMountSettings::self()->serverInodeNumbersItem()->label(), advancedOptions);
    useServerInodes->setObjectName("kcfg_ServerInodeNumbers");

    advancedOptionsLayout->addWidget(useServerInodes, 2, 0);

    // Translate reserved characters
    QCheckBox *translateReservedCharacters = new QCheckBox(Smb4KMountSettings::self()->translateReservedCharsItem()->label(), advancedOptions);
    translateReservedCharacters->setObjectName("kcfg_TranslateReservedChars");

    advancedOptionsLayout->addWidget(translateReservedCharacters, 2, 1);

    // No locking
    QCheckBox *no_locking = new QCheckBox(Smb4KMountSettings::self()->noLockingItem()->label(), advancedOptions);
    no_locking->setObjectName("kcfg_NoLocking");

    advancedOptionsLayout->addWidget(no_locking, 3, 0);

    // Extra widget for the rest of the options
    QWidget *advancedOptionsExtraWidget = new QWidget(advancedOptions);
    QGridLayout *advancedOptionsExtraWidgetLayout = new QGridLayout(advancedOptionsExtraWidget);
    advancedOptionsExtraWidgetLayout->setContentsMargins(0, 0, 0, 0);

    // SMB protocol version
    QCheckBox *useSmbProtocol = new QCheckBox(Smb4KMountSettings::self()->useSmbProtocolVersionItem()->label(), advancedOptionsExtraWidget);
    useSmbProtocol->setObjectName("kcfg_UseSmbProtocolVersion");

    KComboBox *smbProtocol = new KComboBox(advancedOptionsExtraWidget);
    smbProtocol->setObjectName("kcfg_SmbProtocolVersion");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> smbProtocolChoices = Smb4KMountSettings::self()->smbProtocolVersionItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(smbProtocolChoices)) {
        smbProtocol->addItem(c.label);
    }

    advancedOptionsExtraWidgetLayout->addWidget(useSmbProtocol, 0, 0);
    advancedOptionsExtraWidgetLayout->addWidget(smbProtocol, 0, 1);

    // Cache mode
    QCheckBox *useCacheMode = new QCheckBox(Smb4KMountSettings::self()->useCacheModeItem()->label(), advancedOptionsExtraWidget);
    useCacheMode->setObjectName("kcfg_UseCacheMode");

    KComboBox *cacheMode = new KComboBox(advancedOptionsExtraWidget);
    cacheMode->setObjectName("kcfg_CacheMode");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> cacheModeChoices = Smb4KMountSettings::self()->cacheModeItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : qAsConst(cacheModeChoices)) {
        cacheMode->addItem(c.label);
    }

    advancedOptionsExtraWidgetLayout->addWidget(useCacheMode, 1, 0);
    advancedOptionsExtraWidgetLayout->addWidget(cacheMode, 1, 1);

    // Security mode
    QCheckBox *useSecurityMode = new QCheckBox(Smb4KMountSettings::self()->useSecurityModeItem()->label(), advancedOptionsExtraWidget);
    useSecurityMode->setObjectName("kcfg_UseSecurityMode");

    KComboBox *securityMode = new KComboBox(advancedOptionsExtraWidget);
    securityMode->setObjectName("kcfg_SecurityMode");

    QList<KConfigSkeleton::ItemEnum::Choice> securityModeChoices = Smb4KMountSettings::self()->securityModeItem()->choices();

    for (const KConfigSkeleton::ItemEnum::Choice &c : qAsConst(securityModeChoices)) {
        securityMode->addItem(c.label);
    }

    advancedOptionsExtraWidgetLayout->addWidget(useSecurityMode, 2, 0);
    advancedOptionsExtraWidgetLayout->addWidget(securityMode, 2, 1);

    // Additional options
    QLabel *additionalOptionsLabel = new QLabel(Smb4KMountSettings::self()->customCIFSOptionsItem()->label(), advancedOptionsExtraWidget);

    QWidget *additionalOptionsWidget = new QWidget(advancedOptionsExtraWidget);
    QHBoxLayout *additionalOptionsWidgetLayout = new QHBoxLayout(additionalOptionsWidget);
    additionalOptionsWidgetLayout->setContentsMargins(0, 0, 0, 0);

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

    advancedOptionsExtraWidgetLayout->addWidget(additionalOptionsLabel, 3, 0);
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
    connect(additionalOptionsEdit, SIGNAL(toggled(bool)), this, SLOT(slotAdditionalCIFSOptions()));
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

    QLabel *prefixLabel = new QLabel(Smb4KMountSettings::self()->mountPrefixItem()->label(), directoryBox);
    KUrlRequester *prefix = new KUrlRequester(directoryBox);
    prefix->setMode(KFile::Directory | KFile::LocalOnly);
    prefix->setObjectName("kcfg_MountPrefix");

    prefixLabel->setBuddy(prefix);

    QCheckBox *lowercaseSubdirs = new QCheckBox(Smb4KMountSettings::self()->forceLowerCaseSubdirsItem()->label(), directoryBox);
    lowercaseSubdirs->setObjectName("kcfg_ForceLowerCaseSubdirs");

    directoryBoxLayout->addWidget(prefixLabel, 0, 0);
    directoryBoxLayout->addWidget(prefix, 0, 1);
    directoryBoxLayout->addWidget(lowercaseSubdirs, 1, 0, 1, 2);

    basicTabLayout->addWidget(directoryBox, 0);

    //
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), basicTab);
    QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);

    QCheckBox *remountShares = new QCheckBox(Smb4KMountSettings::self()->remountSharesItem()->label(), behaviorBox);
    remountShares->setObjectName("kcfg_RemountShares");

    QLabel *remountAttemptsLabel = new QLabel(Smb4KMountSettings::self()->remountAttemptsItem()->label(), behaviorBox);
    remountAttemptsLabel->setObjectName("RemountAttemptsLabel");
    remountAttemptsLabel->setIndent(25);

    QSpinBox *remountAttempts = new QSpinBox(behaviorBox);
    remountAttempts->setObjectName("kcfg_RemountAttempts");
    remountAttemptsLabel->setBuddy(remountAttempts);

    QLabel *remountIntervalLabel = new QLabel(Smb4KMountSettings::self()->remountIntervalItem()->label(), behaviorBox);
    remountIntervalLabel->setObjectName("RemountIntervalLabel");
    remountIntervalLabel->setIndent(25);

    QSpinBox *remountInterval = new QSpinBox(behaviorBox);
    remountInterval->setObjectName("kcfg_RemountInterval");
    remountInterval->setSuffix(i18n(" min"));
    remountIntervalLabel->setBuddy(remountInterval);

    QCheckBox *unmountAllShares = new QCheckBox(Smb4KMountSettings::self()->unmountSharesOnExitItem()->label(), behaviorBox);
    unmountAllShares->setObjectName("kcfg_UnmountSharesOnExit");

    QCheckBox *unmountForeignShares = new QCheckBox(Smb4KMountSettings::self()->unmountForeignSharesItem()->label(), behaviorBox);
    unmountForeignShares->setObjectName("kcfg_UnmountForeignShares");

    QCheckBox *detectAllShares = new QCheckBox(Smb4KMountSettings::self()->detectAllSharesItem()->label(), behaviorBox);
    detectAllShares->setObjectName("kcfg_DetectAllShares");

    behaviorBoxLayout->addWidget(remountShares, 0, 0, 1, 2);
    behaviorBoxLayout->addWidget(remountAttemptsLabel, 1, 0);
    behaviorBoxLayout->addWidget(remountAttempts, 1, 1);
    behaviorBoxLayout->addWidget(remountIntervalLabel, 2, 0);
    behaviorBoxLayout->addWidget(remountInterval, 2, 1);
    behaviorBoxLayout->addWidget(unmountAllShares, 3, 0, 1, 2);
    behaviorBoxLayout->addWidget(unmountForeignShares, 4, 0, 1, 2);
    behaviorBoxLayout->addWidget(detectAllShares, 5, 0, 1, 2);

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
    useUserId->setObjectName("kcfg_UseUserId");

    QWidget *userIdInputWidget = new QWidget(commonOptionsBox);
    userIdInputWidget->setObjectName("UserIdInputWidget");

    QGridLayout *userLayout = new QGridLayout(userIdInputWidget);
    userLayout->setContentsMargins(0, 0, 0, 0);

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

    for (const KUser &u : allUsers) {
        QAction *userAction = userMenu->addAction(QString("%1 (%2)").arg(u.loginName()).arg(u.userId().nativeId()));
        userAction->setData(u.userId().nativeId());
    }

    userLayout->addWidget(userId, 0, 0);
    userLayout->addWidget(userChooser, 0, 1, Qt::AlignCenter);

    commonOptionsBoxLayout->addWidget(useUserId, 0, 0);
    commonOptionsBoxLayout->addWidget(userIdInputWidget, 0, 1);

    // Group information
    QCheckBox *useGroupId = new QCheckBox(Smb4KMountSettings::self()->useGroupIdItem()->label(), commonOptionsBox);
    useGroupId->setObjectName("kcfg_UseGroupId");

    QWidget *groupIdInputWidget = new QWidget(commonOptionsBox);
    groupIdInputWidget->setObjectName("GroupIdInputWidget");

    QGridLayout *groupLayout = new QGridLayout(groupIdInputWidget);
    groupLayout->setContentsMargins(0, 0, 0, 0);

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

    for (const KUserGroup &g : groupList) {
        QAction *groupAction = groupMenu->addAction(QString("%1 (%2)").arg(g.name()).arg(g.groupId().nativeId()));
        groupAction->setData(g.groupId().nativeId());
    }

    groupLayout->addWidget(groupId, 0, 0);
    groupLayout->addWidget(groupChooser, 0, 1, Qt::AlignCenter);

    commonOptionsBoxLayout->addWidget(useGroupId, 1, 0);
    commonOptionsBoxLayout->addWidget(groupIdInputWidget, 1, 1);

    // File mask
    QCheckBox *useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), commonOptionsBox);
    useFileMode->setObjectName("kcfg_UseFileMode");

    KLineEdit *fileMode = new KLineEdit(commonOptionsBox);
    fileMode->setObjectName("kcfg_FileMode");
    fileMode->setClearButtonEnabled(true);
    fileMode->setAlignment(Qt::AlignRight);

    commonOptionsBoxLayout->addWidget(useFileMode, 2, 0);
    commonOptionsBoxLayout->addWidget(fileMode, 2, 1);

    // Directory mask
    QCheckBox *useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), commonOptionsBox);
    useDirectoryMode->setObjectName("kcfg_UseDirectoryMode");

    KLineEdit *directoryMode = new KLineEdit(commonOptionsBox);
    directoryMode->setObjectName("kcfg_DirectoryMode");
    directoryMode->setClearButtonEnabled(true);
    directoryMode->setAlignment(Qt::AlignRight);

    commonOptionsBoxLayout->addWidget(useDirectoryMode, 3, 0);
    commonOptionsBoxLayout->addWidget(directoryMode, 3, 1);

    //
    // Character sets
    //
    QGroupBox *characterSetsBox = new QGroupBox(i18n("Character Sets"), mountTab);
    QGridLayout *characterSetsBoxLayout = new QGridLayout(characterSetsBox);

    // Client character set
    QCheckBox *useCharacterSets = new QCheckBox(Smb4KMountSettings::self()->useCharacterSetsItem()->label(), characterSetsBox);
    useCharacterSets->setObjectName("kcfg_UseCharacterSets");

    QLabel *clientCharacterSetLabel = new QLabel(Smb4KMountSettings::self()->clientCharsetItem()->label(), characterSetsBox);
    clientCharacterSetLabel->setIndent(25);
    clientCharacterSetLabel->setObjectName("ClientCharacterSetLabel");

    KComboBox *clientCharacterSet = new KComboBox(characterSetsBox);
    clientCharacterSet->setObjectName("kcfg_ClientCharset");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> charsetChoices = Smb4KMountSettings::self()->clientCharsetItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : charsetChoices) {
        clientCharacterSet->addItem(c.label);
    }

    clientCharacterSetLabel->setBuddy(clientCharacterSet);

    // Server character set
    QLabel *serverCharacterSetLabel = new QLabel(Smb4KMountSettings::self()->serverCodepageItem()->label(), characterSetsBox);
    serverCharacterSetLabel->setIndent(25);
    serverCharacterSetLabel->setObjectName("ServerCodepageLabel");

    KComboBox *serverCharacterSet = new KComboBox(characterSetsBox);
    serverCharacterSet->setObjectName("kcfg_ServerCodepage");

    QList<KCoreConfigSkeleton::ItemEnum::Choice> codepageChoices = Smb4KMountSettings::self()->serverCodepageItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : codepageChoices) {
        serverCharacterSet->addItem(c.label);
    }

    serverCharacterSetLabel->setBuddy(serverCharacterSet);

    characterSetsBoxLayout->addWidget(useCharacterSets, 0, 0, 1, 2);
    characterSetsBoxLayout->addWidget(clientCharacterSetLabel, 1, 0);
    characterSetsBoxLayout->addWidget(clientCharacterSet, 1, 1);
    characterSetsBoxLayout->addWidget(serverCharacterSetLabel, 2, 0);
    characterSetsBoxLayout->addWidget(serverCharacterSet, 2, 1);

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
    KLineEdit *userId = findChild<KLineEdit *>("kcfg_UserId");

    if (userId) {
        userId->setText(action->data().toString());
    }
}

void Smb4KConfigPageMounting::slotNewGroupTriggered(QAction *action)
{
    KLineEdit *groupId = findChild<KLineEdit *>("kcfg_GroupId");

    if (groupId) {
        groupId->setText(action->data().toString());
    }
}

void Smb4KConfigPageMounting::slotCIFSUnixExtensionsSupport(bool checked)
{
    QCheckBox *useUserId = findChild<QCheckBox *>("kcfg_UseUserId");

    if (useUserId) {
        useUserId->setEnabled(!checked);
    }

    QWidget *userIdInputWidget = findChild<QWidget *>("UserIdInputWidget");

    if (userIdInputWidget) {
        userIdInputWidget->setEnabled(!checked);
    }

    QCheckBox *useGroupId = findChild<QCheckBox *>("kcfg_UseGroupId");

    if (useGroupId) {
        useGroupId->setEnabled(!checked);
    }

    QWidget *groupIdInputWidget = findChild<QWidget *>("GroupIdInputWidget");

    if (groupIdInputWidget) {
        groupIdInputWidget->setEnabled(!checked);
    }

    QCheckBox *useFileMode = findChild<QCheckBox *>("kcfg_UseFileMode");

    if (useFileMode) {
        useFileMode->setEnabled(!checked);
    }

    KLineEdit *fileMode = findChild<KLineEdit *>("kcfg_FileMode");

    if (fileMode) {
        fileMode->setEnabled(!checked);
    }

    QCheckBox *useDirectoryMode = findChild<QCheckBox *>("kcfg_UseDirectoryMode");

    if (useDirectoryMode) {
        useDirectoryMode->setEnabled(!checked);
    }

    KLineEdit *directoryMode = findChild<KLineEdit *>("kcfg_DirectoryMode");

    if (directoryMode) {
        directoryMode->setEnabled(!checked);
    }
}

void Smb4KConfigPageMounting::slotAdditionalCIFSOptions()
{
#if defined(Q_OS_LINUX)
    KLineEdit *cifsOptions = findChild<KLineEdit *>("kcfg_CustomCIFSOptions");

    if (cifsOptions) {
        QString options = cifsOptions->originalText();

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
                QStringList list = options.split(',', Qt::SkipEmptyParts);
                QMutableStringListIterator it(list);

                while (it.hasNext()) {
                    QString arg = it.next().section("=", 0, 0);

                    if (!allowedArgs.contains(arg)) {
                        deniedArgs << arg;
                        it.remove();
                    }
                }

                if (!deniedArgs.isEmpty()) {
                    QString msg = i18np(
                        "<qt>The following entry is going to be removed from the additional options: %2. Please read the handbook for details.</qt>",
                        "<qt>The following %1 entries are going to be removed from the additional options: %2. Please read the handbook for details.</qt>",
                        deniedArgs.size(),
                        deniedArgs.join(", "));
                    KMessageBox::sorry(this, msg);
                }

                cifsOptions->setText(list.join(",").trimmed());
            } else {
                cifsOptions->clear();
            }
        }
    }
#endif
}

void Smb4KConfigPageMounting::slotCharacterSets(bool on)
{
    //
    // Client character set
    //
    QLabel *clientCharacterSetLabel = findChild<QLabel *>("ClientCharacterSetLabel");

    if (clientCharacterSetLabel) {
        clientCharacterSetLabel->setEnabled(on);
    }

    KComboBox *clientCharacterSet = findChild<KComboBox *>("kcfg_ClientCharset");

    if (clientCharacterSet) {
        clientCharacterSet->setEnabled(on);
    }

    //
    // Server character set
    //
    QLabel *serverCharacterSetLabel = findChild<QLabel *>("ServerCodepageLabel");

    if (serverCharacterSetLabel) {
        serverCharacterSetLabel->setEnabled(on);
    }

    KComboBox *serverCharacterSet = findChild<KComboBox *>("kcfg_ServerCodepage");

    if (serverCharacterSet) {
        serverCharacterSet->setEnabled(on);
    }
}

void Smb4KConfigPageMounting::slotRemountSharesToggled(bool on)
{
    //
    // Get the widget
    //
    QLabel *remountAttemptsLabel = findChild<QLabel *>("RemountAttemptsLabel");
    QSpinBox *remountAttempts = findChild<QSpinBox *>("kcfg_RemountAttempts");
    QLabel *remountIntervalLabel = findChild<QLabel *>("RemountIntervalLabel");
    QSpinBox *remountInterval = findChild<QSpinBox *>("kcfg_RemountInterval");

    //
    // Enable / disable the widgets
    //
    remountAttemptsLabel->setEnabled(on);
    remountAttempts->setEnabled(on);
    remountIntervalLabel->setEnabled(on);
    remountInterval->setEnabled(on);
}
