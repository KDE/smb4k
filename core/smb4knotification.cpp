/*
    This class provides notifications for Smb4K.

    SPDX-FileCopyrightText: 2010-2026 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knotification.h"

// Qt includes
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
#include <QApplicationStatic>
#else
#include <qapplicationstatic.h>
#endif
#include <QApplication>
#include <QTextStream>

// KDE includes
#include <KAuth/ActionReply>
#include <KIO/OpenUrlJob>
#include <KIconLoader>
#include <KLocalizedString>
#include <KNotification>

using namespace KAuth;

class Smb4KNotificationPrivate : public QObject
{
    Q_OBJECT

public:
    QString componentName;
    QString path;
    void open()
    {
        if (!path.isEmpty()) {
            KIO::OpenUrlJob *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(path), QStringLiteral("inode/directory"));
            job->setFollowRedirections(false);
            job->setAutoDelete(true);
            job->start();
        }
    }
};

Q_APPLICATION_STATIC(Smb4KNotificationPrivate, p);

void Smb4KNotification::setComponentName(const QString &name)
{
    p->componentName = name;
}

//
// Notifications
//

void Smb4KNotification::shareMounted(const SharePtr &share)
{
    Q_ASSERT(share);

    if (!share) {
        return;
    }

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("shareMounted"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The share <b>%1</b> has been mounted to <b>%2</b>.", share->displayString(), share->path()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                                KIconLoader::NoGroup,
                                                                0,
                                                                KIconLoader::DefaultState,
                                                                QStringList(QStringLiteral("emblem-mounted"))));

        p->path = share->path();

        auto *openAction = notification->addAction(i18nc("Open the contents of the share with the file manager", "Open"));
        QObject::connect(openAction, &KNotificationAction::activated, p, &Smb4KNotificationPrivate::open);

        notification->sendEvent();
    } else {
        QTextStream(stdout) << i18n("The share %1 has been mounted to %2.", share->displayString(), share->path()) << Qt::endl;
    }
}

void Smb4KNotification::shareUnmounted(const SharePtr &share)
{
    Q_ASSERT(share);

    if (!share) {
        return;
    }

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("shareUnmounted"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The share <b>%1</b> has been unmounted from <b>%2</b>.", share->displayString(), share->path()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                                KIconLoader::NoGroup,
                                                                0,
                                                                KIconLoader::DefaultState,
                                                                QStringList(QStringLiteral("emblem-unmounted"))));
        notification->sendEvent();
    } else {
        QTextStream(stdout) << i18n("The share %1 has been unmounted from %2.", share->displayString(), share->path()) << Qt::endl;
    }
}

void Smb4KNotification::sharesMounted(int number)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("sharesMounted"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18np("%1 share has been mounted.", "%1 shares have been mounted.", number));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                                KIconLoader::NoGroup,
                                                                0,
                                                                KIconLoader::DefaultState,
                                                                QStringList(QStringLiteral("emblem-mounted"))));
        notification->sendEvent();
    } else {
        QTextStream(stdout) << i18np("%1 share has been mounted.", "%1 shares have been mounted.", number) << Qt::endl;
    }
}

void Smb4KNotification::sharesUnmounted(int number)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("sharesUnmounted"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18np("%1 share has been unmounted.", "%1 shares have been unmounted.", number));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"),
                                                                KIconLoader::NoGroup,
                                                                0,
                                                                KIconLoader::DefaultState,
                                                                QStringList(QStringLiteral("emblem-unmounted"))));
        notification->sendEvent();
    } else {
        QTextStream(stdout) << i18np("%1 share has been unmounted.", "%1 shares have been unmounted.", number) << Qt::endl;
    }
}

void Smb4KNotification::migratingLoginCredentials()
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("migratingCredentials"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n(
            "The way Smb4K stores the login credentials has changed. They will now be migrated. This change is incompatible with earlier versions of Smb4K."));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-information"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stdout) << i18n(
            "The way Smb4K stores the login credentials has changed. They will now be migrated. This change is incompatible with earlier versions of Smb4K.")
                            << Qt::endl;
    }
}

//
// Warnings
//

void Smb4KNotification::mimetypeNotSupported(const QString &mimetype)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("mimetypeNotSupported"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(
            i18n("The mimetype <b>%1</b> is not supported for printing. Please convert the file to PDF or Postscript and try again.", mimetype));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The mimetype %1 is not supported for printing. Please convert the file to PDF or Postscript and try again.", mimetype) << Qt::endl;
    }
}

void Smb4KNotification::bookmarkExists(const BookmarkPtr &bookmark)
{
    Q_ASSERT(bookmark);

    if (!bookmark) {
        return;
    }

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("bookmarkExists"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The bookmark for share <b>%1</b> already exists and will be skipped.", bookmark->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The bookmark for share %1 already exists and will be skipped.", bookmark->displayString()) << Qt::endl;
    }
}

void Smb4KNotification::bookmarkLabelInUse(const BookmarkPtr &bookmark)
{
    Q_ASSERT(bookmark);

    if (!bookmark) {
        return;
    }

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("bookmarkLabelInUse"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The label <b>%1</b> of the bookmark for the share <b>%2</b> is already being used and will automatically be renamed.",
                                   bookmark->label(),
                                   bookmark->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-warning"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The label %1 of the bookmark for the share %2 is already being used and will automatically be renamed.",
                       bookmark->label(),
                       bookmark->displayString())
               << Qt::endl;
    }
}

//
// Errors
//

void Smb4KNotification::mountingFailed(const SharePtr &share, const QString &errorMessage)
{
    Q_ASSERT(share);

    if (!share) {
        return;
    }

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Mounting the share <b>%1</b> failed: <tt>%2</tt>", share->displayString(), errorMessage);
        } else {
            text = i18n("Mounting the share <b>%1</b> failed.", share->displayString());
        }

        KNotification *notification = new KNotification(QStringLiteral("mountingFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Mounting the share %1 failed: %2", share->displayString(), errorMessage);
        } else {
            text = i18n("Mounting the share %1 failed.", share->displayString());
        }

        QTextStream(stderr) << text << Qt::endl;
    }
}

void Smb4KNotification::unmountingFailed(const SharePtr &share, const QString &errorMessage)
{
    Q_ASSERT(share);

    if (share) {
        return;
    }

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Unmounting the share <b>%1</b> from <b>%2</b> failed: <tt>%3</tt>", share->displayString(), share->path(), errorMessage);
        } else {
            text = i18n("Unmounting the share <b>%1</b> from <b>%2</b> failed.", share->displayString(), share->path());
        }

        KNotification *notification = new KNotification(QStringLiteral("unmountingFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Unmounting the share %1 from %2 failed: %3", share->displayString(), share->path(), errorMessage);
        } else {
            text = i18n("Unmounting the share %1 from %2 failed.", share->displayString(), share->path());
        }

        QTextStream(stderr) << text << Qt::endl;
    }
}

void Smb4KNotification::synchronizationFailed(const QUrl &src, const QUrl &dest, const QString &errorMessage)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Synchronizing <b>%1</b> with <b>%2</b> failed: <tt>%3</tt>", dest.path(), src.path(), errorMessage);
        } else {
            text = i18n("Synchronizing <b>%1</b> with <b>%2</b> failed.", dest.path(), src.path());
        }

        KNotification *notification = new KNotification(QStringLiteral("synchronizationFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Synchronizing %1 with %2 failed: %3", dest.path(), src.path(), errorMessage);
        } else {
            text = i18n("Synchronizing %1 with %2 failed.", dest.path(), src.path());
        }

        QTextStream(stderr) << text << Qt::endl;
    }
}

void Smb4KNotification::commandNotFound(const QString &command)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("commandNotFound"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The command <b>%1</b> could not be found. Please check your installation.", command));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The command %1 could not be found. Please check your installation.", command) << Qt::endl;
    }
}

void Smb4KNotification::cannotBookmarkPrinter(const SharePtr &share)
{
    Q_ASSERT(share);

    if (!share || !share->isPrinter()) {
        return;
    }

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("cannotBookmarkPrinter"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The share <b>%1</b> is a printer and cannot be bookmarked.", share->displayString()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The share <b>%1</b> is a printer and cannot be bookmarked.", share->displayString()) << Qt::endl;
    }
}

void Smb4KNotification::fileNotFound(const QString &fileName)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("fileNotFound"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The file <b>%1</b> could not be found.", fileName));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The file %1 could not be found.", fileName) << Qt::endl;
    }
}

void Smb4KNotification::openingFileFailed(const QFile &file)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QString text;

        if (!file.errorString().isEmpty()) {
            text = i18n("Opening the file <b>%1</b> failed: <tt>%2</tt>", file.fileName(), file.errorString());
        } else {
            text = i18n("Opening the file <b>%1</b> failed.", file.fileName());
        }

        KNotification *notification = new KNotification(QStringLiteral("openingFileFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QString text;

        if (!file.errorString().isEmpty()) {
            text = i18n("Opening the file %1 failed: %2", file.fileName(), file.errorString());
        } else {
            text = i18n("Opening the file %1 failed.", file.fileName());
        }

        QTextStream(stderr) << text << Qt::endl;
    }
}

void Smb4KNotification::readingFileFailed(const QFile &file, const QString &errorMessage)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Reading from file <b>%1</b> failed: <tt>%2</tt>", file.fileName(), errorMessage);
        } else {
            if (!file.errorString().isEmpty()) {
                text = i18n("Reading from file <b>%1</b> failed: <tt>%2</tt>", file.fileName(), file.errorString());
            } else {
                text = i18n("Reading from file <b>%1</b> failed.", file.fileName());
            }
        }

        KNotification *notification = new KNotification(QStringLiteral("readingFileFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QString text;

        if (!errorMessage.isEmpty()) {
            text = i18n("Reading from file %1 failed: %2", file.fileName(), errorMessage);
        } else {
            if (!file.errorString().isEmpty()) {
                text = i18n("Reading from file %1 failed: %2", file.fileName(), file.errorString());
            } else {
                text = i18n("Reading from file %1 failed.", file.fileName());
            }
        }

        QTextStream(stderr) << text << Qt::endl;
    }
}

void Smb4KNotification::mkdirFailed(const QDir &dir)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("mkdirFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The following directory could not be created: <tt>%1</tt>", dir.absolutePath()));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The following directory could not be created: %1", dir.absolutePath()) << Qt::endl;
    }
}

void Smb4KNotification::processError(QProcess::ProcessError error)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QString text;

        switch (error) {
        case QProcess::FailedToStart: {
            text = i18n("The process failed to start (error code: <tt>%1</tt>).", error);
            break;
        }
        case QProcess::Crashed: {
            text = i18n("The process crashed (error code: <tt>%1</tt>).", error);
            break;
        }
        case QProcess::Timedout: {
            text = i18n("The process timed out (error code: <tt>%1</tt>).", error);
            break;
        }
        case QProcess::WriteError: {
            text = i18n("Could not write to the process (error code: <tt>%1</tt>).", error);
            break;
        }
        case QProcess::ReadError: {
            text = i18n("Could not read from the process (error code: <tt>%1</tt>).", error);
            break;
        }
        case QProcess::UnknownError:
        default: {
            text = i18n("The process reported an unknown error.");
            break;
        }
        }

        KNotification *notification = new KNotification(QStringLiteral("processError"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QString text;

        switch (error) {
        case QProcess::FailedToStart: {
            text = i18n("The process failed to start (error code: %1).", error);
            break;
        }
        case QProcess::Crashed: {
            text = i18n("The process crashed (error code: %1).", error);
            break;
        }
        case QProcess::Timedout: {
            text = i18n("The process timed out (error code: %1).", error);
            break;
        }
        case QProcess::WriteError: {
            text = i18n("Could not write to the process (error code: %1).", error);
            break;
        }
        case QProcess::ReadError: {
            text = i18n("Could not read from the process (error code: %1).", error);
            break;
        }
        case QProcess::UnknownError:
        default: {
            text = i18n("The process reported an unknown error.");
            break;
        }
        }

        QTextStream(stderr) << text << Qt::endl;
    }
}

void Smb4KNotification::actionFailed(int errorCode, const QString &errorMessage)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QString text, errorCodeString;

        switch (errorCode) {
        case ActionReply::NoResponderError: {
            errorCodeString = QStringLiteral("NoResponderError");
            break;
        }
        case ActionReply::NoSuchActionError: {
            errorCodeString = QStringLiteral("NoSuchActionError");
            break;
        }
        case ActionReply::InvalidActionError: {
            errorCodeString = QStringLiteral("InvalidActionError");
            break;
        }
        case ActionReply::AuthorizationDeniedError: {
            errorCodeString = QStringLiteral("AuthorizationDeniedError");
            break;
        }
        case ActionReply::UserCancelledError: {
            errorCodeString = QStringLiteral("UserCancelledError");
            break;
        }
        case ActionReply::HelperBusyError: {
            errorCodeString = QStringLiteral("HelperBusyError");
            break;
        }
        case ActionReply::AlreadyStartedError: {
            errorCodeString = QStringLiteral("AlreadyStartedError");
            break;
        }
        case ActionReply::DBusError: {
            errorCodeString = QStringLiteral("DBusError");
            break;
        }
        case ActionReply::BackendError: {
            errorCodeString = QStringLiteral("BackendError");
            break;
        }
        default: {
            break;
        }
        }

        text = i18n("Executing an action with root privileges failed%1%2",
                    !errorCodeString.isEmpty() ? QStringLiteral(" (") + errorCodeString + QStringLiteral(")") : QString(),
                    !errorMessage.isEmpty() ? QStringLiteral(": ") + errorMessage : QStringLiteral("."));

        KNotification *notification = new KNotification(QStringLiteral("actionFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(text);
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QString text, errorCodeString;

        switch (errorCode) {
        case ActionReply::NoResponderError: {
            errorCodeString = QStringLiteral("NoResponderError");
            break;
        }
        case ActionReply::NoSuchActionError: {
            errorCodeString = QStringLiteral("NoSuchActionError");
            break;
        }
        case ActionReply::InvalidActionError: {
            errorCodeString = QStringLiteral("InvalidActionError");
            break;
        }
        case ActionReply::AuthorizationDeniedError: {
            errorCodeString = QStringLiteral("AuthorizationDeniedError");
            break;
        }
        case ActionReply::UserCancelledError: {
            errorCodeString = QStringLiteral("UserCancelledError");
            break;
        }
        case ActionReply::HelperBusyError: {
            errorCodeString = QStringLiteral("HelperBusyError");
            break;
        }
        case ActionReply::AlreadyStartedError: {
            errorCodeString = QStringLiteral("AlreadyStartedError");
            break;
        }
        case ActionReply::DBusError: {
            errorCodeString = QStringLiteral("DBusError");
            break;
        }
        case ActionReply::BackendError: {
            errorCodeString = QStringLiteral("BackendError");
            break;
        }
        default: {
            break;
        }
        }

        text = i18n("Executing an action with root privileges failed%1%2",
                    !errorCodeString.isEmpty() ? QStringLiteral(" (") + errorCodeString + QStringLiteral(")") : QString(),
                    !errorMessage.isEmpty() ? QStringLiteral(": ") + errorMessage : QStringLiteral("."));

        QTextStream(stderr) << text << Qt::endl;
    }
}

void Smb4KNotification::invalidURLPassed()
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("invalidURL"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The URL that was passed is invalid."));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The URL that was passed is invalid.") << Qt::endl;
    }
}

void Smb4KNotification::networkCommunicationFailed(const QString &errorMessage)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("networkCommunicationFailed"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("The network communication failed with the following error message: <tt>%1</tt>", errorMessage));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("The network communication failed with the following error message: %1", errorMessage);
    }
}

void Smb4KNotification::zeroconfError(const QString &errorMessage)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("zeroconfError"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("An error with the Zeroconf service occurred: <tt>%1</tt>", errorMessage));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("An error with the Zeroconf service occurred: %1", errorMessage) << Qt::endl;
    }
}

void Smb4KNotification::keychainError(const QString &errorMessage)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        KNotification *notification = new KNotification(QStringLiteral("keychainError"), KNotification::CloseOnTimeout);

        if (!p->componentName.isEmpty()) {
            notification->setComponentName(p->componentName);
        }

        notification->setText(i18n("An error occurred while reading the login credentials from the secure storage: <tt>%1</tt>", errorMessage));
        notification->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-error"), KIconLoader::NoGroup, 0, KIconLoader::DefaultState));
        notification->sendEvent();
    } else {
        QTextStream(stderr) << i18n("An error occurred while reading the login credentials from the secure storage: %1", errorMessage) << Qt::endl;
    }
}

#include "smb4knotification.moc"
