/*
    This file contains private helper classes for the Smb4KSynchronizer
    class.

    SPDX-FileCopyrightText: 2008-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSYNCHRONIZER_P_H
#define SMB4KSYNCHRONIZER_P_H

// application specific includes
#include "smb4kglobal.h"
#include "smb4ksynchronizer.h"

// Qt includes
#include <QDialog>
#include <QPushButton>
#include <QUrl>

// KDE includes
#include <KCoreAddons/KJob>
#include <KCoreAddons/KProcess>
#include <KIOWidgets/KUrlRequester>
#include <KJobWidgets/KUiServerJobTracker>

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
     * @param share     The share
     */
    void setupSynchronization(const SharePtr &share);

    /**
     * Returns the source directory.
     *
     * @returns the source directory.
     */
    const QUrl &source()
    {
        return m_sourceUrl;
    }

    /**
     * Returns the destination directory.
     *
     * @returns the destination directory.
     */
    const QUrl &destination()
    {
        return m_destinationUrl;
    }

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
    SharePtr m_share;
    QUrl m_sourceUrl;
    QUrl m_destinationUrl;
    KProcess *m_process;
    KUiServerJobTracker *m_job_tracker;
    bool m_terminated;
};

class Smb4KSynchronizationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * The constructor
     *
     * @param share         The share item
     *
     * @param parent        The parent widget
     */
    explicit Smb4KSynchronizationDialog(const SharePtr &share, QWidget *parent = nullptr);

    /**
     * The destructor
     */
    ~Smb4KSynchronizationDialog();

    /**
     * The source URL
     */
    const QUrl source();

    /**
     * The destination URL
     */
    const QUrl destination();

protected Q_SLOTS:
    void slotCancelClicked();
    void slotSynchronizeClicked();
    void slotSwapPathsClicked();

private:
    QPushButton *m_swap_button;
    QPushButton *m_synchronize_button;
    QPushButton *m_cancel_button;

    /**
     * A pointer to the share object
     */
    SharePtr m_share;

    /**
     * The source URL requester
     */
    KUrlRequester *m_source;

    /**
     * The destination URL requester
     */
    KUrlRequester *m_destination;
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
