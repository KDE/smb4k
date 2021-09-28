/*
    smb4kprofilesmenu  -  The menu for the profiles
    -------------------
    begin                : Do Aug 10 2014
    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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

#ifndef SMB4KPROFILESMENU_H
#define SMB4KPROFILESMENU_H

// Qt include
#include <QString>
#include <QStringList>

// KDE includes
#include <KWidgetsAddons/KSelectAction>

class Smb4KProfilesMenu : public KSelectAction
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KProfilesMenu(QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~Smb4KProfilesMenu();

    /**
     * Force the menu to be set up again. This should be called if
     * the settings changed and the handling of bookmarks might be
     * affected.
     */
    void refreshMenu();

protected Q_SLOTS:
    void slotActiveProfileChanged(const QString &newProfile);
    void slotProfilesListChanged(const QStringList &profiles);
    void slotProfileUsageChanged(bool use);
    void slotActionTriggered(const QString &name);
};

#endif
