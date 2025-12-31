/*
    The configuration page for the mount options

    SPDX-FileCopyrightText: 2015-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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

class ModeValidator : public QValidator
{
public:
    ModeValidator(QObject *parent = nullptr)
        : QValidator(parent)
    {
    }
    ~ModeValidator()
    {
    }
    QValidator::State validate(QString &input, int &pos) const override
    {
        Q_UNUSED(pos);
        if (input.trimmed().size() == 4) {
            QChar ch = input.trimmed().at(0);

            if (ch.isDigit() && ch.toLatin1() != '0') {
                return QValidator::Invalid;
            } else if (ch.isDigit() && ch.toLatin1() == '0') {
                return QValidator::Acceptable;
            }
        }
        return QValidator::Intermediate;
    }
};

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
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), this);
    QVBoxLayout *behaviorBoxLayout = new QVBoxLayout(behaviorBox);

    QCheckBox *remountShares = new QCheckBox(Smb4KMountSettings::self()->remountSharesItem()->label(), behaviorBox);
    remountShares->setObjectName(QStringLiteral("kcfg_RemountShares"));

    m_remountSettingsWidget = new QWidget(behaviorBox);
    m_remountSettingsWidget->setEnabled(false);
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

    QCheckBox *checkServerOnlineState = new QCheckBox(Smb4KMountSettings::self()->checkServerOnlineStateItem()->label(), behaviorBox);
    checkServerOnlineState->setObjectName(QStringLiteral("kcfg_CheckServerOnlineState"));

    QCheckBox *unmountAllShares = new QCheckBox(Smb4KMountSettings::self()->unmountSharesOnExitItem()->label(), behaviorBox);
    unmountAllShares->setObjectName(QStringLiteral("kcfg_UnmountSharesOnExit"));

    QCheckBox *unmountInaccessibleShares = new QCheckBox(Smb4KMountSettings::self()->forceUnmountInaccessibleItem()->label(), behaviorBox);
    unmountInaccessibleShares->setObjectName(QStringLiteral("kcfg_ForceUnmountInaccessible"));

    QCheckBox *detectAllShares = new QCheckBox(Smb4KMountSettings::self()->detectAllSharesItem()->label(), behaviorBox);
    detectAllShares->setObjectName(QStringLiteral("kcfg_DetectAllShares"));

    behaviorBoxLayout->addWidget(remountShares);
    behaviorBoxLayout->addWidget(m_remountSettingsWidget);
    behaviorBoxLayout->addWidget(checkServerOnlineState);
    behaviorBoxLayout->addWidget(unmountAllShares);
    behaviorBoxLayout->addWidget(unmountInaccessibleShares);
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

    for (const KCoreConfigSkeleton::ItemEnum::Choice &wa : std::as_const(writeAccessChoices)) {
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

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : std::as_const(charsetChoices)) {
        characterSet->addItem(c.label);
    }

    commonOptionsLayout->addWidget(useCharacterSet, 1, 0);
    commonOptionsLayout->addWidget(characterSet, 1, 1);

    commonTabLayout->addWidget(commonOptions, 0);

    //
    // CIFS Unix Extensions Support group box
    //
    QGroupBox *cifsExtensionSupportBox = new QGroupBox(i18n("CIFS Unix Extensions Support"), commonTab);
    QVBoxLayout *cifsExtensionSupportLayout = new QVBoxLayout(cifsExtensionSupportBox);

    // CIFS Unix extensions support
    QCheckBox *cifsExtensionsSupport = new QCheckBox(Smb4KMountSettings::self()->cifsUnixExtensionsSupportItem()->label(), cifsExtensionSupportBox);
    cifsExtensionsSupport->setObjectName(QStringLiteral("kcfg_CifsUnixExtensionsSupport"));

    cifsExtensionSupportLayout->addWidget(cifsExtensionsSupport);

    m_singleCifsExtensionsSettingsWidget = new QWidget(cifsExtensionSupportBox);
    m_singleCifsExtensionsSettingsWidget->setEnabled(!Smb4KMountSettings::cifsUnixExtensionsSupport());
    QGridLayout *singleCifsSettingsWidgetLayout = new QGridLayout(m_singleCifsExtensionsSettingsWidget);
    singleCifsSettingsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    // Usage of user and group ID
    QCheckBox *useIds = new QCheckBox(Smb4KMountSettings::self()->useIdsItem()->label(), m_singleCifsExtensionsSettingsWidget);
    useIds->setObjectName(QStringLiteral("kcfg_UseIds"));

    QLabel *userIdLabel = new QLabel(i18n("User ID:"), m_singleCifsExtensionsSettingsWidget);
    userIdLabel->setIndent(25);
    KLineEdit *userId = new KLineEdit(KUser(KUser::UseRealUserID).userId().toString(), m_singleCifsExtensionsSettingsWidget);
    userId->setAlignment(Qt::AlignRight);
    userId->setReadOnly(true);

    QLabel *groupIdLabel = new QLabel(i18n("Group ID:"), m_singleCifsExtensionsSettingsWidget);
    groupIdLabel->setIndent(25);
    KLineEdit *groupId = new KLineEdit(KUser(KUser::UseRealUserID).groupId().toString(), m_singleCifsExtensionsSettingsWidget);
    groupId->setAlignment(Qt::AlignRight);
    groupId->setReadOnly(true);

    singleCifsSettingsWidgetLayout->addWidget(useIds, 0, 0, 1, 2);
    singleCifsSettingsWidgetLayout->addWidget(userIdLabel, 1, 0);
    singleCifsSettingsWidgetLayout->addWidget(userId, 1, 1);
    singleCifsSettingsWidgetLayout->addWidget(groupIdLabel, 2, 0);
    singleCifsSettingsWidgetLayout->addWidget(groupId, 2, 1);

    // File mask
    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), m_singleCifsExtensionsSettingsWidget);
    m_useFileMode->setObjectName(QStringLiteral("kcfg_UseFileMode"));

    m_fileMode = new KLineEdit(m_singleCifsExtensionsSettingsWidget);
    m_fileMode->setObjectName(QStringLiteral("kcfg_FileMode"));
    m_fileMode->setClearButtonEnabled(true);
    m_fileMode->setAlignment(Qt::AlignRight);
    m_fileMode->setInputMask(QStringLiteral("0999"));
    m_fileMode->setValidator(new ModeValidator(m_fileMode));

    singleCifsSettingsWidgetLayout->addWidget(m_useFileMode, 3, 0);
    singleCifsSettingsWidgetLayout->addWidget(m_fileMode, 3, 1);

    // Directory mask
    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), m_singleCifsExtensionsSettingsWidget);
    m_useDirectoryMode->setObjectName(QStringLiteral("kcfg_UseDirectoryMode"));

    m_directoryMode = new KLineEdit(m_singleCifsExtensionsSettingsWidget);
    m_directoryMode->setObjectName(QStringLiteral("kcfg_DirectoryMode"));
    m_directoryMode->setClearButtonEnabled(true);
    m_directoryMode->setAlignment(Qt::AlignRight);
    m_directoryMode->setInputMask(QStringLiteral("0999"));
    m_directoryMode->setValidator(new ModeValidator(m_directoryMode));

    singleCifsSettingsWidgetLayout->addWidget(m_useDirectoryMode, 4, 0);
    singleCifsSettingsWidgetLayout->addWidget(m_directoryMode, 4, 1);

    cifsExtensionSupportLayout->addWidget(m_singleCifsExtensionsSettingsWidget);

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

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : std::as_const(smbProtocolChoices)) {
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

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : std::as_const(cacheModeChoices)) {
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

    for (const KConfigSkeleton::ItemEnum::Choice &c : std::as_const(securityModeChoices)) {
        securityMode->addItem(c.label);
    }

    advancedOptionsExtraWidgetLayout->addWidget(useSecurityMode, 2, 0);
    advancedOptionsExtraWidgetLayout->addWidget(securityMode, 2, 1);

    advancedOptionsLayout->addWidget(advancedOptionsExtraWidget, 4, 0, 1, 2);

    advancedTabLayout->addWidget(advancedOptions, 0);
    advancedTabLayout->addStretch(100);

    addTab(advancedTab, i18n("Advanced Mount Settings"));

    connect(cifsExtensionsSupport, &QCheckBox::toggled, this, &Smb4KConfigPageMounting::slotCIFSUnixExtensionsSupport);
    connect(remountShares, &QCheckBox::toggled, this, &Smb4KConfigPageMounting::slotRemountSharesToggled);
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
    // Behavior
    //
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), this);
    QVBoxLayout *behaviorBoxLayout = new QVBoxLayout(behaviorBox);

    QCheckBox *remountShares = new QCheckBox(Smb4KMountSettings::self()->remountSharesItem()->label(), behaviorBox);
    remountShares->setObjectName(QStringLiteral("kcfg_RemountShares"));

    m_remountSettingsWidget = new QWidget(behaviorBox);
    m_remountSettingsWidget->setEnabled(false);
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

    QCheckBox *checkServerOnlineState = new QCheckBox(Smb4KMountSettings::self()->checkServerOnlineStateItem()->label(), behaviorBox);
    checkServerOnlineState->setObjectName(QStringLiteral("kcfg_CheckServerOnlineState"));

    QCheckBox *unmountAllShares = new QCheckBox(Smb4KMountSettings::self()->unmountSharesOnExitItem()->label(), behaviorBox);
    unmountAllShares->setObjectName(QStringLiteral("kcfg_UnmountSharesOnExit"));

    QCheckBox *detectAllShares = new QCheckBox(Smb4KMountSettings::self()->detectAllSharesItem()->label(), behaviorBox);
    detectAllShares->setObjectName(QStringLiteral("kcfg_DetectAllShares"));

    behaviorBoxLayout->addWidget(remountShares);
    behaviorBoxLayout->addWidget(m_remountSettingsWidget);
    behaviorBoxLayout->addWidget(checkServerOnlineState);
    behaviorBoxLayout->addWidget(unmountAllShares);
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

    // Usage of user and group ID
    QCheckBox *useIds = new QCheckBox(Smb4KMountSettings::self()->useIdsItem()->label(), commonOptionsBox);
    useIds->setObjectName(QStringLiteral("kcfg_UseIds"));

    QLabel *userIdLabel = new QLabel(i18n("User ID:"), commonOptionsBox);
    userIdLabel->setIndent(25);
    KLineEdit *userId = new KLineEdit(KUser(KUser::UseRealUserID).userId().toString(), commonOptionsBox);
    userId->setAlignment(Qt::AlignRight);
    userId->setReadOnly(true);

    QLabel *groupIdLabel = new QLabel(i18n("Group ID:"), commonOptionsBox);
    groupIdLabel->setIndent(25);
    KLineEdit *groupId = new KLineEdit(KUser(KUser::UseRealUserID).groupId().toString(), commonOptionsBox);
    groupId->setAlignment(Qt::AlignRight);
    groupId->setReadOnly(true);

    commonOptionsBoxLayout->addWidget(useIds, 0, 0, 1, 2);
    commonOptionsBoxLayout->addWidget(userIdLabel, 1, 0);
    commonOptionsBoxLayout->addWidget(userId, 1, 1);
    commonOptionsBoxLayout->addWidget(groupIdLabel, 2, 0);
    commonOptionsBoxLayout->addWidget(groupId, 2, 1);

    // File mask
    m_useFileMode = new QCheckBox(Smb4KMountSettings::self()->useFileModeItem()->label(), commonOptionsBox);
    m_useFileMode->setObjectName(QStringLiteral("kcfg_UseFileMode"));

    m_fileMode = new KLineEdit(commonOptionsBox);
    m_fileMode->setObjectName(QStringLiteral("kcfg_FileMode"));
    m_fileMode->setClearButtonEnabled(true);
    m_fileMode->setAlignment(Qt::AlignRight);
    m_fileMode->setInputMask(QStringLiteral("0999"));
    m_fileMode->setValidator(new ModeValidator(m_fileMode));

    commonOptionsBoxLayout->addWidget(m_useFileMode, 3, 0);
    commonOptionsBoxLayout->addWidget(m_fileMode, 3, 1);

    // Directory mask
    m_useDirectoryMode = new QCheckBox(Smb4KMountSettings::self()->useDirectoryModeItem()->label(), commonOptionsBox);
    m_useDirectoryMode->setObjectName(QStringLiteral("kcfg_UseDirectoryMode"));

    m_directoryMode = new KLineEdit(commonOptionsBox);
    m_directoryMode->setObjectName(QStringLiteral("kcfg_DirectoryMode"));
    m_directoryMode->setClearButtonEnabled(true);
    m_directoryMode->setAlignment(Qt::AlignRight);
    m_directoryMode->setInputMask(QStringLiteral("0999"));
    m_directoryMode->setValidator(new ModeValidator(m_directoryMode));

    commonOptionsBoxLayout->addWidget(m_useDirectoryMode, 4, 0);
    commonOptionsBoxLayout->addWidget(m_directoryMode, 4, 1);

    //
    // Character sets
    //
    QGroupBox *characterSetsBox = new QGroupBox(i18n("Character Sets"), mountTab);
    QVBoxLayout *characterSetsBoxLayout = new QVBoxLayout(characterSetsBox);

    // Client character set
    QCheckBox *useCharacterSets = new QCheckBox(Smb4KMountSettings::self()->useCharacterSetsItem()->label(), characterSetsBox);
    useCharacterSets->setObjectName(QStringLiteral("kcfg_UseCharacterSets"));

    m_characterSetSettingsWidget = new QWidget(characterSetsBox);
    m_characterSetSettingsWidget->setEnabled(false);
    QGridLayout *characterSetSettingsWidgetLayout = new QGridLayout(m_characterSetSettingsWidget);
    characterSetSettingsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *clientCharacterSetLabel = new QLabel(Smb4KMountSettings::self()->clientCharsetItem()->label(), m_characterSetSettingsWidget);
    clientCharacterSetLabel->setIndent(25);

    KComboBox *clientCharacterSet = new KComboBox(m_characterSetSettingsWidget);
    clientCharacterSet->setObjectName(QStringLiteral("kcfg_ClientCharset"));

    QList<KCoreConfigSkeleton::ItemEnum::Choice> charsetChoices = Smb4KMountSettings::self()->clientCharsetItem()->choices();

    for (const KCoreConfigSkeleton::ItemEnum::Choice &c : charsetChoices) {
        clientCharacterSet->addItem(c.label);
    }

    clientCharacterSetLabel->setBuddy(clientCharacterSet);

    // Server character set
    QLabel *serverCharacterSetLabel = new QLabel(Smb4KMountSettings::self()->serverCodepageItem()->label(), m_characterSetSettingsWidget);
    serverCharacterSetLabel->setIndent(25);
    serverCharacterSetLabel->setObjectName(QStringLiteral("ServerCodepageLabel"));

    KComboBox *serverCharacterSet = new KComboBox(m_characterSetSettingsWidget);
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
    characterSetsBoxLayout->addWidget(m_characterSetSettingsWidget);

    mountTabLayout->addWidget(commonOptionsBox, 0);
    mountTabLayout->addWidget(characterSetsBox, 0);
    mountTabLayout->addStretch(100);

    addTab(mountTab, i18n("Mount Settings"));

    connect(useCharacterSets, &QCheckBox::toggled, this, &Smb4KConfigPageMounting::slotCharacterSets);
    connect(remountShares, &QCheckBox::toggled, this, &Smb4KConfigPageMounting::slotRemountSharesToggled);
}
#else
//
// Dummy
//
void Smb4KConfigPageMounting::setupWidget()
{
}
#endif

#if defined(Q_OS_LINUX)
void Smb4KConfigPageMounting::slotCIFSUnixExtensionsSupport(bool checked)
{
    m_singleCifsExtensionsSettingsWidget->setEnabled(!checked);
}
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
void Smb4KConfigPageMounting::slotCharacterSets(bool on)
{
    m_characterSetSettingsWidget->setEnabled(on);
}
#endif

void Smb4KConfigPageMounting::slotRemountSharesToggled(bool on)
{
    m_remountSettingsWidget->setEnabled(on);
}
