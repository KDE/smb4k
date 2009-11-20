/***************************************************************************
    smb4kconfigdialog  -  The configuration dialog of Smb4K
                             -------------------
    begin                : Sa Apr 14 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QRadioButton>
#include <QCheckBox>
#include <QTreeWidget>
#include <QShowEvent>
#include <QSize>
#include <QScrollArea>

// KDE includes
#include <klineedit.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kcombobox.h>
#include <kgenericfactory.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>

// system specific includes
#include <unistd.h>
#include <sys/types.h>

// application specific includes
#include <smb4kconfigdialog.h>
#include <smb4kuserinterfaceoptions.h>
#include <smb4knetworkoptions.h>
#include <smb4kshareoptions.h>
#include <smb4kauthoptions.h>
#include <smb4ksuperuseroptions.h>
#include <smb4ksambaoptions.h>
#include <smb4krsyncoptions.h>
#include <smb4klaptopsupportoptions.h>
#include <core/smb4ksettings.h>
#include <core/smb4kglobal.h>
#include <core/smb4ksambaoptionsinfo.h>
#include <core/smb4ksambaoptionshandler.h>
#include <core/smb4kcore.h>
#include <core/smb4kauthinfo.h>
#include <core/smb4kwalletmanager.h>

using namespace Smb4KGlobal;

// Variables we need to determine if super user entries
// have to be written to /etc/sudoers
#ifdef __linux__
bool force_unmount = false;
#endif
bool always_use_su = false;

K_EXPORT_COMPONENT_FACTORY( libsmb4kconfigdialog, KGenericFactory<Smb4KConfigDialog> )


Smb4KConfigDialog::Smb4KConfigDialog( QWidget *parent, const char *name, Smb4KSettings *settings )
: KConfigDialog( parent, name, settings )
{
  setupDialog();
}


Smb4KConfigDialog::Smb4KConfigDialog( QWidget *parent, const QStringList &/*args*/ )
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
  Smb4KUserInterfaceOptions *interface_options = new Smb4KUserInterfaceOptions( this );
  QScrollArea *interface_area = new QScrollArea( this );
  interface_area->setWidget( interface_options );
  interface_area->setWidgetResizable( true );
  interface_area->setFrameStyle( QFrame::NoFrame );

  Smb4KNetworkOptions *network_options = new Smb4KNetworkOptions( this );
  QScrollArea *network_area = new QScrollArea( this );
  network_area->setWidget( network_options );
  network_area->setWidgetResizable( true );
  network_area->setFrameStyle( QFrame::NoFrame );

  Smb4KShareOptions *share_options = new Smb4KShareOptions( this );
  QScrollArea *share_area = new QScrollArea( this );
  share_area->setWidget( share_options );
  share_area->setWidgetResizable( true );
  share_area->setFrameStyle( QFrame::NoFrame );

  Smb4KAuthOptions *auth_options = new Smb4KAuthOptions( this );
  QScrollArea *auth_area = new QScrollArea( this );
  auth_area->setWidget( auth_options );
  auth_area->setWidgetResizable( true );
  auth_area->setFrameStyle( QFrame::NoFrame );

  Smb4KSambaOptions *samba_options = new Smb4KSambaOptions( this );
  QScrollArea *samba_area = new QScrollArea( this );
  samba_area->setWidget( samba_options );
  samba_area->setWidgetResizable( true );
  samba_area->setFrameStyle( QFrame::NoFrame );

  Smb4KRsyncOptions *rsync_options = new Smb4KRsyncOptions( this );
  QScrollArea *rsync_area = new QScrollArea( this );
  rsync_area->setWidget( rsync_options );
  rsync_area->setWidgetResizable( true );
  rsync_area->setFrameStyle( QFrame::NoFrame );

  Smb4KSuperUserOptions *super_user_options = new Smb4KSuperUserOptions( this );
  QScrollArea *super_user_area = new QScrollArea( this );
  super_user_area->setWidget( super_user_options );
  super_user_area->setWidgetResizable( true );
  super_user_area->setFrameStyle( QFrame::NoFrame );

  Smb4KLaptopSupportOptions *laptop_options = new Smb4KLaptopSupportOptions( this );
  QScrollArea *laptop_area = new QScrollArea( this );
  laptop_area->setWidget( laptop_options );
  laptop_area->setWidgetResizable( true );
  laptop_area->setFrameStyle( QFrame::NoFrame );

  // Disable widgets if the respective programs are not installed.
  if ( KStandardDirs::findExe( "rsync" ).isEmpty() )
  {
    rsync_options->setEnabled( false );
  }
  else
  {
    // Do nothing
  }

  if ( KStandardDirs::findExe( "sudo" ).isEmpty() )
  {
    super_user_options->setEnabled( false );
  }
  else
  {
    // Do nothing
  }

  // There are a few settings we need to the initial values of.
  // Initialize them here:
