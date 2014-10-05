/***************************************************************************
    smb4kconfigdialog  -  The configuration dialog of Smb4K
                             -------------------
    begin                : Sa Apr 14 2007
    copyright            : (C) 2004-2014 by Alexander Reinholdt
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
#include "smb4kuserinterfaceoptionspage.h"
#include "smb4knetworkoptionspage.h"
#include "smb4kshareoptionspage.h"
#include "smb4kauthoptionspage.h"
#include "smb4ksambaoptionspage.h"
#include "smb4krsyncoptionspage.h"
#include "smb4klaptopsupportoptionspage.h"
#include "smb4kcustomoptionspage.h"
#include "smb4kprofilespage.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"
#include "core/smb4kauthinfo.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kprofilemanager.h"

// Qt includes
#include <QRadioButton>
#include <QCheckBox>
#include <QTreeWidget>
#include <QShowEvent>
#include <QSize>
#include <QScrollArea>
#include <QPointer>
#include <QPair>
#include <QList>

// KDE includes
#include <klineedit.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kcombobox.h>
#include <kgenericfactory.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kpassworddialog.h>
#include <kapplication.h>
#include <kconfigdialogmanager.h>
#include <keditlistwidget.h>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY( Smb4KConfigDialogFactory, registerPlugin<Smb4KConfigDialog>(); )
K_EXPORT_PLUGIN( Smb4KConfigDialogFactory( "Smb4KConfigDialog" ) );


Smb4KConfigDialog::Smb4KConfigDialog( QWidget *parent, const char *name, Smb4KSettings *settings )
: KConfigDialog( parent, name, settings )
{
  setupDialog();
}


Smb4KConfigDialog::Smb4KConfigDialog( QWidget *parent, const QList<QVariant> &/*args*/ )
: KConfigDialog( parent, "ConfigDialog", Smb4KSettings::self() )
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
  setAttribute( Qt::WA_DeleteOnClose, true );

  // Add the pages:
  Smb4KUserInterfaceOptionsPage *interface_options = new Smb4KUserInterfaceOptionsPage( this );
  QScrollArea *interface_area = new QScrollArea( this );
  interface_area->setWidget( interface_options );
  interface_area->setWidgetResizable( true );
  interface_area->setFrameStyle( QFrame::NoFrame );

  Smb4KNetworkOptionsPage *network_options = new Smb4KNetworkOptionsPage( this );
  QScrollArea *network_area = new QScrollArea( this );
  network_area->setWidget( network_options );
  network_area->setWidgetResizable( true );
  network_area->setFrameStyle( QFrame::NoFrame );

  Smb4KShareOptionsPage *share_options = new Smb4KShareOptionsPage( this );
  QScrollArea *share_area = new QScrollArea( this );
  share_area->setWidget( share_options );
  share_area->setWidgetResizable( true );
  share_area->setFrameStyle( QFrame::NoFrame );

  Smb4KAuthOptionsPage *auth_options = new Smb4KAuthOptionsPage( this );
  QScrollArea *auth_area = new QScrollArea( this );
  auth_area->setWidget( auth_options );
  auth_area->setWidgetResizable( true );
  auth_area->setFrameStyle( QFrame::NoFrame );

  Smb4KSambaOptionsPage *samba_options = new Smb4KSambaOptionsPage( this );
  QScrollArea *samba_area = new QScrollArea( this );
  samba_area->setWidget( samba_options );
  samba_area->setWidgetResizable( true );
  samba_area->setFrameStyle( QFrame::NoFrame );

  Smb4KRsyncOptionsPage *rsync_options = new Smb4KRsyncOptionsPage( this );
  QScrollArea *rsync_area = new QScrollArea( this );
  rsync_area->setWidget( rsync_options );
  rsync_area->setWidgetResizable( true );
  rsync_area->setFrameStyle( QFrame::NoFrame );
  
  rsync_options->setEnabled( !KStandardDirs::findExe( "rsync" ).isEmpty() );

  Smb4KLaptopSupportOptionsPage *laptop_options = new Smb4KLaptopSupportOptionsPage( this );
  QScrollArea *laptop_area = new QScrollArea( this );
  laptop_area->setWidget( laptop_options );
  laptop_area->setWidgetResizable( true );
  laptop_area->setFrameStyle( QFrame::NoFrame );
  
  Smb4KCustomOptionsPage *custom_options = new Smb4KCustomOptionsPage( this );
  QScrollArea *custom_area = new QScrollArea( this );
  custom_area->setWidget( custom_options );
  custom_area->setWidgetResizable( true );
  custom_area->setFrameStyle( QFrame::NoFrame );
  
  Smb4KProfilesPage *profiles_page = new Smb4KProfilesPage(this);
  QScrollArea *profiles_area = new QScrollArea(this);
  profiles_area->setWidget(profiles_page);
  profiles_area->setWidgetResizable(true);
  profiles_area->setFrameStyle(QFrame::NoFrame);

  // Now add the pages to the configuration dialog
  m_user_interface  = addPage(interface_area, i18n( "User Interface" ), "view-choose" );
  m_network         = addPage(network_area, i18n( "Network" ), "network-workgroup" );
  m_shares          = addPage(share_area, i18n( "Shares" ), "folder-remote" );
  m_authentication  = addPage(auth_area, i18n( "Authentication" ), "dialog-password" );
  m_samba           = addPage(samba_area, i18n( "Samba" ), "preferences-system-network" );
  m_synchronization = addPage(rsync_area, i18n( "Synchronization" ), "folder-sync" );
  m_laptop_support  = addPage(laptop_area, i18n( "Laptop Support" ), "computer-laptop" );
  m_custom_options  = addPage(custom_area, i18n( "Custom Options" ), "preferences-system-network" );
  m_profiles        = addPage(profiles_area, i18n("Profiles"), "format-list-unordered");

  // Stuff that's not managed by KConfig XT is loaded by
  // Smb4KConfigDialog::showEvent()!

  // Connections
  connect(custom_options, SIGNAL(customSettingsModified()),
          this,           SLOT(slotEnableApplyButton()));
  
  connect(custom_options, SIGNAL(reloadCustomSettings()),
          this,           SLOT(slotReloadCustomOptions()) );

  connect( auth_options,  SIGNAL(loadWalletEntries()),
           this,          SLOT(slotLoadAuthenticationInformation()) );
           
  connect( auth_options,  SIGNAL(saveWalletEntries()),
           this,          SLOT(slotSaveAuthenticationInformation()) );
           
  connect( auth_options,  SIGNAL(setDefaultLogin()),
           this,          SLOT(slotSetDefaultLogin()) );
           
  connect( auth_options,  SIGNAL(walletEntriesModified()),
           this,          SLOT(slotEnableApplyButton()) );
  
  connect( this,          SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
           this,          SLOT(slotCheckPage(KPageWidgetItem*,KPageWidgetItem*)));
  
  setInitialSize( QSize( 800, 600 ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "ConfigDialog" );
  restoreDialogSize( group );
}


