/*
 *  Handles autostart capabilities for Smb4K
 *
 *  SPDX-FileCopyrightText: 2026 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kautostartmanager.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Qt includes
#include <QApplicationStatic>
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QStandardPaths>

class Smb4KAutoStartManagerPrivate
{
public:
    QFileSystemWatcher watcher;
    QString autostartFolderPath;
    QString autostartFilePath;
};

class Smb4KAutoStartManagerStatic
{
public:
    Smb4KAutoStartManager instance;
};

Q_APPLICATION_STATIC(Smb4KAutoStartManagerStatic, p);

Smb4KAutoStartManager::Smb4KAutoStartManager(QObject *parent)
    : QObject(parent)
    , d(new Smb4KAutoStartManagerPrivate)
{
    d->autostartFolderPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/autostart/");
    d->autostartFilePath = d->autostartFolderPath + QStringLiteral("org.kde.smb4k.desktop");
}

Smb4KAutoStartManager::~Smb4KAutoStartManager() noexcept
{
}

Smb4KAutoStartManager *Smb4KAutoStartManager::self()
{
    return &p->instance;
}

void Smb4KAutoStartManager::init()
{
    if (QFile::exists(d->autostartFilePath)) {
        Smb4KSettings::setAutostartApplication(true);
        d->watcher.addPath(d->autostartFilePath);
    } else {
        Smb4KSettings::setAutostartApplication(false);
    }

    connect(&d->watcher, &QFileSystemWatcher::fileChanged, this, &Smb4KAutoStartManager::slotAutoStartFileChanged);
}

void Smb4KAutoStartManager::enableAutoStart(bool on)
{
    if (Smb4KSettings::autostartApplication() == on) {
        return;
    }

    if (on) {
        QDir autostartFolder(d->autostartFolderPath);

        if (!autostartFolder.exists()) {
            autostartFolder.mkpath(QStringLiteral("."));
        }

        QString desktopFilePath =
            QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QStringLiteral("org.kde.smb4k.desktop"), QStandardPaths::LocateFile);

        if (!desktopFilePath.isEmpty() && QFile::exists(desktopFilePath)) {
            QFile::copy(desktopFilePath, d->autostartFilePath);
        } else {
            Smb4KNotification::fileNotFound(QStringLiteral("org.kde.smb4k.desktop"));
        }
    } else {
        QFile::remove(d->autostartFilePath);
    }
}

void Smb4KAutoStartManager::slotAutoStartFileChanged(const QString &path)
{
    Q_UNUSED(path);

    if (Smb4KSettings::autostartApplication() && QFile::exists(d->autostartFilePath)) {
        return;
    }

    Smb4KSettings::setAutostartApplication(false);
    Smb4KSettings::self()->save();
}
