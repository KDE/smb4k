/***************************************************************************
    smb4kcustomoptionsdialog  -  With this dialog the user can define
    custom Samba options for hosts or shares.
                             -------------------
    begin                : So Jun 25 2006
    copyright            : (C) 2006-2008 by Alexander Reinholdt
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


// FIXME: Maybe introduce a private class here?
static int default_port = -1;
static QString default_protocol = QString();
static bool default_kerberos = false;
static QString default_uid = QString();
static QString default_gid = QString();
#ifndef __FreeBSD__
static bool default_readwrite = true;
#endif

static int port_value = -1;
static QString protocol_value = QString();
static bool kerberos_value = false;
static QString uid_value = QString();
static QString gid_value = QString();
#ifndef __FreeBSD__
static bool readwrite_value = true;
#endif

static bool port_changed_ok = false;
static bool protocol_changed_ok = false;
static bool kerberos_changed_ok = false;
static bool uid_changed_ok = false;
static bool gid_changed_ok = false;
#ifndef __FreeBSD__
static bool readwrite_changed_ok = false;
#endif

static bool port_changed_default = false;
static bool protocol_changed_default = false;
static bool kerberos_changed_default = false;
static bool uid_changed_default = false;
static bool gid_changed_default = false;
#ifndef __FreeBSD__
static bool readwrite_changed_default = false;
#endif


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog( Smb4KHost *host, QWidget *parent )
: KDialog( parent ), m_type( Host ), m_host( host ), m_share( NULL )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Custom Options" ) );
  setButtons( User1|Ok|Cancel );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KStandardGuiItem::defaults() );

  m_initialized = true;

  setupDialog();

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

  if ( QString::compare( m_share->shareName(), "homes" ) != 0 )
  {
    m_initialized = true;
  }
  else
  {
    // We do not use parent as parent for the "Specify User"
    // dialog, so that the behavior is uniform.
    QWidget *p = 0;

    if ( kapp )
    {
      p = kapp->activeWindow();
    }

    (void) Smb4KHomesSharesHandler::self()->specifyUser( m_share, p );
    m_initialized = (QString::compare( m_share->shareName(), "homes" ) != 0);
  }

  setupDialog();

  setMinimumSize( (sizeHint().width() > 350 ? sizeHint().width() : 350), sizeHint().height() );

  setInitialSize( QSize( minimumWidth(), minimumHeight() ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  restoreDialogSize( group );
}


Smb4KCustomOptionsDialog::~Smb4KCustomOptionsDialog()
{
}


void Smb4KCustomOptionsDialog::setupDialog()
{
  // The Smb4KSambaOptionsInfo object:
  Smb4KSambaOptionsInfo *info = NULL;

  // We need this later to decide if the "Default"
  // button needs to be enabled:
  bool enable_default_button = false;

  // These are the input widgets we need below:
  m_port_input = NULL;
  m_kerberos = NULL;
  m_proto_input = NULL;
  m_uid_input = NULL;
  m_gid_input = NULL;
#ifndef __FreeBSD__
  m_rw_input = NULL;
#endif

  // Set up the widget:
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *location_label = new QLabel( m_type == Host ? i18n( "Host:" ) : i18n( "Share:" ), main_widget );
  KLineEdit *location    = new KLineEdit( m_type == Host ?
                           m_host->hostName() :
                           m_share->unc(),
                           main_widget );
  location->setReadOnly( true );

  // The widgets will be put into the layout below.

  // Here comes the item-dependent stuff:
  switch ( m_type )
  {
    case Host:
    {
      QLabel *port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(), main_widget );
      m_port_input = new KIntNumInput( -1, main_widget );
      m_port_input->setMinimumWidth( 200 );
      m_port_input->setMinimum( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt() );
      m_port_input->setMaximum( Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );

      QLabel *protocol_label = new QLabel( i18n( "Protocol:" ), main_widget );
      m_proto_input = new KComboBox( false, main_widget );
      m_proto_input->setMinimumWidth( 200 );

      QStringList protocol_items;
      protocol_items.append( i18n( "automatic" ) );
      protocol_items.append( "RPC" );
      protocol_items.append( "RAP" );
      protocol_items.append( "ADS" );

      m_proto_input->insertItems( 0, protocol_items );

      m_kerberos = new QCheckBox( i18n( "Try to authenticate with Kerberos (Active Directory)" ), main_widget );

      layout->addWidget( location_label, 0, 0, 0 );
      layout->addWidget( location, 0, 1, 0 );
      layout->addWidget( port_label, 1, 0, 0 );
      layout->addWidget( m_port_input, 1, 1, 0 );
      layout->addWidget( protocol_label, 2, 0, 0 );
      layout->addWidget( m_proto_input, 2, 1, 0 );
      layout->addWidget( m_kerberos, 3, 0, 1, 2, 0 );

      info = Smb4KSambaOptionsHandler::self()->findItem( m_host );

      // Get the default values from the config file:
      default_port = Smb4KSettings::remoteSMBPort();
      default_kerberos = Smb4KSettings::useKerberos();

      switch ( Smb4KSettings::protocolHint() )
      {
        case Smb4KSettings::EnumProtocolHint::Automatic:
        {
          // In this case the user leaves it to the net
          // command to determine the right protocol.
          default_protocol = "auto";

          break;
        }
        case Smb4KSettings::EnumProtocolHint::RPC:
        {
          default_protocol = "rpc";

          break;
        }
        case Smb4KSettings::EnumProtocolHint::RAP:
        {
          default_protocol = "rap";

          break;
        }
        case Smb4KSettings::EnumProtocolHint::ADS:
        {
          default_protocol = "ads";

          break;
        }
        default:
        {
          default_protocol = QString();

          break;
        }
      }

      // Define the values that have to be put into the widgets:
      port_value =     (info && info->port() != -1) ?
                       info->port() :
                       default_port;

      if ( info )
      {
        switch ( info->protocol() )
        {
          case Smb4KSambaOptionsInfo::Automatic:
          {
            protocol_value = "auto";

            break;
          }
          case Smb4KSambaOptionsInfo::RPC:
          {
            protocol_value = "rpc";

            break;
          }
          case Smb4KSambaOptionsInfo::RAP:
          {
            protocol_value = "rap";

            break;
          }
          case Smb4KSambaOptionsInfo::ADS:
          {
            protocol_value = "ads";

            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedProtocol:
          {
            protocol_value = default_protocol;

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
        protocol_value = default_protocol;
      }

      if ( info )
      {
        switch ( info->useKerberos() )
        {
          case Smb4KSambaOptionsInfo::UseKerberos:
          {
            kerberos_value = true;

            break;
          }
          case Smb4KSambaOptionsInfo::NoKerberos:
          {
            kerberos_value = false;

            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedKerberos:
          {
            kerberos_value = default_kerberos;

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
        kerberos_value = default_kerberos;
      }

      // Put the values in the widgets:
      m_port_input->setValue( port_value );
      m_proto_input->setCurrentItem( (QString::compare( protocol_value, "auto" ) == 0 ? i18n( "automatic" ) : protocol_value.toUpper()), false );
      m_kerberos->setChecked( kerberos_value );

      // Does the 'Default' button need to be enabled?
      if ( default_port != port_value ||
           QString::compare( default_protocol, protocol_value ) != 0 ||
           default_kerberos != kerberos_value )
      {
        enable_default_button = true;
      }

      // Connections:
      connect( m_port_input,  SIGNAL( valueChanged( int ) ),
               this,          SLOT( slotPortChanged( int ) ) );

      connect( m_kerberos,    SIGNAL( toggled( bool ) ),
               this,          SLOT( slotKerberosToggled( bool ) ) );

      connect( m_proto_input, SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotProtocolChanged( const QString & ) ) );

      break;
    }
    case Share:
    {
#ifndef __FreeBSD__
      QLabel *port_label = new QLabel( Smb4KSettings::self()->remoteFileSystemPortItem()->label(), main_widget );
      m_port_input = new KIntNumInput( -1, main_widget );
      m_port_input->setMinimumWidth( 200 );
      m_port_input->setMinimum( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt() );
      m_port_input->setMaximum( Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );

      QLabel *permission_label = new QLabel( i18n( "Write access:" ), main_widget );
      m_rw_input = new KComboBox( false, main_widget );
      m_rw_input->setMinimumWidth( 200 );

      QStringList write_access_entries;
      write_access_entries.append( i18n( "read-write" ) );
      write_access_entries.append( i18n( "read-only" ) );

      m_rw_input->insertItems( 0, write_access_entries );

      QLabel *uid_label = new QLabel( i18n( "User ID:" ), main_widget );
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

      QLabel *gid_label = new QLabel( i18n( "Group ID:" ), main_widget );
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

      layout->addWidget( location_label, 0, 0, 0 );
      layout->addWidget( location, 0, 1, 0 );
      layout->addWidget( port_label, 1, 0, 0 );
      layout->addWidget( m_port_input, 1, 1, 0 );
      layout->addWidget( uid_label, 2, 0, 0 );
      layout->addWidget( m_uid_input, 2, 1, 0 );
      layout->addWidget( gid_label, 3, 0, 0 );
      layout->addWidget( m_gid_input, 3, 1, 0 );
      layout->addWidget( permission_label, 4, 0, 0 );
      layout->addWidget( m_rw_input, 4, 1, 0 );
#else
      QLabel *port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(), main_widget );
      m_port_input = new KIntNumInput( -1, main_widget );
      m_port_input->setMinimumWidth( 200 );
      m_port_input->setMinimum( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt() );
      m_port_input->setMaximum( Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );

      QLabel *uid_label = new QLabel( i18n( "User ID:" ), main_widget );
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

      QLabel *gid_label = new QLabel( i18n( "Group ID:" ), main_widget );
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

      layout->addWidget( location_label, 0, 0, 0 );
      layout->addWidget( location, 0, 1, 0 );
      layout->addWidget( port_label, 1, 0, 0 );
      layout->addWidget( m_port_input, 1, 1, 0 );
      layout->addWidget( uid_label, 2, 0, 0 );
      layout->addWidget( m_uid_input, 2, 1, 0 );
      layout->addWidget( gid_label, 3, 0, 0 );
      layout->addWidget( m_gid_input, 3, 1, 0 );
#endif

      info = Smb4KSambaOptionsHandler::self()->findItem( m_share );

      // Get the default values from the config file:
#ifndef __FreeBSD__
      default_port = Smb4KSettings::remoteFileSystemPort();
#else
      default_port = Smb4KSettings::remoteSMBPort();
#endif
      default_uid = Smb4KSettings::userID();
      default_gid = Smb4KSettings::groupID();
#ifndef __FreeBSD__
      switch ( Smb4KSettings::writeAccess() )
      {
        case Smb4KSettings::EnumWriteAccess::ReadWrite:
        {
          default_readwrite = true;

          break;
        }
        case Smb4KSettings::EnumWriteAccess::ReadOnly:
        {
          default_readwrite = false;

          break;
        }
        default:
        {
          break;
        }
      }
#endif

      // Define the values that have to be put into the widgets:
      port_value = (info && info->port() != -1) ?
                   info->port() :
                   default_port;

      uid_value = (info && info->uidIsSet()) ?
                  QString( "%1" ).arg( info->uid() ) :
                  default_uid;

      gid_value = (info && info->gidIsSet()) ?
                  QString( "%1" ).arg( info->gid() ) :
                  default_gid;
#ifndef __FreeBSD__
      if ( info )
      {
        switch( info->writeAccess() )
        {
          case Smb4KSambaOptionsInfo::ReadWrite:
          {
            readwrite_value = true;

            break;
          }
          case Smb4KSambaOptionsInfo::ReadOnly:
          {
            readwrite_value = false;

            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedWriteAccess:
          {
            readwrite_value = default_readwrite;

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
        readwrite_value = default_readwrite;
      }
#endif

      // Put the values in the widgets:
      m_port_input->setValue( port_value );

      KUser user( (K_UID)uid_value.toInt() );
      QString user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
      int user_index = m_uid_input->findText( user_text );
      m_uid_input->setCurrentIndex( user_index );

      KUserGroup group( (K_GID)gid_value.toInt() );
      QString group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
      int group_index = m_gid_input->findText( group_text );
      m_gid_input->setCurrentIndex( group_index );
#ifndef __FreeBSD__
      m_rw_input->setCurrentItem( (readwrite_value ?
                                  i18n( "read-write" ) :
                                  i18n( "read-only" )),
                                  false );
#endif

      // Does the 'Default' button need to be enabled?
      if ( default_port != port_value ||
#ifndef __FreeBSD__
           default_readwrite != readwrite_value ||
#endif
           QString::compare( default_uid, uid_value ) != 0 ||
           QString::compare( default_gid, gid_value ) != 0 )
      {
        enable_default_button = true;
      }

      // Connections:
      connect( m_port_input,  SIGNAL( valueChanged( int ) ),
               this,          SLOT( slotPortChanged( int ) ) );

      connect( m_uid_input,   SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotUIDChanged( const QString & ) ) );

      connect( m_gid_input,   SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotGIDChanged( const QString & ) ) );

#ifndef __FreeBSD__
      connect( m_rw_input,    SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotWriteAccessChanged( const QString & ) ) );
#endif

      break;
    }
    default:
    {
      // This should not happen...
      break;
    }
  }

  // Enable the buttons:
  enableButton( Ok, false );
  enableButton( User1, enable_default_button );

  // Connect the buttons:
  connect( this,          SIGNAL( okClicked() ),
           this,          SLOT( slotOKButtonClicked() ) );

  connect( this,          SIGNAL( user1Clicked() ),
           this,          SLOT( slotDefaultButtonClicked() ) );
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomOptionsDialog::slotPortChanged( int val )
{
  port_changed_ok = (port_value != val);
  port_changed_default = (default_port != val);

  switch ( m_type )
  {
    case Host:
    {
      enableButton( Ok, port_changed_ok ||
                        protocol_changed_ok ||
                        kerberos_changed_ok );

      enableButton( User1, port_changed_default ||
                           protocol_changed_default ||
                           kerberos_changed_default );

      break;
    }
    case Share:
    {
      enableButton( Ok, port_changed_ok ||
#ifndef __FreeBSD__
                        readwrite_changed_ok ||
#endif
                        uid_changed_ok ||
                        gid_changed_ok );

      enableButton( User1, port_changed_default ||
#ifndef __FreeBSD__
                           readwrite_changed_default ||
#endif
                           uid_changed_default ||
                           gid_changed_default );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KCustomOptionsDialog::slotProtocolChanged( const QString &protocol )
{
  protocol_changed_ok = (QString::compare( protocol_value, protocol.toLower() ) != 0);
  protocol_changed_default = (QString::compare( default_protocol, protocol.toLower() ) != 0);

  switch ( m_type )
  {
    case Host:
    {
      enableButton( Ok, port_changed_ok ||
                        protocol_changed_ok ||
                        kerberos_changed_ok );

      enableButton( User1, port_changed_default ||
                           protocol_changed_default ||
                           kerberos_changed_default );

      break;
    }
    case Share:
    {
      enableButton( Ok, port_changed_ok ||
#ifndef __FreeBSD__
                        readwrite_changed_ok ||
#endif
                        uid_changed_ok ||
                        gid_changed_ok );

      enableButton( User1, port_changed_default ||
#ifndef __FreeBSD__
                           readwrite_changed_default ||
#endif
                           uid_changed_default ||
                           gid_changed_default );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KCustomOptionsDialog::slotKerberosToggled( bool on )
{
  kerberos_changed_ok = (kerberos_value != on);
  kerberos_changed_default = (default_kerberos != on);

  switch ( m_type )
  {
    case Host:
    {
      enableButton( Ok, port_changed_ok ||
                        protocol_changed_ok ||
                        kerberos_changed_ok );

      enableButton( User1, port_changed_default ||
                           protocol_changed_default ||
                           kerberos_changed_default );

      break;
    }
    case Share:
    {
      enableButton( Ok, port_changed_ok ||
#ifndef __FreeBSD__
                        readwrite_changed_ok ||
#endif
                        uid_changed_ok ||
                        gid_changed_ok );

      enableButton( User1, port_changed_default ||
#ifndef __FreeBSD__
                           readwrite_changed_default ||
#endif
                           uid_changed_default ||
                           gid_changed_default );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KCustomOptionsDialog::slotWriteAccessChanged( const QString &rw )
{
#ifndef __FreeBSD__
  bool readwrite = (QString::compare( rw, i18n( "read-write" ) ) == 0);
  readwrite_changed_ok = (readwrite_value != readwrite);
  readwrite_changed_default = (default_readwrite != readwrite);

  switch ( m_type )
  {
    case Host:
    {
      enableButton( Ok, port_changed_ok ||
                        protocol_changed_ok ||
                        kerberos_changed_ok );

      enableButton( User1, port_changed_default ||
                           protocol_changed_default ||
                           kerberos_changed_default );

      break;
    }
    case Share:
    {
      enableButton( Ok, port_changed_ok ||
                        readwrite_changed_ok ||
                        uid_changed_ok ||
                        gid_changed_ok );

      enableButton( User1, port_changed_default ||
                           readwrite_changed_default ||
                           uid_changed_default ||
                           gid_changed_default );
      break;
    }
    default:
    {
      break;
    }
  }
#endif
}


void Smb4KCustomOptionsDialog::slotUIDChanged( const QString &u )
{
  QString uid = u.section( "(", 1, 1 ).section( ")", 0, 0 );

  uid_changed_ok = (QString::compare( uid_value, uid ) != 0);
  uid_changed_default = (QString::compare( default_uid, uid ) != 0);

  switch ( m_type )
  {
    case Host:
    {
      enableButton( Ok, port_changed_ok ||
                        protocol_changed_ok ||
                        kerberos_changed_ok );

      enableButton( User1, port_changed_default ||
                           protocol_changed_default ||
                           kerberos_changed_default );

      break;
    }
    case Share:
    {
      enableButton( Ok, port_changed_ok ||
#ifndef __FreeBSD__
                        readwrite_changed_ok ||
#endif
                        uid_changed_ok ||
                        gid_changed_ok );

      enableButton( User1, port_changed_default ||
#ifndef __FreeBSD__
                           readwrite_changed_default ||
#endif
                           uid_changed_default ||
                           gid_changed_default );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KCustomOptionsDialog::slotGIDChanged( const QString &g )
{
  QString gid = g.section( "(", 1, 1 ).arg( ")", 0, 0 );

  gid_changed_ok = (QString::compare( gid_value, gid ) != 0);
  gid_changed_default = (QString::compare( default_gid, gid ) != 0);

  switch ( m_type )
  {
    case Host:
    {
      enableButton( Ok, port_changed_ok ||
                        protocol_changed_ok ||
                        kerberos_changed_ok );

      enableButton( User1, port_changed_default ||
                           protocol_changed_default ||
                           kerberos_changed_default );

      break;
    }
    case Share:
    {
      enableButton( Ok, port_changed_ok ||
#ifndef __FreeBSD__
                        readwrite_changed_ok ||
#endif
                        uid_changed_ok ||
                        gid_changed_ok );

      enableButton( User1, port_changed_default ||
#ifndef __FreeBSD__
                           readwrite_changed_default ||
#endif
                           uid_changed_default ||
                           gid_changed_default );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KCustomOptionsDialog::slotOKButtonClicked()
{
  Smb4KSambaOptionsInfo *info = NULL;

  switch ( m_type )
  {
    case Host:
    {
      // Check if we can remove the item:
      if ( !port_changed_default && !protocol_changed_default && !kerberos_changed_default )
      {
        Smb4KSambaOptionsHandler::self()->removeItem( m_host, true );
      }
      else
      {
        // First search for the item in the custom options list
        // and create a new one only if the info could not be
        // found:
        if ( !(info = Smb4KSambaOptionsHandler::self()->findItem( m_host, true )) )
        {
          info = new Smb4KSambaOptionsInfo();
        }

        // Put in the needed information:
        info->setUNC( m_host->unc() );
        info->setPort( m_port_input->value() );
        info->setWorkgroupName( m_host->workgroupName() );
        info->setIP( m_host->ip() );

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

        // Add the new item.
        Smb4KSambaOptionsHandler::self()->addItem( info, true );
      }

      break;
    }
    case Share:
    {
#ifndef __FreeBSD__
      // Check if we can remove the item:
      if ( !port_changed_default && !readwrite_changed_default &&
           !uid_changed_default && !gid_changed_default )
      {
        Smb4KSambaOptionsHandler::self()->removeItem( m_share, true );
      }
      else
      {
        // First search for the item in the custom options list
        // and create a new one only if the info could not be
        // found:
        if ( !(info = Smb4KSambaOptionsHandler::self()->findItem( m_share, true )) )
        {
          info = new Smb4KSambaOptionsInfo();
        }

        // Put in the needed information:
        info->setUNC( m_share->unc() );
        info->setPort( m_port_input->value() );
        info->setWorkgroupName( m_share->workgroupName() );
        info->setIP( m_share->hostIP() );

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

        info->setUID( (uid_t)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        info->setGID( (gid_t)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );

        // Add the new item.
        Smb4KSambaOptionsHandler::self()->addItem( info, true );
      }
#else
      // Check if we can remove the item:
      if ( !port_changed_default && !uid_changed_default && !gid_changed_default )
      {
        Smb4KSambaOptionsHandler::self()->removeItem( m_share, true );
      }
      else
      {
        // First search for the item in the custom options list
        // and create a new one only if the info could not be
        // found:
        if ( !(info = Smb4KSambaOptionsHandler::self()->findItem( m_share, true )) )
        {
          info = new Smb4KSambaOptionsInfo();
        }

        // Put in the needed information:
        info->setUNC( m_share->unc() );
        info->setWorkgroupName( m_share->workgroupName() );
        info->setIP( m_share->hostIP() );
        info->setPort( m_port_input->value() );

        info->setUID( (uid_t)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        info->setGID( (gid_t)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );

        // Add the new item.
        Smb4KSambaOptionsHandler::self()->addItem( info, true );
      }
#endif

      break;
    }
    default:
    {
      break;
    }
  }

  KConfigGroup group( Smb4KSettings::self()->config(), "Dialogs" );
  saveDialogSize( group, KConfigGroup::Normal );
}


void Smb4KCustomOptionsDialog::slotDefaultButtonClicked()
{
  // Here, we only reset the dialog and enable the OK button
  // if necessary.

  switch ( m_type )
  {
    case Host:
    {
      m_port_input->setValue( default_port );
      m_kerberos->setChecked( default_kerberos );
      QString protocol = (QString::compare( default_protocol, "auto" ) == 0 ? i18n( "automatic" ) : protocol_value.toUpper());
      m_proto_input->setCurrentItem( protocol, false );

      // Enable or disable the OK button:
      enableButton( Ok, default_port != port_value ||
                        default_kerberos != kerberos_value ||
                        QString::compare( default_protocol, protocol_value ) != 0 );

      break;
    }
    case Share:
    {
      m_port_input->setValue( default_port );

      KUser user( (K_UID)default_uid.toInt() );
      QString user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
      int user_index = m_uid_input->findText( user_text );
      m_uid_input->setCurrentIndex( user_index );

      KUserGroup group( (K_GID)default_gid.toInt() );
      QString group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
      int group_index = m_gid_input->findText( group_text );
      m_gid_input->setCurrentIndex( group_index );
#ifndef __FreeBSD__
      QString write_access = (default_readwrite ? i18n( "read-write" ) : i18n( "read-only" ));
      m_rw_input->setCurrentItem( write_access, false );
#endif

      // Enable or disable the OK button:
      enableButton( Ok, default_port != port_value ||
#ifndef __FreeBSD__
                        default_readwrite != readwrite_value ||
#endif
                        QString::compare( default_uid, uid_value ) != 0 ||
                        QString::compare( default_gid, gid_value ) != 0 );

      break;
    }
    default:
    {
      break;
    }
  }

  // We just put the default values into the dialog.
  // Disable the 'Default' button:
  enableButton( User1, false );
}

#include "smb4kcustomoptionsdialog.moc"
