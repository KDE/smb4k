/***************************************************************************
    The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2019 by Alexander Reinholdt
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

// Application specific includes
#include "smb4kmounter.h"
#include "smb4kmounter_p.h"
#include "smb4kauthinfo.h"
#include "smb4kshare.h"
#include "smb4ksettings.h"
#include "smb4khomesshareshandler.h"
#include "smb4kwalletmanager.h"
#include "smb4kprocess.h"
#include "smb4knotification.h"
#include "smb4kbookmarkhandler.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kbookmark.h"
#include "smb4kprofilemanager.h"
#include "smb4khardwareinterface.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QTimer>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>
#include <QTest>
#include <QUdpSocket>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KCoreAddons/KShell>
#include <KCoreAddons/KUser>
#include <KIOCore/KIO/Global>
#include <KIOCore/KIO/StatJob>
#include <KIOCore/KMountPoint>
#include <KIOCore/KDiskFreeSpaceInfo>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>
#include <KAuth/KAuthExecuteJob>

using namespace Smb4KGlobal;

#define TIMEOUT 50

Q_GLOBAL_STATIC(Smb4KMounterStatic, p);



Smb4KMounter::Smb4KMounter(QObject *parent)
: KCompositeJob(parent), d(new Smb4KMounterPrivate)
{
  setAutoDelete(false);

  d->timerId = -1;
  d->remountTimeout = 0;
  d->remountAttempts = 0;
  d->checkTimeout = 0;
  d->newlyMounted = 0;
  d->newlyUnmounted = 0;
  d->dialog = 0;
  d->firstImportDone = false;
  d->mountShares = false;
  d->unmountShares = false;
  d->activeProfile = Smb4KProfileManager::self()->activeProfile();
  d->detectAllShares = Smb4KMountSettings::detectAllShares();

  // Connections
  connect(Smb4KHardwareInterface::self(), SIGNAL(onlineStateChanged(bool)), this, SLOT(slotOnlineStateChanged(bool)));
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkShareAdded()), this, SLOT(slotTriggerImport()));
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkShareRemoved()), this, SLOT(slotTriggerImport()));
  
  connect(Smb4KProfileManager::self(), SIGNAL(migratedProfile(QString,QString)), this, SLOT(slotProfileMigrated(QString,QString)));
  connect(Smb4KProfileManager::self(), SIGNAL(aboutToChangeProfile()), this, SLOT(slotAboutToChangeProfile()));
  connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)), this, SLOT(slotActiveProfileChanged(QString)));
  
  connect(Smb4KMountSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));
  
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),this, SLOT(slotAboutToQuit()));
}


Smb4KMounter::~Smb4KMounter()
{
  while (!d->importedShares.isEmpty())
  {
    d->importedShares.takeFirst().clear();
  }

  while (!d->retries.isEmpty())
  {
    d->retries.takeFirst().clear();
  }
}


Smb4KMounter *Smb4KMounter::self()
{
  return &p->instance;
}


void Smb4KMounter::abortAll()
{
  if (!QCoreApplication::closingDown())
  {
    QListIterator<KJob *> it(subjobs());
    
    while (it.hasNext())
    {
      it.next()->kill(KJob::EmitResult);
    }
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KMounter::isRunning()
{
  return hasSubjobs();
}


void Smb4KMounter::triggerRemounts(bool fill_list)
{
  if (Smb4KMountSettings::remountShares() /* one-time remounts */ || 
      !Smb4KCustomOptionsManager::self()->sharesToRemount().isEmpty() /* permanent remounts */)
  {
    if (fill_list)
    {
      //
      // Get the shares that are to be remounted
      //
      QList<OptionsPtr> list = Smb4KCustomOptionsManager::self()->sharesToRemount();

      if (!list.isEmpty())
      {
        //
        // Check which shares actually need to be remounted
        //
        for (const OptionsPtr &opt : list)
        {
          QList<SharePtr> mountedShares = findShareByUrl(opt->url());
          
          if (!mountedShares.isEmpty())
          {
            bool mount = true;
            
            for (const SharePtr &s : mountedShares)
            {
              if (!s->isForeign())
              {
                mount = false;
                break;
              }
              else
              {
                continue;
              }
            }
            
            if (mount)
            {
              SharePtr share = SharePtr(new Smb4KShare());
              share->setUrl(opt->url());
              share->setWorkgroupName(opt->workgroupName());
              share->setHostIpAddress(opt->ipAddress());
              
              if (share->url().isValid() && !share->url().isEmpty())
              {
                d->remounts << share;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
          }
          else
          {
            SharePtr share = SharePtr(new Smb4KShare());
            share->setUrl(opt->url());
            share->setWorkgroupName(opt->workgroupName());
            share->setHostIpAddress(opt->ipAddress());
              
            if (share->url().isValid() && !share->url().isEmpty())
            {
              d->remounts << share;
            }
            else
            {
              // Do nothing
            }
          }
        }
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }
    
    if (!d->remounts.isEmpty())
    {
      mountShares(d->remounts);
    }
    else
    {
      // Do nothing
    }

    d->remountAttempts++;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::import(bool checkInaccessible)
{
  //
  // Immediately return here if we are still processing imported shares
  //
  if (!d->importedShares.isEmpty())
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // Get the mountpoints that are present on the system
  //
  KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
  
  // Now determine all mountpoints that have the SMBFS or the CIFS
  // filesystem.
  for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : mountPoints)
  {
    if (QString::compare(mountPoint->mountType(), "cifs") == 0 || QString::compare(mountPoint->mountType(), "smbfs") == 0)
    {
      // Create new share and set the mountpoint and the filesystem
      SharePtr share = SharePtr(new Smb4KShare());
      share->setUrl(mountPoint->mountedFrom());
      share->setPath(mountPoint->mountPoint());
      share->setMounted(true);
      
      // Get all mount options
      for (const QString &option : mountPoint->mountOptions())
      {
        if (option.startsWith(QLatin1String("domain=")) || option.startsWith(QLatin1String("workgroup=")))
        {
          share->setWorkgroupName(option.section('=', 1, 1).trimmed());
        }
        else if (option.startsWith(QLatin1String("addr=")))
        {
          share->setHostIpAddress(option.section('=', 1, 1).trimmed());
        }
        else if (option.startsWith(QLatin1String("username=")) || option.startsWith(QLatin1String("user=")))
        {
          share->setLogin(option.section('=', 1, 1).trimmed());
        }
        else
        {
          // Do nothing
        }
      }
      
      // Work around empty usernames
      if (share->login().isEmpty())
      {
        share->setLogin("guest");
      }
      else
      {
        // Do nothing
      }
      
      d->importedShares << share;
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Check which shares were unmounted. Remove all obsolete mountpoints, emit
  // the unmounted() signal on each of the unmounted shares and remove them
  // from the global list.
  // NOTE: The unmount() signal is emitted *BEFORE* the share is removed
  // from the global list! You need to account for that in your application.
  //
  QList<SharePtr> unmountedShares;
  bool found = false;
  
  for (const SharePtr &mountedShare : mountedSharesList())
  {
    for (const SharePtr &importedShare : d->importedShares)
    {
      // Check the mountpoint, since that one is unique. We will only use
      // Smb4KShare::path(), so that we do not run into trouble if a share 
      // is inaccessible.
      if (QString::compare(mountedShare->path(), importedShare->path()) == 0)
      {
        found = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    if (!found)
    {
      // Remove the mountpoint if the share is not a foreign one
      if (!mountedShare->isForeign())
      {
        QDir dir;
        dir.cd(mountedShare->canonicalPath());
        dir.rmdir(dir.canonicalPath());
        
        if (dir.cdUp())
        {
          dir.rmdir(dir.canonicalPath());
        }
        else
        {
          // Do nothing
        }        
      }
      else
      {
        // Do nothing
      }
      
      mountedShare->setMounted(false);
      unmountedShares << mountedShare;
    }
    else
    {
      // Do nothing
    }
    
    found = false;
  }
  
  if (!unmountedShares.isEmpty())
  {
    d->newlyUnmounted += unmountedShares.size();
    
    if (!d->mountShares && !d->unmountShares)
    {
      if (d->newlyUnmounted == 1)
      {
        // Copy the share
        SharePtr unmountedShare = unmountedShares.first();
        
        // Remove the share from the global list and notify the program and user
        removeMountedShare(unmountedShares.first());
        emit unmounted(unmountedShare);
        Smb4KNotification::shareUnmounted(unmountedShare);
        unmountedShare.clear();
      }
      else
      {
        for (const SharePtr &share : unmountedShares)
        {
          // Copy the share
          SharePtr unmountedShare = share;
          
          // Remove the share from the global list and notify the program
          removeMountedShare(share);
          emit unmounted(unmountedShare);
          unmountedShare.clear();
        }
        
        // Notify the user
        Smb4KNotification::sharesUnmounted(d->newlyUnmounted);
      }
      
      d->newlyUnmounted = 0;
    }
    else
    {
      for (const SharePtr &share : unmountedShares)
      {
        // Copy the share
        SharePtr unmountedShare = share;
        
        // Remove the share from the global list and notify the program
        removeMountedShare(share);
        emit unmounted(unmountedShare);
        unmountedShare.clear();
      }
    }
    
    emit mountedSharesListChanged();
  }
  else
  {
    d->newlyUnmounted = 0;
  }
  
  //
  // Now stat the imported shares to get information about them.
  // Do not use Smb4KShare::canonicalPath() here, otherwise we might
  // get lock-ups with inaccessible shares.
  //
  if (Smb4KHardwareInterface::self()->isOnline())
  {
    QMutableListIterator<SharePtr> it(d->importedShares);
    
    while (it.hasNext())
    {
      SharePtr share = it.next();
      SharePtr mountedShare = findShareByPath(share->path());
      
      if (mountedShare)
      {
        if (mountedShare->isInaccessible() && !checkInaccessible)
        {
          it.remove();
          continue;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Do nothing
      }
      
      QUrl url = QUrl::fromLocalFile(share->path());
      KIO::StatJob *job = KIO::stat(url, KIO::HideProgressInfo);
      job->setDetails(0);
      connect(job, SIGNAL(result(KJob*)), this, SLOT(slotStatResult(KJob*)));
       
      // Do not use addSubJob(), because that would confuse isRunning(), etc.
      
      job->start();
    }
    
    if (!d->firstImportDone && d->importedShares.isEmpty())
    {
      d->firstImportDone = true;
    }
    else
    {
      // Do nothing. d->firstImportDone will be set in slotStatResult().
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::mountShare(const SharePtr &share, QWidget *parent)
{
  Q_ASSERT(share);
  
  if (share)
  {
    //
    // Check that the URL is valid
    //
    if (!share->url().isValid())
    {
      Smb4KNotification::invalidURLPassed();
      return;
    }
    else
    {
      // Do nothing
    }
    
    //
    // Check if the share has already been mounted. If it is already present,
    // do not process it and return.
    //
    QUrl url;
    
    if (share->isHomesShare())
    {
      if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true, parent))
      {
        return;
      }
      else
      {
        // Do nothing
      }

      url = share->homeUrl();     
    }
    else
    {
      url = share->url();
    }
    
    QList<SharePtr> mountedShares = findShareByUrl(url);
    bool isMounted = false;
      
    for (const SharePtr &s : mountedShares)
    {
      if (!s->isForeign())
      {
        isMounted = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    if (isMounted)
    {
      return;
    }
    else
    {
      // Do nothing
    }

    //
    // Wake-On-LAN: Wake up the host before mounting 
    //
    if (Smb4KSettings::enableWakeOnLAN())
    {
      OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(KIO::upUrl(share->url()));

      if (options && options->wolSendBeforeMount())
      {
        emit aboutToStart(WakeUp);

        QUdpSocket *socket = new QUdpSocket(this);
        QHostAddress addr;

        // Use the host's IP address directly from the share object.
        if (share->hasHostIpAddress())
        {
          addr.setAddress(share->hostIpAddress());
        }
        else
        {
          addr.setAddress("255.255.255.255");
        }

        // Construct magic sequence
        QByteArray sequence;

        // 6 times 0xFF
        for (int j = 0; j < 6; ++j)
        {
          sequence.append(QChar(0xFF).toLatin1());
        }

        // 16 times the MAC address
        QStringList parts = options->macAddress().split(':', QString::SkipEmptyParts);

        for (int j = 0; j < 16; ++j)
        {
          for (int k = 0; k < parts.size(); ++k)
          {
            sequence.append(QChar(QString("0x%1").arg(parts.at(k)).toInt(0, 16)).toLatin1());
          }
        }
            
        socket->writeDatagram(sequence, addr, 9);
        
        delete socket;
        
        // Wait the defined time
        int stop = 1000 * Smb4KSettings::wakeOnLANWaitingTime() / 250;
        int i = 0;
        
        while (i++ < stop)
        {
          QTest::qWait(250);
        }
        
        emit finished(WakeUp);
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }
    
    // 
    // Create the mountpoint
    //
    QString mountpoint;
    mountpoint += Smb4KMountSettings::mountPrefix().path();
    mountpoint += QDir::separator();
    mountpoint += (Smb4KMountSettings::forceLowerCaseSubdirs() ? share->hostName().toLower() : share->hostName());
    mountpoint += QDir::separator();

    if (!share->isHomesShare())
    {
      mountpoint += (Smb4KMountSettings::forceLowerCaseSubdirs() ? share->shareName().toLower() : share->shareName());
    }
    else
    {
      mountpoint += (Smb4KMountSettings::forceLowerCaseSubdirs() ? share->login().toLower() : share->login());
    }
    
    // Get the permissions that should be used for creating the
    // mount prefix and all its subdirectories. 
    // Please note that the actual permissions of the mount points
    // are determined by the mount utility.
    QFile::Permissions permissions;
    QUrl parentDirectory;
      
    if (QFile::exists(Smb4KMountSettings::mountPrefix().path()))
    {
      parentDirectory = Smb4KMountSettings::mountPrefix();
    }
    else
    {
      QUrl u = Smb4KMountSettings::mountPrefix();
      parentDirectory = KIO::upUrl(u);
    }
      
    QFile f(parentDirectory.path());
    permissions = f.permissions();
      
    QDir dir(mountpoint);

    if (!dir.mkpath(dir.path()))
    {
      share->setPath("");
      Smb4KNotification::mkdirFailed(dir);
      return;
    }
    else
    {
      QUrl u = QUrl::fromLocalFile(dir.path());
      
      while (!parentDirectory.matches(u, QUrl::StripTrailingSlash))
      {
        QFile(u.path()).setPermissions(permissions);
        u = KIO::upUrl(u);
      }
    }
    
    share->setPath(QDir::cleanPath(mountpoint));
    
    // 
    // Get the authentication information
    //
    Smb4KWalletManager::self()->readAuthInfo(share);

    //
    // Mount arguments
    //
    QVariantMap args;
    
    if (!fillMountActionArgs(share, args))
    {
      return;
    }
    else
    {
      // Do nothing
    }
    
    //
    // Emit the aboutToStart() signal
    //
    emit aboutToStart(MountShare);

    //
    // Create the mount action
    //
    KAuth::Action mountAction("org.kde.smb4k.mounthelper.mount");
    mountAction.setHelperId("org.kde.smb4k.mounthelper");
    mountAction.setArguments(args);
    
    KAuth::ExecuteJob *job = mountAction.execute();
      
    //
    // Modify the cursor, if necessary.
    //
    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }
      
    //
    // Add the job
    //
    addSubjob(job);

    //
    // Start the job and process the returned result.
    //
    bool success = job->exec();
    
    if (success)
    {
      int errorCode = job->error();
      
      if (errorCode == 0)
      {
        // Get the error message
        QString errorMsg = job->data()["mh_error_message"].toString();
        
        if (!errorMsg.isEmpty())
        {
#if defined(Q_OS_LINUX)
          if (errorMsg.contains("mount error 13") || errorMsg.contains("mount error(13)") /* authentication error */)
          {
            if (Smb4KWalletManager::self()->showPasswordDialog(share))
            {
              d->retries << share;
            }
            else
            {
              // Do nothing
            }
          }
          else if (errorMsg.contains("Unable to find suitable address."))
          {
            // Swallow this
          }
          else
          {
            Smb4KNotification::mountingFailed(share, errorMsg);
          }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
          if (errorMsg.contains("Authentication error") || errorMsg.contains("Permission denied"))
          {
            if (Smb4KWalletManager::self()->showPasswordDialog(share, 0))
            {
              d->retries << share;
            }
            else
            {
              // Do nothing
            }
          }
          else
          {
            Smb4KNotification::mountingFailed(share, errorMsg);
          }
#else
          qWarning() << "Smb4KMounter::slotMountJobFinished(): Error handling not implemented!";
          Smb4KNotification::mountingFailed(share.data(), errorMsg);
#endif
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        Smb4KNotification::actionFailed(errorCode);
      }
    }
    else
    {
      // FIXME: Report that the action could not be started
      // Do nothing
    }
    
    //
    // Remove the job from the job list
    //
    removeSubjob(job);
      
    //
    // Reset the busy cursor
    //
    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::restoreOverrideCursor();
    }
    else
    {
      // Do nothing
    }
    
    //
    // Emit the finished() signal
    //
    emit finished(MountShare);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::mountShares(const QList<SharePtr> &shares, QWidget *parent)
{
  d->mountShares = true;
  
  for (const SharePtr &share : shares)
  {
    mountShare(share, parent);
  }
  
  d->mountShares = false;
}


void Smb4KMounter::unmountShare(const SharePtr &share, bool silent, QWidget *parent)
{
  Q_ASSERT(share);
  
  if (share)
  {
    //
    // Check that the URL is valid.
    //
    if (!share->url().isValid())
    {
      Smb4KNotification::invalidURLPassed();
      return;
    }
    else
    {
      // Do nothing
    }
    
    //
    // Handle foreign shares according to the settings
    //
    if (share->isForeign())
    {
      if (!Smb4KMountSettings::unmountForeignShares())
      {
        if (!silent)
        {
          Smb4KNotification::unmountingNotAllowed(share);
        }
        else
        {
          // Do nothing
        }
        return;
      }
      else
      {
        if (!silent)
        {
          if (KMessageBox::warningYesNo(parent,
              i18n("<qt><p>The share <b>%1</b> is mounted to <br><b>%2</b> and owned by user <b>%3</b>.</p>"
                  "<p>Do you really want to unmount it?</p></qt>",
                  share->displayString(), share->path(), share->user().loginName()),
              i18n("Foreign Share")) == KMessageBox::No)
          {
            return;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Without the confirmation of the user, we are not
          // unmounting a foreign share!
          return;
        }
      }
    }
    else
    {
      // Do nothing
    }
    
    //
    // Force the unmounting of the share either if the system went offline
    // or if the user chose to forcibly unmount inaccessible shares (Linux only).
    //
    bool force = false;
    
    if (Smb4KHardwareInterface::self()->isOnline())
    {
#if defined(Q_OS_LINUX)
      if (share->isInaccessible())
      {
        force = Smb4KMountSettings::forceUnmountInaccessible();
      }
      else
      {
        // Do nothing
      }
#endif
    }
    else
    {
      force = true;
    }
    
    //
    // Unmount arguments
    //
    QVariantMap args;

    if (!fillUnmountActionArgs(share, force, silent, args))
    {
      return;
    }
    else
    {
      // Do nothing
    }
    
    //
    // Emit the aboutToStart() signal
    //
    emit aboutToStart(UnmountShare);

    //
    // Create the unmount action
    //
    KAuth::Action unmountAction("org.kde.smb4k.mounthelper.unmount");
    unmountAction.setHelperId("org.kde.smb4k.mounthelper");
    unmountAction.setArguments(args);
      
    KAuth::ExecuteJob *job = unmountAction.execute();
    
    //
    // Modify the cursor, if necessary.
    //
    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }
      
    //
    // Add the job
    //
    addSubjob(job);

    //
    // Start the job and process the returned result.
    //
    bool success = job->exec();
    
    if (success)
    {
      int errorCode = job->error();
      
      if (errorCode == 0)
      {
        // Get the error message
        QString errorMsg = job->data()["mh_error_message"].toString();
        
        if (!errorMsg.isEmpty())
        {
          // No error handling needed, just report the error message.
          Smb4KNotification::unmountingFailed(share, errorMsg);
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        Smb4KNotification::actionFailed(errorCode);
      }

    }
    else
    {
      // FIXME: Report that the action could not be started
      // Do nothing
    }
    
    //
    // Remove the job from the job list
    //
    removeSubjob(job);
      
    //
    // Reset the busy cursor
    //
    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::restoreOverrideCursor();
    }
    else
    {
      // Do nothing
    }
    
    //
    // Emit the finished() signal
    //
    emit finished(UnmountShare);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::unmountShares(const QList<SharePtr> &shares, bool silent, QWidget *parent)
{
#if defined(Q_OS_LINUX)
  //
  // Under Linux, we have to take an approach that is a bit awkward in this function. 
  // Since the import function is invoked via Smb4KHardwareInterface::networkShareRemoved()
  // before mountShare() returns, no unmount of multiple shares will ever be reported
  // when d->unmountShares is set to FALSE *after* the loop ended. To make the reporting
  // work correctly, we need to set d->unmountShares to FALSE before the last unmount
  // is started.
  //
  d->unmountShares = true;
  int number = shares.size();
  
  for (const SharePtr &share : shares)
  {
    number--;
    d->unmountShares = (number != 0);
    unmountShare(share, silent, parent);
  }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
  //
  // Since under FreeBSD the emission of Smb4KHardwareInterface::networkShareRemoved() is 
  // triggered by a timer, we can use a nice approach here.
  //
  d->unmountShares = true;
  
  for (const SharePtr &share : shares)
  {
    unmountShare(share, silent, parent);
  }
  
  d->unmountShares = false;
#endif
}


void Smb4KMounter::unmountAllShares(bool silent, QWidget* parent)
{
  unmountShares(mountedSharesList(), silent, parent);
}


void Smb4KMounter::openMountDialog(QWidget *parent)
{
  if (!d->dialog)
  {
    SharePtr share = SharePtr(new Smb4KShare());
    
    d->dialog = new Smb4KMountDialog(share, parent);

    if (d->dialog->exec() == QDialog::Accepted && d->dialog->validUserInput())
    {
      // Pass the share to mountShare().
      mountShare(share, parent);
      
      // Bookmark the share if the user wants this.
      if (d->dialog->bookmarkShare())
      {
        Smb4KBookmarkHandler::self()->addBookmark(share);
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }

    delete d->dialog;
    d->dialog = 0;

    share.clear();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::start()
{
  //
  // Check the network configurations
  //
  Smb4KHardwareInterface::self()->updateNetworkConfig();
  
  //
  // Connect to Smb4KHardwareInterface to be able to get the response
  // 
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkConfigUpdated()), this, SLOT(slotStartJobs()));
}


void Smb4KMounter::saveSharesForRemount()
{
  //
  // Save the shares for remount
  //
  for (const SharePtr &share : mountedSharesList())
  {
    if (!share->isForeign())
    {
      Smb4KCustomOptionsManager::self()->addRemount(share, false);
    }
    else
    {
      Smb4KCustomOptionsManager::self()->removeRemount(share, false);
    }
  }
  
  //
  // Also save each failed remount and remove it from the list
  //
  while (!d->remounts.isEmpty())
  {
    SharePtr share = d->remounts.takeFirst();
    Smb4KCustomOptionsManager::self()->addRemount(share, false);
    share.clear();
  }  
}


void Smb4KMounter::timerEvent(QTimerEvent *)
{
  // Try to remount those shares that could not be mounted 
  // before. Do this only if there are no subjobs, because we
  // do not want to get crashes because a share was invalidated
  // during processing the shares.
  if ((Smb4KMountSettings::remountShares() || !Smb4KCustomOptionsManager::self()->sharesToRemount().isEmpty()) && 
       Smb4KMountSettings::remountAttempts() > d->remountAttempts)
  {
    if (d->firstImportDone && !isRunning())
    {
      if (d->remountAttempts == 0)
      {
        triggerRemounts(true);
      }
      else if (!d->remounts.isEmpty() && d->remountTimeout >= (60000 * Smb4KMountSettings::remountInterval()))
      {
        triggerRemounts(false);
        d->remountTimeout = -TIMEOUT;
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }
    
    d->remountTimeout += TIMEOUT;
  }
  else
  {
    // Do nothing
  }
  
  // Retry mounting those shares that failed. This is also only 
  // done when there are no subjobs.
  if (!d->retries.isEmpty() && !hasSubjobs())
  {
    mountShares(d->retries);
      
    while (!d->retries.isEmpty())
    {
      d->retries.takeFirst().clear();
    }
  }
  else
  {
    // Do nothing
  }
  
  //
  // Check the size, accessibility, etc. of the shares
  // 
  // FIXME: Hopefully we can replace this with a recursive QFileSystemWatcher 
  // approach in the future. However, using the existing QFileSystemWatcher
  // and a QDirIterator to add all the subdirectories of a share to the watcher
  // seems to be too resource consuming...
  //
  if (d->checkTimeout >= 2500 && !isRunning() && d->importedShares.isEmpty())
  {
    for (const SharePtr &share : mountedSharesList())
    {
      check(share);
      emit updated(share);
    }

    d->checkTimeout = 0;
  }
  else
  {
    d->checkTimeout += TIMEOUT;
  }
}


#if defined(Q_OS_LINUX)
//
// Linux arguments
//
bool Smb4KMounter::fillMountActionArgs(const SharePtr &share, QVariantMap& map)
{
  //
  // Find the mount executable
  // 
  const QString mount = findMountExecutable();
  
  if (!mount.isEmpty())
  {
    map.insert("mh_command", mount);
  }
  else
  {
    Smb4KNotification::commandNotFound("mount.cifs");
    return false;
  }
  
  //
  // Global and custom options
  // 
  QMap<QString, QString> globalOptions = globalSambaOptions();
  OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(share);
  
  //
  // Pass the remote file system port to the URL
  // 
  if (options)
  {
    if (options->useFileSystemPort())
    {
      share->setPort(options->fileSystemPort());
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useRemoteFileSystemPort())
    {
      share->setPort(Smb4KMountSettings::remoteFileSystemPort());
    }
    else
    {
      // Do nothing
    }
  }

  //
  // List of arguments passed via "-o ..." to the mount command
  // 
  QStringList argumentsList;

  //
  // Workgroup or domain
  // 
  if (!share->workgroupName().trimmed().isEmpty())
  {
    argumentsList << QString("domain=%1").arg(KShell::quoteArg(share->workgroupName()));
  }
  else
  {
    // Do nothing
  }
  
  //
  // Host IP address
  // 
  if (share->hasHostIpAddress())
  {
    argumentsList << QString("ip=%1").arg(share->hostIpAddress());
  }
  else
  {
    // Do nothing
  }
  
  //
  // User name (login)
  // 
  if (!share->login().isEmpty())
  {
    argumentsList << QString("username=%1").arg(share->login());
  }
  else
  {
    argumentsList << "guest";
  }
  
  //
  // Client's and server's NetBIOS name
  // 
  // According to the manual page, this is only needed when port 139
  // is used. So, we only pass the NetBIOS name in that case.
  // 
  if (options)
  {
    if (options->useFileSystemPort() && options->fileSystemPort() == 139)
    {
      // The client's NetBIOS name
      if (!Smb4KSettings::netBIOSName().isEmpty())
      {
        argumentsList << QString("netbiosname=%1").arg(KShell::quoteArg(Smb4KSettings::netBIOSName()));
      }
      else if (!globalOptions["netbios name"].isEmpty())
      {
        argumentsList << QString("netbiosname=%1").arg(KShell::quoteArg(globalOptions["netbios name"]));
      }
      else
      {
        // Do nothing
      }
      
      // The server's NetBIOS name
      argumentsList << QString("servernetbiosname=%1").arg(KShell::quoteArg(share->hostName()));
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useRemoteFileSystemPort() && Smb4KMountSettings::remoteFileSystemPort() == 139)
    {
      // The client's NetBIOS name
      if (!Smb4KSettings::netBIOSName().isEmpty())
      {
        argumentsList << QString("netbiosname=%1").arg(KShell::quoteArg(Smb4KSettings::netBIOSName()));
      }
      else if (!globalOptions["netbios name"].isEmpty())
      {
        argumentsList << QString("netbiosname=%1").arg(KShell::quoteArg(globalOptions["netbios name"]));
      }
      else
      {
        // Do nothing
      }
      
      // The server's NetBIOS name
      argumentsList << QString("servernetbiosname=%1").arg(KShell::quoteArg(share->hostName()));
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // CIFS Unix extensions support
  // 
  // This sets the uid, gid, file_mode and dir_mode arguments, if necessary.
  // 
  if (options)
  {
    if (!options->cifsUnixExtensionsSupport())
    {
      // User id
      if (options->useUser())
      {
        argumentsList << QString("uid=%1").arg(options->user().userId().nativeId());
      }
      else
      {
        // Do nothing
      }
      
      // Group id
      if (options->useGroup())
      {
        argumentsList << QString("gid=%1").arg(options->group().groupId().nativeId());
      }
      else
      {
        // Do nothing
      }
      
      // File mode
      if (options->useFileMode())
      {
        argumentsList << QString("file_mode=%1").arg(options->fileMode());
      }
      else
      {
        // Do nothing
      }
      
      // Directory mode
      if (options->useDirectoryMode())
      {
        argumentsList << QString("dir_mode=%1").arg(options->directoryMode());
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (!Smb4KMountSettings::cifsUnixExtensionsSupport())
    {
      // User id
      if (Smb4KMountSettings::useUserId())
      {
        argumentsList << QString("uid=%1").arg((K_UID)Smb4KMountSettings::userId().toInt());
      }
      else
      {
        // Do nothing
      }
      
      // Group id
      if (Smb4KMountSettings::useGroupId())
      {
        argumentsList << QString("gid=%1").arg((K_GID)Smb4KMountSettings::groupId().toInt());
      }
      else
      {
        // Do nothing
      }
      
      // File mode
      if (Smb4KMountSettings::useFileMode())
      {
        argumentsList << QString("file_mode=%1").arg(Smb4KMountSettings::fileMode());
      }
      else
      {
        // Do nothing
      }
      
      // Directory mode
      if (Smb4KMountSettings::useDirectoryMode())
      {
        argumentsList << QString("dir_mode=%1").arg(Smb4KMountSettings::directoryMode());
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Force user id
  // 
  // FIXME: The manual page is not clear about this: Is this option only useful when the uid=...
  // argument is given? If so, this should be moved into the 'User id' code block above.
  // 
  if (Smb4KMountSettings::forceUID())
  {
    argumentsList << "forceuid";
  }
  else
  {
    // Do nothing
  }
  
  //
  // Force group id
  // 
  // FIXME: The manual page is not clear about this: Is this option only useful when the gid=...
  // argument is given? If so, this should be moved into the 'Group id' code block above.
  //   
  if (Smb4KMountSettings::forceGID())
  {
    argumentsList << "forcegid";
  }
  else
  {
    // Do nothing
  }
  
  //
  // Client character set
  // 
  if (Smb4KMountSettings::useClientCharset())
  {
    switch (Smb4KMountSettings::clientCharset())
    {
      case Smb4KMountSettings::EnumClientCharset::default_charset:
      {
        if (!globalOptions["unix charset"].isEmpty())
        {
          argumentsList << QString("iocharset=%1").arg(globalOptions["unix charset"].toLower());
        }
        else
        {
          // Do nothing
        }
        break;
      }
      default:
      {
        argumentsList << QString("iocharset=%1").arg(Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label);
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  //
  // File system port
  // 
  if (options)
  {
    if (options->useFileSystemPort())
    {
      argumentsList << QString("port=%1").arg(options->fileSystemPort());
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useRemoteFileSystemPort())
    {
      argumentsList << QString("port=%1").arg(Smb4KMountSettings::remoteFileSystemPort());
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Write access
  // 
  if (options)
  {
    if (options->useWriteAccess())
    {
      switch (options->writeAccess())
      {
        case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
        {
          argumentsList << "rw";
          break;
        }
        case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
        {
          argumentsList << "ro";
          break;
        }
        default:
        {
          break;
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useWriteAccess())
    {
      switch (Smb4KMountSettings::writeAccess())
      {
        case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
        {
          argumentsList << "rw";
          break;
        }
        case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
        {
          argumentsList << "ro";
          break;
        }
        default:
        {
          break;
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Permission checks
  // 
  if (Smb4KMountSettings::permissionChecks())
  {
    argumentsList << "perm";
  }
  else
  {
    argumentsList << "noperm";
  }
  
  //
  // Client controls ids
  // 
  if (Smb4KMountSettings::clientControlsIDs())
  {
    argumentsList << "setuids";
  }
  else
  {
    argumentsList << "nosetuids";
  }
  
  //   
  // Server inode numbers
  // 
  if (Smb4KMountSettings::serverInodeNumbers())
  {
    argumentsList << "serverino";
  }
  else
  {
    argumentsList << "noserverino";
  }
  
  //
  // Cache mode
  //
  if (Smb4KMountSettings::useCacheMode())
  {
    switch (Smb4KMountSettings::cacheMode())
    {
      case Smb4KMountSettings::EnumCacheMode::None:
      {
        argumentsList << "cache=none";
        break;
      }
      case Smb4KMountSettings::EnumCacheMode::Strict:
      {
        argumentsList << "cache=strict";
        break;
      }
      case Smb4KMountSettings::EnumCacheMode::Loose:
      {
        argumentsList << "cache=loose";
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  //  
  // Translate reserved characters
  //
  if (Smb4KMountSettings::translateReservedChars())
  {
    argumentsList << "mapchars";
  }
  else
  {
    argumentsList << "nomapchars";
  }

  //   
  // Locking
  //
  if (Smb4KMountSettings::noLocking())
  {
    argumentsList << "nolock";
  }
  else
  {
    // Do nothing
  }
  
  // 
  // Security mode
  // 
  if (options)
  {
    if (options->useSecurityMode())
    {
      switch (options->securityMode())
      {
        case Smb4KMountSettings::EnumSecurityMode::None:
        {
          argumentsList << "sec=none";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Krb5:
        {
          argumentsList << "sec=krb5";
          argumentsList << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Krb5i:
        {
          argumentsList << "sec=krb5i";
          argumentsList << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlm:
        {
          argumentsList << "sec=ntlm";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
        {
          argumentsList << "sec=ntlmi";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
        {
          argumentsList << "sec=ntlmv2";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
        {
          argumentsList << "sec=ntlmv2i";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
        {
          argumentsList << "sec=ntlmssp";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
        {
          argumentsList << "sec=ntlmsspi";
          break;
        }
        default:
        {
          // Smb4KSettings::EnumSecurityMode::Default,
          break;
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useSecurityMode())
    {
      switch (Smb4KMountSettings::securityMode())
      {
        case Smb4KMountSettings::EnumSecurityMode::None:
        {
          argumentsList << "sec=none";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Krb5:
        {
          argumentsList << "sec=krb5";
          argumentsList << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Krb5i:
        {
          argumentsList << "sec=krb5i";
          argumentsList << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlm:
        {
          argumentsList << "sec=ntlm";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
        {
          argumentsList << "sec=ntlmi";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
        {
          argumentsList << "sec=ntlmv2";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
        {
          argumentsList << "sec=ntlmv2i";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
        {
          argumentsList << "sec=ntlmssp";
          break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
        {
          argumentsList << "sec=ntlmsspi";
          break;
        }
        default:
        {
          // Smb4KSettings::EnumSecurityMode::Default,
          break;
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // SMB protocol version
  // 
  if (Smb4KMountSettings::useSmbProtocolVersion())
  {
    switch (Smb4KMountSettings::smbProtocolVersion())
    {
      case Smb4KMountSettings::EnumSmbProtocolVersion::OnePointZero:
      {
        argumentsList << "vers=1.0";
        break;
      }
      case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointZero:
      {
        argumentsList << "vers=2.0";
        break;
      }
      case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointOne:
      {
        argumentsList << "vers=2.1";
        break;
      }
      case Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointZero:
      {
        argumentsList << "vers=3.0";
        break;
      }
      case Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointOnePointOne:
      {
        argumentsList << "vers=3.1.1";
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }

  //
  // Mount options provided by the user
  // 
  if (!Smb4KMountSettings::customCIFSOptions().isEmpty())
  {
    // SECURITY: Only pass those arguments to mount.cifs that do not pose
    // a potential security risk and that have not already been defined.
    //
    // This is, among others, the proper fix to the security issue reported
    // by Heiner Markert (aka CVE-2014-2581).
    QStringList whitelist = whitelistedMountArguments();
    QStringList list = Smb4KMountSettings::customCIFSOptions().split(',', QString::SkipEmptyParts);
    QMutableStringListIterator it(list);
    
    while (it.hasNext())
    {
      QString arg = it.next().section("=", 0, 0);
      
      if (!whitelist.contains(arg))
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
      
      argumentsList += list;
    }
  }
  else
  {
    // Do nothing
  }
  
  //
  // Insert the mount options into the map
  // 
  QStringList mh_options;
  mh_options << "-o";
  mh_options << argumentsList.join(",");
  map.insert("mh_options", mh_options);
  
  //
  // Insert the mountpoint into the map
  // 
  map.insert("mh_mountpoint", share->canonicalPath());
  
  //
  // Insert information about the share and its URL into the map
  // 
  if (!share->isHomesShare())
  {
    map.insert("mh_url", share->url());
  }
  else
  {
    map.insert("mh_url", share->homeUrl());
    map.insert("mh_homes_url", share->url());
  }  

  map.insert("mh_workgroup", share->workgroupName());
  map.insert("mh_ip", share->hostIpAddress());
  
  //
  // Location of the Kerberos ticket
  // 
  // The path to the Kerberos ticket is stored - if it exists - in the
  // KRB5CCNAME environment variable. By default, the ticket is located
  // at /tmp/krb5cc_[uid]. So, if the environment variable does not exist,
  // but the cache file is there, try to use it.
  // 
  if (QProcessEnvironment::systemEnvironment().contains("KRB5CCNAME"))
  {
    map.insert("mh_krb5ticket", QProcessEnvironment::systemEnvironment().value("KRB5CCNAME", ""));
  }
  else
  {
    QString ticket = QString("/tmp/krb5cc_%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
    
    if (QFile::exists(ticket))
    {
      map.insert("mh_krb5ticket", "FILE:"+ticket);
    }
    else
    {
      // Do nothing
    }
  }
  
  return true;
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD arguments
//
bool Smb4KMounter::fillMountActionArgs(const SharePtr &share, QVariantMap& map)
{
  //
  // Find the mount executable
  // 
  const QString mount = findMountExecutable();
  
  if (!mount.isEmpty())
  {
    map.insert("mh_command", mount);
  }
  else
  {
    Smb4KNotification::commandNotFound("mount_smbfs");
    return false;
  }
  
  //
  // Global and custom options
  // 
  QMap<QString, QString> globalOptions = globalSambaOptions();
  OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(share);
  
  //
  // List of arguments 
  // 
  QStringList argumentsList;
  
  //
  // Workgroup
  // 
  if (!share->workgroupName().isEmpty())
  {
    argumentsList << "-W";
    argumentsList << KShell::quoteArg(share->workgroupName());
  }
  else
  {
    // Do nothing
  }
  
  //
  // IP address
  // 
  if (!share->hostIpAddress().isEmpty())
  {
    argumentsList << "-I";
    argumentsList << share->hostIpAddress();
  }
  else
  {
    // Do nothing
  }
  
  //
  // User Id
  // 
  if (options)
  {
    if (options->useUser())
    {
      argumentsList << "-u";
      argumentsList << QString("%1").arg(options->user().userId().nativeId());
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useUserId())
    {
      argumentsList << "-u";
      argumentsList << QString("%1").arg((K_UID)Smb4KMountSettings::userId().toInt());
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Group Id
  // 
  if (options)
  {
    if (options->useGroup())
    {
      argumentsList << "-g";
      argumentsList << QString("%1").arg(options->group().groupId().nativeId());
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useGroupId())
    {
      argumentsList << "-g";
      argumentsList << QString("%1").arg((K_GID)Smb4KMountSettings::groupId().toInt());
    }
    else
    {
      // Do nothing
    }
  }
  
  if (Smb4KMountSettings::useCharacterSets())
  {
    // Client character set
    QString clientCharset, serverCharset;
    
    switch (Smb4KMountSettings::clientCharset())
    {
      case Smb4KMountSettings::EnumClientCharset::default_charset:
      {
        clientCharset = globalOptions["unix charset"].toLower(); // maybe empty
        break;
      }
      default:
      {
        clientCharset = Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label;
        break;
      }
    }
    
    // Server character set
    switch (Smb4KMountSettings::serverCodepage())
    {
      case Smb4KMountSettings::EnumServerCodepage::default_codepage:
      {
        serverCharset = globalOptions["dos charset"].toLower(); // maybe empty
        break;
      }
      default:
      {
        serverCharset = Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::serverCodepage()).label;
        break;
      }
    }
    
    if (!clientCharset.isEmpty() && !serverCharset.isEmpty())
    {
      argumentsList << "-E";
      argumentsList << QString("%1:%2").arg(clientCharset, serverCharset);
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }

  //
  // File mode
  // 
  if (options)
  {
    if (options->useFileMode())
    {
      argumentsList << "-f";
      argumentsList << options->fileMode();
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useFileMode())
    {
      argumentsList << "-f";
      argumentsList << Smb4KMountSettings::fileMode();
    }
    else
    {
      // Do nothing
    }
  }

  //
  // Directory mode
  // 
  if (options)
  {
    if (options->useDirectoryMode())
    {
      argumentsList << "-d";
      argumentsList << options->directoryMode();
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KMountSettings::useDirectoryMode())
    {
      argumentsList << "-d";
      argumentsList << Smb4KMountSettings::directoryMode();
    }
    else
    {
      // Do nothing
    }
  }

  //
  // User name (login)
  // 
  if (!share->login().isEmpty())
  {
    argumentsList << "-U";
    argumentsList << share->login();
  }
  else
  {
    argumentsList << "-N";
  }
  
  //
  // Insert the mount options into the map
  // 
  map.insert("mh_options", argumentsList);

  //
  // Insert the mountpoint into the map
  // 
  map.insert("mh_mountpoint", share->canonicalPath());
  
  //
  // Insert information about the share and its URL into the map
  // 
  if (!share->isHomesShare())
  {
    map.insert("mh_url", share->url());
  }
  else
  {
    map.insert("mh_url", share->homeUrl());
    map.insert("mh_homes_url", share->url());
  }  

  map.insert("mh_workgroup", share->workgroupName());
  map.insert("mh_ip", share->hostIpAddress());
  
  return true;
}
#else
//
// Dummy 
//
bool Smb4KMounter::fillMountActionArgs(const SharePtr &, QVariantMap&)
{
  qWarning() << "Smb4KMounter::fillMountActionArgs() is not implemented!";
  qWarning() << "Mounting under this operating system is not supported...";
  return false;
}
#endif


#if defined(Q_OS_LINUX)
//
// Linux arguments
//
bool Smb4KMounter::fillUnmountActionArgs(const SharePtr &share, bool force, bool silent, QVariantMap &map)
{
  //
  // The umount program
  //
  const QString umount = findUmountExecutable();

  if (umount.isEmpty() && !silent)
  {
    Smb4KNotification::commandNotFound("umount");
    return false;
  }
  else
  {
    // Do nothing
  }
  
  //
  // The options
  //
  QStringList options;

  if (force)
  {
    options << "-l"; // lazy unmount
  }
  else
  {
    // Do nothing
  }

  //
  // Insert data into the map
  //
  map.insert("mh_command", umount);
  map.insert("mh_url", share->url());
  
  if (Smb4KHardwareInterface::self()->isOnline())
  {
    map.insert("mh_mountpoint", share->canonicalPath());
  }
  else
  {
    map.insert("mh_mountpoint", share->path());
  }
  
  map.insert("mh_options", options);
  
  return true;
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD arguments
//
bool Smb4KMounter::fillUnmountActionArgs(const SharePtr &share, bool force, bool silent, QVariantMap &map)
{
  //
  // The umount program
  //
  const QString umount = findUmountExecutable();

  if (umount.isEmpty() && !silent)
  {
    Smb4KNotification::commandNotFound("umount");
    return false;
  }
  else
  {
    // Do nothing
  }

  //
  // The options
  //
  QStringList options;
  
  if (force)
  {
    options << "-f";
  }
  else
  {
    // Do nothing
  }

  //
  // Insert data into the map
  //
  map.insert("mh_command", umount);
  map.insert("mh_url", share->url());
  
  if (Smb4KHardwareInterface::self()->isOnline())
  {
    map.insert("mh_mountpoint", share->canonicalPath());
  }
  else
  {
    map.insert("mh_mountpoint", share->path());
  }
  
  map.insert("mh_options", options);
  
  return true;
}
#else
//
// Dummy
//
bool Smb4KMounter::fillUnmountActionArgs(const SharePtr &, bool, bool, QVariantMap &)
{
  qWarning() << "Smb4KMounter::fillUnmountActionArgs() is not implemented!";
  qWarning() << "Unmounting under this operating system is not supported...";
  return false;
}
#endif


void Smb4KMounter::check(const SharePtr &share)
{
  // Get the info about the usage, etc.
  KDiskFreeSpaceInfo spaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(share->path());
    
  if (spaceInfo.isValid())
  {
    // Accessibility
    share->setInaccessible(false);
       
    // Size information
    share->setFreeDiskSpace(spaceInfo.available());
    share->setTotalDiskSpace(spaceInfo.size());
    share->setUsedDiskSpace(spaceInfo.used());
      
    // Get the owner an group, if possible.
    QFileInfo fileInfo(share->path());
    fileInfo.setCaching(false);

    if (fileInfo.exists())
    {
      share->setUser(KUser(static_cast<K_UID>(fileInfo.ownerId())));
      share->setGroup(KUserGroup(static_cast<K_GID>(fileInfo.groupId())));
      share->setInaccessible(!(fileInfo.isDir() && fileInfo.isExecutable()));
    }
    else
    {
      share->setInaccessible(true);
      share->setFreeDiskSpace(0);
      share->setTotalDiskSpace(0);
      share->setUsedDiskSpace(0);
      share->setUser(KUser(KUser::UseRealUserID));
      share->setGroup(KUserGroup(KUser::UseRealUserID));
    }
  }
  else
  {
    share->setInaccessible(true);
    share->setFreeDiskSpace(0);
    share->setTotalDiskSpace(0);
    share->setUsedDiskSpace(0);
    share->setUser(KUser(KUser::UseRealUserID));
    share->setGroup(KUserGroup(KUser::UseRealUserID));
  } 
}



/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KMounter::slotStartJobs()
{
  //
  // Disconnect from Smb4KHardwareInterface.
  //
  disconnect(Smb4KHardwareInterface::self(), SIGNAL(networkConfigUpdated()), this, SLOT(slotStartJobs()));
  
  //
  // Start the import of shares
  // 
  if (Smb4KHardwareInterface::self()->isOnline())
  {
    //
    // Import the mounted shares
    //
    if (!d->firstImportDone)
    {
      import(true);
    }
    else
    {
      // Do nothing
    }
    
    //
    // Start the timer
    //
    if (d->timerId == -1)
    {
      d->timerId = startTimer(TIMEOUT);
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotAboutToQuit()
{
  //
  // Abort any actions
  //
  abortAll();

  //
  // Check if the user wants to remount shares and save the
  // shares for remount if so.
  //
  if (Smb4KMountSettings::remountShares())
  {
    saveSharesForRemount();
  }
  else
  {
    // Do nothing
  }

  //
  // Unmount the shares if the user chose to do so.
  //
  if (Smb4KMountSettings::unmountSharesOnExit())
  {
    unmountAllShares(true, 0);
  }
  else
  {
    // Do nothing
  }

  //
  // Clean up the mount prefix.
  //
  KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
  
  QDir dir;
  dir.cd(Smb4KMountSettings::mountPrefix().path());
  QStringList hostDirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort);
  QStringList mountpoints;
  
  for (const QString &hostDir : hostDirs)
  {
    dir.cd(hostDir);
    
    QStringList shareDirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort);
    
    for (const QString &shareDir : shareDirs)
    {
      dir.cd(shareDir);
      mountpoints << dir.absolutePath();
      dir.cdUp();
    }
    
    dir.cdUp();
  }
  
  // Remove those mountpoints where a share is actually mounted.
  for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : mountPoints)
  {
    mountpoints.removeOne(mountPoint->mountPoint());
  }
  
  // Remove the empty mountpoints.
  for (const QString &mp : mountpoints)
  {
    dir.cd(mp);
    dir.rmdir(dir.canonicalPath());
    
    if (dir.cdUp())
    {
      dir.rmdir(dir.canonicalPath());
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KMounter::slotOnlineStateChanged(bool online)
{
  if (online)
  {
    // Remount shares after the network became available (again)
    // If the computer awakes from a sleep state, there might still be 
    // an unmount job in the queue. So, wait until all jobs have been
    // performed before starting to remount the shares.
    while (isRunning())
    {
      QTest::qWait(TIMEOUT);
    }
    
    triggerRemounts(true);
  }
  else
  {
    // Abort all running mount jobs
    abortAll();
    
    // Mark all mounted shares as inaccessible
    for (const SharePtr &share : mountedSharesList())
    {
      share->setInaccessible(true);
    }
    
    // Save the shares for automatic remounting
    saveSharesForRemount();
    
    // Unmount all shares
    unmountAllShares(true, 0);
  }
}


void Smb4KMounter::slotStatResult(KJob *job)
{
  Q_ASSERT(job);
  
  //
  // Stat job
  //
  KIO::StatJob *statJob = static_cast<KIO::StatJob *>(job);
  
  //
  // Get the mountpoint
  //
  QString mountpoint = statJob->url().toDisplayString(QUrl::PreferLocalFile);
  
  //
  // Find the imported share
  //
  SharePtr importedShare;
  
  for (int i = 0; i < d->importedShares.size(); ++i)
  {
    if (QString::compare(d->importedShares.at(i)->path(), mountpoint) == 0)
    {
      importedShare = d->importedShares.takeAt(i);
      break;
    }
    else
    {
      continue;
    }
  }
  
  //
  // If the share should have vanished in the meantime, return here.
  //
  if (!importedShare)
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // Add the size, user and group information
  //
  if (statJob->error() == 0 /* no error */)
  {
    check(importedShare);
  }
  else
  {
    importedShare->setInaccessible(true);
    importedShare->setFreeDiskSpace(0);
    importedShare->setTotalDiskSpace(0);
    importedShare->setUsedDiskSpace(0);
    importedShare->setUser(KUser(KUser::UseRealUserID));
    importedShare->setGroup(KUserGroup(KUser::UseRealUserID));    
  }
  
  //
  // Is this a mount done by the user or by someone else?
  // Make an educated guess...
  // 
  if ((importedShare->user().userId() == KUser(KUser::UseRealUserID).userId() &&
       importedShare->group().groupId() == KUserGroup(KUser::UseRealUserID).groupId()) ||
      importedShare->path().startsWith(Smb4KMountSettings::mountPrefix().path()) ||
      importedShare->path().startsWith(QDir::homePath()) ||
      importedShare->canonicalPath().startsWith(QDir(Smb4KMountSettings::mountPrefix().path()).canonicalPath()) ||
      importedShare->canonicalPath().startsWith(QDir::home().canonicalPath()))
  {
    // Same UID and GID
    importedShare->setForeign(false);
  }
  else
  {
    importedShare->setForeign(true);
  }
  
  //
  // Search for a previously added mounted share and try to update it. If this fails,
  // add the share to the global list.
  //
  if (!importedShare->isForeign() || Smb4KMountSettings::detectAllShares())
  {
    if (updateMountedShare(importedShare))
    {
      SharePtr updatedShare = findShareByPath(importedShare->path());
      
      if (updatedShare)
      {
        emit updated(updatedShare);
      }
      else
      {
        // Do nothing
      }
      
      importedShare.clear();
    }
    else
    {
      if (addMountedShare(importedShare))
      {
        // Remove the share from the list of shares that are to be remounted
        QMutableListIterator<SharePtr> s(d->remounts);

        while (s.hasNext())
        {
          SharePtr remount = s.next();

          if (!importedShare->isForeign() && 
              QString::compare(remount->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                               importedShare->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                               Qt::CaseInsensitive) == 0)
          {
            Smb4KCustomOptionsManager::self()->removeRemount(remount);
            s.remove();
            break;
          }
          else
          {
            continue;
          }
        }
        
        // Tell the program and the user that the share was mounted. Also, reset the
        // counter of newly mounted shares, if necessary.
        d->newlyMounted += 1;
        emit mounted(importedShare);
        
        if (d->importedShares.isEmpty() && !d->mountShares && !d->unmountShares)
        {
          if (d->firstImportDone)
          {
            if (d->newlyMounted == 1)
            {
              Smb4KNotification::shareMounted(importedShare);
            }
            else
            {
              Smb4KNotification::sharesMounted(d->newlyMounted);
            }
          }
          else
          {
            // Do nothing
          }
          
          d->newlyMounted = 0;
        }
        else
        {
          // Do nothing
        }
        
        emit mountedSharesListChanged();
      }
      else
      {
        importedShare.clear();
      }
    }
  }
  else
  {
    importedShare.clear();
  }
  
  if (!d->firstImportDone && d->importedShares.isEmpty())
  {
    d->firstImportDone = true;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotAboutToChangeProfile()
{
  //
  // Save those shares that are to be remounted
  //
  if (Smb4KMountSettings::remountShares())
  {
    saveSharesForRemount();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotActiveProfileChanged(const QString &newProfile)
{
  if (d->activeProfile != newProfile)
  {
    // Stop the timer.
    killTimer(d->timerId);

    abortAll();
    
    // Clear all remounts.
    while (!d->remounts.isEmpty())
    {
      d->remounts.takeFirst().clear();
    }
    
    // Clear all retries.
    while (!d->retries.isEmpty())
    {
      d->retries.takeFirst().clear();
    }
    
    // Unmount all shares
    unmountAllShares(true, 0);
    
    // Reset some variables.
    d->remountTimeout = 0;
    d->remountAttempts = 0;
    d->firstImportDone = false;
    d->activeProfile = newProfile;
    
    // Restart the timer
    d->timerId = startTimer(TIMEOUT);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotProfileMigrated(const QString& from, const QString& to)
{
  if (QString::compare(from, d->activeProfile, Qt::CaseSensitive) == 0)
  {
    d->activeProfile = to;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotTriggerImport()
{
  // Wait until there are no jobs anymore
  while(isRunning())
  {
    QTest::qWait(TIMEOUT);
  }
  
  // Initialize an import
  import(true);
}


void Smb4KMounter::slotConfigChanged()
{
  if (d->detectAllShares != Smb4KMountSettings::detectAllShares())
  {
    slotTriggerImport();
    d->detectAllShares = Smb4KMountSettings::detectAllShares();
  }
  else
  {
    // Do nothing
  }
}



