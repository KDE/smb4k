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

#ifndef SMB4KQMLPLUGIN_H
#define SMB4KQMLPLUGIN_H

// Qt includes
#include <QQmlExtensionPlugin>
#include <QtPlugin>

class Smb4KQMLPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.smb4k.smb4kqmlplugin")

public:
    void registerTypes(const char *uri) override;
};

#endif
