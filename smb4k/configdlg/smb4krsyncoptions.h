/***************************************************************************
    smb4krsyncoptions  -  The configuration page for the rsync options
                             -------------------
    begin                : So Nov 20 2005
    copyright            : (C) 2005-2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KRSYNCOPTIONS_H
#define SMB4KRSYNCOPTIONS_H

// KDE includes
#include <ktabwidget.h>


/**
 * This class belongs to the configuration dialog and takes
 * care of the options that can be defined for rsync.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KRsyncOptions : public KTabWidget
{
  Q_OBJECT

  public:
    /**
     * Tab enumeration
     */
    enum Tabs{ CopyingTab = 0,
               DelTransTab = 1,
               FilteringTab = 2,
               AdvancedTab = 3 };
    
    /**
     * The constructor.
     *
     * @param parent        The parent widget
     */
    explicit Smb4KRsyncOptions( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KRsyncOptions();
    
  protected slots:
    /**
     * This slot is invoked if the "Archive mode" checkbox has been
     * toggled.
     *
     * @param checked       Is TRUE if the checkbox is checked and FALSE otherwise.
     */
    void slotArchiveToggled( bool checked );

    /**
     * This slot is invoked if the --archive option has to be switched
     * off.
     *
     * @param checked       Is FALSE if one of the connected checkboxes is unchecked
     *                      and TRUE otherwise.
     */
    void slotUncheckArchive( bool checked );

    /**
     * This slot is called, when the backup checkbox has been toggled.
     * It enables/disables all other backup options according to the
     * state the backup button is in.
     *
     * @param checked       Is TRUE if the m_backup check box has been
     *                      checked and FALSE otherwise.
     */
    void slotBackupToggled( bool checked );

    /**
     * This slot is called if the '-F' shortcut has been toggled.
     * It unchecks the '-F -F' shortcut.
     *
     * @param checked       Is TRUE is m_f_filter is checked and FALSE otherwise.
     */
    void slotFShortcutToggled( bool checked );

    /**
     * This slot is called if the '-F -F' shortcut has been toggled.
     * It unchecks the '-F' shortcut.
     *
     * @param checked       Is TRUE is m_ff_filter is checked and FALSE otherwise.
     */
    void slotFFShortcutToggled( bool checked );
};

#endif
