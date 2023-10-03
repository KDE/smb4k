/*
    This file contains private helper classes for the Smb4KSynchronizer
    class.

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSYNCHRONIZER_P_H
#define SMB4KSYNCHRONIZER_P_H

// application specific includes
#include "smb4kglobal.h"
#include "smb4ksynchronizer.h"

// Qt includes
#include <QUrl>

// KDE includes
#include <KJob>
#include <KProcess>
#include <KUiServerJobTracker>

class Smb4KSyncJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KSyncJob(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KSyncJob();

    /**
     * Starts the synchronization
     */
    void start() override;

    /**
     * Setup the synchronization process. This function must be
     * called before start() is run.
     *
     * @param sourceUrl         The source URL
     *
     * @param destinationUrl    The destination URL
     */
    void setupSynchronization(const QUrl &sourceUrl, const QUrl &destinationUrl);

Q_SIGNALS:
    /**
     * This signal is emitted when a job is started. The emitted path
     * is the one of the destination.
     *
     * @param dest        The destination's URL
     */
    void aboutToStart(const QString &dest);

    /**
     * This signal is emitted when a job has finished. The emitted
     * URL is the one of the destination.
     *
     * @param dest        The destination's URL
     */
    void finished(const QString &dest);

protected:
    /**
     * Reimplemented from KJob. Kills the internal process and
     * then the job itself.
     */
    bool doKill() override;

protected Q_SLOTS:
    void slotStartSynchronization();
    void slotReadStandardOutput();
    void slotReadStandardError();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QUrl m_sourceUrl;
    QUrl m_destinationUrl;
    KProcess *m_process;
    KUiServerJobTracker *m_jobTracker;
    bool m_terminated;
};

class Smb4KSynchronizerPrivate
{
};

class Smb4KSynchronizerStatic
{
public:
    Smb4KSynchronizer instance;
};

#endif
