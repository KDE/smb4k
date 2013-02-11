/***************************************************************************
    smb4kcustomoptionspage  -  The configuration page for the custom 
    options
                             -------------------
    begin                : Sa Jan 19 2013
    copyright            : (C) 2013 by Alexander Reinholdt
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

// application specific includes
#include "smb4kcustomoptionspage.h"
#include "core/smb4ksettings.h"
#include "core/smb4kcustomoptions.h"

// Qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <klocale.h>
#include <kmenu.h>


Smb4KCustomOptionsPage::Smb4KCustomOptionsPage( QWidget *parent ) : QWidget( parent )
{
  m_collection = new KActionCollection( this );
  m_maybe_changed = false;
  m_removed = false;
  m_current_options = NULL;
  
  QHBoxLayout *custom_layout = new QHBoxLayout( this );
  custom_layout->setSpacing( 5 );
  custom_layout->setMargin( 0 );
  
  m_custom_options           = new KListWidget( this );
  m_custom_options->setObjectName( "CustomOptionsList" );
  m_custom_options->viewport()->installEventFilter( this );
  m_custom_options->setSelectionMode( KListWidget::SingleSelection );
  m_custom_options->setContextMenuPolicy( Qt::CustomContextMenu );

  m_menu = new KActionMenu( m_custom_options );

  KAction *edit_action      = new KAction( KIcon( "edit-rename" ), i18n( "Edit" ),
                              m_collection );
  edit_action->setEnabled( false );

  KAction *remove_action    = new KAction( KIcon( "edit-delete" ), i18n( "Remove" ),
                               m_collection );
  remove_action->setEnabled( false );
  
  KAction *clear_action     = new KAction( KIcon( "edit-clear-list" ), i18n( "Clear List" ),
                              m_collection );
  clear_action->setEnabled( false );
  
  KAction *undo_action      = new KAction( KIcon( "edit-undo" ), i18n( "Undo" ),
                              m_collection );
  undo_action->setEnabled( false );
  
  m_collection->addAction( "edit_action", edit_action );
  m_collection->addAction( "remove_action", remove_action );
  m_collection->addAction( "clear_action", clear_action );
  m_collection->addAction( "undo_action", undo_action );

  m_menu->addAction( edit_action );
  m_menu->addAction( remove_action );
  m_menu->addAction( clear_action );
  m_menu->addAction( undo_action );
  
  QWidget *editors = new QWidget( this );
  
  QVBoxLayout *editors_layout = new QVBoxLayout( editors );
  editors_layout->setSpacing( 5 );
  editors_layout->setMargin( 0 );
  
  m_general_editors = new QGroupBox( i18n( "General" ), editors );
  
  QGridLayout *general_editor_layout = new QGridLayout( m_general_editors );
  general_editor_layout->setSpacing( 5 );
  general_editor_layout->setContentsMargins( general_editor_layout->margin(),
                                             general_editor_layout->margin() + 10,
                                             general_editor_layout->margin(),
                                             general_editor_layout->margin() );

  QLabel *unc_label = new QLabel( i18n( "UNC Address:" ), m_general_editors );
  
  m_unc_address = new KLineEdit( m_general_editors );
  m_unc_address->setReadOnly( true );

  unc_label->setBuddy( m_unc_address );
  
  QLabel *ip_label = new QLabel( i18n( "IP Address:" ), m_general_editors );
  
  m_ip_address = new KLineEdit( m_general_editors );
  m_ip_address->setClearButtonShown( true );
  
  ip_label->setBuddy( m_ip_address );
  
  general_editor_layout->addWidget( unc_label, 0, 0, 0 );
  general_editor_layout->addWidget( m_unc_address, 0, 1, 0 );
  general_editor_layout->addWidget( ip_label, 1, 0, 0 );
  general_editor_layout->addWidget( m_ip_address, 1, 1, 0 );
  
  m_samba_editors = new QGroupBox( i18n( "Samba" ), editors );
  
  QGridLayout *samba_editor_layout = new QGridLayout( m_samba_editors );
  samba_editor_layout->setSpacing( 5 );
  samba_editor_layout->setContentsMargins( samba_editor_layout->margin(),
                                           samba_editor_layout->margin() + 10,
                                           samba_editor_layout->margin(),
                                           samba_editor_layout->margin() );
  
  QLabel *smb_port_label = new QLabel( "SMB Port:", m_samba_editors );
  
  m_smb_port = new KIntNumInput( m_samba_editors );
  m_smb_port->setRange( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                        Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );
  m_smb_port->setSliderEnabled( true );

  smb_port_label->setBuddy( m_smb_port );

#ifndef Q_OS_FREEBSD
  QLabel *fs_port_label = new QLabel( i18n( "Filesystem Port:" ), m_samba_editors );
  
  m_fs_port = new KIntNumInput( m_samba_editors );
  m_fs_port->setRange( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );
  m_fs_port->setSliderEnabled( true );

  fs_port_label->setBuddy( m_fs_port );
  
  QLabel *rw_label = new QLabel( i18n( "Write Access:" ), m_samba_editors );
  
  m_write_access = new KComboBox( m_samba_editors );
  m_write_access->insertItem( 0, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadWrite ).label, 
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadWrite ) );
  m_write_access->insertItem( 1, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadOnly ).label, 
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadOnly ) );

  rw_label->setBuddy( m_write_access );
#endif
  
  QLabel *protocol_label = new QLabel( i18n( "Protocol Hint:" ), m_samba_editors );
  
  m_protocol_hint        = new KComboBox( m_samba_editors );
  m_protocol_hint->insertItem( 0, Smb4KSettings::self()->protocolHintItem()->choices()
                                  .value( Smb4KSettings::EnumProtocolHint::Automatic ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Automatic ) );
  m_protocol_hint->insertItem( 1, Smb4KSettings::self()->protocolHintItem()->choices()
                                  .value( Smb4KSettings::EnumProtocolHint::RPC ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::RPC ) );
  m_protocol_hint->insertItem( 2, Smb4KSettings::self()->protocolHintItem()->choices()
                                  .value( Smb4KSettings::EnumProtocolHint::RAP ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::RAP ) );
  m_protocol_hint->insertItem( 3, Smb4KSettings::self()->protocolHintItem()->choices()
                                  .value( Smb4KSettings::EnumProtocolHint::ADS ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::ADS ) );

  protocol_label->setBuddy( m_protocol_hint );
  
  QLabel *uid_label = new QLabel( i18n( "User ID:" ), m_samba_editors );
  m_user_id         = new KComboBox( m_samba_editors );
  
  QList<KUser> all_users = KUser::allUsers();
  
  for ( int i = 0; i < all_users.size(); ++i )
  {
    KUser user = all_users.at( i );
    m_user_id->insertItem( i, QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() ), 
                           QVariant::fromValue<K_UID>( user.uid() ) );
  }

  uid_label->setBuddy( m_user_id );
  
  QLabel *gid_label = new QLabel( i18n( "Group ID:" ), m_samba_editors );
  m_group_id        = new KComboBox( m_samba_editors );
  
  QList<KUserGroup> all_groups = KUserGroup::allGroups();
  
  for ( int i = 0; i < all_groups.size(); ++i )
  {
    KUserGroup group = all_groups.at( i );
    m_group_id->insertItem( i, QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() ), 
                           QVariant::fromValue<K_GID>( group.gid() ) );
  }

  gid_label->setBuddy( m_group_id );
  
  m_kerberos = new QCheckBox( Smb4KSettings::self()->useKerberosItem()->label(), m_samba_editors );
  
#ifndef Q_OS_FREEBSD
  samba_editor_layout->addWidget( smb_port_label, 0, 0, 0 );
  samba_editor_layout->addWidget( m_smb_port, 0, 1, 0 );
  samba_editor_layout->addWidget( fs_port_label, 1, 0, 0 );
  samba_editor_layout->addWidget( m_fs_port, 1, 1, 0 );
  samba_editor_layout->addWidget( rw_label, 2, 0, 0 );
  samba_editor_layout->addWidget( m_write_access, 2, 1, 0 );
  samba_editor_layout->addWidget( protocol_label, 3, 0, 0 );
  samba_editor_layout->addWidget( m_protocol_hint, 3, 1, 0 );
  samba_editor_layout->addWidget( uid_label, 4, 0, 0 );
  samba_editor_layout->addWidget( m_user_id, 4, 1, 0 );
  samba_editor_layout->addWidget( gid_label, 5, 0, 0 );
  samba_editor_layout->addWidget( m_group_id, 5, 1, 0 );
  samba_editor_layout->addWidget( m_kerberos, 6, 0, 1, 2, 0 );
//   samba_editor_layout->setRowStretch( 7, 100 );
#else
  samba_editor_layout->addWidget( smb_port_label, 0, 0, 0 );
  samba_editor_layout->addWidget( m_smb_port, 0, 1, 0 );
  samba_editor_layout->addWidget( protocol_label, 1, 0, 0 );
  samba_editor_layout->addWidget( m_protocol_hint, 1, 1, 0 );
  samba_editor_layout->addWidget( uid_label, 2, 0, 0 );
  samba_editor_layout->addWidget( m_user_id, 2, 1, 0 );
  samba_editor_layout->addWidget( gid_label, 3, 0, 0 );
  samba_editor_layout->addWidget( m_group_id, 3, 1, 0 );
  samba_editor_layout->addWidget( m_kerberos, 4, 0, 1, 2, 0 );
//   samba_editor_layout->setRowStretch( 5, 100 );
#endif
  
  m_mac_editors = new QGroupBox( i18n( "Wake-On-LAN" ), editors );
  
  QGridLayout *mac_editor_layout = new QGridLayout( m_mac_editors );
  mac_editor_layout->setSpacing( 5 );
  mac_editor_layout->setContentsMargins( mac_editor_layout->margin(),
                                         mac_editor_layout->margin() + 10,
                                         mac_editor_layout->margin(),
                                         mac_editor_layout->margin() );
  
  QLabel *mac_label = new QLabel( i18n( "MAC address:" ), m_mac_editors );
  
  m_mac_address = new KLineEdit( m_mac_editors );
  m_mac_address->setClearButtonShown( true );
  
  mac_label->setBuddy( m_mac_address );
  
  // If you change the texts here please also alter them in the custom 
  // options dialog.
  m_send_before_scan = new QCheckBox( i18n( "Send magic package before scanning the network neighborhood" ), m_mac_editors );
  m_send_before_scan->setEnabled( false );
  m_send_before_mount = new QCheckBox( i18n( "Send magic package before mounting a share" ), m_mac_editors );
  m_send_before_mount->setEnabled( false );
  
  mac_editor_layout->addWidget( mac_label, 0, 0, 0 );
  mac_editor_layout->addWidget( m_mac_address, 0, 1, 0 );
  mac_editor_layout->addWidget( m_send_before_scan, 1, 0, 1, 2, 0 );
  mac_editor_layout->addWidget( m_send_before_mount, 2, 0, 1, 2, 0 );

  editors_layout->addWidget( m_general_editors );
  editors_layout->addWidget( m_samba_editors );
  editors_layout->addWidget( m_mac_editors );
  editors_layout->addStretch( 100 );
                                 
  custom_layout->addWidget( m_custom_options );
  custom_layout->addWidget( editors );

  m_general_editors->setEnabled( false );
  m_samba_editors->setEnabled( false );
  m_mac_editors->setEnabled( false );

  clearEditors();
  
  //
  // Connections
  //
  connect( m_custom_options, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           this,             SLOT(slotEditCustomItem(QListWidgetItem*)) );

  connect( m_custom_options, SIGNAL(itemSelectionChanged()),
           this,             SLOT(slotItemSelectionChanged()) );

  connect( m_custom_options, SIGNAL(customContextMenuRequested(QPoint)),
           this,             SLOT(slotCustomContextMenuRequested(QPoint)) );
  
  connect( edit_action,      SIGNAL(triggered(bool)),
           this,             SLOT(slotEditActionTriggered(bool)) );
           
  connect( remove_action,    SIGNAL(triggered(bool)),
           this,             SLOT(slotRemoveActionTriggered(bool)) );
           
  connect( clear_action,     SIGNAL(triggered(bool)),
           this,             SLOT(slotClearActionTriggered(bool)) );
  
  connect( undo_action,      SIGNAL(triggered(bool)),
           this,             SLOT(slotUndoActionTriggered(bool)) );
  
  connect( m_smb_port,       SIGNAL(valueChanged(int)),
           this,             SLOT(slotEntryChanged()) );
  
#ifndef Q_OS_FREEBSD
  connect( m_fs_port,        SIGNAL(valueChanged(int)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_write_access,   SIGNAL(currentIndexChanged(int)),
           this,             SLOT(slotEntryChanged()) );
#endif
  
  connect( m_protocol_hint,  SIGNAL(currentIndexChanged(int)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_user_id,        SIGNAL(currentIndexChanged(int)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_group_id,       SIGNAL(currentIndexChanged(int)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_kerberos,       SIGNAL(toggled(bool)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_mac_address,    SIGNAL(textChanged(QString)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_send_before_scan,   SIGNAL(toggled(bool)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_send_before_mount,  SIGNAL(toggled(bool)),
           this,             SLOT(slotEntryChanged()) );
  
  connect( m_mac_address,    SIGNAL(textChanged(QString)),
           this,             SLOT(slotEnableWOLFeatures(QString)) );
}


Smb4KCustomOptionsPage::~Smb4KCustomOptionsPage()
{
  while ( !m_options_list.isEmpty() )
  {
    delete m_options_list.takeFirst();
  }
}


void Smb4KCustomOptionsPage::insertCustomOptions(const QList<Smb4KCustomOptions*> &list)
{
  // Insert those options that are not there.
  for ( int i = 0; i < list.size(); ++i )
  {
    Smb4KCustomOptions *options = findOptions( list.at( i )->url().prettyUrl() );
    
    if ( !options )
    {
      m_options_list << new Smb4KCustomOptions( *list[i] );
    }
    else
    {
      // Do nothing
    }
  }
  
  // Clear the list widget before (re)displaying the list
  while ( m_custom_options->count() != 0 )
  {
    delete m_custom_options->item( 0 );
  }
  
  // Display the list.
  for ( int i = 0; i < m_options_list.size(); ++i )
  {
    switch ( m_options_list.at( i )->type() )
    {
      case Smb4KCustomOptions::Host:
      {
        QListWidgetItem *item = new QListWidgetItem( KIcon( "network-server" ), 
                                    m_options_list.at( i )->unc(),
                                    m_custom_options, Host );
        item->setData( Qt::UserRole, m_options_list.at( i )->url().prettyUrl() );
        break;
      }
      case Smb4KCustomOptions::Share:
      {
        QListWidgetItem *item = new QListWidgetItem( KIcon( "folder-remote" ), 
                                    m_options_list.at( i )->unc(),
                                    m_custom_options, Share );
        item->setData( Qt::UserRole, m_options_list.at( i )->url().prettyUrl() );
        break;
      }
      default:
      {
        break;
      }
    }
  }

  m_custom_options->sortItems( Qt::AscendingOrder );
  
  m_removed = false;
}


const QList< Smb4KCustomOptions* > Smb4KCustomOptionsPage::getCustomOptions()
{
  return m_options_list;
}


void Smb4KCustomOptionsPage::clearEditors()
{
  // Do not reset the current custom options object here,
  // so that we can undo the last changes!
  
  // Clearing the editors means to reset them to their initial/default values.
  m_unc_address->clear();
  m_ip_address->clear();
  m_smb_port->setValue( Smb4KSettings::remoteSMBPort() );
#ifndef Q_OS_FREEBSD
  m_fs_port->setValue( Smb4KSettings::remoteFileSystemPort() );

  switch ( Smb4KSettings::writeAccess() )
  {
    case Smb4KSettings::EnumWriteAccess::ReadWrite:
    {
      m_write_access->setCurrentIndex( 0 );
      break;
    }
    case Smb4KSettings::EnumWriteAccess::ReadOnly:
    {
      m_write_access->setCurrentIndex( 1 );
      break;
    }
    default:
    {
      break;
    }
  }  
#endif
  switch ( Smb4KSettings::protocolHint() )
  {
    case Smb4KSettings::EnumProtocolHint::Automatic:
    {
      m_protocol_hint->setCurrentIndex( 0 );
      break;
    }
    case Smb4KSettings::EnumProtocolHint::RPC:
    {
      m_protocol_hint->setCurrentIndex( 1 );
      break;
    }
    case Smb4KSettings::EnumProtocolHint::RAP:
    {
      m_protocol_hint->setCurrentIndex( 2 );
      break;
    }
    case Smb4KSettings::EnumProtocolHint::ADS:
    {
      m_protocol_hint->setCurrentIndex( 3 );
      break;
    }
    default:
    {
      break;
    }
  }
      
  KUser user( getuid() );
  m_user_id->setCurrentItem( QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() ) );
  KUserGroup group( getgid() );
  m_group_id->setCurrentItem( QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() ) );
  m_kerberos->setChecked( false );
  m_mac_address->clear();
  m_send_before_scan->setChecked( false );
  m_send_before_mount->setChecked( false );
  
  // Disable widget
  m_general_editors->setEnabled( false );
  m_samba_editors->setEnabled( false );
  m_mac_editors->setEnabled( false );
}


Smb4KCustomOptions* Smb4KCustomOptionsPage::findOptions(const QString& url)
{
  Smb4KCustomOptions *options = NULL;
  
  for ( int i = 0; i < m_options_list.size(); ++i )
  {
    if ( QString::compare(url, m_options_list.at(i)->url().prettyUrl(), Qt::CaseInsensitive) == 0 )
    {
      options = m_options_list[i];
      break;
    }
    else
    {
      continue;
    }
  }
  
  return options;
}


void Smb4KCustomOptionsPage::populateEditors(Smb4KCustomOptions* options)
{
  // Commit changes
  commitChanges();
  
  // Copy custom options object
  m_current_options = options;
  
  // Populate the editors with the stored values.
  switch ( m_current_options->type() )
  {
    case Smb4KCustomOptions::Host:
    {
      m_unc_address->setText( m_current_options->unc() );
      break;
    }
    case Smb4KCustomOptions::Share:
    {
      m_unc_address->setText( m_current_options->unc() );
      break;
    }
    default:
    {
      break;
    }
  }
  
  if ( !m_current_options->ip().isEmpty() )
  {
    m_ip_address->setText( m_current_options->ip() );
  }
  else
  {
    // Do nothing
  } 
  
  if ( m_current_options->smbPort() != -1 )
  {
    m_smb_port->setValue( m_current_options->smbPort() );
  }
  else
  {
    m_smb_port->setValue( Smb4KSettings::remoteSMBPort() );
  }
  
#ifndef Q_OS_FREEBSD
  if ( m_current_options->fileSystemPort() != -1 )
  {
    m_fs_port->setValue( m_current_options->fileSystemPort() );
  }
  else
  {
    m_fs_port->setValue( Smb4KSettings::remoteFileSystemPort() );
  }
  
  if ( m_current_options->writeAccess() == Smb4KCustomOptions::UndefinedWriteAccess )
  {
    switch ( Smb4KSettings::writeAccess() )
    {
      case Smb4KSettings::EnumWriteAccess::ReadWrite:
      {
        m_write_access->setCurrentIndex( 0 );
        break;
      }
      case Smb4KSettings::EnumWriteAccess::ReadOnly:
      {
        m_write_access->setCurrentIndex( 1 );
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    switch ( m_current_options->writeAccess() )
    {
      case Smb4KCustomOptions::ReadWrite:
      {
        m_write_access->setCurrentIndex( 0 );
        break;
      }
      case Smb4KCustomOptions::ReadOnly:
      {
        m_write_access->setCurrentIndex( 1 );
        break;
      }
      default:
      {
        break;
      }
    }
  }
#endif

  if ( m_current_options->protocolHint() == Smb4KCustomOptions::UndefinedProtocolHint )
  {
    switch ( Smb4KSettings::protocolHint() )
    {
      case Smb4KSettings::EnumProtocolHint::Automatic:
      {
        m_protocol_hint->setCurrentIndex( 0 );
        break;
      }
      case Smb4KSettings::EnumProtocolHint::RPC:
      {
        m_protocol_hint->setCurrentIndex( 1 );
        break;
      }
      case Smb4KSettings::EnumProtocolHint::RAP:
      {
        m_protocol_hint->setCurrentIndex( 2 );
        break;
      }
      case Smb4KSettings::EnumProtocolHint::ADS:
      {
        m_protocol_hint->setCurrentIndex( 3 );
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    switch ( m_current_options->protocolHint() )
    {
      case Smb4KCustomOptions::Automatic:
      {
        m_protocol_hint->setCurrentIndex( 0 );
        break;
      }
      case Smb4KCustomOptions::RPC:
      {
        m_protocol_hint->setCurrentIndex( 1 );
        break;
      }
      case Smb4KCustomOptions::RAP:
      {
        m_protocol_hint->setCurrentIndex( 2 );
        break;
      }
      case Smb4KCustomOptions::ADS:
      {
        m_protocol_hint->setCurrentIndex( 3 );
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  KUser user( m_current_options->uid() );
  m_user_id->setCurrentItem( QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() ) );
  
  KUserGroup group( m_current_options->gid() );
  m_group_id->setCurrentItem( QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() ) );
  
  if ( m_current_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos )
  {
    m_kerberos->setChecked( Smb4KSettings::useKerberos() );
  }
  else
  {
    switch ( m_current_options->useKerberos() )
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        m_kerberos->setChecked( true );
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        m_kerberos->setChecked( false );
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  m_mac_address->setText( m_current_options->macAddress() );
  m_send_before_scan->setChecked( m_current_options->wolSendBeforeNetworkScan() );
  m_send_before_mount->setChecked( m_current_options->wolSendBeforeMount() );
  
  // Enable widget
  m_general_editors->setEnabled( true );
  m_samba_editors->setEnabled( true );
  
  if ( m_current_options->type() == Smb4KCustomOptions::Host && Smb4KSettings::enableWakeOnLAN() )
  {
    m_mac_editors->setEnabled( true );
  }
  else
  {
    // Do nothing
  }
  
  slotEnableWOLFeatures( m_mac_address->text() );
}


void Smb4KCustomOptionsPage::commitChanges()
{
  if ( m_current_options && !m_options_list.isEmpty() && m_samba_editors->isEnabled() )
  {
    Smb4KCustomOptions *options = findOptions( m_current_options->url().prettyUrl() );
    
    QHostAddress addr( m_ip_address->text() );
    
    if ( addr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol )
    {
      options->setIP( m_ip_address->text() );
    }
    else
    {
      // Do nothing
    }
    
    options->setSMBPort( m_smb_port->value() );
#ifndef Q_OS_FREEBSD
    options->setFileSystemPort( m_fs_port->value() );
    options->setWriteAccess( (Smb4KCustomOptions::WriteAccess)m_write_access->itemData( m_write_access->currentIndex() ).toInt() );
#endif
    options->setProtocolHint( (Smb4KCustomOptions::ProtocolHint)m_protocol_hint->itemData( m_protocol_hint->currentIndex() ).toInt() );
    options->setUID( m_user_id->itemData( m_user_id->currentIndex() ).toInt() );
    options->setGID( m_group_id->itemData( m_group_id->currentIndex() ).toInt() );

    if ( m_kerberos->isChecked() )
    {
      options->setUseKerberos( Smb4KCustomOptions::UseKerberos );
    }
    else
    {
      options->setUseKerberos( Smb4KCustomOptions::NoKerberos );
    }
    
    QRegExp exp( "..\\:..\\:..\\:..\\:..\\:.." );
    
    if ( exp.exactMatch( m_mac_address->text() ) )
    {
      options->setMACAddress( m_mac_address->text() );
    }
    else
    {
      // Do nothing
    }
    
    options->setWOLSendBeforeNetworkScan( m_send_before_scan->isChecked() );
    options->setWOLSendBeforeMount( m_send_before_mount->isChecked() );
    
    // In case of a host, propagate the changes to its shares.
    if ( options->type() == Smb4KCustomOptions::Host )
    {
      for ( int i = 0; i < m_options_list.size(); ++i )
      {
        if ( m_options_list.at( i )->type() == Smb4KCustomOptions::Share &&
             QString::compare( m_options_list.at( i )->hostName() , options->hostName(), Qt::CaseInsensitive ) == 0 &&
             QString::compare( m_options_list.at( i )->workgroupName() , options->workgroupName(), Qt::CaseInsensitive ) == 0 )
        {
#ifndef Q_OS_FREEBSD
          m_options_list.at( i )->setSMBPort( options->smbPort() );
#endif
          m_options_list.at( i )->setProtocolHint( options->protocolHint() );
          m_options_list.at( i )->setUseKerberos( options->useKerberos() );
          m_options_list.at( i )->setMACAddress( options->macAddress() );
          m_options_list.at( i )->setWOLSendBeforeNetworkScan( options->wolSendBeforeNetworkScan() );
          m_options_list.at( i )->setWOLSendBeforeMount( options->wolSendBeforeMount() );
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // Do nothing
    }
    
    m_maybe_changed = true;
    emit customSettingsModified();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KCustomOptionsPage::eventFilter(QObject* obj, QEvent* e)
{
  if ( obj == m_custom_options->viewport() )
  {
    if ( e->type() == QEvent::MouseButtonPress )
    {
      QMouseEvent *mev = static_cast<QMouseEvent *>( e );
      QPoint pos = m_custom_options->viewport()->mapFromGlobal( mev->globalPos() );
      QListWidgetItem *item = m_custom_options->itemAt( pos );
      
      if ( !item )
      {
        clearEditors();
        m_custom_options->clearSelection();
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
  else
  {
    // Do nothing
  }
  
  return QObject::eventFilter(obj, e);
}



void Smb4KCustomOptionsPage::slotEditCustomItem( QListWidgetItem *item )
{
  Smb4KCustomOptions *options = findOptions( item->data( Qt::UserRole ).toString() );
  
  if ( options )
  {
    populateEditors( options );
  }
  else
  {
    clearEditors();
  }
}


void Smb4KCustomOptionsPage::slotItemSelectionChanged()
{
  clearEditors();
}


void Smb4KCustomOptionsPage::slotCustomContextMenuRequested(const QPoint& pos)
{
  QListWidgetItem *item = m_custom_options->itemAt( pos );
  
  if ( item )
  {
    m_collection->action( "edit_action" )->setEnabled( true );
    m_collection->action( "remove_action" )->setEnabled( true );
  }
  else
  {
    m_collection->action( "edit_action" )->setEnabled( false );
    m_collection->action( "remove_action" )->setEnabled( false );
  }
  
  m_collection->action( "clear_action" )->setEnabled( m_custom_options->count() != 0 );
  m_collection->action( "undo_action" )->setEnabled( m_current_options || m_removed );
  
  m_menu->menu()->popup( m_custom_options->viewport()->mapToGlobal( pos ) );
}


void Smb4KCustomOptionsPage::slotEditActionTriggered( bool /*checked*/ )
{
  slotEditCustomItem( m_custom_options->currentItem() );
}


