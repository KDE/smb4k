/***************************************************************************
    This is the new synchronizer of Smb4K.
                             -------------------
    begin                : Fr Feb 04 2011
    copyright            : (C) 2011-2021 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

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
    explicit Smb4KSynchronizer(QObject *parent = 0);

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
