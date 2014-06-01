/***************************************************************************
    smb4ksambaoptions.cpp  -  This is the configuration page for the
    Samba settings of Smb4K
                             -------------------
    begin                : Mo Jan 26 2004
    copyright            : (C) 2004-2014 by Alexander Reinholdt
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

#ifndef SMB4KSAMBAOPTIONSPAGE_H
#define SMB4KSAMBAOPTIONSPAGE_H

// Qt includes
#include <QList>
#include <QListWidgetItem>

// KDE includes
#include <ktabwidget.h>

/**
 * This class manages the configuration dialog of the options
 * that can be passed to smbmount and other programs of the
 * Samba software suite.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */


class Smb4KSambaOptionsPage : public KTabWidget
{
  Q_OBJECT

  public:
    enum Tabs{ GeneralTab = 0,
               MountingTab = 1,
               ClientProgramsTab = 2 };

    /**
     * The constructor.
     *
     * @param parent            The parent widget
     *
     * @param name              This widget's name
     */
    explicit Smb4KSambaOptionsPage( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KSambaOptionsPage();

  protected slots:
    /**
     * Sets the new general user ID.
     *
     * @param action              The action that represents the new user.
     */
    void slotNewUserTriggered( QAction *action );

    /**
     * Sets the new general group ID.
     *
     * @param action              The action that represents the new group.
     */
    void slotNewGroupTriggered( QAction *action );
    
    /**
     * This slot is activated when the entry in the "Additional CIFS
     * Options" line edit changed.
     * 
     * @param options             The options string
     */
    void slotAdditionalCIFSOptionsChanged( const QString &options );
};

#endif