void Smb4KConfigDialog::loadCustomOptions()
{
  if ( m_custom_options )
  {
    QList<Smb4KCustomOptions *> options = Smb4KCustomOptionsManager::self()->customOptions();
    m_custom_options->widget()->findChild<Smb4KCustomOptionsPage *>()->insertCustomOptions( options );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::saveCustomOptions()
{
  if ( m_custom_options )
  {
    QList<Smb4KCustomOptions *> options = m_custom_options->widget()->findChild<Smb4KCustomOptionsPage *>()->getCustomOptions();
    Smb4KCustomOptionsManager::self()->replaceCustomOptions( options );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::propagateProfilesChanges()
{
  Smb4KProfilesPage *profiles_page = m_profiles->widget()->findChild<Smb4KProfilesPage *>();
  
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

  if ((query_custom_master && query_custom_master->isChecked()) &&
      (custom_master_input && custom_master_input->text().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
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
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
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
  
  if (mount_prefix && mount_prefix->url().path().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
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


bool Smb4KConfigDialog::checkSambaPage()
{
  KLineEdit *file_mask = m_samba->widget()->findChild<KLineEdit *>("kcfg_FileMask");
  
  if (file_mask && file_mask->text().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_samba);
    
    Smb4KSambaOptionsPage *samba_options = m_samba->widget()->findChild<Smb4KSambaOptionsPage *>();
    
    if (samba_options)
    {
      samba_options->setCurrentIndex(Smb4KSambaOptionsPage::MountingTab);
    }
    else
    {
      // Do nothing
    }
    
    file_mask->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  KLineEdit *directory_mask = m_samba->widget()->findChild<KLineEdit *>("kcfg_DirectoryMask");
  
  if (directory_mask && directory_mask->text().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_samba);
    
    Smb4KSambaOptionsPage *samba_options = m_samba->widget()->findChild<Smb4KSambaOptionsPage *>();
    
    if (samba_options)
    {
      samba_options->setCurrentIndex( Smb4KSambaOptionsPage::MountingTab );
    }
    else
    {
      // Do nothing
    }
    
    directory_mask->setFocus();
    return false;
  }
  else
  {
    // Do nothing
  }
  
  return true;
}


bool Smb4KConfigDialog::checkSynchronizationPage()
{
  KUrlRequester *sync_prefix = m_synchronization->widget()->findChild<KUrlRequester *>("kcfg_RsyncPrefix");
  
  if (sync_prefix && sync_prefix->url().path().trimmed().isEmpty())
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::CopyingTab);
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
  KIntNumInput *max_delete_val = m_synchronization->widget()->findChild<KIntNumInput *>("kcfg_MaximumDeleteValue");
  
  if ((max_delete && max_delete->isChecked()) && (max_delete_val && max_delete_val->value() == 0))
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::DelTransTab);
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
  KIntNumInput *min_trans_size_val = m_synchronization->widget()->findChild<KIntNumInput *>("kcfg_MinimalTransferSize");
  
  if ((min_trans_size && min_trans_size->isChecked()) && (min_trans_size_val && min_trans_size_val->value() == 0))
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::DelTransTab);
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
  KIntNumInput *max_trans_size_val = m_synchronization->widget()->findChild<KIntNumInput *>("kcfg_MaximalTransferSize");
  
  if ((max_trans_size && max_trans_size->isChecked()) && (max_trans_size_val && max_trans_size_val->value() == 0))
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::DelTransTab);
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
  
  QCheckBox *use_partial_directory = m_synchronization->widget()->findChild<QCheckBox *>( "kcfg_UsePartialDirectory" );
  KUrlRequester *partial_directory = m_synchronization->widget()->findChild<KUrlRequester *>( "kcfg_PartialDirectory" );
  
  if ((use_partial_directory && use_partial_directory->isChecked()) &&
      (partial_directory && partial_directory->url().path().trimmed().isEmpty()))
  {
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage( m_synchronization );
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::DelTransTab);
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
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::FilteringTab);
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
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::FilteringTab);
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
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::FilteringTab);
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
    KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
    setCurrentPage(m_synchronization);
    
    Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
    
    if (sync_options)
    {
      sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::FilteringTab);
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
      KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
      setCurrentPage(m_synchronization);
      
      Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
      
      if (sync_options)
      {
        sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::AdvancedTab);
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
      KMessageBox::sorry(this, i18n("An incorrect setting has been found. You are now taken there to fix it."));
      setCurrentPage(m_synchronization);
      
      Smb4KRsyncOptionsPage *sync_options = m_synchronization->widget()->findChild<Smb4KRsyncOptionsPage *>();
      
      if (sync_options)
      {
        sync_options->setCurrentIndex(Smb4KRsyncOptionsPage::AdvancedTab);
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
  
  // Check Samba page
  if (!checkSambaPage())
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


void Smb4KConfigDialog::showEvent( QShowEvent *e )
{
  // Spontaneous show events come from outside the application.
  // We do not want to react on them.
  if ( !e->spontaneous() )
  {
    loadCustomOptions();
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KConfigDialog::updateSettings()
{
  saveCustomOptions();
  slotSaveAuthenticationInformation();
  propagateProfilesChanges();
      
  KConfigDialog::updateSettings();
}


void Smb4KConfigDialog::slotButtonClicked(int button)
{
  switch(button)
  {
    case Apply:
    {
      // If some settings are not complete, stop here and give
      // the user the opportunity to fill in the needed string(s).
      if (!checkSettings())
      {
        return;
      }

      break;
    }
    case Ok:
    {
      // If some settings are not complete, stop here and give
      // the user the opportunity to fill in the needed string(s).
      if (!checkSettings())
      {
        return;
      }

      KConfigGroup group(Smb4KSettings::self()->config(), "ConfigDialog");
      saveDialogSize(group, KConfigGroup::Normal);

      break;
    }
    default:
    {
      break;
    }
  }

  KConfigDialog::slotButtonClicked(button);
}


void Smb4KConfigDialog::slotLoadAuthenticationInformation()
{
  Smb4KAuthOptionsPage *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptionsPage *>();
  QList<Smb4KAuthInfo *> entries = Smb4KWalletManager::self()->walletEntries();
  auth_options->insertWalletEntries( entries );
  auth_options->displayWalletEntries();
}


void Smb4KConfigDialog::slotSaveAuthenticationInformation()
{
  Smb4KAuthOptionsPage *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptionsPage *>();
  
  if ( auth_options->walletEntriesDisplayed() )
  {
    QList<Smb4KAuthInfo *> entries = auth_options->getWalletEntries();
    Smb4KWalletManager::self()->writeWalletEntries( entries );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::slotSetDefaultLogin()
{
  Smb4KAuthOptionsPage *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptionsPage *>();
  
  if ( !auth_options->undoRemoval() )
  {
    Smb4KAuthInfo authInfo;
    // We do not need to call useDefaultAuthInfo(), because 
    // Smb4KWalletManager::readDefaultAuthInfo() will do this
    // for us.
    Smb4KWalletManager::self()->readDefaultAuthInfo( &authInfo );
    
    QPointer<KPasswordDialog> dlg = new KPasswordDialog( this, KPasswordDialog::ShowUsernameLine );
    dlg->setPrompt( i18n( "Enter the default login information." ) );
    dlg->setUsername( authInfo.userName() );
    dlg->setPassword( authInfo.password() );
    
    if ( dlg->exec() == KPasswordDialog::Accepted )
    {
      authInfo.setUserName( dlg->username() );
      authInfo.setPassword( dlg->password() );
      
      Smb4KWalletManager::self()->writeDefaultAuthInfo( &authInfo );
      
      if ( auth_options->walletEntriesDisplayed() )
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
      auth_options->findChild<QCheckBox *>( "kcfg_UseDefaultLogin" )->setChecked( false );
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
  Smb4KAuthOptionsPage *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptionsPage *>();

  if ( auth_options->walletEntriesMaybeChanged() )
  {
    QList<Smb4KAuthInfo *> old_wallet_entries = Smb4KWalletManager::self()->walletEntries();
    QList<Smb4KAuthInfo *> new_wallet_entries = auth_options->getWalletEntries();
    
    for ( int i = 0; i < old_wallet_entries.size(); ++i )
    {
      for ( int j = 0; j < new_wallet_entries.size(); ++j )
      {
        if ( QString::compare( old_wallet_entries.at( i )->unc(),
                               new_wallet_entries.at( j )->unc(),
                               Qt::CaseInsensitive ) == 0 &&
             (QString::compare( old_wallet_entries.at( i )->workgroupName(),
                                new_wallet_entries.at( j )->workgroupName(),
                                Qt::CaseInsensitive ) != 0 ||
              QString::compare( old_wallet_entries.at( i )->userName(),
                                new_wallet_entries.at( j )->userName(),
                                Qt::CaseInsensitive ) != 0 ||
              QString::compare( old_wallet_entries.at( i )->password(),
                                new_wallet_entries.at( j )->password(),
                                Qt::CaseInsensitive ) != 0) )
        {
          enable = true;
          break;
        }
        else
        {
          continue;
        }
      }
      
      if ( enable )
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
  Smb4KCustomOptionsPage *custom_options = m_custom_options->widget()->findChild<Smb4KCustomOptionsPage *>();
  
  if ( !enable && custom_options && custom_options->customSettingsMaybeChanged() )
  {
    QList<Smb4KCustomOptions *> new_list = custom_options->getCustomOptions();
    QList<Smb4KCustomOptions *> old_list = Smb4KCustomOptionsManager::self()->customOptions();
    
    if ( new_list.size() == old_list.size() )
    {
      for ( int i = 0; i < new_list.size(); ++i )
      {
        for ( int j = 0; j < old_list.size(); ++j )
        {
          if ( !new_list[i]->equals( old_list.at( j ) ) )
          {
            enable = true;
            break;
          }
          else
          {
            continue;
          }
        }
        
        if ( enable )
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
  
  enableButtonApply( enable );
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
    (void)checkSambaPage();
  }
  else if (before == m_synchronization)
  {
    (void)checkSynchronizationPage();
  }
  else if (before == m_laptop_support)
  {
    // Do nothing
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
