/***************************************************************************
    smb4khomesshareshandler_p  -  Private helpers for the homes shares
    handler.
                             -------------------
    begin                : Mo Apr 11 2011
    copyright            : (C) 2011-2014 by Alexander Reinholdt
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
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4khomesshareshandler_p.h"
#include "smb4ksettings.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

// KDE includes
#include <klocale.h>


Smb4KHomesUsers::Smb4KHomesUsers( const Smb4KShare &s, const QStringList &u )
{
  m_workgroup_name = s.workgroupName();
  m_host_name      = s.hostName();
  m_share_name     = s.shareName();
  m_host_ip.setAddress(s.hostIP());
  m_users          = u;
}


Smb4KHomesUsers::Smb4KHomesUsers( const Smb4KHomesUsers &u )
{
  m_workgroup_name = u.workgroupName();
  m_host_name      = u.hostName();
  m_share_name     = u.shareName();
  m_host_ip.setAddress(u.hostIP());
  m_users          = u.users();
  m_profile        = u.profile();
}


Smb4KHomesUsers::Smb4KHomesUsers()
{
}


Smb4KHomesUsers::~Smb4KHomesUsers()
{
}


QString Smb4KHomesUsers::workgroupName() const
{
  return m_workgroup_name;
}


void Smb4KHomesUsers::setWorkgroupName(const QString& name)
{
  m_workgroup_name = name;
}


QString Smb4KHomesUsers::hostName() const
{
  return m_host_name;
}


void Smb4KHomesUsers::setHostName(const QString& name)
{
  m_host_name = name;
}


QString Smb4KHomesUsers::shareName() const
{
  return m_share_name;
}


void Smb4KHomesUsers::setShareName(const QString& name)
{
  m_share_name = name;
}


QString Smb4KHomesUsers::hostIP() const
{
  return m_host_ip.toString();
}


void Smb4KHomesUsers::setHostIP(const QString& ip)
{
  m_host_ip.setAddress(ip);
}


QStringList Smb4KHomesUsers::users() const
{
  return m_users;
}


void Smb4KHomesUsers::setUsers(const QStringList& users)
{
  m_users = users;
}


QString Smb4KHomesUsers::profile() const
{
  return m_profile;
}


void Smb4KHomesUsers::setProfile(const QString& profile)
{
  m_profile = profile;
}



Smb4KHomesUserDialog::Smb4KHomesUserDialog( QWidget *parent ) : KDialog( parent )
{
  setCaption( i18n( "Specify User" ) );
  setButtons( KDialog::User1|KDialog::Ok|KDialog::Cancel );
  setDefaultButton( KDialog::Ok );
  setButtonGuiItem( KDialog::User1, KGuiItem( i18n( "Clear List" ), "edit-clear", 0, 0 ) );
  enableButton( KDialog::Ok, false );
  enableButton( KDialog::User1, false );
  
  setupView();
  
  connect( m_user_combo, SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)) );
  connect( m_user_combo->lineEdit(), SIGNAL(editingFinished()), SLOT(slotHomesUserEntered()) );
  connect( this, SIGNAL(user1Clicked()), SLOT(slotClearClicked()) );
  connect( this, SIGNAL(okClicked()), SLOT(slotOkClicked()) );
  
  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );
  
  KConfigGroup group( Smb4KSettings::self()->config(), "HomesUserDialog" );
  restoreDialogSize( group );
  m_user_combo->completionObject()->setItems( group.readEntry( "HomesUsersCompletion", QStringList() ) );
}


Smb4KHomesUserDialog::~Smb4KHomesUserDialog()
{
}


void Smb4KHomesUserDialog::setupView()
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
  QPixmap user_pix = KIcon( "user-identity" ).pixmap( KIconLoader::SizeHuge );
  pixmap->setPixmap( user_pix );
  pixmap->setAlignment( Qt::AlignBottom );

  QLabel *label = new QLabel( i18n( "Please specify a username." ), description );
  label->setWordWrap( true );
  label->setAlignment( Qt::AlignBottom );

  desc_layout->addWidget( pixmap, 0 );
  desc_layout->addWidget( label, Qt::AlignBottom );
  
  QWidget *input = new QWidget( main_widget );
  
  QGridLayout *input_layout = new QGridLayout( input );
  input_layout->setSpacing( 5 );
  input_layout->setMargin( 0 );
  input_layout->setColumnStretch( 0, 0 );
  input_layout->setColumnStretch( 1, 1 );
  
  QLabel *input_label = new QLabel( i18n( "User:" ), input );

  m_user_combo = new KComboBox( true, input );
  m_user_combo->setDuplicatesEnabled( false );
  m_user_combo->setEditable( true );
  
  input_layout->addWidget( input_label, 0, 0, 0 );
  input_layout->addWidget( m_user_combo, 0, 1, 0 );

  layout->addWidget( description, 0 );
  layout->addWidget( input, 0 );
  
  m_user_combo->setFocus();
}


void Smb4KHomesUserDialog::setUserNames( const QStringList &users )
{
  if ( !users.isEmpty() )
  {
    m_user_combo->addItems( users );
    m_user_combo->setCurrentItem( "" );
    enableButton( KDialog::User1, true );
  }
  else
  {
    // Do nothing
  }
}


QStringList Smb4KHomesUserDialog::userNames()
{
  QStringList users;
  
  for ( int i = 0; i < m_user_combo->count(); ++i )
  {
    users << m_user_combo->itemText( i );
  }
  
  if ( !users.contains( m_user_combo->currentText() ) )
  {
    users << m_user_combo->currentText();
  }
  else
  {
    // Do nothing
  }
  
  return users;
}


void Smb4KHomesUserDialog::slotTextChanged( const QString &text )
{
  if ( !text.isEmpty() )
  {
    enableButtonOk( true );
  }
  else
  {
    enableButtonOk( false );
  }
}


void Smb4KHomesUserDialog::slotClearClicked()
{
  m_user_combo->clearEditText();
  m_user_combo->clear();
  enableButton( KDialog::User1, false );
}


void Smb4KHomesUserDialog::slotOkClicked()
{
  KConfigGroup group( Smb4KSettings::self()->config(), "HomesUserDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
  group.writeEntry( "HomesUsersCompletion", m_user_combo->completionObject()->items() );
}


void Smb4KHomesUserDialog::slotHomesUserEntered()
{
  KCompletion *completion = m_user_combo->completionObject();

  if ( !m_user_combo->currentText().isEmpty() )
  {
    completion->addItem( m_user_combo->currentText() );
  }
  else
  {
    // Do nothing
  }
}

#include "smb4khomesshareshandler_p.moc"
