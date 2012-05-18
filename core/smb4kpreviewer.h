/***************************************************************************
    smb4kpreviewer  -  This class queries a remote share for a preview
                             -------------------
    begin                : Sa Mär 05 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KPREVIEWER_H
#define SMB4KPREVIEWER_H

// Qt includes
#include <QtCore/QUrl>
#include <QtCore/QScopedPointer>
#include <QtGui/QWidget>

// KDE includes
#include <kdemacros.h>
#include <kcompositejob.h>

// forward declarations
class Smb4KPreviewerPrivate;
class Smb4KShare;
class Smb4KPreviewJob;
class Smb4KPreviewDialog;

/**
 * This class acquires previews from defined shares.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class KDE_EXPORT Smb4KPreviewer : public KCompositeJob
{
  Q_OBJECT

  friend class Smb4KPreviewerPrivate;

  public:
    /**
     * Constructor
     */
    Smb4KPreviewer( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KPreviewer();
    
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KPreviewer class.
     */
    static Smb4KPreviewer *self();

    /**
     * Previews the contents of the given share @param share. This 
     * function will just return if you pass a printer share.
     *
     * @param share       The share for which a preview should be
     *                    generated
     *
     * @param parent      The parent widget
     */
    void preview( Smb4KShare *share,
                  QWidget *parent = 0 );

    /**
     * This function tells you whether preview jobs are running
     * or not.
     *
     * @returns TRUE if at least one preview job is running
     */
    bool isRunning();

    /**
     * With this function you can test whether a preview job for a certain
     * share @param share is already running.
     *
     * @returns TRUE if a preview job is already running
     */
    bool isRunning( Smb4KShare *share );

    /**
     * This function aborts all print jobs at once.
     */
    void abortAll();

    /**
     * This function aborts the acquisition of a preview for a certain
     * remote share.
     *
     * Only use this function if you have no access to a widget, that
     * tracks the job.
     *
     * @param share         The Smb4KShare object
     */
    void abort( Smb4KShare *share );

    /**
     * This function starts the composite job
     */
    void start();

  Q_SIGNALS:
    /**
     * Emitted when the acquistition process is about to begin.
     * 
     * @param item          The Smb4KShare item
     *
     * @param url           The location for which the preview should be
     *                      acquired
     */
    void aboutToStart( Smb4KShare *share,
                       const QUrl &url );

    /**
     * Emitted after the acquisition process finished.
     * 
     * @param item          The Smb4KShare item
     *
     * @param url           The location for which the preview should be
     *                      acquired
     */
    void finished( Smb4KShare *share,
                   const QUrl &url );

  protected Q_SLOTS:
    /**
     * Invoked by start() function
     */
    void slotStartJobs();

    /**
     * Called when a job finished
     */
    void slotJobFinished( KJob *job );

    /**
     * Called when an authentication error occurred
     */
    void slotAuthError( Smb4KPreviewJob *job );

    /**
     * Called when a preview dialog is closed
     */
    void slotDialogClosed( Smb4KPreviewDialog *dialog );

    /**
     * This slot starts the acquisition of a preview. It is
     * invoked by the preview dialog.
     * 
     * @param share       The remote share
     *
     * @param url         The location
     *
     * @param parent      The parent widget
     */
    void slotAcquirePreview( Smb4KShare *share,
                             const QUrl &url,
                             QWidget *parent );

    /**
     * This slot kills the acquisition of the preview for the
     * share @p share.
     *
     * @param share       The remote share
     */
    void slotAbortPreview( Smb4KShare *share );

    /**
     * Called when the application exits
     */
    void slotAboutToQuit();

  private:
    /**
     * Pointer to the Smb4KPreviewerPrivate class
     */
    const QScopedPointer<Smb4KPreviewerPrivate> d;
};

#endif
