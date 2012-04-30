/***************************************************************************
    smb4ksambaoptions.cpp  -  This is the configuration page for the
    Samba settings of Smb4K
                             -------------------
    begin                : Mo Jan 26 2004
    copyright            : (C) 2004-2012 by Alexander Reinholdt
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
#include "smb4ksambaoptions.h"
#include "core/smb4kglobal.h"
#include "core/smb4ksettings.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QToolButton>

// KDE includes
#include <klocale.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <kuser.h>
#include <kmenu.h>
#include <kicon.h>

// System includes
#include <unistd.h>
#include <sys/types.h>

using namespace Smb4KGlobal;


Smb4KSambaOptions::Smb4KSambaOptions( QWidget *parent ) : KTabWidget( parent )
{
  m_collection = new KActionCollection( this );
  m_maybe_changed = false;
  m_removed = false;
  m_current_options = NULL;

  //
  // General
  //
  QWidget *general_tab          = new QWidget( this );

  QVBoxLayout *general_layout   = new QVBoxLayout( general_tab );
  general_layout->setSpacing( 5 );
  general_layout->setMargin( 0 );

  // General options
  QGroupBox *general_box        = new QGroupBox( i18n( "General Options" ), general_tab );

  QGridLayout *gen_opt_layout   = new QGridLayout( general_box );
  gen_opt_layout->setSpacing( 5 );

  QLabel *netbios_name_label    = new QLabel( Smb4KSettings::self()->netBIOSNameItem()->label(),
                                  general_box );

  KLineEdit *netbios_name       = new KLineEdit( general_box );
  netbios_name->setObjectName( "kcfg_NetBIOSName" );

  QLabel *domain_label          = new QLabel( Smb4KSettings::self()->domainNameItem()->label(),
                                  general_box );

  KLineEdit *domain             = new KLineEdit( general_box );
  domain->setObjectName( "kcfg_DomainName" );

  QLabel *socket_options_label  = new QLabel( Smb4KSettings::self()->socketOptionsItem()->label(),
                                  general_box );

  KLineEdit *socket_options     = new KLineEdit( general_box );
  socket_options->setObjectName( "kcfg_SocketOptions" );

  QLabel *netbios_scope_label   = new QLabel( Smb4KSettings::self()->netBIOSScopeItem()->label(),
                                  general_box );

  KLineEdit *netbios_scope      = new KLineEdit( general_box );
  netbios_scope->setObjectName( "kcfg_NetBIOSScope" );

  gen_opt_layout->addWidget( netbios_name_label, 0, 0, 0 );
  gen_opt_layout->addWidget( netbios_name, 0, 1, 0 );
  gen_opt_layout->addWidget( domain_label, 2, 0, 0 );
  gen_opt_layout->addWidget( domain, 2, 1, 0 );
  gen_opt_layout->addWidget( socket_options_label, 3, 0, 0 );
  gen_opt_layout->addWidget( socket_options, 3, 1, 0 );
  gen_opt_layout->addWidget( netbios_scope_label, 4, 0, 0 );
  gen_opt_layout->addWidget( netbios_scope, 4, 1, 0 );

  // General client options
  QGroupBox *ports_box          = new QGroupBox( i18n( "Remote Ports" ), general_tab );

  QGridLayout *ports_layout     = new QGridLayout( ports_box );
  ports_layout->setSpacing( 5 );

  QLabel *remote_smb_port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(),
                                  ports_box );

  KIntNumInput *remote_smb_port = new KIntNumInput( ports_box );
  remote_smb_port->setObjectName( "kcfg_RemoteSMBPort" );
  remote_smb_port->setSliderEnabled( true );

#ifndef Q_OS_FREEBSD
  QLabel *remote_fs_port_label  = new QLabel( Smb4KSettings::self()->remoteFileSystemPortItem()->label(),
                                  ports_box );

  KIntNumInput *remote_fs_port  = new KIntNumInput( ports_box );
  remote_fs_port->setObjectName( "kcfg_RemoteFileSystemPort" );
  remote_fs_port->setSliderEnabled( true );
#endif

  ports_layout->addWidget( remote_smb_port_label, 0, 0, 0 );
  ports_layout->addWidget( remote_smb_port, 0, 1, 0 );
#ifndef Q_OS_FREEBSD
  ports_layout->addWidget( remote_fs_port_label, 1, 0, 0 );
  ports_layout->addWidget( remote_fs_port, 1, 1, 0 );
#endif

  QGroupBox *auth_box           = new QGroupBox( i18n( "Authentication" ), general_tab );

  QGridLayout *auth_layout      = new QGridLayout( auth_box );
  auth_layout->setSpacing( 5 );

  QCheckBox *auth_kerberos      = new QCheckBox( Smb4KSettings::self()->useKerberosItem()->label(),
                                  auth_box );
  auth_kerberos->setObjectName( "kcfg_UseKerberos" );

  QCheckBox *auth_machine_acc   = new QCheckBox( Smb4KSettings::self()->machineAccountItem()->label(),
                                  auth_box );
  auth_machine_acc->setObjectName( "kcfg_MachineAccount" );

  QCheckBox *use_ccache         = new QCheckBox( Smb4KSettings::self()->useWinbindCCacheItem()->label(),
                                  auth_box );
  use_ccache->setObjectName( "kcfg_UseWinbindCCache" );

  auth_layout->addWidget( auth_kerberos, 0, 0, 0 );
  auth_layout->addWidget( auth_machine_acc, 0, 1, 0 );
  auth_layout->addWidget( use_ccache, 1, 0, 0 );

  QGroupBox *signing_box        = new QGroupBox( i18n( "Security" ), general_tab );

  QGridLayout *signing_layout   = new QGridLayout( signing_box );
  signing_layout->setSpacing( 5 );

  QLabel *signing_state_label   = new QLabel( Smb4KSettings::self()->signingStateItem()->label(),
                                  signing_box );
  KComboBox *signing_state      = new KComboBox( signing_box );
  signing_state->setObjectName( "kcfg_SigningState" );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::None,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::None ).label );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::On,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::On ).label );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::Off,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::Off ).label );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::Required,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::Required ).label );

  QCheckBox *encrypt_transport  = new QCheckBox( Smb4KSettings::self()->encryptSMBTransportItem()->label(),
                                  signing_box );
  encrypt_transport->setObjectName( "kcfg_EncryptSMBTransport" );

  signing_layout->addWidget( signing_state_label, 0, 0, 0 );
  signing_layout->addWidget( signing_state, 0, 1, 0 );
  signing_layout->addWidget( encrypt_transport, 1, 0, 1, 2, 0 );

  general_layout->addWidget( general_box );
  general_layout->addWidget( ports_box );
  general_layout->addWidget( auth_box );
  general_layout->addWidget( signing_box );
  general_layout->addStretch( 100 );

  insertTab( GeneralTab, general_tab, i18n( "General Settings" ) );

  //
  // Options for the mount commands
  //
  QWidget *mount_tab            = new QWidget( this );

  QVBoxLayout *mount_layout     = new QVBoxLayout( mount_tab );
  mount_layout->setSpacing( 5 );
  mount_layout->setMargin( 0 );

  // Common Options
  QGroupBox *common_options    = new QGroupBox( i18n( "Common Options" ), mount_tab );

  QGridLayout *common_layout   = new QGridLayout( common_options );
  common_layout->setSpacing( 5 );

  QLabel *user_id_label        = new QLabel( Smb4KSettings::self()->userIDItem()->label(),
                                 common_options );

  QWidget *user_widget         = new QWidget( common_options );

  QGridLayout *user_layout     = new QGridLayout( user_widget );
  user_layout->setSpacing( 5 );
  user_layout->setMargin( 0 );

  KLineEdit *user_id           = new KLineEdit( user_widget );
  user_id->setObjectName( "kcfg_UserID" );
  user_id->setAlignment( Qt::AlignRight );
  user_id->setReadOnly( true );

  QToolButton *user_chooser    = new QToolButton( user_widget );
  user_chooser->setIcon( KIcon( "edit-find-user" ) );
  user_chooser->setToolTip( i18n( "Choose a different user" ) );
  
  QMenu *user_menu             = new QMenu( user_chooser );
  user_chooser->setMenu( user_menu );

  QList<KUser> user_list = KUser::allUsers();
  QMap<QString,QString> users;

  for ( int i = 0; i < user_list.size(); ++i )
  {
    users.insert( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                      .arg( user_list.at( i ).uid() ),
                  QString( "%1" ).arg( user_list.at( i ).uid() ) );
  }

  QMap<QString,QString>::const_iterator u_it = users.constBegin();

  while ( u_it != users.constEnd() )
  {
    QAction *user_action = user_menu->addAction( u_it.key() );
    user_action->setData( u_it.value() );

    ++u_it;
  }

  user_layout->addWidget( user_id, 0, 0, 0 );
  user_layout->addWidget( user_chooser, 0, 1, Qt::AlignCenter );

  QLabel *group_id_label       = new QLabel( Smb4KSettings::self()->groupIDItem()->label(),
                                 common_options );

  QWidget *group_widget        = new QWidget( common_options );

  QGridLayout *group_layout    = new QGridLayout( group_widget );
  group_layout->setSpacing( 5 );
  group_layout->setMargin( 0 );

  KLineEdit *group_id          = new KLineEdit( group_widget );
  group_id->setObjectName( "kcfg_GroupID" );
  group_id->setAlignment( Qt::AlignRight );
  group_id->setReadOnly( true );
  
  QToolButton *group_chooser   = new QToolButton( group_widget );
  group_chooser->setIcon( KIcon( "edit-find-user" ) );
  group_chooser->setToolTip( i18n( "Choose a different group" ) );

  QMenu *group_menu            = new QMenu( group_chooser );
  group_chooser->setMenu( group_menu );

  QList<KUserGroup> group_list = KUserGroup::allGroups();
  QMap<QString,QString> groups;

  for ( int i = 0; i < group_list.size(); ++i )
  {
    groups.insert( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                       .arg( group_list.at( i ).gid() ),
                   QString( "%1" ).arg( group_list.at( i ).gid() ) );
  }

  QMap<QString,QString>::const_iterator g_it = groups.constBegin();

  while ( g_it != groups.constEnd() )
  {
    QAction *group_action = group_menu->addAction( g_it.key() );
    group_action->setData( g_it.value() );

    ++g_it;
  }

  group_layout->addWidget( group_id, 0, 0, 0 );
  group_layout->addWidget( group_chooser, 0, 1, Qt::AlignCenter );

  QLabel *fmask_label          = new QLabel( Smb4KSettings::self()->fileMaskItem()->label(),
                                 common_options );

  KLineEdit *fmask             = new KLineEdit( common_options );
  fmask->setObjectName( "kcfg_FileMask" );
  fmask->setAlignment( Qt::AlignRight );

  QLabel *dmask_label          = new QLabel( Smb4KSettings::self()->directoryMaskItem()->label(),
                                 common_options );

  KLineEdit *dmask             = new KLineEdit( common_options );
  dmask->setObjectName( "kcfg_DirectoryMask" );
  dmask->setAlignment( Qt::AlignRight );

#ifndef Q_OS_FREEBSD
  QLabel *write_access_label   = new QLabel( Smb4KSettings::self()->writeAccessItem()->label(),
                                 common_options );

  KComboBox *write_access      = new KComboBox( common_options );
  write_access->setObjectName( "kcfg_WriteAccess" );
  write_access->insertItem( Smb4KSettings::EnumWriteAccess::ReadWrite,
                            Smb4KSettings::self()->writeAccessItem()->choices().value( Smb4KSettings::EnumWriteAccess::ReadWrite ).label );
  write_access->insertItem( Smb4KSettings::EnumWriteAccess::ReadOnly,
                            Smb4KSettings::self()->writeAccessItem()->choices().value( Smb4KSettings::EnumWriteAccess::ReadOnly ).label );
#endif

  QLabel *charset_label        = new QLabel( Smb4KSettings::self()->clientCharsetItem()->label(),
                                 common_options );

  KComboBox *charset           = new KComboBox( common_options );
  charset->setObjectName( "kcfg_ClientCharset" );
  charset->insertItem( Smb4KSettings::EnumClientCharset::default_charset,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::default_charset ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_1,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_1 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_2,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_2 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_3,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_3 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_4,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_4 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_5,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_5 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_6,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_6 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_7,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_7 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_8,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset:: iso8859_8).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_9,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_9 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_13,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_13 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_14,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_14 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_15,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_15 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::utf8,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::utf8 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::koi8_r,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::koi8_r ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::koi8_u,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::koi8_u ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::koi8_ru,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::koi8_ru ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::cp1251,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::cp1251 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::gb2312,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::gb2312 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::big5,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::big5 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::euc_jp,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::euc_jp ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::euc_kr,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::euc_kr ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::tis_620,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::tis_620 ).label );

#ifdef Q_OS_FREEBSD
  QLabel *codepage_label       = new QLabel( Smb4KSettings::self()->serverCodepageItem()->label(),
                                 common_options );
  codepage_label->setObjectName( "CodepageLabel" );

  KComboBox *codepage          = new KComboBox( common_options );
  codepage->setObjectName( "kcfg_ServerCodepage" );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::default_codepage,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::default_codepage ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp437,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp437 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp720,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp720 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp737,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp737 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp775,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp775 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp850,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp850 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp852,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp852 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp855,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp855 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp857,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp857 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp858,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp858 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp860,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp860 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp861,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp861 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp862,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp862 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp863,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp863 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp864,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp864 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp865,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp865 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp866,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp866 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp869,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp869 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp874,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp874 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp932,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp932 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp936,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp936 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp949,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp949 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp950,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp950 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1250,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1250 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1251,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1251 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1252,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1252 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1253,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1253 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1254,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1254 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1255,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1255 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1256,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1256 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1257,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1257 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1258,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1258 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::unicode,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::unicode ).label );
#endif

#ifndef Q_OS_FREEBSD
  common_layout->addWidget( user_id_label, 0, 0, 0 );
  common_layout->addWidget( user_widget, 0, 1, 0 );
  common_layout->addWidget( group_id_label, 1, 0, 0 );
  common_layout->addWidget( group_widget, 1, 1, 0 );
  common_layout->addWidget( fmask_label, 2, 0, 0 );
  common_layout->addWidget( fmask, 2, 1, 0 );
  common_layout->addWidget( dmask_label, 3, 0, 0 );
  common_layout->addWidget( dmask, 3, 1, 0 );
  common_layout->addWidget( write_access_label, 4, 0, 0 );
  common_layout->addWidget( write_access, 4, 1, 0 );
  common_layout->addWidget( charset_label, 5, 0, 0 );
  common_layout->addWidget( charset, 5, 1, 0 );
#else
  common_layout->addWidget( user_id_label, 0, 0, 0 );
  common_layout->addWidget( user_widget, 0, 1, 0 );
  common_layout->addWidget( group_id_label, 1, 0, 0 );
  common_layout->addWidget( group_widget, 1, 1, 0 );
  common_layout->addWidget( fmask_label, 2, 0, 0 );
  common_layout->addWidget( fmask, 2, 1, 0 );
  common_layout->addWidget( dmask_label, 3, 0, 0 );
  common_layout->addWidget( dmask, 3, 1, 0 );
  common_layout->addWidget( charset_label, 4, 0, 0 );
  common_layout->addWidget( charset, 4, 1, 0 );
  common_layout->addWidget( codepage_label, 5, 0, 0 );
  common_layout->addWidget( codepage, 5, 1, 0 );
#endif

#ifndef Q_OS_FREEBSD
  // Advanced CIFS options
  QGroupBox *advanced_options  = new QGroupBox( i18n( "Advanced Options" ), mount_tab );

  QGridLayout *advanced_layout = new QGridLayout( advanced_options );
  advanced_layout->setSpacing( 5 );

  QCheckBox *permission_checks = new QCheckBox( Smb4KSettings::self()->permissionChecksItem()->label(),
                                 advanced_options );
  permission_checks->setObjectName( "kcfg_PermissionChecks" );

  QCheckBox *client_controls   = new QCheckBox( Smb4KSettings::self()->clientControlsIDsItem()->label(),
                                 advanced_options );
  client_controls->setObjectName( "kcfg_ClientControlsIDs" );

  QCheckBox *server_inodes     = new QCheckBox( Smb4KSettings::self()->serverInodeNumbersItem()->label(),
                                 advanced_options );
  server_inodes->setObjectName( "kcfg_ServerInodeNumbers" );

  QCheckBox *no_inode_caching  = new QCheckBox( Smb4KSettings::self()->noInodeDataCachingItem()->label(),
                                 advanced_options );
  no_inode_caching->setObjectName( "kcfg_NoInodeDataCaching" );

  QCheckBox *reserved_chars    = new QCheckBox( Smb4KSettings::self()->translateReservedCharsItem()->label(),
                                 advanced_options );
  reserved_chars->setObjectName( "kcfg_TranslateReservedChars" );

  QCheckBox *no_locking        = new QCheckBox( Smb4KSettings::self()->noLockingItem()->label(),
                                 advanced_options );
  no_locking->setObjectName( "kcfg_NoLocking" );

  QWidget *c_extra_widget      = new QWidget( advanced_options );

  QGridLayout *c_extra_layout  = new QGridLayout( c_extra_widget );
  c_extra_layout->setSpacing( 5 );
  c_extra_layout->setMargin( 0 );
  
  QLabel *security_label       = new QLabel( Smb4KSettings::self()->securityModeItem()->label(),
                                 c_extra_widget );
                                 
  KComboBox *security_box      = new KComboBox( c_extra_widget );
  security_box->setObjectName( "kcfg_SecurityMode" );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::Default,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Default ).label );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::None,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::None ).label );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::Krb5,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Krb5 ).label );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::Krb5i,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Krb5i ).label );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::Ntlm,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlm ).label );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::Ntlmi,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmi ).label );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::Ntlmv2,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmv2 ).label );
  security_box->insertItem( Smb4KSettings::EnumSecurityMode::Ntlmv2i,
                            Smb4KSettings::self()->securityModeItem()->choices().value( Smb4KSettings::EnumSecurityMode::Ntlmv2i ).label );

  QLabel *add_options_label    = new QLabel( Smb4KSettings::self()->customCIFSOptionsItem()->label(),
                                 c_extra_widget );

  KLineEdit *additional_opts   = new KLineEdit( c_extra_widget );
  additional_opts->setObjectName( "kcfg_CustomCIFSOptions" );

  c_extra_layout->addWidget( security_label, 0, 0, 0 );
  c_extra_layout->addWidget( security_box, 0, 1, 0 );
  c_extra_layout->addWidget( add_options_label, 1, 0, 0 );
  c_extra_layout->addWidget( additional_opts, 1, 1, 0 );

  advanced_layout->addWidget( permission_checks, 0, 0, 0 );
  advanced_layout->addWidget( client_controls, 0, 1, 0 );
  advanced_layout->addWidget( server_inodes, 1, 0, 0 );
  advanced_layout->addWidget( no_inode_caching, 1, 1, 0 );
  advanced_layout->addWidget( reserved_chars, 2, 0, 0 );
  advanced_layout->addWidget( no_locking, 2, 1, 0 );
  advanced_layout->addWidget( c_extra_widget, 3, 0, 1, 2, 0 );
#endif

  mount_layout->addWidget( common_options );
#ifndef Q_OS_FREEBSD
  mount_layout->addWidget( advanced_options );
#endif
  mount_layout->addStretch( 100 );

  insertTab( MountingTab, mount_tab, i18n( "Mounting" ) );

  //
  // Options for the client programs
  //
  QWidget *clients_tab          = new QWidget( this );

  QVBoxLayout *client_layout    = new QVBoxLayout( clients_tab );
  client_layout->setSpacing( 5 );
  client_layout->setMargin( 0 );

  // 'net' program
  QGroupBox *net_box            = new QGroupBox( i18n( "net" ), clients_tab );

  QGridLayout *net_layout       = new QGridLayout( net_box );
  net_layout->setSpacing( 5 );

  QLabel *proto_hint_label      = new QLabel( Smb4KSettings::self()->protocolHintItem()->label(),
                                  net_box );

  KComboBox *protocol_hint      = new KComboBox( net_box );
  protocol_hint->setObjectName( "kcfg_ProtocolHint" );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::Automatic,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::Automatic ).label );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::RPC,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::RPC ).label );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::RAP,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::RAP ).label );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::ADS,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::ADS ).label );

  net_layout->addWidget( proto_hint_label, 0, 0, 0 );
  net_layout->addWidget( protocol_hint, 0, 1, 0 );

  // 'smbclient' program
  QGroupBox *smbclient_box      = new QGroupBox( i18n( "smbclient" ), clients_tab );

  QGridLayout *smbclient_layout = new QGridLayout( smbclient_box );
  smbclient_layout->setSpacing( 5 );

  QLabel *name_resolve_label   = new QLabel( Smb4KSettings::self()->nameResolveOrderItem()->label(),
                                 smbclient_box );

  KLineEdit *name_resolve      = new KLineEdit( smbclient_box );
  name_resolve->setObjectName( "kcfg_NameResolveOrder" );

  QLabel *buffer_size_label    = new QLabel( Smb4KSettings::self()->bufferSizeItem()->label(),
                                 smbclient_box );

  KIntNumInput *buffer_size    = new KIntNumInput( smbclient_box );
  buffer_size->setObjectName( "kcfg_BufferSize" );
  buffer_size->setSuffix( i18n( " Bytes" ) );
  buffer_size->setSliderEnabled( true );

  smbclient_layout->addWidget( name_resolve_label, 0, 0, 0 );
  smbclient_layout->addWidget( name_resolve, 0, 1, 0 );
  smbclient_layout->addWidget( buffer_size_label, 2, 0, 0 );
  smbclient_layout->addWidget( buffer_size, 2, 1, 0 );

  // 'nmblookup' program
  QGroupBox *nmblookup_box     = new QGroupBox( i18n( "nmblookup" ), clients_tab );

  QGridLayout *nmblookup_layout = new QGridLayout( nmblookup_box );
  nmblookup_layout->setSpacing( 5 );

  QLabel *broadcast_add_label  = new QLabel( Smb4KSettings::self()->broadcastAddressItem()->label(),
                                 nmblookup_box );

  KLineEdit *broadcast_address = new KLineEdit( nmblookup_box );
  broadcast_address->setObjectName( "kcfg_BroadcastAddress" );

  QCheckBox *port_137          = new QCheckBox( Smb4KSettings::self()->usePort137Item()->label(),
                                 nmblookup_box );
  port_137->setObjectName( "kcfg_UsePort137" );

  nmblookup_layout->addWidget( broadcast_add_label, 0, 0, 0 );
  nmblookup_layout->addWidget( broadcast_address, 0, 1, 0 );
  nmblookup_layout->addWidget( port_137, 1, 0, 1, 2, 0 );

  // 'smbtree' program
  QGroupBox *smbtree_box       = new QGroupBox( i18n( "smbtree" ), clients_tab );

  QGridLayout *smbtree_layout  = new QGridLayout( smbtree_box );
  smbtree_layout->setSpacing( 5 );

  QCheckBox *smbtree_bcasts    = new QCheckBox( Smb4KSettings::self()->smbtreeSendBroadcastsItem()->label(),
                                 smbtree_box );
  smbtree_bcasts->setObjectName( "kcfg_SmbtreeSendBroadcasts" );

  smbtree_layout->addWidget( smbtree_bcasts, 0, 0, 0 );

  client_layout->addWidget( net_box );
  client_layout->addWidget( nmblookup_box );
  client_layout->addWidget( smbclient_box );
  client_layout->addWidget( smbtree_box );
  client_layout->addStretch( 100 );

  insertTab( ClientProgramsTab, clients_tab, i18n( "Utility Programs" ) );

  //
  // Custom options
  //
  QWidget *custom_tab        = new QWidget( this );

  QHBoxLayout *custom_layout = new QHBoxLayout( custom_tab );
  custom_layout->setSpacing( 5 );
  custom_layout->setMargin( 0 );

  m_custom_options           = new KListWidget( custom_tab );
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
  
  m_editors = new QWidget( custom_tab );
  
  QGridLayout *edit_layout = new QGridLayout( m_editors );
  edit_layout->setSpacing( 5 );
  edit_layout->setContentsMargins( edit_layout->margin(),
                                   edit_layout->margin() + 10,
                                   edit_layout->margin(),
                                   edit_layout->margin() );
  
  QLabel *unc_label = new QLabel( i18n( "UNC Address:" ), m_editors );
  
  m_unc_address = new KLineEdit( m_editors );
  m_unc_address->setReadOnly( true );
  
  QLabel *smb_port_label = new QLabel( "SMB Port:", m_editors );
  
  m_smb_port = new KIntNumInput( m_editors );
  m_smb_port->setRange( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt(),
                        Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );
  m_smb_port->setSliderEnabled( true );

#ifndef Q_OS_FREEBSD
  QLabel *fs_port_label = new QLabel( i18n( "Filesystem Port:" ), m_editors );
  
  m_fs_port = new KIntNumInput( m_editors );
  m_fs_port->setRange( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt(),
                       Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );
  m_fs_port->setSliderEnabled( true );
  
  QLabel *rw_label = new QLabel( i18n( "Write Access:" ), m_editors );
  
  m_write_access = new KComboBox( m_editors );
  m_write_access->insertItem( 0, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadWrite ).label, 
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadWrite ) );
  m_write_access->insertItem( 1, Smb4KSettings::self()->writeAccessItem()->choices()
                                 .value( Smb4KSettings::EnumWriteAccess::ReadOnly ).label, 
                              QVariant::fromValue<int>( Smb4KCustomOptions::ReadOnly ) );
#endif
  
  QLabel *protocol_label = new QLabel( i18n( "Protocol Hint:" ), m_editors );
  
  m_protocol_hint        = new KComboBox( m_editors );
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
  
  QLabel *uid_label = new QLabel( i18n( "User ID:" ), m_editors );
  m_user_id         = new KComboBox( m_editors );
  
  QList<KUser> all_users = KUser::allUsers();
  
  for ( int i = 0; i < all_users.size(); ++i )
  {
    KUser user = all_users.at( i );
    m_user_id->insertItem( i, QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() ), 
                           QVariant::fromValue<K_UID>( user.uid() ) );
  }
  
  QLabel *gid_label = new QLabel( i18n( "Group ID:" ), m_editors );
  m_group_id        = new KComboBox( m_editors );
  
  QList<KUserGroup> all_groups = KUserGroup::allGroups();
  
  for ( int i = 0; i < all_groups.size(); ++i )
  {
    KUserGroup group = all_groups.at( i );
    m_group_id->insertItem( i, QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() ), 
                           QVariant::fromValue<K_UID>( group.gid() ) );
  }
  
  m_kerberos = new QCheckBox( Smb4KSettings::self()->useKerberosItem()->label(), m_editors );
  
  m_editors->setEnabled( false );

  clearEditors();
  
#ifndef Q_OS_FREEBSD
  edit_layout->addWidget( unc_label, 0, 0, 0 );
  edit_layout->addWidget( m_unc_address, 0, 1, 0 );
  edit_layout->addWidget( smb_port_label, 1, 0, 0 );
  edit_layout->addWidget( m_smb_port, 1, 1, 0 );
  edit_layout->addWidget( fs_port_label, 2, 0, 0 );
  edit_layout->addWidget( m_fs_port, 2, 1, 0 );
  edit_layout->addWidget( rw_label, 3, 0, 0 );
  edit_layout->addWidget( m_write_access, 3, 1, 0 );
  edit_layout->addWidget( protocol_label, 4, 0, 0 );
  edit_layout->addWidget( m_protocol_hint, 4, 1, 0 );
  edit_layout->addWidget( uid_label, 5, 0, 0 );
  edit_layout->addWidget( m_user_id, 5, 1, 0 );
  edit_layout->addWidget( gid_label, 6, 0, 0 );
  edit_layout->addWidget( m_group_id, 6, 1, 0 );
  edit_layout->addWidget( m_kerberos, 7, 0, 1, 2, 0 );
  edit_layout->setRowStretch( 8, 100 );
#else
  edit_layout->addWidget( unc_label, 0, 0, 0 );
  edit_layout->addWidget( m_unc_address, 0, 1, 0 );
  edit_layout->addWidget( smb_port_label, 1, 0, 0 );
  edit_layout->addWidget( m_smb_port, 1, 1, 0 );
  edit_layout->addWidget( protocol_label, 2, 0, 0 );
  edit_layout->addWidget( m_protocol_hint, 2, 1, 0 );
  edit_layout->addWidget( uid_label, 3, 0, 0 );
  edit_layout->addWidget( m_user_id, 3, 1, 0 );
  edit_layout->addWidget( gid_label, 4, 0, 0 );
  edit_layout->addWidget( m_group_id, 4, 1, 0 );
  edit_layout->addWidget( m_kerberos, 5, 0, 1, 2, 0 );
  edit_layout->setRowStretch( 6, 100 );
#endif

  custom_layout->addWidget( m_custom_options );
  custom_layout->addWidget( m_editors );

  insertTab( CustomOptionsTab, custom_tab, i18n( "Custom Options" ) );

  //
  // Connections
  //
  connect( user_menu,        SIGNAL( triggered( QAction * ) ),
           this,             SLOT( slotNewUserTriggered( QAction * ) ) );

  connect( group_menu,       SIGNAL( triggered( QAction * ) ),
           this,             SLOT( slotNewGroupTriggered( QAction * ) ) );

  connect( m_custom_options, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
           this,             SLOT( slotEditCustomItem( QListWidgetItem * ) ) );

  connect( m_custom_options, SIGNAL( itemSelectionChanged() ),
           this,             SLOT( slotItemSelectionChanged() ) );

  connect( m_custom_options, SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this,             SLOT( slotCustomContextMenuRequested( const QPoint & ) ) );
           
  connect( edit_action,      SIGNAL( triggered( bool ) ),
           this,             SLOT( slotEditActionTriggered( bool ) ) );
           
  connect( remove_action,    SIGNAL( triggered( bool ) ),
           this,             SLOT( slotRemoveActionTriggered( bool ) ) );
           
  connect( clear_action,     SIGNAL( triggered( bool ) ),
           this,             SLOT( slotClearActionTriggered( bool ) ) );
  
  connect( undo_action,      SIGNAL( triggered( bool ) ),
           this,             SLOT( slotUndoActionTriggered( bool ) ) );
  
  connect( m_smb_port,       SIGNAL( valueChanged( int ) ),
           this,             SLOT( slotEntryChanged() ) );
  
#ifndef Q_OS_FREEBSD
  connect( m_fs_port,        SIGNAL( valueChanged( int ) ),
           this,             SLOT( slotEntryChanged() ) );
  
  connect( m_write_access,   SIGNAL( currentIndexChanged( int ) ),
           this,             SLOT( slotEntryChanged() ) );
#endif
  
  connect( m_protocol_hint,  SIGNAL( currentIndexChanged( int ) ),
           this,             SLOT( slotEntryChanged() ) );
  
  connect( m_user_id,        SIGNAL( currentIndexChanged( int ) ),
           this,             SLOT( slotEntryChanged() ) );
  
  connect( m_group_id,       SIGNAL( currentIndexChanged( int ) ),
           this,             SLOT( slotEntryChanged() ) );
  
  connect( m_kerberos,       SIGNAL( toggled( bool ) ),
           this,             SLOT( slotEntryChanged() ) );
}


Smb4KSambaOptions::~Smb4KSambaOptions()
{
  while ( !m_options_list.isEmpty() )
  {
    delete m_options_list.takeFirst();
  }
}


void Smb4KSambaOptions::insertCustomOptions( const QList<Smb4KCustomOptions *> &list )
{
  // Insert those options that are not there.
  for ( int i = 0; i < list.size(); ++i )
  {
    Smb4KCustomOptions *options = findOptions( list.at( i )->url() );
    
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
        item->setData( Qt::UserRole, m_options_list.at( i )->url() );
        break;
      }
      case Smb4KCustomOptions::Share:
      {
        QListWidgetItem *item = new QListWidgetItem( KIcon( "folder-remote" ), 
                                    m_options_list.at( i )->unc(),
                                    m_custom_options, Share );
        item->setData( Qt::UserRole, m_options_list.at( i )->url() );
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


const QList<Smb4KCustomOptions *> Smb4KSambaOptions::getCustomOptions()
{
  return m_options_list;
}


Smb4KCustomOptions *Smb4KSambaOptions::findOptions( const QUrl &url )
{
  Smb4KCustomOptions *options = NULL;
  
  for ( int i = 0; i < m_options_list.size(); ++i )
  {
    if ( url == m_options_list.at( i )->url() )
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


void Smb4KSambaOptions::populateEditors( Smb4KCustomOptions *options )
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
  
  // Enable widget
  m_editors->setEnabled( true );
}


void Smb4KSambaOptions::clearEditors()
{
  // Do not reset the current custom options object here,
  // so that we can undo the last changes!
  
  // Clearing the editors means to reset them to their initial/default values.
  m_unc_address->clear();
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
  
  // Disable widget
  m_editors->setEnabled( false );
}


void Smb4KSambaOptions::commitChanges()
{
  if ( m_current_options && !m_options_list.isEmpty() && m_editors->isEnabled() )
  {
    Smb4KCustomOptions *options = findOptions( m_current_options->url() );
    
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
    
    m_maybe_changed = true;
    emit customSettingsModified();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KSambaOptions::eventFilter( QObject *obj, QEvent *e )
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


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSambaOptions::slotNewUserTriggered( QAction *action )
{
  KLineEdit *user_id = findChild<KLineEdit *>( "kcfg_UserID" );

  if ( user_id )
  {
    user_id->setText( action->data().toString() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptions::slotNewGroupTriggered( QAction *action )
{
  KLineEdit *group_id = findChild<KLineEdit *>( "kcfg_GroupID" );

  if ( group_id )
  {
    group_id->setText( action->data().toString() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptions::slotEditCustomItem( QListWidgetItem *item )
{
  Smb4KCustomOptions *options = findOptions( item->data( Qt::UserRole ).toUrl() );
  
  if ( options )
  {
    populateEditors( options );
  }
  else
  {
    clearEditors();
  }
}


void Smb4KSambaOptions::slotItemSelectionChanged()
{
  clearEditors();
}


void Smb4KSambaOptions::slotCustomContextMenuRequested( const QPoint &pos )
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


void Smb4KSambaOptions::slotEditActionTriggered( bool /*checked*/ )
{
  slotEditCustomItem( m_custom_options->currentItem() );
}


void Smb4KSambaOptions::slotRemoveActionTriggered( bool /*checked*/ )
{
  QListWidgetItem *item = m_custom_options->currentItem();
  Smb4KCustomOptions *options = findOptions( item->data( Qt::UserRole ).toUrl() );
  
  if ( item && options )
  {
    if ( m_current_options->url() == options->url() )
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


void Smb4KSambaOptions::slotClearActionTriggered( bool /*checked*/ )
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


void Smb4KSambaOptions::slotUndoActionTriggered( bool /*checked*/ )
{
  if ( m_removed )
  {
    emit reloadCustomSettings();
  }
  else
  {
    if ( m_current_options )
    {
      if ( m_custom_options->currentItem()->data( Qt::UserRole ).toUrl() == m_current_options->url() )
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
        Smb4KCustomOptions *options = findOptions( m_current_options->url() );
        
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


void Smb4KSambaOptions::slotEntryChanged()
{
  commitChanges();
}



#include "smb4ksambaoptions.moc"
