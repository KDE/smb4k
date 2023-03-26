/*
    The helper that mounts and unmounts shares.

    SPDX-FileCopyrightText: 2010-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kmounthelper.h"
#include "../core/smb4kglobal.h"

// Qt includes
#include <QDebug>
#include <QNetworkInterface>
#include <QProcessEnvironment>
#include <QUrl>

// KDE includes
#include <KAuth/HelperSupport>
#include <KCoreAddons/KProcess>
#include <KI18n/KLocalizedString>
#include <KIOCore/KMountPoint>

using namespace Smb4KGlobal;

KAUTH_HELPER_MAIN("org.kde.smb4k.mounthelper", Smb4KMountHelper);

KAuth::ActionReply Smb4KMountHelper::mount(const QVariantMap &args)
{
    //
    // The action reply
    //
    ActionReply reply;

    //
    // Check if the system is online and return an error if it is not
    //
    bool online = false;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : qAsConst(interfaces)) {
        if (interface.isValid() && interface.type() != QNetworkInterface::Loopback && interface.flags() & QNetworkInterface::IsRunning && !online) {
            online = true;
            break;
        }
    }

    if (!online) {
        reply.setType(ActionReply::HelperErrorType);
        return reply;
    }

    //
    // Get the mount executable
    //
    const QString mount = findMountExecutable();

    //
    // Check the mount executable
    //
    if (mount != args[QStringLiteral("mh_command")].toString()) {
        // Something weird is going on, bail out.
        reply.setType(ActionReply::HelperErrorType);
        return reply;
    }

    //
    // Mount command
    //
    QStringList command;
#if defined(Q_OS_LINUX)
    command << mount;
    command << args[QStringLiteral("mh_url")].toUrl().toString(QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::RemovePort);
    command << args[QStringLiteral("mh_mountpoint")].toString();
    command << args[QStringLiteral("mh_options")].toStringList();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    command << mount;
    command << args[QStringLiteral("mh_options")].toStringList();
    command << args[QStringLiteral("mh_url")].toUrl().toString(QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::RemovePort);
    command << args[QStringLiteral("mh_mountpoint")].toString();
#endif

    //
    // The process
    //
    KProcess proc(this);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
#if defined(Q_OS_LINUX)
    proc.setEnv(QStringLiteral("PASSWD"), args[QStringLiteral("mh_url")].toUrl().password(), true);
#endif
    // We need this to avoid a translated password prompt.
    proc.setEnv(QStringLiteral("LANG"), QStringLiteral("C"));
    // If the location of a Kerberos ticket is passed, it needs to
    // be passed to the process environment here.
    if (args.contains(QStringLiteral("mh_krb5ticket"))) {
        proc.setEnv(QStringLiteral("KRB5CCNAME"), args[QStringLiteral("mh_krb5ticket")].toString());
    }

    proc.setProgram(command);
    proc.start();

    if (proc.waitForStarted(-1)) {
        int timeout = 0;

        while (proc.state() != KProcess::NotRunning) {
            // We want to be able to terminate the process from outside.
            // Thus, we implement a loop that checks periodically, if we
            // need to kill the process.
            if (HelperSupport::isStopped() || timeout == 30000) {
                proc.kill();
                break;
            }

            // Check if there is a password prompt. If there is one, pass
            // the password to it.
            QByteArray out = proc.readAllStandardError();

            if (out.startsWith("Password")) {
                proc.write(args[QStringLiteral("mh_url")].toUrl().password().toUtf8().data());
                proc.write("\r");
            }

            timeout += 10;

            wait(10);
        }

        if (proc.exitStatus() == KProcess::NormalExit) {
            QString stdErr = QString::fromUtf8(proc.readAllStandardError());
            reply.addData(QStringLiteral("mh_error_message"), stdErr.trimmed());
        }
    } else {
        reply.setType(ActionReply::HelperErrorType);
        reply.setErrorDescription(i18n("The mount process could not be started."));
    }

    return reply;
}

KAuth::ActionReply Smb4KMountHelper::unmount(const QVariantMap &args)
{
    ActionReply reply;

    //
    // Get the umount executable
    //
    const QString umount = findUmountExecutable();

    //
    // Check the mount executable
    //
    if (umount != args[QStringLiteral("mh_command")].toString()) {
        // Something weird is going on, bail out.
        reply.setType(ActionReply::HelperErrorType);
        return reply;
    }

    //
    // Check if the mountpoint is valid and the filesystem is correct.
    //
    bool mountPointOk = false;
    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded | KMountPoint::NeedMountOptions);

    for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : qAsConst(mountPoints)) {
        if (args[QStringLiteral("mh_mountpoint")].toString() == mountPoint->mountPoint()
            && (mountPoint->mountType() == QStringLiteral("cifs") || mountPoint->mountType() == QStringLiteral("smb3")
                || mountPoint->mountType() == QStringLiteral("smbfs"))) {
            mountPointOk = true;
            break;
        }
    }

    //
    // Stop here if the mountpoint is not valid
    //
    if (!mountPointOk) {
        reply.setType(ActionReply::HelperErrorType);
        reply.setErrorDescription(i18n("The mountpoint %1 is invalid.", args[QStringLiteral("mh_mountpoint")].toString()));
    }

    //
    // The command
    //
    QStringList command;
    command << umount;
    command << args[QStringLiteral("mh_options")].toStringList();
    command << args[QStringLiteral("mh_mountpoint")].toString();

    //
    // The process
    //
    KProcess proc(this);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    proc.setProgram(command);

    //
    // Depending on the online state, use a different behavior for unmounting.
    //
    // Extensive tests have shown that - when offline - unmounting does not
    // work properly when the process is not detached. Thus, detach it when
    // the system is offline.
    //
    bool online = false;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : qAsConst(interfaces)) {
        if (interface.isValid() && interface.type() != QNetworkInterface::Loopback && interface.flags() & QNetworkInterface::IsRunning && !online) {
            online = true;
            break;
        }
    }

    if (online) {
        proc.start();

        if (proc.waitForStarted(-1)) {
            // We want to be able to terminate the process from outside.
            // Thus, we implement a loop that checks periodically, if we
            // need to kill the process.
            int timeout = 0;

            while (proc.state() != KProcess::NotRunning) {
                if (HelperSupport::isStopped() || timeout == 30000) {
                    proc.kill();
                    break;
                }

                timeout += 10;

                wait(10);
            }

            if (proc.exitStatus() == KProcess::NormalExit) {
                QString stdErr = QString::fromUtf8(proc.readAllStandardError());
                reply.addData(QStringLiteral("mh_error_message"), stdErr.trimmed());
            }
        } else {
            reply.setType(ActionReply::HelperErrorType);
            reply.setErrorDescription(i18n("The unmount process could not be started."));
        }
    } else {
        proc.startDetached();
    }

    return reply;
}
