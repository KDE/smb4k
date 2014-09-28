/***************************************************************************
    smb4kprofilemanager_p  -  Private helper class(es) for the profile 
    manager.
                             -------------------
    begin                : Mi Aug 12 2014
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

#ifndef SMB4KPROFILEMANAGER_P_H
#define SMB4KPROFILEMANAGER_P_H

// Application specific includes
#include "smb4kprofilemanager.h"

// Qt includes
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QWidget>

// KDE includes
#include <kdialog.h>
#include <kcombobox.h>


class Smb4KProfileMigrationDialog : public KDialog
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KProfileMigrationDialog(const QStringList &from,
                                         const QStringList &to,
                                         QWidget* parent = 0);
    
    /**
     * Destructor
     */
    virtual ~Smb4KProfileMigrationDialog();
    
    /**
     * Returns the name of the profile of which the entries 
     * should be migrated.
     * 
     * @returns the name of the profile.
     */
    QString from() const;
    
    /**
     * Returns the name of the profile where the entries should
     * be migrated to.
     * 
     * @returns the name of the new profile.
     */
    QString to() const;
    
  protected Q_SLOTS:
    void slotOkClicked();
    
  private:
    void setupView();
    QStringList m_from_list;
    QStringList m_to_list;
    KComboBox *m_from_box;
    KComboBox *m_to_box;
};


class Smb4KProfileManagerPrivate
{
  public:
    QString activeProfile;
    QStringList profiles;
    bool useProfiles;
};


class Smb4KProfileManagerStatic
{
  public:
    Smb4KProfileManager instance;
};


#endif
