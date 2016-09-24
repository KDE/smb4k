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

// Qt includes
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDebug>
#include <QtCore/QLatin1String>
#include <QtCore/QUrl>

// KDE includes
#include <KAuth/KAuthHelperSupport>
#include <KCoreAddons/KProcess>
#include <KI18n/KLocalizedString>
#include <KIOCore/KMountPoint>

KAUTH_HELPER_MAIN("org.kde.smb4k.mounthelper", Smb4KMountHelper);


ActionReply Smb4KMountHelper::mount(const QVariantMap &args)
{
  ActionReply reply;
  
  //
  // Iterate through the entries.
  //
  QMapIterator<QString, QVariant> it(args);
  
  while (it.hasNext())
  {
    it.next();
    QString index = it.key();
    QVariantMap entry = it.value().toMap();
    
    //
    // The process
    //
    KProcess proc(this);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
#if defined(Q_OS_LINUX)
    proc.setEnv("PASSWD", entry["mh_url"].toUrl().password(), true);
#endif
    // We need this to avoid a translated password prompt.
    proc.setEnv("LANG", "C");
    // If the location of a Kerberos ticket is passed, it needs to
    // be passed to the process environment here.
    if (args.contains("mh_krb5ticket"))
    {
      proc.setEnv("KRB5CCNAME", entry["mh_krb5ticket"].toString());
    }
    else
    {
      // Do nothing
    }
    
    //
    // Mount command
    //
    QStringList command;
#if defined(Q_OS_LINUX)
    command << entry["mh_command"].toString();
    command << entry["mh_unc"].toString();
    command << entry["mh_mountpoint"].toString();
    command << entry["mh_options"].toStringList();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    command << entry["mh_command"].toString();
    command << entry["mh_options"].toStringList();
    command << entry["mh_unc"].toString();
    command << entry["mh_mountpoint"].toString();
#else
    // Do nothing.
#endif
    proc.setProgram(command);

    //
    // Run the mount process
    //
    proc.start();
    
    if (proc.waitForStarted(-1))
    {
      bool userKill = false;

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
          userKill = true;
          break;
        }
        else
        {
          // Do nothing
        }
      }

      if (proc.exitStatus() == KProcess::CrashExit)
      {
        if (!userKill)
        {
          reply.setType(ActionReply::HelperErrorType);
          reply.setErrorDescription(i18n("The mount process crashed."));
          break;
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
        reply.addData(QString("mh_error_message_%1").arg(index), stdErr.trimmed());
      }
    }
    else
    {
      reply.setType(ActionReply::HelperErrorType);
      reply.setErrorDescription(i18n("The mount process could not be started."));
      break;
    }
  }
  
  return reply;
}


ActionReply Smb4KMountHelper::unmount(const QVariantMap& args)
{
  //
  // The action reply
  //
  ActionReply reply;
  
  //
  // Check if the helper function should unmount all shares 
  // one by one or at once.
  //
  if (!args.value("mh_unmount_at_once").toBool())
  {
    reply = unmountOneByOne(args);
  }
  else
  {
    reply = unmountAtOnce(args);
  }
  
  //
  // Return the action reply
  //
  return reply;
}


