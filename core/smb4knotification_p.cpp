/*
    These are the private helper classes of the Smb4KNotification
    namespace.

    SPDX-FileCopyrightText: 2014-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knotification_p.h"

// KDE includes
#include <KIO/OpenUrlJob>

Smb4KNotifier::Smb4KNotifier(const QString &event)
    : KNotification(event, KNotification::CloseOnTimeout)
{
    if (event == QStringLiteral("shareMounted")) {
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
    KIO::OpenUrlJob *job = new KIO::OpenUrlJob(m_mountpoint, QStringLiteral("inode/directory"));
    job->setFollowRedirections(false);
    job->setAutoDelete(true);
    job->start();
}
