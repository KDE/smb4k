/*
    These are the private helper classes of the Smb4KNotification
    namespace.
    -------------------
    begin                : So Jun 22 2014
    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knotification_p.h"

// KDE includes
#include <KIO/OpenUrlJob>
#include <kio_version.h>

Smb4KNotifier::Smb4KNotifier(const QString &event)
    : KNotification(event, KNotification::CloseOnTimeout)
{
    if (event == "shareMounted") {
        connect(this, SIGNAL(activated(uint)), SLOT(slotOpenShare()));
    }
}

Smb4KNotifier::~Smb4KNotifier()
{
}

void Smb4KNotifier::setMountpoint(const QUrl &mountpoint)
{
    m_mountpoint = mountpoint;
}

QUrl Smb4KNotifier::mountpoint() const
{
    return m_mountpoint;
}

void Smb4KNotifier::slotOpenShare()
{
    KIO::OpenUrlJob *job = new KIO::OpenUrlJob(m_mountpoint, "inode/directory");
    job->setFollowRedirections(false);
    job->setAutoDelete(true);
    job->start();
}
