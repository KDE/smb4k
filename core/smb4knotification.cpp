/*
    This class provides notifications for Smb4K.

    SPDX-FileCopyrightText: 2010-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knotification.h"
#include "smb4kbookmark.h"
#include "smb4khost.h"
#include "smb4knotification_p.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

// KDE includes
#include "kauth_version.h"
#include "kiconthemes_version.h"
#if KAUTH_VERSION >= QT_VERSION_CHECK(5, 92, 0)
#include <KAuth/ActionReply>
#else
#include <KAuth/KAuthActionReply>
#endif
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KNotifications/KNotification>

using namespace KAuth;

//
// Notifications
//

void Smb4KNotification::shareMounted(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        QUrl mountpoint = QUrl::fromLocalFile(share->path());

        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("shareMounted"));
        notification->setText(i18n("<p>The share <b>%1</b> has been mounted to <b>%2</b>.</p>", share->displayString(), share->path()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                                KIconLoader::NoGroup,
                                                                0,
                                                                KIconLoader::DefaultState,
                                                                QStringList(QStringLiteral("emblem-mounted"))));
        notification->setActions(QStringList(i18n("Open")));
        notification->setMountpoint(mountpoint);
        notification->sendEvent();
    }
}

void Smb4KNotification::shareUnmounted(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("shareUnmounted"));
        notification->setText(i18n("<p>The share <b>%1</b> has been unmounted from <b>%2</b>.</p>", share->displayString(), share->path()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                                KIconLoader::NoGroup,
                                                                0,
                                                                KIconLoader::DefaultState,
                                                                QStringList(QStringLiteral("emblem-unmounted"))));
        notification->sendEvent();
    }
}

void Smb4KNotification::sharesMounted(int number)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("sharesMounted"));
    notification->setText(i18np("<p>%1 share has been mounted.</p>", "<p>%1 shares have been mounted.</p>", number));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                            KIconLoader::NoGroup,
                                                            0,
                                                            KIconLoader::DefaultState,
                                                            QStringList(QStringLiteral("emblem-mounted"))));
    notification->sendEvent();
}

void Smb4KNotification::sharesUnmounted(int number)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("sharesUnmounted"));
    notification->setText(i18np("<p>%1 share has been unmounted.</p>", "<p>%1 shares have been unmounted.</p>", number));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                            KIconLoader::NoGroup,
                                                            0,
                                                            KIconLoader::DefaultState,
                                                            QStringList(QStringLiteral("emblem-unmounted"))));
    notification->sendEvent();
}

//
// Warnings
//

void Smb4KNotification::openingWalletFailed(const QString &name)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("openingWalletFailed"));
    notification->setText(i18n("<p>Opening the wallet <b>%1</b> failed.</p>", name));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::credentialsNotAccessible()
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("credentialsNotAccessible"));
    notification->setText(
        i18n("<p>The credentials stored in the wallet could not be accessed. "
             "There is either no wallet available or it could not be opened.</p>"));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::mimetypeNotSupported(const QString &mimetype)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("mimetypeNotSupported"));
    notification->setText(
        i18n("<p>The mimetype <b>%1</b> is not supported for printing. "
             "Please convert the file to PDF or Postscript and try again.</p>",
             mimetype));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::bookmarkExists(Smb4KBookmark *bookmark)
{
    if (bookmark) {
        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("bookmarkExists"));
        notification->setText(i18n("<p>The bookmark for share <b>%1</b> already exists and will be skipped.</p>", bookmark->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::bookmarkLabelInUse(Smb4KBookmark *bookmark)
{
    if (bookmark) {
        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("bookmarkLabelInUse"));
        notification->setText(
            i18n("<p>The label <b>%1</b> of the bookmark for the share <b>%2</b> "
                 "is already being used and will automatically be renamed.</p>",
                 bookmark->label(),
                 bookmark->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

//
// Errors
//
void Smb4KNotification::mountingFailed(const SharePtr &share, const QString &err_msg)
{
    if (share) {
        QString text;

        if (!err_msg.isEmpty()) {
            text = i18n("<p>Mounting the share <b>%1</b> failed:</p><p><tt>%2</tt></p>", share->displayString(), err_msg);
        } else {
            text = i18n("<p>Mounting the share <b>%1</b> failed.</p>", share->displayString());
        }

        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("mountingFailed"));
        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::unmountingFailed(const SharePtr &share, const QString &err_msg)
{
    if (share) {
        QString text;

        if (!err_msg.isEmpty()) {
            text = i18n("<p>Unmounting the share <b>%1</b> from <b>%2</b> failed:</p><p><tt>%3</tt></p>", share->displayString(), share->path(), err_msg);
        } else {
            text = i18n("<p>Unmounting the share <b>%1</b> from <b>%2</b> failed.</p>", share->displayString(), share->path());
        }

        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("unmountingFailed"));
        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::unmountingNotAllowed(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("unmountingNotAllowed"));
        notification->setText(
            i18n("<p>You are not allowed to unmount the share <b>%1</b> from <b>%2</b>. "
                 "It is owned by the user <b>%3</b>.</p>",
                 share->displayString(),
                 share->path(),
                 share->user().loginName()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::synchronizationFailed(const QUrl &src, const QUrl &dest, const QString &err_msg)
{
    QString text;

    if (!err_msg.isEmpty()) {
        text = i18n("<p>Synchronizing <b>%1</b> with <b>%2</b> failed:</p><p><tt>%3</tt></p>", dest.path(), src.path(), err_msg);
    } else {
        text = i18n("<p>Synchronizing <b>%1</b> with <b>%2</b> failed.</p>", dest.path(), src.path());
    }

    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("synchronizationFailed"));
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::commandNotFound(const QString &command)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("commandNotFound"));
    notification->setText(i18n("<p>The command <b>%1</b> could not be found. Please check your installation.</p>", command));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::cannotBookmarkPrinter(const SharePtr &share)
{
    if (share && share->isPrinter()) {
        Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("cannotBookmarkPrinter"));
        notification->setText(i18n("<p>The share <b>%1</b> is a printer and cannot be bookmarked.</p>", share->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::fileNotFound(const QString &fileName)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("fileNotFound"));
    notification->setText(i18n("<p>The file <b>%1</b> could not be found.</p>", fileName));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::openingFileFailed(const QFile &file)
{
    QString text;

    if (!file.errorString().isEmpty()) {
        text = i18n("<p>Opening the file <b>%1</b> failed:</p><p><tt>%2</tt></p>", file.fileName(), file.errorString());
    } else {
        text = i18n("<p>Opening the file <b>%1</b> failed.</p>", file.fileName());
    }

    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("openingFileFailed"));
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::readingFileFailed(const QFile &file, const QString &err_msg)
{
    QString text;

    if (!err_msg.isEmpty()) {
        text = i18n("<p>Reading from file <b>%1</b> failed:</p><p><tt>%2</tt></p>", file.fileName(), err_msg);
    } else {
        if (!file.errorString().isEmpty()) {
            text = i18n("<p>Reading from file <b>%1</b> failed:</p><p><tt>%2</tt></p>", file.fileName(), file.errorString());
        } else {
            text = i18n("<p>Reading from file <b>%1</b> failed.</p>", file.fileName());
        }
    }

    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("readingFileFailed"));
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::mkdirFailed(const QDir &dir)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("mkdirFailed"));
    notification->setText(i18n("<p>The following directory could not be created:</p><p><tt>%1</tt></p>", dir.absolutePath()));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::processError(QProcess::ProcessError error)
{
    QString text;

    switch (error) {
    case QProcess::FailedToStart: {
        text = i18n("<p>The process failed to start (error code: <tt>%1</tt>).</p>", error);
        break;
    }
    case QProcess::Crashed: {
        text = i18n("<p>The process crashed (error code: <tt>%1</tt>).</p>", error);
        break;
    }
    case QProcess::Timedout: {
        text = i18n("<p>The process timed out (error code: <tt>%1</tt>).</p>", error);
        break;
    }
    case QProcess::WriteError: {
        text = i18n("<p>Could not write to the process (error code: <tt>%1</tt>).</p>", error);
        break;
    }
    case QProcess::ReadError: {
        text = i18n("<p>Could not read from the process (error code: <tt>%1</tt>).</p>", error);
        break;
    }
    case QProcess::UnknownError:
    default: {
        text = i18n("<p>The process reported an unknown error.</p>");
        break;
    }
    }

    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("processError"));
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::actionFailed(int errorCode)
{
    QString text, errorMessage;

    switch (errorCode) {
    case ActionReply::NoResponderError: {
        errorMessage = QStringLiteral("NoResponderError");
        break;
    }
    case ActionReply::NoSuchActionError: {
        errorMessage = QStringLiteral("NoSuchActionError");
        break;
    }
    case ActionReply::InvalidActionError: {
        errorMessage = QStringLiteral("InvalidActionError");
        break;
    }
    case ActionReply::AuthorizationDeniedError: {
        errorMessage = QStringLiteral("AuthorizationDeniedError");
        break;
    }
    case ActionReply::UserCancelledError: {
        errorMessage = QStringLiteral("UserCancelledError");
        break;
    }
    case ActionReply::HelperBusyError: {
        errorMessage = QStringLiteral("HelperBusyError");
        break;
    }
    case ActionReply::AlreadyStartedError: {
        errorMessage = QStringLiteral("AlreadyStartedError");
        break;
    }
    case ActionReply::DBusError: {
        errorMessage = QStringLiteral("DBusError");
        break;
    }
    case ActionReply::BackendError: {
        errorMessage = QStringLiteral("BackendError");
        break;
    }
    default: {
        break;
    }
    }

    if (!errorMessage.isEmpty()) {
        text = i18n("<p>Executing an action with root privileges failed (error code: <tt>%1</tt>).</p>", errorMessage);
    } else {
        text = i18n("<p>Executing an action with root privileges failed.</p>");
    }

    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("actionFailed"));
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::invalidURLPassed()
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("invalidURL"));
    notification->setText(i18n("<p>The URL that was passed is invalid.</p>"));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::networkCommunicationFailed(const QString &errorMessage)
{
    Smb4KNotifier *notification = new Smb4KNotifier(QStringLiteral("networkCommunicationFailed"));
    notification->setText(i18n("The network communication failed with the following error message: <s>%1</s>", errorMessage));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}
