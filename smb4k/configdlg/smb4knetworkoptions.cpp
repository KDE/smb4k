/***************************************************************************
    smb4knetworkoptions  -  The configuration page for the network
    settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2010 by Alexander Reinholdt
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
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QCheckBox>

// KDE includes
#include <klocale.h>
#include <klineedit.h>
#include <kcombobox.h>

// application specific includes
#include "smb4knetworkoptions.h"
#include <core/smb4ksettings.h>

Smb4KNetworkOptions::Smb4KNetworkOptions( QWidget *parent )
: QWidget( parent )
{
  QGridLayout *layout               = new QGridLayout( this );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  //
  // The browse list group box.
  //
  QGroupBox *browse_list_box        = new QGroupBox( i18n( "Browse List" ), this );

  QGridLayout *browse_box_layout    = new QGridLayout( browse_list_box );
  browse_box_layout->setSpacing( 5 );

  QButtonGroup *browse_list_buttons = new QButtonGroup( browse_list_box );

  QRadioButton *lookup_workgroups   = new QRadioButton( Smb4KSettings::self()->lookupDomainsItem()->label(),
                                      browse_list_box );
  lookup_workgroups->setObjectName( "kcfg_LookupDomains" );

  QRadioButton *query_current       = new QRadioButton( Smb4KSettings::self()->queryCurrentMasterItem()->label(),
                                      browse_list_box );
  query_current->setObjectName( "kcfg_QueryCurrentMaster" );

  QRadioButton *query_custom        = new QRadioButton( Smb4KSettings::self()->queryCustomMasterItem()->label(),
                                      browse_list_box );
  query_custom->setObjectName( "kcfg_QueryCustomMaster" );

  KLineEdit *custom_name            = new KLineEdit( browse_list_box );
  custom_name->setObjectName( "kcfg_CustomMasterBrowser" );

  QRadioButton *scan_broadcast      = new QRadioButton( Smb4KSettings::self()->scanBroadcastAreasItem()->label(),
                                      browse_list_box );
  scan_broadcast->setObjectName( "kcfg_ScanBroadcastAreas" );

  KLineEdit *broadcast_areas        = new KLineEdit( browse_list_box );
  broadcast_areas->setObjectName( "kcfg_BroadcastAreas" );

  browse_list_buttons->addButton( lookup_workgroups );
  browse_list_buttons->addButton( query_current );
  browse_list_buttons->addButton( query_custom );
  browse_list_buttons->addButton( scan_broadcast );

  browse_box_layout->addWidget( lookup_workgroups, 0, 0, 1, 3, 0 );
  browse_box_layout->addWidget( query_current, 1, 0, 1, 3, 0 );
  browse_box_layout->addWidget( query_custom, 2, 0, 0 );
  browse_box_layout->addWidget( custom_name, 2, 1, 1, 2, 0 );
  browse_box_layout->addWidget( scan_broadcast, 3, 0, 0 );
  browse_box_layout->addWidget( broadcast_areas, 3, 1, 1, 2, 0 );

  //
  // The authentication group box.
  //
  QGroupBox *auth_box               = new QGroupBox( i18n( "Authentication" ), this );

  QGridLayout *auth_box_layout      = new QGridLayout( auth_box );
  auth_box_layout->setSpacing( 5 );

  QCheckBox *master_browser_auth    = new QCheckBox( Smb4KSettings::self()->masterBrowsersRequireAuthItem()->label(),
                                      auth_box );
  master_browser_auth->setObjectName( "kcfg_MasterBrowsersRequireAuth" );

  auth_box_layout->addWidget( master_browser_auth, 0, 0, 0 );

  QSpacerItem *spacer = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  layout->addWidget( browse_list_box, 0, 0, 0 );
  layout->addWidget( auth_box, 1, 0, 0 );
  layout->addItem( spacer, 2, 0 );
}


Smb4KNetworkOptions::~Smb4KNetworkOptions()
{
}


#include "smb4knetworkoptions.moc"
