/***************************************************************************
    smb4knotification  -  This class provides notifications for Smb4K.
                             -------------------
    begin                : Son Jun 27 2010
    copyright            : (C) 2010-2011 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.org
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

#ifndef SMB4KNOTIFICATION_H
#define SMB4KNOTIFICATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QFile>
#include <QDir>
#include <QProcess>

// KDE includes
#include <kurl.h>

// application specific includes
#include <smb4kshare.h>

// forward declarations
class Smb4KBookmark;
class Smb4KWorkgroup;
class Smb4KHost;

/**
 * This class provides notifications used thoughout Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */


class KDE_EXPORT Smb4KNotification : public QObject
{
  Q_OBJECT

  public:
    /**
     * The constructor
     */
    Smb4KNotification( QObject *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KNotification();

    //
    // Notifications
    //

    /**
     * Notify the user that a share has been mounted.
     *
     * @param share     The share that has been mounted
     */
    void shareMounted( Smb4KShare *share );

    /**
     * Notify the user that a share has been unmounted.
     *
     * @param share     The share that has been unmounted
     */
    void shareUnmounted( Smb4KShare *share );

    /**
     * Notify the user that shares have been remounted.
     *
     * @param total     The number of remounts that were scheduled
     *
     * @param actual    The number of remounts that were actually
     *                  mounted.
     */
    void sharesMounted( int total, int actual );

    /**
     * Notify the user that all shares have been unmounted at once.
     *
     * @param total     The number of unmounts that were scheduled
     *
     * @param actual    The number of unmounts that actually finished
     *                  successfully.
     */
    void allSharesUnmounted( int total, int actual );

    //
    // Warnings
    //

    /**
     * Warn the user that the wallet could not be opened.
     *
     * @param name      The name of the wallet
     */
    void openingWalletFailed( const QString &name );

    /**
     * Warn the user that the logins stored in the wallet could not
     * be accessed.
     */
    void loginsNotAccessible();

    /**
     * Tell the user that the mimetype is not supported and that he/she
     * should convert the file.
     *
     * @param mimetype  The mimetype
     */
    void mimetypeNotSupported( const QString &mimetype );

    /**
     * Tell the user that this bookmark is already present and that it will
     * thus be skipped.
     *
     * @param bookmark  The bookmark
     */
    void bookmarkExists( Smb4KBookmark *bookmark );

    /**
     * Tell the user that the label he/she chose for the bookmark is already
     * being used and that it will be changed automatically.
     *
     * @param bookmark  The bookmark
     */
    void bookmarkLabelInUse( Smb4KBookmark *bookmark );

    /**
     * This warning is shown if the entry of the custom master browser
     * is empty.
     */
    void emptyCustomMasterBrowser();

    //
    // Errors
    //

    /**
     * This error message is shown if the list of workgroups could not
     * be retrieved.
     *
     * @param err_msg   The error message
     */
    void retrievingDomainsFailed( const QString &err_msg );

    /**
     * This error message is shown if the scanning of the broadcast
     * areas failed.
     *
     * @param err_msg   The error message
     */
    void scanningBroadcastAreaFailed( const QString &err_msg );

    /**
     * This error message is shown if the list of servers could not
     * be retrieved.
     *
     * @param err_msg   The error message
     */
    void retrievingServersFailed( Smb4KWorkgroup *workgroup, const QString &err_msg );

    /**
     * This error message is shown if the list of shares could not
     * be retrieved.
     *
     * @param host      The host object
     *
     * @param err_msg   The error message
     */
    void retrievingSharesFailed( Smb4KHost *host, const QString &err_msg );

    /**
     * This error message is shown if the preview could not be
     * retrieved.
     *
     * @param err_meg   The error message
     */
    void retrievingPreviewFailed( Smb4KShare *share, const QString &err_msg );

    /**
     * This error message is shown if the mounting of a share failed.
     *
     * @param share     The share that was to be mounted
     *
     * @param err_msg   The error message
     */
    void mountingFailed( Smb4KShare *share, const QString &err_msg );

    /**
     * This error message is shown if the unmounting of a share failed.
     *
     * @param share     The share that was to be unmounted
     *
     * @param err_msg   The error message
     */
    void unmountingFailed( Smb4KShare *share, const QString &err_msg );

    /**
     * This error message is shown if the unmounting of a certain share
     * is not allowed for the user.
     *
     * @param share     The share that was to be unmounted
     */
    void unmountingNotAllowed( Smb4KShare *share );

    /**
     * This error message is shown if printing failed.
     *
     * @param printer   The printer share
     *
     * @param err_msg   The error message
     */
    void printingFailed( Smb4KShare *printer, const QString &err_msg );

    /**
     * This error message is shown if the synchronization failed.
     *
     * @param src       The source URL
     *
     * @param dest      The destination URL
     *
     * @param err_msg   The error message
     */
    void synchronizationFailed( const KUrl &src,
                                const KUrl &dest,
                                const QString &err_msg );

    /**
     * This error message is shown if the searching of the network
     * neighborhood failed.
     *
     * @param item      The search item
     *
     * @param err_msg   The error message
     */
    void searchingFailed( const QString &item, const QString &err_msg );

    /**
     * This error message is shown if a command could not be found.
     *
     * @param command   The command that could not be found
     */
    void commandNotFound( const QString &command );

    /**
     * This error message is shown if the user tried to bookmark a printer.
     *
     * @param share     The Smb4KShare object
     */
    void cannotBookmarkPrinter( Smb4KShare *share );

    /**
     * This error message is shown if a file could not be found.
     *
     * @param fileName  The file name
     */
    void fileNotFound( const QString &fileName );

    /**
     * This error message is shown if a file could not be opened.
     *
     * @param file      The QFile object
     */
    void openingFileFailed( const QFile &file );

    /**
     * This error message is shown if a file could not be read.
     *
     * @param file      The QFile object
     *
     * @param err_msg   The error message (optional)
     */
    void readingFileFailed( const QFile &file, const QString &err_msg );

    /**
     * This error message is shown if the creation of a directory
     * failed.
     *
     * @param path      The path
     */
    void mkdirFailed( const QDir &dir );

    /**
     * This error message is shown if there are mandatorily needed
     * programs missing.
     *
     * @param programs  The list of missing programs
     */
    void missingPrograms( const QStringList &programs );

    /**
     * This error message is shown if a process threw an error.
     *
     * @param proc_err  The code describing the process error
     */
    void processError( QProcess::ProcessError error );

    /**
     * This error message is shown if a system call returned an error.
     *
     * @param sys_call  The system call as string, e.g. "gethostname()"
     *
     * @param errno     The error number
     */
    void systemCallFailed( const QString &sys_call, int err_no );

    /**
     * This error message is shown if a KAuth action could not be
     * executed and KAuth::ActionReply::failed() reported true. Pass
     * the error code supplied by KAuth::ActionReply::errorCode() to
     * this function if available.
     *
     * @param err_code  The error code
     */
    void actionFailed( int err_code = -1 );
    
    /**
    * This error message is shown when an invalid URL was passed to some core
    * class that refuses to process it.
    */
    void invalidURLPassed();

    /**
     * This error message is emitted if the entry of the broadcast areas
     * is empty.
     */
    void emptyBroadcastAreas();

  protected slots:
    /**
     * This slot is invoked when the notification is closed or ignored.
     */
    void slotNotificationClosed();

    /**
     * Opens the contents of a share in a file manager
     */
    void slotOpenShare();

  private:
    Smb4KShare m_share;
};

#endif
