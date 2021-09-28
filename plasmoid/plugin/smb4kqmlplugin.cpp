/*
    smb4kqmlplugin - The QML plugin for use with Plasma/QtQuick
    -------------------
    begin                : Di Feb 21 2012
    SPDX-FileCopyrightText: 2012-2021 Alexander Reinholdt
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

// application specific includes
#include "smb4kqmlplugin.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kglobal.h"
#include "smb4kbookmarkobject.h"
#include "smb4kdeclarative.h"
#include "smb4knetworkobject.h"
#include "smb4kprofileobject.h"

// Qt includes
#include <QQmlComponent>

void Smb4KQMLPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<Smb4KNetworkObject>(uri, 2, 0, "NetworkObject");
    qmlRegisterType<Smb4KBookmarkObject>(uri, 2, 0, "BookmarkObject");
    qmlRegisterType<Smb4KProfileObject>(uri, 2, 0, "ProfileObject");
    qmlRegisterType<Smb4KDeclarative>(uri, 2, 0, "Interface");
}
