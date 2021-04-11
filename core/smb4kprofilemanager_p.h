/***************************************************************************
    Private helper class(es) for the profile manager.
                             -------------------
    begin                : Mi Aug 12 2014
    copyright            : (C) 2014-2021 by Alexander Reinholdt
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
#include <QDialog>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QWidget>

// KDE includes
#include <KCompletion/KComboBox>

class Smb4KProfileMigrationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KProfileMigrationDialog(const QStringList &from, const QStringList &to, QWidget *parent = 0);

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
    QPushButton *m_ok_button;
    QPushButton *m_cancel_button;
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
