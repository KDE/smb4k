/***************************************************************************
    smb4kcustomoptionsdialog  -  With this dialog the user can define
    custom Samba options for hosts or shares.
                             -------------------
    begin                : So Jun 25 2006
    copyright            : (C) 2006-2010 by Alexander Reinholdt
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
#include <QGridLayout>
#include <QLabel>
#include <QList>

// KDE includes
#include <klocale.h>
#include <kapplication.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kstandardguiitem.h>
#include <kuser.h>

// application specific includes
#include "smb4kcustomoptionsdialog.h"
#include <core/smb4kglobal.h>
#include <core/smb4kcore.h>
#include <core/smb4ksambaoptionsinfo.h>
#include <core/smb4ksambaoptionshandler.h>
#include <core/smb4ksettings.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>
#include <core/smb4khomesshareshandler.h>

using namespace Smb4KGlobal;


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog( Smb4KHost *host, QWidget *parent )
: KDialog( parent ), m_type( Host ), m_host( host ), m_share( NULL )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Custom Options" ) );
  setButtons( User1|Ok|Cancel );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KStandardGuiItem::defaults() );

  setupHostDialog();

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  restoreDialogSize( group );
}


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ), m_type( Share ), m_host( NULL ), m_share( share )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Custom Options" ) );
  setButtons( User1|Ok|Cancel );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KStandardGuiItem::defaults() );

  if ( !m_share->isHomesShare() )
  {
    // We do not use parent as parent for the "Specify User"
    // dialog, so that the behavior is uniform.
    QWidget *p = 0;

    if ( kapp )
    {
      p = kapp->activeWindow();
    }

    (void) Smb4KHomesSharesHandler::self()->specifyUser( m_share, p );
  }
  else
  {
    // Do nothing
  }
  
  setupShareDialog();

  setMinimumSize( (sizeHint().width() > 350 ? sizeHint().width() : 350), sizeHint().height() );

  setInitialSize( QSize( minimumWidth(), minimumHeight() ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  restoreDialogSize( group );
}


Smb4KCustomOptionsDialog::~Smb4KCustomOptionsDialog()
{
}


void Smb4KCustomOptionsDialog::setupHostDialog()
{
  // Input widgets.
  m_smb_port_input = NULL;
  m_kerberos = NULL;
  m_proto_input = NULL;
  m_uid_input = NULL;
  m_gid_input = NULL;
#ifndef Q_OS_FREEBSD
  m_fs_port_input = NULL;
  m_rw_input = NULL;
#endif

  // Set up the widget:
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );
  
  // UNC
  QLabel *unc_label = new QLabel( i18n( "UNC Address:" ), main_widget );
  KLineEdit *unc    = new KLineEdit( m_host->unc(), main_widget );
  unc->setReadOnly( true );
  unc->setWhatsThis( i18n( "The Uniform Naming Convention (UNC) address "
    "describes the location of the host. It has the following syntax: "
    "//HOST." ) );
//   unc->setToolTip( i18n( "The UNC address of the host" ) );
  
  // SMB port
  QLabel *smb_port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(), main_widget );
  m_smb_port_input = new KIntNumInput( -1, main_widget );
  m_smb_port_input->setMinimumWidth( 200 );
  m_smb_port_input->setMinimum( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt() );
  m_smb_port_input->setMaximum( Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );
  m_smb_port_input->setWhatsThis( Smb4KSettings::self()->remoteSMBPortItem()->whatsThis() );
//   m_smb_port_input->setToolTip( i18n( "The port used for communicating with servers" ) );
  
  // Protocol hint
  QLabel *protocol_label = new QLabel( Smb4KSettings::self()->protocolHintItem()->label(), main_widget );
  m_proto_input = new KComboBox( false, main_widget );
  m_proto_input->setMinimumWidth( 200 );

  QStringList protocol_items;
  protocol_items.append( i18n( "automatic" ) );
  protocol_items.append( "RPC" );
  protocol_items.append( "RAP" );
  protocol_items.append( "ADS" );

  m_proto_input->insertItems( 0, protocol_items );
  m_proto_input->setWhatsThis( Smb4KSettings::self()->protocolHintItem()->whatsThis() );
//   m_proto_input->setToolTip( i18n( "The protocol hint" ) );
  
  // Kerberos
  m_kerberos = new QCheckBox( Smb4KSettings::self()->useKerberosItem()->label(), main_widget );
  m_kerberos->setWhatsThis( Smb4KSettings::self()->useKerberosItem()->whatsThis() );
//   m_kerberos->setToolTip( i18n( "Use Kerberos for authentication" ) );
  
  // Line
  QFrame *line = new QFrame( main_widget );
  line->setFrameStyle( QFrame::HLine );
  
#ifndef Q_OS_FREEBSD
  // File system port
  QLabel *fs_port_label = new QLabel( Smb4KSettings::self()->remoteFileSystemPortItem()->label(), main_widget );
  m_fs_port_input = new KIntNumInput( -1, main_widget );
  m_fs_port_input->setMinimumWidth( 200 );
  m_fs_port_input->setMinimum( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt() );
  m_fs_port_input->setMaximum( Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );
  m_fs_port_input->setWhatsThis( Smb4KSettings::self()->remoteFileSystemPortItem()->whatsThis() );
//   m_fs_port_input->setToolTip( i18n( "The port used for mounting shares" ) );
  
  // Write access
  QLabel *permission_label = new QLabel( Smb4KSettings::self()->writeAccessItem()->label(), main_widget );
  m_rw_input = new KComboBox( false, main_widget );
  m_rw_input->setMinimumWidth( 200 );

  QStringList write_access_entries;
  write_access_entries.append( i18n( "read-write" ) );
  write_access_entries.append( i18n( "read-only" ) );

  m_rw_input->insertItems( 0, write_access_entries );
  m_rw_input->setWhatsThis( Smb4KSettings::self()->writeAccessItem()->whatsThis() );
//   m_rw_input->setToolTip( i18n( "The write access" ) );
#endif

  // User ID
  QLabel *uid_label = new QLabel( Smb4KSettings::self()->userIDItem()->label(), main_widget );
  m_uid_input = new KComboBox( main_widget );
  m_uid_input->setMinimumWidth( 200 );

  QList<KUser> user_list = KUser::allUsers();
  QStringList uids;

  for ( int i = 0; i < user_list.size(); ++i )
  {
    uids.append( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                     .arg( user_list.at( i ).uid() ) );
  }

  uids.sort();

  m_uid_input->addItems( uids );
  m_uid_input->setWhatsThis( Smb4KSettings::self()->userIDItem()->whatsThis() );
//   m_uid_input->setToolTip( i18n( "The user ID" ) );

  // Group ID
  QLabel *gid_label = new QLabel( Smb4KSettings::self()->groupIDItem()->label(), main_widget );
  m_gid_input = new KComboBox( main_widget );
  m_gid_input->setMinimumWidth( 200 );

  QList<KUserGroup> group_list = KUserGroup::allGroups();
  QStringList gids;

  for ( int i = 0; i < group_list.size(); ++i )
  {
    gids.append( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                     .arg( group_list.at( i ).gid() ) );
  }

  gids.sort();

  m_gid_input->addItems( gids );
  m_gid_input->setWhatsThis( Smb4KSettings::self()->groupIDItem()->whatsThis() );
//   m_gid_input->setToolTip( i18n( "The group ID" ) );
  
  // Put everything in the layout.
  layout->addWidget( unc_label, 0, 0, 0 );
  layout->addWidget( unc, 0, 1, 0 );
  layout->addWidget( smb_port_label, 1, 0, 0 );
  layout->addWidget( m_smb_port_input, 1, 1, 0 );
#ifndef Q_OS_FREEBSD
  layout->addWidget( protocol_label, 2, 0, 0 );
  layout->addWidget( m_proto_input, 2, 1, 0 );
  layout->addWidget( m_kerberos, 3, 0, 1, 2, 0 );
  layout->addWidget( line, 4, 0, 1, 2, 0 );
  layout->addWidget( fs_port_label, 5, 0, 0 );
  layout->addWidget( m_fs_port_input, 5, 1, 0 );
  layout->addWidget( permission_label, 6, 0, 0 );
  layout->addWidget( m_rw_input, 6, 1, 0 );
  layout->addWidget( uid_label, 7, 0, 0 );
  layout->addWidget( m_uid_input, 7, 1, 0 );
  layout->addWidget( gid_label, 8, 0, 0 );
  layout->addWidget( m_gid_input, 8, 1, 0 );
#else
  layout->addWidget( protocol_label, 2, 0, 0 );
  layout->addWidget( m_proto_input, 2, 1, 0 );
  layout->addWidget( m_kerberos, 3, 0, 1, 2, 0 );
  layout->addWidget( line, 4, 0, 1, 2, 0 );
  layout->addWidget( uid_label, 5, 0, 0 );
  layout->addWidget( m_uid_input, 5, 1, 0 );
  layout->addWidget( gid_label, 6, 0, 0 );
  layout->addWidget( m_gid_input, 6, 1, 0 );
#endif

  // Get the custom options.
  Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_host );
  
  // Set the SMB port.
  if ( info && info->smbPort() != -1 )
  {
    m_smb_port_input->setValue( info->smbPort() );
  }
  else
  {
    m_smb_port_input->setValue( Smb4KSettings::remoteSMBPort() );
  }

  // Set the protocol.
  if ( info && info->protocol() != Smb4KSambaOptionsInfo::UndefinedProtocol )
  {
    switch ( info->protocol() )
    {
      case Smb4KSambaOptionsInfo::Automatic:
      {
        m_proto_input->setCurrentItem( i18n( "automatic" ), false );
        break;
      }
      case Smb4KSambaOptionsInfo::RPC:
      {
        m_proto_input->setCurrentItem( "RPC", false );
        break;
      }
      case Smb4KSambaOptionsInfo::RAP:
      {
        m_proto_input->setCurrentItem( "RAP", false );
        break;
      }
      case Smb4KSambaOptionsInfo::ADS:
      {
        m_proto_input->setCurrentItem( "ADS", false );
        break;
      }
      case Smb4KSambaOptionsInfo::UndefinedProtocol:
      default:
      {
        break;
      }
    }
  }
  else
  {
    switch ( Smb4KSettings::protocolHint() )
    {
      case Smb4KSettings::EnumProtocolHint::Automatic:
      {
        m_proto_input->setCurrentItem( i18n( "automatic" ), false );
        break;
      }
      case Smb4KSettings::EnumProtocolHint::RPC:
      {
        m_proto_input->setCurrentItem( "RPC", false );
        break;
      }
      case Smb4KSettings::EnumProtocolHint::RAP:
      {
        m_proto_input->setCurrentItem( "RAP", false );
        break;
      }
      case Smb4KSettings::EnumProtocolHint::ADS:
      {
        m_proto_input->setCurrentItem( "ADS", false );
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  // Set Kerberos usage.
  if ( info && info->useKerberos() != Smb4KSambaOptionsInfo::UndefinedKerberos )
  {
    switch ( info->useKerberos() )
    {
      case Smb4KSambaOptionsInfo::UseKerberos:
      {
        m_kerberos->setChecked( true );
        break;
      }
      case Smb4KSambaOptionsInfo::NoKerberos:
      {
        m_kerberos->setChecked( false );
        break;
      }
      case Smb4KSambaOptionsInfo::UndefinedKerberos:
      default:
      {
        break;
      }
    }
  }
  else
  {
    m_kerberos->setChecked( Smb4KSettings::useKerberos() );
  }
  
#ifndef Q_OS_FREEBSD
  // Set the file system port.
  if ( info && info->fileSystemPort() != -1 )
  {
    m_fs_port_input->setValue( info->fileSystemPort() );
  }
  else
  {
    m_fs_port_input->setValue( Smb4KSettings::remoteFileSystemPort() );
  }
  
  // Set the write access.
  if ( info && info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
  {
    switch( info->writeAccess() )
    {
      case Smb4KSambaOptionsInfo::ReadWrite:
      {
        m_rw_input->setCurrentItem( i18n( "read-write" ), false );
        break;
      }
      case Smb4KSambaOptionsInfo::ReadOnly:
      {
        m_rw_input->setCurrentItem( i18n( "read-only" ), false );
        break;
      }
      case Smb4KSambaOptionsInfo::UndefinedWriteAccess:
      default:
      {
        break;
      }
    }        
  }
  else
  {
    switch ( Smb4KSettings::writeAccess() )
    {
      case Smb4KSettings::EnumWriteAccess::ReadWrite:
      {
        m_rw_input->setCurrentItem( i18n( "read-write" ), false );
        break;
      }
      case Smb4KSettings::EnumWriteAccess::ReadOnly:
      {
        m_rw_input->setCurrentItem( i18n( "read-only" ), false );
        break;
      }
      default:
      {
        break;
      }
    }
  }
#endif

  // Set the user ID.
  QString user_text;
      
  if ( info && info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
  {
    KUser user( (K_UID)info->uid() );
    user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
  }
  else
  {
    KUser user( (K_UID)Smb4KSettings::userID().toInt() );
    user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
  }
      
  int user_index = m_uid_input->findText( user_text );
  m_uid_input->setCurrentIndex( user_index );

  // Set the GID.
  QString group_text;
      
  if ( info && info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
  {
    KUserGroup group( (K_GID)info->gid() );
    group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
  }
  else
  {
    KUserGroup group( (K_GID)Smb4KSettings::groupID().toInt() );
    group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
  }
      
  int group_index = m_gid_input->findText( group_text );
  m_gid_input->setCurrentIndex( group_index );
  
  // Connections
  connect( m_smb_port_input, SIGNAL( valueChanged( int ) ),
           this, SLOT( slotSMBPortChanged( int ) ) );
           
  connect( m_proto_input, SIGNAL( activated( const QString & ) ),
           this, SLOT( slotProtocolChanged( const QString & ) ) );

  connect( m_kerberos, SIGNAL( toggled( bool ) ),
           this, SLOT( slotKerberosToggled( bool ) ) );
           
#ifndef Q_OS_FREEBSD
  connect( m_fs_port_input, SIGNAL( valueChanged( int ) ),
           this, SLOT( slotFileSystemPortChanged( int ) ) );
           
  connect( m_rw_input, SIGNAL( activated( const QString & ) ),
           this, SLOT( slotWriteAccessChanged( const QString & ) ) );
#endif
           
  connect( m_uid_input, SIGNAL( activated( const QString & ) ),
           this, SLOT( slotUIDChanged( const QString & ) ) );

  connect( m_gid_input, SIGNAL( activated( const QString & ) ),
           this, SLOT( slotGIDChanged( const QString & ) ) );
  
  connect( this, SIGNAL( okClicked() ),
           this, SLOT( slotOKButtonClicked() ) );

  connect( this, SIGNAL( user1Clicked() ),
           this, SLOT( slotDefaultButtonClicked() ) );

  // Enable the buttons.
  enableButton( Ok, false );
  enableButton( User1, !hasDefaultSettings() );
}


void Smb4KCustomOptionsDialog::setupShareDialog()
{
  // Input widgets.
  m_smb_port_input = NULL;
  m_kerberos = NULL;
  m_proto_input = NULL;
  m_uid_input = NULL;
  m_gid_input = NULL;
#ifndef Q_OS_FREEBSD
  m_fs_port_input = NULL;
  m_rw_input = NULL;
#endif

  // Set up the widget:
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );
  
  // UNC
  QLabel *unc_label = new QLabel( i18n( "UNC Address:" ), main_widget );
  KLineEdit *unc    = new KLineEdit( m_share->unc(), main_widget );
  unc->setReadOnly( true );
  unc->setWhatsThis( i18n( "The Uniform Naming Convention (UNC) address "
    "describes the location of the share. It has the following syntax: "
    "//HOST/SHARE" ) );
//   unc->setToolTip( i18n( "The UNC address of the share" ) );
  
#ifndef Q_OS_FREEBSD
  // File system port
  QLabel *fs_port_label = new QLabel( Smb4KSettings::self()->remoteFileSystemPortItem()->label(), main_widget );
  m_fs_port_input = new KIntNumInput( -1, main_widget );
  m_fs_port_input->setMinimumWidth( 200 );
  m_fs_port_input->setMinimum( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt() );
  m_fs_port_input->setMaximum( Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );
  m_fs_port_input->setWhatsThis( Smb4KSettings::self()->remoteFileSystemPortItem()->whatsThis() );
//   m_fs_port_input->setToolTip( i18n( "The port used for mounting shares" ) );

  // Write access
  QLabel *permission_label = new QLabel( Smb4KSettings::self()->writeAccessItem()->label(), main_widget );
  m_rw_input = new KComboBox( false, main_widget );
  m_rw_input->setMinimumWidth( 200 );

  QStringList write_access_entries;
  write_access_entries.append( i18n( "read-write" ) );
  write_access_entries.append( i18n( "read-only" ) );

  m_rw_input->insertItems( 0, write_access_entries );
  m_rw_input->setWhatsThis( Smb4KSettings::self()->writeAccessItem()->whatsThis() );
//   m_rw_input->setToolTip( i18n( "The write access" ) );
#else
  // SMB port
  QLabel *smb_port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(), main_widget );
  m_smb_port_input = new KIntNumInput( -1, main_widget );
  m_smb_port_input->setMinimumWidth( 200 );
  m_smb_port_input->setMinimum( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt() );
  m_smb_port_input->setMaximum( Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );
  m_smb_port_input->setWhatsThis( Smb4KSettings::self()->remoteSMBPortItem()->whatsThis() );
//   m_smb_port_input->setToolTip( i18n( "The port used for mounting shares" ) );
#endif

  // User ID
  QLabel *uid_label = new QLabel( Smb4KSettings::self()->userIDItem()->label(), main_widget );
  m_uid_input = new KComboBox( main_widget );
  m_uid_input->setMinimumWidth( 200 );

  QList<KUser> user_list = KUser::allUsers();
  QStringList uids;

  for ( int i = 0; i < user_list.size(); ++i )
  {
    uids.append( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                     .arg( user_list.at( i ).uid() ) );
  }

  uids.sort();

  m_uid_input->addItems( uids );
  m_uid_input->setWhatsThis( Smb4KSettings::self()->userIDItem()->whatsThis() );
//   m_uid_input->setToolTip( i18n( "The user ID" ) );

  // Group ID
  QLabel *gid_label = new QLabel( Smb4KSettings::self()->groupIDItem()->label(), main_widget );
  m_gid_input = new KComboBox( main_widget );
  m_gid_input->setMinimumWidth( 200 );

  QList<KUserGroup> group_list = KUserGroup::allGroups();
  QStringList gids;

  for ( int i = 0; i < group_list.size(); ++i )
  {
    gids.append( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                     .arg( group_list.at( i ).gid() ) );
  }

  gids.sort();

  m_gid_input->addItems( gids );
  m_gid_input->setWhatsThis( Smb4KSettings::self()->groupIDItem()->whatsThis() );
//   m_gid_input->setToolTip( i18n( "The group ID" ) );
  
  // Put everything in the layout.
  layout->addWidget( unc_label, 0, 0, 0 );
  layout->addWidget( unc, 0, 1, 0 );
#ifndef Q_OS_FREEBSD
  layout->addWidget( fs_port_label, 1, 0, 0 );
  layout->addWidget( m_fs_port_input, 1, 1, 0 );
  layout->addWidget( permission_label, 2, 0, 0 );
  layout->addWidget( m_rw_input, 2, 1, 0 );
  layout->addWidget( uid_label, 3, 0, 0 );
  layout->addWidget( m_uid_input, 3, 1, 0 );
  layout->addWidget( gid_label, 4, 0, 0 );
  layout->addWidget( m_gid_input, 4, 1, 0 );
#else
  layout->addWidget( smb_port_label, 1, 0, 0 );
  layout->addWidget( m_smb_port_input, 1, 1, 0 );
  layout->addWidget( uid_label, 2, 0, 0 );
  layout->addWidget( m_uid_input, 2, 1, 0 );
  layout->addWidget( gid_label, 3, 0, 0 );
  layout->addWidget( m_gid_input, 3, 1, 0 );
#endif

  // Get the custom options.
  Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_share );
  
  // Set the port.
#ifndef Q_OS_FREEBSD
  if ( info && info->fileSystemPort() != -1 )
  {
    m_fs_port_input->setValue( info->fileSystemPort() );
  }
  else
  {
    m_fs_port_input->setValue( Smb4KSettings::remoteFileSystemPort() );
  }
#else
  if ( info && info->smbPort() != -1 )
  {
    m_smb_port_input->setValue( info->smbPort() );
  }
  else
  {
    m_smb_port_input->setValue( Smb4KSettings::remoteSMBPort() );
  }
#endif

#ifndef Q_OS_FREEBSD
  // Set the write access.
  if ( info && info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
  {
    switch( info->writeAccess() )
    {
      case Smb4KSambaOptionsInfo::ReadWrite:
      {
        m_rw_input->setCurrentItem( i18n( "read-write" ), false );
        break;
      }
      case Smb4KSambaOptionsInfo::ReadOnly:
      {
        m_rw_input->setCurrentItem( i18n( "read-only" ), false );
        break;
      }
      case Smb4KSambaOptionsInfo::UndefinedWriteAccess:
      default:
      {
        break;
      }
    }        
  }
  else
  {
    switch ( Smb4KSettings::writeAccess() )
    {
      case Smb4KSettings::EnumWriteAccess::ReadWrite:
      {
        m_rw_input->setCurrentItem( i18n( "read-write" ), false );
        break;
      }
      case Smb4KSettings::EnumWriteAccess::ReadOnly:
      {
        m_rw_input->setCurrentItem( i18n( "read-only" ), false );
        break;
      }
      default:
      {
        break;
      }
    }
  }
#endif

  // Set the user ID.
  QString user_text;
      
  if ( info && info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
  {
    KUser user( (K_UID)info->uid() );
    user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
  }
  else
  {
    KUser user( (K_UID)Smb4KSettings::userID().toInt() );
    user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
  }
      
  int user_index = m_uid_input->findText( user_text );
  m_uid_input->setCurrentIndex( user_index );

  // Set the GID.
  QString group_text;
      
  if ( info && info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
  {
    KUserGroup group( (K_GID)info->gid() );
    group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
  }
  else
  {
    KUserGroup group( (K_GID)Smb4KSettings::groupID().toInt() );
    group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
  }
      
  int group_index = m_gid_input->findText( group_text );
  m_gid_input->setCurrentIndex( group_index );

  // Connections
#ifndef Q_OS_FREEBSD
  connect( m_fs_port_input, SIGNAL( valueChanged( int ) ),
           this, SLOT( slotFileSystemPortChanged( int ) ) );

  connect( m_rw_input, SIGNAL( activated( const QString & ) ),
           this, SLOT( slotWriteAccessChanged( const QString & ) ) );
#else
  connect( m_smb_port_input, SIGNAL( valueChanged( int ) ),
           this, SLOT( slotSMBPortChanged( int ) ) );
#endif

  connect( m_uid_input, SIGNAL( activated( const QString & ) ),
           this, SLOT( slotUIDChanged( const QString & ) ) );

  connect( m_gid_input, SIGNAL( activated( const QString & ) ),
           this, SLOT( slotGIDChanged( const QString & ) ) );
  
  connect( this, SIGNAL( okClicked() ),
           this, SLOT( slotOKButtonClicked() ) );

  connect( this, SIGNAL( user1Clicked() ),
           this, SLOT( slotDefaultButtonClicked() ) );

  // Enable the buttons.
  enableButton( Ok, false );
  enableButton( User1, !hasDefaultSettings() );
}


bool Smb4KCustomOptionsDialog::hasDefaultSettings()
{
  switch ( m_type )
  {
    case Host:
    {
      // SMB port
      if ( m_smb_port_input->value() != Smb4KSettings::remoteSMBPort() )
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
#ifndef Q_OS_FREEBSD
      // File system port
      if ( m_fs_port_input->value() != Smb4KSettings::remoteFileSystemPort() )
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      // Write access
      switch ( Smb4KSettings::writeAccess() )
      {
        case Smb4KSettings::EnumWriteAccess::ReadWrite:
        {
          if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumWriteAccess::ReadOnly:
        {
          if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        default:
        {
          break;
        }
      }
#endif
      
      // Protocol
      switch ( Smb4KSettings::protocolHint() )
      {
        case Smb4KSettings::EnumProtocolHint::Automatic:
        {
          if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RPC:
        {
          if ( QString::compare( m_proto_input->currentText(), "RPC", Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RAP:
        {
          if ( QString::compare( m_proto_input->currentText(), "RAP", Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumProtocolHint::ADS:
        {
          if ( QString::compare( m_proto_input->currentText(), "ADS", Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        default:
        {
          break;
        }
      }
      
      // Kerberos
      if ( m_kerberos->isChecked() != Smb4KSettings::useKerberos() )
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      // User ID
      if ( (K_UID)Smb4KSettings::userID().toInt() != (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() )
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      // Group ID
      if ( (K_GID)Smb4KSettings::groupID().toInt() != (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() )
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      break;
    }
    case Share:
    {
      // Port
#ifndef Q_OS_FREEBSD
      if ( m_fs_port_input->value() != Smb4KSettings::remoteFileSystemPort() )
#else
      if ( m_smb_port_input->value() != Smb4KSettings::remoteSMBPort() )
#endif
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      // User ID
      if ( (K_UID)Smb4KSettings::userID().toInt() != (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() )
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      // Group ID
      if ( (K_GID)Smb4KSettings::groupID().toInt() != (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() )
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
#ifndef Q_OS_FREEBSD
      // Write access
      switch ( Smb4KSettings::writeAccess() )
      {
        case Smb4KSettings::EnumWriteAccess::ReadWrite:
        {
          if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumWriteAccess::ReadOnly:
        {
          if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
          {
            return false;
          }
          else
          {
            // Do nothing
          }
          break;
        }
        default:
        {
          break;
        }
      }
#endif
      
      break;
    }
    default:
    {
      break;
    }
  }
  
  return true;
}


bool Smb4KCustomOptionsDialog::hasInitialSettings()
{
  switch ( m_type )
  {
    case Host:
    {
      Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_host );
      
      // SMB port
      if ( info && info->smbPort() != -1 )
      {
        if ( m_smb_port_input->value() != info->smbPort() )
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
        if ( m_smb_port_input->value() != Smb4KSettings::remoteSMBPort() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }

#ifndef Q_OS_FREEBSD
      // File system port
      if ( info && info->fileSystemPort() != -1 )
      {
        if ( m_fs_port_input->value() != info->fileSystemPort() )
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
        if ( m_fs_port_input->value() != Smb4KSettings::remoteFileSystemPort() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }

      // Write access
      if ( info && info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
      {
        switch( info->writeAccess() )
        {
          case Smb4KSambaOptionsInfo::ReadWrite:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::ReadOnly:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
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
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
#endif
      
      // Protocol.
      if ( info && info->protocol() != Smb4KSambaOptionsInfo::UndefinedProtocol )
      {
        switch ( info->protocol() )
        {
          case Smb4KSambaOptionsInfo::Automatic:
          {
            if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::RPC:
          {
            if ( QString::compare( m_proto_input->currentText(), "RPC", Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::RAP:
          {
            if ( QString::compare( m_proto_input->currentText(), "RAP", Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::ADS:
          {
            if ( QString::compare( m_proto_input->currentText(), "ADS", Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
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
        switch ( Smb4KSettings::protocolHint() )
        {
          case Smb4KSettings::EnumProtocolHint::Automatic:
          {
            if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RPC:
          {
            if ( QString::compare( m_proto_input->currentText(), "RPC", Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RAP:
          {
            if ( QString::compare( m_proto_input->currentText(), "RAP", Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumProtocolHint::ADS:
          {
            if ( QString::compare( m_proto_input->currentText(), "ADS", Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
      
      // Kerberos usage
      if ( info && info->useKerberos() != Smb4KSambaOptionsInfo::UndefinedKerberos )
      {
        switch ( info->useKerberos() )
        {
          case Smb4KSambaOptionsInfo::UseKerberos:
          {
            if ( !m_kerberos->isChecked() )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::NoKerberos:
          {
            if ( m_kerberos->isChecked() )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
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
        if ( m_kerberos->isChecked() != Smb4KSettings::useKerberos() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }
      
      // User ID
      if ( info && info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
      {
        if ( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != info->uid() )
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
        if ( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_UID)Smb4KSettings::userID().toInt() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }
      
      // Group ID
      if ( info && info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
      {
        if ( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_GID)info->gid() )
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
        if ( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_GID)Smb4KSettings::groupID().toInt() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }
      
      break;
    }
    case Share:
    {
      Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_share );
      
#ifndef Q_OS_FREEBSD
      if ( info && info->fileSystemPort() != -1 )
      {
        if ( m_fs_port_input->value() != info->fileSystemPort() )
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
        if ( m_fs_port_input->value() != Smb4KSettings::remoteFileSystemPort() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }
#else
      if ( info && info->smbPort() != -1 )
      {
        if ( m_smb_port_input->value() != info->smbPort() )
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
        if ( m_smb_port_input->value() != Smb4KSettings::smbPort() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }
#endif
      
      // User ID
      if ( info && info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
      {
        if ( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != info->uid() )
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
        if ( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_UID)Smb4KSettings::userID().toInt() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }
      
      // Group ID
      if ( info && info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
      {
        if ( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_GID)info->gid() )
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
        if ( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_GID)Smb4KSettings::groupID().toInt() )
        {
          return false;
        }
        else
        {
          // Do nothing
        }
      }
      
#ifndef Q_OS_FREEBSD
      // Write access
      if ( info && info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
      {
        switch( info->writeAccess() )
        {
          case Smb4KSambaOptionsInfo::ReadWrite:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::ReadOnly:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
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
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
            {
              return false;
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
#endif      

      break;
    }
    default:
    {
      break;
    }
  }
  
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomOptionsDialog::slotSMBPortChanged( int /*val*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotFileSystemPortChanged( int /*val*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotProtocolChanged( const QString &/*protocol*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotKerberosToggled( bool /*on*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotWriteAccessChanged( const QString &/*rw*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotUIDChanged( const QString &/*u*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotGIDChanged( const QString &/*g*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotOKButtonClicked()
{
  if ( hasDefaultSettings() )
  {
    // Remove the item, since it uses the default settings.
    switch ( m_type )
    {
      case Host:
      {
        Smb4KSambaOptionsHandler::self()->removeItem( m_host, true );
        break;
      }
      case Share:
      {
        Smb4KSambaOptionsHandler::self()->removeItem( m_share, true );
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
    // Save the item with the changed settings.
    switch ( m_type )
    {
      case Host:
      {
        Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_host );
        
        if ( !info )
        {
          // Create a new object
          info = new Smb4KSambaOptionsInfo( m_host );
        }
        else
        {
          // Do nothing
        }
        
        // Put in the information.
        info->setSMBPort( m_smb_port_input->value() );
#ifndef Q_OS_FREEBSD
        info->setFileSystemPort( m_fs_port_input->value() );
        
        if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadWrite );
        }
        else if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadOnly );
        }
        else
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::UndefinedWriteAccess );
        }        
#endif

        if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::Automatic );
        }
        else if ( QString::compare( m_proto_input->currentText(), "rpc", Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::RPC );
        }
        else if ( QString::compare( m_proto_input->currentText(), "rap", Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::RAP );
        }
        else if ( QString::compare( m_proto_input->currentText(), "ads", Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::ADS );
        }
        else
        {
          info->setProtocol( Smb4KSambaOptionsInfo::UndefinedProtocol );
        }

        if ( m_kerberos->isChecked() )
        {
          info->setUseKerberos( Smb4KSambaOptionsInfo::UseKerberos );
        }
        else
        {
          info->setUseKerberos( Smb4KSambaOptionsInfo::NoKerberos );
        }
        
        info->setUID( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        info->setGID( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        
        // Add the item.
        Smb4KSambaOptionsHandler::self()->addItem( info, true );
          
        break;
      }
      case Share:
      {
        Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_share, true );
        
        if ( !info )
        {
          // Create a new object
          info = new Smb4KSambaOptionsInfo( m_share );
        }
        else
        {
          // Do nothing
        }
        
        // Put in the information.
#ifndef Q_OS_FREEBSD
        info->setFileSystemPort( m_fs_port_input->value() );
#else
        info->setSMBPort( m_smb_port_input->value() );
#endif
        info->setUID( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        info->setGID( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        
#ifndef Q_OS_FREEBSD
        if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadWrite );
        }
        else if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadOnly );
        }
        else
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::UndefinedWriteAccess );
        }
#endif

        // Add the item.
        Smb4KSambaOptionsHandler::self()->addItem( info, true );
        
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
}


void Smb4KCustomOptionsDialog::slotDefaultButtonClicked()
{
  switch ( m_type )
  {
    case Host:
    {
      m_smb_port_input->setValue( Smb4KSettings::remoteSMBPort() );
#ifndef Q_OS_FREEBSD
      m_fs_port_input->setValue( Smb4KSettings::remoteFileSystemPort() );
      
      switch ( Smb4KSettings::writeAccess() )
      {
        case Smb4KSettings::EnumWriteAccess::ReadWrite:
        {
          m_rw_input->setCurrentItem( i18n( "read-write" ), false );
          break;
        }
        case Smb4KSettings::EnumWriteAccess::ReadOnly:
        {
          m_rw_input->setCurrentItem( i18n( "read-only" ), false );
          break;
        }
        default:
        {
          break;
        }
      }
#endif
      m_kerberos->setChecked( Smb4KSettings::useKerberos() );
      
      switch ( Smb4KSettings::protocolHint() )
      {
        case Smb4KSettings::EnumProtocolHint::Automatic:
        {
          m_proto_input->setCurrentItem( i18n( "automatic" ), false );
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RPC:
        {
          m_proto_input->setCurrentItem( "RPC", false );
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RAP:
        {
          m_proto_input->setCurrentItem( "RAP", false );
          break;
        }
        case Smb4KSettings::EnumProtocolHint::ADS:
        {
          m_proto_input->setCurrentItem( "ADS", false );
          break;
        }
        default:
        {
          break;
        }
      }
      
      KUser user( (K_UID)Smb4KSettings::userID().toInt() );
      QString user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
      int user_index = m_uid_input->findText( user_text );
      m_uid_input->setCurrentIndex( user_index );

      KUserGroup group( (K_GID)Smb4KSettings::groupID().toInt() );
      QString group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
      int group_index = m_gid_input->findText( group_text );
      m_gid_input->setCurrentIndex( group_index );
      
      break;
    }
    case Share:
    {
#ifndef Q_OS_FREEBSD
      m_fs_port_input->setValue( Smb4KSettings::remoteFileSystemPort() );
#else
      m_smb_port_input->setValue( Smb4KSettings::remoteSMBPort() );
#endif

      KUser user( (K_UID)Smb4KSettings::userID().toInt() );
      QString user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
      int user_index = m_uid_input->findText( user_text );
      m_uid_input->setCurrentIndex( user_index );

      KUserGroup group( (K_GID)Smb4KSettings::groupID().toInt() );
      QString group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
      int group_index = m_gid_input->findText( group_text );
      m_gid_input->setCurrentIndex( group_index );

#ifndef Q_OS_FREEBSD
      switch ( Smb4KSettings::writeAccess() )
      {
        case Smb4KSettings::EnumWriteAccess::ReadWrite:
        {
          m_rw_input->setCurrentItem( i18n( "read-write" ), false );
          break;
        }
        case Smb4KSettings::EnumWriteAccess::ReadOnly:
        {
          m_rw_input->setCurrentItem( i18n( "read-only" ), false );
          break;
        }
        default:
        {
          break;
        }
      }
#endif

      break;
    }
    default:
    {
      break;
    }
  }

  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}

#include "smb4kcustomoptionsdialog.moc"
