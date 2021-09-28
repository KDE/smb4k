/*
    These are the private helper classes of the Smb4KNotification
    namespace.
    -------------------
    begin                : So Jun 22 2014
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

#ifndef SMB4KNOTIFICATION_P_H
#define SMB4KNOTIFICATION_P_H

// Qt includes
#include <QObject>
#include <QUrl>

// KDE includes
#include <KNotifications/KNotification>

class Smb4KNotifier : public KNotification
{
    Q_OBJECT

public:
    Smb4KNotifier(const QString &event);
    ~Smb4KNotifier();
    void setMountpoint(const QUrl &mountpoint);
    QUrl mountpoint() const;

public Q_SLOTS:
    void slotOpenShare();

private:
    QUrl m_mountpoint;
};

#endif
