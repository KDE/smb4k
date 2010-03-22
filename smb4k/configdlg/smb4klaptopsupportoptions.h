/***************************************************************************
    smb4klaptopsupportoptions  -  The configuration page for the laptop
    support of Smb4K
                             -------------------
    begin                : Mi Sep 17 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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

#ifndef SMB4KLAPTOPSUPPORTOPTIONS_H
#define SMB4KLAPTOPSUPPORTOPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>

/**
 * This class provides the configuration page for the laptop
 * support in Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KLaptopSupportOptions : public QWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor
     */
    Smb4KLaptopSupportOptions( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KLaptopSupportOptions();
};

#endif
