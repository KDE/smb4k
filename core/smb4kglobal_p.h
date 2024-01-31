/*
    These are the private helper classes of the Smb4KGlobal namespace.

    SPDX-FileCopyrightText: 2007-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KGLOBAL_P_H
#define SMB4KGLOBAL_P_H

// application specific includes
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

// Qt includes
#include <QFileSystemWatcher>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSharedPointer>

/**
 * This class is a private helper for the Smb4KGlobal namespace.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KGlobalPrivate : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    Smb4KGlobalPrivate();

    /**
     * Destructor
     */
    ~Smb4KGlobalPrivate();

    /**
     * This is the global workgroup list.
     */
    QList<QSharedPointer<Smb4KWorkgroup>> workgroupsList;

    /**
     * This is the global host list.
     */
    QList<QSharedPointer<Smb4KHost>> hostsList;

    /**
     * This is global list of mounted shares.
     */
    QList<QSharedPointer<Smb4KShare>> mountedSharesList;

    /**
     * This is the global list of shares.
     */
    QList<QSharedPointer<Smb4KShare>> sharesList;

    /**
     * Boolean that is TRUE when only foreign shares
     * are in the list of mounted shares
     */
    bool onlyForeignShares;

#ifdef Q_OS_LINUX
    /**
     * This list contains all allowed arguments for the mount.cifs binary and
     * is only present under the Linux operating system.
     */
    QStringList allowedMountArguments;
#endif

    /**
     * The machine's NetBIOS name
     */
    QString machineNetbiosName;

    /**
     * The machine's workgroup name
     */
    QString machineWorkgroupName;

protected Q_SLOTS:
    /**
     * This slot does last things before the application quits
     */
    void slotAboutToQuit();
};

#endif
