/*
    These are the private helper classes of the Smb4KGlobal namespace.

    SPDX-FileCopyrightText: 2007-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kglobal_p.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QAbstractSocket>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QHostAddress>
#include <QHostInfo>

Smb4KGlobalPrivate::Smb4KGlobalPrivate()
{
    onlyForeignShares = false;
    coreInitialized = false;

#ifdef Q_OS_LINUX
    //
    // 2023-01-08: Updated to mount.cifs, version 7.0
    //
    // The options 'credentials', 'domainauto', 'intr', 'nointr'
    // and 'snapshot' were not included for various reasons.
    //
    allowedMountArguments << QStringLiteral("idsfromsid");
    allowedMountArguments << QStringLiteral("dynperm");
    allowedMountArguments << QStringLiteral("nostrictsync");
    allowedMountArguments << QStringLiteral("handlecache");
    allowedMountArguments << QStringLiteral("nohandlecache");
    allowedMountArguments << QStringLiteral("handletimeout");
    allowedMountArguments << QStringLiteral("rwpidforward");
    allowedMountArguments << QStringLiteral("mapposix");
    allowedMountArguments << QStringLiteral("hard");
    allowedMountArguments << QStringLiteral("soft");
    allowedMountArguments << QStringLiteral("noacl");
    allowedMountArguments << QStringLiteral("cifsacl");
    allowedMountArguments << QStringLiteral("backupuid");
    allowedMountArguments << QStringLiteral("backupgid");
    allowedMountArguments << QStringLiteral("ignorecase");
    allowedMountArguments << QStringLiteral("nocase");
    allowedMountArguments << QStringLiteral("seal");
    allowedMountArguments << QStringLiteral("rdma");
    allowedMountArguments << QStringLiteral("resilienthandles");
    allowedMountArguments << QStringLiteral("noresilienthandles");
    allowedMountArguments << QStringLiteral("persistenthandles");
    allowedMountArguments << QStringLiteral("nopersistenthandles");
    allowedMountArguments << QStringLiteral("forcemandatorylock");
    allowedMountArguments << QStringLiteral("locallease");
    allowedMountArguments << QStringLiteral("nolease");
    allowedMountArguments << QStringLiteral("sfu");
    allowedMountArguments << QStringLiteral("mfsymlinks");
    allowedMountArguments << QStringLiteral("echo_interval");
    allowedMountArguments << QStringLiteral("posix");
    allowedMountArguments << QStringLiteral("unix");
    allowedMountArguments << QStringLiteral("linux");
    allowedMountArguments << QStringLiteral("noposix");
    allowedMountArguments << QStringLiteral("nounix");
    allowedMountArguments << QStringLiteral("nolinux");
    allowedMountArguments << QStringLiteral("nouser_xattr");
    allowedMountArguments << QStringLiteral("nodfs");
    allowedMountArguments << QStringLiteral("noautotune");
    allowedMountArguments << QStringLiteral("nosharesock");
    allowedMountArguments << QStringLiteral("noblocksend");
    allowedMountArguments << QStringLiteral("rsize");
    allowedMountArguments << QStringLiteral("wsize");
    allowedMountArguments << QStringLiteral("bsize");
    allowedMountArguments << QStringLiteral("max_credits");
    allowedMountArguments << QStringLiteral("fsc");
    allowedMountArguments << QStringLiteral("multiuser");
    allowedMountArguments << QStringLiteral("actimeo");
    allowedMountArguments << QStringLiteral("noposixpaths");
    allowedMountArguments << QStringLiteral("posixpaths");
#endif

    //
    // Create and init the SMB context and read the NetBIOS and
    // workgroup name of this machine.
    //
    SMBCCTX *smbContext = smbc_new_context();

    if (smbContext) {
        smbContext = smbc_init_context(smbContext);

        if (!smbContext) {
            smbc_free_context(smbContext, 1);
        } else {
            machineNetbiosName = QString::fromUtf8(smbc_getNetbiosName(smbContext)).toUpper();
            machineWorkgroupName = QString::fromUtf8(smbc_getWorkgroup(smbContext)).toUpper();
        }
    }

    //
    // Free the SMB context
    //
    smbc_free_context(smbContext, 1);

    //
    // Connections
    //
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}

Smb4KGlobalPrivate::~Smb4KGlobalPrivate()
{
    //
    // Clear the workgroup list
    //
    while (!workgroupsList.isEmpty()) {
        workgroupsList.takeFirst().clear();
    }

    //
    // Clear the host list
    //
    while (!hostsList.isEmpty()) {
        hostsList.takeFirst().clear();
    }

    //
    // Clear the list of mounted shares
    //
    while (!mountedSharesList.isEmpty()) {
        mountedSharesList.takeFirst().clear();
    }

    //
    // Clear the list of shares
    //
    while (!sharesList.isEmpty()) {
        sharesList.takeFirst().clear();
    }
}

void Smb4KGlobalPrivate::slotAboutToQuit()
{
    Smb4KSettings::self()->save();
}
