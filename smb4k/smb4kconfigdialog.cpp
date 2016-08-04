/***************************************************************************
    The configuration dialog of Smb4K
                             -------------------
    begin                : Sa Apr 14 2007
    copyright            : (C) 2004-2016 by Alexander Reinholdt
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
#include "smb4kconfigdialog.h"
#include "smb4kconfigpageauthentication.h"
#include "smb4kconfigpagecustomoptions.h"
#include "smb4kconfigpagemounting.h"
#include "smb4kconfigpagenetwork.h"
#include "smb4kconfigpageprofiles.h"
#include "smb4kconfigpagesamba.h"
#include "smb4kconfigpageshares.h"
#include "smb4kconfigpagesynchronization.h"
#include "smb4kconfigpageuserinterface.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"
#include "core/smb4kauthinfo.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kprofilemanager.h"

#if defined(Q_OS_LINUX)
#include "core/smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "core/smb4kmountsettings_freebsd.h"
#else
#define UNSUPPORTED_PLATFORM
#endif

// Qt includes
#include <QtCore/QPointer>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QSize>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QScrollArea>
#include <QtGui/QShowEvent>

// KDE includes
#include <KCoreAddons/KPluginFactory>
#include <KI18n/KLocalizedString>
#include <KConfigGui/KWindowConfig>
#include <KWidgetsAddons/KMessageBox>
#include <KWidgetsAddons/KPasswordDialog>
#include <KIOWidgets/KUrlRequester>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY(Smb4KConfigDialogFactory, registerPlugin<Smb4KConfigDialog>();)


Smb4KConfigDialog::Smb4KConfigDialog(QWidget *parent, const QList<QVariant> &/*args*/)
: KConfigDialog(parent, "ConfigDialog", Smb4KSettings::self())
{
  setupDialog();
}


Smb4KConfigDialog::~Smb4KConfigDialog()
{
}


