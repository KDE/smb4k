/*
    The helper that mounts and unmounts shares.
    -------------------
    begin                : Sa Okt 16 2010
    SPDX-FileCopyrightText: 2010-2020 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
