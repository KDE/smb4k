/***************************************************************************
    The configuration page for the synchronization options
                             -------------------
    begin                : So Nov 20 2005
    copyright            : (C) 2005-2016 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kconfigpagesynchronization.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlRequester>
#include <KCompletion/KLineEdit>


Smb4KConfigPageSynchronization::Smb4KConfigPageSynchronization(QWidget *parent)
: QTabWidget(parent)
{
  //
  // The Copying tab
  //
  QWidget *copying_tab          = new QWidget(this);

  QGridLayout *copying_layout   = new QGridLayout(copying_tab);
  copying_layout->setSpacing(5);
  copying_layout->setMargin(0);

  // Directories
  QGroupBox *directory_box      = new QGroupBox(i18n("Default Destination"), copying_tab);

  QGridLayout *directory_layout = new QGridLayout(directory_box);
  directory_layout->setSpacing(5);

  QLabel *rsync_prefix_label    = new QLabel(Smb4KSettings::self()->rsyncPrefixItem()->label(),
                                  directory_box);

  KUrlRequester *prefix = new KUrlRequester(directory_box);
  prefix->setMode(KFile::Directory | KFile::LocalOnly);
  prefix->setObjectName("kcfg_RsyncPrefix");

  rsync_prefix_label->setBuddy(prefix);

  directory_layout->addWidget(rsync_prefix_label, 0, 0, 0);
  directory_layout->addWidget(prefix, 0, 1, 0);

  // General
  QGroupBox *general_box        = new QGroupBox(i18n("General"), copying_tab);

  QGridLayout *general_layout   = new QGridLayout(general_box);
  general_layout->setSpacing(5);

  QCheckBox *archive_mode       = new QCheckBox(Smb4KSettings::self()->archiveModeItem()->label(),
                                  general_box);
  archive_mode->setObjectName("kcfg_ArchiveMode");

  QCheckBox *recursive          = new QCheckBox(Smb4KSettings::self()->recurseIntoDirectoriesItem()->label(),
                                  general_box);
  recursive->setObjectName("kcfg_RecurseIntoDirectories");

  QCheckBox *update             = new QCheckBox(Smb4KSettings::self()->updateTargetItem()->label(),
                                  general_box);
  update->setObjectName("kcfg_UpdateTarget");

  QCheckBox *inplace            = new QCheckBox(Smb4KSettings::self()->updateInPlaceItem()->label(),
                                  general_box);
  inplace->setObjectName("kcfg_UpdateInPlace");

  QCheckBox *relative_paths     = new QCheckBox(Smb4KSettings::self()->relativePathNamesItem()->label(),
                                  general_box);
  relative_paths->setObjectName("kcfg_RelativePathNames");

  QCheckBox *no_implied_dirs    = new QCheckBox(Smb4KSettings::self()->noImpliedDirectoriesItem()->label(),
                                  general_box);
  no_implied_dirs->setObjectName("kcfg_NoImpliedDirectories");

  QCheckBox *transfer_dirs      = new QCheckBox(Smb4KSettings::self()->transferDirectoriesItem()->label(),
                                  general_box);
  transfer_dirs->setObjectName("kcfg_TransferDirectories");

  QCheckBox *compress_data      = new QCheckBox(Smb4KSettings::self()->compressDataItem()->label(),
                                  general_box);
  compress_data->setObjectName("kcfg_CompressData");

  general_layout->addWidget(archive_mode, 0, 0, 0);
  general_layout->addWidget(recursive, 0, 1, 0);
  general_layout->addWidget(update, 1, 0, 0);
  general_layout->addWidget(inplace, 1, 1, 0);
  general_layout->addWidget(relative_paths, 2, 0, 0);
  general_layout->addWidget(no_implied_dirs, 2, 1, 0);
  general_layout->addWidget(transfer_dirs, 3, 0, 0);
  general_layout->addWidget(compress_data, 3, 1, 0);

  // Links
  QGroupBox *links_box          = new QGroupBox(i18n("Links"), copying_tab);

  QGridLayout *links_layout     = new QGridLayout(links_box);
  links_layout->setSpacing(5);

  QCheckBox *preserve_links     = new QCheckBox(Smb4KSettings::self()->preserveSymlinksItem()->label(),
                                  links_box);
  preserve_links->setObjectName("kcfg_PreserveSymlinks");

  QCheckBox *transform_links    = new QCheckBox(Smb4KSettings::self()->transformSymlinksItem()->label(),
                                  links_box);
  transform_links->setObjectName("kcfg_TransformSymlinks");

  QCheckBox *transform_unsafe   = new QCheckBox(Smb4KSettings::self()->transformUnsafeSymlinksItem()->label(),
                                  links_box);
  transform_unsafe->setObjectName("kcfg_TransformUnsafeSymlinks");

  QCheckBox *ignore_unsafe      = new QCheckBox(Smb4KSettings::self()->ignoreUnsafeSymlinksItem()->label(),
                                  links_box);
  ignore_unsafe->setObjectName("kcfg_IgnoreUnsafeSymlinks");

  QCheckBox *preserve_hlinks    = new QCheckBox(Smb4KSettings::self()->preserveHardLinksItem()->label(),
                                  links_box);
  preserve_hlinks->setObjectName("kcfg_PreserveHardLinks");

  QCheckBox *keep_dir_links     = new QCheckBox(Smb4KSettings::self()->keepDirectorySymlinksItem()->label(),
                                  links_box);
  keep_dir_links->setObjectName("kcfg_KeepDirectorySymlinks");

  links_layout->addWidget(preserve_links, 0, 0, 0);
  links_layout->addWidget(transform_links, 0, 1, 0);
  links_layout->addWidget(transform_unsafe, 1, 0, 0);
  links_layout->addWidget(ignore_unsafe, 1, 1, 0);
  links_layout->addWidget(preserve_hlinks, 2, 0, 0);
  links_layout->addWidget(keep_dir_links, 2, 1, 0);

  // Permissions
  QGroupBox *perm_box           = new QGroupBox(i18n("File Permissions, etc."), copying_tab);

  QGridLayout *perm_layout      = new QGridLayout(perm_box);
  perm_layout->setSpacing(5);

  QCheckBox *preserve_perms     = new QCheckBox(Smb4KSettings::self()->preservePermissionsItem()->label(),
                                  perm_box);
  preserve_perms->setObjectName("kcfg_PreservePermissions");

  QCheckBox *preserve_group     = new QCheckBox(Smb4KSettings::self()->preserveGroupItem()->label(),
                                  perm_box);
  preserve_group->setObjectName("kcfg_PreserveGroup");

  QCheckBox *preserve_owner     = new QCheckBox(Smb4KSettings::self()->preserveOwnerItem()->label(),
                                  perm_box);
  preserve_owner->setObjectName("kcfg_PreserveOwner");

  QCheckBox *preserve_devices   = new QCheckBox(Smb4KSettings::self()->preserveDevicesAndSpecialsItem()->label(),
                                  perm_box);
  preserve_devices->setObjectName("kcfg_PreserveDevicesAndSpecials");

  QCheckBox *preserve_times     = new QCheckBox(Smb4KSettings::self()->preserveTimesItem()->label(),
                                  perm_box);
  preserve_times->setObjectName("kcfg_PreserveTimes");

  QCheckBox *omit_dir_times     = new QCheckBox(Smb4KSettings::self()->omitDirectoryTimesItem()->label(),
                                  perm_box);
  omit_dir_times->setObjectName("kcfg_OmitDirectoryTimes");

  perm_layout->addWidget(preserve_perms, 0, 0, 0);
  perm_layout->addWidget(preserve_group, 0, 1, 0);
  perm_layout->addWidget(preserve_owner, 1, 0, 0);
  perm_layout->addWidget(preserve_devices, 1, 1, 0);
  perm_layout->addWidget(preserve_times, 2, 0, 0);
  perm_layout->addWidget(omit_dir_times, 2, 1, 0);

  QSpacerItem *spacer1 = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

  copying_layout->addWidget(directory_box, 0, 0, 0);
  copying_layout->addWidget(general_box, 1, 0, 0);
  copying_layout->addWidget(links_box, 2, 0, 0);
  copying_layout->addWidget(perm_box, 3, 0, 0);
  copying_layout->addItem(spacer1, 4, 0, 1, 1, 0);

  insertTab(CopyingTab, copying_tab, i18n("Copying"));

  //
  // The File Deletion & Transfer tab
  //
  QWidget *deltrans_tab         = new QWidget(this);

  QGridLayout *deltrans_layout  = new QGridLayout(deltrans_tab);
  deltrans_layout->setSpacing(5);
  deltrans_layout->setMargin(0);

  // File deletion
  QGroupBox *delete_box         = new QGroupBox(i18n("File Deletion"), deltrans_tab);

  QGridLayout *delete_layout    = new QGridLayout(delete_box);
  delete_layout->setSpacing(5);

  QCheckBox *remove_source      = new QCheckBox(Smb4KSettings::self()->removeSourceFilesItem()->label(),
                                  delete_box);
  remove_source->setObjectName("kcfg_RemoveSourceFiles");

  QCheckBox *delete_extraneous  = new QCheckBox(Smb4KSettings::self()->deleteExtraneousItem()->label(),
                                  delete_box);
  delete_extraneous->setObjectName("kcfg_DeleteExtraneous");

  QCheckBox *delete_before      = new QCheckBox(Smb4KSettings::self()->deleteBeforeItem()->label(),
                                  delete_box);
  delete_before->setObjectName("kcfg_DeleteBefore");

  QCheckBox *delete_after       = new QCheckBox(Smb4KSettings::self()->deleteAfterItem()->label(),
                                  delete_box);
  delete_after->setObjectName("kcfg_DeleteAfter");

  QCheckBox *delete_during      = new QCheckBox(Smb4KSettings::self()->deleteDuringItem()->label(),
                                  delete_box);
  delete_during->setObjectName("kcfg_DeleteDuring");

  QCheckBox *delete_excluded    = new QCheckBox(Smb4KSettings::self()->deleteExcludedItem()->label(),
                                  delete_box);
  delete_excluded->setObjectName("kcfg_DeleteExcluded");

  QCheckBox *ignore_io_errors   = new QCheckBox(Smb4KSettings::self()->ignoreErrorsItem()->label(),
                                  delete_box);
  ignore_io_errors->setObjectName("kcfg_IgnoreErrors");

  QCheckBox *force_dir_deletion = new QCheckBox(Smb4KSettings::self()->forceDirectoryDeletionItem()->label(),
                                  delete_box);
  force_dir_deletion->setObjectName("kcfg_ForceDirectoryDeletion");

  delete_layout->addWidget(remove_source, 0, 0, 0);
  delete_layout->addWidget(delete_extraneous, 0, 1, 0);
  delete_layout->addWidget(delete_before, 1, 0, 0);
  delete_layout->addWidget(delete_after, 1, 1, 0);
  delete_layout->addWidget(delete_during, 2, 0, 0);
  delete_layout->addWidget(delete_excluded, 2, 1, 0);
  delete_layout->addWidget(ignore_io_errors, 3, 0, 0);
  delete_layout->addWidget(force_dir_deletion, 3, 1, 0);

  // Restrictions
  QGroupBox *restrictions_box   = new QGroupBox(i18n("Restrictions"), deltrans_tab);

  QGridLayout *restrict_layout  = new QGridLayout(restrictions_box);
  restrict_layout->setSpacing(5);

  QCheckBox *max_number_button  = new QCheckBox(Smb4KSettings::self()->useMaximumDeleteItem()->label(),
                                  restrictions_box);
  max_number_button->setObjectName("kcfg_UseMaximumDelete");

  QSpinBox *max_number = new QSpinBox(restrictions_box);
  max_number->setObjectName("kcfg_MaximumDeleteValue");

  restrict_layout->addWidget(max_number_button, 0, 0, 0);
  restrict_layout->addWidget(max_number, 0, 1, 0);

  // File transfer
  QGroupBox *transfer_box       = new QGroupBox(i18n("File Transfer"), deltrans_tab);

  QGridLayout *transfer_layout  = new QGridLayout(transfer_box);
  transfer_layout->setSpacing(5);

  QCheckBox *min_size_button    = new QCheckBox(Smb4KSettings::self()->useMinimalTransferSizeItem()->label(),
                                  transfer_box);
  min_size_button->setObjectName("kcfg_UseMinimalTransferSize");

  QSpinBox *min_size = new QSpinBox(transfer_box);
  min_size->setObjectName("kcfg_MinimalTransferSize");
  min_size->setSuffix(i18n(" kB"));

  QCheckBox *max_size_button    = new QCheckBox(Smb4KSettings::self()->useMaximalTransferSizeItem()->label(),
                                  transfer_box);
  max_size_button->setObjectName("kcfg_UseMaximalTransferSize");

  QSpinBox *max_size = new QSpinBox(transfer_box);
  max_size->setObjectName("kcfg_MaximalTransferSize");
  max_size->setSuffix(i18n(" kB"));

  QCheckBox *keep_partial       = new QCheckBox(Smb4KSettings::self()->keepPartialItem()->label(),
                                  transfer_box);
  keep_partial->setObjectName("kcfg_KeepPartial");

  QCheckBox *partial_dir_button = new QCheckBox(Smb4KSettings::self()->usePartialDirectoryItem()->label(),
                                  transfer_box);
  partial_dir_button->setObjectName("kcfg_UsePartialDirectory");

  KUrlRequester *partial_dir    = new KUrlRequester(transfer_box);
  partial_dir->setObjectName("kcfg_PartialDirectory");
  partial_dir->setMode(KFile::Directory | KFile::LocalOnly);

  transfer_layout->addWidget(min_size_button, 0, 0, 0);
  transfer_layout->addWidget(min_size, 0, 1, 0);
  transfer_layout->addWidget(max_size_button, 1, 0, 0);
  transfer_layout->addWidget(max_size, 1, 1, 0);
  transfer_layout->addWidget(keep_partial, 2, 0, 0);
  transfer_layout->addWidget(partial_dir_button, 3, 0, 0);
  transfer_layout->addWidget(partial_dir, 3, 1, 0);

  QSpacerItem *spacer2 = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

  deltrans_layout->addWidget(delete_box, 0, 0, 0);
  deltrans_layout->addWidget(restrictions_box, 1, 0, 0);
  deltrans_layout->addWidget(transfer_box, 2, 0, 0);
  deltrans_layout->addItem(spacer2, 3, 0, 1, 1, 0);

  insertTab(DelTransTab, deltrans_tab, i18n("File Deletion && Transfer"));

  //
  // The Filter tab
  //
  QWidget *filter_tab           = new QWidget(this);

  QGridLayout *filter_layout    = new QGridLayout(filter_tab);
  filter_layout->setSpacing(5);
  filter_layout->setMargin(0);

  // General
  QGroupBox *general_filter_box = new QGroupBox(i18n("General"), filter_tab);

  QGridLayout *g_filter_layout  = new QGridLayout(general_filter_box);
  g_filter_layout->setSpacing(5);

  QCheckBox *cvs_exclude        = new QCheckBox(Smb4KSettings::self()->useCVSExcludeItem()->label(),
                                  general_filter_box);
  cvs_exclude->setObjectName("kcfg_UseCVSExclude");

  QCheckBox *ex_pattern_button  = new QCheckBox(Smb4KSettings::self()->useExcludePatternItem()->label(),
                                  general_filter_box);
  ex_pattern_button->setObjectName("kcfg_UseExcludePattern");

  KLineEdit *exclude_pattern    = new KLineEdit(general_filter_box);
  exclude_pattern->setObjectName("kcfg_ExcludePattern");

  QCheckBox *ex_from_button     = new QCheckBox(Smb4KSettings::self()->useExcludeFromItem()->label(),
                                  general_filter_box);
  ex_from_button->setObjectName("kcfg_UseExcludeFrom");

  KUrlRequester *exclude_from   = new KUrlRequester(general_filter_box);
  exclude_from->setObjectName("kcfg_ExcludeFrom");
  exclude_from->setMode(KFile::File | KFile::LocalOnly);

  QCheckBox *in_pattern_button  = new QCheckBox(Smb4KSettings::self()->useIncludePatternItem()->label(),
                                  general_filter_box);
  in_pattern_button->setObjectName("kcfg_UseIncludePattern");

  KLineEdit *include_pattern    = new KLineEdit(general_filter_box);
  include_pattern->setObjectName("kcfg_IncludePattern");

  QCheckBox *in_from_button     = new QCheckBox(Smb4KSettings::self()->useIncludeFromItem()->label(),
                                  general_filter_box);
  in_from_button->setObjectName("kcfg_UseIncludeFrom");

  KUrlRequester *include_from   = new KUrlRequester(general_filter_box);
  include_from->setObjectName("kcfg_IncludeFrom");
  include_from->setMode(KFile::File | KFile::LocalOnly);

  g_filter_layout->addWidget(cvs_exclude, 0, 0, 1, 2, 0);
  g_filter_layout->addWidget(ex_pattern_button, 1, 0, 0);
  g_filter_layout->addWidget(exclude_pattern, 1, 1, 0);
  g_filter_layout->addWidget(ex_from_button, 2, 0, 0);
  g_filter_layout->addWidget(exclude_from, 2, 1, 0);
  g_filter_layout->addWidget(in_pattern_button, 3, 0, 0);
  g_filter_layout->addWidget(include_pattern, 3, 1, 0);
  g_filter_layout->addWidget(in_from_button, 4, 0, 0);
  g_filter_layout->addWidget(include_from, 4, 1, 0);

  // Filter rules
  QGroupBox *filter_rules_box   = new QGroupBox(i18n("Filter Rules"), filter_tab);

  QGridLayout *f_rules_layout   = new QGridLayout(filter_rules_box);
  f_rules_layout->setSpacing(5);

  QLabel *custom_rules_label    = new QLabel(Smb4KSettings::self()->customFilteringRulesItem()->label(),
                                  filter_rules_box);

  KLineEdit *custom_rules       = new KLineEdit(filter_rules_box);
  custom_rules->setObjectName("kcfg_CustomFilteringRules");

  custom_rules_label->setBuddy(custom_rules);

  QLabel *special_rules         = new QLabel(i18n("Special filter rules:"), filter_rules_box);

  QCheckBox *f_filter           = new QCheckBox(Smb4KSettings::self()->useFFilterRuleItem()->label(),
                                  filter_rules_box);
  f_filter->setObjectName("kcfg_UseFFilterRule");

  QCheckBox *ff_filter          = new QCheckBox(Smb4KSettings::self()->useFFFilterRuleItem()->label(),
                                  filter_rules_box);
  ff_filter->setObjectName("kcfg_UseFFFilterRule");

  f_rules_layout->addWidget(custom_rules_label, 0, 0, 0);
  f_rules_layout->addWidget(custom_rules, 0, 1, 0);
  f_rules_layout->addWidget(special_rules, 1, 0, 1, 2, 0);
  f_rules_layout->addWidget(f_filter, 2, 0, 1, 2, 0);
  f_rules_layout->addWidget(ff_filter, 3, 0, 1, 2, 0);

  QSpacerItem *spacer3 = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

  filter_layout->addWidget(general_filter_box, 0, 0, 0);
  filter_layout->addWidget(filter_rules_box, 1, 0, 0);
  filter_layout->addItem(spacer3, 2, 0, 1, 1, 0);

  insertTab(FilteringTab, filter_tab, i18n("Filtering"));

  //
  // The Advanced tab
  //
  QWidget *advanced_tab         = new QWidget(this);

  QGridLayout *advanced_layout  = new QGridLayout(advanced_tab);
  advanced_layout->setSpacing(5);
  advanced_layout->setMargin(0);

  // General
  QGroupBox *misc_box           = new QGroupBox(i18n("General"), advanced_tab);

  QGridLayout *misc_layout      = new QGridLayout(misc_box);
  misc_layout->setSpacing(5);

  QCheckBox *sparse_files       = new QCheckBox(Smb4KSettings::self()->efficientSparseFileHandlingItem()->label(),
                                  misc_box);
  sparse_files->setObjectName("kcfg_EfficientSparseFileHandling");

  QCheckBox *copy_whole         = new QCheckBox(Smb4KSettings::self()->copyFilesWholeItem()->label(),
                                  misc_box);
  copy_whole->setObjectName("kcfg_CopyFilesWhole");

  QCheckBox *one_filesystem     = new QCheckBox(Smb4KSettings::self()->oneFileSystemItem()->label(),
                                  misc_box);
  one_filesystem->setObjectName("kcfg_OneFileSystem");

  QCheckBox *update_existing    = new QCheckBox(Smb4KSettings::self()->updateExistingItem()->label(),
                                  misc_box);
  update_existing->setObjectName("kcfg_UpdateExisting");

  QCheckBox *ignore_existing    = new QCheckBox(Smb4KSettings::self()->ignoreExistingItem()->label(),
                                  misc_box);
  ignore_existing->setObjectName("kcfg_IgnoreExisting");

  QCheckBox *delay_updates      = new QCheckBox(Smb4KSettings::self()->delayUpdatesItem()->label(),
                                  misc_box);
  delay_updates->setObjectName("kcfg_DelayUpdates");

  misc_layout->addWidget(sparse_files, 0, 0, 0);
  misc_layout->addWidget(copy_whole, 0, 1, 0);
  misc_layout->addWidget(one_filesystem, 1, 0, 0);
  misc_layout->addWidget(update_existing, 1, 1, 0);
  misc_layout->addWidget(ignore_existing, 2, 0, 0);
  misc_layout->addWidget(delay_updates, 2, 1, 0);

  // Backup
  QGroupBox *backup_box         = new QGroupBox(i18n("Backup"), advanced_tab);

  QGridLayout *backup_layout    = new QGridLayout(backup_box);
  backup_layout->setSpacing(5);

  QCheckBox *backup             = new QCheckBox(Smb4KSettings::self()->makeBackupsItem()->label(),
                                  backup_box);
  backup->setObjectName("kcfg_MakeBackups");

  QCheckBox *backup_suf_button  = new QCheckBox(Smb4KSettings::self()->useBackupSuffixItem()->label(),
                                  backup_box);
  backup_suf_button->setObjectName("kcfg_UseBackupSuffix");

  KLineEdit *backup_suffix      = new KLineEdit(backup_box);
  backup_suffix->setObjectName("kcfg_BackupSuffix");

  QCheckBox *backup_dir_button  = new QCheckBox(Smb4KSettings::self()->useBackupDirectoryItem()->label(),
                                  backup_box);
  backup_dir_button->setObjectName("kcfg_UseBackupDirectory");

  KUrlRequester *backup_dir     = new KUrlRequester(backup_box);
  backup_dir->setObjectName("kcfg_BackupDirectory");
  backup_dir->setMode(KFile::Directory | KFile::LocalOnly);

  backup_layout->addWidget(backup, 0, 0, 0);
  backup_layout->addWidget(backup_suf_button, 1, 0, 0);
  backup_layout->addWidget(backup_suffix, 1, 1, 0);
  backup_layout->addWidget(backup_dir_button, 2, 0, 0);
  backup_layout->addWidget(backup_dir, 2, 1, 0);

  // Checksums
  QGroupBox *checksum_box       = new QGroupBox(i18n("Checksums"), advanced_tab);

  QGridLayout *checksum_layout  = new QGridLayout(checksum_box);
  checksum_layout->setSpacing(5);

  QCheckBox *block_size_button  = new QCheckBox(Smb4KSettings::self()->useBlockSizeItem()->label(),
                                  checksum_box);
  block_size_button->setObjectName("kcfg_UseBlockSize");

  QSpinBox *block_size = new QSpinBox(checksum_box);
  block_size->setObjectName("kcfg_BlockSize");

  QCheckBox *chksum_seed_button = new QCheckBox(Smb4KSettings::self()->useChecksumSeedItem()->label(),
                                  checksum_box);
  chksum_seed_button->setObjectName("kcfg_UseChecksumSeed");

  QSpinBox *checksum_seed = new QSpinBox(checksum_box);
  checksum_seed->setObjectName("kcfg_ChecksumSeed");

  QCheckBox *use_checksum       = new QCheckBox(Smb4KSettings::self()->useChecksumItem()->label(),
                                  checksum_box);
  use_checksum->setObjectName("kcfg_UseChecksum");

  checksum_layout->addWidget(block_size_button, 0, 0, 0);
  checksum_layout->addWidget(block_size, 0, 1, 0);
  checksum_layout->addWidget(chksum_seed_button, 1, 0, 0);
  checksum_layout->addWidget(checksum_seed, 1, 1, 0);
  checksum_layout->addWidget(use_checksum, 2, 0, 0);

  QSpacerItem *spacer4 = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

  advanced_layout->addWidget(misc_box, 0, 0, 0);
  advanced_layout->addWidget(backup_box, 1, 0, 0);
  advanced_layout->addWidget(checksum_box, 2, 0, 0);
  advanced_layout->addItem(spacer4, 3, 0, 1, 1, 0);

  insertTab(AdvancedTab, advanced_tab, i18n("Advanced Settings"));

  connect(archive_mode,     SIGNAL(toggled(bool)),
           this,             SLOT(slotArchiveToggled(bool)));

  connect(recursive,        SIGNAL(toggled(bool)),
           this,             SLOT(slotUncheckArchive(bool)));

  connect(preserve_links,   SIGNAL(toggled(bool)),
           this,             SLOT(slotUncheckArchive(bool)));

  connect(preserve_perms,   SIGNAL(toggled(bool)),
           this,             SLOT(slotUncheckArchive(bool)));

  connect(preserve_times,   SIGNAL(toggled(bool)),
           this,             SLOT(slotUncheckArchive(bool)));

  connect(preserve_group,   SIGNAL(toggled(bool)),
           this,             SLOT(slotUncheckArchive(bool)));

  connect(preserve_owner,   SIGNAL(toggled(bool)),
           this,             SLOT(slotUncheckArchive(bool)));

  connect(preserve_devices, SIGNAL(toggled(bool)),
           this,             SLOT(slotUncheckArchive(bool)));

  connect(backup,           SIGNAL(toggled(bool)),
           this,             SLOT(slotBackupToggled(bool)));

  connect(f_filter,         SIGNAL(toggled(bool)),
           this,             SLOT(slotFShortcutToggled(bool)));

  connect(ff_filter,        SIGNAL(toggled(bool)),
           this,             SLOT(slotFFShortcutToggled(bool)));

  slotArchiveToggled(true);
  slotBackupToggled(false);
}


Smb4KConfigPageSynchronization::~Smb4KConfigPageSynchronization()
{
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigPageSynchronization::slotArchiveToggled(bool checked)
{
  if (checked)
  {
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
  if (!checked)
  {
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


void Smb4KConfigPageSynchronization::slotFShortcutToggled(bool on)
{
  QCheckBox *ff_filter = findChild<QCheckBox *>("kcfg_UseFFFilterRule");

  if (on && ff_filter->isChecked())
  {
    ff_filter->setChecked(false);
  }
}


void Smb4KConfigPageSynchronization::slotFFShortcutToggled(bool on)
{
  QCheckBox *f_filter = findChild<QCheckBox *>("kcfg_UseFFilterRule");

  if (on && f_filter->isChecked())
  {
    f_filter->setChecked(false);
  }
}