void Smb4KConfigDialog::setupDialog()
{
  // FIXME: I guess, normally we would not need to close destructively,
  // but at the moment there are issues with the KURLRequester in file
  // mode. To work around those, we are closing the dialog destructively.
  // Maybe we can remove this if we moved to KDE4.
  setAttribute(Qt::WA_DeleteOnClose, true);

  // Add the pages:
  Smb4KConfigPageUserInterface *interface_options = new Smb4KConfigPageUserInterface(this);
  QScrollArea *interface_area = new QScrollArea(this);
  interface_area->setWidget(interface_options);
  interface_area->setWidgetResizable(true);
  interface_area->setFrameStyle(QFrame::NoFrame);

  Smb4KConfigPageNetwork *network_options = new Smb4KConfigPageNetwork(this);
  QScrollArea *network_area = new QScrollArea(this);
  network_area->setWidget(network_options);
  network_area->setWidgetResizable(true);
  network_area->setFrameStyle(QFrame::NoFrame);

  Smb4KConfigPageShares *share_options = new Smb4KConfigPageShares(this);
  QScrollArea *share_area = new QScrollArea(this);
  share_area->setWidget(share_options);
  share_area->setWidgetResizable(true);
  share_area->setFrameStyle(QFrame::NoFrame);

  Smb4KConfigPageAuthentication *auth_options = new Smb4KConfigPageAuthentication(this);
  QScrollArea *auth_area = new QScrollArea(this);
  auth_area->setWidget(auth_options);
  auth_area->setWidgetResizable(true);
  auth_area->setFrameStyle(QFrame::NoFrame);

  Smb4KConfigPageSamba *samba_options = new Smb4KConfigPageSamba(this);
  QScrollArea *samba_area = new QScrollArea(this);
  samba_area->setWidget(samba_options);
  samba_area->setWidgetResizable(true);
  samba_area->setFrameStyle(QFrame::NoFrame);

#if !defined(UNSUPPORTED_PLATFORM)  
  Smb4KConfigPageMounting *mount_options = new Smb4KConfigPageMounting(this);
  QScrollArea *mount_area = new QScrollArea(this);
  mount_area->setWidget(mount_options);
  mount_area->setWidgetResizable(true);
  mount_area->setFrameStyle(QFrame::NoFrame);
#endif

  Smb4KConfigPageSynchronization *rsync_options = new Smb4KConfigPageSynchronization(this);
  QScrollArea *rsync_area = new QScrollArea(this);
  rsync_area->setWidget(rsync_options);
  rsync_area->setWidgetResizable(true);
  rsync_area->setFrameStyle(QFrame::NoFrame);
  
  rsync_options->setEnabled(!QStandardPaths::findExecutable("rsync").isEmpty());

  Smb4KConfigPageCustomOptions *custom_options = new Smb4KConfigPageCustomOptions(this);
  QScrollArea *custom_area = new QScrollArea(this);
  custom_area->setWidget(custom_options);
  custom_area->setWidgetResizable(true);
  custom_area->setFrameStyle(QFrame::NoFrame);
  
  Smb4KConfigPageProfiles *profiles_page = new Smb4KConfigPageProfiles(this);
  QScrollArea *profiles_area = new QScrollArea(this);
  profiles_area->setWidget(profiles_page);
  profiles_area->setWidgetResizable(true);
  profiles_area->setFrameStyle(QFrame::NoFrame);

  // Now add the pages to the configuration dialog
  m_user_interface  = addPage(interface_area, Smb4KSettings::self(), i18n("User Interface"), "view-choose");
  m_network         = addPage(network_area, Smb4KSettings::self(), i18n("Network"), "network-workgroup");
  m_shares          = addPage(share_area, Smb4KSettings::self(), i18n("Shares"), "folder-remote");
  m_authentication  = addPage(auth_area, Smb4KSettings::self(), i18n("Authentication"), "dialog-password");
  m_samba           = addPage(samba_area, Smb4KSettings::self(), i18n("Samba"), "preferences-system-network");
#if !defined(UNSUPPORTED_PLATFORM)
  m_mounting        = addPage(mount_area, Smb4KMountSettings::self(), i18n("Mounting"), "system-run");
#endif
  m_synchronization = addPage(rsync_area, Smb4KSettings::self(),i18n("Synchronization"), "folder-sync");
  m_custom_options  = addPage(custom_area, Smb4KSettings::self(), i18n("Custom Options"), "preferences-system-network");
  m_profiles        = addPage(profiles_area, Smb4KSettings::self(), i18n("Profiles"), "format-list-unordered");

  // Connections
  connect(custom_options, SIGNAL(customSettingsModified()),
          this,           SLOT(slotEnableApplyButton()));
  
  connect(custom_options, SIGNAL(reloadCustomSettings()),
          this,           SLOT(slotReloadCustomOptions()));

  connect(auth_options,  SIGNAL(loadWalletEntries()),
          this,          SLOT(slotLoadAuthenticationInformation()));
           
  connect(auth_options,  SIGNAL(saveWalletEntries()),
          this,          SLOT(slotSaveAuthenticationInformation()));
           
  connect(auth_options,  SIGNAL(setDefaultLogin()),
          this,          SLOT(slotSetDefaultLogin()));
           
  connect(auth_options,  SIGNAL(walletEntriesModified()),
          this,          SLOT(slotEnableApplyButton()));
  
  connect(this,          SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
          this,          SLOT(slotCheckPage(KPageWidgetItem*,KPageWidgetItem*)));
  
  resize(QSize(800, 600));

  KConfigGroup group(Smb4KSettings::self()->config(), "ConfigDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
}


void Smb4KConfigDialog::loadCustomOptions()
{
  if (m_custom_options)
  {
    QList<Smb4KCustomOptions *> options = Smb4KCustomOptionsManager::self()->customOptions();
    m_custom_options->widget()->findChild<Smb4KConfigPageCustomOptions *>()->insertCustomOptions(options);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::saveCustomOptions()
{
  if (m_custom_options)
  {
    QList<Smb4KCustomOptions *> options = m_custom_options->widget()->findChild<Smb4KConfigPageCustomOptions *>()->getCustomOptions();
    Smb4KCustomOptionsManager::self()->replaceCustomOptions(options);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::propagateProfilesChanges()
{
  Smb4KConfigPageProfiles *profiles_page = m_profiles->widget()->findChild<Smb4KConfigPageProfiles *>();
  
  if (profiles_page)
  {
    // Remove the profiles.
    QStringList removed_profiles = profiles_page->removedProfiles();
    
    if (!removed_profiles.isEmpty())
    {
      Smb4KProfileManager::self()->removeProfiles(removed_profiles, this);
      profiles_page->clearRemovedProfiles();
    }
    else
    {
      // Do nothing
    }
    
    // Rename the profiles.
    QList< QPair<QString,QString> > renamed_profiles = profiles_page->renamedProfiles();
    
    if (!renamed_profiles.isEmpty())
    {
      Smb4KProfileManager::self()->migrateProfiles(renamed_profiles);
      profiles_page->clearRenamedProfiles();
    }
    else
    {
      // Do nothing
    }
    
    // Finally reload the custom options.
    if (!removed_profiles.isEmpty() || !renamed_profiles.isEmpty())
    {
      loadCustomOptions();
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KConfigDialog::checkNetworkPage()
{
  QRadioButton *query_custom_master = m_network->widget()->findChild<QRadioButton *>("kcfg_QueryCustomMaster");
  KLineEdit *custom_master_input    = m_network->widget()->findChild<KLineEdit *>("kcfg_CustomMasterBrowser");
  
  QString msg = i18n("An incorrect setting has been found. You are now taken to the corresponding dialog page to fix it.");

  if ((query_custom_master && query_custom_master->isChecked()) &&
      (custom_master_input && custom_master_input->text().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_network);
    custom_master_input->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QRadioButton *scan_bcast_areas = m_network->widget()->findChild<QRadioButton *>("kcfg_ScanBroadcastAreas");
  KLineEdit *bcast_areas_input   = m_network->widget()->findChild<KLineEdit *>("kcfg_BroadcastAreas");
  
  if ((scan_bcast_areas && scan_bcast_areas->isChecked()) &&
      (bcast_areas_input && bcast_areas_input->text().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_network);
    bcast_areas_input->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }  
  
  return true;
}


bool Smb4KConfigDialog::checkSharesPage()
{
  KUrlRequester *mount_prefix = m_shares->widget()->findChild<KUrlRequester *>("kcfg_MountPrefix");
  
  QString msg = i18n("An incorrect setting has been found. You are now taken to the corresponding dialog page to fix it.");
  
  if (mount_prefix && mount_prefix->url().path().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_shares);
    mount_prefix->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  return true;
}


bool Smb4KConfigDialog::checkMountingPage()
{
#if !defined(UNSUPPORTED_PLATFORM)
  KLineEdit *file_mask = m_mounting->widget()->findChild<KLineEdit *>("kcfg_FileMask");
  
  QString msg = i18n("An incorrect setting has been found. You are now taken to the corresponding dialog page to fix it.");
  
  if (file_mask && file_mask->text().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_mounting);
    file_mask->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  KLineEdit *directory_mask = m_mounting->widget()->findChild<KLineEdit *>("kcfg_DirectoryMask");
  
  if (directory_mask && directory_mask->text().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_samba);
    directory_mask->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
#endif
  
  return true;
}


bool Smb4KConfigDialog::checkSynchronizationPage()
{
  KUrlRequester *sync_prefix = m_synchronization->widget()->findChild<KUrlRequester *>("kcfg_RsyncPrefix");
  
  QString msg = i18n("An incorrect setting has been found. You are now taken to the corresponding dialog page to fix it.");
  
  if (sync_prefix && sync_prefix->url().path().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::CopyingTab);
    }
    else
    {
      // Do nothing
    }
    
    sync_prefix->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *max_delete = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseMaximumDelete");
  QSpinBox *max_delete_val = m_synchronization->widget()->findChild<QSpinBox *>("kcfg_MaximumDeleteValue");
  
  if ((max_delete && max_delete->isChecked()) && (max_delete_val && max_delete_val->value() == 0))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::DelTransTab);
    }
    else
    {
      // Do nothing
    }
    
    max_delete_val->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *min_trans_size = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseMinimalTransferSize");
  QSpinBox *min_trans_size_val = m_synchronization->widget()->findChild<QSpinBox *>("kcfg_MinimalTransferSize");
  
  if ((min_trans_size && min_trans_size->isChecked()) && (min_trans_size_val && min_trans_size_val->value() == 0))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::DelTransTab);
    }
    else
    {
      // Do nothing
    }
    
    min_trans_size_val->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *max_trans_size = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseMaximalTransferSize");
  QSpinBox *max_trans_size_val = m_synchronization->widget()->findChild<QSpinBox *>("kcfg_MaximalTransferSize");
  
  if ((max_trans_size && max_trans_size->isChecked()) && (max_trans_size_val && max_trans_size_val->value() == 0))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::DelTransTab);
    }
    else
    {
      // Do nothing
    }
    
    max_trans_size_val->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *use_partial_directory = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UsePartialDirectory");
  KUrlRequester *partial_directory = m_synchronization->widget()->findChild<KUrlRequester *>("kcfg_PartialDirectory");
  
  if ((use_partial_directory && use_partial_directory->isChecked()) &&
      (partial_directory && partial_directory->url().path().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::DelTransTab);
    }
    else
    {
      // Do nothing
    }
    
    partial_directory->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *use_exclude_pattern = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseExcludePattern");
  KLineEdit *exclude_pattern = m_synchronization->widget()->findChild<KLineEdit *>("kcfg_ExcludePattern");
  
  if ((use_exclude_pattern && use_exclude_pattern->isChecked()) &&
      (exclude_pattern && exclude_pattern->text().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::FilteringTab);
    }
    else
    {
      // Do nothing
    }
    
    exclude_pattern->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *use_exclude_file = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseExcludeFrom");
  KUrlRequester *exclude_file = m_synchronization->widget()->findChild<KUrlRequester *>("kcfg_ExcludeFrom");
  
  if ((use_exclude_file && use_exclude_file->isChecked()) &&
      (exclude_file && exclude_file->url().path().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::FilteringTab);
    }
    else
    {
      // Do nothing
    }
    
    exclude_file->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *use_include_pattern = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseIncludePattern");
  KLineEdit *include_pattern = m_synchronization->widget()->findChild<KLineEdit *>("kcfg_IncludePattern");
  
  if ((use_include_pattern && use_include_pattern->isChecked()) &&
      (include_pattern && include_pattern->text().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::FilteringTab);
    }
    else
    {
      // Do nothing
    }
    
    include_pattern->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *use_include_file = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseIncludeFrom");
  KUrlRequester *include_file = m_synchronization->widget()->findChild<KUrlRequester *>("kcfg_IncludeFrom");
  
  if ((use_include_file && use_include_file->isChecked()) &&
      (include_file && include_file->url().path().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, msg);
    setCurrentPage(m_synchronization);
    
    Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::FilteringTab);
    }
    else
    {
      // Do nothing
    }
    
    include_file->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  QCheckBox *make_backups = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_MakeBackups");
  
  if (make_backups && make_backups->isChecked())
  {
    QCheckBox *use_backup_suffix = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseBackupSuffix");
    KLineEdit *backup_suffix = m_synchronization->widget()->findChild<KLineEdit *>("kcfg_BackupSuffix");
    
    if ((use_backup_suffix && use_backup_suffix->isChecked()) &&
        (backup_suffix && backup_suffix->text().trimmed().isEmpty()))
    {
      KMessageBox::sorry(this, msg);
      setCurrentPage(m_synchronization);
      
      Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
      
      if (sync_options)
      {
        sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::AdvancedTab);
      }
      else
      {
        // Do nothing
      }
      
      backup_suffix->setFocus();
      return false;
    }
    else
    {
      // Do nothing
    }
    
    QCheckBox *use_backup_dir = m_synchronization->widget()->findChild<QCheckBox *>("kcfg_UseBackupDirectory");
    KUrlRequester *backup_dir = m_synchronization->widget()->findChild<KUrlRequester *>("kcfg_BackupDirectory");
    
    if ((use_backup_dir && use_backup_dir->isChecked()) &&
        (backup_dir && backup_dir->url().path().trimmed().isEmpty()))
    {
      KMessageBox::sorry(this, msg);
      setCurrentPage(m_synchronization);
      
      Smb4KConfigPageSynchronization *sync_options = m_synchronization->widget()->findChild<Smb4KConfigPageSynchronization *>();
      
      if (sync_options)
      {
        sync_options->setCurrentIndex(Smb4KConfigPageSynchronization::AdvancedTab);
      }
      else
      {
        // Do nothing
      }
      
      backup_dir->setFocus();
      return false;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }
  
  // FIXME: Also check --block-size (kcfg_UseBlockSize & kcfg_BlockSize) and 
  // --checksum-seed (kcfg_UseChecksumSeed & kcfg_ChecksumSeed), if necessary.
  // However, I need more info on what values are needed ...

  return true;
}



bool Smb4KConfigDialog::checkSettings()
{
  // Check Network page
  if (!checkNetworkPage())
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Check Shares page
  if (!checkSharesPage())
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Check Mounting page
  if (!checkMountingPage())
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Check Synchronization page
  if (!checkSynchronizationPage())
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  return true;
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KConfigDialog::updateSettings()
{
  saveCustomOptions();
  slotSaveAuthenticationInformation();
  propagateProfilesChanges();
  
  KConfigGroup group(Smb4KSettings::self()->config(), "ConfigDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  
  qDebug() << "Smb4KConfigDialog::updateSettings(): Implement checkSettings()!";
      
  KConfigDialog::updateSettings();
}


void Smb4KConfigDialog::updateWidgets()
{
  loadCustomOptions();
  
  KConfigDialog::updateWidgets();
}


void Smb4KConfigDialog::slotLoadAuthenticationInformation()
{
  Smb4KConfigPageAuthentication *auth_options = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();
  QList<Smb4KAuthInfo *> entries = Smb4KWalletManager::self()->walletEntries();
  auth_options->insertWalletEntries(entries);
  auth_options->displayWalletEntries();
}


void Smb4KConfigDialog::slotSaveAuthenticationInformation()
{
  Smb4KConfigPageAuthentication *auth_options = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();
  
  if (auth_options->walletEntriesDisplayed())
  {
    QList<Smb4KAuthInfo *> entries = auth_options->getWalletEntries();
    Smb4KWalletManager::self()->writeWalletEntries(entries);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::slotSetDefaultLogin()
{
  Smb4KConfigPageAuthentication *auth_options = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();
  
  if (!auth_options->undoRemoval())
  {
    Smb4KAuthInfo authInfo;
    // We do not need to call useDefaultAuthInfo(), because 
    // Smb4KWalletManager::readDefaultAuthInfo() will do this
    // for us.
    Smb4KWalletManager::self()->readDefaultAuthInfo(&authInfo);
    
    QPointer<KPasswordDialog> dlg = new KPasswordDialog(this, KPasswordDialog::ShowUsernameLine);
    dlg->setPrompt(i18n("Enter the default login information."));
    dlg->setUsername(authInfo.userName());
    dlg->setPassword(authInfo.password());
    
    if (dlg->exec() == KPasswordDialog::Accepted)
    {
      authInfo.setUserName(dlg->username());
      authInfo.setPassword(dlg->password());
      
      Smb4KWalletManager::self()->writeDefaultAuthInfo(&authInfo);
      
      if (auth_options->walletEntriesDisplayed())
      {
        slotLoadAuthenticationInformation();
      }
      else
      {
        // Do nothing
      }    
    }
    else
    {
      // Reset the checkbox.
      auth_options->findChild<QCheckBox *>("kcfg_UseDefaultLogin")->setChecked(false);
    }
    
    delete dlg;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::slotEnableApplyButton()
{
  // Check if we need to enable the Apply button.
  bool enable = false;
  
  // Check the wallet entries.
  Smb4KConfigPageAuthentication *auth_options = m_authentication->widget()->findChild<Smb4KConfigPageAuthentication *>();

  if (auth_options->walletEntriesMaybeChanged())
  {
    QList<Smb4KAuthInfo *> old_wallet_entries = Smb4KWalletManager::self()->walletEntries();
    QList<Smb4KAuthInfo *> new_wallet_entries = auth_options->getWalletEntries();
    
    for (int i = 0; i < old_wallet_entries.size(); ++i)
    {
      for (int j = 0; j < new_wallet_entries.size(); ++j)
      {
        if (QString::compare(old_wallet_entries.at(i)->unc(),
                               new_wallet_entries.at(j)->unc(),
                               Qt::CaseInsensitive) == 0 &&
             (QString::compare(old_wallet_entries.at(i)->workgroupName(),
                                new_wallet_entries.at(j)->workgroupName(),
                                Qt::CaseInsensitive) != 0 ||
              QString::compare(old_wallet_entries.at(i)->userName(),
                                new_wallet_entries.at(j)->userName(),
                                Qt::CaseInsensitive) != 0 ||
              QString::compare(old_wallet_entries.at(i)->password(),
                                new_wallet_entries.at(j)->password(),
                                Qt::CaseInsensitive) != 0))
        {
          enable = true;
          break;
        }
        else
        {
          continue;
        }
      }
      
      if (enable)
      {
        break;
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  // Check the custom settings.
  Smb4KConfigPageCustomOptions *custom_options = m_custom_options->widget()->findChild<Smb4KConfigPageCustomOptions *>();
  
  if (!enable && custom_options && custom_options->customSettingsMaybeChanged())
  {
    QList<Smb4KCustomOptions *> new_list = custom_options->getCustomOptions();
    QList<Smb4KCustomOptions *> old_list = Smb4KCustomOptionsManager::self()->customOptions();
    
    if (new_list.size() == old_list.size())
    {
      for (int i = 0; i < new_list.size(); ++i)
      {
        for (int j = 0; j < old_list.size(); ++j)
        {
          if (!new_list[i]->equals(old_list.at(j)))
          {
            enable = true;
            break;
          }
          else
          {
            continue;
          }
        }
        
        if (enable)
        {
          break;
        }
        else
        {
          continue;
        }
      }
    }
    else
    {
      enable = true;
    }
  }
  else
  {
    // Do nothing
  }

  QPushButton *applyButton = buttonBox()->button(QDialogButtonBox::Apply);
  
  if (applyButton)
  {
    applyButton->setEnabled(enable);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::slotReloadCustomOptions()
{
  loadCustomOptions();
}


void Smb4KConfigDialog::slotCheckPage(KPageWidgetItem* /*current*/, KPageWidgetItem* before)
{
  if (before == m_user_interface)
  {
    // Nothing to do
  }
  else if (before == m_network)
  {
    (void)checkNetworkPage();
  }
  else if (before == m_shares)
  {
    (void)checkSharesPage();
  }
  else if (before == m_authentication)
  {
    // Do nothing
  }
  else if (before == m_samba)
  {
    // Do nothing
  }
#if !defined(UNSUPPORTED_PLATFORM)
  else if (before == m_mounting)
  {
    (void)checkMountingPage();
  }
#endif
  else if (before == m_synchronization)
  {
    (void)checkSynchronizationPage();
  }
  else if (before == m_custom_options)
  {
    // Do nothing
  }
  else if (before == m_profiles)
  {
    // Do nothing
  }
  else
  {
    // Do nothing
  }
}

#include "smb4kconfigdialog.moc"
