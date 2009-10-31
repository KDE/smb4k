/***************************************************************************
    smb4ksynchronizationdialog  -  The synchronization dialog of Smb4K
                             -------------------
    begin                : Sa Mai 19 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSYNCHRONIZATIONDIALOG_H
#define SMB4KSYNCHRONIZATIONDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>
#include <QProgressBar>
#include <QLabel>

// KDE includes
#include <kdialog.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kdemacros.h>

// forward declarations
class Smb4KShare;
class Smb4KSynchronizationInfo;

/**
 * This class provides a synchronization dialog. It contains URL requesters
 * for the source and destination as well as widgets to monitor the process
 * of the synchronization.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KSynchronizationDialog : public KDialog
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
    Smb4KSynchronizationDialog( Smb4KShare *share, QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSynchronizationDialog();

  protected slots:
    /**
     * This slot is called when the User1 button is clicked.
     * It initializes the synchronization.
     */
    void slotUser1Clicked();

    /**
     * This slot is called when the User2 button is clicked.
     * It swaps the source and destination.
     */
    void slotUser2Clicked();

    /**
     * This slot is called when the Cancel button is clicked.
     * It aborts any action the synchronizer is performing.
     */
    void slotUser3Clicked();

    /**
     * This slot receives information about the progress of the
     * current synchronization and puts it into the dialog.
     *
     * @param info          Information about the progress of the
     *                      current synchronization process.
     */
    void slotProgress( Smb4KSynchronizationInfo *info );

    /**
     * This slot is invoked when the synchronization is about to be started.
     * It is connected to the Smb4KSynchronizer::aboutToStart() signal.
     *
     * @param info          The Smb4KSynchronizationInfo that represents
     *                      the synchronization process.
     */
    void slotSynchronizationAboutToStart( Smb4KSynchronizationInfo *info );

    /**
     * This slot is invoked when the synchronization finished. It is
     * connected to the Smb4KSynchronizer::finished() signal.
     *
     * @param info          The Smb4KSynchronizationInfo that represents
     *                      the synchronization process.
     */
    void slotSynchronizationFinished( Smb4KSynchronizationInfo *info );

  private:
    /**
     * A pointer to the share object
     */
    Smb4KShare *m_share;

    /**
     * The source URL requester
     */
    KUrlRequester *m_source;

    /**
     * The destination URL requester
     */
    KUrlRequester *m_destination;

    /**
     * The line edit where the file is shown that's currently
     * transferred
     */
    KLineEdit *m_current_file;

    /**
     * The progress bar that shows the progress of the current
     * transfer
     */
    QProgressBar *m_current_progress;

    /**
     * The progress bar that shows the total progress
     */
    QProgressBar *m_total_progress;

    /**
     * The widget that holds information about transfer rate and
     * considered files
     */
    QWidget *m_transfer_widget;

    /**
     * Transferred files
     */
    QLabel *m_transferred_files;

    /**
     * The transfer rate
     */
    QLabel *m_transfer_rate;

    /**
     * The internal Smb4KSynchronizationInfo object.
     */
    Smb4KSynchronizationInfo *m_info;

    /**
     * Was the abort button pressed?
     */
    bool m_aborted;
};

#endif