#ifdef __linux__
  force_unmount = Smb4KSettings::useForceUnmount();
#endif
  always_use_su = Smb4KSettings::alwaysUseSuperUser();

  // Now add the pages to the configuration dialog
  addPage( interface_area, i18n( "User Interface" ), "view-choose" );
  addPage( network_area, i18n( "Network" ), "network-workgroup" );
  addPage( share_area, i18n( "Shares" ), "folder-remote" );
  addPage( auth_area, i18n( "Authentication" ), "dialog-password" );
  addPage( samba_area, i18n( "Samba" ), "preferences-system-network" );
  addPage( rsync_area, i18n( "Synchronization" ), "go-bottom" );
  addPage( super_user_area, i18n( "Super User" ), "user-identity" );
  addPage( laptop_area, i18n( "Laptop Support" ), "computer-laptop" );

  // Stuff that's not managed by KConfig XT is loaded by
  // Smb4KConfigDialog::showEvent()!

  // Connections
  connect( samba_options,           SIGNAL( customSettingsModified() ),
           this,                    SLOT( slotCustomSambaSettingsModified() ) );

  connect( super_user_options,      SIGNAL( removeEntries() ),
           this,                    SLOT( slotRemoveSuperUserEntries() ) );

  setInitialSize( QSize( 800, 600 ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "ConfigDialog" );
  restoreDialogSize( group );
}


