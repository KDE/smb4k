/*
    The helper that mounts and unmounts shares.

    SPDX-FileCopyrightText: 2010-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KMOUNTHELPER_H
#define SMB4KMOUNTHELPER_H

// Qt includes
#include <QDBusUnixFileDescriptor>
#include <QObject>

// KDE includes
#include <KAuth/ActionReply>

using namespace KAuth;

class Smb4KMountHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    /**
     * Mounts a CIFS/SMBFS share.
     */
    KAuth::ActionReply mount(const QVariantMap &args);

    /**
     * Unmounts a CIFS/SMBFS share.
     */
    KAuth::ActionReply unmount(const QVariantMap &args);

private:
    bool isOnline() const;
    bool checkMountArguments(QStringList *argList) const;
    bool checkUnmountArguments(QStringList* argList) const;
    QString mountPrefix() const;
    ActionReply errorReply(const QString &message) const;
    std::optional<QString> createMountPoint(const QUrl &url) const;
    void removeMountPoint(const QString &mountPoint) const;
    bool isMountPointAllowed(const QString &mountPoint) const;
    bool checkFileDescriptor(const QDBusUnixFileDescriptor &dbusFd);
};

#endif
