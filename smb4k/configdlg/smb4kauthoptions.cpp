/***************************************************************************
    smb4kauthoptions  -  The configuration page for the authentication
    settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2008 by Alexander Reinholdt
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
#include <QCheckBox>
#include <QLabel>

// KDE includes
#include <klocale.h>
#include <klineedit.h>

// application specific includes
#include "smb4kauthoptions.h"
#include <core/smb4ksettings.h>

Smb4KAuthOptions::Smb4KAuthOptions( QWidget *parent )
: QWidget( parent )
{
  //
  // Default Authentication
  //
  QGridLayout *layout      = new QGridLayout( this );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  // Password storage box
  QGroupBox *password_box  = new QGroupBox( i18n( "Password Storage" ), this );

  QGridLayout *pass_layout = new QGridLayout( password_box );
  pass_layout->setSpacing( 5 );

  QCheckBox *use_wallet    = new QCheckBox( Smb4KSettings::self()->useWalletItem()->label(),
                             password_box );
  use_wallet->setObjectName( "kcfg_UseWallet" );

  QCheckBox *remember_pass = new QCheckBox( Smb4KSettings::self()->rememberLoginsItem()->label(),
                             password_box );
  remember_pass->setObjectName( "kcfg_RememberLogins" );

  pass_layout->addWidget( use_wallet, 0, 0, 0 );
  pass_layout->addWidget( remember_pass, 1, 0, 0 );

  // Default login box
  QGroupBox *default_box   = new QGroupBox( i18n( "Default Login" ), this );
  default_box->setObjectName( "DefaultLoginBox" );

  QGridLayout *def_layout  = new QGridLayout( default_box );
  def_layout->setSpacing( 5 );

  QCheckBox *default_auth  = new QCheckBox( Smb4KSettings::self()->useDefaultLoginItem()->label(),
                             default_box );
  default_auth->setObjectName( "kcfg_UseDefaultLogin" );

  QLabel *user             = new QLabel( i18n( "User:" ), default_box );
  user->setObjectName( "DefaultUserNameLabel" );
  KLineEdit *default_user  = new KLineEdit( default_box );
  default_user->setObjectName( "DefaultUserName" );
  default_user->setMinimumWidth( 150 );
  default_user->setWhatsThis( i18n( "This username is used by default to authenticate to a remote server." ) );

  QLabel *password         = new QLabel( i18n( "Password:" ), default_box );
  password->setObjectName( "DefaultPasswordLabel" );
  KLineEdit *default_pass  = new KLineEdit( default_box );
  default_pass->setObjectName( "DefaultPassword" );
  default_pass->setEchoMode( KLineEdit::Password );
  default_pass->setMinimumWidth( 150 );
  default_pass->setWhatsThis( i18n( "This password is used by default to authenticate to a remote server. It may be empty." ) );

  def_layout->addWidget( default_auth, 0, 0, 1, 2, 0 );
  def_layout->addWidget( user, 1, 0, 0 );
  def_layout->addWidget( default_user, 1, 1, 1, 1, 0 );
  def_layout->addWidget( password, 2, 0, 0 );
  def_layout->addWidget( default_pass, 2, 1, 1, 1, 0 );

  QSpacerItem *spacer = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  layout->addWidget( password_box, 0, 0, 0 );
  layout->addWidget( default_box, 1, 0, 0 );
  layout->addItem( spacer, 2, 0, 1, 1, 0 );

  connect( use_wallet,   SIGNAL( toggled( bool ) ),
           this,         SLOT( slotKWalletButtonToggled( bool ) ) );

  connect( default_auth, SIGNAL( toggled( bool ) ),
           this,         SLOT( slotDefaultLoginToggled( bool ) ) );

  slotKWalletButtonToggled( use_wallet->isChecked() );
  slotDefaultLoginToggled( default_auth->isChecked() );
}


Smb4KAuthOptions::~Smb4KAuthOptions()
{
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KAuthOptions::slotKWalletButtonToggled( bool checked )
{
  findChild<QGroupBox *>( "DefaultLoginBox" )->setEnabled( checked );
}


void Smb4KAuthOptions::slotDefaultLoginToggled( bool checked )
{
  findChild<QLabel *>( "DefaultUserNameLabel" )->setEnabled( checked );
  findChild<KLineEdit *>( "DefaultUserName" )->setEnabled( checked );
  findChild<QLabel *>( "DefaultPasswordLabel" )->setEnabled( checked );
  findChild<KLineEdit *>( "DefaultPassword" )->setEnabled( checked );
}

#include "smb4kauthoptions.moc"
