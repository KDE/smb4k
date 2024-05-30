/*
    This class provides notifications for Smb4K

    SPDX-FileCopyrightText: 2010-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KNOTIFICATION_H
#define SMB4KNOTIFICATION_H

// application specific includes
#include "smb4kcore_export.h"
#include "smb4kglobal.h"

// Qt includes
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QUrl>

/**
 * This namespace provides notifications used thoughout Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

namespace Smb4KNotification
{
/**
 * Set the component name for the notifications. This is only
 * necessary when running from the plasmoid.
 *
 * @param name      The component name
 */
SMB4KCORE_EXPORT void setComponentName(const QString &name);

/**
 * Notify the user that a share has been mounted.
 *
 * @param share     The share that has been mounted
 */
SMB4KCORE_EXPORT void shareMounted(const SharePtr &share);

/**
 * Notify the user that a share has been unmounted.
 *
 * @param share     The share that has been unmounted
 */
SMB4KCORE_EXPORT void shareUnmounted(const SharePtr &share);

/**
 * Notify the user that multiple shares have been mounted.
 * @param number    The number of mounts
 */
SMB4KCORE_EXPORT void sharesMounted(int number);

/**
 * Notify the user that multiple shares have been unmounted at once.
 * @param number    The number of unmounts
 */
SMB4KCORE_EXPORT void sharesUnmounted(int number);

/**
 * Tell the user that the mimetype is not supported and that he/she
 * should convert the file.
 *
 * @param mimetype  The mimetype
 */
SMB4KCORE_EXPORT void mimetypeNotSupported(const QString &mimetype);

/**
 * Tell the user that this bookmark is already present and that it will
 * thus be skipped.
 *
 * @param bookmark  The bookmark
 */
SMB4KCORE_EXPORT void bookmarkExists(const BookmarkPtr &bookmark);

/**
 * Tell the user that the label he/she chose for the bookmark is already
 * being used and that it will be changed automatically.
 *
 * @param bookmark  The bookmark
 */
SMB4KCORE_EXPORT void bookmarkLabelInUse(const BookmarkPtr &bookmark);

/**
 * This error message is shown if the mounting of a share failed.
 *
 * @param share     The share that was to be mounted
 *
 * @param errorMessage   The error message
 */
SMB4KCORE_EXPORT void mountingFailed(const SharePtr &share, const QString &errorMessage);

/**
 * This error message is shown if the unmounting of a share failed.
 *
 * @param share     The share that was to be unmounted
 *
 * @param errorMessage   The error message
 */
SMB4KCORE_EXPORT void unmountingFailed(const SharePtr &share, const QString &errorMessage);

/**
 * This error message is shown if the unmounting of a certain share
 * is not allowed for the user.
 *
 * @param share     The share that was to be unmounted
 */
SMB4KCORE_EXPORT void unmountingNotAllowed(const SharePtr &share);

/**
 * This error message is shown if the synchronization failed.
 *
 * @param src       The source URL
 *
 * @param dest      The destination URL
 *
 * @param errorMessage   The error message
 */
SMB4KCORE_EXPORT void synchronizationFailed(const QUrl &src, const QUrl &dest, const QString &errorMessage);

/**
 * This error message is shown if a command could not be found.
 *
 * @param command   The command that could not be found
 */
SMB4KCORE_EXPORT void commandNotFound(const QString &command);

/**
 * This error message is shown if the user tried to bookmark a printer.
 *
 * @param share     The Smb4KShare object
 */
SMB4KCORE_EXPORT void cannotBookmarkPrinter(const SharePtr &share);

/**
 * This error message is shown if a file could not be found.
 *
 * @param fileName  The file name
 */
SMB4KCORE_EXPORT void fileNotFound(const QString &fileName);

/**
 * This error message is shown if a file could not be opened.
 *
 * @param file      The QFile object
 */
SMB4KCORE_EXPORT void openingFileFailed(const QFile &file);

/**
 * This error message is shown if a file could not be read.
 *
 * @param file      The QFile object
 *
 * @param errorMessage   The error message (optional)
 */
SMB4KCORE_EXPORT void readingFileFailed(const QFile &file, const QString &errorMessage);

/**
 * This error message is shown if the creation of a directory
 * failed.
 *
 * @param path      The path
 */
SMB4KCORE_EXPORT void mkdirFailed(const QDir &dir);

/**
 * This error message is shown if a process threw an error.
 *
 * @param error     The code describing the process error
 */
SMB4KCORE_EXPORT void processError(QProcess::ProcessError error);

/**
 * This error message is shown if a KAuth action could not be
 * executed and KAuth::ActionReply::failed() reported true. Pass
 * the error code supplied by KAuth::ActionReply::errorCode() to
 * this function if available.
 *
 * @param errorCode  The error code
 */
SMB4KCORE_EXPORT void actionFailed(int errorCode = -1);

/**
 * This error message is shown when an invalid URL was passed to some core
 * class that refuses to process it.
 */
SMB4KCORE_EXPORT void invalidURLPassed();

/**
 * This error message is shown when a network related action could not be
 * perfomed or failed.
 *
 * @param errorCode     The error code
 *
 * @param errorMessage  The error message
 */
SMB4KCORE_EXPORT void networkCommunicationFailed(const QString &errorMessage);

/**
 * This error message is shown when an error with the Zeroconf daemon
 * (Avahi or mDNS) occurred.
 *
 * @param errorMessage  The error message
 */
SMB4KCORE_EXPORT void zeroconfError(const QString &errorMessage);

/**
 * This error message is shown when an error occurred while reading the
 * login credentials from the secure storage.
 *
 * @param errorMessage  The error message
 */
SMB4KCORE_EXPORT void keychainError(const QString &errorMessage);
};

#endif
