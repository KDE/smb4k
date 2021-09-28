/*
    This is the new synchronizer of Smb4K.
    -------------------
    begin                : Fr Feb 04 2011
    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksynchronizer.h"
#include "smb4kglobal.h"
#include "smb4knotification.h"
#include "smb4kshare.h"
#include "smb4ksynchronizer_p.h"

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTimer>

// KDE includes
#include <KCoreAddons/KShell>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KSynchronizerStatic, p);

Smb4KSynchronizer::Smb4KSynchronizer(QObject *parent)
    : KCompositeJob(parent)
    , d(new Smb4KSynchronizerPrivate)
{
    setAutoDelete(false);
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}

Smb4KSynchronizer::~Smb4KSynchronizer()
{
}

Smb4KSynchronizer *Smb4KSynchronizer::self()
{
    return &p->instance;
}

void Smb4KSynchronizer::synchronize(const SharePtr &share)
{
    if (!isRunning(share)) {
        // Create a new job, add it to the subjobs and register it
        // with the job tracker.
        Smb4KSyncJob *job = new Smb4KSyncJob(this);
        job->setObjectName(QString("SyncJob_%1").arg(share->canonicalPath()));
        job->setupSynchronization(share);

        connect(job, SIGNAL(result(KJob *)), SLOT(slotJobFinished(KJob *)));
        connect(job, SIGNAL(aboutToStart(QString)), SIGNAL(aboutToStart(QString)));
        connect(job, SIGNAL(finished(QString)), SIGNAL(finished(QString)));

        addSubjob(job);

        job->start();
    }
}

bool Smb4KSynchronizer::isRunning()
{
    return hasSubjobs();
}

bool Smb4KSynchronizer::isRunning(const SharePtr &share)
{
    bool running = false;

    for (int i = 0; i < subjobs().size(); ++i) {
        if (QString::compare(QString("SyncJob_%1").arg(share->canonicalPath()), subjobs().at(i)->objectName()) == 0) {
            running = true;
            break;
        } else {
            continue;
        }
    }

    return running;
}

void Smb4KSynchronizer::abort(const SharePtr &share)
{
    if (share && !share.isNull()) {
        for (KJob *job : subjobs()) {
            if (QString("SyncJob_%1").arg(share->canonicalPath()) == job->objectName()) {
                job->kill(KJob::EmitResult);
                break;
            }
        }
    } else {
        QListIterator<KJob *> it(subjobs());

        while (it.hasNext()) {
            it.next()->kill(KJob::EmitResult);
        }
    }
}

void Smb4KSynchronizer::start()
{
    QTimer::singleShot(0, this, SLOT(slotStartJobs()));
}

/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSynchronizer::slotStartJobs()
{
    // FIXME: Not implemented yet. I do not see a use case at the moment.
}

void Smb4KSynchronizer::slotJobFinished(KJob *job)
{
    // Remove the job.
    removeSubjob(job);
}

void Smb4KSynchronizer::slotAboutToQuit()
{
    abort();
}
