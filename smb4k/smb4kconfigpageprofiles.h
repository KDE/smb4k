/***************************************************************************
    The configuration page for the profiles
                             -------------------
    begin                : Do Aug 07 2014
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

#ifndef SMB4KCONFIGPAGEPROFILES_H
#define SMB4KCONFIGPAGEPROFILES_H

// Qt includes
#include <QList>
#include <QPair>
#include <QWidget>

// KDE includes
#include <KWidgetsAddons/KEditListWidget>

class Smb4KConfigPageProfiles : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KConfigPageProfiles(QWidget *parent = 0);

    /**
     * Destructor
     */
    virtual ~Smb4KConfigPageProfiles();

    /**
     * This function returns a list of pairs that contains the
     * renamed profiles. The first entry of the pair is the old name
     * of the profile and the second entry is the new name.
     *
     * @returns a list of name pairs
     */
    QList<QPair<QString, QString>> renamedProfiles() const;

    /**
     * Clear the list of renamed profiles.
     */
    void clearRenamedProfiles();

    /**
     * This function returns the list of removed profiles.
     *
     * @returns the removed profiles
     */
    QStringList removedProfiles() const;

    /**
     * Clear the list of removed profiles.
     */
    void clearRemovedProfiles();

protected Q_SLOTS:
    void slotEnableWidget(int state);
    void slotProfileRemoved(const QString &name);
    void slotProfileChanged();

private:
    KEditListWidget *m_profiles;
    QList<QPair<QString, QString>> m_renamed;
    QStringList m_removed;
};

#endif
