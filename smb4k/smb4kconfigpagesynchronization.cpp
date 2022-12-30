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
    rsyncPrefixWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *rsyncPrefixLabel = new QLabel(rsyncPrefixWidget);
    rsyncPrefixLabel->setText(Smb4KSettings::self()->rsyncPrefixItem()->label());
    rsyncPrefixWidgetLayout->addWidget(rsyncPrefixLabel);

    KUrlRequester *rsyncPrefix = new KUrlRequester(rsyncPrefixWidget);
    rsyncPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    rsyncPrefix->setObjectName(QStringLiteral("kcfg_RsyncPrefix"));
    rsyncPrefixWidgetLayout->addWidget(rsyncPrefix);

    rsyncPrefixLabel->setBuddy(rsyncPrefix);

    synchronizationDirectoryBoxLayout->addWidget(rsyncPrefixWidget);

    basicTabLayout->addWidget(synchronizationDirectoryBox);

    // Behavior
    QGroupBox *behaviorBox = new QGroupBox(i18n("Behavior"), basicTab);
    QGridLayout *behaviorBoxLayout = new QGridLayout(behaviorBox);

    QCheckBox *archiveMode = new QCheckBox(Smb4KSettings::self()->archiveModeItem()->label(), behaviorBox);
    archiveMode->setObjectName(QStringLiteral("kcfg_ArchiveMode"));

    behaviorBoxLayout->addWidget(archiveMode, 0, 0);

    QCheckBox *recurseDirectories = new QCheckBox(Smb4KSettings::self()->recurseIntoDirectoriesItem()->label(), behaviorBox);
    recurseDirectories->setObjectName(QStringLiteral("kcfg_RecurseIntoDirectories"));

    behaviorBoxLayout->addWidget(recurseDirectories, 0, 1);

    QCheckBox *relativePaths = new QCheckBox(Smb4KSettings::self()->relativePathNamesItem()->label(), behaviorBox);
    relativePaths->setObjectName(QStringLiteral("kcfg_RelativePathNames"));

    behaviorBoxLayout->addWidget(relativePaths, 1, 0);

    QCheckBox *noImpliedDirs = new QCheckBox(Smb4KSettings::self()->noImpliedDirectoriesItem()->label(), behaviorBox);
    noImpliedDirs->setObjectName(QStringLiteral("kcfg_NoImpliedDirectories"));

    behaviorBoxLayout->addWidget(noImpliedDirs, 1, 1);

    QCheckBox *transferDirs = new QCheckBox(Smb4KSettings::self()->transferDirectoriesItem()->label(), behaviorBox);
    transferDirs->setObjectName(QStringLiteral("kcfg_TransferDirectories"));

    behaviorBoxLayout->addWidget(transferDirs, 2, 0);

    basicTabLayout->addWidget(behaviorBox);

    // Backups
    QGroupBox *backupsBox = new QGroupBox(i18n("Backups"), basicTab);
    QVBoxLayout *backupsBoxLayout = new QVBoxLayout(backupsBox);

    QCheckBox *makeBackups = new QCheckBox(Smb4KSettings::self()->makeBackupsItem()->label(), backupsBox);
    makeBackups->setObjectName(QStringLiteral("kcfg_MakeBackups"));

    backupsBoxLayout->addWidget(makeBackups);

    QWidget *backupSuffixWidget = new QWidget(backupsBox);
    QHBoxLayout *backupSuffixLayout = new QHBoxLayout(backupSuffixWidget);
    backupSuffixLayout->setContentsMargins(0, 0, 0, 0);

    QCheckBox *backupSuffixButton = new QCheckBox(Smb4KSettings::self()->useBackupSuffixItem()->label(), backupSuffixWidget);
    backupSuffixButton->setObjectName(QStringLiteral("kcfg_UseBackupSuffix"));
    KLineEdit *backupSuffix = new KLineEdit(backupSuffixWidget);
    backupSuffix->setObjectName(QStringLiteral("kcfg_BackupSuffix"));
    backupSuffix->setClearButtonEnabled(true);

    backupSuffixLayout->addWidget(backupSuffixButton);
    backupSuffixLayout->addWidget(backupSuffix);

    backupsBoxLayout->addWidget(backupSuffixWidget);

    QWidget *backupDirWidget = new QWidget(backupsBox);
    QHBoxLayout *backupDirLayout = new QHBoxLayout(backupDirWidget);
    backupDirLayout->setContentsMargins(0, 0, 0, 0);

    QCheckBox *backupDirButton = new QCheckBox(Smb4KSettings::self()->useBackupDirectoryItem()->label(), backupDirWidget);
    backupDirButton->setObjectName(QStringLiteral("kcfg_UseBackupDirectory"));
    KUrlRequester *backupDir = new KUrlRequester(backupDirWidget);
    backupDir->setObjectName(QStringLiteral("kcfg_BackupDirectory"));
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
    updateTarget->setObjectName(QStringLiteral("kcfg_UpdateTarget"));

    generalBoxLayout->addWidget(updateTarget, 0, 0);

    QCheckBox *updateInPlace = new QCheckBox(Smb4KSettings::self()->updateInPlaceItem()->label(), generalBox);
    updateInPlace->setObjectName(QStringLiteral("kcfg_UpdateInPlace"));

    generalBoxLayout->addWidget(updateInPlace, 0, 1);

    QCheckBox *sparseFiles = new QCheckBox(Smb4KSettings::self()->efficientSparseFileHandlingItem()->label(), generalBox);
    sparseFiles->setObjectName(QStringLiteral("kcfg_EfficientSparseFileHandling"));

    generalBoxLayout->addWidget(sparseFiles, 1, 0);

    QCheckBox *copyFilesWhole = new QCheckBox(Smb4KSettings::self()->copyFilesWholeItem()->label(), generalBox);
    copyFilesWhole->setObjectName(QStringLiteral("kcfg_CopyFilesWhole"));

    generalBoxLayout->addWidget(copyFilesWhole, 1, 1);

    QCheckBox *updateExisting = new QCheckBox(Smb4KSettings::self()->updateExistingItem()->label(), generalBox);
    updateExisting->setObjectName(QStringLiteral("kcfg_UpdateExisting"));

    generalBoxLayout->addWidget(updateExisting, 2, 0);

    QCheckBox *ignoreExisting = new QCheckBox(Smb4KSettings::self()->ignoreExistingItem()->label(), generalBox);
    ignoreExisting->setObjectName(QStringLiteral("kcfg_IgnoreExisting"));

    generalBoxLayout->addWidget(ignoreExisting, 2, 1);

    fileHandlingTabLayout->addWidget(generalBox);

    // Links
    QGroupBox *linksBox = new QGroupBox(i18n("Links"), fileHandlingTab);
    QGridLayout *linksBoxLayout = new QGridLayout(linksBox);

    QCheckBox *preserveLinks = new QCheckBox(Smb4KSettings::self()->preserveSymlinksItem()->label(), linksBox);
    preserveLinks->setObjectName(QStringLiteral("kcfg_PreserveSymlinks"));

    linksBoxLayout->addWidget(preserveLinks, 0, 0);

    QCheckBox *transformLinks = new QCheckBox(Smb4KSettings::self()->transformSymlinksItem()->label(), linksBox);
    transformLinks->setObjectName(QStringLiteral("kcfg_TransformSymlinks"));

    linksBoxLayout->addWidget(transformLinks, 0, 1);

    QCheckBox *transformUnsafe = new QCheckBox(Smb4KSettings::self()->transformUnsafeSymlinksItem()->label(), linksBox);
    transformUnsafe->setObjectName(QStringLiteral("kcfg_TransformUnsafeSymlinks"));

    linksBoxLayout->addWidget(transformUnsafe, 1, 0);

    QCheckBox *ignoreUnsafe = new QCheckBox(Smb4KSettings::self()->ignoreUnsafeSymlinksItem()->label(), linksBox);
    ignoreUnsafe->setObjectName(QStringLiteral("kcfg_IgnoreUnsafeSymlinks"));

    linksBoxLayout->addWidget(ignoreUnsafe, 1, 1);

    QCheckBox *mungeLinks = new QCheckBox(Smb4KSettings::self()->mungeSymlinksItem()->label(), linksBox);
    mungeLinks->setObjectName(QStringLiteral("kcfg_MungeSymlinks"));

    linksBoxLayout->addWidget(mungeLinks, 2, 0);

    QCheckBox *preserveHardLinks = new QCheckBox(Smb4KSettings::self()->preserveHardLinksItem()->label(), linksBox);
    preserveHardLinks->setObjectName(QStringLiteral("kcfg_PreserveHardLinks"));

    linksBoxLayout->addWidget(preserveHardLinks, 2, 1);

    QCheckBox *copyDirLinks = new QCheckBox(Smb4KSettings::self()->copyDirectorySymlinksItem()->label(), linksBox);
    ;
    copyDirLinks->setObjectName(QStringLiteral("kcfg_CopyDirectorySymlinks"));

    linksBoxLayout->addWidget(copyDirLinks, 3, 0);

    QCheckBox *keepDirLinks = new QCheckBox(Smb4KSettings::self()->keepDirectorySymlinksItem()->label(), linksBox);
    keepDirLinks->setObjectName(QStringLiteral("kcfg_KeepDirectorySymlinks"));

    linksBoxLayout->addWidget(keepDirLinks, 3, 1);

    fileHandlingTabLayout->addWidget(linksBox);
    fileHandlingTabLayout->addStretch(100);

    addTab(fileHandlingTab, i18n("File Handling"));

    //
    // 'File Attributes and Ownership' tab
    //
    QWidget *attributesAndOwnershipTab = new QWidget(this);
    QVBoxLayout *attributesAndOwnershipTabLayout = new QVBoxLayout(attributesAndOwnershipTab);

    QGroupBox *attributesBox = new QGroupBox(i18n("File Attributes"), attributesAndOwnershipTab);
    QGridLayout *attributesBoxLayout = new QGridLayout(attributesBox);

    QCheckBox *preservePermissions = new QCheckBox(Smb4KSettings::self()->preservePermissionsItem()->label(), attributesBox);
    preservePermissions->setObjectName(QStringLiteral("kcfg_PreservePermissions"));

    attributesBoxLayout->addWidget(preservePermissions, 0, 0);

    QCheckBox *preserveACLs = new QCheckBox(Smb4KSettings::self()->preserveACLsItem()->label(), attributesBox);
    preserveACLs->setObjectName(QStringLiteral("kcfg_PreserveACLs"));

    attributesBoxLayout->addWidget(preserveACLs, 0, 1);

    QCheckBox *preserveExtendedArrtibutes = new QCheckBox(Smb4KSettings::self()->preserveExtendedAttributesItem()->label(), attributesBox);
    preserveExtendedArrtibutes->setObjectName(QStringLiteral("kcfg_PreserveExtendedAttributes"));

    attributesBoxLayout->addWidget(preserveExtendedArrtibutes, 1, 0);

    QCheckBox *preserveAccessTimes = new QCheckBox(Smb4KSettings::self()->preserveAccessTimesItem()->label(), attributesBox);
    preserveAccessTimes->setObjectName(QStringLiteral("kcfg_PreserveAccessTimes"));

    attributesBoxLayout->addWidget(preserveAccessTimes, 1, 1);

    QCheckBox *preserveCreateTimes = new QCheckBox(Smb4KSettings::self()->preserveCreateTimesItem()->label(), attributesBox);
    preserveCreateTimes->setObjectName(QStringLiteral("kcfg_PreserveCreateTimes"));

    attributesBoxLayout->addWidget(preserveCreateTimes, 2, 0);

    QCheckBox *preserveDevices = new QCheckBox(Smb4KSettings::self()->preserveDevicesAndSpecialsItem()->label(), attributesBox);
    preserveDevices->setObjectName(QStringLiteral("kcfg_PreserveDevicesAndSpecials"));

    attributesBoxLayout->addWidget(preserveDevices, 2, 1);

    QCheckBox *preserveTimes = new QCheckBox(Smb4KSettings::self()->preserveTimesItem()->label(), attributesBox);
    preserveTimes->setObjectName(QStringLiteral("kcfg_PreserveTimes"));

    attributesBoxLayout->addWidget(preserveTimes, 3, 0);

    QCheckBox *omitDirectoriesTimes = new QCheckBox(Smb4KSettings::self()->omitDirectoryTimesItem()->label(), attributesBox);
    omitDirectoriesTimes->setObjectName(QStringLiteral("kcfg_OmitDirectoryTimes"));

    attributesBoxLayout->addWidget(omitDirectoriesTimes, 3, 1);

    attributesAndOwnershipTabLayout->addWidget(attributesBox);

    QGroupBox *ownershipBox = new QGroupBox(i18n("Ownership"), attributesAndOwnershipTab);
    QGridLayout *ownershipBoxLayout = new QGridLayout(ownershipBox);

    QCheckBox *preserveOwner = new QCheckBox(Smb4KSettings::self()->preserveOwnerItem()->label(), ownershipBox);
    preserveOwner->setObjectName(QStringLiteral("kcfg_PreserveOwner"));

    ownershipBoxLayout->addWidget(preserveOwner, 1, 0);

    QCheckBox *preserveGroup = new QCheckBox(Smb4KSettings::self()->preserveGroupItem()->label(), ownershipBox);
    preserveGroup->setObjectName(QStringLiteral("kcfg_PreserveGroup"));

    ownershipBoxLayout->addWidget(preserveGroup, 1, 1);

    attributesAndOwnershipTabLayout->addWidget(ownershipBox);
    attributesAndOwnershipTabLayout->addStretch(100);

    addTab(attributesAndOwnershipTab, i18n("File Attributes && Ownership"));

    //
    // 'File Transfer' tab
    //
    QWidget *transferTab = new QWidget(this);
    QVBoxLayout *transferTabLayout = new QVBoxLayout(transferTab);

    // Compression
    QGroupBox *compressionBox = new QGroupBox(i18n("Compression"), transferTab);
    QGridLayout *compressionBoxLayout = new QGridLayout(compressionBox);

    QCheckBox *compressData = new QCheckBox(Smb4KSettings::self()->compressDataItem()->label(), compressionBox);
    compressData->setObjectName(QStringLiteral("kcfg_CompressData"));

    compressionBoxLayout->addWidget(compressData, 0, 0, 1, 2);

    QCheckBox *compressionLevelButton = new QCheckBox(Smb4KSettings::self()->useCompressionLevelItem()->label(), compressionBox);
    compressionLevelButton->setObjectName(QStringLiteral("kcfg_UseCompressionLevel"));

    compressionBoxLayout->addWidget(compressionLevelButton, 1, 0);

    QSpinBox *compressionLevel = new QSpinBox(compressionBox);
    compressionLevel->setObjectName(QStringLiteral("kcfg_CompressionLevel"));

    compressionBoxLayout->addWidget(compressionLevel, 1, 1);

    QCheckBox *skipCompressionButton = new QCheckBox(Smb4KSettings::self()->useSkipCompressionItem()->label(), compressionBox);
    skipCompressionButton->setObjectName(QStringLiteral("kcfg_UseSkipCompression"));

    compressionBoxLayout->addWidget(skipCompressionButton, 2, 0);

    KLineEdit *skipCompression = new KLineEdit(compressionBox);
    skipCompression->setObjectName(QStringLiteral("kcfg_SkipCompression"));
    skipCompression->setClearButtonEnabled(true);

    compressionBoxLayout->addWidget(skipCompression, 2, 1);

    transferTabLayout->addWidget(compressionBox);

    // Files
    QGroupBox *filesBox = new QGroupBox(i18n("Files"), transferTab);
    QGridLayout *filesBoxLayout = new QGridLayout(filesBox);

    QCheckBox *minTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMinimalTransferSizeItem()->label(), filesBox);
    minTransferSizeButton->setObjectName(QStringLiteral("kcfg_UseMinimalTransferSize"));

    filesBoxLayout->addWidget(minTransferSizeButton, 0, 0);

    QSpinBox *minTransferSize = new QSpinBox(filesBox);
    minTransferSize->setObjectName(QStringLiteral("kcfg_MinimalTransferSize"));
    minTransferSize->setSuffix(i18n(" kB"));

    filesBoxLayout->addWidget(minTransferSize, 0, 1);

    QCheckBox *maxTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMaximalTransferSizeItem()->label(), filesBox);
    maxTransferSizeButton->setObjectName(QStringLiteral("kcfg_UseMaximalTransferSize"));

    filesBoxLayout->addWidget(maxTransferSizeButton, 1, 0);

    QSpinBox *maxTransferSize = new QSpinBox(filesBox);
    maxTransferSize->setObjectName(QStringLiteral("kcfg_MaximalTransferSize"));
    maxTransferSize->setSuffix(i18n(" kB"));

    filesBoxLayout->addWidget(maxTransferSize, 1, 1);

    QCheckBox *keepPartial = new QCheckBox(Smb4KSettings::self()->keepPartialItem()->label(), filesBox);
    keepPartial->setObjectName(QStringLiteral("kcfg_KeepPartial"));

    filesBoxLayout->addWidget(keepPartial, 2, 0, 1, 2);

    QCheckBox *partialDirButton = new QCheckBox(Smb4KSettings::self()->usePartialDirectoryItem()->label(), filesBox);
    partialDirButton->setObjectName(QStringLiteral("kcfg_UsePartialDirectory"));

    filesBoxLayout->addWidget(partialDirButton, 3, 0);

    KUrlRequester *partialDir = new KUrlRequester(filesBox);
    partialDir->setObjectName(QStringLiteral("kcfg_PartialDirectory"));
    partialDir->setMode(KFile::Directory | KFile::LocalOnly);

    filesBoxLayout->addWidget(partialDir, 3, 1);

    transferTabLayout->addWidget(filesBox);

    // Restrictions
    QGroupBox *miscellaneousBox = new QGroupBox(i18n("Miscellaneous"), transferTab);
    QGridLayout *miscellaneousBoxLayout = new QGridLayout(miscellaneousBox);

    QCheckBox *bandwidthLimitButton = new QCheckBox(Smb4KSettings::self()->useBandwidthLimitItem()->label(), miscellaneousBox);
    bandwidthLimitButton->setObjectName(QStringLiteral("kcfg_UseBandwidthLimit"));

    miscellaneousBoxLayout->addWidget(bandwidthLimitButton, 0, 0);

    QSpinBox *bandwidthLimit = new QSpinBox(miscellaneousBox);
    bandwidthLimit->setObjectName(QStringLiteral("kcfg_BandwidthLimit"));
    bandwidthLimit->setSuffix(i18n(" kB/s"));

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
    removeSource->setObjectName(QStringLiteral("kcfg_RemoveSourceFiles"));

    filesAndDirectoriesBoxLayout->addWidget(removeSource, 0, 0);

    QCheckBox *deleteExtraneous = new QCheckBox(Smb4KSettings::self()->deleteExtraneousItem()->label(), filesAndDirectoriesBox);
    deleteExtraneous->setObjectName(QStringLiteral("kcfg_DeleteExtraneous"));

    filesAndDirectoriesBoxLayout->addWidget(deleteExtraneous, 0, 1);

    QCheckBox *deleteBefore = new QCheckBox(Smb4KSettings::self()->deleteBeforeItem()->label(), filesAndDirectoriesBox);
    deleteBefore->setObjectName(QStringLiteral("kcfg_DeleteBefore"));

    filesAndDirectoriesBoxLayout->addWidget(deleteBefore, 1, 0);

    QCheckBox *deleteAfter = new QCheckBox(Smb4KSettings::self()->deleteAfterItem()->label(), filesAndDirectoriesBox);
    deleteAfter->setObjectName(QStringLiteral("kcfg_DeleteAfter"));

    filesAndDirectoriesBoxLayout->addWidget(deleteAfter, 1, 1);

    QCheckBox *deleteDuring = new QCheckBox(Smb4KSettings::self()->deleteDuringItem()->label(), filesAndDirectoriesBox);
    deleteDuring->setObjectName(QStringLiteral("kcfg_DeleteDuring"));

    filesAndDirectoriesBoxLayout->addWidget(deleteDuring, 2, 0);

    QCheckBox *deleteExcluded = new QCheckBox(Smb4KSettings::self()->deleteExcludedItem()->label(), filesAndDirectoriesBox);
    deleteExcluded->setObjectName(QStringLiteral("kcfg_DeleteExcluded"));

    filesAndDirectoriesBoxLayout->addWidget(deleteExcluded, 2, 1);

    QCheckBox *ignoreIoErrors = new QCheckBox(Smb4KSettings::self()->ignoreErrorsItem()->label(), filesAndDirectoriesBox);
    ignoreIoErrors->setObjectName(QStringLiteral("kcfg_IgnoreErrors"));

    filesAndDirectoriesBoxLayout->addWidget(ignoreIoErrors, 3, 0);

    QCheckBox *forceDirDeletion = new QCheckBox(Smb4KSettings::self()->forceDirectoryDeletionItem()->label(), filesAndDirectoriesBox);
    forceDirDeletion->setObjectName(QStringLiteral("kcfg_ForceDirectoryDeletion"));

    filesAndDirectoriesBoxLayout->addWidget(forceDirDeletion, 3, 1);

    deleteTabLayout->addWidget(filesAndDirectoriesBox);

    // Restrictions
    QGroupBox *restrictionsBox = new QGroupBox(i18n("Restrictions"), deleteTab);
    QGridLayout *restrictionsBoxLayout = new QGridLayout(restrictionsBox);

    QCheckBox *maximumDeleteButton = new QCheckBox(Smb4KSettings::self()->useMaximumDeleteItem()->label(), restrictionsBox);
    maximumDeleteButton->setObjectName(QStringLiteral("kcfg_UseMaximumDelete"));

    restrictionsBoxLayout->addWidget(maximumDeleteButton, 0, 0);

    QSpinBox *maximumDelete = new QSpinBox(restrictionsBox);
    maximumDelete->setObjectName(QStringLiteral("kcfg_MaximumDeleteValue"));

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
    cvsExclude->setObjectName(QStringLiteral("kcfg_UseCVSExclude"));

    generalFilteringBoxLayout->addWidget(cvsExclude, 0, 0, 1, 2);

    QCheckBox *excludePatternButton = new QCheckBox(Smb4KSettings::self()->useExcludePatternItem()->label(), generalFilteringBox);
    excludePatternButton->setObjectName(QStringLiteral("kcfg_UseExcludePattern"));

    generalFilteringBoxLayout->addWidget(excludePatternButton, 1, 0);

    KLineEdit *excludePattern = new KLineEdit(generalFilteringBox);
    excludePattern->setObjectName(QStringLiteral("kcfg_ExcludePattern"));
    excludePattern->setClearButtonEnabled(true);

    generalFilteringBoxLayout->addWidget(excludePattern, 1, 1);

    QCheckBox *excludeFromButton = new QCheckBox(Smb4KSettings::self()->useExcludeFromItem()->label(), generalFilteringBox);
    excludeFromButton->setObjectName(QStringLiteral("kcfg_UseExcludeFrom"));

    generalFilteringBoxLayout->addWidget(excludeFromButton, 2, 0);

    KUrlRequester *excludeFrom = new KUrlRequester(generalFilteringBox);
    excludeFrom->setObjectName(QStringLiteral("kcfg_ExcludeFrom"));
    excludeFrom->setMode(KFile::File | KFile::LocalOnly);

    generalFilteringBoxLayout->addWidget(excludeFrom, 2, 1);

    QCheckBox *includePatternButton = new QCheckBox(Smb4KSettings::self()->useIncludePatternItem()->label(), generalFilteringBox);
    includePatternButton->setObjectName(QStringLiteral("kcfg_UseIncludePattern"));

    generalFilteringBoxLayout->addWidget(includePatternButton, 3, 0);

    KLineEdit *includePattern = new KLineEdit(generalFilteringBox);
    includePattern->setObjectName(QStringLiteral("kcfg_IncludePattern"));
    includePattern->setClearButtonEnabled(true);

    generalFilteringBoxLayout->addWidget(includePattern, 3, 1);

    QCheckBox *includeFromButton = new QCheckBox(Smb4KSettings::self()->useIncludeFromItem()->label(), generalFilteringBox);
    includeFromButton->setObjectName(QStringLiteral("kcfg_UseIncludeFrom"));

    generalFilteringBoxLayout->addWidget(includeFromButton, 4, 0);

    KUrlRequester *includeFrom = new KUrlRequester(generalFilteringBox);
    includeFrom->setObjectName(QStringLiteral("kcfg_IncludeFrom"));
    includeFrom->setMode(KFile::File | KFile::LocalOnly);

    generalFilteringBoxLayout->addWidget(includeFrom, 4, 1);

    filterTabLayout->addWidget(generalFilteringBox);

    // Filter rules
    QGroupBox *filterRulesBox = new QGroupBox(i18n("Filter Rules"), filterTab);
    QGridLayout *filterRulesBoxLayout = new QGridLayout(filterRulesBox);

    QCheckBox *useFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFilterRuleItem()->label(), filterRulesBox);
    useFFilterRule->setObjectName(QStringLiteral("kcfg_UseFFilterRule"));

    filterRulesBoxLayout->addWidget(useFFilterRule, 0, 0, 1, 2);

    QCheckBox *useFFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFFilterRuleItem()->label(), filterRulesBox);
    useFFFilterRule->setObjectName(QStringLiteral("kcfg_UseFFFilterRule"));

    filterRulesBoxLayout->addWidget(useFFFilterRule, 1, 0, 1, 2);

    QLabel *customFilterRulesLabel = new QLabel(Smb4KSettings::self()->customFilteringRulesItem()->label(), filterRulesBox);

    filterRulesBoxLayout->addWidget(customFilterRulesLabel, 2, 0);

    KLineEdit *customFilterRules = new KLineEdit(filterTab);
    customFilterRules->setObjectName(QStringLiteral("kcfg_CustomFilteringRules"));
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
    blockSizeButton->setObjectName(QStringLiteral("kcfg_UseBlockSize"));

    checksumsBoxLayout->addWidget(blockSizeButton, 0, 0);

    QSpinBox *blockSize = new QSpinBox(checksumsBox);
    blockSize->setObjectName(QStringLiteral("kcfg_BlockSize"));

    checksumsBoxLayout->addWidget(blockSize, 0, 1);

    QCheckBox *checksumSeedButton = new QCheckBox(Smb4KSettings::self()->useChecksumSeedItem()->label(), checksumsBox);
    checksumSeedButton->setObjectName(QStringLiteral("kcfg_UseChecksumSeed"));

    checksumsBoxLayout->addWidget(checksumSeedButton, 1, 0);

    QSpinBox *checksumSeed = new QSpinBox(checksumsBox);
    checksumSeed->setObjectName(QStringLiteral("kcfg_ChecksumSeed"));

    checksumsBoxLayout->addWidget(checksumSeed, 1, 1);

    QCheckBox *useChecksum = new QCheckBox(Smb4KSettings::self()->useChecksumItem()->label(), checksumsBox);
    useChecksum->setObjectName(QStringLiteral("kcfg_UseChecksum"));

    checksumsBoxLayout->addWidget(useChecksum, 2, 0, 1, 2);

    miscellaneousTabLayout->addWidget(checksumsBox);

    // Miscellaneous
    QGroupBox *miscellaneousBox2 = new QGroupBox(i18n("Miscellaneous"), miscellaneousTab);
    QGridLayout *miscellaneousBox2Layout = new QGridLayout(miscellaneousBox2);

    QCheckBox *oneFilesystem = new QCheckBox(Smb4KSettings::self()->oneFileSystemItem()->label(), miscellaneousBox2);
    oneFilesystem->setObjectName(QStringLiteral("kcfg_OneFileSystem"));

    miscellaneousBox2Layout->addWidget(oneFilesystem, 0, 0);

    QCheckBox *delayUpdates = new QCheckBox(Smb4KSettings::self()->delayUpdatesItem()->label(), miscellaneousBox2);
    delayUpdates->setObjectName(QStringLiteral("kcfg_DelayUpdates"));

    miscellaneousBox2Layout->addWidget(delayUpdates, 0, 1);

    miscellaneousTabLayout->addWidget(miscellaneousBox2);
    miscellaneousTabLayout->addStretch(100);

    addTab(miscellaneousTab, i18n("Miscellaneous"));

    //
    // Connections
    //
    connect(useFFilterRule, SIGNAL(toggled(bool)), this, SLOT(slotFFilterRuleToggled(bool)));
    connect(useFFFilterRule, SIGNAL(toggled(bool)), this, SLOT(slotFFFilterRuleToggled(bool)));
    connect(makeBackups, SIGNAL(toggled(bool)), this, SLOT(slotBackupToggled(bool)));
    connect(compressData, SIGNAL(toggled(bool)), this, SLOT(slotCompressToggled(bool)));
    connect(keepPartial, SIGNAL(toggled(bool)), this, SLOT(slotKeepPartialToggled(bool)));

    slotBackupToggled(Smb4KSettings::makeBackups());
    slotCompressToggled(Smb4KSettings::compressData());
    slotKeepPartialToggled(Smb4KSettings::keepPartial());
}

