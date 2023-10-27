/*
    This class provides notifications for Smb4K.

    SPDX-FileCopyrightText: 2010-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knotification.h"
#include "smb4kbookmark.h"
#include "smb4khost.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

// Qt includes
#include <QEventLoop>

// KDE includes
#include "kauth_version.h"
#include <KAuth/ActionReply>
#include <KIO/OpenUrlJob>
#include <KIconLoader>
#include <KLocalizedString>
#include <KNotification>

using namespace KAuth;

//
// Notifications
//

void Smb4KNotification::shareMounted(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        QEventLoop loop;

        KNotification *notification = new KNotification(QStringLiteral("shareMounted"), KNotification::CloseOnTimeout);
        notification->setText(i18n("<p>The share <b>%1</b> has been mounted to <b>%2</b>.</p>", share->displayString(), share->path()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                                KIconLoader::NoGroup,
                                                                0,
                                                                KIconLoader::DefaultState,
                                                                QStringList(QStringLiteral("emblem-mounted"))));

        auto open = [&]() {
            KIO::OpenUrlJob *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(share->path()), QStringLiteral("inode/directory"));
            job->setFollowRedirections(false);
            job->setAutoDelete(true);
            job->start();
        };

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto *openAction = notification->addAction(i18nc("Open the contents of the share with the file manager", "Open"));
        QObject::connect(openAction, &KNotificationAction::activated, open);
#else
        notification->setActions(QStringList(i18nc("Open the contents of the share with the file manager", "Open")));
        QObject::connect(notification, &KNotification::action1Activated, open);
#endif
        QObject::connect(notification, &KNotification::closed, &loop, &QEventLoop::quit);

        notification->sendEvent();

        loop.exec();
    }
}

void Smb4KNotification::shareUnmounted(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        KNotification *notification = new KNotification(QStringLiteral("shareUnmounted"), KNotification::CloseOnTimeout);
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
    KNotification *notification = new KNotification(QStringLiteral("sharesMounted"), KNotification::CloseOnTimeout);
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
    KNotification *notification = new KNotification(QStringLiteral("sharesUnmounted"), KNotification::CloseOnTimeout);
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
    KNotification *notification = new KNotification(QStringLiteral("openingWalletFailed"), KNotification::CloseOnTimeout);
    notification->setText(i18n("<p>Opening the wallet <b>%1</b> failed.</p>", name));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::credentialsNotAccessible()
{
    KNotification *notification = new KNotification(QStringLiteral("credentialsNotAccessible"), KNotification::CloseOnTimeout);
    notification->setText(
        i18n("<p>The credentials stored in the wallet could not be accessed. "
             "There is either no wallet available or it could not be opened.</p>"));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::mimetypeNotSupported(const QString &mimetype)
{
    KNotification *notification = new KNotification(QStringLiteral("mimetypeNotSupported"), KNotification::CloseOnTimeout);
    notification->setText(
        i18n("<p>The mimetype <b>%1</b> is not supported for printing. "
             "Please convert the file to PDF or Postscript and try again.</p>",
             mimetype));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::bookmarkExists(const BookmarkPtr &bookmark)
{
    Q_ASSERT(bookmark);

    if (bookmark) {
        KNotification *notification = new KNotification(QStringLiteral("bookmarkExists"), KNotification::CloseOnTimeout);
        notification->setText(i18n("<p>The bookmark for share <b>%1</b> already exists and will be skipped.</p>", bookmark->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::bookmarkLabelInUse(const BookmarkPtr &bookmark)
{
    Q_ASSERT(bookmark);

    if (bookmark) {
        KNotification *notification = new KNotification(QStringLiteral("bookmarkLabelInUse"), KNotification::CloseOnTimeout);
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
    Q_ASSERT(share);

    if (share) {
        QString text;

        if (!err_msg.isEmpty()) {
            text = i18n("<p>Mounting the share <b>%1</b> failed:</p><p><tt>%2</tt></p>", share->displayString(), err_msg);
        } else {
            text = i18n("<p>Mounting the share <b>%1</b> failed.</p>", share->displayString());
        }

        KNotification *notification = new KNotification(QStringLiteral("mountingFailed"), KNotification::CloseOnTimeout);
        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::unmountingFailed(const SharePtr &share, const QString &err_msg)
{
    Q_ASSERT(share);

    if (share) {
        QString text;

        if (!err_msg.isEmpty()) {
            text = i18n("<p>Unmounting the share <b>%1</b> from <b>%2</b> failed:</p><p><tt>%3</tt></p>", share->displayString(), share->path(), err_msg);
        } else {
            text = i18n("<p>Unmounting the share <b>%1</b> from <b>%2</b> failed.</p>", share->displayString(), share->path());
        }

        KNotification *notification = new KNotification(QStringLiteral("unmountingFailed"), KNotification::CloseOnTimeout);
        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::unmountingNotAllowed(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        KNotification *notification = new KNotification(QStringLiteral("unmountingNotAllowed"), KNotification::CloseOnTimeout);
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

    KNotification *notification = new KNotification(QStringLiteral("synchronizationFailed"), KNotification::CloseOnTimeout);
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::commandNotFound(const QString &command)
{
    KNotification *notification = new KNotification(QStringLiteral("commandNotFound"), KNotification::CloseOnTimeout);
    notification->setText(i18n("<p>The command <b>%1</b> could not be found. Please check your installation.</p>", command));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::cannotBookmarkPrinter(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share && share->isPrinter()) {
        KNotification *notification = new KNotification(QStringLiteral("cannotBookmarkPrinter"), KNotification::CloseOnTimeout);
        notification->setText(i18n("<p>The share <b>%1</b> is a printer and cannot be bookmarked.</p>", share->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    }
}

void Smb4KNotification::fileNotFound(const QString &fileName)
{
    KNotification *notification = new KNotification(QStringLiteral("fileNotFound"), KNotification::CloseOnTimeout);
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

    KNotification *notification = new KNotification(QStringLiteral("openingFileFailed"), KNotification::CloseOnTimeout);
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

    KNotification *notification = new KNotification(QStringLiteral("readingFileFailed"), KNotification::CloseOnTimeout);
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::mkdirFailed(const QDir &dir)
{
    KNotification *notification = new KNotification(QStringLiteral("mkdirFailed"), KNotification::CloseOnTimeout);
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

    KNotification *notification = new KNotification(QStringLiteral("processError"), KNotification::CloseOnTimeout);
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

    KNotification *notification = new KNotification(QStringLiteral("actionFailed"), KNotification::CloseOnTimeout);
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::invalidURLPassed()
{
    KNotification *notification = new KNotification(QStringLiteral("invalidURL"), KNotification::CloseOnTimeout);
    notification->setText(i18n("<p>The URL that was passed is invalid.</p>"));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::networkCommunicationFailed(const QString &errorMessage)
{
    KNotification *notification = new KNotification(QStringLiteral("networkCommunicationFailed"), KNotification::CloseOnTimeout);
    notification->setText(i18n("<p>The network communication failed with the following error message: <s>%1</s></p>", errorMessage));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}

void Smb4KNotification::zeroconfError(const QString &errorMessage)
{
    KNotification *notification = new KNotification(QStringLiteral("zeroconfError"), KNotification::CloseOnTimeout);
    notification->setText(i18n("<p>An error with the Zeroconf service occurred: <s>%1</s></p>", errorMessage));
    notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
    notification->sendEvent();
}
