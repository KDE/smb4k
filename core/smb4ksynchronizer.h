/*
    This is the new synchronizer of Smb4K.

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSYNCHRONIZER_H
#define SMB4KSYNCHRONIZER_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QScopedPointer>
#include <QString>

// KDE includes
#include <KCoreAddons/KCompositeJob>

// forward declarations
class Smb4KSynchronizerPrivate;

class Q_DECL_EXPORT Smb4KSynchronizer : public KCompositeJob
{
    Q_OBJECT

    friend class Smb4KSynchronizerPrivate;

public:
    /**
     * The constructor
     */
    explicit Smb4KSynchronizer(QObject *parent = nullptr);

    /**
     * The destructor
     */
    ~Smb4KSynchronizer();

    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KSynchronizer class.
     */
    static Smb4KSynchronizer *self();

    /**
     * Sets the URL for the source and destination by showing the user a
     * dialog for URL input. The URLs are then passed to the actual job that
     * does all the work.
     *
     * @param share         The Smb4KShare object
     */
    void synchronize(const SharePtr &share);

    /**
     * This function tells you whether the synchronizer is running
     * or not.
     *
     * @returns TRUE if the synchronizer is running and FALSE otherwise
     */
    bool isRunning();

    /**
     * With this function you can test whether a synchronization job
     * for a certain share @param share is already running.
     *
     * @returns TRUE if a synchronization process is already running
     */
    bool isRunning(const SharePtr &share);

    /**
     * This function either aborts the synchronization for a certain
     * mounted share, if a valid pointer is passed, or aborts all running
     * processes.
     *
     * @param share         The Smb4KShare object
     */
    void abort(const SharePtr &share = SharePtr());

    /**
     * This function starts the composite job
     */
    void start() override;

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

protected Q_SLOTS:
    /**
     * Invoked by start() function
     */
    void slotStartJobs();

    /**
     * Invoked when a job finished
     */
    void slotJobFinished(KJob *job);

    /**
     * Invoked when the application goes down
     */
    void slotAboutToQuit();

private:
    /**
     * Pointer to Smb4KSearchPrivate class
     */
    const QScopedPointer<Smb4KSynchronizerPrivate> d;
};

#endif