void Smb4KConfigDialog::loadCustomSambaOptions()
{
  QList<Smb4KSambaOptionsInfo *> list = Smb4KSambaOptionsHandler::self()->customOptionsList();

  Smb4KSambaOptions *samba_options = findChild<Smb4KSambaOptions *>();

  if ( samba_options )
  {
    samba_options->insertCustomOptions( list );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::saveCustomSambaOptions()
{
  QList<Smb4KSambaOptionsInfo *> list;

  Smb4KSambaOptions *samba_options = findChild<Smb4KSambaOptions *>();

  if ( samba_options )
  {
    list = samba_options->getCustomOptions();
  }
  else
  {
    return;
  }

  Smb4KSambaOptionsHandler::self()->updateCustomOptions( list );
}


void Smb4KConfigDialog::loadAuthenticationData()
{
  // Read the default authentication information.
  Smb4KAuthInfo authInfo;
  authInfo.setDefaultAuthInfo();

  Smb4KWalletManager::self()->readAuthInfo( &authInfo );

  KLineEdit *default_user = findChild<KLineEdit *>( "DefaultUserName" );

  if ( default_user )
  {
    default_user->setText( authInfo.login() );
  }
  else
  {
    // Do nothing
  }

  KLineEdit *default_pass = findChild<KLineEdit *>( "DefaultPassword" );

  if ( default_pass )
  {
    default_pass->setText( authInfo.password() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::saveAuthenticationData()
{
  // Write the default authentication information.
  Smb4KAuthInfo authInfo;
  authInfo.setDefaultAuthInfo();

  KLineEdit *default_user = findChild<KLineEdit *>( "DefaultUserName" );

  if ( default_user )
  {
    authInfo.setLogin( default_user->text() );
  }
  else
  {
    // Do nothing
  }

  KLineEdit *default_pass = findChild<KLineEdit *>( "DefaultPassword" );

  if ( default_pass )
  {
    authInfo.setPassword( default_pass->text() );
  }
  else
  {
    // Do nothing
  }

  Smb4KWalletManager::self()->writeAuthInfo( &authInfo );
}


void Smb4KConfigDialog::writeSuperUserEntries()
{
  // Get the checkboxes in the "Super User" page.
#ifdef __linux__
  QCheckBox *force_button = findChild<QCheckBox *>( "kcfg_UseForceUnmount" );
#endif
  QCheckBox *always_button = findChild<QCheckBox *>( "kcfg_AlwaysUseSuperUser" );

  Q_ASSERT( force_button );
  Q_ASSERT( always_button );

  // Check if we have to write anything and initiate the writing
  // if necessary.
#ifdef __linux__
  if ( (!force_unmount && !always_use_su) &&
       ((force_button->isChecked() && !force_unmount) ||
       (always_button->isChecked() && !always_use_su)) )
#else
  if ( always_button->isChecked() && !always_use_su )
#endif
  {
    setEnabled( false );

    if ( !Smb4KCore::sudoWriter()->addUser() )
    {
      // The write process failed. Reset the values.
#ifdef __linux__
      force_button->setChecked( force_unmount );
      Smb4KSettings::setUseForceUnmount( force_unmount );
#endif
      always_button->setChecked( always_use_su );
      Smb4KSettings::setAlwaysUseSuperUser( always_use_su );
    }
    else
    {
      // Do nothing
    }

    setEnabled( true );
  }
  else
  {
    // Do nothing
  }

  // Set the variables.
#ifdef __linux__
  force_unmount = force_button->isChecked();
#endif
  always_use_su = always_button->isChecked();
}


void Smb4KConfigDialog::removeSuperUserEntries()
{
  // Get the checkboxes in the "Super User" page.
#ifdef __linux__
  QCheckBox *force_button = findChild<QCheckBox *>( "kcfg_UseForceUnmount" );
#endif
  QCheckBox *always_button = findChild<QCheckBox *>( "kcfg_AlwaysUseSuperUser" );

  Q_ASSERT( force_button );
  Q_ASSERT( always_button );

  // Uncheck both buttons.
#ifdef __linux__
  force_button->setChecked( false );
  Smb4KSettings::setUseForceUnmount( false );
#endif
  always_button->setChecked( false );
  Smb4KSettings::setAlwaysUseSuperUser( false );

  setEnabled( false );

  // Remove the entries.
  if ( !Smb4KCore::sudoWriter()->removeUser() )
  {
    // Actually, we do not need to care about failed removals.
  }
  else
  {
    // Do nothing
  }

  setEnabled( true );

  // Set the variables.
#ifdef __linux__
  force_unmount = force_button->isChecked();
#endif
  always_use_su = always_button->isChecked();
}


bool Smb4KConfigDialog::checkSettings()
{
  bool ok = true;
  QString issues;
  int index = 0;

  // If the user chose "Query custom master browser" in the
  // "Network" tab, there must be a master browser present:
  QRadioButton *query_custom_master = findChild<QRadioButton *>( "kcfg_QueryCustomMaster" );
  KLineEdit *custom_master_input    = findChild<KLineEdit *>( "kcfg_CustomMasterBrowser" );

  if ( query_custom_master && custom_master_input &&
       query_custom_master->isChecked() &&
       custom_master_input->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Network] The custom master browser has not been entered." )+"</li>" );
  }

  // If the user chose "Scan broadcast areas" in the
  // "Network" tab, there must broadcast areas present:
  QRadioButton *scan_bcast_areas    = findChild<QRadioButton *>( "kcfg_ScanBroadcastAreas" );
  KLineEdit *bcast_areas_input      = findChild<KLineEdit *>( "kcfg_BroadcastAreas" );

  if ( scan_bcast_areas && bcast_areas_input &&
       scan_bcast_areas->isChecked() &&
       bcast_areas_input->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Network] The broadcast areas have not been entered." )+"</li>" );
  }

  // The mount prefix must not be empty:
  KUrlRequester *mount_prefix       = findChild<KUrlRequester *>( "kcfg_MountPrefix" );

  if ( mount_prefix && mount_prefix->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Shares] The mount prefix is empty." )+"</li>" );
  }

  // If the user wants to use a default login, the user
  // name must not be empty.
  QCheckBox *use_default_login      = findChild<QCheckBox *>( "kcfg_UseDefaultLogin" );
  KLineEdit *default_user_name      = findChild<KLineEdit *>( "DefaultUserName" );

  if ( use_default_login && default_user_name &&
       use_default_login->isChecked() &&
       default_user_name->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Authentication] The default username has not been entered." )+"</li>" );
  }

  // The file mask must not be empty.
  KLineEdit *file_mask              = findChild<KLineEdit *>( "kcfg_FileMask" );

  if ( file_mask && file_mask->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Samba] The file mask is empty." )+"</li>" );
  }

  // The directory mask must not be empty.
  KLineEdit *directory_mask         = findChild<KLineEdit *>( "kcfg_DirectoryMask" );

  if ( directory_mask && directory_mask->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Samba] The directory mask is empty." )+"</li>" );
  }

  // The rsync prefix must not be empty.
  KUrlRequester *rsync_prefix       = findChild<KUrlRequester *>( "kcfg_RsyncPrefix" );

  if ( rsync_prefix && rsync_prefix->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The rsync prefix is empty." )+"</li>" );
  }

  // The path where to store partial files must not be empty.
  QCheckBox *use_partical_directory = findChild<QCheckBox *>( "kcfg_UsePartialDirectory" );
  KUrlRequester *partial_directory  = findChild<KUrlRequester *>( "kcfg_PartialDirectory" );

  if ( use_partical_directory && use_partical_directory->isChecked() &&
       partial_directory && partial_directory->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The directory where partially transferred files should be stored is empty." )+"</li>" );
  }

  // The the exclude patterns must not be empty.
  QCheckBox *use_exclude_pattern    = findChild<QCheckBox *>( "kcfg_UseExcludePattern" );
  KLineEdit *exclude_pattern        = findChild<KLineEdit *>( "kcfg_ExcludePattern" );

  if ( use_exclude_pattern && use_exclude_pattern->isChecked() &&
       exclude_pattern && exclude_pattern->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The exclude patterns have not been entered." )+"</li>" );
  }

  // The the path of the exclude file must not be empty.
  QCheckBox *use_exclude_file       = findChild<QCheckBox *>( "kcfg_UseExcludeFrom" );
  KUrlRequester *exclude_file       = findChild<KUrlRequester *>( "kcfg_ExcludeFrom" );

  if ( use_exclude_file && use_exclude_file->isChecked() &&
       exclude_file && exclude_file->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The path of the exclude file is empty." )+"</li>" );
  }

  // The the include patterns must not be empty.
  QCheckBox *use_include_pattern    = findChild<QCheckBox *>( "kcfg_UseIncludePattern" );
  KLineEdit *include_pattern        = findChild<KLineEdit *>( "kcfg_IncludePattern" );

  if ( use_include_pattern && use_include_pattern->isChecked() &&
       include_pattern && include_pattern->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The include patterns have not been entered." )+"</li>" );
  }

  // The the path of the exclude file must not be empty.
  QCheckBox *use_include_file       = findChild<QCheckBox *>( "kcfg_UseIncludeFrom" );
  KUrlRequester *include_file       = findChild<KUrlRequester *>( "kcfg_IncludeFrom" );

  if ( use_include_file && use_include_file->isChecked() &&
       include_file && include_file->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The path of the include file is empty." )+"</li>" );
  }

  // If you make backups, check that the suffix and that the
  // backup directory is not empty.
  QCheckBox *make_backups           = findChild<QCheckBox *>( "kcfg_MakeBackups" );

  if ( make_backups && make_backups->isChecked() )
  {
    // The backup suffix must not be empty.
    QCheckBox *use_backup_suffix    = findChild<QCheckBox *>( "kcfg_UseBackupSuffix" );
    KLineEdit *backup_suffix        = findChild<KLineEdit *>( "kcfg_BackupSuffix" );

    if ( use_backup_suffix && use_backup_suffix->isChecked() &&
         backup_suffix && backup_suffix->text().trimmed().isEmpty() )
    {
      ok = false;
      index++;

      issues.append( "<li>"+i18n( "[Synchronization] The backup suffix has not been defined." )+"</li>" );
    }

    // The the path for backups must not be empty.
    QCheckBox *use_backup_dir       = findChild<QCheckBox *>( "kcfg_UseBackupDirectory" );
    KUrlRequester *backup_dir       = findChild<KUrlRequester *>( "kcfg_BackupDirectory" );

    if ( use_backup_dir && use_backup_dir->isChecked() &&
         backup_dir && backup_dir->url().path().trimmed().isEmpty() )
    {
      ok = false;
      index++;

      issues.append( "<li>"+i18n( "[Synchronization] The backup directory is empty." )+"</li>" );
    }
  }

  if ( !ok )
  {
    KMessageBox::error( this, i18np( "<qt>The configuration could not be written, because one setting is incomplete:<ul>%2</ul>Please correct this issue.</qt>",
                                     "<qt>The configuration could not be written, because %1 settings are incomplete:<ul>%2</ul>Please correct these issues.</qt>",
                                     index, issues ) );
  }

  return ok;
}


