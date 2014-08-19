/***************************************************************************
    smb4kprofilespage  -  The configuration page for the profiles
                             -------------------
    begin                : Do Aug 07 2014
    copyright            : (C) 2014 by Alexander Reinholdt
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

#ifndef SMB4KPROFILESPAGE_H
#define SMB4KPROFILESPAGE_H

// Qt includes
#include <QWidget>

// KDE includes
#include <keditlistwidget.h>


class Smb4KProfilesPage : public QWidget
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KProfilesPage(QWidget *parent = 0);
    
    /**
     * Destructor
     */
    virtual ~Smb4KProfilesPage();

  protected Q_SLOTS:
    void slotEnableWidget(int state);

  private:
    KEditListWidget *m_profiles;
};

#endif