ActionReply Smb4KMountHelper::unmountOneByOne(const QVariantMap& args)
{
  //
  // Action reply
  //
  ActionReply reply;
  
  //
  // Iterate through the entries.
  //
  QMapIterator<QString, QVariant> it(args);
    
  while (it.hasNext())
  {
    it.next();
    QString index = it.key();
    QVariantMap entry = it.value().toMap();
      
    //
    // Check if the mountpoint is valid and the filesystem is correct.
    //
    bool mountPointOk = false;
    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
      
    Q_FOREACH(const QExplicitlySharedDataPointer<KMountPoint> mountPoint, mountPoints)
    {
#if defined(Q_OS_LINUX)
      if (QString::compare(entry.value("mh_mountpoint").toString(), mountPoint->mountPoint()) == 0 &&
          QString::compare(mountPoint->mountType(), "cifs", Qt::CaseInsensitive) == 0)
#else
      if (QString::compare(entry.value("mh_mountpoint").toString(), mountPoint->mountPoint()) == 0 &&
          QString::compare(mountPoint->mountType(), "smbfs", Qt::CaseInsensitive) == 0)
#endif
      {
        mountPointOk = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }
      
    //
    // Stop here if the mountpoint is not valid
    //
    if (!mountPointOk)
    {
      reply.setType(ActionReply::HelperErrorType);
      reply.setErrorDescription(i18n("The mountpoint %1 is invalid.", entry.value("mh_mountpoint").toString()));
      break;
    }
    else
    {
      // Do nothing
    }
      
    //
    // The command
    //
    QStringList command;
    command << entry["mh_command"].toString();
    command << entry["mh_options"].toStringList();
    command << entry["mh_mountpoint"].toString();
    
    //
    // The process
    //
    KProcess proc(this);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    proc.setProgram(command);
      
    //
    // Run the unmount process
    //
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
        else
        {
          // Do nothing
        }
      }

      if (proc.exitStatus() == KProcess::CrashExit)
      {
        if (!userKill)
        {
          reply.setType(ActionReply::HelperErrorType);
          reply.setErrorDescription(i18n("The unmount process crashed."));
          break;
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
        reply.addData(QString("mh_error_message_%1").arg(index), stdErr.trimmed());
      }
    }
    else
    {
      reply.setType(ActionReply::HelperErrorType);
      reply.setErrorDescription(i18n("The unmount process could not be started."));
      break;
    }
  }
  
  //
  // Return the reply
  //
  return reply;
}


ActionReply Smb4KMountHelper::unmountAtOnce(const QVariantMap& args)
{
  //
  // Action reply
  //
  ActionReply reply;
  
  //
  // Check the mountpoints and put the valid ones into a string list
  //
  QStringList validMountPoints;
  QMapIterator<QString, QVariant> it(args);
    
  while (it.hasNext())
  {
    it.next();
    QVariantMap entry = it.value().toMap();
      
    bool mountPointOk = false;
    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
      
    Q_FOREACH(const QExplicitlySharedDataPointer<KMountPoint> mountPoint, mountPoints)
    {
#if defined(Q_OS_LINUX)
      if (QString::compare(entry.value("mh_mountpoint").toString(), mountPoint->mountPoint()) == 0 &&
          QString::compare(mountPoint->mountType(), "cifs", Qt::CaseInsensitive) == 0)
#else
      if (QString::compare(entry.value("mh_mountpoint").toString(), mountPoint->mountPoint()) == 0 &&
          QString::compare(mountPoint->mountType(), "smbfs", Qt::CaseInsensitive) == 0)
#endif
      {
        validMountPoints << mountPoint->mountPoint();
        mountPointOk = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }
      
    //
    // Stop here if the mountpoint is not valid
    //
    if (!mountPointOk)
    {
      reply.setType(ActionReply::HelperErrorType);
      reply.setErrorDescription(i18n("The mountpoint %1 is invalid.", entry.value("mh_mountpoint").toString()));
      break;
    }
    else
    {
      // Do nothing
    }
  }
    
  //
  // If everything went fine, create the process and unmount the shares. For the
  // path of the unmount program and the options we use the first entry in the 
  // map.
  //
  QStringList errorMessages;
    
  if (!validMountPoints.isEmpty())
  {
    // The command
    QStringList command;
    command << args.first().toMap().value("mh_command").toString();
    command << args.first().toMap().value("mh_options").toStringList();
    command << validMountPoints;
      
    // The process
    KProcess proc(this);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    proc.setProgram(command);
      
    // Start the process
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
        else
        {
          // Do nothing
        }
      }

      if (proc.exitStatus() == KProcess::CrashExit)
      {
        if (!userKill)
        {
          reply.setType(ActionReply::HelperErrorType);
          reply.setErrorDescription(i18n("The unmount process crashed."));
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
        errorMessages << stdErr.trimmed();
        errorMessages << "";
      }
    }
    else
    {
      reply.setType(ActionReply::HelperErrorType);
      reply.setErrorDescription(i18n("The unmount process could not be started."));
    }
  }
  else
  {
    // Do nothing
  }
    
  //
  // Pass the error messages to the reply object
  //  
  if (!errorMessages.isEmpty())
  {
    reply.addData("mh_error_message_0", errorMessages);
  }
  else
  {
    // Do nothing
  }
  
  //
  // Return the reply
  //
  return reply;
}