Smb4KConfigPageSynchronization::~Smb4KConfigPageSynchronization()
{
}

bool Smb4KConfigPageSynchronization::checkSettings()
{
    for (int i = 0; i < count(); i++) {
        // Synchronization prefix
        KUrlRequester *synchronizationPrefix = widget(i)->findChild<KUrlRequester *>(QStringLiteral("kcfg_RsyncPrefix"));

        if (synchronizationPrefix) {
            if (!synchronizationPrefix->url().isValid()) {
                setCurrentIndex(i);
                synchronizationPrefix->setFocus();
                return false;
            }
        }

        // Backups
        QCheckBox *makeBackups = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_MakeBackups"));
        QCheckBox *useBackupSuffix = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseBackupSuffix"));
        KLineEdit *backupSuffix = widget(i)->findChild<KLineEdit *>(QStringLiteral("kcfg_BackupSuffix"));
        QCheckBox *useBackupDir = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseBackupDirectory"));
        KUrlRequester *backupDir = widget(i)->findChild<KUrlRequester *>(QStringLiteral("kcfg_BackupDirectory"));

        if (makeBackups && makeBackups->isChecked()) {
            if (useBackupSuffix && backupSuffix) {
                if (useBackupSuffix->isChecked() && backupSuffix->text().trimmed().isEmpty()) {
                    setCurrentIndex(i);
                    backupSuffix->setFocus();
                    return false;
                }
            }

            if (useBackupDir && backupDir) {
                if (useBackupDir->isChecked() && !backupDir->url().isValid()) {
                    setCurrentIndex(i);
                    backupDir->setFocus();
                    return false;
                }
            }
        }

        // Minimal transfer size
        QCheckBox *useMinTransferSize = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseMinimalTransferSize"));
        QSpinBox *minTransferSize = widget(i)->findChild<QSpinBox *>(QStringLiteral("kcfg_MinimalTransferSize"));

        if (useMinTransferSize && minTransferSize) {
            if (useMinTransferSize->isChecked() && minTransferSize->value() == 0) {
                setCurrentIndex(i);
                minTransferSize->setFocus();
                return false;
            }
        }

        // Maximal transfer size
        QCheckBox *useMaxTransferSize = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseMaximalTransferSize"));
        QSpinBox *maxTransferSize = widget(i)->findChild<QSpinBox *>(QStringLiteral("kcfg_MaximalTransferSize"));

        if (useMaxTransferSize && maxTransferSize) {
            if (useMaxTransferSize->isChecked() && maxTransferSize->value() == 0) {
                setCurrentIndex(i);
                maxTransferSize->setFocus();
                return false;
            }
        }

        // Partial directory
        QCheckBox *usePartialDirectory = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UsePartialDirectory"));
        KUrlRequester *partialDirectory = widget(i)->findChild<KUrlRequester *>(QStringLiteral("kcfg_PartialDirectory"));

        if (usePartialDirectory && partialDirectory) {
            if (usePartialDirectory->isChecked() && !partialDirectory->url().isValid()) {
                setCurrentIndex(i);
                partialDirectory->setFocus();
                return false;
            }
        }

        // Exclude exclude
        QCheckBox *useExcludePattern = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseExcludePattern"));
        KLineEdit *excludePattern = widget(i)->findChild<KLineEdit *>(QStringLiteral("kcfg_ExcludePattern"));

        if (useExcludePattern && excludePattern) {
            if (useExcludePattern->isChecked() && excludePattern->text().trimmed().isEmpty()) {
                setCurrentIndex(i);
                excludePattern->setFocus();
                return false;
            }
        }

        // Read exclude pattern from file
        QCheckBox *useExcludeFrom = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseExcludeFrom"));
        KUrlRequester *excludeFrom = widget(i)->findChild<KUrlRequester *>(QStringLiteral("kcfg_ExcludeFrom"));

        if (useExcludeFrom && excludeFrom) {
            if (useExcludeFrom->isChecked() && !excludeFrom->url().isValid()) {
                setCurrentIndex(i);
                excludeFrom->setFocus();
                return false;
            }
        }

        // Exclude exclude
        QCheckBox *useIncludePattern = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseIncludePattern"));
        KLineEdit *includePattern = widget(i)->findChild<KLineEdit *>(QStringLiteral("kcfg_IncludePattern"));

        if (useIncludePattern && includePattern) {
            if (useIncludePattern->isChecked() && includePattern->text().trimmed().isEmpty()) {
                setCurrentIndex(i);
                includePattern->setFocus();
                return false;
            }
        }

        // Read exclude pattern from file
        QCheckBox *useIncludeFrom = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseIncludeFrom"));
        KUrlRequester *includeFrom = widget(i)->findChild<KUrlRequester *>(QStringLiteral("kcfg_IncludeFrom"));

        if (useIncludeFrom && includeFrom) {
            if (useIncludeFrom->isChecked() && !includeFrom->url().isValid()) {
                setCurrentIndex(i);
                includeFrom->setFocus();
                return false;
            }
        }

        // Block size
        QCheckBox *useFixedBlocksize = widget(i)->findChild<QCheckBox *>(QStringLiteral("kcfg_UseBlockSize"));
        QSpinBox *fixedBlocksize = widget(i)->findChild<QSpinBox *>(QStringLiteral("kcfg_BlockSize"));

        if (useFixedBlocksize && fixedBlocksize) {
            if (useFixedBlocksize->isChecked() && fixedBlocksize->value() == 0) {
                setCurrentIndex(i);
                fixedBlocksize->setFocus();
                return false;
            }
        }

        // NOTE: There is no need to check the following settings, because they may be empty or 0:
        // - kcfg_UseCompressionLevel & kcfg_CompressionLevel
        // - kcfg_UseSkipCompression & kcfg_SkipCompression
        // - kcfg_UseBandwidthLimit & kcfg_BandwidthLimit
        // - kcfg_UseMaximumDelete & kcfg_MaximumDeleteValue
        // - kcfg_CustomFilteringRules
        // - kcfg_UseChecksumSeed & kcfg_ChecksumSeed
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigPageSynchronization::slotBackupToggled(bool checked)
{
    findChild<QCheckBox *>(QStringLiteral("kcfg_UseBackupDirectory"))->setEnabled(checked);
    findChild<KUrlRequester *>(QStringLiteral("kcfg_BackupDirectory"))->setEnabled(checked);
    findChild<QCheckBox *>(QStringLiteral("kcfg_UseBackupSuffix"))->setEnabled(checked);
    findChild<KLineEdit *>(QStringLiteral("kcfg_BackupSuffix"))->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotCompressToggled(bool checked)
{
    findChild<QCheckBox *>(QStringLiteral("kcfg_UseCompressionLevel"))->setEnabled(checked);
    findChild<QSpinBox *>(QStringLiteral("kcfg_CompressionLevel"))->setEnabled(checked);
    findChild<QCheckBox *>(QStringLiteral("kcfg_UseSkipCompression"))->setEnabled(checked);
    findChild<KLineEdit *>(QStringLiteral("kcfg_SkipCompression"))->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotKeepPartialToggled(bool checked)
{
    findChild<QCheckBox *>(QStringLiteral("kcfg_UsePartialDirectory"))->setEnabled(checked);
    findChild<KUrlRequester *>(QStringLiteral("kcfg_PartialDirectory"))->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotFFilterRuleToggled(bool on)
{
    QCheckBox *ff_filter = findChild<QCheckBox *>(QStringLiteral("kcfg_UseFFFilterRule"));

    if (on && ff_filter->isChecked()) {
        ff_filter->setChecked(false);
    }
}

void Smb4KConfigPageSynchronization::slotFFFilterRuleToggled(bool on)
{
    QCheckBox *f_filter = findChild<QCheckBox *>(QStringLiteral("kcfg_UseFFilterRule"));

    if (on && f_filter->isChecked()) {
        f_filter->setChecked(false);
    }
}
