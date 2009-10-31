/***************************************************************************
    smb4klaptopsupportoptions  -  The configuration page for the laptop
    support of Smb4K
                             -------------------
    begin                : Mi Sep 17 2008
    copyright            : (C) 2008 by Alexander Reinholdt
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
#include <kiconloader.h>

// application specific includes
#include <smb4klaptopsupportoptions.h>
#include <smb4ksettings.h>

Smb4KLaptopSupportOptions::Smb4KLaptopSupportOptions( QWidget *parent )
: QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  // Unmounting due to changes in state of hardware
  QGroupBox *hardware = new QGroupBox( i18n( "Hardware" ), this );

  QGridLayout *hardware_layout = new QGridLayout( hardware );
  hardware_layout->setSpacing( 5 );

  QCheckBox *sleep_button = new QCheckBox(
                            Smb4KSettings::self()->unmountWhenSleepButtonPressedItem()->label(),
                            hardware );
  sleep_button->setObjectName( "kcfg_UnmountWhenSleepButtonPressed" );

  QCheckBox *lid_button = new QCheckBox(
                          Smb4KSettings::self()->unmountWhenLidButtonPressedItem()->label(),
                          hardware );
  lid_button->setObjectName( "kcfg_UnmountWhenLidButtonPressed" );

  QCheckBox *power_button = new QCheckBox(
                            Smb4KSettings::self()->unmountWhenPowerButtonPressedItem()->label(),
                            hardware );
  power_button->setObjectName( "kcfg_UnmountWhenPowerButtonPressed" );

  QCheckBox *network = new QCheckBox(
                       Smb4KSettings::self()->unmountWhenNetworkDisconnectedItem()->label(),
                       hardware );
  network->setObjectName( "kcfg_UnmountWhenNetworkDisconnected" );

  hardware_layout->addWidget( sleep_button, 0, 0, 0 );
  hardware_layout->addWidget( lid_button, 1, 0, 0 );
  hardware_layout->addWidget( power_button, 2, 0, 0 );
  hardware_layout->addWidget( network, 3, 0, 0 );

  QFrame *note                 = new QFrame( this );

  QGridLayout *note_layout     = new QGridLayout( note );
  note_layout->setSpacing( 10 );
  note_layout->setMargin( 5 );

  QLabel *important_pix        = new QLabel( note );
  important_pix->setPixmap( KIconLoader::global()->loadIcon( "emblem-important", KIconLoader::Desktop, KIconLoader::SizeMedium ) );
  important_pix->adjustSize();

  QLabel *message              = new QLabel( note );
  message->setText( i18n( "<qt>Smb4K only detects changes in state of your hardware. If you are performing a software suspend or the like, you should unmount the shares manually.</qt>" ) );
  message->setTextFormat( Qt::AutoText );
  message->setWordWrap( true );
  message->setAlignment( Qt::AlignJustify );

  note_layout->addWidget( important_pix, 0, 0, Qt::AlignCenter );
  note_layout->addWidget( message, 0, 1, Qt::AlignVCenter );

  note_layout->setColumnStretch( 1, 1 );

  QSpacerItem *spacer = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  layout->addWidget( hardware, 0, 0, 0 );
  layout->addWidget( note, 1, 0, 0 );
  layout->addItem( spacer, 2, 0 );
}


Smb4KLaptopSupportOptions::~Smb4KLaptopSupportOptions()
{
}

#include "smb4klaptopsupportoptions.moc"
