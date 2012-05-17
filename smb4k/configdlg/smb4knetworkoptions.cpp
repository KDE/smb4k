/***************************************************************************
    smb4knetworkoptions  -  The configuration page for the network
    settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2011 by Alexander Reinholdt
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
#include "smb4knetworkoptions.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QVBoxLayout>
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
#include <knuminput.h>


Smb4KNetworkOptions::Smb4KNetworkOptions( QWidget *parent )
: QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
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
  
  // Periodic scaning
  QGroupBox *periodic_box           = new QGroupBox( i18n( "Periodic Scanning" ), this );
  
  QGridLayout *periodic_layout      = new QGridLayout( periodic_box );
  periodic_layout->setSpacing( 5 );
  
  QCheckBox *periodic_scanning      = new QCheckBox( Smb4KSettings::self()->periodicScanningItem()->label(),
                                      periodic_box );
  periodic_scanning->setObjectName( "kcfg_PeriodicScanning" );
  
  QLabel *interval_label            = new QLabel( Smb4KSettings::self()->scanIntervalItem()->label(),
                                      periodic_box );
  
  KIntNumInput *scan_interval       = new KIntNumInput( periodic_box );
  scan_interval->setObjectName( "kcfg_ScanInterval" );
  scan_interval->setSuffix( " min" );
  scan_interval->setSingleStep( 1 );
  scan_interval->setSliderEnabled( true );
  
  periodic_layout->addWidget( periodic_scanning, 0, 0, 1, 2, 0 );
  periodic_layout->addWidget( interval_label, 1, 0, 0 );
  periodic_layout->addWidget( scan_interval, 1, 1, 0 );  

  layout->addWidget( browse_list_box, 0 );
  layout->addWidget( auth_box, 0 );
  layout->addWidget( periodic_box, 0 );
  layout->addStretch( 100 );
}


Smb4KNetworkOptions::~Smb4KNetworkOptions()
{
}


#include "smb4knetworkoptions.moc"
