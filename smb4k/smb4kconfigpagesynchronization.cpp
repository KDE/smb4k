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
#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QFormLayout>

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
    QFormLayout *basicTabLayout = new QFormLayout(basicTab);

    // Default destination (rsync)
    KUrlRequester *rsyncPrefix = new KUrlRequester(basicTab);
    rsyncPrefix->setMode(KFile::Directory | KFile::LocalOnly);
    rsyncPrefix->setObjectName("kcfg_RsyncPrefix");
    
    basicTabLayout->addRow(Smb4KSettings::self()->rsyncPrefixItem()->label(), rsyncPrefix);

    // Behavior
    QCheckBox *archiveMode = new QCheckBox(Smb4KSettings::self()->archiveModeItem()->label(), basicTab);
    archiveMode->setObjectName("kcfg_ArchiveMode");
    
    basicTabLayout->addRow(i18n("Behavior:"), archiveMode);
    
    QCheckBox *recurseDirs = new QCheckBox(Smb4KSettings::self()->recurseIntoDirectoriesItem()->label(), basicTab);
    recurseDirs->setObjectName("kcfg_RecurseIntoDirectories");
    
    basicTabLayout->addRow(QString(), recurseDirs);
    
    QCheckBox *relativePaths = new QCheckBox(Smb4KSettings::self()->relativePathNamesItem()->label(), basicTab);
    relativePaths->setObjectName("kcfg_RelativePathNames");
    
    basicTabLayout->addRow(QString(), relativePaths);
    
    QCheckBox *noImpliedDirs = new QCheckBox(Smb4KSettings::self()->noImpliedDirectoriesItem()->label(), basicTab);
    noImpliedDirs->setObjectName("kcfg_NoImpliedDirectories");
    
    basicTabLayout->addRow(QString(), noImpliedDirs);
    
    QCheckBox *transferDirs = new QCheckBox(Smb4KSettings::self()->transferDirectoriesItem()->label(), basicTab);
    transferDirs->setObjectName("kcfg_TransferDirectories");
    
    basicTabLayout->addRow(QString(), transferDirs);
    
    // Backup
    QCheckBox *makeBackups = new QCheckBox(Smb4KSettings::self()->makeBackupsItem()->label(), basicTab);
    makeBackups->setObjectName("kcfg_MakeBackups");
    
    basicTabLayout->addRow(i18n("Backups:"), makeBackups);
    
    QWidget *backupSuffixWidget = new QWidget(basicTab);
    QHBoxLayout *backupSuffixLayout = new QHBoxLayout(backupSuffixWidget);
    backupSuffixLayout->setMargin(0);
    
    QCheckBox *backupSuffixButton = new QCheckBox(Smb4KSettings::self()->useBackupSuffixItem()->label(), backupSuffixWidget);
    backupSuffixButton->setObjectName("kcfg_UseBackupSuffix");
    KLineEdit *backupSuffix = new KLineEdit(backupSuffixWidget);
    backupSuffix->setObjectName("kcfg_BackupSuffix");
    backupSuffix->setClearButtonEnabled(true);
    
    backupSuffixLayout->addWidget(backupSuffixButton);
    backupSuffixLayout->addWidget(backupSuffix);
    
    basicTabLayout->addRow(QString(), backupSuffixWidget);
    
    QWidget *backupDirWidget = new QWidget(basicTab);
    QHBoxLayout *backupDirLayout = new QHBoxLayout(backupDirWidget);
    backupDirLayout->setMargin(0);
    
    QCheckBox *backupDirButton = new QCheckBox(Smb4KSettings::self()->useBackupDirectoryItem()->label(), backupDirWidget);
    backupDirButton->setObjectName("kcfg_UseBackupDirectory");
    KUrlRequester *backupDir = new KUrlRequester(backupDirWidget);
    backupDir->setObjectName("kcfg_BackupDirectory");
    backupDir->setMode(KFile::Directory | KFile::LocalOnly);
    
    backupDirLayout->addWidget(backupDirButton);
    backupDirLayout->addWidget(backupDir);
    
    basicTabLayout->addRow(QString(), backupDirWidget);

    addTab(basicTab, i18n("Basic Settings"));

    //
    // 'File Handling' tab
    //
    QWidget *fileHandlingTab = new QWidget(this);
    QFormLayout *fileHandlingTabLayout = new QFormLayout(fileHandlingTab);

    // General
    QCheckBox *updateTarget = new QCheckBox(Smb4KSettings::self()->updateTargetItem()->label(), fileHandlingTab);
    updateTarget->setObjectName("kcfg_UpdateTarget");
    
    fileHandlingTabLayout->addRow(i18n("General:"), updateTarget);
    
    QCheckBox *updateInPlace = new QCheckBox(Smb4KSettings::self()->updateInPlaceItem()->label(), fileHandlingTab);
    updateInPlace->setObjectName("kcfg_UpdateInPlace");
    
    fileHandlingTabLayout->addRow(QString(), updateInPlace);
    
    QCheckBox *sparseFiles = new QCheckBox(Smb4KSettings::self()->efficientSparseFileHandlingItem()->label(), fileHandlingTab);
    sparseFiles->setObjectName("kcfg_EfficientSparseFileHandling");
    
    fileHandlingTabLayout->addRow(QString(), sparseFiles);
    
    QCheckBox *copyFilesWhole = new QCheckBox(Smb4KSettings::self()->copyFilesWholeItem()->label(), fileHandlingTab);
    copyFilesWhole->setObjectName("kcfg_CopyFilesWhole");
    
    fileHandlingTabLayout->addRow(QString(), copyFilesWhole);
    
    QCheckBox *updateExisting = new QCheckBox(Smb4KSettings::self()->updateExistingItem()->label(), fileHandlingTab);
    updateExisting->setObjectName("kcfg_UpdateExisting");
    
    fileHandlingTabLayout->addRow(QString(), updateExisting);
    
    QCheckBox *ignoreExisting = new QCheckBox(Smb4KSettings::self()->ignoreExistingItem()->label(), fileHandlingTab);
    ignoreExisting->setObjectName("kcfg_IgnoreExisting");
    
    fileHandlingTabLayout->addRow(QString(), ignoreExisting);

    // Links
    QCheckBox *preserveLinks = new QCheckBox(Smb4KSettings::self()->preserveSymlinksItem()->label(), fileHandlingTab);
    preserveLinks->setObjectName("kcfg_PreserveSymlinks");
    
    fileHandlingTabLayout->addRow(i18n("Links:"), preserveLinks);
    
    QCheckBox *transformLinks = new QCheckBox(Smb4KSettings::self()->transformSymlinksItem()->label(), fileHandlingTab);
    transformLinks->setObjectName("kcfg_TransformSymlinks");
    
    fileHandlingTabLayout->addRow(QString(), transformLinks);
    
    QCheckBox *transformUnsafe = new QCheckBox(Smb4KSettings::self()->transformUnsafeSymlinksItem()->label(), fileHandlingTab);
    transformUnsafe->setObjectName("kcfg_TransformUnsafeSymlinks");
    
    fileHandlingTabLayout->addRow(QString(), transformUnsafe);
    
    QCheckBox *ignoreUnsafe = new QCheckBox(Smb4KSettings::self()->ignoreUnsafeSymlinksItem()->label(), fileHandlingTab);
    ignoreUnsafe->setObjectName("kcfg_IgnoreUnsafeSymlinks");
    
    fileHandlingTabLayout->addRow(QString(), ignoreUnsafe);
    
    QCheckBox *mungeLinks = new QCheckBox(Smb4KSettings::self()->mungeSymlinksItem()->label(), fileHandlingTab);
    mungeLinks->setObjectName("kcfg_MungeSymlinks");
    
    fileHandlingTabLayout->addRow(QString(), mungeLinks);
    
    QCheckBox *preserveHlinks = new QCheckBox(Smb4KSettings::self()->preserveHardLinksItem()->label(), fileHandlingTab);
    preserveHlinks->setObjectName("kcfg_PreserveHardLinks");
    
    fileHandlingTabLayout->addRow(QString(), preserveHlinks);
    
    QCheckBox *copyDirLinks = new QCheckBox(Smb4KSettings::self()->copyDirectorySymlinksItem()->label(), fileHandlingTab);
    copyDirLinks->setObjectName("kcfg_CopyDirectorySymlinks");
    
    fileHandlingTabLayout->addRow(QString(), copyDirLinks);
    
    QCheckBox *keepDirLinks = new QCheckBox(Smb4KSettings::self()->keepDirectorySymlinksItem()->label(), fileHandlingTab);
    keepDirLinks->setObjectName("kcfg_KeepDirectorySymlinks");
    
    fileHandlingTabLayout->addRow(QString(), keepDirLinks);
    
    addTab(fileHandlingTab, i18n("File Handling"));
    
    //
    // 'File Attributes and Ownership' tab
    // 
    QWidget *fileAttributesTab = new QWidget(this);
    QFormLayout *fileAttributesTabLayout = new QFormLayout(fileAttributesTab);

    // Permissions & Attributes
    QCheckBox *preservePerms = new QCheckBox(Smb4KSettings::self()->preservePermissionsItem()->label(), fileAttributesTab);
    preservePerms->setObjectName("kcfg_PreservePermissions");
    
    fileAttributesTabLayout->addRow(i18n("File Attributes:"), preservePerms);
    
    QCheckBox *preserveDevices = new QCheckBox(Smb4KSettings::self()->preserveDevicesAndSpecialsItem()->label(), fileAttributesTab);
    preserveDevices->setObjectName("kcfg_PreserveDevicesAndSpecials");
    
    fileAttributesTabLayout->addRow(QString(), preserveDevices);
    
    QCheckBox *preserveTimes = new QCheckBox(Smb4KSettings::self()->preserveTimesItem()->label(), fileAttributesTab);
    preserveTimes->setObjectName("kcfg_PreserveTimes");
    
    fileAttributesTabLayout->addRow(QString(), preserveTimes);
    
    QCheckBox *omitDirTimes = new QCheckBox(Smb4KSettings::self()->omitDirectoryTimesItem()->label(), fileAttributesTab);
    omitDirTimes->setObjectName("kcfg_OmitDirectoryTimes");
    
    fileAttributesTabLayout->addRow(QString(), omitDirTimes);
    
    QCheckBox *preserveOwner = new QCheckBox(Smb4KSettings::self()->preserveOwnerItem()->label(), fileAttributesTab);
    preserveOwner->setObjectName("kcfg_PreserveOwner");
    
    fileAttributesTabLayout->addRow(i18n("Ownership:"), preserveOwner);
    
    QCheckBox *preserveGroup = new QCheckBox(Smb4KSettings::self()->preserveGroupItem()->label(), fileAttributesTab);
    preserveGroup->setObjectName("kcfg_PreserveGroup");
    
    fileAttributesTabLayout->addRow(QString(), preserveGroup);
    
    addTab(fileAttributesTab, i18n("File Attributes and Ownership"));

    //
    // 'File Transfer' tab
    //
    QWidget *transferTab = new QWidget(this);
    QFormLayout *transferTabLayout = new QFormLayout(transferTab);

    // Compression
    QCheckBox *compressData = new QCheckBox(Smb4KSettings::self()->compressDataItem()->label(), transferTab);
    compressData->setObjectName("kcfg_CompressData");
    
    transferTabLayout->addRow(i18n("Compression:"), compressData);
    
    QWidget *compressionLevelWidget = new QWidget(transferTab);
    QHBoxLayout *compressionLevelWidgetLayout = new QHBoxLayout(compressionLevelWidget);
    compressionLevelWidgetLayout->setMargin(0);
    
    QCheckBox *compressionLevelButton = new QCheckBox(Smb4KSettings::self()->useCompressionLevelItem()->label(), compressionLevelWidget);
    compressionLevelButton->setObjectName("kcfg_UseCompressionLevel");
    QSpinBox *compressionLevel = new QSpinBox(compressionLevelWidget);
    compressionLevel->setObjectName("kcfg_CompressionLevel");

    compressionLevelWidgetLayout->addWidget(compressionLevelButton);
    compressionLevelWidgetLayout->addWidget(compressionLevel);
    
    transferTabLayout->addRow(QString(), compressionLevelWidget);

    QWidget *skipCompressionWidget = new QWidget(transferTab);
    QBoxLayout *skipCompressionWidgetLayout = new QHBoxLayout(skipCompressionWidget);
    skipCompressionWidgetLayout->setMargin(0);
    
    QCheckBox *skipCompressionButton = new QCheckBox(Smb4KSettings::self()->useSkipCompressionItem()->label(), skipCompressionWidget);
    skipCompressionButton->setObjectName("kcfg_UseSkipCompression");
    KLineEdit *skipCompression = new KLineEdit(skipCompressionWidget);
    skipCompression->setObjectName("kcfg_SkipCompression");
    skipCompression->setClearButtonEnabled(true);
    
    skipCompressionWidgetLayout->addWidget(skipCompressionButton);
    skipCompressionWidgetLayout->addWidget(skipCompression);
    
    transferTabLayout->addRow(QString(), skipCompressionWidget);

    // Files
    QWidget *minTransferSizeWidget = new QWidget(transferTab);
    QHBoxLayout *minTransferSizeWidgetLayout = new QHBoxLayout(minTransferSizeWidget);
    minTransferSizeWidgetLayout->setMargin(0);
    
    QCheckBox *minTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMinimalTransferSizeItem()->label(), minTransferSizeWidget);
    minTransferSizeButton->setObjectName("kcfg_UseMinimalTransferSize");
    QSpinBox *minTransferSize = new QSpinBox(minTransferSizeWidget);
    minTransferSize->setObjectName("kcfg_MinimalTransferSize");
    minTransferSize->setSuffix(i18n(" KiB"));
    
    minTransferSizeWidgetLayout->addWidget(minTransferSizeButton);
    minTransferSizeWidgetLayout->addWidget(minTransferSize);
    
    transferTabLayout->addRow(i18n("Files:"), minTransferSizeWidget);
    
    QWidget *maxTransferSizeWidget = new QWidget(transferTab);
    QHBoxLayout *maxTransferSizeWidgetLayout = new QHBoxLayout(maxTransferSizeWidget);
    maxTransferSizeWidgetLayout->setMargin(0);
    
    QCheckBox *maxTransferSizeButton = new QCheckBox(Smb4KSettings::self()->useMaximalTransferSizeItem()->label(), maxTransferSizeWidget);
    maxTransferSizeButton->setObjectName("kcfg_UseMaximalTransferSize");
    QSpinBox *maxTransferSize = new QSpinBox(maxTransferSizeWidget);
    maxTransferSize->setObjectName("kcfg_MaximalTransferSize");
    maxTransferSize->setSuffix(i18n(" KiB"));
    
    maxTransferSizeWidgetLayout->addWidget(maxTransferSizeButton);
    maxTransferSizeWidgetLayout->addWidget(maxTransferSize);
    
    transferTabLayout->addRow(QString(), maxTransferSizeWidget);
    
    QCheckBox *keepPartial = new QCheckBox(Smb4KSettings::self()->keepPartialItem()->label(), transferTab);
    keepPartial->setObjectName("kcfg_KeepPartial");
    
    transferTabLayout->addRow(QString(), keepPartial);
    
    QWidget *partialDirWidget = new QWidget(transferTab);
    QHBoxLayout *partialDirWidgetLayout = new QHBoxLayout(partialDirWidget);
    partialDirWidgetLayout->setMargin(0);
    
    QCheckBox *partialDirButton = new QCheckBox(Smb4KSettings::self()->usePartialDirectoryItem()->label(), partialDirWidget);
    partialDirButton->setObjectName("kcfg_UsePartialDirectory");
    KUrlRequester *partialDir = new KUrlRequester(partialDirWidget);
    partialDir->setObjectName("kcfg_PartialDirectory");
    partialDir->setMode(KFile::Directory | KFile::LocalOnly);
    
    partialDirWidgetLayout->addWidget(partialDirButton);
    partialDirWidgetLayout->addWidget(partialDir);
    
    transferTabLayout->addRow(QString(), partialDirWidget);

    // Miscellaneous
    QWidget *bandwidthLimitWidget = new QWidget(transferTab);
    QHBoxLayout*bandwidthLimitWidgetLayout =new QHBoxLayout(bandwidthLimitWidget);
    bandwidthLimitWidgetLayout->setMargin(0);

    QCheckBox *bandwidthLimitButton = new QCheckBox(Smb4KSettings::self()->useBandwidthLimitItem()->label(), bandwidthLimitWidget);
    bandwidthLimitButton->setObjectName("kcfg_UseBandwidthLimit");
    QSpinBox *bandwidthLimit = new QSpinBox(transferTab);
    bandwidthLimit->setObjectName("kcfg_BandwidthLimit");
    bandwidthLimit->setSuffix(i18n(" KiB/s"));

    bandwidthLimitWidgetLayout->addWidget(bandwidthLimitButton);
    bandwidthLimitWidgetLayout->addWidget(bandwidthLimit);
    
    transferTabLayout->addRow(i18n("Miscellaneous:"), bandwidthLimitWidget);

    addTab(transferTab, i18n("File Transfer"));

    //
    // 'File Deletion' tab
    //
    QWidget *deleteTab = new QWidget(this);
    QFormLayout *deleteTabLayout = new QFormLayout(deleteTab);

    // Files and Directories
    QCheckBox *removeSource = new QCheckBox(Smb4KSettings::self()->removeSourceFilesItem()->label(), deleteTab);
    removeSource->setObjectName("kcfg_RemoveSourceFiles");
    
    deleteTabLayout->addRow(i18n("Files && Directories:"), removeSource);
    
    QCheckBox *deleteExtraneous = new QCheckBox(Smb4KSettings::self()->deleteExtraneousItem()->label(), deleteTab);
    deleteExtraneous->setObjectName("kcfg_DeleteExtraneous");
    
    deleteTabLayout->addRow(QString(), deleteExtraneous);
    
    QCheckBox *deleteBefore = new QCheckBox(Smb4KSettings::self()->deleteBeforeItem()->label(), deleteTab);
    deleteBefore->setObjectName("kcfg_DeleteBefore");
    
    deleteTabLayout->addRow(QString(), deleteBefore);
    
    QCheckBox *deleteAfter = new QCheckBox(Smb4KSettings::self()->deleteAfterItem()->label(), deleteTab);
    deleteAfter->setObjectName("kcfg_DeleteAfter");
    
    deleteTabLayout->addRow(QString(), deleteAfter);
    
    QCheckBox *deleteDuring = new QCheckBox(Smb4KSettings::self()->deleteDuringItem()->label(), deleteTab);
    deleteDuring->setObjectName("kcfg_DeleteDuring");
    
    deleteTabLayout->addRow(QString(), deleteDuring);
    
    QCheckBox *deleteExcluded = new QCheckBox(Smb4KSettings::self()->deleteExcludedItem()->label(), deleteTab);
    deleteExcluded->setObjectName("kcfg_DeleteExcluded");
    
    deleteTabLayout->addRow(QString(), deleteExcluded);
    
    QCheckBox *ignoreIOErrors = new QCheckBox(Smb4KSettings::self()->ignoreErrorsItem()->label(), deleteTab);
    ignoreIOErrors->setObjectName("kcfg_IgnoreErrors");
    
    deleteTabLayout->addRow(QString(), ignoreIOErrors);
    
    QCheckBox *forceDirDeletion = new QCheckBox(Smb4KSettings::self()->forceDirectoryDeletionItem()->label(), deleteTab);
    forceDirDeletion->setObjectName("kcfg_ForceDirectoryDeletion");
    
    deleteTabLayout->addRow(QString(), forceDirDeletion);

    // Restrictions
    QWidget *maximumDeleteWidget = new QWidget(deleteTab);
    QHBoxLayout *maximumDeleteWidgetLayout = new QHBoxLayout(maximumDeleteWidget);
    maximumDeleteWidgetLayout->setMargin(0);

    QCheckBox *maximumDeleteButton = new QCheckBox(Smb4KSettings::self()->useMaximumDeleteItem()->label(), maximumDeleteWidget);
    maximumDeleteButton->setObjectName("kcfg_UseMaximumDelete");
    QSpinBox *maximumDelete = new QSpinBox(maximumDeleteWidget);
    maximumDelete->setObjectName("kcfg_MaximumDeleteValue");

    maximumDeleteWidgetLayout->addWidget(maximumDeleteButton);
    maximumDeleteWidgetLayout->addWidget(maximumDelete);
    
    deleteTabLayout->addRow(i18n("Restrictions:"), maximumDeleteWidget);

    addTab(deleteTab, i18n("File Deletion"));

    //
    // 'Filter' tab
    //
    QWidget *filterTab = new QWidget(this);
    QFormLayout *filterTabLayout = new QFormLayout(filterTab);

    // General
    QCheckBox *cvsExclude = new QCheckBox(Smb4KSettings::self()->useCVSExcludeItem()->label(), filterTab);
    cvsExclude->setObjectName("kcfg_UseCVSExclude");

    filterTabLayout->addRow(i18n("General:"), cvsExclude);

    QWidget *excludePatternWidget = new QWidget(filterTab);
    QHBoxLayout *excludePatternWidgetLayout = new QHBoxLayout(excludePatternWidget);
    excludePatternWidgetLayout->setMargin(0);

    QCheckBox *excludePatternButton = new QCheckBox(Smb4KSettings::self()->useExcludePatternItem()->label(), excludePatternWidget);
    excludePatternButton->setObjectName("kcfg_UseExcludePattern");
    KLineEdit *excludePattern = new KLineEdit(excludePatternWidget);
    excludePattern->setObjectName("kcfg_ExcludePattern");
    excludePattern->setClearButtonEnabled(true);

    excludePatternWidgetLayout->addWidget(excludePatternButton);
    excludePatternWidgetLayout->addWidget(excludePattern);

    filterTabLayout->addRow(QString(), excludePatternWidget);

    QWidget *excludeFromWidget = new QWidget(filterTab);
    QHBoxLayout *excludeFromWidgetLayout = new QHBoxLayout(excludeFromWidget);
    excludeFromWidgetLayout->setMargin(0);

    QCheckBox *excludeFromButton = new QCheckBox(Smb4KSettings::self()->useExcludeFromItem()->label(), excludeFromWidget);
    excludeFromButton->setObjectName("kcfg_UseExcludeFrom");
    KUrlRequester *excludeFrom = new KUrlRequester(excludeFromWidget);
    excludeFrom->setObjectName("kcfg_ExcludeFrom");
    excludeFrom->setMode(KFile::File | KFile::LocalOnly);

    excludeFromWidgetLayout->addWidget(excludeFromButton);
    excludeFromWidgetLayout->addWidget(excludeFrom);

    filterTabLayout->addRow(QString(), excludeFromWidget);

    QWidget *includePatternWidget = new QWidget(filterTab);
    QHBoxLayout *includePatternWidgetLayout = new QHBoxLayout(includePatternWidget);
    includePatternWidgetLayout->setMargin(0);

    QCheckBox *includePatternButton = new QCheckBox(Smb4KSettings::self()->useIncludePatternItem()->label(), includePatternWidget);
    includePatternButton->setObjectName("kcfg_UseIncludePattern");
    KLineEdit *includePattern = new KLineEdit(includePatternWidget);
    includePattern->setObjectName("kcfg_IncludePattern");
    includePattern->setClearButtonEnabled(true);

    includePatternWidgetLayout->addWidget(includePatternButton);
    includePatternWidgetLayout->addWidget(includePattern);

    filterTabLayout->addRow(QString(), includePatternWidget);

    QWidget *includeFromWidget = new QWidget(filterTab);
    QHBoxLayout *includeFromWidgetLayout = new QHBoxLayout(includeFromWidget);
    includeFromWidgetLayout->setMargin(0);

    QCheckBox *includeFromButton = new QCheckBox(Smb4KSettings::self()->useIncludeFromItem()->label(), includeFromWidget);
    includeFromButton->setObjectName("kcfg_UseIncludeFrom");
    KUrlRequester *includeFrom = new KUrlRequester(includeFromWidget);
    includeFrom->setObjectName("kcfg_IncludeFrom");
    includeFrom->setMode(KFile::File | KFile::LocalOnly);

    includeFromWidgetLayout->addWidget(includeFromButton);
    includeFromWidgetLayout->addWidget(includeFrom);

    filterTabLayout->addRow(QString(), includeFromWidget);

    // Filter rules
    QCheckBox *useFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFilterRuleItem()->label(), filterTab);
    useFFilterRule->setObjectName("kcfg_UseFFilterRule");

    filterTabLayout->addRow(i18n("Special filter rules:"), useFFilterRule);

    QCheckBox *useFFFilterRule = new QCheckBox(Smb4KSettings::self()->useFFFilterRuleItem()->label(), filterTab);
    useFFFilterRule->setObjectName("kcfg_UseFFFilterRule");

    filterTabLayout->addRow(QString(), useFFFilterRule);

    KLineEdit *customRules = new KLineEdit(filterTab);
    customRules->setObjectName("kcfg_CustomFilteringRules");
    customRules->setClearButtonEnabled(true);

    filterTabLayout->addRow(Smb4KSettings::self()->customFilteringRulesItem()->label(), customRules);

    addTab(filterTab, i18n("Filtering"));

    //
    // 'Miscellaneous' tab
    //
    QWidget *miscTab = new QWidget(this);
    QFormLayout *miscTabLayout = new QFormLayout(miscTab);

    // Checksums
    QWidget *blockSizeWidget = new QWidget(miscTab);
    QHBoxLayout *blockSizeWidgetLayout = new QHBoxLayout(blockSizeWidget);
    blockSizeWidgetLayout->setMargin(0);

    QCheckBox *blockSizeButton = new QCheckBox(Smb4KSettings::self()->useBlockSizeItem()->label(), miscTab);
    blockSizeButton->setObjectName("kcfg_UseBlockSize");
    QSpinBox *blockSize = new QSpinBox(miscTab);
    blockSize->setObjectName("kcfg_BlockSize");

    blockSizeWidgetLayout->addWidget(blockSizeButton);
    blockSizeWidgetLayout->addWidget(blockSize);

    miscTabLayout->addRow(i18n("Checksums:"), blockSizeWidget);

    QWidget *checksumSeedWidget = new QWidget(miscTab);
    QHBoxLayout *checksumSeedWidgetLayout = new QHBoxLayout(checksumSeedWidget);
    checksumSeedWidgetLayout->setMargin(0);

    QCheckBox *checksumSeedButton = new QCheckBox(Smb4KSettings::self()->useChecksumSeedItem()->label(), checksumSeedWidget);
    checksumSeedButton->setObjectName("kcfg_UseChecksumSeed");
    QSpinBox *checksumSeed = new QSpinBox(checksumSeedWidget);
    checksumSeed->setObjectName("kcfg_ChecksumSeed");

    checksumSeedWidgetLayout->addWidget(checksumSeedButton);
    checksumSeedWidgetLayout->addWidget(checksumSeed);

    miscTabLayout->addRow(QString(), checksumSeedWidget);

    QCheckBox *useChecksum = new QCheckBox(Smb4KSettings::self()->useChecksumItem()->label(), miscTab);
    useChecksum->setObjectName("kcfg_UseChecksum");

    miscTabLayout->addRow(QString(), useChecksum);

    // Miscellaneous
    QCheckBox *oneFilesystem = new QCheckBox(Smb4KSettings::self()->oneFileSystemItem()->label(), miscTab);
    oneFilesystem->setObjectName("kcfg_OneFileSystem");

    miscTabLayout->addRow(i18n("Miscellaneous:"), oneFilesystem);

    QCheckBox *delayUpdates = new QCheckBox(Smb4KSettings::self()->delayUpdatesItem()->label(), miscTab);
    delayUpdates->setObjectName("kcfg_DelayUpdates");

    miscTabLayout->addRow(QString(), delayUpdates);

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
