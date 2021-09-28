/*
    smb4kqmlplugin - The QML plugin for use with Plasma/QtQuick
    -------------------
    begin                : Di Feb 21 2012
    SPDX-FileCopyrightText: 2012-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
