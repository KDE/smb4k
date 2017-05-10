/***************************************************************************
    smb4kmounthelper  -  The helper that mounts and unmounts shares.
                             -------------------
    begin                : Sa Okt 16 2010
    copyright            : (C) 2010-2015 by Alexander Reinholdt
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
#include "core/smb4kglobal.h"

// Qt includes
#include <QProcessEnvironment>
#include <QDebug>
#include <QLatin1String>

// KDE includes
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmountpoint.h>
#include <kurl.h>

using namespace Smb4KGlobal;

KDE4_AUTH_HELPER_MAIN( "net.sourceforge.smb4k.mounthelper", Smb4KMountHelper )


ActionReply Smb4KMountHelper::mount(const QVariantMap &args)
{
  ActionReply reply;
  
  //
  // Get the mount executable
  //
  const QString mount = findMountExecutable();
  
  //
  // Check the executable
  //
  if (mount != args["mh_command"].toString())
  {
    // Something weird is going on, bail out.
    reply.setErrorCode(ActionReply::HelperError);
    reply.setErrorDescription(i18n("Wrong executable passed. Bailing out."));
    return reply;
  }
  else
  {
    // Do nothing
  }
  
  // The mountpoint is a unique and can be used to
  // find the share.
  reply.addData("mh_mountpoint", args["mh_mountpoint"]);
  
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
  else
  {
    // Do nothing
  }
  
  // Set the mount command here.
  QStringList command;
#if defined(Q_OS_LINUX)
  command << mount;
  command << args["mh_unc"].toString();
  command << args["mh_mountpoint"].toString();
  command << args["mh_options"].toStringList();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
  command << mount;
  command << args["mh_options"].toStringList();
  command << args["mh_unc"].toString();
  command << args["mh_mountpoint"].toString();
#else
  // Do nothing.
#endif
  proc.setProgram(command);

  // Run the mount process.
  proc.start();

  if (proc.waitForStarted(-1))
  {
    bool user_kill = false;

    while (!proc.waitForFinished(10))
    {
      // Check if there is a password prompt. If there is one, pass
      // the password to it.
      QByteArray out = proc.readAllStandardError();
      
      if (out.startsWith("Password:"))
      {
        proc.write(args["mh_url"].toUrl().password().toUtf8());
        proc.write("\r");
      }
      else
      {
        // Do nothing
      }

      // We want to be able to terminate the process from outside.
      // Thus, we implement a loop that checks periodically, if we
      // need to kill the process.
      if (HelperSupport::isStopped())
      {
        proc.kill();
        user_kill = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }

    if (proc.exitStatus() == KProcess::CrashExit)
    {
      if (!user_kill)
      {
        reply.setErrorCode(ActionReply::HelperError);
        reply.setErrorDescription(i18n("The mount process crashed."));
        return reply;
      }
      else
      {
        // Do nothing
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
    reply.setErrorCode(ActionReply::HelperError);
    reply.setErrorDescription(i18n("The mount process could not be started."));
  }

  return reply;
}



ActionReply Smb4KMountHelper::unmount(const QVariantMap &args)
{
  ActionReply reply;
  
  //
  // Get the umount executable
  //
  const QString umount = findUmountExecutable();
  
  //
  // Check the executable
  //
  if (umount != args["mh_command"].toString())
  {
    // Something weird is going on, bail out.
    reply.setErrorCode(ActionReply::HelperError);
    reply.setErrorDescription(i18n("Wrong executable passed. Bailing out."));
    return reply;
  }
  else
  {
    // Do nothing
  }
  
  // The mountpoint is a unique and can be used to
  // find the share.
  reply.addData("mh_mountpoint", args["mh_mountpoint"]);

  // Check if the mountpoint is valid and the file system is
  // also correct.
  bool mountpoint_ok = false;
  KMountPoint::List mountpoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);

  for(int j = 0; j < mountpoints.size(); j++)
  {
#ifdef Q_OS_LINUX
    if (QString::compare(args["mh_mountpoint"].toString(),
                         mountpoints.at(j)->mountPoint(), Qt::CaseInsensitive) == 0 &&
        QString::compare(mountpoints.at(j)->mountType(), "cifs", Qt::CaseInsensitive) == 0)
#else
    if (QString::compare(args["mh_mountpoint"].toString(),
                         mountpoints.at(j)->mountPoint(), Qt::CaseInsensitive) == 0 &&
        QString::compare(mountpoints.at(j)->mountType(), "smbfs", Qt::CaseInsensitive) == 0)
#endif
    {
      mountpoint_ok = true;
      break;
    }
    else
    {
      continue;
    }
  }

  if (!mountpoint_ok)
  {
    reply.setErrorCode(ActionReply::HelperError);
    reply.setErrorDescription(i18n("The mountpoint is invalid."));
    return reply;
  }
  else
  {
    // Do nothing
  }

  KProcess proc(this);
  proc.setOutputChannelMode(KProcess::SeparateChannels);
  proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
  
  // Set the umount command here.
  QStringList command;
  command << umount;
  command << args["mh_options"].toStringList();
  command << args["mh_mountpoint"].toString();

  proc.setProgram(command);

  // Run the unmount process.
  proc.start();

  if (proc.waitForStarted(-1))
  {
    // We want to be able to terminate the process from outside.
    // Thus, we implement a loop that checks periodically, if we
    // need to kill the process.
    bool user_kill = false;

    while (!proc.waitForFinished(10))
    {
      if (HelperSupport::isStopped())
      {
        proc.kill();
        user_kill = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }

    if (proc.exitStatus() == KProcess::CrashExit)
    {
      if (!user_kill)
      {
        reply.setErrorCode(ActionReply::HelperError);
        reply.setErrorDescription(i18n("The unmount process crashed."));
        return reply;
      }
      else
      {
        // Do nothing
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
    reply.setErrorCode(ActionReply::HelperError);
    reply.setErrorDescription(i18n("The unmount process could not be started."));
  }

  return reply;
}

#include "smb4kmounthelper.moc"
