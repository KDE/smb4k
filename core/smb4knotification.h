/*
    This class provides notifications for Smb4K
    -------------------
    begin                : Son Jun 27 2010
    SPDX-FileCopyrightText: 2010-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KNOTIFICATION_H
#define SMB4KNOTIFICATION_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QScopedPointer>
#include <QUrl>

// forward declarations
class Smb4KBookmark;
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
Q_DECL_EXPORT void shareMounted(const SharePtr &share);

/**
 * Notify the user that a share has been unmounted.
 *
 * @param share     The share that has been unmounted
 */
Q_DECL_EXPORT void shareUnmounted(const SharePtr &share);

/**
 * Notify the user that multiple shares have been mounted.
 * @param number    The number of mounts
 */
Q_DECL_EXPORT void sharesMounted(int number);

/**
 * Notify the user that multiple shares have been unmounted at once.
 * @param number    The number of unmounts
 */
Q_DECL_EXPORT void sharesUnmounted(int number);

/**
 * Warn the user that the wallet could not be opened.
 *
 * @param name      The name of the wallet
 */
Q_DECL_EXPORT void openingWalletFailed(const QString &name);

/**
 * Warn the user that the credentials stored in the wallet could not
 * be accessed.
 */
Q_DECL_EXPORT void credentialsNotAccessible();

/**
 * Tell the user that the mimetype is not supported and that he/she
 * should convert the file.
 *
 * @param mimetype  The mimetype
 */
Q_DECL_EXPORT void mimetypeNotSupported(const QString &mimetype);

/**
 * Tell the user that this bookmark is already present and that it will
 * thus be skipped.
 *
 * @param bookmark  The bookmark
 */
Q_DECL_EXPORT void bookmarkExists(Smb4KBookmark *bookmark);

/**
 * Tell the user that the label he/she chose for the bookmark is already
 * being used and that it will be changed automatically.
 *
 * @param bookmark  The bookmark
 */
Q_DECL_EXPORT void bookmarkLabelInUse(Smb4KBookmark *bookmark);

/**
 * This warning is shown if the configuration file for the Samba suite
 * (smb.conf) could not be loaded.
 */
Q_DECL_EXPORT void sambaConfigFileMissing();

/**
 * This error message is shown if the mounting of a share failed.
 *
 * @param share     The share that was to be mounted
 *
 * @param errorMessage   The error message
 */
Q_DECL_EXPORT void mountingFailed(const SharePtr &share, const QString &errorMessage);

/**
 * This error message is shown if the unmounting of a share failed.
 *
 * @param share     The share that was to be unmounted
 *
 * @param errorMessage   The error message
 */
Q_DECL_EXPORT void unmountingFailed(const SharePtr &share, const QString &errorMessage);

/**
 * This error message is shown if the unmounting of a certain share
 * is not allowed for the user.
 *
 * @param share     The share that was to be unmounted
 */
Q_DECL_EXPORT void unmountingNotAllowed(const SharePtr &share);

/**
 * This error message is shown if the synchronization failed.
 *
 * @param src       The source URL
 *
 * @param dest      The destination URL
 *
 * @param errorMessage   The error message
 */
Q_DECL_EXPORT void synchronizationFailed(const QUrl &src, const QUrl &dest, const QString &errorMessage);

/**
 * This error message is shown if a command could not be found.
 *
 * @param command   The command that could not be found
 */
Q_DECL_EXPORT void commandNotFound(const QString &command);

/**
 * This error message is shown if the user tried to bookmark a printer.
 *
 * @param share     The Smb4KShare object
 */
Q_DECL_EXPORT void cannotBookmarkPrinter(const SharePtr &share);

/**
 * This error message is shown if a file could not be found.
 *
 * @param fileName  The file name
 */
Q_DECL_EXPORT void fileNotFound(const QString &fileName);

/**
 * This error message is shown if a file could not be opened.
 *
 * @param file      The QFile object
 */
Q_DECL_EXPORT void openingFileFailed(const QFile &file);

/**
 * This error message is shown if a file could not be read.
 *
 * @param file      The QFile object
 *
 * @param errorMessage   The error message (optional)
 */
Q_DECL_EXPORT void readingFileFailed(const QFile &file, const QString &errorMessage);

/**
 * This error message is shown if the creation of a directory
 * failed.
 *
 * @param path      The path
 */
Q_DECL_EXPORT void mkdirFailed(const QDir &dir);

/**
 * This error message is shown if a process threw an error.
 *
 * @param proc_err  The code describing the process error
 */
Q_DECL_EXPORT void processError(QProcess::ProcessError error);

/**
 * This error message is shown if a KAuth action could not be
 * executed and KAuth::ActionReply::failed() reported true. Pass
 * the error code supplied by KAuth::ActionReply::errorCode() to
 * this function if available.
 *
 * @param errorCode  The error code
 */
Q_DECL_EXPORT void actionFailed(int errorCode = -1);

/**
 * This error message is shown when an invalid URL was passed to some core
 * class that refuses to process it.
 */
Q_DECL_EXPORT void invalidURLPassed();

/**
 * This error message is shown when a network related action could not be
 * perfomed or failed.
 *
 * @param errorCode     The error code
 *
 * @param errorMessage  The error message
 */
Q_DECL_EXPORT void networkCommunicationFailed(const QString &errorMessage);
};

#endif
