/*
    smb4kqmlplugin - The QML plugin for use with Plasma/QtQuick

    SPDX-FileCopyrightText: 2012-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kqmlplugin.h"
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
