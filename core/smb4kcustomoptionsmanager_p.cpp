/***************************************************************************
    smb4kcustomoptionsmanager_p - Private helper classes for 
    Smb4KCustomOptionsManager class
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCoreApplication>

// KDE includes
#include <klocale.h>
#include <kstandardguiitem.h>
#include <kuser.h>

// application specific includes
#include <smb4kcustomoptionsmanager_p.h>
#include <smb4ksettings.h>


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog( Smb4KCustomOptions *options, QWidget *parent )
: KDialog( parent ), m_options( options )
{
  setCaption( i18n( "Custom Options" ) );
  setButtons( User1|Ok|Cancel );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KStandardGuiItem::defaults() );
  
  setupView();
  
  connect( this, SIGNAL( user1Clicked() ), SLOT( slotSetDefaultValues() ) );
  connect( this, SIGNAL( okClicked() ), SLOT( slotOKClicked() ) );
  
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
                          m_options->host()->hostName() ), description );
      break;
    }
    case Smb4KCustomOptions::Share:
    {
      label = new QLabel( i18n( "<p>Define custom options for share <b>%1</b> at host <b>%2</b>.</p>", 
                          m_options->share()->shareName(), m_options->share()->hostName() ), 
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
  
  QWidget *editors = new QWidget( main_widget );
  
  QGridLayout *editors_layout = new QGridLayout( editors );
  editors_layout->setSpacing( 5 );
  editors_layout->setMargin( 0 );
 
  QLabel *unc_label = new QLabel( i18n( "UNC Address:" ), editors );
  KLineEdit *unc    = NULL;
  
  switch ( m_options->type() )
  {
    case Smb4KCustomOptions::Host:
    {
      unc = new KLineEdit( m_options->host()->unc(), editors );
      break;
    }
    case Smb4KCustomOptions::Share:
    {
      unc = new KLineEdit( m_options->share()->unc(), editors );
      break;
    }
    default:
    {
      break;
    }
  }
  unc->setReadOnly( true );
      
  QLabel *smb_label = new QLabel( i18n( "SMB Port:" ), editors );
  m_smb_port        = new KIntNumInput( (m_options->smbPort() != Smb4KSettings::remoteSMBPort() ?
                      m_options->smbPort() : Smb4KSettings::remoteSMBPort()), editors );
  m_smb_port->setRange( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                        Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );
  m_smb_port->setSliderEnabled( true );

#ifndef Q_OS_FREEBSD
  QLabel *fs_label = new QLabel( i18n( "Filesystem Port:" ), editors );
  m_fs_port        = new KIntNumInput( (m_options->fileSystemPort() != Smb4KSettings::remoteFileSystemPort() ? 
                     m_options->fileSystemPort() : Smb4KSettings::remoteFileSystemPort()), editors );
  m_fs_port->setRange( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );
  m_fs_port->setSliderEnabled( true );
     
  QLabel *rw_label = new QLabel( i18n( "Write Access:" ), editors );
  m_write_access   = new KComboBox( editors );
  m_write_access->insertItem( 0, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadWrite ).label, 
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadWrite ) );
  m_write_access->insertItem( 1, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadOnly ).label, 
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadOnly ) );
 
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
#endif

  QLabel *protocol_label = new QLabel( i18n( "Protocol Hint:" ), editors );
  m_protocol_hint        = new KComboBox( editors );
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
  
  QLabel *uid_label = new QLabel( i18n( "User ID:" ), editors );
  m_user_id         = new KComboBox( editors );
  
  for ( int i = 0; i < KUser::allUsers().size(); i++ )
  {
    KUser user = KUser::allUsers().at( i );
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
  
  QLabel *gid_label = new QLabel( i18n( "Group ID:" ), editors );
  m_group_id        = new KComboBox( editors );
  
  for ( int i = 0; i < KUserGroup::allGroups().size(); i++ )
  {
    KUserGroup group = KUserGroup::allGroups().at( i );
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
  
  m_kerberos = new QCheckBox( Smb4KSettings::self()->useKerberosItem()->label(), editors );
  
  editors_layout->addWidget( unc_label, 0, 0, 0 );
  editors_layout->addWidget( unc, 0, 1, 0 );
  editors_layout->addWidget( smb_label, 1, 0, 0 );
  editors_layout->addWidget( m_smb_port, 1, 1, 0 );
#ifndef Q_OS_FREEBSD
  editors_layout->addWidget( fs_label, 2, 0, 0 );
  editors_layout->addWidget( m_fs_port, 2, 1, 0 );
  editors_layout->addWidget( rw_label, 3, 0, 0 );
  editors_layout->addWidget( m_write_access, 3, 1, 0 );
  editors_layout->addWidget( protocol_label, 4, 0, 0 );
  editors_layout->addWidget( m_protocol_hint, 4, 1, 0 );
  editors_layout->addWidget( uid_label, 5, 0, 0 );
  editors_layout->addWidget( m_user_id, 5, 1, 0 );
  editors_layout->addWidget( gid_label, 6, 0, 0 );
  editors_layout->addWidget( m_group_id, 6, 1, 0 );
  editors_layout->addWidget( m_kerberos, 7, 0, 1, 2, 0 );
#else
  editors_layout->addWidget( protocol_label, 2, 0, 0 );
  editors_layout->addWidget( m_protocol_hint, 2, 1, 0 );
  editors_layout->addWidget( uid_label, 3, 0, 0 );
  editors_layout->addWidget( m_user_id, 3, 1, 0 );
  editors_layout->addWidget( gid_label, 4, 0, 0 );
  editors_layout->addWidget( m_group_id, 4, 1, 0 );
  editors_layout->addWidget( m_kerberos, 5, 0, 1, 2, 0 );
#endif
  
  layout->addWidget( description );
  layout->addWidget( editors );
  
  connect( m_smb_port, SIGNAL( valueChanged( int ) ), SLOT( slotCheckValues() ) );
#ifndef Q_OS_FREEBSD
  connect( m_fs_port, SIGNAL( valueChanged(int) ), SLOT( slotCheckValues() ) );
  connect( m_write_access, SIGNAL( currentIndexChanged( int ) ), SLOT( slotCheckValues() ) );
#endif
  connect( m_protocol_hint, SIGNAL( currentIndexChanged( int ) ), SLOT( slotCheckValues() ) );
  connect( m_user_id, SIGNAL( currentIndexChanged( int ) ), SLOT( slotCheckValues() ) );
  connect( m_group_id, SIGNAL( currentIndexChanged( int ) ), SLOT( slotCheckValues() ) );
  connect( m_kerberos, SIGNAL( toggled( bool ) ), SLOT( slotCheckValues() ) );
  
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
  
  for ( int i = 0; i < m_user_id->count(); i++ )
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
  
  for ( int i = 0; i < m_group_id->count(); i++ )
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
#endif
  m_options->setProtocolHint( (Smb4KCustomOptions::ProtocolHint)m_protocol_hint->itemData( m_protocol_hint->currentIndex() ).toInt() );
  m_options->setUID( m_user_id->itemData( m_user_id->currentIndex() ).toInt() );
  m_options->setGID( m_group_id->itemData( m_group_id->currentIndex() ).toInt() );
  
  if ( m_kerberos->isEnabled() )
  {
    if ( m_kerberos->isChecked() )
    {
      m_options->setUseKerberos( Smb4KCustomOptions::UseKerberos );
    }
    else
    {
      m_options->setUseKerberos( Smb4KCustomOptions::NoKerberos );
    }
  }
  else
  {
    m_options->setUseKerberos( Smb4KCustomOptions::UndefinedKerberos );
  }
  
  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
}



Smb4KCustomOptionsManagerPrivate::Smb4KCustomOptionsManagerPrivate()
{
}


Smb4KCustomOptionsManagerPrivate::~Smb4KCustomOptionsManagerPrivate()
{
}

#include "smb4kcustomoptionsmanager_p.moc"

