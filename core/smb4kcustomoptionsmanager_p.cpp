/***************************************************************************
    smb4kcustomoptionsmanager_p - Private helper classes for
    Smb4KCustomOptionsManager class
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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
#include "smb4kcustomoptionsmanager_p.h"
#include "smb4ksettings.h"

// Qt includes
#include <QtCore/QCoreApplication>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>

// KDE includes
#include <klocale.h>
#include <kstandardguiitem.h>
#include <kuser.h>
#include <ktabwidget.h>


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog( Smb4KCustomOptions *options, QWidget *parent )
: KDialog( parent ), m_options( options )
{
  setCaption( i18n( "Custom Options" ) );
  setButtons( User1|Ok|Cancel );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KStandardGuiItem::defaults() );

  setupView();

  connect( this, SIGNAL(user1Clicked()), SLOT(slotSetDefaultValues()) );
  connect( this, SIGNAL(okClicked()), SLOT(slotOKClicked()) );

  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  restoreDialogSize( group );
}


Smb4KCustomOptionsDialog::~Smb4KCustomOptionsDialog()
{
}


void Smb4KCustomOptionsDialog::setupView()
{
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QVBoxLayout *layout = new QVBoxLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QWidget *description = new QWidget( main_widget );

  QHBoxLayout *desc_layout = new QHBoxLayout( description );
  desc_layout->setSpacing( 5 );
  desc_layout->setMargin( 0 );

  QLabel *pixmap = new QLabel( description );
  QPixmap mount_pix = KIcon( "preferences-system-network" ).pixmap( KIconLoader::SizeHuge );
  pixmap->setPixmap( mount_pix );
  pixmap->setAlignment( Qt::AlignBottom );

  QLabel *label = NULL;

  switch ( m_options->type() )
  {
    case Smb4KCustomOptions::Host:
    {
      label = new QLabel( i18n( "<p>Define custom options for host <b>%1</b> and all the shares it provides.</p>",
                          m_options->hostName() ), description );
      break;
    }
    case Smb4KCustomOptions::Share:
    {
      label = new QLabel( i18n( "<p>Define custom options for share <b>%1</b> at host <b>%2</b>.</p>",
                          m_options->shareName(), m_options->hostName() ),
                          description );
      break;
    }
    default:
    {
      label = new QLabel();
      break;
    }
  }

  label->setWordWrap( true );
  label->setAlignment( Qt::AlignBottom );

  desc_layout->addWidget( pixmap, 0 );
  desc_layout->addWidget( label, Qt::AlignBottom );

  QGroupBox *general = new QGroupBox( i18n( "General" ), main_widget );

  QGridLayout *general_layout = new QGridLayout( general );
  general_layout->setSpacing( 5 );

  QLabel *unc_label = new QLabel( i18n( "UNC Address:" ), general );
  KLineEdit *unc    = new KLineEdit( m_options->unc(), general );
  unc->setReadOnly( true );
  
  QLabel *ip_label = new QLabel( i18n( "IP Address:" ), general );
  KLineEdit *ip    = new KLineEdit( m_options->ip(), general );
  ip->setReadOnly( true );
  
  general_layout->addWidget( unc_label, 0, 0, 0 );
  general_layout->addWidget( unc, 0, 1, 0 );
  general_layout->addWidget( ip_label, 1, 0, 0 );
  general_layout->addWidget( ip, 1, 1, 0 );


  //
  // Tab widget with settings
  //

  KTabWidget *tab_widget = new KTabWidget( main_widget );

  //
  // Custom options for Samba
  //

  QWidget *samba_editors = new QWidget( tab_widget );
  
  QGridLayout *samba_editors_layout = new QGridLayout( samba_editors );
  samba_editors_layout->setSpacing( 5 );

  QLabel *smb_label = new QLabel( i18n( "SMB Port:" ), samba_editors );
  m_smb_port        = new KIntNumInput( (m_options->smbPort() != Smb4KSettings::remoteSMBPort() ?
                      m_options->smbPort() : Smb4KSettings::remoteSMBPort()), samba_editors );
  m_smb_port->setRange( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                        Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );
  m_smb_port->setSliderEnabled( true );
  smb_label->setBuddy( m_smb_port );

#ifndef Q_OS_FREEBSD
  QLabel *fs_label = new QLabel( i18n( "Filesystem Port:" ), samba_editors );
  m_fs_port        = new KIntNumInput( (m_options->fileSystemPort() != Smb4KSettings::remoteFileSystemPort() ?
                     m_options->fileSystemPort() : Smb4KSettings::remoteFileSystemPort()), samba_editors );
  m_fs_port->setRange( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );
  m_fs_port->setSliderEnabled( true );
  fs_label->setBuddy( m_fs_port );

  QLabel *rw_label = new QLabel( i18n( "Write Access:" ), samba_editors );
  m_write_access   = new KComboBox( samba_editors );
  m_write_access->insertItem( 0, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadWrite ).label,
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadWrite ) );
  m_write_access->insertItem( 1, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadOnly ).label,
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadOnly ) );
  rw_label->setBuddy( m_write_access );

  if ( m_options->writeAccess() == Smb4KCustomOptions::UndefinedWriteAccess )
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
    switch ( m_options->writeAccess() )
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

  QLabel *security_label = new QLabel( i18n( "Security Mode:" ), samba_editors );
  
  m_security_mode        = new KComboBox( samba_editors );
  m_security_mode->insertItem( 0, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::None ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::NoSecurityMode ) );
  m_security_mode->insertItem( 1, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Krb5 ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Krb5 ) );
  m_security_mode->insertItem( 2, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Krb5i ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Krb5i ) );
  m_security_mode->insertItem( 3, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlm ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Ntlm ) );
  m_security_mode->insertItem( 4, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmi ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Ntlmi ) );
  m_security_mode->insertItem( 5, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmv2 ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Ntlmv2 ) );
  m_security_mode->insertItem( 6, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmv2i ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Ntlmv2i ) );
  m_security_mode->insertItem( 7, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmssp ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Ntlmssp ) );
  m_security_mode->insertItem( 8, Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmsspi ).label,
                               QVariant::fromValue<int>( Smb4KCustomOptions::Ntlmsspi ) );
  security_label->setBuddy( m_security_mode );

  if ( m_options->securityMode() == Smb4KCustomOptions::UndefinedSecurityMode )
  {
    switch ( Smb4KSettings::securityMode() )
    {
      case Smb4KSettings::EnumSecurityMode::None:
      {
        m_security_mode->setCurrentIndex( 0 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Krb5:
      {
        m_security_mode->setCurrentIndex( 1 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Krb5i:
      {
        m_security_mode->setCurrentIndex( 2 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Ntlm:
      {
        m_security_mode->setCurrentIndex( 3 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Ntlmi:
      {
        m_security_mode->setCurrentIndex( 4 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Ntlmv2:
      {
        m_security_mode->setCurrentIndex( 5 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Ntlmv2i:
      {
        m_security_mode->setCurrentIndex( 6 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Ntlmssp:
      {
        m_security_mode->setCurrentIndex( 7 );
        break;
      }
      case Smb4KSettings::EnumSecurityMode::Ntlmsspi:
      {
        m_security_mode->setCurrentIndex( 8 );
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
    switch ( m_options->securityMode() )
    {
      case Smb4KCustomOptions::NoSecurityMode:
      {
        m_security_mode->setCurrentIndex( 0 );
        break;
      }
      case Smb4KCustomOptions::Krb5:
      {
        m_security_mode->setCurrentIndex( 1 );
        break;
      }
      case Smb4KCustomOptions::Krb5i:
      {
        m_security_mode->setCurrentIndex( 2 );
        break;
      }
      case Smb4KCustomOptions::Ntlm:
      {
        m_security_mode->setCurrentIndex( 3 );
        break;
      }
      case Smb4KCustomOptions::Ntlmi:
      {
        m_security_mode->setCurrentIndex( 4 );
        break;
      }
      case Smb4KCustomOptions::Ntlmv2:
      {
        m_security_mode->setCurrentIndex( 5 );
        break;
      }
      case Smb4KCustomOptions::Ntlmv2i:
      {
        m_security_mode->setCurrentIndex( 6 );
        break;
      }
      case Smb4KCustomOptions::Ntlmssp:
      {
        m_security_mode->setCurrentIndex( 7 );
        break;
      }
      case Smb4KCustomOptions::Ntlmsspi:
      {
        m_security_mode->setCurrentIndex( 8 );
        break;
      }
      default:
      {
        break;
      }
    }
  }
#endif

  QLabel *protocol_label = new QLabel( i18n( "Protocol Hint:" ), samba_editors );
  m_protocol_hint        = new KComboBox( samba_editors );
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

  if ( m_options->protocolHint() == Smb4KCustomOptions::UndefinedProtocolHint )
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
    switch ( m_options->protocolHint() )
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

  QLabel *uid_label = new QLabel( i18n( "User ID:" ), samba_editors );
  m_user_id         = new KComboBox( samba_editors );
  uid_label->setBuddy( m_user_id );

  // To avoid weird crashes under FreeBSD, first copy KUser::allUsers().
  QList<KUser> all_users = KUser::allUsers();

  for ( int i = 0; i < all_users.size(); ++i )
  {
    KUser user = all_users.at( i );
    m_user_id->insertItem( i, QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() ),
                           QVariant::fromValue<K_UID>( user.uid() ) );

    if ( m_options->uid() == user.uid() )
    {
      m_user_id->setCurrentIndex( i );
    }
    else
    {
      // Do nothing
    }
  }

  QLabel *gid_label = new QLabel( i18n( "Group ID:" ), samba_editors );
  m_group_id        = new KComboBox( samba_editors );
  gid_label->setBuddy( m_group_id );

  // To avoid weird crashes under FreeBSD, first copy KUserGroup::allGroups().
  QList<KUserGroup> all_groups = KUserGroup::allGroups();

  for ( int i = 0; i < all_groups.size(); ++i )
  {
    KUserGroup group = all_groups.at( i );
    m_group_id->insertItem( i, QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() ),
                           QVariant::fromValue<K_UID>( group.gid() ) );

    if ( m_options->gid() == group.gid() )
    {
      m_group_id->setCurrentIndex( i );
    }
    else
    {
      // Do nothing
    }
  }

  m_kerberos = new QCheckBox( Smb4KSettings::self()->useKerberosItem()->label(), samba_editors );

  if ( m_options->useKerberos() == Smb4KCustomOptions::UndefinedKerberos )
  {
    m_kerberos->setChecked( Smb4KSettings::useKerberos() );
  }
  else
  {
    switch ( m_options->useKerberos() )
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

  samba_editors_layout->addWidget( smb_label, 0, 0, 0 );
  samba_editors_layout->addWidget( m_smb_port, 0, 1, 0 );
#ifndef Q_OS_FREEBSD
  samba_editors_layout->addWidget( fs_label, 1, 0, 0 );
  samba_editors_layout->addWidget( m_fs_port, 1, 1, 0 );
  samba_editors_layout->addWidget( rw_label, 2, 0, 0 );
  samba_editors_layout->addWidget( m_write_access, 2, 1, 0 );
  samba_editors_layout->addWidget( security_label, 3, 0, 0 );
  samba_editors_layout->addWidget( m_security_mode, 3, 1, 0 );
  samba_editors_layout->addWidget( protocol_label, 4, 0, 0 );
  samba_editors_layout->addWidget( m_protocol_hint, 4, 1, 0 );
  samba_editors_layout->addWidget( uid_label, 5, 0, 0 );
  samba_editors_layout->addWidget( m_user_id, 5, 1, 0 );
  samba_editors_layout->addWidget( gid_label, 6, 0, 0 );
  samba_editors_layout->addWidget( m_group_id, 6, 1, 0 );
  samba_editors_layout->addWidget( m_kerberos, 7, 0, 1, 2, 0 );
#else
  samba_editors_layout->addWidget( protocol_label, 1, 0, 0 );
  samba_editors_layout->addWidget( m_protocol_hint, 1, 1, 0 );
  samba_editors_layout->addWidget( uid_label, 2, 0, 0 );
  samba_editors_layout->addWidget( m_user_id, 2, 1, 0 );
  samba_editors_layout->addWidget( gid_label, 3, 0, 0 );
  samba_editors_layout->addWidget( m_group_id, 3, 1, 0 );
  samba_editors_layout->addWidget( m_kerberos, 4, 0, 1, 2, 0 );
#endif

  tab_widget->addTab( samba_editors, i18n( "Samba" ) );


  //
  // Custom options for Wake-On-LAN
  //

  QWidget *wol_editors = new QWidget( tab_widget );
  
  QGridLayout *wol_editors_layout = new QGridLayout( wol_editors );
  wol_editors_layout->setSpacing( 5 );
  
  QLabel *mac_label = new QLabel( i18n( "MAC Address:" ), wol_editors );
  m_mac_address     = new KLineEdit( m_options->macAddress(), wol_editors );
  mac_label->setBuddy( m_mac_address );
  
  // If you change the texts here, please also alter them in the config
  // dialog.
  m_send_before_scan = new QCheckBox( i18n( "Send magic package before scanning the network neighborhood" ), wol_editors );
  m_send_before_scan->setChecked( m_options->wolSendBeforeNetworkScan() );
  m_send_before_scan->setEnabled( (m_options->type() == Smb4KCustomOptions::Host) );
  
  m_send_before_mount = new QCheckBox( i18n( "Send magic package before mounting a share" ), wol_editors );
  m_send_before_mount->setChecked( m_options->wolSendBeforeMount() );
  m_send_before_mount->setEnabled( (m_options->type() == Smb4KCustomOptions::Host) );
  
  wol_editors_layout->addWidget( mac_label, 0, 0, 0 );
  wol_editors_layout->addWidget( m_mac_address, 0, 1, 0 );
  wol_editors_layout->addWidget( m_send_before_scan, 1, 0, 1, 2, 0 );
  wol_editors_layout->addWidget( m_send_before_mount, 2, 0, 1, 2, 0 );
  wol_editors_layout->setRowStretch( 3, 100 );

  tab_widget->addTab( wol_editors, i18n( "Wake-On-LAN" ) );

  layout->addWidget( description );
  layout->addWidget( general );
  layout->addWidget( tab_widget );

  connect( m_smb_port, SIGNAL(valueChanged(int)), SLOT(slotCheckValues()) );
#ifndef Q_OS_FREEBSD
  connect( m_fs_port, SIGNAL(valueChanged(int)), SLOT(slotCheckValues()) );
  connect( m_write_access, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()) );
  connect( m_security_mode, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()) );
#endif
  connect( m_protocol_hint, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()) );
  connect( m_user_id, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()) );
  connect( m_group_id, SIGNAL(currentIndexChanged(int)), SLOT(slotCheckValues()) );
  connect( m_kerberos, SIGNAL(toggled(bool)), SLOT(slotCheckValues()) );
  connect( m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotCheckValues()) );
  connect( m_mac_address, SIGNAL(textChanged(QString)), SLOT(slotEnableWOLFeatures(QString)) );
  connect( m_send_before_scan, SIGNAL(toggled(bool)), SLOT(slotCheckValues()) );
  connect( m_send_before_mount, SIGNAL(toggled(bool)), SLOT(slotCheckValues()) );
  
  wol_editors->setEnabled( (m_options->type() == Smb4KCustomOptions::Host && Smb4KSettings::enableWakeOnLAN()) );
  
  enableButton( User1, !defaultValues() );
}


bool Smb4KCustomOptionsDialog::defaultValues()
{
  if ( m_smb_port->value() != Smb4KSettings::remoteSMBPort() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

#ifndef Q_OS_FREEBSD
  if ( m_fs_port->value() != Smb4KSettings::remoteFileSystemPort() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_write_access->currentText(),
       Smb4KSettings::self()->writeAccessItem()->choices().value( Smb4KSettings::self()->writeAccess() ).label,
       Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_security_mode->currentText(),
       Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::self()->securityMode() ).label,
       Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
#endif

  if ( QString::compare( m_protocol_hint->currentText(),
       Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::self()->protocolHint() ).label,
       Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  K_UID uid = (K_UID)m_user_id->itemData( m_user_id->currentIndex() ).toInt();

  if ( uid != (K_UID)Smb4KSettings::userID().toInt() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  K_GID gid = (K_GID)m_group_id->itemData( m_group_id->currentIndex() ).toInt();

  if ( gid != (K_GID)Smb4KSettings::groupID().toInt() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_kerberos->isChecked() != Smb4KSettings::useKerberos() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( m_options->type() == Smb4KCustomOptions::Host )
  {
    if ( !m_mac_address->text().isEmpty() )
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    if ( m_send_before_scan->isChecked() )
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    if ( m_send_before_mount->isChecked() )
    {
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

  return true;
}


void Smb4KCustomOptionsDialog::slotSetDefaultValues()
{
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

  switch ( Smb4KSettings::securityMode() )
  {
    case Smb4KSettings::EnumSecurityMode::None:
    {
      m_security_mode->setCurrentIndex( 0 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5:
    {
      m_security_mode->setCurrentIndex( 1 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5i:
    {
      m_security_mode->setCurrentIndex( 2 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlm:
    {
      m_security_mode->setCurrentIndex( 3 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmi:
    {
      m_security_mode->setCurrentIndex( 4 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2:
    {
      m_security_mode->setCurrentIndex( 5 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2i:
    {
      m_security_mode->setCurrentIndex( 6 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmssp:
    {
      m_security_mode->setCurrentIndex( 7 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmsspi:
    {
      m_security_mode->setCurrentIndex( 8 );
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

  for ( int i = 0; i < m_user_id->count(); ++i )
  {
    if ( m_user_id->itemData( i ).toInt() == Smb4KSettings::userID().toInt() )
    {
      m_user_id->setCurrentIndex( i );
      break;
    }
    else
    {
      continue;
    }
  }

  for ( int i = 0; i < m_group_id->count(); ++i )
  {
    if ( m_group_id->itemData( i ).toInt() == Smb4KSettings::groupID().toInt() )
    {
      m_group_id->setCurrentIndex( i );
      break;
    }
    else
    {
      continue;
    }
  }

  m_kerberos->setChecked( Smb4KSettings::self()->useKerberos() );

  if ( m_options->type() == Smb4KCustomOptions::Host )
  {
    m_mac_address->clear();
    m_send_before_scan->setChecked( false );
    m_send_before_mount->setChecked( false );
  }
  else
  {
    // Do nothing, because with shares these widgets are
    // disabled.
  }
}


void Smb4KCustomOptionsDialog::slotCheckValues()
{
  enableButton( User1, !defaultValues() );
}


void Smb4KCustomOptionsDialog::slotOKClicked()
{
  m_options->setSMBPort( m_smb_port->value() );
#ifndef Q_OS_FREEBSD
  m_options->setFileSystemPort( m_fs_port->value() );
  m_options->setWriteAccess( (Smb4KCustomOptions::WriteAccess)m_write_access->itemData( m_write_access->currentIndex() ).toInt() );
  m_options->setSecurityMode( (Smb4KCustomOptions::SecurityMode)m_security_mode->itemData( m_security_mode->currentIndex() ).toInt() );
#endif
  m_options->setProtocolHint( (Smb4KCustomOptions::ProtocolHint)m_protocol_hint->itemData( m_protocol_hint->currentIndex() ).toInt() );
  m_options->setUID( m_user_id->itemData( m_user_id->currentIndex() ).toInt() );
  m_options->setGID( m_group_id->itemData( m_group_id->currentIndex() ).toInt() );

  if ( m_kerberos->isChecked() )
  {
    m_options->setUseKerberos( Smb4KCustomOptions::UseKerberos );
  }
  else
  {
    m_options->setUseKerberos( Smb4KCustomOptions::NoKerberos );
  }
  
  m_options->setMACAddress( m_mac_address->text() );
  m_options->setWOLSendBeforeNetworkScan( m_send_before_scan->isChecked() );
  m_options->setWOLSendBeforeMount( m_send_before_mount->isChecked() );

  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
}


void Smb4KCustomOptionsDialog::slotEnableWOLFeatures( const QString &mac )
{
  QRegExp exp( "..\\:..\\:..\\:..\\:..\\:.." );
    
  m_send_before_scan->setEnabled( exp.exactMatch( mac ) );
  m_send_before_mount->setEnabled( exp.exactMatch( mac ) );
}


#include "smb4kcustomoptionsmanager_p.moc"

