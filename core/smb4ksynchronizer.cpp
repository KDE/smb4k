/*
    This is the new synchronizer of Smb4K.

    SPDX-FileCopyrightText: 2011-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksynchronizer.h"
#include "smb4kglobal.h"
#include "smb4knotification.h"
#include "smb4kshare.h"
#include "smb4ksynchronizer_p.h"

// Qt includes
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

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

void Smb4KSynchronizer::synchronize(const QUrl &sourceUrl, const QUrl &destinationUrl)
{
    if (!isRunning(sourceUrl)) {
        Smb4KSyncJob *job = new Smb4KSyncJob(this);
        job->setObjectName(QStringLiteral("SyncJob_") + sourceUrl.toLocalFile());
        job->setupSynchronization(sourceUrl, destinationUrl);

        connect(job, &Smb4KSyncJob::result, this, &Smb4KSynchronizer::slotJobFinished);
        connect(job, &Smb4KSyncJob::aboutToStart, this, &Smb4KSynchronizer::aboutToStart);
        connect(job, &Smb4KSyncJob::finished, this, &Smb4KSynchronizer::finished);

        addSubjob(job);

        job->start();
    }
}

bool Smb4KSynchronizer::isRunning()
{
    return hasSubjobs();
}

bool Smb4KSynchronizer::isRunning(const QUrl &sourceUrl)
{
    bool running = false;

    QListIterator<KJob *> it(subjobs());

    while (it.hasNext()) {
        if (it.next()->objectName() == QStringLiteral("SyncJob_") + sourceUrl.toLocalFile()) {
            running = true;
            break;
        }
    }

    return running;
}

void Smb4KSynchronizer::abort(const QUrl &sourceUrl)
{
    if (!sourceUrl.isEmpty() && sourceUrl.isValid()) {
        QListIterator<KJob *> it(subjobs());

        while (it.hasNext()) {
            KJob *job = it.next();

            if (QStringLiteral("SyncJob_") + sourceUrl.toLocalFile() == job->objectName()) {
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