void Smb4KCustomOptionsPage::slotRemoveActionTriggered( bool /*checked*/ )
{
  QListWidgetItem *item = m_custom_options->currentItem();
  Smb4KCustomOptions *options = findOptions( item->data( Qt::UserRole ).toString() );
  
  if ( item && options )
  {
    if ( m_current_options && m_current_options->url().equals( options->url(), KUrl::CompareWithoutTrailingSlash ) )
    {
      m_current_options = NULL;
    }
    else
    {
      // Do nothing
    }
    
    int index = m_options_list.indexOf( options );
    
    if ( index != -1 )
    {
      m_options_list.removeAt( index );
    }
    else
    {
      // Do nothing
    }
    
    if ( QString::compare( item->text(), m_unc_address->text(), Qt::CaseInsensitive ) == 0 )
    {
      clearEditors();
    }
    else
    {
      // Do nothing
    }
    
    delete item;
    
    m_removed = true;
    m_maybe_changed = true;
    emit customSettingsModified();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KCustomOptionsPage::slotClearActionTriggered( bool /*checked*/ )
{
  clearEditors();

  while ( m_custom_options->count() != 0 )
  {
    delete m_custom_options->item( 0 );
  }
  
  while ( !m_options_list.isEmpty() )
  {
    delete m_options_list.takeFirst();
  }
  
  m_current_options = NULL;

  m_removed = true;
  m_maybe_changed = true;
  emit customSettingsModified();
}


void Smb4KCustomOptionsPage::slotUndoActionTriggered( bool /*checked*/ )
{
  if ( m_removed )
  {
    emit reloadCustomSettings();
  }
  else
  {
    if ( m_current_options )
    {
      if ( QString::compare(m_custom_options->currentItem()->data( Qt::UserRole ).toString(),
                            m_current_options->url().prettyUrl(), Qt::CaseInsensitive) == 0 )
      {
        // Populate the editor with the original values and commit
        // the changes.
        populateEditors( m_current_options );
        commitChanges();
      }
      else
      {
        // Copy the original values to the appropriate options object
        // in the list.
        Smb4KCustomOptions *options = findOptions( m_current_options->url().prettyUrl() );
        
        if ( options )
        {
          options->setSMBPort( m_current_options->smbPort() );
#ifndef Q_OS_FREEBSD
          options->setFileSystemPort( m_current_options->fileSystemPort() );
          options->setWriteAccess( m_current_options->writeAccess() );
#endif
          options->setProtocolHint( m_current_options->protocolHint() );
          options->setUID( m_current_options->uid() );
          options->setGID( m_current_options->gid() );
          options->setUseKerberos( m_current_options->useKerberos() );
          options->setMACAddress( m_current_options->macAddress() );
          options->setWOLSendBeforeNetworkScan( m_current_options->wolSendBeforeNetworkScan() );
          options->setWOLSendBeforeMount( m_current_options->wolSendBeforeMount() );
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  
  m_maybe_changed = true;
  emit customSettingsModified();
}


void Smb4KCustomOptionsPage::slotEntryChanged()
{
  commitChanges();
}


void Smb4KCustomOptionsPage::slotEnableWOLFeatures( const QString &mac_address )
{
  QRegExp exp( "..\\:..\\:..\\:..\\:..\\:.." );
    
  m_send_before_scan->setEnabled( exp.exactMatch( mac_address ) );
  m_send_before_mount->setEnabled( exp.exactMatch( mac_address ) );
}



#include "smb4kcustomoptionspage.moc"

