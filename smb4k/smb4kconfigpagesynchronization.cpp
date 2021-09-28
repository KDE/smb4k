/*
    The configuration page for the synchronization options
    -------------------
    begin                : So Nov 20 2005
    SPDX-FileCopyrightText: 2005-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagesynchronization.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

// KDE includes
#include <KCompletion/KLineEdit>
#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlRequester>

Smb4KConfigPageSynchronization::Smb4KConfigPageSynchronization(QWidget *parent)
    : QTabWidget(parent)
{
    //
    // 'Basic Settings' tab
    //
    QWidget *basicTab = new QWidget(this);
    QVBoxLayout *basicTabLayout = new QVBoxLayout(basicTab);

    // Default destination (rsync)
    QGroupBox *pathsBox = new QGroupBox(i18n("Default Destination"), basicTab);
    QGridLayout *pathsBoxLayout = new QGridLayout(pathsBox);

    QLabel *rsyncPrefixLabel = new QLabel(Smb4KSettings::self()->rsyncPrefixItem()->label(), pathsBox);
    KUrlRequester *rsyncPrefix = new KUrlRequester(pathsBox);
    rsyncPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    rsyncPrefix->setObjectName("kcfg_RsyncPrefix");
    rsyncPrefixLabel->setBuddy(rsyncPrefix);

    pathsBoxLayout->addWidget(rsyncPrefixLabel, 0, 0);
    pathsBoxLayout->addWidget(rsyncPrefix, 0, 1);

    // Behavior
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), basicTab);
    QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);

    QCheckBox *archiveMode = new QCheckBox(Smb4KSettings::self()->archiveModeItem()->label(), behaviorBox);
    archiveMode->setObjectName("kcfg_ArchiveMode");
    QCheckBox *recurseDirs = new QCheckBox(Smb4KSettings::self()->recurseIntoDirectoriesItem()->label(), behaviorBox);
    recurseDirs->setObjectName("kcfg_RecurseIntoDirectories");
    QCheckBox *relativePaths = new QCheckBox(Smb4KSettings::self()->relativePathNamesItem()->label(), behaviorBox);
    relativePaths->setObjectName("kcfg_RelativePathNames");
    QCheckBox *noImpliedDirs = new QCheckBox(Smb4KSettings::self()->noImpliedDirectoriesItem()->label(), behaviorBox);
    noImpliedDirs->setObjectName("kcfg_NoImpliedDirectories");
    QCheckBox *transferDirs = new QCheckBox(Smb4KSettings::self()->transferDirectoriesItem()->label(), behaviorBox);
    transferDirs->setObjectName("kcfg_TransferDirectories");

    behaviorBoxLayout->addWidget(archiveMode, 0, 0);
    behaviorBoxLayout->addWidget(recurseDirs, 0, 1);
    behaviorBoxLayout->addWidget(relativePaths, 1, 0);
    behaviorBoxLayout->addWidget(noImpliedDirs, 1, 1);
    behaviorBoxLayout->addWidget(transferDirs, 2, 0);

    // Backup
    QGroupBox *backupBox = new QGroupBox(i18n("Backup"), basicTab);
    QGridLayout *backupBoxLayout = new QGridLayout(backupBox);

    QCheckBox *makeBackups = new QCheckBox(Smb4KSettings::self()->makeBackupsItem()->label(), backupBox);
    makeBackups->setObjectName("kcfg_MakeBackups");
    QCheckBox *backupSuffixButton = new QCheckBox(Smb4KSettings::self()->useBackupSuffixItem()->label(), backupBox);
    backupSuffixButton->setObjectName("kcfg_UseBackupSuffix");
    KLineEdit *backupSuffix = new KLineEdit(backupBox);
    backupSuffix->setObjectName("kcfg_BackupSuffix");
    backupSuffix->setClearButtonEnabled(true);
    QCheckBox *backupDirButton = new QCheckBox(Smb4KSettings::self()->useBackupDirectoryItem()->label(), backupBox);
    backupDirButton->setObjectName("kcfg_UseBackupDirectory");
    KUrlRequester *backupDir = new KUrlRequester(backupBox);
    backupDir->setObjectName("kcfg_BackupDirectory");
    backupDir->setMode(KFile::Directory | KFile::LocalOnly);

    backupBoxLayout->addWidget(makeBackups, 0, 0);
    backupBoxLayout->addWidget(backupSuffixButton, 1, 0);
    backupBoxLayout->addWidget(backupSuffix, 1, 1);
    backupBoxLayout->addWidget(backupDirButton, 2, 0);
    backupBoxLayout->addWidget(backupDir, 2, 1);

    basicTabLayout->addWidget(pathsBox, 0);
    basicTabLayout->addWidget(behaviorBox, 0);
    basicTabLayout->addWidget(backupBox, 0);
    basicTabLayout->addStretch(100);

    addTab(basicTab, i18n("Basic Settings"));

    //
    // 'File Handling' tab
    //
    QWidget *fileHandlingTab = new QWidget(this);
    QVBoxLayout *fileHandlingTabLayout = new QVBoxLayout(fileHandlingTab);

    // General
    QGroupBox *generalHandlingBox = new QGroupBox(i18n("General"), fileHandlingTab);
    QGridLayout *generalHandlingBoxLayout = new QGridLayout(generalHandlingBox);

    QCheckBox *updateTarget = new QCheckBox(Smb4KSettings::self()->updateTargetItem()->label(), generalHandlingBox);
    updateTarget->setObjectName("kcfg_UpdateTarget");
    QCheckBox *updateInPlace = new QCheckBox(Smb4KSettings::self()->updateInPlaceItem()->label(), generalHandlingBox);
    updateInPlace->setObjectName("kcfg_UpdateInPlace");
    QCheckBox *sparseFiles = new QCheckBox(Smb4KSettings::self()->efficientSparseFileHandlingItem()->label(), generalHandlingBox);
    sparseFiles->setObjectName("kcfg_EfficientSparseFileHandling");
    QCheckBox *copyFilesWhole = new QCheckBox(Smb4KSettings::self()->copyFilesWholeItem()->label(), generalHandlingBox);
    copyFilesWhole->setObjectName("kcfg_CopyFilesWhole");
    QCheckBox *updateExisting = new QCheckBox(Smb4KSettings::self()->updateExistingItem()->label(), generalHandlingBox);
    updateExisting->setObjectName("kcfg_UpdateExisting");
    QCheckBox *ignoreExisting = new QCheckBox(Smb4KSettings::self()->ignoreExistingItem()->label(), generalHandlingBox);
    ignoreExisting->setObjectName("kcfg_IgnoreExisting");

    generalHandlingBoxLayout->addWidget(updateTarget, 0, 0);
    generalHandlingBoxLayout->addWidget(updateInPlace, 0, 1);
    generalHandlingBoxLayout->addWidget(sparseFiles, 1, 0);
    generalHandlingBoxLayout->addWidget(copyFilesWhole, 1, 1);
    generalHandlingBoxLayout->addWidget(updateExisting, 2, 0);
    generalHandlingBoxLayout->addWidget(ignoreExisting, 2, 1);

    // Links
    QGroupBox *linksBox = new QGroupBox(i18n("Links"), fileHandlingTab);
    QGridLayout *linksBoxLayout = new QGridLayout(linksBox);

    QCheckBox *preserveLinks = new QCheckBox(Smb4KSettings::self()->preserveSymlinksItem()->label(), linksBox);
    preserveLinks->setObjectName("kcfg_PreserveSymlinks");
    QCheckBox *transformLinks = new QCheckBox(Smb4KSettings::self()->transformSymlinksItem()->label(), linksBox);
    transformLinks->setObjectName("kcfg_TransformSymlinks");
    QCheckBox *transformUnsafe = new QCheckBox(Smb4KSettings::self()->transformUnsafeSymlinksItem()->label(), linksBox);
    transformUnsafe->setObjectName("kcfg_TransformUnsafeSymlinks");
    QCheckBox *ignoreUnsafe = new QCheckBox(Smb4KSettings::self()->ignoreUnsafeSymlinksItem()->label(), linksBox);
    ignoreUnsafe->setObjectName("kcfg_IgnoreUnsafeSymlinks");
    QCheckBox *mungeLinks = new QCheckBox(Smb4KSettings::self()->mungeSymlinksItem()->label(), linksBox);
    mungeLinks->setObjectName("kcfg_MungeSymlinks");
    QCheckBox *preserveHlinks = new QCheckBox(Smb4KSettings::self()->preserveHardLinksItem()->label(), linksBox);
    preserveHlinks->setObjectName("kcfg_PreserveHardLinks");
    QCheckBox *copyDirLinks = new QCheckBox(Smb4KSettings::self()->copyDirectorySymlinksItem()->label(), linksBox);
    copyDirLinks->setObjectName("kcfg_CopyDirectorySymlinks");
    QCheckBox *keepDirLinks = new QCheckBox(Smb4KSettings::self()->keepDirectorySymlinksItem()->label(), linksBox);
    keepDirLinks->setObjectName("kcfg_KeepDirectorySymlinks");

    linksBoxLayout->addWidget(preserveLinks, 0, 0);
    linksBoxLayout->addWidget(transformLinks, 0, 1);
    linksBoxLayout->addWidget(transformUnsafe, 1, 0);
    linksBoxLayout->addWidget(ignoreUnsafe, 1, 1);
    linksBoxLayout->addWidget(mungeLinks, 2, 0);
    linksBoxLayout->addWidget(preserveHlinks, 2, 1);
    linksBoxLayout->addWidget(copyDirLinks, 3, 0);
    linksBoxLayout->addWidget(keepDirLinks, 3, 1);

    // Permissions & Attributes
    QGroupBox *permissionsBox = new QGroupBox(i18n("Permissions, etc."), fileHandlingTab);
    QGridLayout *permissionsBoxLayout = new QGridLayout(permissionsBox);

    QCheckBox *preservePerms = new QCheckBox(Smb4KSettings::self()->preservePermissionsItem()->label(), permissionsBox);
    preservePerms->setObjectName("kcfg_PreservePermissions");
    QCheckBox *preserveOwner = new QCheckBox(Smb4KSettings::self()->preserveOwnerItem()->label(), permissionsBox);
    preserveOwner->setObjectName("kcfg_PreserveOwner");
    QCheckBox *preserveGroup = new QCheckBox(Smb4KSettings::self()->preserveGroupItem()->label(), permissionsBox);
    preserveGroup->setObjectName("kcfg_PreserveGroup");
    QCheckBox *preserveDevices = new QCheckBox(Smb4KSettings::self()->preserveDevicesAndSpecialsItem()->label(), permissionsBox);
    preserveDevices->setObjectName("kcfg_PreserveDevicesAndSpecials");
    QCheckBox *preserveTimes = new QCheckBox(Smb4KSettings::self()->preserveTimesItem()->label(), permissionsBox);
    preserveTimes->setObjectName("kcfg_PreserveTimes");
    QCheckBox *omitDirTimes = new QCheckBox(Smb4KSettings::self()->omitDirectoryTimesItem()->label(), permissionsBox);
    omitDirTimes->setObjectName("kcfg_OmitDirectoryTimes");

    permissionsBoxLayout->addWidget(preservePerms, 0, 0);
    permissionsBoxLayout->addWidget(preserveOwner, 0, 1);
    permissionsBoxLayout->addWidget(preserveGroup, 1, 0);
    permissionsBoxLayout->addWidget(preserveDevices, 1, 1);
    permissionsBoxLayout->addWidget(preserveTimes, 2, 0);
    permissionsBoxLayout->addWidget(omitDirTimes, 2, 1);

    fileHandlingTabLayout->addWidget(generalHandlingBox, 0);
    fileHandlingTabLayout->addWidget(linksBox, 0);
    fileHandlingTabLayout->addWidget(permissionsBox, 0);
    fileHandlingTabLayout->addStretch(100);

    addTab(fileHandlingTab, i18n("File Handling"));

    //
    // 'File Transfer' tab
    //
    QWidget *transferTab = new QWidget(this);
    QVBoxLayout *transferTabLayout = new QVBoxLayout(transferTab);

    // Compression
    QGroupBox *compressionBox = new QGroupBox(i18n("Compression"), transferTab);
    QGridLayout *compressionBoxLayout = new QGridLayout(compressionBox);

    QCheckBox *compressData = new QCheckBox(Smb4KSettings::self()->compressDataItem()->label(), compressionBox);
    compressData->setObjectName("kcfg_CompressData");
    QCheckBox *compressionLevelButton = new QCheckBox(Smb4KSettings::self()->useCompressionLevelItem()->label(), compressionBox);
    compressionLevelButton->setObjectName("kcfg_UseCompressionLevel");
    QSpinBox *compressionLevel = new QSpinBox(compressionBox);
    compressionLevel->setObjectName("kcfg_CompressionLevel");
    QCheckBox *skipCompressionButton = new QCheckBox(Smb4KSettings::self()->useSkipCompressionItem()->label(), compressionBox);
    skipCompressionButton->setObjectName("kcfg_UseSkipCompression");
    KLineEdit *skipCompression = new KLineEdit(compressionBox);
    skipCompression->setObjectName("kcfg_SkipCompression");
    skipCompression->setClearButtonEnabled(true);

    compressionBoxLayout->addWidget(compressData, 0, 0, 1, -1);
    compressionBoxLayout->addWidget(compressionLevelButton, 1, 0);
    compressionBoxLayout->addWidget(compressionLevel, 1, 1);
    compressionBoxLayout->addWidget(skipCompressionButton, 2, 0);
    compressionBoxLayout->addWidget(skipCompression, 2, 1);

    // Files
    QGroupBox *filesBox = new QGroupBox(i18n("Files"), transferTab);
    QGridLayout *filesBoxLayout = new QGridLayout(filesBox);

    QCheckBox *minTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMinimalTransferSizeItem()->label(), filesBox);
    minTransferSizeButton->setObjectName("kcfg_UseMinimalTransferSize");
    QSpinBox *minTransferSize = new QSpinBox(filesBox);
    minTransferSize->setObjectName("kcfg_MinimalTransferSize");
    minTransferSize->setSuffix(i18n(" KiB"));
    QCheckBox *maxTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMaximalTransferSizeItem()->label(), filesBox);
    maxTransferSizeButton->setObjectName("kcfg_UseMaximalTransferSize");
    QSpinBox *maxTransferSize = new QSpinBox(filesBox);
    maxTransferSize->setObjectName("kcfg_MaximalTransferSize");
    maxTransferSize->setSuffix(i18n(" KiB"));
    QCheckBox *keepPartial = new QCheckBox(Smb4KSettings::self()->keepPartialItem()->label(), filesBox);
    keepPartial->setObjectName("kcfg_KeepPartial");
    QCheckBox *partialDirButton = new QCheckBox(Smb4KSettings::self()->usePartialDirectoryItem()->label(), filesBox);
    partialDirButton->setObjectName("kcfg_UsePartialDirectory");
    KUrlRequester *partialDir = new KUrlRequester(filesBox);
    partialDir->setObjectName("kcfg_PartialDirectory");
    partialDir->setMode(KFile::Directory | KFile::LocalOnly);

    filesBoxLayout->addWidget(minTransferSizeButton, 0, 0);
    filesBoxLayout->addWidget(minTransferSize, 0, 1);
    filesBoxLayout->addWidget(maxTransferSizeButton, 1, 0);
    filesBoxLayout->addWidget(maxTransferSize, 1, 1);
    filesBoxLayout->addWidget(keepPartial, 2, 0, 1, -1);
    filesBoxLayout->addWidget(partialDirButton, 3, 0);
    filesBoxLayout->addWidget(partialDir, 3, 1);

    // Miscellaneous
    QGroupBox *miscTransferBox = new QGroupBox(i18n("Miscellaneous"), transferTab);
    QGridLayout *miscTransferBoxLayout = new QGridLayout(miscTransferBox);

    QCheckBox *bwLimitButton = new QCheckBox(Smb4KSettings::self()->useBandwidthLimitItem()->label(), miscTransferBox);
    bwLimitButton->setObjectName("kcfg_UseBandwidthLimit");
    QSpinBox *bwLimit = new QSpinBox(miscTransferBox);
    bwLimit->setObjectName("kcfg_BandwidthLimit");
    bwLimit->setSuffix(i18n(" KiB/s"));

    miscTransferBoxLayout->addWidget(bwLimitButton, 0, 0);
    miscTransferBoxLayout->addWidget(bwLimit, 0, 1);

    transferTabLayout->addWidget(compressionBox, 0);
    transferTabLayout->addWidget(filesBox, 0);
    transferTabLayout->addWidget(miscTransferBox, 0);
    transferTabLayout->addStretch(100);

    addTab(transferTab, i18n("File Transfer"));

    //
    // 'File Deletion' tab
    //
    QWidget *deleteTab = new QWidget(this);
    QVBoxLayout *deleteTabLayout = new QVBoxLayout(deleteTab);

    // Files and Directories
    QGroupBox *filesAndDirsBox = new QGroupBox(i18n("Files && Directories"), deleteTab);
    QGridLayout *filesAndDirsBoxLayout = new QGridLayout(filesAndDirsBox);

    QCheckBox *removeSource = new QCheckBox(Smb4KSettings::self()->removeSourceFilesItem()->label(), filesAndDirsBox);
    removeSource->setObjectName("kcfg_RemoveSourceFiles");
    QCheckBox *deleteExtraneous = new QCheckBox(Smb4KSettings::self()->deleteExtraneousItem()->label(), filesAndDirsBox);
    deleteExtraneous->setObjectName("kcfg_DeleteExtraneous");
    QCheckBox *deleteBefore = new QCheckBox(Smb4KSettings::self()->deleteBeforeItem()->label(), filesAndDirsBox);
    deleteBefore->setObjectName("kcfg_DeleteBefore");
    QCheckBox *deleteAfter = new QCheckBox(Smb4KSettings::self()->deleteAfterItem()->label(), filesAndDirsBox);
    deleteAfter->setObjectName("kcfg_DeleteAfter");
    QCheckBox *deleteDuring = new QCheckBox(Smb4KSettings::self()->deleteDuringItem()->label(), filesAndDirsBox);
    deleteDuring->setObjectName("kcfg_DeleteDuring");
    QCheckBox *deleteExcluded = new QCheckBox(Smb4KSettings::self()->deleteExcludedItem()->label(), filesAndDirsBox);
    deleteExcluded->setObjectName("kcfg_DeleteExcluded");
    QCheckBox *ignoreIOErrors = new QCheckBox(Smb4KSettings::self()->ignoreErrorsItem()->label(), filesAndDirsBox);
    ignoreIOErrors->setObjectName("kcfg_IgnoreErrors");
    QCheckBox *forceDirDeletion = new QCheckBox(Smb4KSettings::self()->forceDirectoryDeletionItem()->label(), filesAndDirsBox);
    forceDirDeletion->setObjectName("kcfg_ForceDirectoryDeletion");

    filesAndDirsBoxLayout->addWidget(removeSource, 0, 0);
    filesAndDirsBoxLayout->addWidget(deleteExtraneous, 0, 1);
    filesAndDirsBoxLayout->addWidget(deleteBefore, 1, 0);
    filesAndDirsBoxLayout->addWidget(deleteAfter, 1, 1);
    filesAndDirsBoxLayout->addWidget(deleteDuring, 2, 0);
    filesAndDirsBoxLayout->addWidget(deleteExcluded, 2, 1);
    filesAndDirsBoxLayout->addWidget(ignoreIOErrors, 3, 0);
    filesAndDirsBoxLayout->addWidget(forceDirDeletion, 3, 1);

    // Restrictions
    QGroupBox *restrictionsBox = new QGroupBox(i18n("Restrictions"), deleteTab);
    QGridLayout *restrictionsBoxLayout = new QGridLayout(restrictionsBox);

    QCheckBox *maxDeleteButton = new QCheckBox(Smb4KSettings::self()->useMaximumDeleteItem()->label(), restrictionsBox);
    maxDeleteButton->setObjectName("kcfg_UseMaximumDelete");
    QSpinBox *maxDelete = new QSpinBox(restrictionsBox);
    maxDelete->setObjectName("kcfg_MaximumDeleteValue");

    restrictionsBoxLayout->addWidget(maxDeleteButton, 0, 0);
    restrictionsBoxLayout->addWidget(maxDelete, 0, 1);

    deleteTabLayout->addWidget(filesAndDirsBox, 0);
    deleteTabLayout->addWidget(restrictionsBox, 0);
    deleteTabLayout->addStretch(100);

    addTab(deleteTab, i18n("File Deletion"));

    //
    // 'Filter' tab
    //
    QWidget *filterTab = new QWidget(this);
    QVBoxLayout *filterTabLayout = new QVBoxLayout(filterTab);

    // General
    QGroupBox *generalFilterBox = new QGroupBox(i18n("General"), filterTab);
    QGridLayout *generalFilterBoxLayout = new QGridLayout(generalFilterBox);

    QCheckBox *cvsExclude = new QCheckBox(Smb4KSettings::self()->useCVSExcludeItem()->label(), generalFilterBox);
    cvsExclude->setObjectName("kcfg_UseCVSExclude");
    QCheckBox *excludePatternButton = new QCheckBox(Smb4KSettings::self()->useExcludePatternItem()->label(), generalFilterBox);
    excludePatternButton->setObjectName("kcfg_UseExcludePattern");
    KLineEdit *excludePattern = new KLineEdit(generalFilterBox);
    excludePattern->setObjectName("kcfg_ExcludePattern");
    excludePattern->setClearButtonEnabled(true);
    QCheckBox *excludeFromButton = new QCheckBox(Smb4KSettings::self()->useExcludeFromItem()->label(), generalFilterBox);
    excludeFromButton->setObjectName("kcfg_UseExcludeFrom");
    KUrlRequester *excludeFrom = new KUrlRequester(generalFilterBox);
    excludeFrom->setObjectName("kcfg_ExcludeFrom");
    excludeFrom->setMode(KFile::File | KFile::LocalOnly);
    QCheckBox *includePatternButton = new QCheckBox(Smb4KSettings::self()->useIncludePatternItem()->label(), generalFilterBox);
    includePatternButton->setObjectName("kcfg_UseIncludePattern");
    KLineEdit *includePattern = new KLineEdit(generalFilterBox);
    includePattern->setObjectName("kcfg_IncludePattern");
    includePattern->setClearButtonEnabled(true);
    QCheckBox *includeFromButton = new QCheckBox(Smb4KSettings::self()->useIncludeFromItem()->label(), generalFilterBox);
    includeFromButton->setObjectName("kcfg_UseIncludeFrom");
    KUrlRequester *includeFrom = new KUrlRequester(generalFilterBox);
    includeFrom->setObjectName("kcfg_IncludeFrom");
    includeFrom->setMode(KFile::File | KFile::LocalOnly);

    generalFilterBoxLayout->addWidget(cvsExclude, 0, 0, 1, -1);
    generalFilterBoxLayout->addWidget(excludePatternButton, 1, 0);
    generalFilterBoxLayout->addWidget(excludePattern, 1, 1);
    generalFilterBoxLayout->addWidget(excludeFromButton, 2, 0);
    generalFilterBoxLayout->addWidget(excludeFrom, 2, 1);
    generalFilterBoxLayout->addWidget(includePatternButton, 3, 0);
    generalFilterBoxLayout->addWidget(includePattern, 3, 1);
    generalFilterBoxLayout->addWidget(includeFromButton, 4, 0);
    generalFilterBoxLayout->addWidget(includeFrom, 4, 1);

    // Filter rules
    QGroupBox *filterRulesBox = new QGroupBox(i18n("Filter Rules"), filterTab);
    QGridLayout *filterRulesBoxLayout = new QGridLayout(filterRulesBox);

    QLabel *customRulesLabel = new QLabel(Smb4KSettings::self()->customFilteringRulesItem()->label(), filterRulesBox);
    KLineEdit *customRules = new KLineEdit(filterRulesBox);
    customRules->setObjectName("kcfg_CustomFilteringRules");
    customRules->setClearButtonEnabled(true);
    customRulesLabel->setBuddy(customRules);
    QLabel *specialRulesLabel = new QLabel(i18n("Special filter rules:"), filterRulesBox);
    QCheckBox *useFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFilterRuleItem()->label(), filterRulesBox);
    useFFilterRule->setObjectName("kcfg_UseFFilterRule");
    QCheckBox *useFFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFFilterRuleItem()->label(), filterRulesBox);
    useFFFilterRule->setObjectName("kcfg_UseFFFilterRule");

    filterRulesBoxLayout->addWidget(customRulesLabel, 0, 0);
    filterRulesBoxLayout->addWidget(customRules, 0, 1);
    filterRulesBoxLayout->addWidget(specialRulesLabel, 1, 0, 1, -1);
    filterRulesBoxLayout->addWidget(useFFilterRule, 2, 0, 1, -1);
    filterRulesBoxLayout->addWidget(useFFFilterRule, 3, 0, 1, -1);

    filterTabLayout->addWidget(generalFilterBox, 0);
    filterTabLayout->addWidget(filterRulesBox, 0);
    filterTabLayout->addStretch(100);

    addTab(filterTab, i18n("Filtering"));

    //
    // 'Miscellaneous' tab
    //
    QWidget *miscTab = new QWidget(this);
    QVBoxLayout *miscTabLayout = new QVBoxLayout(miscTab);

    // Checksums
    QGroupBox *checksumsBox = new QGroupBox(i18n("Checksums"), miscTab);
    QGridLayout *checksumsBoxLayout = new QGridLayout(checksumsBox);

    QCheckBox *blockSizeButton = new QCheckBox(Smb4KSettings::self()->useBlockSizeItem()->label(), checksumsBox);
    blockSizeButton->setObjectName("kcfg_UseBlockSize");
    QSpinBox *blockSize = new QSpinBox(checksumsBox);
    blockSize->setObjectName("kcfg_BlockSize");

    QCheckBox *checksumSeedButton = new QCheckBox(Smb4KSettings::self()->useChecksumSeedItem()->label(), checksumsBox);
    checksumSeedButton->setObjectName("kcfg_UseChecksumSeed");
    QSpinBox *checksumSeed = new QSpinBox(checksumsBox);
    checksumSeed->setObjectName("kcfg_ChecksumSeed");
    QCheckBox *useChecksum = new QCheckBox(Smb4KSettings::self()->useChecksumItem()->label(), checksumsBox);
    useChecksum->setObjectName("kcfg_UseChecksum");

    checksumsBoxLayout->addWidget(blockSizeButton, 0, 0);
    checksumsBoxLayout->addWidget(blockSize, 0, 1);
    checksumsBoxLayout->addWidget(checksumSeedButton, 1, 0);
    checksumsBoxLayout->addWidget(checksumSeed, 1, 1);
    checksumsBoxLayout->addWidget(useChecksum, 2, 0);

    // Miscellaneous
    QGroupBox *miscBox = new QGroupBox(i18n("Miscellaneous"), miscTab);
    QGridLayout *miscBoxLayout = new QGridLayout(miscBox);

    QCheckBox *oneFilesystem = new QCheckBox(Smb4KSettings::self()->oneFileSystemItem()->label(), miscBox);
    oneFilesystem->setObjectName("kcfg_OneFileSystem");
    QCheckBox *delayUpdates = new QCheckBox(Smb4KSettings::self()->delayUpdatesItem()->label(), miscBox);
    delayUpdates->setObjectName("kcfg_DelayUpdates");

    miscBoxLayout->addWidget(oneFilesystem, 0, 0);
    miscBoxLayout->addWidget(delayUpdates, 0, 1);

    miscTabLayout->addWidget(checksumsBox, 0);
    miscTabLayout->addWidget(miscBox, 0);
    miscTabLayout->addStretch(100);

    addTab(miscTab, i18n("Miscellaneous"));

    //
    // Connections
    //
    connect(archiveMode, SIGNAL(toggled(bool)), this, SLOT(slotArchiveToggled(bool)));
    connect(recurseDirs, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preserveLinks, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preservePerms, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preserveTimes, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preserveGroup, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preserveOwner, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preserveDevices, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(useFFilterRule, SIGNAL(toggled(bool)), this, SLOT(slotFFilterRuleToggled(bool)));
    connect(useFFFilterRule, SIGNAL(toggled(bool)), this, SLOT(slotFFFilterRuleToggled(bool)));
    connect(makeBackups, SIGNAL(toggled(bool)), this, SLOT(slotBackupToggled(bool)));
    connect(compressData, SIGNAL(toggled(bool)), this, SLOT(slotCompressToggled(bool)));
    connect(keepPartial, SIGNAL(toggled(bool)), this, SLOT(slotKeepPartialToggled(bool)));

    slotArchiveToggled(Smb4KSettings::archiveMode());
    slotBackupToggled(Smb4KSettings::makeBackups());
    slotCompressToggled(Smb4KSettings::compressData());
    slotKeepPartialToggled(Smb4KSettings::keepPartial());
}

Smb4KConfigPageSynchronization::~Smb4KConfigPageSynchronization()
{
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigPageSynchronization::slotArchiveToggled(bool checked)
{
    if (checked) {
        findChild<QCheckBox *>("kcfg_RecurseIntoDirectories")->setChecked(checked);
        findChild<QCheckBox *>("kcfg_PreserveSymlinks")->setChecked(checked);
        findChild<QCheckBox *>("kcfg_PreservePermissions")->setChecked(checked);
        findChild<QCheckBox *>("kcfg_PreserveTimes")->setChecked(checked);
        findChild<QCheckBox *>("kcfg_PreserveGroup")->setChecked(checked);
        findChild<QCheckBox *>("kcfg_PreserveOwner")->setChecked(checked);
        findChild<QCheckBox *>("kcfg_PreserveDevicesAndSpecials")->setChecked(checked);
    }
}

void Smb4KConfigPageSynchronization::slotUncheckArchive(bool checked)
{
    if (!checked) {
        findChild<QCheckBox *>("kcfg_ArchiveMode")->setChecked(checked);
    }
}

void Smb4KConfigPageSynchronization::slotBackupToggled(bool checked)
{
    findChild<QCheckBox *>("kcfg_UseBackupDirectory")->setEnabled(checked);
    findChild<KUrlRequester *>("kcfg_BackupDirectory")->setEnabled(checked);
    findChild<QCheckBox *>("kcfg_UseBackupSuffix")->setEnabled(checked);
    findChild<KLineEdit *>("kcfg_BackupSuffix")->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotCompressToggled(bool checked)
{
    findChild<QCheckBox *>("kcfg_UseCompressionLevel")->setEnabled(checked);
    findChild<QSpinBox *>("kcfg_CompressionLevel")->setEnabled(checked);
    findChild<QCheckBox *>("kcfg_UseSkipCompression")->setEnabled(checked);
    findChild<KLineEdit *>("kcfg_SkipCompression")->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotKeepPartialToggled(bool checked)
{
    findChild<QCheckBox *>("kcfg_UsePartialDirectory")->setEnabled(checked);
    findChild<KUrlRequester *>("kcfg_PartialDirectory")->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotFFilterRuleToggled(bool on)
{
    QCheckBox *ff_filter = findChild<QCheckBox *>("kcfg_UseFFFilterRule");

    if (on && ff_filter->isChecked()) {
        ff_filter->setChecked(false);
    }
}

void Smb4KConfigPageSynchronization::slotFFFilterRuleToggled(bool on)
{
    QCheckBox *f_filter = findChild<QCheckBox *>("kcfg_UseFFilterRule");

    if (on && f_filter->isChecked()) {
        f_filter->setChecked(false);
    }
}
