/***************************************************************************
    The configuration page for the mount options
                             -------------------
    begin                : So Mär 22 2015
    copyright            : (C) 2015-2018 by Alexander Reinholdt
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

#ifndef SMB4KCONFIGPAGEMOUNTING_H
#define SMB4KCONFIGPAGEMOUNTING_H

// Qt includes
#include <QTabWidget>

/**
 * This configuration page contains the mount options
 * 
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 2.0.0
 */

class Smb4KConfigPageMounting : public QTabWidget
{
  Q_OBJECT
  
  public:
    /**
     * The constructor
     */
    explicit Smb4KConfigPageMounting(QWidget* parent = 0);
    
    /**
     * The destructor
     */
    virtual ~Smb4KConfigPageMounting();
    
  protected Q_SLOTS:
    /**
     * Sets the new general user ID.
     *
     * @param action              The action that represents the new user.
     */
    void slotNewUserTriggered(QAction *action);

    /**
     * Sets the new general group ID.
     *
     * @param action              The action that represents the new group.
     */
    void slotNewGroupTriggered(QAction *action);
    
    /**
     * Enable / disable the options that are only necessary when the servers
     * do not support the CIFS Unix extensions.
     * 
     * @param checked             TRUE if the button is checked
     */
    void slotCIFSUnixExtensionsSupport(bool checked);
    
    /**
     * This slot is activated when the additional CIFS options are to be
     * edited (Linux only).
     */
    void slotAdditionalCIFSOptions();
    
    /**
     * This slot is activated when the setting of the character set usage is changed
     * (BSD only).
     */
    void slotCharacterSets(bool on);
    
  private:
    /**
     * Set up the widget
     */
    void setupWidget();
};

#endif
