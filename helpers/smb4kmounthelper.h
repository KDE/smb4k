/*
    The helper that mounts and unmounts shares.
    -------------------
    begin                : Sa Okt 16 2010
    SPDX-FileCopyrightText: 2010-2020 Alexander Reinholdt
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

#ifndef SMB4KMOUNTHELPER_H
#define SMB4KMOUNTHELPER_H

// Qt includes
#include <QObject>

// KDE includes
#include <KAuth/KAuthActionReply>

using namespace KAuth;

class Smb4KMountHelper : public QObject
{
    Q_OBJECT

public slots:
    /**
     * Mounts a CIFS/SMBFS share.
     */
    KAuth::ActionReply mount(const QVariantMap &args);

    /**
     * Unmounts a CIFS/SMBFS share.
     */
    KAuth::ActionReply unmount(const QVariantMap &args);
};

#endif
