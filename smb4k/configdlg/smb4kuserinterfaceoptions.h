/***************************************************************************
    smb4kuserinterfaceoptions  -  This configuration page takes care
    of all settings concerning the user interface of Smb4K
                             -------------------
    begin                : Mi Aug 30 2006
    copyright            : (C) 2006-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500,        *
 *   Boston, MA 02110-1335, USA                                            *
 ***************************************************************************/

#ifndef SMB4KUSERINTERFACEOPTIONS_H
#define SMB4KUSERINTERFACEOPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>

// KDE includes
#include <ktabwidget.h>


/**
 * The configuration page for the user interface of Smb4K.
 *
 * @author Alexander Reinholdt  <dustpuppy@mail.berlios.de>
 */

class Smb4KUserInterfaceOptions : public KTabWidget
{
  Q_OBJECT

  public:
    /**
     * Tabs enumeration
     */
    enum Tabs{ MainWindowSystemTrayTab = 0,
               NetworkNeighborhoodTab = 1,
               MountedSharesTab = 2,
               PreviewDialogTab = 3 };
    /**
     * The constructor
     *
     * @param parent          The parent widget of this class.
     */
    Smb4KUserInterfaceOptions( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KUserInterfaceOptions();
    
  protected slots:
    /**
     * Enables/disables buttons according to the toggle state of the
     * "Show hidden shares" button.
     *
     * @param checked         TRUE if the check box is checked.
     */
    void slotShowHiddenClicked( bool checked );
};

#endif
