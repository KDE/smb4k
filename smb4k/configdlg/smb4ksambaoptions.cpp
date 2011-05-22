/***************************************************************************
    smb4ksambaoptions.cpp  -  This is the configuration page for the
    Samba settings of Smb4K
                             -------------------
    begin                : Mo Jan 26 2004
    copyright            : (C) 2004-2011 by Alexander Reinholdt
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
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTreeWidgetItemIterator>
#include <QToolButton>

// KDE includes
#include <klocale.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <kuser.h>
// #include <kpushbutton.h>
#include <kmenu.h>

// System includes
#include <unistd.h>
#include <sys/types.h>

// application specific includes
#include <smb4ksambaoptions.h>
#include <core/smb4kglobal.h>
#include <core/smb4ksettings.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>

using namespace Smb4KGlobal;


Smb4KSambaOptions::Smb4KSambaOptions( QWidget *parent ) : KTabWidget( parent )
{
  m_collection = new KActionCollection( this );
  m_maybe_changed = false;

  //
  // General
  //
  QWidget *general_tab          = new QWidget( this );

  QGridLayout *general_layout   = new QGridLayout( general_tab );
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

#ifndef Q_OS_FREEBSD
  QLabel *remote_fs_port_label  = new QLabel( Smb4KSettings::self()->remoteFileSystemPortItem()->label(),
                                  ports_box );

  KIntNumInput *remote_fs_port  = new KIntNumInput( ports_box );
  remote_fs_port->setObjectName( "kcfg_RemoteFileSystemPort" );
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

  auth_layout->addWidget( auth_kerberos, 0, 0, 0 );
  auth_layout->addWidget( auth_machine_acc, 0, 1, 0 );

  QGroupBox *signing_box        = new QGroupBox( i18n( "Signing State" ), general_tab );

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

  signing_layout->addWidget( signing_state_label, 0, 0, 0 );
  signing_layout->addWidget( signing_state, 0, 1, 0 );

  QSpacerItem *spacer1 = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  general_layout->addWidget( general_box, 0, 0, 0 );
  general_layout->addWidget( ports_box, 1, 0, 0 );
  general_layout->addWidget( auth_box, 2, 0, 0 );
  general_layout->addWidget( signing_box, 3, 0, 0 );
  general_layout->addItem( spacer1, 4, 0 );

  insertTab( GeneralTab, general_tab, i18n( "General Settings" ) );

  //
  // Options for the mount commands
  //
  QWidget *mount_tab            = new QWidget( this );

  QGridLayout *mount_layout     = new QGridLayout( mount_tab );
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
  
//   KPushButton *user_chooser    = new KPushButton( KGuiItem( QString(), "edit-find-user",
//                                  i18n( "Choose a different user" ) ), user_widget );
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

//   KPushButton *group_chooser   = new KPushButton( KGuiItem( QString(), "edit-find-user",
//                                  i18n( "Choose a different group" ) ), group_widget );
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

  QSpacerItem *spacer2 = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  mount_layout->addWidget( common_options, 0, 0, 0 );
#ifndef Q_OS_FREEBSD
  mount_layout->addWidget( advanced_options, 1, 0, 0 );
  mount_layout->addItem( spacer2, 2, 0 );
#else
  mount_layout->addItem( spacer2, 1, 0 );
#endif

  insertTab( MountingTab, mount_tab, i18n( "Mounting" ) );

  //
  // Options for the client programs
  //
  QWidget *clients_tab          = new QWidget( this );

  QGridLayout *client_layout    = new QGridLayout( clients_tab );
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

  QSpacerItem *spacer3 = new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding );

  client_layout->addWidget( net_box, 0, 0, 0 );
  client_layout->addWidget( nmblookup_box, 1, 0, 0 );
  client_layout->addWidget( smbclient_box, 2, 0, 0 );
  client_layout->addWidget( smbtree_box, 3, 0, 0 );
  client_layout->addItem( spacer3, 4, 0 );

  insertTab( ClientProgramsTab, clients_tab, i18n( "Utility Programs" ) );

  //
  // Custom options
  //
  QWidget *custom_tab        = new QWidget( this );

  QGridLayout *custom_layout = new QGridLayout( custom_tab );
  custom_layout->setSpacing( 5 );
  custom_layout->setMargin( 0 );

  m_custom_options           = new QTreeWidget( custom_tab );
  m_custom_options->setObjectName( "CustomOptionsList" );
  m_custom_options->viewport()->installEventFilter( this );
  m_custom_options->setSelectionMode( QTreeWidget::ExtendedSelection );
  m_custom_options->setRootIsDecorated( false );
  m_custom_options->setEditTriggers( QTreeWidget::NoEditTriggers );
  m_custom_options->setContextMenuPolicy( Qt::CustomContextMenu );
#ifndef Q_OS_FREEBSD
  m_custom_options->setColumnCount( 7 );
#else
  m_custom_options->setColumnCount( 6 );
#endif

  QStringList header_labels;
  header_labels.append( i18n( "UNC Address" ) );
  header_labels.append( i18n( "Protocol" ) );
#ifndef Q_OS_FREEBSD
  header_labels.append( i18n( "Write Access" ) );
#endif
  header_labels.append( i18n( "Kerberos" ) );
  header_labels.append( i18n( "User ID" ) );
  header_labels.append( i18n( "Group ID" ) );
  header_labels.append( i18n( "SMB Port" ) );
#ifndef Q_OS_FREEBSD
  header_labels.append( i18n( "File System Port" ) );
#endif

  m_custom_options->setHeaderLabels( header_labels );

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
  
  m_collection->addAction( "edit_action", edit_action );
  m_collection->addAction( "remove_action", remove_action );
  m_collection->addAction( "clear_action", clear_action );

  m_menu->addAction( edit_action );
  m_menu->addAction( remove_action );
  m_menu->addAction( clear_action );

  custom_layout->addWidget( m_custom_options, 0, 0, 0 );

  insertTab( CustomOptionsTab, custom_tab, i18n( "Custom Options" ) );

  //
  // Connections
  //
  connect( user_menu,        SIGNAL( triggered( QAction * ) ),
           this,             SLOT( slotNewUserTriggered( QAction * ) ) );

  connect( group_menu,       SIGNAL( triggered( QAction * ) ),
           this,             SLOT( slotNewGroupTriggered( QAction * ) ) );

  connect( m_custom_options, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
           this,             SLOT( slotEditCustomItem( QTreeWidgetItem *, int ) ) );

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
}


Smb4KSambaOptions::~Smb4KSambaOptions()
{
}


bool Smb4KSambaOptions::eventFilter( QObject *obj, QEvent *e )
{
  bool eat_event = false;
  
  if ( obj == m_custom_options->viewport() )
  {
    switch ( e->type() )
    {
      case QEvent::MouseButtonPress:
      {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>( e );
        
        QTreeWidgetItem *item = m_custom_options->itemAt( m_custom_options->viewport()->mapFromGlobal( mouse_event->globalPos() ) );

        if ( !item )
        {
          // Remove all edit widgets.
          removeEditWidgets();
          
          // Resize the columns to the contents.          
          for ( int i = 0; i < m_custom_options->columnCount(); ++i )
          {
            m_custom_options->resizeColumnToContents( i );
          }

          // Sort items.
          m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
          
          // Enable/disable actions.
          m_collection->action( "edit_action" )->setEnabled( false );
          m_collection->action( "remove_action" )->setEnabled( false );
          m_collection->action( "clear_action" )->setEnabled( (m_custom_options->topLevelItemCount() != 0) );
        }
        else
        {
          // We only need to enable/disable actions here. Everything else is
          // managed elsewhere.
          int col = m_custom_options->columnAt( m_custom_options->viewport()->mapFromGlobal( mouse_event->globalPos() ).x() );
          
          switch ( col )
          {
            case ItemName:
            {
              m_collection->action( "edit_action" )->setEnabled( false );
              break;
            }
            case Protocol:
            {
              switch ( item->type() )
              {
                case Host:
                {
                  m_collection->action( "edit_action" )->setEnabled( true );
                  break;
                }
                case Share:
                {
                  m_collection->action( "edit_action" )->setEnabled( false );
                  break;
                }
                default:
                {
                  break;
                }
              }
              break;
            }
#ifndef Q_OS_FREEBSD
            case WriteAccess:
            {
              m_collection->action( "edit_action" )->setEnabled( true );
              break;
            }
#endif
            case Kerberos:
            {
              switch ( item->type() )
              {
                case Host:
                {
                  m_collection->action( "edit_action" )->setEnabled( true );
                  break;
                }
                case Share:
                {
                  m_collection->action( "edit_action" )->setEnabled( false );
                  break;
                }
                default:
                {
                  break;
                }
              }
              break;
            }
            case UID:
            {
              m_collection->action( "edit_action" )->setEnabled( true );
              break;              
            }
            case GID:
            {
              m_collection->action( "edit_action" )->setEnabled( true );
              break;
            }
            case SMBPort:
            {
#ifndef Q_OS_FREEBSD
              switch ( item->type() )
              {
                case Host:
                {
                  m_collection->action( "edit_action" )->setEnabled( true );
                  break;
                }
                case Share:
                {
                  m_collection->action( "edit_action" )->setEnabled( false );
                  break;
                }
                default:
                {
                  break;
                }
              }
#else
              m_collection->action( "edit_action" )->setEnabled( true );
#endif
              break;
            }
#ifndef Q_OS_FREEBSD
            case FileSystemPort:
            {
              m_collection->action( "edit_action" )->setEnabled( true );
              break;
            }
#endif
            default:
            {
              break;
            }
          }
          
          m_collection->action( "remove_action" )->setEnabled( true );
          m_collection->action( "clear_action" )->setEnabled( true );
        }
          
        break;
      }
      case QEvent::KeyPress:
      {
        QKeyEvent *key_event = static_cast<QKeyEvent *>( e );
        
        switch ( key_event->key() )
        {
          case Qt::Key_Return:
          case Qt::Key_Enter:
          {
            switch ( m_custom_options->currentColumn() )
            {
              case SMBPort:
              {
                for ( int i = 0; i < m_custom_options->topLevelItemCount(); ++i )
                {
                  KIntNumInput *input = static_cast<KIntNumInput *>( m_custom_options->itemWidget( m_custom_options->topLevelItem( i ), SMBPort ) );
                  
                  if ( input )
                  {
                    input->clearFocus();
                    input->unsetCursor();
                    m_custom_options->topLevelItem( i )->setText( SMBPort, QString( "%1" ).arg( input->value() ) );
                    
                    removeEditWidgets();
                    
                    for ( int i = 0; i < m_custom_options->columnCount(); ++i )
                    {
                      m_custom_options->resizeColumnToContents( i );
                    }

                    m_custom_options->sortItems( ItemName, Qt::AscendingOrder );

                    eat_event = true;
                    
                    // A value changed. Set m_maybe_changed to TRUE and emit 
                    // customSettingsModified
                    m_maybe_changed = true;
                    emit customSettingsModified();
                    
                    break;
                  }
                  else
                  {
                    continue;
                  }
                }
                
                break;
              }
#ifndef Q_OS_FREEBSD
              case FileSystemPort:
              {
                for ( int i = 0; i < m_custom_options->topLevelItemCount(); ++i )
                {
                  KIntNumInput *input = static_cast<KIntNumInput *>( m_custom_options->itemWidget( m_custom_options->topLevelItem( i ), FileSystemPort ) );
                  
                  if ( input )
                  {
                    input->clearFocus();
                    input->unsetCursor();
                    m_custom_options->topLevelItem( i )->setText( FileSystemPort, QString( "%1" ).arg( input->value() ) );
                    
                    removeEditWidgets();
                    
                    for ( int i = 0; i < m_custom_options->columnCount(); ++i )
                    {
                      m_custom_options->resizeColumnToContents( i );
                    }

                    m_custom_options->sortItems( ItemName, Qt::AscendingOrder );

                    eat_event = true;
                    
                    // A value changed. Set m_maybe_changed to TRUE and emit 
                    // customSettingsModified
                    m_maybe_changed = true;
                    emit customSettingsModified();
                    
                    break;
                  }
                  else
                  {
                    continue;
                  }
                }
                
                break;
              }
#endif
              default:
              {
                break;
              }
            }
            
            break;
          }
          default:
          {
            break;
          }
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
    // Do nothing
  }

  return (!eat_event ? KTabWidget::eventFilter( obj, e ) : eat_event);
}


void Smb4KSambaOptions::insertCustomOptions( const QList<Smb4KCustomOptions *> &list )
{
  // Populate the internal list.
  m_options_list.clear();
  
  for ( int i = 0; i < list.size(); ++i )
  {
    m_options_list << *list[i];
  }
  
  // Clear the tree widget if necessary.
  while ( m_custom_options->topLevelItemCount() != 0 )
  {
    delete m_custom_options->topLevelItem( 0 );
  }
  
  // Display the list.
  for ( int i = 0; i < list.size(); ++i )
  {
    QTreeWidgetItem *item = NULL;
  
    switch ( list.at( i )->type() )
    {
      case Smb4KCustomOptions::Host:
      {
        item = new QTreeWidgetItem( m_custom_options, Host );
        
        // UNC & icon
        item->setText( ItemName, list.at( i )->host()->unc() );
        item->setIcon( ItemName, KIcon( "preferences-other" ) );
        
        // Protocol
        switch ( list.at( i )->protocolHint() )
        {
          case Smb4KCustomOptions::Automatic:
          {
            item->setText( Protocol, i18n( "Automatic" ) );
            break;
          }
          case Smb4KCustomOptions::RPC:
          {
            item->setText( Protocol, "RPC" );
            break;
          }
          case Smb4KCustomOptions::RAP:
          {
            item->setText( Protocol, "RAP" );
            break;
          }
          case Smb4KCustomOptions::ADS:
          {
            item->setText( Protocol, "ADS" );
            break;
          }
          case Smb4KCustomOptions::UndefinedProtocolHint:
          {
            switch ( Smb4KSettings::protocolHint() )
            {
              case Smb4KSettings::EnumProtocolHint::Automatic:
              {
                item->setText( Protocol, i18n( "Automatic" ) );
                break;
              }
              case Smb4KSettings::EnumProtocolHint::RPC:
              {
                item->setText( Protocol, "RPC" );
                break;
              }
              case Smb4KSettings::EnumProtocolHint::RAP:
              {
                item->setText( Protocol, "RAP" );
                break;
              }
              case Smb4KSettings::EnumProtocolHint::ADS:
              {
                item->setText( Protocol, "ADS" );
                break;
              }
              default:
              {
                item->setText( Protocol, "-" );
              }
            }
            break;
          }
          default:
          {
            break;
          }
        }
        
#ifndef Q_OS_FREEBSD
        // Write access
        switch ( list.at( i )->writeAccess() )
        {
          case Smb4KCustomOptions::ReadWrite:
          {
            item->setText( WriteAccess, i18n( "read-write" ) );
            break;
          }
          case Smb4KCustomOptions::ReadOnly:
          {
            item->setText( WriteAccess, i18n( "read-only" ) );
            break;
          }
          case Smb4KCustomOptions::UndefinedWriteAccess:
          {
            switch ( Smb4KSettings::writeAccess() )
            {
              case Smb4KSettings::EnumWriteAccess::ReadWrite:
              {
                item->setText( WriteAccess, i18n( "read-write" ) );
                break;
              }
              case Smb4KSettings::EnumWriteAccess::ReadOnly:
              {
                item->setText( WriteAccess, i18n( "read-only" ) );
                break;
              }
              default:
              {
                break;
              }
            }
            break;
          }
          default:
          {
            break;
          }
        }
#endif
      
        // Kerberos
        switch ( list.at( i )->useKerberos() )
        {
          case Smb4KCustomOptions::UseKerberos:
          {
            item->setText( Kerberos, i18n( "yes" ) );
            break;
          }
          case Smb4KCustomOptions::NoKerberos:
          {
            item->setText( Kerberos, i18n( "no" ) );
            break;
          }
          case Smb4KCustomOptions::UndefinedKerberos:
          {
            item->setText( Kerberos, Smb4KSettings::self()->useKerberos() ? i18n( "yes" ) : i18n( "no" ) );
            break;
          }
          default:
          {
            break;
          }
        }
        
        // UID
        item->setText( UID, QString( "%1 (%2)" ).arg( list.at( i )->owner() ).arg( list.at( i )->uid() ) );

        // GID
        item->setText( GID, QString( "%1 (%2)" ).arg( list.at( i )->group() ).arg( list.at( i )->gid() ) );
        
        // SMB Port
        if ( list.at( i )->smbPort() != -1 )
        {
          item->setText( SMBPort, QString( "%1" ).arg( list.at( i )->smbPort() ) );
        }
        else
        {
          item->setText( SMBPort, QString( "%1" ).arg( Smb4KSettings::remoteSMBPort() ) );
        }
        
#ifndef Q_OS_FREEBSD
        // File System Port
        if ( list.at( i )->fileSystemPort() != -1 )
        {
          item->setText( FileSystemPort, QString( "%1" ).arg( list.at( i )->fileSystemPort() ) );
        }
        else
        {
          item->setText( FileSystemPort, QString( "%1" ).arg( Smb4KSettings::remoteFileSystemPort() ) );
        }
#endif
        
        break;
      }
      case Smb4KCustomOptions::Share:
      {
        item = new QTreeWidgetItem( m_custom_options, Share );
        
        // UNC & icon
        item->setText( ItemName, list.at( i )->share()->unc() );
        item->setIcon( ItemName, KIcon( "preferences-other" ) );
        
        // Protocol
        item->setText( Protocol, "-" );
        
#ifndef Q_OS_FREEBSD
        switch ( list.at( i )->writeAccess() )
        {
          case Smb4KCustomOptions::ReadWrite:
          {
            item->setText( WriteAccess, i18n( "read-write" ) );
            break;
          }
          case Smb4KCustomOptions::ReadOnly:
          {
            item->setText( WriteAccess, i18n( "read-only" ) );
            break;
          }
          case Smb4KCustomOptions::UndefinedWriteAccess:
          {
            switch ( Smb4KSettings::writeAccess() )
            {
              case Smb4KSettings::EnumWriteAccess::ReadWrite:
              {
                item->setText( WriteAccess, i18n( "read-write" ) );
                break;
              }
              case Smb4KSettings::EnumWriteAccess::ReadOnly:
              {
                item->setText( WriteAccess, i18n( "read-only" ) );
                break;
              }
              default:
              {
                break;
              }
            }
            break;
          }
          default:
          {
            break;
          }
        }
#endif
        // Kerberos
        item->setText( Kerberos, "-" );
        
        // UID
        item->setText( UID, QString( "%1 (%2)" ).arg( list.at( i )->owner() ).arg( list.at( i )->uid() ) );

        // GID
        item->setText( GID, QString( "%1 (%2)" ).arg( list.at( i )->group() ).arg( list.at( i )->gid() ) );

        // Ports
#ifndef Q_OS_FREEBSD
        item->setText( SMBPort, "-" );
        
        if ( list.at( i )->fileSystemPort() != -1 )
        {
          item->setText( FileSystemPort, QString( "%1" ).arg( list.at( i )->fileSystemPort() ) );
        }
        else
        {
          item->setText( FileSystemPort, QString( "%1" ).arg( Smb4KSettings::remoteFileSystemPort() ) );
        }
#else
        if ( list.at( i )->smbPort() != -1 )
        {
          item->setText( SMBPort, QString( "%1" ).arg( list.at( i )->smbPort() ) );
        }
        else
        {
          item->setText( SMBPort, QString( "%1" ).arg( Smb4KSettings::remoteSMBPort() ) );
        }
#endif
        
        break;
      }
      default:
      {
        break;
      }
    }
  }

  for ( int i = 0; i < m_custom_options->columnCount(); ++i )
  {
    m_custom_options->resizeColumnToContents( i );
  }

  m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
}


const QList<Smb4KCustomOptions *> Smb4KSambaOptions::getCustomOptions()
{
  QList<Smb4KCustomOptions *> options;
  
  for ( int i = 0; i < m_options_list.size(); ++i )
  {
    options << &m_options_list[i];
  }
  
  return options;
}


Smb4KCustomOptions *Smb4KSambaOptions::findOptions( const QString &unc )
{
  Smb4KCustomOptions *options = NULL;
  
  for ( int i = 0; i < m_options_list.size(); ++i )
  {
    if ( m_options_list.at( i ).type() == Smb4KCustomOptions::Host )
    {
      if ( QString::compare( m_options_list.at( i ).host()->unc(), unc ) == 0 )
      {
        options = &m_options_list[i];
        break;
      }
      else
      {
        continue;
      }
    }
    else if ( m_options_list.at( i ).type() == Smb4KCustomOptions::Share )
    {
      if ( QString::compare( m_options_list.at( i ).share()->unc(), unc ) == 0 )
      {
        options = &m_options_list[i];
        break;
      }
      else
      {
        continue;
      }
    }
    else
    {
      continue;
    }
  }
  
  return options;
}


void Smb4KSambaOptions::removeEditWidgets()
{
  for ( int i = 0; i < m_custom_options->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *top_level_item = m_custom_options->topLevelItem( i );
      
    for ( int j = 0; j < m_custom_options->columnCount(); ++j )
    {
      QWidget *edit_widget = m_custom_options->itemWidget( top_level_item, j );
        
      if ( edit_widget )
      {
        m_custom_options->removeItemWidget( top_level_item, j );
        delete edit_widget;
      }
      else
      {
        // Do nothing
      }
    }
  }
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


void Smb4KSambaOptions::slotEditCustomItem( QTreeWidgetItem *item, int column )
{  
  // First of all, get rid of all open edit widgets, because we
  // only want to have one open at a time.
  removeEditWidgets();

  if ( item )
  {
    switch ( item->type() )
    {
      case Host:
      {
        switch ( column )
        {
          case Protocol:
          {
            QStringList choices;
            choices.append( i18n( "Automatic" ) );
            choices.append( "RPC" );
            choices.append( "RAP" );
            choices.append( "ADS" );

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;
          }
          case Kerberos:
          {
            QStringList choices;
            choices.append( i18n( "yes" ) );
            choices.append( i18n( "no" ) );

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;
          }
          case SMBPort:
          {
            KIntNumInput *input = new KIntNumInput( m_custom_options );
            input->setAutoFillBackground( true );
            input->setMinimum( 0 );
            input->setMaximum( 65535 );
            input->setValue( item->text( column ).toInt() );
            m_custom_options->setItemWidget( item, column, input );
            input->adjustSize();

            connect( input, SIGNAL( valueChanged( int ) ),
                     this,  SLOT( slotCustomIntValueChanged( int ) ) );
            break;
          }
#ifndef Q_OS_FREEBSD
          case FileSystemPort:
          {
            KIntNumInput *input = new KIntNumInput( m_custom_options );
            input->setAutoFillBackground( true );
            input->setMinimum( 0 );
            input->setMaximum( 65535 );
            input->setValue( item->text( column ).toInt() );
            m_custom_options->setItemWidget( item, column, input );
            input->adjustSize();

            connect( input, SIGNAL( valueChanged( int ) ),
                     this,  SLOT( slotCustomIntValueChanged( int ) ) );
            break;
          }
          case WriteAccess:
          {
            QStringList choices;
            choices.append( i18n( "read-write" ) );
            choices.append( i18n( "read-only" ) );

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;            
          }
#endif
          case UID:
          {
            QList<KUser> user_list = KUser::allUsers();
            QStringList choices;

            for ( int i = 0; i < user_list.size(); ++i )
            {
              choices.append( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                                  .arg( user_list.at( i ).uid() ) );
            }

            choices.sort();

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;
          }
          case GID:
          {
            QList<KUserGroup> group_list = KUserGroup::allGroups();
            QStringList choices;

            for ( int i = 0; i < group_list.size(); ++i )
            {
              choices.append( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                                  .arg( group_list.at( i ).gid() ) );
            }

            choices.sort();

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;
          }
          default:
          {
            break;
          }
        }

        break;
      }
      case Share:
      {
        switch ( column )
        {
#ifndef Q_OS_FREEBSD
          case WriteAccess:
          {
            QStringList choices;
            choices.append( i18n( "read-write" ) );
            choices.append( i18n( "read-only" ) );

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;
          }
          case FileSystemPort:
          {
            KIntNumInput *input = new KIntNumInput( m_custom_options );
            input->setAutoFillBackground( true );
            input->setMinimum( 0 );
            input->setMaximum( 65535 );
            input->setValue( item->text( column ).toInt() );
            m_custom_options->setItemWidget( item, column, input );
            input->adjustSize();

            connect( input, SIGNAL( valueChanged( int ) ),
                     this,  SLOT( slotCustomIntValueChanged( int ) ) );
            break;
          }
#else
          case SMBPort:
          {
            KIntNumInput *input = new KIntNumInput( m_custom_options );
            input->setAutoFillBackground( true );
            input->setMinimum( 0 );
            input->setMaximum( 65535 );
            input->setValue( item->text( column ).toInt() );
            m_custom_options->setItemWidget( item, column, input );
            input->adjustSize();

            connect( input, SIGNAL( valueChanged( int ) ),
                     this,  SLOT( slotCustomIntValueChanged( int ) ) );
            break;
          }
#endif
          case UID:
          {
            QList<KUser> user_list = KUser::allUsers();
            QStringList choices;

            for ( int i = 0; i < user_list.size(); ++i )
            {
              choices.append( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                                  .arg( user_list.at( i ).uid() ) );
            }

            choices.sort();

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;
          }
          case GID:
          {
            QList<KUserGroup> group_list = KUserGroup::allGroups();
            QStringList choices;

            for ( int i = 0; i < group_list.size(); ++i )
            {
              choices.append( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                                  .arg( group_list.at( i ).gid() ) );
            }

            choices.sort();

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );
            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );
            break;
          }
          default:
          {
            break;
          }
        }

        break;
      }
      default:
      {
        break;
      }
    }

    for ( int i = 0; i < m_custom_options->columnCount(); ++i )
    {
      m_custom_options->resizeColumnToContents( i );
    }

    m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptions::slotCustomTextChanged( const QString &text )
{
  // A value changed. Set m_maybe_changed to TRUE.
  m_maybe_changed = true;
  
  // Since only one edit widget will be open at a time, we can just 
  // loop through all entries and get the combo box.
  for ( int i = 0; i < m_custom_options->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *top_level_item = m_custom_options->topLevelItem( i );
    KComboBox *combo = NULL;
      
    for ( int j = 0; j < m_custom_options->columnCount(); ++j )
    {
      combo = static_cast<KComboBox *>( m_custom_options->itemWidget( top_level_item, j ) );
        
      if ( combo && QString::compare( combo->currentText(), text, Qt::CaseSensitive ) == 0 )
      {
        top_level_item->setText( j, text );
        m_custom_options->removeItemWidget( top_level_item, j );
        
        Smb4KCustomOptions *options = findOptions( top_level_item->text( ItemName ) );
         
        if ( options )
        {
          // Now update the info.
          switch ( j )
          {
            case Protocol:
            {
              if ( QString::compare( text, i18n( "Automatic" ) ) == 0 )
              {
                options->setProtocolHint( Smb4KCustomOptions::Automatic );
              }
              else if ( QString::compare( text, "RPC" ) == 0 )
              {
                options->setProtocolHint( Smb4KCustomOptions::RPC );
              }
              else if ( QString::compare( text, "RAP" ) == 0 )
              {
                options->setProtocolHint( Smb4KCustomOptions::RAP );
              }
              else if ( QString::compare( text, "ADS" ) == 0 )
              {
                options->setProtocolHint( Smb4KCustomOptions::ADS );
              }
              else
              {
                options->setProtocolHint( Smb4KCustomOptions::UndefinedProtocolHint );
              }
              break;
            }
#ifndef Q_OS_FREEBSD
            case WriteAccess:
            {
              if ( QString::compare( text, i18n( "read-write" ) ) == 0 )
              {
                options->setWriteAccess( Smb4KCustomOptions::ReadWrite );
              }
              else if ( QString::compare( text, i18n( "read-only" ) ) == 0 )
              {
                options->setWriteAccess( Smb4KCustomOptions::ReadOnly );
              }
              else
              {
                options->setWriteAccess( Smb4KCustomOptions::UndefinedWriteAccess );
              }
              break;
            }
#endif
            case Kerberos:
            {
              if ( QString::compare( text, i18n( "yes" ) ) == 0 )
              {
                options->setUseKerberos( Smb4KCustomOptions::UseKerberos );
              }
              else if ( QString::compare( text, i18n( "no" ) ) == 0 )
              {
                options->setUseKerberos( Smb4KCustomOptions::NoKerberos );
              }
              else
              {
                options->setUseKerberos( Smb4KCustomOptions::UndefinedKerberos );
              }
              break;
            }
            case UID:
            {
              QString uid = text.section( "(", 1, 1 ).section( ")", 0, 0 ).trimmed();
              options->setUID( (K_UID)uid.toInt() );
              break;
            }
            case GID:
            {
              QString gid = text.section( "(", 1, 1 ).section( ")", 0, 0 ).trimmed();
              options->setGID( (K_GID)gid.toInt() );
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
          // Do nothing
        }
          
        break;
      }
      else
      {
        continue;
      }
    }
    
    if ( combo )
    {
      delete combo;
      break;
    }
    else
    {
      continue;
    }
  }  
  
  for ( int i = 0; i < m_custom_options->columnCount(); ++i )
  {
    m_custom_options->resizeColumnToContents( i );
  }

  m_custom_options->sortItems( ItemName, Qt::AscendingOrder );

  // A value changed. Set m_maybe_changed to TRUE and emit 
  // customSettingsModified().
  m_maybe_changed = true;
  emit customSettingsModified();
}


void Smb4KSambaOptions::slotCustomIntValueChanged( int value )
{
  // The edit widget will be removed by the eventFilter()
  // function when the user presses Return (or Enter).

  // Find the KIntNumInput widget. Since only one edit widget 
  // will be open at a time, we can just loop through all 
  // entries and get the combo box.
  for ( int i = 0; i < m_custom_options->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *top_level_item = m_custom_options->topLevelItem( i );
    KIntNumInput *input = NULL;
      
    for ( int j = 0; j < m_custom_options->columnCount(); ++j )
    {
      input = static_cast<KIntNumInput *>( m_custom_options->itemWidget( top_level_item, j ) );
      
      if ( input && input->value() == value )
      {
        top_level_item->setText( j, QString( "%1" ).arg( value ) );

        Smb4KCustomOptions *options = findOptions( top_level_item->text( ItemName ) );
          
        if ( options )
        {
          // Now update the info.
          switch ( j )
          {
            case SMBPort:
            {
              options->setSMBPort( value );
              break;
            }
#ifndef Q_OS_FREEBSD
            case FileSystemPort:
            {
              options->setFileSystemPort( value );
              break;
            }
#endif
            default:
            {
              break;
            }
          }
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
  }
}


void Smb4KSambaOptions::slotItemSelectionChanged()
{
  // First of all, get rid of all open edit widgets, because we
  // only want to have one open at a time.
  removeEditWidgets();
}


void Smb4KSambaOptions::slotCustomContextMenuRequested( const QPoint &pos )
{
  m_menu->menu()->popup( m_custom_options->viewport()->mapToGlobal( pos ) );
}


void Smb4KSambaOptions::slotEditActionTriggered( bool /*checked*/ )
{
  slotEditCustomItem( m_custom_options->currentItem(), m_custom_options->currentColumn() );
}


