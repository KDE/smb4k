/***************************************************************************
    smb4klaptopsupportoptionspage  -  The configuration page for the laptop
    support of Smb4K
                             -------------------
    begin                : Mi Sep 17 2008
    copyright            : (C) 2008-2013 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4klaptopsupportoptionspage.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>

// KDE includes
#include <klocale.h>
#include <kiconloader.h>


Smb4KLaptopSupportOptionsPage::Smb4KLaptopSupportOptionsPage( QWidget *parent )
: QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
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

  layout->addWidget( hardware );
  layout->addWidget( note );
  layout->addStretch( 100 );
}


Smb4KLaptopSupportOptionsPage::~Smb4KLaptopSupportOptionsPage()
{
}

#include "smb4klaptopsupportoptionspage.moc"
