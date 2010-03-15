/***************************************************************************
    smb4ksuperuseroptions  -  The configuration page for the super user
    settings of Smb4K
                             -------------------
    begin                : Sa Okt 30 2004
    copyright            : (C) 2004-2010 by Alexander Reinholdt
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

// KDE includes
#include <klocale.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <kcombobox.h>

// application specific includes
#include "smb4ksuperuseroptions.h"
#include <core/smb4ksettings.h>


Smb4KSuperUserOptions::Smb4KSuperUserOptions( QWidget *parent )
: QWidget( parent )
{
  QGridLayout *layout           = new QGridLayout( this );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  // Actions
  QGroupBox *actions_box        = new QGroupBox( i18n( "Actions" ), this );

  QGridLayout *actions_layout   = new QGridLayout( actions_box );
  actions_layout->setSpacing( 5 );

#ifdef Q_OS_LINUX
  QCheckBox *force_unmounting   = new QCheckBox( Smb4KSettings::self()->useForceUnmountItem()->label(),
                                  actions_box );
  force_unmounting->setObjectName( "kcfg_UseForceUnmount" );
#endif
  QCheckBox *use_privileges     = new QCheckBox( Smb4KSettings::self()->alwaysUseSuperUserItem()->label(),
                                  actions_box );
  use_privileges->setObjectName( "kcfg_AlwaysUseSuperUser" );

#ifdef Q_OS_LINUX
  actions_layout->addWidget( force_unmounting, 0, 0, 0 );
  actions_layout->addWidget( use_privileges, 1, 0, 0 );
#else
  actions_layout->addWidget( use_privileges, 0, 0, 0 );
#endif

  KGuiItem remove_item          = KGuiItem(  i18n( "Remove Entries" ), "edit-clear",
                                  i18n( "Remove entries from the configuration file" ),
                                  i18n( "All entries that were written by Smb4K will be removed from /etc/sudoers. Additionally, all your choices under \"Actions\" will be cleared." ) );

  KPushButton *remove           = new KPushButton( remove_item, this );

  QSpacerItem *spacer = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  layout->addWidget( actions_box, 0, 0, 1, 6, 0 );
  layout->addWidget( remove, 1, 5, 0 );
  layout->addItem( spacer, 2, 0, 1, 6, 0 );
  
  connect( remove, SIGNAL( clicked( bool ) ),
           this,   SLOT( slotRemoveClicked( bool ) ) );
}


Smb4KSuperUserOptions::~Smb4KSuperUserOptions()
{
}


bool Smb4KSuperUserOptions::writeSuperUserEntries()
{
  return (
#ifdef Q_OS_LINUX
          (forceUnmountingStateChanged() && 
           findChild<QCheckBox *>( "kcfg_UseForceUnmount" ) ->isChecked()) ||
#endif
          (alwaysUseSuperUserStateChanged() && 
           findChild<QCheckBox *>( "kcfg_AlwaysUseSuperUser" )->isChecked()));
}


void Smb4KSuperUserOptions::resetSuperUserSettings()
{
#ifdef Q_OS_LINUX
  findChild<QCheckBox *>( "kcfg_UseForceUnmount" )->setChecked( false );
#endif
  findChild<QCheckBox *>( "kcfg_AlwaysUseSuperUser" )->setChecked( false );
}


#ifdef Q_OS_LINUX
bool Smb4KSuperUserOptions::forceUnmountingStateChanged()
{
  return (findChild<QCheckBox *>( "kcfg_UseForceUnmount" )->isChecked() != Smb4KSettings::useForceUnmount());
}
#endif


bool Smb4KSuperUserOptions::alwaysUseSuperUserStateChanged()
{
  return (findChild<QCheckBox *>( "kcfg_AlwaysUseSuperUser" )->isChecked() != Smb4KSettings::alwaysUseSuperUser());
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSuperUserOptions::slotRemoveClicked( bool /*checked*/ )
{
  resetSuperUserSettings();  
  emit removeEntries();
}

#include "smb4ksuperuseroptions.moc"
