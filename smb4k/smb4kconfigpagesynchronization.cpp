/*
    The configuration page for the synchronization options

    SPDX-FileCopyrightText: 2005-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagesynchronization.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
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

    // Synchronization Directory
    QGroupBox *synchronizationDirectoryBox = new QGroupBox(i18n("Synchronization Directory"), basicTab);
    QVBoxLayout *synchronizationDirectoryBoxLayout = new QVBoxLayout(synchronizationDirectoryBox);

    QWidget *rsyncPrefixWidget = new QWidget(synchronizationDirectoryBox);
    QHBoxLayout *rsyncPrefixWidgetLayout = new QHBoxLayout(rsyncPrefixWidget);
    rsyncPrefixWidgetLayout->setMargin(0);

    QLabel *rsyncPrefixLabel = new QLabel(rsyncPrefixWidget);
    rsyncPrefixLabel->setText(Smb4KSettings::self()->rsyncPrefixItem()->label());
    rsyncPrefixWidgetLayout->addWidget(rsyncPrefixLabel);

    KUrlRequester *rsyncPrefix = new KUrlRequester(rsyncPrefixWidget);
    rsyncPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    rsyncPrefix->setObjectName("kcfg_RsyncPrefix");
    rsyncPrefixWidgetLayout->addWidget(rsyncPrefix);

    rsyncPrefixLabel->setBuddy(rsyncPrefix);

    synchronizationDirectoryBoxLayout->addWidget(rsyncPrefixWidget);

    basicTabLayout->addWidget(synchronizationDirectoryBox);

    // Behavior
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), basicTab);
    QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);

    QCheckBox *archiveMode = new QCheckBox(Smb4KSettings::self()->archiveModeItem()->label(), behaviorBox);
    archiveMode->setObjectName("kcfg_ArchiveMode");

    behaviorBoxLayout->addWidget(archiveMode, 0, 0);

    QCheckBox *recurseDirectories = new QCheckBox(Smb4KSettings::self()->recurseIntoDirectoriesItem()->label(), behaviorBox);
    recurseDirectories->setObjectName("kcfg_RecurseIntoDirectories");

    behaviorBoxLayout->addWidget(recurseDirectories, 0, 1);

    QCheckBox *relativePaths = new QCheckBox(Smb4KSettings::self()->relativePathNamesItem()->label(), behaviorBox);
    relativePaths->setObjectName("kcfg_RelativePathNames");

    behaviorBoxLayout->addWidget(relativePaths, 1, 0);

    QCheckBox *noImpliedDirs = new QCheckBox(Smb4KSettings::self()->noImpliedDirectoriesItem()->label(), behaviorBox);
    noImpliedDirs->setObjectName("kcfg_NoImpliedDirectories");

    behaviorBoxLayout->addWidget(noImpliedDirs, 1, 1);

    QCheckBox *transferDirs = new QCheckBox(Smb4KSettings::self()->transferDirectoriesItem()->label(), behaviorBox);
    transferDirs->setObjectName("kcfg_TransferDirectories");

    behaviorBoxLayout->addWidget(transferDirs, 2, 0);

    basicTabLayout->addWidget(behaviorBox);

    // Backups
    QGroupBox *backupsBox = new QGroupBox(i18n("Backups"), basicTab);
    QVBoxLayout *backupsBoxLayout = new QVBoxLayout(backupsBox);

    QCheckBox *makeBackups = new QCheckBox(Smb4KSettings::self()->makeBackupsItem()->label(), backupsBox);
    makeBackups->setObjectName("kcfg_MakeBackups");

    backupsBoxLayout->addWidget(makeBackups);

    QWidget *backupSuffixWidget = new QWidget(backupsBox);
    QHBoxLayout *backupSuffixLayout = new QHBoxLayout(backupSuffixWidget);
    backupSuffixLayout->setMargin(0);

    QCheckBox *backupSuffixButton = new QCheckBox(Smb4KSettings::self()->useBackupSuffixItem()->label(), backupSuffixWidget);
    backupSuffixButton->setObjectName("kcfg_UseBackupSuffix");
    KLineEdit *backupSuffix = new KLineEdit(backupSuffixWidget);
    backupSuffix->setObjectName("kcfg_BackupSuffix");
    backupSuffix->setClearButtonEnabled(true);

    backupSuffixLayout->addWidget(backupSuffixButton);
    backupSuffixLayout->addWidget(backupSuffix);

    backupsBoxLayout->addWidget(backupSuffixWidget);

    QWidget *backupDirWidget = new QWidget(backupsBox);
    QHBoxLayout *backupDirLayout = new QHBoxLayout(backupDirWidget);
    backupDirLayout->setMargin(0);

    QCheckBox *backupDirButton = new QCheckBox(Smb4KSettings::self()->useBackupDirectoryItem()->label(), backupDirWidget);
    backupDirButton->setObjectName("kcfg_UseBackupDirectory");
    KUrlRequester *backupDir = new KUrlRequester(backupDirWidget);
    backupDir->setObjectName("kcfg_BackupDirectory");
    backupDir->setMode(KFile::Directory | KFile::LocalOnly);

    backupDirLayout->addWidget(backupDirButton);
    backupDirLayout->addWidget(backupDir);

    backupsBoxLayout->addWidget(backupDirWidget);

    basicTabLayout->addWidget(backupsBox);
    basicTabLayout->addStretch(100);

    addTab(basicTab, i18n("Basic Settings"));

    //
    // 'File Handling' tab
    //
    QWidget *fileHandlingTab = new QWidget(this);
    QVBoxLayout *fileHandlingTabLayout = new QVBoxLayout(fileHandlingTab);

    // General
    QGroupBox *generalBox = new QGroupBox(i18n("General"), fileHandlingTab);
    QGridLayout *generalBoxLayout = new QGridLayout(generalBox);

    QCheckBox *updateTarget = new QCheckBox(Smb4KSettings::self()->updateTargetItem()->label(), generalBox);
    updateTarget->setObjectName("kcfg_UpdateTarget");

    generalBoxLayout->addWidget(updateTarget, 0, 0);

    QCheckBox *updateInPlace = new QCheckBox(Smb4KSettings::self()->updateInPlaceItem()->label(), generalBox);
    updateInPlace->setObjectName("kcfg_UpdateInPlace");

    generalBoxLayout->addWidget(updateInPlace, 0, 1);

    QCheckBox *sparseFiles = new QCheckBox(Smb4KSettings::self()->efficientSparseFileHandlingItem()->label(), generalBox);
    sparseFiles->setObjectName("kcfg_EfficientSparseFileHandling");

    generalBoxLayout->addWidget(sparseFiles, 1, 0);

    QCheckBox *copyFilesWhole = new QCheckBox(Smb4KSettings::self()->copyFilesWholeItem()->label(), generalBox);
    copyFilesWhole->setObjectName("kcfg_CopyFilesWhole");

    generalBoxLayout->addWidget(copyFilesWhole, 1, 1);

    QCheckBox *updateExisting = new QCheckBox(Smb4KSettings::self()->updateExistingItem()->label(), generalBox);
    updateExisting->setObjectName("kcfg_UpdateExisting");

    generalBoxLayout->addWidget(updateExisting, 2, 0);

    QCheckBox *ignoreExisting = new QCheckBox(Smb4KSettings::self()->ignoreExistingItem()->label(), generalBox);
    ignoreExisting->setObjectName("kcfg_IgnoreExisting");

    generalBoxLayout->addWidget(ignoreExisting, 2, 1);

    fileHandlingTabLayout->addWidget(generalBox);

    // Links
    QGroupBox *linksBox = new QGroupBox(i18n("Links"), fileHandlingTab);
    QGridLayout *linksBoxLayout = new QGridLayout(linksBox);

    QCheckBox *preserveLinks = new QCheckBox(Smb4KSettings::self()->preserveSymlinksItem()->label(), linksBox);
    preserveLinks->setObjectName("kcfg_PreserveSymlinks");

    linksBoxLayout->addWidget(preserveLinks, 0, 0);

    QCheckBox *transformLinks = new QCheckBox(Smb4KSettings::self()->transformSymlinksItem()->label(), linksBox);
    transformLinks->setObjectName("kcfg_TransformSymlinks");

    linksBoxLayout->addWidget(transformLinks, 0, 1);

    QCheckBox *transformUnsafe = new QCheckBox(Smb4KSettings::self()->transformUnsafeSymlinksItem()->label(), linksBox);
    transformUnsafe->setObjectName("kcfg_TransformUnsafeSymlinks");

    linksBoxLayout->addWidget(transformUnsafe, 1, 0);

    QCheckBox *ignoreUnsafe = new QCheckBox(Smb4KSettings::self()->ignoreUnsafeSymlinksItem()->label(), linksBox);
    ignoreUnsafe->setObjectName("kcfg_IgnoreUnsafeSymlinks");

    linksBoxLayout->addWidget(ignoreUnsafe, 1, 1);

    QCheckBox *mungeLinks = new QCheckBox(Smb4KSettings::self()->mungeSymlinksItem()->label(), linksBox);
    mungeLinks->setObjectName("kcfg_MungeSymlinks");

    linksBoxLayout->addWidget(mungeLinks, 2, 0);

    QCheckBox *preserveHardLinks = new QCheckBox(Smb4KSettings::self()->preserveHardLinksItem()->label(), linksBox);
    preserveHardLinks->setObjectName("kcfg_PreserveHardLinks");

    linksBoxLayout->addWidget(preserveHardLinks, 2, 1);

    QCheckBox *copyDirLinks = new QCheckBox(Smb4KSettings::self()->copyDirectorySymlinksItem()->label(), linksBox);
    ;
    copyDirLinks->setObjectName("kcfg_CopyDirectorySymlinks");

    linksBoxLayout->addWidget(copyDirLinks, 3, 0);

    QCheckBox *keepDirLinks = new QCheckBox(Smb4KSettings::self()->keepDirectorySymlinksItem()->label(), linksBox);
    keepDirLinks->setObjectName("kcfg_KeepDirectorySymlinks");

    linksBoxLayout->addWidget(keepDirLinks, 3, 1);

    fileHandlingTabLayout->addWidget(linksBox);

    // File Attributes and Ownership
    QGroupBox *attributesOwnershipBox = new QGroupBox(i18n("File Attributes && Ownership"), fileHandlingTab);
    QGridLayout *attributesOwnershipBoxLayout = new QGridLayout(attributesOwnershipBox);

    QCheckBox *preservePermissions = new QCheckBox(Smb4KSettings::self()->preservePermissionsItem()->label(), attributesOwnershipBox);
    preservePermissions->setObjectName("kcfg_PreservePermissions");

    attributesOwnershipBoxLayout->addWidget(preservePermissions, 0, 0);

    QCheckBox *preserveDevices = new QCheckBox(Smb4KSettings::self()->preserveDevicesAndSpecialsItem()->label(), attributesOwnershipBox);
    preserveDevices->setObjectName("kcfg_PreserveDevicesAndSpecials");

    attributesOwnershipBoxLayout->addWidget(preserveDevices, 0, 1);

    QCheckBox *preserveTimes = new QCheckBox(Smb4KSettings::self()->preserveTimesItem()->label(), attributesOwnershipBox);
    preserveTimes->setObjectName("kcfg_PreserveTimes");

    attributesOwnershipBoxLayout->addWidget(preserveTimes, 1, 0);

    QCheckBox *omitDirectoriesTimes = new QCheckBox(Smb4KSettings::self()->omitDirectoryTimesItem()->label(), attributesOwnershipBox);
    omitDirectoriesTimes->setObjectName("kcfg_OmitDirectoryTimes");

    attributesOwnershipBoxLayout->addWidget(omitDirectoriesTimes, 1, 1);

    QCheckBox *preserveOwner = new QCheckBox(Smb4KSettings::self()->preserveOwnerItem()->label(), attributesOwnershipBox);
    preserveOwner->setObjectName("kcfg_PreserveOwner");

    attributesOwnershipBoxLayout->addWidget(preserveOwner, 2, 0);

    QCheckBox *preserveGroup = new QCheckBox(Smb4KSettings::self()->preserveGroupItem()->label(), attributesOwnershipBox);
    preserveGroup->setObjectName("kcfg_PreserveGroup");

    attributesOwnershipBoxLayout->addWidget(preserveGroup, 2, 1);

    fileHandlingTabLayout->addWidget(attributesOwnershipBox);
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

    compressionBoxLayout->addWidget(compressData, 0, 0, 1, 2);

    QCheckBox *compressionLevelButton = new QCheckBox(Smb4KSettings::self()->useCompressionLevelItem()->label(), compressionBox);
    compressionLevelButton->setObjectName("kcfg_UseCompressionLevel");

    compressionBoxLayout->addWidget(compressionLevelButton, 1, 0);

    QSpinBox *compressionLevel = new QSpinBox(compressionBox);
    compressionLevel->setObjectName("kcfg_CompressionLevel");

    compressionBoxLayout->addWidget(compressionLevel, 1, 1);

    QCheckBox *skipCompressionButton = new QCheckBox(Smb4KSettings::self()->useSkipCompressionItem()->label(), compressionBox);
    skipCompressionButton->setObjectName("kcfg_UseSkipCompression");

    compressionBoxLayout->addWidget(skipCompressionButton, 2, 0);

    KLineEdit *skipCompression = new KLineEdit(compressionBox);
    skipCompression->setObjectName("kcfg_SkipCompression");
    skipCompression->setClearButtonEnabled(true);

    compressionBoxLayout->addWidget(skipCompression, 2, 1);

    transferTabLayout->addWidget(compressionBox);

    // Files
    QGroupBox *filesBox = new QGroupBox(i18n("Files"), transferTab);
    QGridLayout *filesBoxLayout = new QGridLayout(filesBox);

    QCheckBox *minTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMinimalTransferSizeItem()->label(), filesBox);
    minTransferSizeButton->setObjectName("kcfg_UseMinimalTransferSize");

    filesBoxLayout->addWidget(minTransferSizeButton, 0, 0);

    QSpinBox *minTransferSize = new QSpinBox(filesBox);
    minTransferSize->setObjectName("kcfg_MinimalTransferSize");
    minTransferSize->setSuffix(i18n(" KiB"));

    filesBoxLayout->addWidget(minTransferSize, 0, 1);

    QCheckBox *maxTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMaximalTransferSizeItem()->label(), filesBox);
    maxTransferSizeButton->setObjectName("kcfg_UseMaximalTransferSize");

    filesBoxLayout->addWidget(maxTransferSizeButton, 1, 0);

    QSpinBox *maxTransferSize = new QSpinBox(filesBox);
    maxTransferSize->setObjectName("kcfg_MaximalTransferSize");
    maxTransferSize->setSuffix(i18n(" KiB"));

    filesBoxLayout->addWidget(maxTransferSize, 1, 1);

    QCheckBox *keepPartial = new QCheckBox(Smb4KSettings::self()->keepPartialItem()->label(), filesBox);
    keepPartial->setObjectName("kcfg_KeepPartial");

    filesBoxLayout->addWidget(keepPartial, 2, 0, 1, 2);

    QCheckBox *partialDirButton = new QCheckBox(Smb4KSettings::self()->usePartialDirectoryItem()->label(), filesBox);
    partialDirButton->setObjectName("kcfg_UsePartialDirectory");

    filesBoxLayout->addWidget(partialDirButton, 3, 0);

    KUrlRequester *partialDir = new KUrlRequester(filesBox);
    partialDir->setObjectName("kcfg_PartialDirectory");
    partialDir->setMode(KFile::Directory | KFile::LocalOnly);

    filesBoxLayout->addWidget(partialDir, 3, 1);

    transferTabLayout->addWidget(filesBox);

    // Miscellaneous
    QGroupBox *miscellaneousBox = new QGroupBox(transferTab);
    QGridLayout *miscellaneousBoxLayout = new QGridLayout(miscellaneousBox);

    QCheckBox *bandwidthLimitButton = new QCheckBox(Smb4KSettings::self()->useBandwidthLimitItem()->label(), miscellaneousBox);
    bandwidthLimitButton->setObjectName("kcfg_UseBandwidthLimit");

    miscellaneousBoxLayout->addWidget(bandwidthLimitButton, 0, 0);

    QSpinBox *bandwidthLimit = new QSpinBox(miscellaneousBox);
    bandwidthLimit->setObjectName("kcfg_BandwidthLimit");
    bandwidthLimit->setSuffix(i18n(" KiB/s"));

    miscellaneousBoxLayout->addWidget(bandwidthLimit, 0, 1);

    transferTabLayout->addWidget(miscellaneousBox);
    transferTabLayout->addStretch(100);

    addTab(transferTab, i18n("File Transfer"));

    //
    // 'File Deletion' tab
    //
    QWidget *deleteTab = new QWidget(this);
    QVBoxLayout *deleteTabLayout = new QVBoxLayout(deleteTab);

    // Files and Directories
    QGroupBox *filesAndDirectoriesBox = new QGroupBox(i18n("Files && Directories"), deleteTab);
    QGridLayout *filesAndDirectoriesBoxLayout = new QGridLayout(filesAndDirectoriesBox);    
    
    QCheckBox *removeSource = new QCheckBox(Smb4KSettings::self()->removeSourceFilesItem()->label(), filesAndDirectoriesBox);
    removeSource->setObjectName("kcfg_RemoveSourceFiles");

    filesAndDirectoriesBoxLayout->addWidget(removeSource, 0, 0);
    
    QCheckBox *deleteExtraneous = new QCheckBox(Smb4KSettings::self()->deleteExtraneousItem()->label(), filesAndDirectoriesBox);
    deleteExtraneous->setObjectName("kcfg_DeleteExtraneous");

    filesAndDirectoriesBoxLayout->addWidget(deleteExtraneous, 0, 1);

    QCheckBox *deleteBefore = new QCheckBox(Smb4KSettings::self()->deleteBeforeItem()->label(), filesAndDirectoriesBox);
    deleteBefore->setObjectName("kcfg_DeleteBefore");

    filesAndDirectoriesBoxLayout->addWidget(deleteBefore, 1, 0);

    QCheckBox *deleteAfter = new QCheckBox(Smb4KSettings::self()->deleteAfterItem()->label(), filesAndDirectoriesBox);
    deleteAfter->setObjectName("kcfg_DeleteAfter");

    filesAndDirectoriesBoxLayout->addWidget(deleteAfter, 1, 1);

    QCheckBox *deleteDuring = new QCheckBox(Smb4KSettings::self()->deleteDuringItem()->label(), filesAndDirectoriesBox);
    deleteDuring->setObjectName("kcfg_DeleteDuring");

    filesAndDirectoriesBoxLayout->addWidget(deleteDuring, 2, 0);

    QCheckBox *deleteExcluded = new QCheckBox(Smb4KSettings::self()->deleteExcludedItem()->label(), filesAndDirectoriesBox);
    deleteExcluded->setObjectName("kcfg_DeleteExcluded");

    filesAndDirectoriesBoxLayout->addWidget(deleteExcluded, 2, 1);

    QCheckBox *ignoreIoErrors = new QCheckBox(Smb4KSettings::self()->ignoreErrorsItem()->label(), filesAndDirectoriesBox);
    ignoreIoErrors->setObjectName("kcfg_IgnoreErrors");

    filesAndDirectoriesBoxLayout->addWidget(ignoreIoErrors, 3, 0);

    QCheckBox *forceDirDeletion = new QCheckBox(Smb4KSettings::self()->forceDirectoryDeletionItem()->label(), filesAndDirectoriesBox);
    forceDirDeletion->setObjectName("kcfg_ForceDirectoryDeletion");

    filesAndDirectoriesBoxLayout->addWidget(forceDirDeletion, 3, 1);
    
    deleteTabLayout->addWidget(filesAndDirectoriesBox);

    // Restrictions
    QGroupBox *restrictionsBox = new QGroupBox(i18n("Restrictions"), deleteTab);
    QGridLayout *restrictionsBoxLayout = new QGridLayout(restrictionsBox);
    
    QCheckBox *maximumDeleteButton = new QCheckBox(Smb4KSettings::self()->useMaximumDeleteItem()->label(), restrictionsBox);
    maximumDeleteButton->setObjectName("kcfg_UseMaximumDelete");
    
    restrictionsBoxLayout->addWidget(maximumDeleteButton, 0, 0);
    
    QSpinBox *maximumDelete = new QSpinBox(restrictionsBox);
    maximumDelete->setObjectName("kcfg_MaximumDeleteValue");
    
    restrictionsBoxLayout->addWidget(maximumDelete, 0, 1);

    deleteTabLayout->addWidget(restrictionsBox);
    deleteTabLayout->addStretch(100);

    addTab(deleteTab, i18n("File Deletion"));

    //
    // 'Filter' tab
    //
    QWidget *filterTab = new QWidget(this);
    QVBoxLayout *filterTabLayout = new QVBoxLayout(filterTab);

    // General
    QGroupBox *generalFilteringBox = new QGroupBox(i18n("General Filtering Settings"), filterTab);
    QGridLayout *generalFilteringBoxLayout = new QGridLayout(generalFilteringBox);
    
    QCheckBox *cvsExclude = new QCheckBox(Smb4KSettings::self()->useCVSExcludeItem()->label(), generalFilteringBox);
    cvsExclude->setObjectName("kcfg_UseCVSExclude");
    
    generalFilteringBoxLayout->addWidget(cvsExclude, 0, 0, 1, 2);
    
    QCheckBox *excludePatternButton = new QCheckBox(Smb4KSettings::self()->useExcludePatternItem()->label(), generalFilteringBox);
    excludePatternButton->setObjectName("kcfg_UseExcludePattern");
    
    generalFilteringBoxLayout->addWidget(excludePatternButton, 1, 0);
    
    KLineEdit *excludePattern = new KLineEdit(generalFilteringBox);
    excludePattern->setObjectName("kcfg_ExcludePattern");
    excludePattern->setClearButtonEnabled(true);
    
    generalFilteringBoxLayout->addWidget(excludePattern, 1, 1);

    QCheckBox *excludeFromButton = new QCheckBox(Smb4KSettings::self()->useExcludeFromItem()->label(), generalFilteringBox);
    excludeFromButton->setObjectName("kcfg_UseExcludeFrom");
    
    generalFilteringBoxLayout->addWidget(excludeFromButton, 2, 0);
    
    KUrlRequester *excludeFrom = new KUrlRequester(generalFilteringBox);
    excludeFrom->setObjectName("kcfg_ExcludeFrom");
    excludeFrom->setMode(KFile::File | KFile::LocalOnly);
    
    generalFilteringBoxLayout->addWidget(excludeFrom, 2, 1);

    QCheckBox *includePatternButton = new QCheckBox(Smb4KSettings::self()->useIncludePatternItem()->label(), generalFilteringBox);
    includePatternButton->setObjectName("kcfg_UseIncludePattern");
    
    generalFilteringBoxLayout->addWidget(includePatternButton, 3, 0);
    
    KLineEdit *includePattern = new KLineEdit(generalFilteringBox);
    includePattern->setObjectName("kcfg_IncludePattern");
    includePattern->setClearButtonEnabled(true);
    
    generalFilteringBoxLayout->addWidget(includePattern, 3, 1);

    QCheckBox *includeFromButton = new QCheckBox(Smb4KSettings::self()->useIncludeFromItem()->label(), generalFilteringBox);
    includeFromButton->setObjectName("kcfg_UseIncludeFrom");
    
    generalFilteringBoxLayout->addWidget(includeFromButton, 4, 0);
    
    KUrlRequester *includeFrom = new KUrlRequester(generalFilteringBox);
    includeFrom->setObjectName("kcfg_IncludeFrom");
    includeFrom->setMode(KFile::File | KFile::LocalOnly);
    
    generalFilteringBoxLayout->addWidget(includeFrom, 4, 1);
    
    filterTabLayout->addWidget(generalFilteringBox);

    // Filter rules
    QGroupBox *filterRulesBox = new QGroupBox(i18n("Filter Rules"), filterTab);
    QGridLayout *filterRulesBoxLayout = new QGridLayout(filterRulesBox);
    
    QCheckBox *useFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFilterRuleItem()->label(), filterRulesBox);
    useFFilterRule->setObjectName("kcfg_UseFFilterRule");
    
    filterRulesBoxLayout->addWidget(useFFilterRule, 0, 0, 1, 2);

    QCheckBox *useFFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFFilterRuleItem()->label(), filterRulesBox);
    useFFFilterRule->setObjectName("kcfg_UseFFFilterRule");
    
    filterRulesBoxLayout->addWidget(useFFFilterRule, 1, 0, 1, 2);

    QLabel *customFilterRulesLabel = new QLabel(Smb4KSettings::self()->customFilteringRulesItem()->label(), filterRulesBox);
    
    filterRulesBoxLayout->addWidget(customFilterRulesLabel, 2, 0);
    
    KLineEdit *customFilterRules = new KLineEdit(filterTab);
    customFilterRules->setObjectName("kcfg_CustomFilteringRules");
    customFilterRules->setClearButtonEnabled(true);

    filterRulesBoxLayout->addWidget(customFilterRules, 2, 1);
    
    filterTabLayout->addWidget(filterRulesBox);
    filterTabLayout->addStretch(100);

    addTab(filterTab, i18n("Filtering"));

    //
    // 'Miscellaneous' tab
    //
    QWidget *miscellaneousTab = new QWidget(this);
    QVBoxLayout *miscellaneousTabLayout = new QVBoxLayout(miscellaneousTab);

    // Checksums
    QGroupBox *checksumsBox = new QGroupBox(i18n("Checksums"), miscellaneousTab);
    QGridLayout *checksumsBoxLayout = new QGridLayout(checksumsBox);
    
    QCheckBox *blockSizeButton = new QCheckBox(Smb4KSettings::self()->useBlockSizeItem()->label(), checksumsBox);
    blockSizeButton->setObjectName("kcfg_UseBlockSize");
    
    checksumsBoxLayout->addWidget(blockSizeButton, 0, 0);

    QSpinBox *blockSize = new QSpinBox(checksumsBox);
    blockSize->setObjectName("kcfg_BlockSize");
    
    checksumsBoxLayout->addWidget(blockSize, 0, 1);

    QCheckBox *checksumSeedButton = new QCheckBox(Smb4KSettings::self()->useChecksumSeedItem()->label(), checksumsBox);
    checksumSeedButton->setObjectName("kcfg_UseChecksumSeed");
    
    checksumsBoxLayout->addWidget(checksumSeedButton, 1, 0);
    
    QSpinBox *checksumSeed = new QSpinBox(checksumsBox);
    checksumSeed->setObjectName("kcfg_ChecksumSeed");
    
    checksumsBoxLayout->addWidget(checksumSeed, 1, 1);

    QCheckBox *useChecksum = new QCheckBox(Smb4KSettings::self()->useChecksumItem()->label(), checksumsBox);
    useChecksum->setObjectName("kcfg_UseChecksum");
    
    checksumsBoxLayout->addWidget(useChecksum, 2, 0, 1, 2);
    
    miscellaneousTabLayout->addWidget(checksumsBox);

    // Miscellaneous
    QGroupBox *miscellaneousBox2 = new QGroupBox(i18n("Miscellaneous"), miscellaneousTab);
    QGridLayout *miscellaneousBox2Layout = new QGridLayout(miscellaneousBox2);
    
    QCheckBox *oneFilesystem = new QCheckBox(Smb4KSettings::self()->oneFileSystemItem()->label(), miscellaneousBox2);
    oneFilesystem->setObjectName("kcfg_OneFileSystem");
    
    miscellaneousBox2Layout->addWidget(oneFilesystem, 0, 0);

    QCheckBox *delayUpdates = new QCheckBox(Smb4KSettings::self()->delayUpdatesItem()->label(), miscellaneousBox2);
    delayUpdates->setObjectName("kcfg_DelayUpdates");

    miscellaneousBox2Layout->addWidget(delayUpdates, 0, 1);
    
    miscellaneousTabLayout->addWidget(miscellaneousBox2);
    miscellaneousTabLayout->addStretch(100);
    
    addTab(miscellaneousTab, i18n("Miscellaneous"));

    //
    // Connections
    //
    connect(archiveMode, SIGNAL(toggled(bool)), this, SLOT(slotArchiveToggled(bool)));
    connect(recurseDirectories, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preserveLinks, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
    connect(preservePermissions, SIGNAL(toggled(bool)), this, SLOT(slotUncheckArchive(bool)));
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
