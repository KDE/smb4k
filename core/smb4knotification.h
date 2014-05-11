/***************************************************************************
    smb4knotification  -  This class provides notifications for Smb4K.
                             -------------------
    begin                : Son Jun 27 2010
    copyright            : (C) 2010-2014 by Alexander Reinholdt
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

#ifndef SMB4KNOTIFICATION_H
#define SMB4KNOTIFICATION_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QScopedPointer>

// KDE includes
#include <kurl.h>

// forward declarations
class Smb4KBookmark;
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;
class Smb4KNotificationPrivate;

/**
 * This namespace provides notifications used thoughout Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

namespace Smb4KNotification
{
  /**
   * Notify the user that a share has been mounted.
   *
   * @param share     The share that has been mounted
   */
  KDE_EXPORT void shareMounted( Smb4KShare *share );

  /**
   * Notify the user that a share has been unmounted.
   *
   * @param share     The share that has been unmounted
   */
  KDE_EXPORT void shareUnmounted( Smb4KShare *share );
  
  /**
   * Notify the user that multiple shares have been mounted.
   *
   * @param total     The number of mounts that were scheduled
   *
   * @param actual    The number of mounts that were actually
   *                  mounted.
   */
  KDE_EXPORT void sharesMounted( int total, int actual );

  /**
   * Notify the user that multiple shares have been unmounted at once.
   *
   * @param total     The number of unmounts that were scheduled
   *
   * @param actual    The number of unmounts that actually finished
   *                  successfully.
   */
  KDE_EXPORT void sharesUnmounted( int total, int actual );

  /**
   * Warn the user that the wallet could not be opened.
   *
   * @param name      The name of the wallet
   */
  KDE_EXPORT void openingWalletFailed( const QString &name );
  
  /**
   * Warn the user that the credentials stored in the wallet could not
   * be accessed.
   */
  KDE_EXPORT void credentialsNotAccessible();
  
  /**
   * Tell the user that the mimetype is not supported and that he/she
   * should convert the file.
   *
   * @param mimetype  The mimetype
   */
  KDE_EXPORT void mimetypeNotSupported( const QString &mimetype );
  
  /**
   * Tell the user that this bookmark is already present and that it will
   * thus be skipped.
   *
   * @param bookmark  The bookmark
   */
  KDE_EXPORT void bookmarkExists( Smb4KBookmark *bookmark );
  
  /**
   * Tell the user that the label he/she chose for the bookmark is already
   * being used and that it will be changed automatically.
   *
   * @param bookmark  The bookmark
   */
  KDE_EXPORT void bookmarkLabelInUse( Smb4KBookmark *bookmark );
  
  /**
   * This warning is shown if the entry of the custom master browser
   * is empty.
   */
  KDE_EXPORT void emptyCustomMasterBrowser();
  
  /**
   * This error message is shown if the list of workgroups could not
   * be retrieved.
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void retrievingDomainsFailed( const QString &err_msg );
  
  /**
   * This error message is shown if the scanning of the broadcast
   * areas failed.
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void scanningBroadcastAreaFailed( const QString &err_msg );
  
  /**
   * This error message is shown if the list of hosts could not
   * be retrieved.
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void retrievingHostsFailed( Smb4KWorkgroup *workgroup, const QString &err_msg );
  
  /**
   * This error message is shown if the list of shares could not
   * be retrieved.
   *
   * @param host      The host object
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void retrievingSharesFailed( Smb4KHost *host, const QString &err_msg );
  
  /**
   * This error message is shown if the preview could not be
   * retrieved.
   *
   * @param err_meg   The error message
   */
  KDE_EXPORT void retrievingPreviewFailed( Smb4KShare *share, const QString &err_msg );
  
  /**
   * This error message is shown if the mounting of a share failed.
   *
   * @param share     The share that was to be mounted
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void mountingFailed( Smb4KShare *share, const QString &err_msg );
  
  /**
   * This error message is shown if the unmounting of a share failed.
   *
   * @param share     The share that was to be unmounted
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void unmountingFailed( Smb4KShare *share, const QString &err_msg );
  
  /**
   * This error message is shown if the unmounting of a certain share
   * is not allowed for the user.
   *
   * @param share     The share that was to be unmounted
   */
  KDE_EXPORT void unmountingNotAllowed( Smb4KShare *share );
  
  /**
   * This error message is shown if printing failed.
   *
   * @param printer   The printer share
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void printingFailed( Smb4KShare *printer, const QString &err_msg );
  
  /**
   * This error message is shown if the synchronization failed.
   *
   * @param src       The source URL
   *
   * @param dest      The destination URL
   *
   * @param err_msg   The error message
   */
  KDE_EXPORT void synchronizationFailed( const KUrl &src,
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
  KDE_EXPORT void searchingFailed( const QString &item, const QString &err_msg );
  
  /**
   * This error message is shown if a command could not be found.
   *
   * @param command   The command that could not be found
   */
  KDE_EXPORT void commandNotFound( const QString &command );
  
  /**
   * This error message is shown if the user tried to bookmark a printer.
   *
   * @param share     The Smb4KShare object
   */
  KDE_EXPORT void cannotBookmarkPrinter( Smb4KShare *share );
  
  /**
   * This error message is shown if a file could not be found.
   *
   * @param fileName  The file name
   */
  KDE_EXPORT void fileNotFound( const QString &fileName );
  
  /**
   * This error message is shown if a file could not be opened.
   *
   * @param file      The QFile object
   */
  KDE_EXPORT void openingFileFailed( const QFile &file );
  
  /**
   * This error message is shown if a file could not be read.
   *
   * @param file      The QFile object
   *
   * @param err_msg   The error message (optional)
   */
  KDE_EXPORT void readingFileFailed( const QFile &file, const QString &err_msg );
  
  /**
   * This error message is shown if the creation of a directory
   * failed.
   *
   * @param path      The path
   */
  KDE_EXPORT void mkdirFailed( const QDir &dir );
  
  /**
   * This error message is shown if a process threw an error.
   *
   * @param proc_err  The code describing the process error
   */
  KDE_EXPORT void processError( QProcess::ProcessError error );
  
  /**
   * This error message is shown if a system call returned an error.
   *
   * @param sys_call  The system call as string, e.g. "gethostname()"
   *
   * @param errno     The error number
   */
  KDE_EXPORT void systemCallFailed( const QString &sys_call, int err_no );
  
  /**
   * This error message is shown if a KAuth action could not be
   * executed and KAuth::ActionReply::failed() reported true. Pass
   * the error code supplied by KAuth::ActionReply::errorCode() to
   * this function if available.
   *
   * @param err_code  The error code
   */
  KDE_EXPORT void actionFailed( int err_code = -1 );
  
  /**
   * This error message is shown when an invalid URL was passed to some core
   * class that refuses to process it.
   */
  KDE_EXPORT void invalidURLPassed();
  
  /**
   * This error message is emitted if the entry of the broadcast areas
   * is empty.
   */
  KDE_EXPORT void emptyBroadcastAreas();
};


#endif
