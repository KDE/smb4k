/***************************************************************************
    smb4ksuperuseroptions  -  The configuration page for the super user
    settings of Smb4K
                             -------------------
    begin                : Sa Okt 30 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
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

#ifndef SMB4KSUPERUSEROPTIONS_H
#define SMB4KSUPERUSEROPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>

/**
 * This is the configuration tab where the user can determine
 * whether he/she wants to use the program super or sudo to
 * gain super user privileges.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSuperUserOptions : public QWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param parent      The parent widget
     */
    Smb4KSuperUserOptions( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSuperUserOptions();

  signals:
    /**
     * This signal is emitted when the "Remove Entries" button has been
     * clicked. It is provided for convenience. You could also connect
     * to the clicked() signal.
     */
    void removeEntries();

  protected slots:
    /**
     * This slot is activated when the "Remove Entries" button has been
     * clicked.
     *
     * @param checked    TRUE if the button is checked and FALSE otherwise
     *                   (not used here).
     */
    void slotRemoveClicked( bool checked );
};

#endif
