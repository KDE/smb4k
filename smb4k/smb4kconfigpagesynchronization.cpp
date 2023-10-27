/*
    The configuration page for the synchronization options

    SPDX-FileCopyrightText: 2005-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagesynchronization.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes
#include <KLocalizedString>

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
    QGridLayout *synchronizationDirectoryBoxLayout = new QGridLayout(synchronizationDirectoryBox);

    QLabel *synchronizationPrefixLabel = new QLabel(synchronizationDirectoryBox);
    synchronizationPrefixLabel->setText(Smb4KSettings::self()->rsyncPrefixItem()->label());

    synchronizationDirectoryBoxLayout->addWidget(synchronizationPrefixLabel, 0, 0);

    m_synchronizationPrefix = new KUrlRequester(synchronizationDirectoryBox);
    m_synchronizationPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    m_synchronizationPrefix->setObjectName(QStringLiteral("kcfg_RsyncPrefix"));

    synchronizationDirectoryBoxLayout->addWidget(m_synchronizationPrefix, 0, 1);

    synchronizationPrefixLabel->setBuddy(m_synchronizationPrefix);

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

    m_makeBackups = new QCheckBox(Smb4KSettings::self()->makeBackupsItem()->label(), backupsBox);
    m_makeBackups->setObjectName(QStringLiteral("kcfg_MakeBackups"));

    backupsBoxLayout->addWidget(m_makeBackups);

    m_backupSettingsWidget = new QWidget(backupsBox);
    m_backupSettingsWidget->setEnabled(false);
    QGridLayout *backupSettingsWidgetLayout = new QGridLayout(m_backupSettingsWidget);
    backupSettingsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_useBackupSuffix = new QCheckBox(Smb4KSettings::self()->useBackupSuffixItem()->label(), m_backupSettingsWidget);
    m_useBackupSuffix->setObjectName(QStringLiteral("kcfg_UseBackupSuffix"));
    m_backupSuffix = new KLineEdit(m_backupSettingsWidget);
    m_backupSuffix->setObjectName(QStringLiteral("kcfg_BackupSuffix"));
    m_backupSuffix->setClearButtonEnabled(true);

    backupSettingsWidgetLayout->addWidget(m_useBackupSuffix, 0, 0);
    backupSettingsWidgetLayout->addWidget(m_backupSuffix, 0, 1);

    m_useBackupDirectory = new QCheckBox(Smb4KSettings::self()->useBackupDirectoryItem()->label(), m_backupSettingsWidget);
    m_useBackupDirectory->setObjectName(QStringLiteral("kcfg_UseBackupDirectory"));
    m_backupDirectory = new KUrlRequester(m_backupSettingsWidget);
    m_backupDirectory->setObjectName(QStringLiteral("kcfg_BackupDirectory"));
    m_backupDirectory->setMode(KFile::Directory | KFile::LocalOnly);

    backupSettingsWidgetLayout->addWidget(m_useBackupDirectory, 1, 0);
    backupSettingsWidgetLayout->addWidget(m_backupDirectory, 1, 1);

    backupsBoxLayout->addWidget(m_backupSettingsWidget);

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
    QVBoxLayout *compressionBoxLayout = new QVBoxLayout(compressionBox);

    QCheckBox *compressData = new QCheckBox(Smb4KSettings::self()->compressDataItem()->label(), compressionBox);
    compressData->setObjectName(QStringLiteral("kcfg_CompressData"));

    compressionBoxLayout->addWidget(compressData);

    m_compressionSettingsWidget = new QWidget(compressionBox);
    m_compressionSettingsWidget->setEnabled(false);
    QGridLayout *compressionSettingsWidgetLayout = new QGridLayout(m_compressionSettingsWidget);
    compressionSettingsWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QCheckBox *compressionLevelButton = new QCheckBox(Smb4KSettings::self()->useCompressionLevelItem()->label(), m_compressionSettingsWidget);
    compressionLevelButton->setObjectName(QStringLiteral("kcfg_UseCompressionLevel"));

    compressionSettingsWidgetLayout->addWidget(compressionLevelButton, 0, 0);

    QSpinBox *compressionLevel = new QSpinBox(m_compressionSettingsWidget);
    compressionLevel->setObjectName(QStringLiteral("kcfg_CompressionLevel"));

    compressionSettingsWidgetLayout->addWidget(compressionLevel, 0, 1);

    QCheckBox *skipCompressionButton = new QCheckBox(Smb4KSettings::self()->useSkipCompressionItem()->label(), m_compressionSettingsWidget);
    skipCompressionButton->setObjectName(QStringLiteral("kcfg_UseSkipCompression"));

    compressionSettingsWidgetLayout->addWidget(skipCompressionButton, 1, 0);

    KLineEdit *skipCompression = new KLineEdit(m_compressionSettingsWidget);
    skipCompression->setObjectName(QStringLiteral("kcfg_SkipCompression"));
    skipCompression->setClearButtonEnabled(true);

    compressionSettingsWidgetLayout->addWidget(skipCompression, 1, 1);

    compressionBoxLayout->addWidget(m_compressionSettingsWidget);

    transferTabLayout->addWidget(compressionBox);

    // Files
    QGroupBox *filesBox = new QGroupBox(i18n("Files"), transferTab);
    QGridLayout *filesBoxLayout = new QGridLayout(filesBox);

    m_useMinimalTransferSize = new QCheckBox(Smb4KSettings::self()->useMinimalTransferSizeItem()->label(), filesBox);
    m_useMinimalTransferSize->setObjectName(QStringLiteral("kcfg_UseMinimalTransferSize"));

    filesBoxLayout->addWidget(m_useMinimalTransferSize, 0, 0);

    m_minimalTransferSize = new QSpinBox(filesBox);
    m_minimalTransferSize->setObjectName(QStringLiteral("kcfg_MinimalTransferSize"));
    m_minimalTransferSize->setSuffix(i18n(" kB"));

    filesBoxLayout->addWidget(m_minimalTransferSize, 0, 1);

    m_useMaximalTransferSize = new QCheckBox(Smb4KSettings::self()->useMaximalTransferSizeItem()->label(), filesBox);
    m_useMaximalTransferSize->setObjectName(QStringLiteral("kcfg_UseMaximalTransferSize"));

    filesBoxLayout->addWidget(m_useMaximalTransferSize, 1, 0);

    m_maximalTransferSize = new QSpinBox(filesBox);
    m_maximalTransferSize->setObjectName(QStringLiteral("kcfg_MaximalTransferSize"));
    m_maximalTransferSize->setSuffix(i18n(" kB"));

    filesBoxLayout->addWidget(m_maximalTransferSize, 1, 1);

    QCheckBox *keepPartial = new QCheckBox(Smb4KSettings::self()->keepPartialItem()->label(), filesBox);
    keepPartial->setObjectName(QStringLiteral("kcfg_KeepPartial"));

    filesBoxLayout->addWidget(keepPartial, 2, 0, 1, 2);

    m_usePartialDirectory = new QCheckBox(Smb4KSettings::self()->usePartialDirectoryItem()->label(), filesBox);
    m_usePartialDirectory->setObjectName(QStringLiteral("kcfg_UsePartialDirectory"));
    m_usePartialDirectory->setEnabled(false);

    filesBoxLayout->addWidget(m_usePartialDirectory, 3, 0);

    m_partialDirectory = new KUrlRequester(filesBox);
    m_partialDirectory->setObjectName(QStringLiteral("kcfg_PartialDirectory"));
    m_partialDirectory->setMode(KFile::Directory | KFile::LocalOnly);
    m_partialDirectory->setEnabled(false);

    filesBoxLayout->addWidget(m_partialDirectory, 3, 1);

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

    m_useExcludePattern = new QCheckBox(Smb4KSettings::self()->useExcludePatternItem()->label(), generalFilteringBox);
    m_useExcludePattern->setObjectName(QStringLiteral("kcfg_UseExcludePattern"));

    generalFilteringBoxLayout->addWidget(m_useExcludePattern, 1, 0);

    m_excludePattern = new KLineEdit(generalFilteringBox);
    m_excludePattern->setObjectName(QStringLiteral("kcfg_ExcludePattern"));
    m_excludePattern->setClearButtonEnabled(true);

    generalFilteringBoxLayout->addWidget(m_excludePattern, 1, 1);

    m_useExcludeFrom = new QCheckBox(Smb4KSettings::self()->useExcludeFromItem()->label(), generalFilteringBox);
    m_useExcludeFrom->setObjectName(QStringLiteral("kcfg_UseExcludeFrom"));

    generalFilteringBoxLayout->addWidget(m_useExcludeFrom, 2, 0);

    m_excludeFrom = new KUrlRequester(generalFilteringBox);
    m_excludeFrom->setObjectName(QStringLiteral("kcfg_ExcludeFrom"));
    m_excludeFrom->setMode(KFile::File | KFile::LocalOnly);

    generalFilteringBoxLayout->addWidget(m_excludeFrom, 2, 1);

    m_useIncludePattern = new QCheckBox(Smb4KSettings::self()->useIncludePatternItem()->label(), generalFilteringBox);
    m_useIncludePattern->setObjectName(QStringLiteral("kcfg_UseIncludePattern"));

    generalFilteringBoxLayout->addWidget(m_useIncludePattern, 3, 0);

    m_includePattern = new KLineEdit(generalFilteringBox);
    m_includePattern->setObjectName(QStringLiteral("kcfg_IncludePattern"));
    m_includePattern->setClearButtonEnabled(true);

    generalFilteringBoxLayout->addWidget(m_includePattern, 3, 1);

    m_useIncludeFrom = new QCheckBox(Smb4KSettings::self()->useIncludeFromItem()->label(), generalFilteringBox);
    m_useIncludeFrom->setObjectName(QStringLiteral("kcfg_UseIncludeFrom"));

    generalFilteringBoxLayout->addWidget(m_useIncludeFrom, 4, 0);

    m_includeFrom = new KUrlRequester(generalFilteringBox);
    m_includeFrom->setObjectName(QStringLiteral("kcfg_IncludeFrom"));
    m_includeFrom->setMode(KFile::File | KFile::LocalOnly);

    generalFilteringBoxLayout->addWidget(m_includeFrom, 4, 1);

    filterTabLayout->addWidget(generalFilteringBox);

    // Filter rules
    QGroupBox *filterRulesBox = new QGroupBox(i18n("Filter Rules"), filterTab);
    QGridLayout *filterRulesBoxLayout = new QGridLayout(filterRulesBox);

    m_useFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFilterRuleItem()->label(), filterRulesBox);
    m_useFFilterRule->setObjectName(QStringLiteral("kcfg_UseFFilterRule"));

    filterRulesBoxLayout->addWidget(m_useFFilterRule, 0, 0, 1, 2);

    m_useFFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFFilterRuleItem()->label(), filterRulesBox);
    m_useFFFilterRule->setObjectName(QStringLiteral("kcfg_UseFFFilterRule"));

    filterRulesBoxLayout->addWidget(m_useFFFilterRule, 1, 0, 1, 2);

    QCheckBox *useCustomFilterRules = new QCheckBox(Smb4KSettings::self()->useCustomFilteringRulesItem()->label(), filterRulesBox);
    useCustomFilterRules->setObjectName(QStringLiteral("kcfg_UseCustomFilteringRules"));

    filterRulesBoxLayout->addWidget(useCustomFilterRules, 2, 0);

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

    m_useBlockSize = new QCheckBox(Smb4KSettings::self()->useBlockSizeItem()->label(), checksumsBox);
    m_useBlockSize->setObjectName(QStringLiteral("kcfg_UseBlockSize"));

    checksumsBoxLayout->addWidget(m_useBlockSize, 0, 0);

    m_blockSize = new QSpinBox(checksumsBox);
    m_blockSize->setObjectName(QStringLiteral("kcfg_BlockSize"));

    checksumsBoxLayout->addWidget(m_blockSize, 0, 1);

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
    connect(m_useFFilterRule, &QCheckBox::toggled, this, &Smb4KConfigPageSynchronization::slotFFilterRuleToggled);
    connect(m_useFFFilterRule, &QCheckBox::toggled, this, &Smb4KConfigPageSynchronization::slotFFFilterRuleToggled);
    connect(m_makeBackups, &QCheckBox::toggled, this, &Smb4KConfigPageSynchronization::slotBackupToggled);
    connect(compressData, &QCheckBox::toggled, this, &Smb4KConfigPageSynchronization::slotCompressToggled);
    connect(keepPartial, &QCheckBox::toggled, this, &Smb4KConfigPageSynchronization::slotKeepPartialToggled);
}

Smb4KConfigPageSynchronization::~Smb4KConfigPageSynchronization()
{
}

bool Smb4KConfigPageSynchronization::checkSettings()
{
    for (int i = 0; i < count(); i++) {
        // Synchronization prefix
        if (widget(i)->isAncestorOf(m_synchronizationPrefix) && !m_synchronizationPrefix->url().isValid()) {
            setCurrentIndex(i);
            m_synchronizationPrefix->setFocus();
            return false;
        }

        // Backups
        if (widget(i)->isAncestorOf(m_makeBackups) && m_makeBackups->isChecked()) {
            if (m_useBackupSuffix->isChecked() && m_backupSuffix->text().trimmed().isEmpty()) {
                setCurrentIndex(i);
                m_backupSuffix->setFocus();
                return false;
            }

            if (m_useBackupDirectory->isChecked() && !m_backupDirectory->url().isValid()) {
                setCurrentIndex(i);
                m_backupDirectory->setFocus();
                return false;
            }
        }

        // Minimal transfer size
        if (widget(i)->isAncestorOf(m_useMinimalTransferSize) && widget(i)->isAncestorOf(m_minimalTransferSize) && m_useMinimalTransferSize->isChecked()
            && m_minimalTransferSize->value() == 0) {
            setCurrentIndex(i);
            m_minimalTransferSize->setFocus();
            return false;
        }

        // Maximal transfer size
        if (widget(i)->isAncestorOf(m_useMaximalTransferSize) && widget(i)->isAncestorOf(m_maximalTransferSize) && m_useMaximalTransferSize->isChecked()
            && m_maximalTransferSize->value() == 0) {
            setCurrentIndex(i);
            m_maximalTransferSize->setFocus();
            return false;
        }

        // Partial directory
        if (widget(i)->isAncestorOf(m_usePartialDirectory) && widget(i)->isAncestorOf(m_partialDirectory) && m_usePartialDirectory->isChecked()
            && !m_partialDirectory->url().isValid()) {
            setCurrentIndex(i);
            m_partialDirectory->setFocus();
            return false;
        }

        // Exclude exclude
        if (widget(i)->isAncestorOf(m_useExcludePattern) && widget(i)->isAncestorOf(m_excludePattern) && m_useExcludePattern->isChecked()
            && m_excludePattern->text().trimmed().isEmpty()) {
            setCurrentIndex(i);
            m_excludePattern->setFocus();
            return false;
        }

        // Read exclude pattern from file
        if (widget(i)->isAncestorOf(m_useExcludeFrom) && widget(i)->isAncestorOf(m_excludeFrom) && m_useExcludeFrom->isChecked()
            && !m_excludeFrom->url().isValid()) {
            setCurrentIndex(i);
            m_excludeFrom->setFocus();
            return false;
        }

        // Exclude exclude
        if (widget(i)->isAncestorOf(m_useIncludePattern) && widget(i)->isAncestorOf(m_includePattern) && m_useIncludePattern->isChecked()
            && m_includePattern->text().trimmed().isEmpty()) {
            setCurrentIndex(i);
            m_includePattern->setFocus();
            return false;
        }

        // Read exclude pattern from file
        if (widget(i)->isAncestorOf(m_useIncludeFrom) && widget(i)->isAncestorOf(m_includeFrom) && m_useIncludeFrom->isChecked()
            && !m_includeFrom->url().isValid()) {
            setCurrentIndex(i);
            m_includeFrom->setFocus();
            return false;
        }

        // Block size
        if (widget(i)->isAncestorOf(m_useBlockSize) && widget(i)->isAncestorOf(m_blockSize) && m_useBlockSize->isChecked() && m_blockSize->value() == 0) {
            setCurrentIndex(i);
            m_blockSize->setFocus();
            return false;
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
    m_backupSettingsWidget->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotCompressToggled(bool checked)
{
    m_compressionSettingsWidget->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotKeepPartialToggled(bool checked)
{
    m_usePartialDirectory->setEnabled(checked);
    m_partialDirectory->setEnabled(checked);
}

void Smb4KConfigPageSynchronization::slotFFilterRuleToggled(bool on)
{
    if (on && m_useFFFilterRule->isChecked()) {
        m_useFFFilterRule->setChecked(false);
    }
}

void Smb4KConfigPageSynchronization::slotFFFilterRuleToggled(bool on)
{
    if (on && m_useFFilterRule->isChecked()) {
        m_useFFilterRule->setChecked(false);
    }
}