void Smb4KSambaOptions::slotRemoveActionTriggered( bool /*checked*/ )
{
  // Delete selected items.
  QList<QTreeWidgetItem *> selected_items = m_custom_options->selectedItems();
  
  while ( !selected_items.isEmpty() )
  {
    Smb4KCustomOptions *options = findOptions( selected_items.first()->text( ItemName ) );
    
    if ( options )
    {
      int index = m_options_list.indexOf( *options );
      m_options_list.removeAt( index );
    }
    else
    {
      // Do nothing
    }
    
    delete selected_items.takeFirst();
  }
  
  // Adjust the rows and columns.
  for ( int i = 0; i < m_custom_options->columnCount(); ++i )
  {
    m_custom_options->resizeColumnToContents( i );
  }

  m_custom_options->sortItems( ItemName, Qt::AscendingOrder );

  // An item has been removed. Set m_maybe_changed to TRUE and
  // emit customSettingsModified().
  m_maybe_changed = true;
  emit customSettingsModified();
}


void Smb4KSambaOptions::slotClearActionTriggered( bool /*checked*/ )
{
  while ( !m_options_list.isEmpty() )
  {
    m_options_list.removeFirst();
  }
  
  m_custom_options->clear();

  // The list of items has been cleared. Set m_maybe_changed to TRUE
  // and emit customSettingsModified().
  m_maybe_changed = true;
  emit customSettingsModified();
}


#include "smb4ksambaoptions.moc"