void Smb4KConfigDialog::showEvent( QShowEvent *e )
{
  // Spontaneous show events come from outside the application.
  // We do not want to react on them.
  if ( !e->spontaneous() )
  {
    loadCustomSambaOptions();
    loadAuthenticationData();
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KConfigDialog::slotButtonClicked( int button )
{
  switch( button )
  {
    case Apply:
    {
      // If some settings are not complete, stop here and give
      // the user the opportunity to fill in the needed string(s).
      if ( !checkSettings() )
      {
        return;
      }

      saveCustomSambaOptions();
      saveAuthenticationData();
      writeSuperUserEntries();

      break;
    }
    case Ok:
    {
      // If some settings are not complete, stop here and give
      // the user the opportunity to fill in the needed string(s).
      if ( !checkSettings() )
      {
        return;
      }

      saveCustomSambaOptions();
      saveAuthenticationData();
      writeSuperUserEntries();

      KConfigGroup group( Smb4KSettings::self()->config(), "ConfigDialog" );
      saveDialogSize( group, KConfigGroup::Normal );

      break;
    }
    default:
    {
      break;
    }
  }

  KConfigDialog::slotButtonClicked( button );
}


void Smb4KConfigDialog::slotCustomSambaSettingsModified()
{
  // Get the list view and all other input widgets:
  QTreeWidget *view = findChild<QTreeWidget *>( "CustomOptionsList" );
  bool enable_apply = false;

  if ( view )
  {
    // Get the old list of custom options from the options handler.
    QList<Smb4KSambaOptionsInfo *> old_list = Smb4KSambaOptionsHandler::self()->customOptionsList();

    // Get the new list of custom options from the Samba options tab.
    QList<Smb4KSambaOptionsInfo *> new_list = static_cast<Smb4KSambaOptions *>( findChild<Smb4KSambaOptions *>() )->getCustomOptions();

    if ( old_list.size() == new_list.size() )
    {
      for ( int i = 0; i < old_list.size(); ++i )
      {
        for ( int j = 0; j < new_list.size(); ++j )
        {
          if ( QString::compare( old_list.at( i )->unc(), new_list.at( j )->unc(),
               Qt::CaseInsensitive ) == 0 )
          {
            if ( old_list.at( i )->protocol() != new_list.at( j )->protocol() )
            {
              enable_apply = true;

              break;
            }
            else
            {
              // Do nothing
            }

#ifndef __FreeBSD__
            if ( old_list.at( i )->writeAccess() != new_list.at( j )->writeAccess() )
            {
              enable_apply = true;

              break;
            }
            else
            {
              // Do nothing
            }
#endif

            if ( old_list.at( i )->useKerberos() != new_list.at( j )->useKerberos() )
            {
              enable_apply = true;

              break;
            }
            else
            {
              // Do nothing
            }

            if ( old_list.at( i )->uid() != new_list.at( j )->uid() )
            {
              enable_apply = true;

              break;
            }
            else
            {
              // Do nothing
            }

            if ( old_list.at( i )->gid() != new_list.at( j )->gid() )
            {
              enable_apply = true;

              break;
            }
            else
            {
              // Do nothing
            }

            if ( old_list.at( i )->port() != new_list.at( j )->port() )
            {
              enable_apply = true;

              break;
            }
            else
            {
              // Do nothing
            }

            break;
          }
          else
          {
            continue;
          }
        }

        if ( enable_apply )
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
      enable_apply = true;
    }
  }
  else
  {
    // Do nothing
  }

  enableButtonApply( enable_apply );
}


void Smb4KConfigDialog::slotRemoveSuperUserEntries()
{
  removeSuperUserEntries();
}


// void Smb4KConfigDialog::slotReceivedSudoWriterFailed( Smb4KSudoWriterInterface::Operation operation )
// {
//   switch ( operation )
//   {
//     case Smb4KSudoWriterInterface::AddUser:
//     {
// #ifdef __linux__
//       QCheckBox *force    = findChild<QCheckBox *>( "kcfg_UseForceUnmount" );
// #endif
//       QCheckBox *full_use = findChild<QCheckBox *>( "kcfg_AlwaysUseSuperUser" );
//
// #ifdef __linux__
//       if ( force && full_use )
//       {
//         force->setChecked( force_unmount );
//         Smb4KSettings::setUseForceUnmount( force_unmount );
// #else
//       if ( full_use )
//       {
// #endif
//         full_use->setChecked( always_use_su );
//         Smb4KSettings::setAlwaysUseSuperUser( always_use_su );
//       }
//       else
//       {
//         // Do nothing
//       }
//
//       break;
//     }
//     default:
//     {
//       break;
//     }
//   }
// }
//
//
// void Smb4KConfigDialog::slotReceivedSudoWriterFinished( Smb4KSudoWriterInterface::Operation operation )
// {
//   // Set the variables.
// #ifdef __linux__
//   force_unmount = Smb4KSettings::useForceUnmount();
// #endif
//   always_use_su = Smb4KSettings::alwaysUseSuperUser();
//
//   // Enable the widget again.
//   setEnabled( true );
//
// //   switch ( operation )
// //   {
// //     case Smb4KSudoWriterInterface::RemoveUser:
// //     {
// //       slotButtonClicked( KConfigDialog::Apply );
// //       break;
// //     }
// //     default:
// //     {
// //       break;
// //     }
// //   }
//
//   if ( close_dialog )
//   {
//     KConfigDialog::slotButtonClicked( KConfigDialog::Ok );
//   }
//   else
//   {
//     KConfigDialog::slotButtonClicked( KConfigDialog::Apply );
//   }
// }

#include "smb4kconfigdialog.moc"
