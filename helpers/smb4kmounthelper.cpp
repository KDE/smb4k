/***************************************************************************
    The helper that mounts and unmounts shares.
                             -------------------
    begin                : Sa Okt 16 2010
    copyright            : (C) 2010-2017 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kmounthelper.h"
#include "../core/smb4kglobal.h"

// Qt includes
#include <QProcessEnvironment>
#include <QDebug>
#include <QLatin1String>
#include <QUrl>

// KDE includes
#include <KAuth/KAuthHelperSupport>
#include <KCoreAddons/KProcess>
#include <KI18n/KLocalizedString>
#include <KIOCore/KMountPoint>

using namespace Smb4KGlobal;

KAUTH_HELPER_MAIN("org.kde.smb4k.mounthelper", Smb4KMountHelper);

KAuth::ActionReply Smb4KMountHelper::mount(const QVariantMap& args)
{
  //
  // The action reply
  //
  ActionReply reply;
  
  //
  // Get the mount executable
  //
  const QString mount = findMountExecutable();
  
  //
  // Check the mount executable
  //
  if (mount != args["mh_command"].toString())
  {
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
  command << args["mh_url"].toUrl().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort);
  command << args["mh_mountpoint"].toString();
  command << args["mh_options"].toStringList();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
  command << mount;
  command << args["mh_options"].toStringList();
  command << args["mh_url"].toUrl().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort);
  command << args["mh_mountpoint"].toString();
#endif
  
  //
  // The process
  //
  KProcess proc(this);
  proc.setOutputChannelMode(KProcess::SeparateChannels);
  proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
#if defined(Q_OS_LINUX)
  proc.setEnv("PASSWD", args["mh_url"].toUrl().password(), true);
#endif
  // We need this to avoid a translated password prompt.
  proc.setEnv("LANG", "C");
  // If the location of a Kerberos ticket is passed, it needs to
  // be passed to the process environment here.
  if (args.contains("mh_krb5ticket"))
  {
    proc.setEnv("KRB5CCNAME", args["mh_krb5ticket"].toString());
  }

  proc.setProgram(command);
  proc.start();
    
  if (proc.waitForStarted(-1))
  {
    bool userKill = false;

    while (!proc.waitForFinished(10))
    {
      // Check if there is a password prompt. If there is one, pass
      // the password to it.
      QByteArray out = proc.readAllStandardError();

      if (out.startsWith("Password"))
      {
        proc.write(args["mh_url"].toUrl().password().toUtf8());
        proc.write("\r");
      }

      // We want to be able to terminate the process from outside.
      // Thus, we implement a loop that checks periodically, if we
      // need to kill the process.
      if (HelperSupport::isStopped())
      {
        proc.kill();
        userKill = true;
      }
    }

    if (proc.exitStatus() == KProcess::CrashExit)
    {
      if (!userKill)
      {
        reply.setType(ActionReply::HelperErrorType);
        reply.setErrorDescription(i18n("The mount process crashed."));
      }
    }
    else
    {
      // Check if there is output on stderr.
      QString stdErr = QString::fromUtf8(proc.readAllStandardError());
      reply.addData("mh_error_message", stdErr.trimmed());
    }
  }
  else
  {
    reply.setType(ActionReply::HelperErrorType);
    reply.setErrorDescription(i18n("The mount process could not be started."));
  }  
  
  return reply;
}


KAuth::ActionReply Smb4KMountHelper::unmount(const QVariantMap& args)
{
  ActionReply reply;
  
  //
  // Get the umount executable
  //
  const QString umount = findUmountExecutable();
  
  //
  // Check the mount executable
  //
  if (umount != args["mh_command"].toString())
  {
    // Something weird is going on, bail out.
    reply.setType(ActionReply::HelperErrorType);
    return reply;
  }
  
  //
  // Check if the mountpoint is valid and the filesystem is correct.
  //
  bool mountPointOk = false;
  KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
      
  for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : mountPoints)
  {
#if defined(Q_OS_LINUX)
    if (QString::compare(args["mh_mountpoint"].toString(), mountPoint->mountPoint()) == 0 &&
        QString::compare(mountPoint->mountType(), "cifs", Qt::CaseInsensitive) == 0)
#else
    if (QString::compare(args["mh_mountpoint"].toString(), mountPoint->mountPoint()) == 0 &&
        QString::compare(mountPoint->mountType(), "smbfs", Qt::CaseInsensitive) == 0)
#endif
    {
      mountPointOk = true;
      break;
    }
  }
  
  //
  // Stop here if the mountpoint is not valid
  //
  if (!mountPointOk)
  {
    reply.setType(ActionReply::HelperErrorType);
    reply.setErrorDescription(i18n("The mountpoint %1 is invalid.", args["mh_mountpoint"].toString()));
  }
  
  //
  // The command
  //
  QStringList command;
  command << umount;
  command << args["mh_options"].toStringList();
  command << args["mh_mountpoint"].toString();
  
  //
  // The process
  //
  KProcess proc(this);
  proc.setOutputChannelMode(KProcess::SeparateChannels);
  proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
  proc.setProgram(command);
  proc.start();
  
  if (proc.waitForStarted(-1))
  {
    // We want to be able to terminate the process from outside.
    // Thus, we implement a loop that checks periodically, if we
    // need to kill the process.
    bool userKill = false;

    while (!proc.waitForFinished(10))
    {
      if (HelperSupport::isStopped())
      {
        proc.kill();
        userKill = true;
        break;
      }
    }

    if (proc.exitStatus() == KProcess::CrashExit)
    {
      if (!userKill)
      {
        reply.setType(ActionReply::HelperErrorType);
        reply.setErrorDescription(i18n("The unmount process crashed."));
      }
    }
    else
    {
      // Check if there is output on stderr.
      QString stdErr = QString::fromUtf8(proc.readAllStandardError());
      reply.addData("mh_error_message", stdErr.trimmed());
    }
  }
  else
  {
    reply.setType(ActionReply::HelperErrorType);
    reply.setErrorDescription(i18n("The unmount process could not be started."));
  }
  
  return reply;
}


