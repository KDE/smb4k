/***************************************************************************
    smb4kmounter.cpp  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2015 by Alexander Reinholdt
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
#include "smb4kglobal.h"
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
#include "smb4kmountsettings_freebsd.h"
#endif

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtWidgets/QApplication>
#include <QtTest/QTest>
#include <QtNetwork/QUdpSocket>

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

  if (!coreIsInitialized())
  {
    setDefaultSettings();
  }
  else
  {
    // Do nothing
  }

  d->timerId = -1;
  d->remountTimeout = 0;
  d->remountAttempts = 0;
  d->checks = 0;
  d->dialog = 0;
  d->firstImportDone = false;
  d->importsAllowed = true;
  d->internalReason = false;
  d->activeProfile = Smb4KProfileManager::self()->activeProfile();

  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
          this, SLOT(slotAboutToQuit()));
  
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkConfigUpdated()), 
          this, SLOT(slotStartJobs()));
  
  connect(Smb4KHardwareInterface::self(), SIGNAL(onlineStateChanged(bool)),
          this, SLOT(slotOnlineStateChanged(bool)));
  
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkShareAdded()),
          this, SLOT(slotTriggerImport()));
  
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkShareRemoved()),
          this, SLOT(slotTriggerImport()));
  
  connect(Smb4KProfileManager::self(), SIGNAL(migratedProfile(QString,QString)),
          this, SLOT(slotProfileMigrated(QString,QString)));
  
  connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)),
          this, SLOT(slotActiveProfileChanged(QString)));
}


Smb4KMounter::~Smb4KMounter()
{
  while (!d->importedShares.isEmpty())
  {
    delete d->importedShares.takeFirst();
  }

  while (!d->retries.isEmpty())
  {
    delete d->retries.takeFirst();
  }
}


Smb4KMounter *Smb4KMounter::self()
{
  return &p->instance;
}


void Smb4KMounter::abort(Smb4KShare *share)
{
  Q_ASSERT(share);
  
  QString unc;

  if (!share->isHomesShare())
  {
    unc = share->unc();
  }
  else
  {
    unc = share->homeUNC();
  }

  QListIterator<KJob *> it(subjobs());
  
  while (it.hasNext())
  {
    KJob *job = it.next();
    
    if (QString::compare(job->objectName(), QString("MountJob_%1").arg(unc), Qt::CaseInsensitive) == 0)
    {
      job->kill(KJob::EmitResult);
      continue;
    }
    else if (QString::compare(job->objectName(), QString("UnmountJob_%1").arg(share->canonicalPath()), Qt::CaseInsensitive) == 0)
    {
      job->kill(KJob::EmitResult);
      continue;
    }
    else
    {
      continue;
    }
  }
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
    // p has already been deleted
  }
}


bool Smb4KMounter::isRunning(Smb4KShare *share)
{
  Q_ASSERT(share);
  
  QString unc;

  if (!share->isHomesShare())
  {
    unc = share->unc();
  }
  else
  {
    unc = share->homeUNC();
  }

  QListIterator<KJob *> it(subjobs());
  
  while (it.hasNext())
  {
    KJob *job = it.next();
    
    if (QString::compare(job->objectName(), QString("MountJob_%1").arg(unc), Qt::CaseInsensitive) == 0)
    {
      return true;
    }
    else if (QString::compare(job->objectName(), QString("UnmountJob_%1").arg(unc), Qt::CaseInsensitive) == 0)
    {
      return true;
    }
    else
    {
      continue;
    }
  }
  
  return false;
}


bool Smb4KMounter::isRunning()
{
  return hasSubjobs();
}


void Smb4KMounter::triggerRemounts(bool fill_list)
{
  if (Smb4KSettings::remountShares() /* one-time remounts */ || 
      !Smb4KCustomOptionsManager::self()->sharesToRemount().isEmpty() /* permanent remounts */ ||
      d->internalReason /* internal reason */)
  {
    if (fill_list)
    {
      // Get the shares that are to be remounted
      QList<Smb4KCustomOptions *> list = Smb4KCustomOptionsManager::self()->sharesToRemount();

      if (!list.isEmpty())
      {
        // Check which ones actually need to be remounted.
        for (int i = 0; i < list.size(); ++i)
        {
          QList<Smb4KShare *> mounted_shares = findShareByUNC(list.at(i)->unc());

          if (!mounted_shares.isEmpty())
          {
            bool mount = true;

            for (int j = 0; j < mounted_shares.size(); ++j)
            {
              if (!mounted_shares.at(j)->isForeign())
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
              Smb4KShare *share = new Smb4KShare();
              share->setURL(list.at(i)->url());
              share->setWorkgroupName(list.at(i)->workgroupName());
              share->setHostIP(list.at(i)->ip());

              if (!share->url().isEmpty())
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
            Smb4KShare *share = new Smb4KShare();
            share->setURL(list.at(i)->url());
            share->setWorkgroupName(list.at(i)->workgroupName());
            share->setHostIP(list.at(i)->ip());

            if (!share->url().isEmpty())
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
    
      // Wait until done.
      while (hasSubjobs())
      {
        QTest::qWait(TIMEOUT);
      }
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


void Smb4KMounter::import(bool check_inaccessible)
{
  // Determine mountpoints.
  KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
  
  for (int i = 0; i < mountPoints.size(); ++i)
  {
    // Check for all supported filesystems.
    if (QString::compare(mountPoints.at(i)->mountType(), "cifs") == 0 ||
        QString::compare(mountPoints.at(i)->mountType(), "smbfs") == 0)
    {
      // Create a new share and set the mountpoint and filesystem
      Smb4KShare *share = new Smb4KShare(mountPoints.at(i)->mountedFrom());
      share->setPath(mountPoints.at(i)->mountPoint());
      share->setIsMounted(true);
      
      if (QString::compare(mountPoints.at(i)->mountType(), "cifs") == 0)
      {
        share->setFileSystem(Smb4KShare::CIFS);
      }
      else if (QString::compare(mountPoints.at(i)->mountType(), "smbfs") == 0)
      {
        share->setFileSystem(Smb4KShare::SMBFS);
      }
      else
      {
        share->setFileSystem(Smb4KShare::Unknown);
      }
      
      // Try to acquire all needed mount options from KMountPoint::mountOptions()
      QStringList mountOptions = mountPoints.at(i)->mountOptions();
      
      for (int j = 0; j < mountOptions.size(); ++j)
      {
        if (mountOptions.at(j).startsWith("domain=") || mountOptions.at(j).startsWith("workgroup="))
        {
          share->setWorkgroupName(mountOptions.at(j).section('=', 1, 1).trimmed());
        }
        else if (mountOptions.at(j).startsWith("addr="))
        {
          share->setHostIP(mountOptions.at(j).section('=', 1, 1).trimmed());
        }
        else if (mountOptions.at(j).startsWith("username=") || mountOptions.at(j).startsWith("user="))
        {
          share->setLogin(mountOptions.at(j).section('=', 1, 1).trimmed());
        }
        else
        {
          // Do nothing
        }
      }
      
      // Work around empty user entries.
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
  
  // Check which shares were unmounted, emit the unmounted() signal
  // on each of the unmounted shares and remove them from the global
  // list.
  // NOTE: The unmount() signal is emitted *BEFORE* the share is removed
  // from the global list! You need to account for that in your application.
  bool found = false;

  for (int i = 0; i < mountedSharesList().size(); ++i)
  {
    for (int j = 0; j < d->importedShares.size(); ++j)
    {
      // Check the mount point, since that is unique. We will
      // only use Smb4KShare::path(), so that we do not run into
      // trouble if a share is inaccessible.
      if (QString::compare(mountedSharesList().at(i)->path(), d->importedShares.at(j)->path()) == 0)
      {
        found = true;
        break;
      }
      else
      {
        continue;
      }
    }

    if (!found)
    {
      mountedSharesList()[i]->setIsMounted(false);
      emit unmounted(mountedSharesList().at(i));
      removeMountedShare(mountedSharesList().at(i));
      emit mountedSharesListChanged();
    }
    else
    {
      // Do nothing
    }

    found = false;
  }
  
  // Now stat the imported shares to get information about them.
  // Do not use Smb4KShare::canonicalPath() here, otherwise we might
  // get lock-ups with inaccessible shares.
  if (Smb4KHardwareInterface::self()->isOnline())
  {
    for (int i = 0; i < d->importedShares.size(); ++i)
    {
      // Check if the share is inaccessible and should be checked.
      Smb4KShare *share = findShareByPath(d->importedShares.at(i)->path());

      if (share)
      {
        if (share->isInaccessible() && !check_inaccessible)
        {
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
      
      QUrl url = QUrl::fromLocalFile(d->importedShares.at(i)->path());
      KIO::StatJob *job = KIO::stat(url, KIO::HideProgressInfo);
      job->setDetails(0);
      connect(job, SIGNAL(result(KJob*)), SLOT(slotStatResult(KJob*)));

      // Do not use addSubJob(), because that would confuse isRunning, etc.
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


void Smb4KMounter::mountShare(Smb4KShare *share, QWidget *parent)
{
  Q_ASSERT(share);
  
  if (share)
  {
    // Check that the URL is valid. Otherwise, we can just return here
    // with an error message.
    if (!share->url().isValid())
    {
      Smb4KNotification::invalidURLPassed();
      return;
    }
    else
    {
      // Do nothing
    }
    
    // Check if the share has already been mounted or a mount
    // is currently in progress.
    QList<Smb4KShare *> mounted_shares;
    QString unc;
    bool mounted = false;

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

      unc = share->homeUNC();
    }
    else
    {
      unc = share->unc();
    }

    mounted_shares = findShareByUNC(unc);

    // Check if it is already mounted:
    for (int i = 0; i != mounted_shares.size(); ++i)
    {
      if (!mounted_shares.at(i)->isForeign())
      {
        mounted = true;
        break;
      }
      else
      {
        continue;
      }
    }
    
    if (!mounted)
    {
      QListIterator<KJob *> it(subjobs());

      while (it.hasNext())
      {
        KJob *job = it.next();

        if (QString::compare(job->objectName(), QString("MountJob_%1").arg(unc), Qt::CaseInsensitive) == 0)
        {
          // Already running
          return;
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      return;
    }
    
    // Check if the host should be woken up before we start mounting the
    // share.
    if (Smb4KSettings::enableWakeOnLAN())
    {
      Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions(KIO::upUrl(share->url()));

      if (options && options->wolSendBeforeMount())
      {
        emit aboutToStart(share, WakeUp);

        QUdpSocket *socket = new QUdpSocket(this);
        QHostAddress addr;

        // Use the host's IP address directly from the share object.
        if (!share->hostIP().isEmpty())
        {
          addr.setAddress(share->hostIP());
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
          sequence.append(QChar(0xFF).toAscii());
        }

        // 16 times the MAC address
        QStringList parts = options->macAddress().split(':', QString::SkipEmptyParts);

        for (int j = 0; j < 16; ++j)
        {
          for (int k = 0; k < parts.size(); ++k)
          {
            sequence.append(QChar(QString("0x%1").arg(parts.at(k)).toInt(0, 16)).toAscii());
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
        
        emit finished(share, WakeUp);
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
    
    // Create the mountpoint.
    QString mountpoint;
    mountpoint += Smb4KSettings::mountPrefix().path();
    mountpoint += QDir::separator();
    mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->hostName().toLower() : share->hostName());
    mountpoint += QDir::separator();

    if (!share->isHomesShare())
    {
      mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->shareName().toLower() : share->shareName());
    }
    else
    {
      mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->login().toLower() : share->login());
    }
    
    // Get the permissions that should be used for creating the
    // mount prefix and all its subdirectories. 
    // Please note that the actual permissions of the mount points
    // are determined by the mount utility.
    QFile::Permissions permissions;
    QUrl parentDirectory;
      
    if (QFile::exists(Smb4KSettings::mountPrefix().path()))
    {
      parentDirectory = Smb4KSettings::mountPrefix();
    }
    else
    {
      QUrl u = Smb4KSettings::mountPrefix();
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
    
    // Get the authentication information
    Smb4KWalletManager::self()->readAuthInfo(share);
    
    // Fill the mount arguments, i. e. set the mount command
    // and all the other parameters we need.
    QVariantMap args;
    
    if (!fillMountActionArgs(share, args))
    {
      return;
    }
    else
    {
      // Do nothing
    }
    
    // Emit the aboutToStart() signal
    emit aboutToStart(share, MountShare);

    // Create the mount action
    KAuth::Action mountAction("net.sourceforge.smb4k.mounthelper.mount");
    mountAction.setHelperId("net.sourceforge.smb4k.mounthelper");
    mountAction.setArguments(args);
    
    KAuth::ExecuteJob *job = mountAction.execute();
    job->setObjectName(QString("MountJob_%1").arg(unc));
    
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotMountJobFinished(KJob*)));
    
    // Modify the cursor, if necessary.
    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }
    
    // Add the job
    addSubjob(job);
    
    // Start the job
    job->start();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::mountShares(const QList<Smb4KShare *> &shares, QWidget *parent)
{
  for (int i = 0; i < shares.size(); ++i)
  {
    mountShare(shares[i], parent);
  }
}


void Smb4KMounter::unmountShare(Smb4KShare *share, bool silent, QWidget *parent)
{
  Q_ASSERT(share);
  
  if (share)
  {
    // Check that the URL is valid. Otherwise, we can just return here
    // with an error message.
    if (!share->url().isValid())
    {
      Smb4KNotification::invalidURLPassed();
      return;
    }
    else
    {
      // Do nothing
    }
    
    // Check if the unmount process is already in progress.
    QListIterator<KJob *> it(subjobs());
      
    while (it.hasNext())
    {
      KJob *job = it.next();
        
      if (QString::compare(job->objectName(), QString("UnmountJob_%1").arg(share->canonicalPath()), Qt::CaseInsensitive) == 0)
      {
        // Already running
        return;
      }
      else
      {
        // Do nothing
      }
    }
    
    // Complain if the share is a foreign one and unmounting those
    // is prohibited.
    if (share->isForeign())
    {
      if (!Smb4KSettings::unmountForeignShares())
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
                  share->unc(), share->path(), share->user().loginName()),
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
    
    // Forcibly unmount an inaccessible share if the user set
    // the appropriate setting (Linux only).
    bool force = false;

#if defined(Q_OS_LINUX)
    if (share->isInaccessible())
    {
      force = Smb4KSettings::forceUnmountInaccessible();
    }
    else
    {
      // Do nothing
    }
#endif

    // Fill the mount arguments, i. e. set the mount command
    // and all the other parameters we need.
    QVariantMap args;

    if (!fillUnmountActionArgs(share, force, silent, args))
    {
      return;
    }
    else
    {
      // Do nothing
    }
    
    // Emit the aboutToStart() signal
    emit aboutToStart(share, UnmountShare);

    // Create the mount action
    KAuth::Action unmountAction("net.sourceforge.smb4k.mounthelper.unmount");
    unmountAction.setHelperId("net.sourceforge.smb4k.mounthelper");
    unmountAction.setArguments(args);
    
    KAuth::ExecuteJob *job = unmountAction.execute();
    job->setObjectName(QString("UnmountJob_%1").arg(share->canonicalPath()));
    
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotUnmountJobFinished(KJob*)));
    
    // Modify the cursor, if necessary.
    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }
    
    // Add the job
    addSubjob(job);
    
    // Start the job
    job->start();    
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::unmountShares(const QList<Smb4KShare *> &shares, bool silent, QWidget *parent)
{
  for (int i = 0; i < shares.size(); ++i)
  {
    unmountShare(shares[i], silent, parent);
  }
}


void Smb4KMounter::unmountAllShares(QWidget* parent)
{
  if (!d->internalReason)
  {
    unmountShares(mountedSharesList(), false, parent);
  }
  else
  {
    unmountShares(mountedSharesList(), true, parent);
  }
}


void Smb4KMounter::openMountDialog(QWidget *parent)
{
  if (!d->dialog)
  {
    Smb4KShare *share = new Smb4KShare();
    
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

    delete share;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::start()
{
  // Check the network configurations
  Smb4KHardwareInterface::self()->updateNetworkConfig();
}


void Smb4KMounter::check(Smb4KShare *share)
{
  // Get the info about the usage, etc.
  KDiskFreeSpaceInfo space_info = KDiskFreeSpaceInfo::freeSpaceInfo(share->path());
  
  if (space_info.isValid())
  {
    share->setInaccessible(false);
    share->setFreeDiskSpace(space_info.available());
    share->setTotalDiskSpace(space_info.size());
    share->setUsedDiskSpace(space_info.used());
      
    // Get the owner an group, if possible.
    QFileInfo file_info(share->path());
    file_info.setCaching(false);

    if (file_info.exists())
    {
      share->setUser(KUser(static_cast<K_UID>(file_info.ownerId())));
      share->setGroup(KUserGroup(static_cast<K_GID>(file_info.groupId())));
      share->setInaccessible(!(file_info.isDir() && file_info.isExecutable()));
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


void Smb4KMounter::saveSharesForRemount()
{
  // Save the shares for remount.
  for (int i = 0; i < mountedSharesList().size(); ++i)
  {
    if (!mountedSharesList().at(i)->isForeign())
    {
      Smb4KCustomOptionsManager::self()->addRemount(mountedSharesList().at(i), false);
    }
    else
    {
      Smb4KCustomOptionsManager::self()->removeRemount(mountedSharesList().at(i),false);
    }
  }
  
  // Also save the failed remounts.
  for (int i = 0; i < d->remounts.size(); ++i)
  {
    Smb4KCustomOptionsManager::self()->addRemount(d->remounts.at(i),false);
  }
  
  // Clear the failed remounts list.
  while (!d->remounts.isEmpty())
  {
    delete d->remounts.takeFirst();
  }  
}


void Smb4KMounter::timerEvent(QTimerEvent *)
{
  // Try to remount those shares that could not be mounted 
  // before. Do this only if there are no subjobs, because we
  // do not want to get crashes because a share was invalidated
  // during processing the shares.
  if ((Smb4KSettings::remountShares() || !Smb4KCustomOptionsManager::self()->sharesToRemount().isEmpty()) && 
       Smb4KSettings::remountAttempts() > d->remountAttempts)
  {
    // Inhibit automatic sleeping.
    int cookie = Smb4KHardwareInterface::self()->beginSuppressingSleep(i18n("Remounting shares. Please wait."));
    
    if (d->firstImportDone && !hasSubjobs())
    {
      if (d->remountAttempts == 0)
      {
        triggerRemounts(true);
      }
      else if (!d->remounts.isEmpty() &&
                d->remountTimeout >= (60000 * Smb4KSettings::remountInterval()))
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
    
    // Allow automatic sleeping.
    Smb4KHardwareInterface::self()->endSuppressingSleep(cookie);
  }
  else
  {
    // Do nothing
  }
  
  // Retry mounting those shares that failed. This is also only 
  // done when there are no subjobs.
  if (!d->retries.isEmpty() && !hasSubjobs())
  {
    // Inhibit automatic sleeping.
    int cookie = Smb4KHardwareInterface::self()->beginSuppressingSleep(i18n("Remounting shares. Please wait."));
    
    mountShares(d->retries);
      
    while (!d->retries.isEmpty())
    {
      delete d->retries.takeFirst();
    }
    
    // Allow automatic sleeping.
    Smb4KHardwareInterface::self()->endSuppressingSleep(cookie);
  }
  else
  {
    // Do nothing
  }
}


#if defined(Q_OS_LINUX)
//
// Linux arguments
//
bool Smb4KMounter::fillMountActionArgs(Smb4KShare *share, QVariantMap& map)
{
  // Find the mount program.
  QString mount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  mount = QStandardPaths::findExecutable("mount.cifs", paths);

  if (!mount.isEmpty())
  {
    map.insert("mh_command", mount);
  }
  else
  {
    Smb4KNotification::commandNotFound("mount.cifs");
    return false;
  }

  // Mount arguments.
  QMap<QString, QString> global_options = globalSambaOptions();
  Smb4KCustomOptions *options  = Smb4KCustomOptionsManager::self()->findOptions(share);

  // Set some settings for the share.
  share->setFileSystem(Smb4KShare::CIFS);
  
  if (options)
  {
    share->setPort(options->fileSystemPort() != Smb4KMountSettings::remoteFileSystemPort() ?
                   options->fileSystemPort() : Smb4KMountSettings::remoteFileSystemPort());
  }
  else
  {
    share->setPort(Smb4KMountSettings::remoteFileSystemPort());
  }
  
  // Compile the list of arguments, that is added to the
  // mount command via "-o ...".
  QStringList args_list;
  
  // Workgroup or domain
  if (!share->workgroupName().trimmed().isEmpty())
  {
    args_list << QString("domain=%1").arg(KShell::quoteArg(share->workgroupName()));
  }
  else
  {
    // Do nothing
  }
  
  // Host IP
  if (!share->hostIP().trimmed().isEmpty())
  {
    args_list << QString("ip=%1").arg(share->hostIP());
  }
  else
  {
    // Do nothing
  }
  
  // User name
  if (!share->login().isEmpty())
  {
    args_list << QString("username=%1").arg(share->login());
  }
  else
  {
    args_list << "guest";
  }
  
  // Client's and server's NetBIOS name
  // According to the manual page, this is only needed when port 139
  // is used. So, we only pass the NetBIOS name in that case.
  if (Smb4KMountSettings::remoteFileSystemPort() == 139 || (options && options->fileSystemPort() == 139))
  {
    // The client's NetBIOS name.
    if (!Smb4KSettings::netBIOSName().isEmpty())
    {
      args_list << QString("netbiosname=%1").arg(KShell::quoteArg(Smb4KSettings::netBIOSName()));
    }
    else
    {
      if (!global_options["netbios name"].isEmpty())
      {
        args_list << QString("netbiosname=%1").arg(KShell::quoteArg(global_options["netbios name"]));
      }
      else
      {
        // Do nothing
      }
    }

    // The server's NetBIOS name.
    // ('servern' is a synonym for 'servernetbiosname')
    args_list << QString("servern=%1").arg(KShell::quoteArg(share->hostName()));
  }
  else
  {
    // Do nothing
  }
  
  // UID
  args_list << QString("uid=%1").arg(options ? options->user().userId().nativeId() : (K_UID)Smb4KMountSettings::userID().toInt());
  
  // Force user
  if (Smb4KMountSettings::forceUID())
  {
    args_list << "forceuid";
  }
  else
  {
    // Do nothing
  }
  
  // GID
  args_list << QString("gid=%1").arg(options ? options->group().groupId().currentGroupId().nativeId() : (K_GID)Smb4KMountSettings::groupID().toInt());
  
  // Force GID
  if (Smb4KMountSettings::forceGID())
  {
    args_list << "forcegid";
  }
  else
  {
    // Do nothing
  }
  
  // Client character set
  switch (Smb4KMountSettings::clientCharset())
  {
    case Smb4KMountSettings::EnumClientCharset::default_charset:
    {
      if (!global_options["unix charset"].isEmpty())
      {
        args_list << QString("iocharset=%1").arg(global_options["unix charset"].toLower());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      args_list << QString("iocharset=%1")
                   .arg(Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label);
      break;
    }
  }
  
  // Port. 
  args_list << QString("port=%1").arg(share->port());
  
  // Write access
  if (options)
  {
    switch (options->writeAccess())
    {
      case Smb4KCustomOptions::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KCustomOptions::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        switch (Smb4KMountSettings::writeAccess())
        {
          case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
          {
            args_list << "rw";
            break;
          }
          case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
          {
            args_list << "ro";
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
    }
  }
  else
  {
    switch (Smb4KMountSettings::writeAccess())
    {
      case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  // File mask
  if (!Smb4KMountSettings::fileMask().isEmpty())
  {
    args_list << QString("file_mode=%1").arg(Smb4KMountSettings::fileMask());
  }
  else
  {
    // Do nothing
  }

  // Directory mask
  if (!Smb4KMountSettings::directoryMask().isEmpty())
  {
    args_list << QString("dir_mode=%1").arg(Smb4KMountSettings::directoryMask());
  }
  else
  {
    // Do nothing
  }
  
  // Permission checks
  if (Smb4KMountSettings::permissionChecks())
  {
    args_list << "perm";
  }
  else
  {
    args_list << "noperm";
  }
  
  // Client controls IDs
  if (Smb4KMountSettings::clientControlsIDs())
  {
    args_list << "setuids";
  }
  else
  {
    args_list << "nosetuids";
  }
  
  // Server inode numbers
  if (Smb4KMountSettings::serverInodeNumbers())
  {
    args_list << "serverino";
  }
  else
  {
    args_list << "noserverino";
  }
  
  // Cache mode
  switch (Smb4KMountSettings::cacheMode())
  {
    case Smb4KMountSettings::EnumCacheMode::None:
    {
      args_list << "cache=none";
      break;
    }
    case Smb4KMountSettings::EnumCacheMode::Strict:
    {
      args_list << "cache=strict";
      break;
    }
    case Smb4KMountSettings::EnumCacheMode::Loose:
    {
      args_list << "cache=loose";
      break;
    }
    default:
    {
      break;
    }
  }
  
  // Translate reserved characters
  if (Smb4KMountSettings::translateReservedChars())
  {
    args_list << "mapchars";
  }
  else
  {
    args_list << "nomapchars";
  }
  
  // Locking
  if (Smb4KMountSettings::noLocking())
  {
    args_list << "nolock";
  }
  else
  {
    // Do nothing
  }
  
  // Security mode
  if (options)
  {
    switch (options->securityMode())
    {
      case Smb4KCustomOptions::NoSecurityMode:
      {
        args_list << "sec=none";
        break;
      }
      case Smb4KCustomOptions::Krb5:
      {
        args_list << "sec=krb5";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
        break;
      }
      case Smb4KCustomOptions::Krb5i:
      {
        args_list << "sec=krb5i";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
        break;
      }
      case Smb4KCustomOptions::Ntlm:
      {
        args_list << "sec=ntlm";
        break;
      }
      case Smb4KCustomOptions::Ntlmi:
      {
        args_list << "sec=ntlmi";
        break;
      }
      case Smb4KCustomOptions::Ntlmv2:
      {
        args_list << "sec=ntlmv2";
        break;
      }
      case Smb4KCustomOptions::Ntlmv2i:
      {
        args_list << "sec=ntlmv2i";
        break;
      }
      case Smb4KCustomOptions::Ntlmssp:
      {
        args_list << "sec=ntlmssp";
        break;
      }
      case Smb4KCustomOptions::Ntlmsspi:
      {
        args_list << "sec=ntlmsspi";
        break;
      }
      default:
      {
        // Smb4KCustomOptions::DefaultSecurityMode
        break;
      }
    }
  }
  else
  {
    switch (Smb4KMountSettings::securityMode())
    {
      case Smb4KMountSettings::EnumSecurityMode::None:
      {
        args_list << "sec=none";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5:
      {
        args_list << "sec=krb5";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5i:
      {
        args_list << "sec=krb5i";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).userId().nativeId());
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlm:
      {
        args_list << "sec=ntlm";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
      {
        args_list << "sec=ntlmi";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
      {
        args_list << "sec=ntlmv2";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
      {
        args_list << "sec=ntlmv2i";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
      {
        args_list << "sec=ntlmssp";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
      {
        args_list << "sec=ntlmsspi";
        break;
      }
      default:
      {
        // Smb4KSettings::EnumSecurityMode::Default,
        break;
      }
    }
  }
  
  // SMB protocol version
  switch (Smb4KMountSettings::smbProtocolVersion())
  {
    case Smb4KMountSettings::EnumSmbProtocolVersion::OnePointZero:
    {
      args_list << "vers=1.0";
      break;
    }
    case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointZero:
    {
      args_list << "vers=2.0";
      break;
    }
    case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointOne:
    {
      args_list << "vers=2.1";
      break;
    }
    case Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointZero:
    {
      args_list << "vers=3.0";
      break;
    }
    default:
    {
      break;
    }
  }
    
  // Global custom options provided by the user
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
      
      args_list += list;
    }
  }
  else
  {
    // Do nothing
  }
  
  // Mount options
  QStringList mh_options;
  mh_options << "-o";
  mh_options << args_list.join(",");
  map.insert("mh_options", mh_options);
  
  // Mount point
  map.insert("mh_mountpoint", share->canonicalPath());
  
  if (!share->isHomesShare())
  {
    map.insert("mh_url", share->url());
    map.insert("mh_unc", share->unc());
  }
  else
  {
    map.insert("mh_url", share->homeURL());
    map.insert("mh_homes_url", share->url());
    map.insert("mh_unc", share->homeUNC());
    map.insert("mh_homes_unc", share->unc());
  }  

  map.insert("mh_workgroup", share->workgroupName());
  map.insert("mh_ip", share->hostIP());

  // The path to the Kerberos ticket is stored - if it exists - in the
  // KRB5CCNAME environment variable. By default, the ticket is located
  // at /tmp/krb5cc_[uid]. So, if the environment variable does not exist,
  // but the cache file is there, try to use it.
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
void Smb4KMounter::fillMountActionArgs(Smb4KShare *share, QVariantMap& map)
{
  // Find the mount program.
  QString mount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  mount = QStandardPaths::findExecutable("mount_smbfs", paths);

  if (!mount.isEmpty())
  {
    map.insert("mh_command", mount);
  }
  else
  {
    Smb4KNotification::commandNotFound("mount_smbfs");
    return false;
  }
  
  // Mount arguments.
  QMap<QString, QString> global_options = globalSambaOptions();
  Smb4KCustomOptions *options  = Smb4KCustomOptionsManager::self()->findOptions(share);

  // Set some settings for the share.
  share->setFileSystem(Smb4KShare::SMBFS);
  
  // Compile the list of arguments.
  QStringList args_list;
  
  // Workgroup
  if (!share->workgroupName().isEmpty())
  {
    args_list << "-W";
    args_list << KShell::quoteArg(share->workgroupName());
  }
  else
  {
    // Do nothing
  }
  
  // Host IP
  if (!share->hostIP().isEmpty())
  {
    args_list << "-I";
    args_list << share->hostIP();
  }
  else
  {
    // Do nothing
  }
  
  // UID
  if (options)
  {
    args_list << "-u";
    args_list << QString("%1").arg(options->uid());
  }
  else
  {
    args_list << "-u";
    args_list << QString("%1").arg((K_UID)Smb4KMountSettings::userID().toInt());
  }
  
  // GID
  if (options)
  {
    args_list << "-g";
    args_list << QString("%1").arg(options->gid());
  }
  else
  {
    args_list << "-g";
    args_list << QString("%1").arg((K_GID)Smb4KMountSettings::groupID().toInt());
  }
  
  // Character sets for the client and server
  QString client_charset, server_charset;

  switch (Smb4KMountSettings::clientCharset())
  {
    case Smb4KMountSettings::EnumClientCharset::default_charset:
    {
      client_charset = global_options["unix charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      client_charset = Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label;
      break;
    }
  }

  switch (Smb4KMountSettings::serverCodepage())
  {
    case Smb4KMountSettings::EnumServerCodepage::default_codepage:
    {
      server_charset = global_options["dos charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      server_charset = Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::serverCodepage()).label;
      break;
    }
  }

  if (!client_charset.isEmpty() && !server_charset.isEmpty())
  {
    args_list << "-E";
    args_list << QString("%1:%2").arg(client_charset, server_charset);
  }
  else
  {
    // Do nothing
  }
  
  // File mask
  if (!Smb4KMountSettings::fileMask().isEmpty())
  {
    args_list << "-f";
    args_list << QString("%1").arg(Smb4KMountSettings::fileMask());
  }
  else
  {
    // Do nothing
  }

  // Directory mask
  if (!Smb4KMountSettings::directoryMask().isEmpty())
  {
    args_list << "-d";
    args_list << QString("%1").arg(Smb4KMountSettings::directoryMask());
  }
  else
  {
    // Do nothing
  }
  
  // User name
  if (!share->login().isEmpty())
  {
    args_list << "-U";
    args_list << QString("%1").arg(share->login());
  }
  else
  {
    args_list << "-N";
  }
  
  // Mount options
  map.insert("mh_options", args_list);
  
  // Mount point
  map.insert("mh_mountpoint", share->canonicalPath());
  
  if (!share->isHomesShare())
  {
    map.insert("mh_url", share->url());
    map.insert("mh_unc", share->unc());
  }
  else
  {
    map.insert("mh_url", share->homeURL());
    map.insert("mh_homes_url", share->url());
    map.insert("mh_unc", share->homeUNC());
    map.insert("mh_homes_unc", share->unc());
  }  

  map.insert("mh_workgroup", share->workgroupName());
  map.insert("mh_ip", share->hostIP());
}
#else
//
// Dummy 
//
void Smb4KMounter::fillMountActionArgs(Smb4KShare *, QVariantMap&)
{
  qWarning() << "Smb4KMounter::fillMountActionArgs() is not implemented!";
  qWarning() << "Mounting under this operating system is not supported...";
}
#endif


#if defined(Q_OS_LINUX)
//
// Linux arguments
//
bool Smb4KMounter::fillUnmountActionArgs(Smb4KShare *share, bool force, bool silent, QVariantMap &map)
{
  // Find the umount program.
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  QString umount = QStandardPaths::findExecutable("umount", paths);

  if (umount.isEmpty() && !silent)
  {
    Smb4KNotification::commandNotFound("umount");
    return false;
  }
  else
  {
    // Do nothing
  }

  QStringList options;

  if (force)
  {
    options << "-l"; // lazy unmount
  }
  else
  {
    // Do nothing
  }

  map.insert("mh_command", umount);
  map.insert("mh_url", share->url());
  map.insert("mh_unc", share->unc());
  map.insert("mh_mountpoint", share->canonicalPath());
  map.insert("mh_options", options);
  
  return true;
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD arguments
//
bool Smb4KMounter::fillUnmountActionArgs(Smb4KShare *share, bool /*force*/, bool silent, QVariantMap &map)
{
  // Find the umount program.
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  QString umount = QStandardPaths::findExecutable("umount", paths);

  if (umount.isEmpty() && !silent)
  {
    Smb4KNotification::commandNotFound("umount");
    return false;
  }
  else
  {
    // Do nothing
  }

  QStringList options;

  map.insert("mh_command", umount);
  map.insert("mh_url", share->url());
  map.insert("mh_unc", share->unc());
  map.insert("mh_mountpoint", share->canonicalPath());
  map.insert("mh_options", options);
  
  return true;
}
#else
//
// Dummy
//
bool Smb4KMounter::fillUnmountActionArgs(Smb4KShare *, bool, bool, QVariantMap &)
{
  qWarning() << "Smb4KMounter::fillUnmountActionArgs() is not implemented!";
  qWarning() << "Unmounting under this operating system is not supported...";
  return false;
}
#endif



/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KMounter::slotStartJobs()
{
  // Import the mounted shares
  import(true);
  
  if (d->timerId == -1)
  {
    // Start the timer
    d->timerId = startTimer(TIMEOUT);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotAboutToQuit()
{
  // Due to an internal reason some special things should
  // be done, like unmounting synchronously.
  d->internalReason = true;
  
  // Abort any actions.
  abortAll();

  // Check if the user wants to remount shares and save the
  // shares for remount if so.
  if (Smb4KSettings::remountShares())
  {
    saveSharesForRemount();
  }
  else
  {
    // Do nothing
  }

  // Unmount the shares if the user chose to do so.
  if (Smb4KSettings::unmountSharesOnExit())
  {
    unmountAllShares();
    
    // Wait until done.
    while (hasSubjobs())
    {
      QTest::qWait(TIMEOUT);
    }
  }
  else
  {
    // Do nothing
  }

  // Clean up the mount prefix.
  QDir dir;
  QStringList mountpoints;
  dir.cd(Smb4KSettings::mountPrefix().path());
  QStringList dirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort);

  QList<Smb4KShare *> inaccessible = findInaccessibleShares();

  // Remove all directories from the list that belong to
  // inaccessible shares.
  for (int i = 0; i < inaccessible.size(); ++i)
  {
    int index = dirs.indexOf(inaccessible.at(i)->hostName(), 0);

    if (index != -1)
    {
      dirs.removeAt(index);
      continue;
    }
    else
    {
      dir.cd(dirs.at(i));
      mountpoints += dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort);
      continue;
    }
  }
  
  // Unset internal reason
  d->internalReason = false;
}


void Smb4KMounter::slotMountJobFinished(KJob *job)
{
  // Do not allow imports. We do not want to operate on a 
  // network item that might get invalidated between imports.
  d->importsAllowed = false;
  
  // We need the execute job
  KAuth::ExecuteJob *j = static_cast<KAuth::ExecuteJob *>(job);
  
  // Get the error code
  int errorCode = j->error();
  
  // Get the workgroup, mountpoint and UNC of the share.
  QString workgroup = j->action().arguments()["mh_workgroup"].toString();
  QString mountpoint = j->action().arguments()["mh_mountpoint"].toString();
  QString unc;
  
  if (j->action().arguments().contains("mh_homes_unc"))
  {
    unc = j->action().arguments()["mh_homes_unc"].toString();
  }
  else
  {
    unc = j->action().arguments()["mh_unc"].toString();
  }
  
  // Get the returned data
  QVariantMap returnedData = j->data();
  
  // Remove the job
  removeSubjob(job);
  
  // Find the share in the global network share list
  Smb4KShare *networkShare = findNetworkShare(unc, workgroup);
  
  // Evaluate the answer of the mount job.
  if (networkShare)
  {
    if (errorCode == 0)
    {
      QString errorMsg = returnedData["mh_error_message"].toString();

      if (errorMsg.isEmpty())
      {
        // Copy the share for later use
        Smb4KShare *mountedShare = new Smb4KShare(*networkShare);
        
        // Check if the share has been mounted and set the mountpoint
        KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);

        for (int i = 0; i < mountPoints.size(); ++i)
        {
          if (QString::compare(mountPoints.at(i)->mountPoint(), mountpoint) == 0)
          {
            mountedShare->setPath(mountpoint);
            mountedShare->setIsMounted(true);
            
            if (QString::compare(mountPoints.at(i)->mountType(), "cifs") == 0)
            {
              mountedShare->setFileSystem(Smb4KShare::CIFS);
            }
            else if (QString::compare(mountPoints.at(i)->mountType(), "smbfs") == 0)
            {
              mountedShare->setFileSystem(Smb4KShare::SMBFS);
            }
            else
            {
              mountedShare->setFileSystem(Smb4KShare::Unknown);
            }
            break;
          }
          else
          {
            continue;
          }
        }        
        
        // Remove the share from the list of shares that are to be remounted.
        QMutableListIterator<Smb4KShare *> s(d->remounts);

        while (s.hasNext())
        {
          Smb4KShare *remount = s.next();

          if (QString::compare(remount->unc(), mountedShare->unc(), Qt::CaseInsensitive) == 0)
          {
            s.remove();
            break;
          }
          else
          {
            continue;
          }
        }
        
        // Check the usage, etc.
        check(mountedShare);

        // Add the share
        addMountedShare(mountedShare);

        if (Smb4KSettings::remountShares())
        {
          Smb4KCustomOptionsManager::self()->removeRemount(mountedShare, false);
        }
        else
        {
          // Do nothing
        }

        // Emit the mounted() and mountedSharesListChanged() signals.
        emit mounted(mountedShare);
        emit mountedSharesListChanged();
          
        // Notify the user
        Smb4KNotification::shareMounted(mountedShare);
      }
      else
      {
#if defined(Q_OS_LINUX)
        if (errorMsg.contains("mount error 13") || errorMsg.contains("mount error(13)") /* authentication error */)
        {
          if (Smb4KWalletManager::self()->showPasswordDialog(networkShare, 0))
          {
            d->retries << new Smb4KShare(*networkShare);
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
          Smb4KNotification::mountingFailed(networkShare, errorMsg);
        }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
        if (errorMsg.contains("Authentication error") || errorMsg.contains("Permission denied"))
        {
          if (Smb4KWalletManager::self()->showPasswordDialog(share, 0))
          {
            d->retries << new Smb4KShare(*networkShare);
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          Smb4KNotification::mountingFailed(networkShare, errorMsg);
        }
#else
        qWarning() << "Smb4KMounter::slotMountJobFinished(): Error handling not implemented!";
        Smb4KNotification::mountingFailed(share, errorMsg);
#endif
      }
    }
    else
    {
      Smb4KNotification::actionFailed();
    }
    
    emit finished(networkShare, MountShare);
  }
  else
  {
    // Do nothing
  }
  
  // Reset the busy cursor
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Do nothing
  }
  
  // Allow imports again
  d->importsAllowed = true;  
}


void Smb4KMounter::slotUnmountJobFinished(KJob* job)
{
  // Do not allow imports. We do not want to operate on a 
  // network item that might get invalidated between imports.
  d->importsAllowed = false;
  
  KAuth::ExecuteJob *j = static_cast<KAuth::ExecuteJob *>(job);
  
  // Get the error code
  int errorCode = j->error();
  
  // Get the mountpoint
  QString mountpoint = j->action().arguments()["mh_mountpoint"].toString();
  
  // Get the returned data
  QVariantMap returnedData = j->data();
  
  // Remove the job
  removeSubjob(job);
  
  // Get the share from the global list of shares.
  Smb4KShare *share = findShareByPath(mountpoint);
  Smb4KShare notify_share = *share;
  
  if (share)
  {
    if (errorCode == 0)
    {
      QString errorMsg = returnedData["mh_error_message"].toString();
      
      if (errorMsg.isEmpty())
      {
        // Check if the share has been unmounted, emit the unmounted()
        // signal, notify the user and remove the mounpoint if appropriate.
        if (share->isMounted())
        {
          KMountPoint::List mount_points = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
          bool mountpoint_found = false;

          for (int i = 0; i < mount_points.size(); ++i)
          {
            if (QString::compare(mount_points.at(i)->mountPoint(), share->path()) == 0 ||
                QString::compare(mount_points.at(i)->mountPoint(), share->canonicalPath()) == 0)
            {
              mountpoint_found = true;
              break;
            }
            else
            {
              continue;
            }
          }

          if (!mountpoint_found)
          {
            // Set the unmounted.
            share->setIsMounted(false);
            
            // Emit the unmounted() signal
            emit unmounted(share);
            
            // Notify the user
            Smb4KNotification::shareUnmounted(share);
            
            // Remove the share from the list of mounted shares.
            removeMountedShare(share);
            
            // Emit the mountedSharesListChanged() signal.
            emit mountedSharesListChanged();
            
            // Remove the mountpoint
            QDir dir(share->canonicalPath());
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
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        Smb4KNotification::unmountingFailed(share, errorMsg);
      }
    }
    else
    {
      Smb4KNotification::actionFailed();
    }
    
    emit finished(&notify_share, UnmountShare);
  }
  else
  {
    // Do nothing
  }
  
  // Reset the busy cursor
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Do nothing
  }
  
  // Allow imports again
  d->importsAllowed = true;
}


void Smb4KMounter::slotOnlineStateChanged(bool online)
{
  // Suppress automatic sleeping
  int cookie = Smb4KHardwareInterface::self()->beginSuppressingSleep(i18n("Unmounting shares. Please wait."));
  
  d->internalReason = true;
  
  if (online)
  {
    // Remount shares after the network became available (again)
    triggerRemounts(true);
  }
  else
  {
    // Abort all running mount jobs
    abortAll();
    
    // Save the shares for automatic remounting.
    saveSharesForRemount();
    
    // Unmount all shares
    unmountAllShares();

    // Wait until done
    while (hasSubjobs())
    {
      QTest::qWait(TIMEOUT);
    }
  }
  
  d->internalReason = false;
  
  // End suppression of automatic sleeping
  Smb4KHardwareInterface::self()->endSuppressingSleep(cookie);
}


void Smb4KMounter::slotStatResult(KJob *job)
{
  Q_ASSERT(job);
  
  KIO::StatJob *stat = static_cast<KIO::StatJob *>(job);
  QString path = stat->url().toDisplayString(QUrl::PreferLocalFile);
  
  Smb4KShare *share = 0;
  
  for (int i = 0; i < d->importedShares.size(); ++i)
  {
    if (QString::compare(d->importedShares.at(i)->path(), path) == 0)
    {
      share = d->importedShares.takeAt(i);
      break;
    }
    else
    {
      continue;
    }
  }

  if (share)
  {
    if (stat->error() == 0 /* no error */)
    {
      // We do not use KIO::StatJob::statResult(), because its
      // information is limited.
      check(share);
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

    // Is this a mount that was done by the user or by
    // someone else (or the system)?
    if ((share->user().userId() == KUser(KUser::UseRealUserID).userId() && 
         share->group().groupId() == KUserGroup(KUser::UseRealUserID).groupId()) ||
        (share->path().startsWith(Smb4KSettings::mountPrefix().path()) || share->path().startsWith(QDir::homePath())) ||
        (share->canonicalPath().startsWith(QDir(Smb4KSettings::mountPrefix().path()).canonicalPath()) ||
         share->canonicalPath().startsWith(QDir::home().canonicalPath())))
    {
      share->setForeign(false);
    }
    else
    {
      share->setForeign(true);
    }

    // Copy data from the mounted share to the newly discovered
    // one. We can use Smb4KShare::canonicalPath() here, because
    // Smb4KShare::isInaccessibe() has already been set.
    Smb4KShare *mounted_share = findShareByPath(share->canonicalPath());

    if (mounted_share)
    {
      if (!mounted_share->login().isEmpty() && QString::compare(mounted_share->login(), share->login()) != 0)
      {
        share->setLogin(mounted_share->login());
      }
      else
      {
        // Do nothing
      }

      if (!mounted_share->workgroupName().isEmpty() && !mounted_share->hostIP().isEmpty())
      {
        if (share->workgroupName().isEmpty())
        {
          share->setWorkgroupName(mounted_share->workgroupName());
        }
        else
        {
          // Do nothing
        }

        if (share->hostIP().isEmpty())
        {
          share->setHostIP(mounted_share->hostIP());
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Get the host that shares this resource and check if we
        // need to set the IP address or workgroup/domain.
        Smb4KHost *host = findHost(share->hostName(), share->workgroupName());

        if (host)
        {
          // Set the IP address if necessary.
          if (share->hostIP().isEmpty() || QString::compare(host->ip(), share->hostIP()) != 0)
          {
            share->setHostIP(host->ip());
          }
          else
          {
            // Do nothing
          }

          // Set the workgroup/domain name if necessary.
          if (share->workgroupName().isEmpty())
          {
            share->setWorkgroupName(host->workgroupName());
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          if (!mounted_share->hostIP().isEmpty() && share->hostIP().isEmpty())
          {
            share->setHostIP(mounted_share->hostIP());
          }
          else
          {
            // Do nothing
          }

          if (!mounted_share->workgroupName().isEmpty() && share->workgroupName().isEmpty())
          {
            share->setWorkgroupName(mounted_share->workgroupName());
          }
          else
          {
            // Do nothing
          }
        }
      }

      // Now remove the obsolete share entry from the global list
      // of shares and add the stat'ed one. Emit the appropriate
      // signal when done.
      if (!share->isForeign() || Smb4KSettings::detectAllShares())
      {
        // This share was previouly mounted.
        removeMountedShare(mounted_share);

        Smb4KShare *new_share = new Smb4KShare(*share);
        addMountedShare(new_share);
        emit updated(new_share);
      }
      else
      {
        mounted_share->setIsMounted(false);
        emit unmounted(mounted_share);
        removeMountedShare(mounted_share);
        emit mountedSharesListChanged();
      }
    }
    else
    {
      // Get the host that shares this resource and check if we
      // need to set the IP address or workgroup/domain.
      Smb4KHost *host = findHost(share->hostName(), share->workgroupName());

      if (host)
      {
        // Set the IP address if necessary.
        if (share->hostIP().isEmpty() || QString::compare(host->ip(), share->hostIP()) != 0)
        {
          share->setHostIP(host->ip());
        }
        else
        {
          // Do nothing
        }

        // Set the workgroup/domain name if necessary.
        if (share->workgroupName().isEmpty())
        {
          share->setWorkgroupName(host->workgroupName());
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

      // Now add the stat'ed share to the global list of shares.
      // Emit the appropriate signal when done.
      if (!share->isForeign() || Smb4KSettings::detectAllShares())
      {
        // This is a new share.
        Smb4KShare *new_share = new Smb4KShare(*share);
        addMountedShare(new_share);
        emit mounted(new_share);
        emit mountedSharesListChanged();
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }

  delete share;

  if (!d->firstImportDone && d->importedShares.isEmpty())
  {
    d->firstImportDone = true;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotActiveProfileChanged(const QString &newProfile)
{
  if (QString::compare(d->activeProfile, newProfile) != 0)
  {
    // Stop the timer.
    killTimer(d->timerId);

    // Check if the user wants to remount shares and save the
    // shares for remount if so.
    if (Smb4KSettings::remountShares())
    {
      saveSharesForRemount();
    }
    else
    {
      // Do nothing
    }
    
    abortAll();
    
    // Clear all remounts.
    while (!d->remounts.isEmpty())
    {
      delete d->remounts.takeFirst();
    }
    
    // Clear all retries.
    while (!d->retries.isEmpty())
    {
      delete d->retries.takeFirst();
    }
    
    // Unmount all shares and wait until done.
    unmountAllShares();
    
    while (hasSubjobs())
    {
      QTest::qWait(TIMEOUT);
    }
    
    // Reset some variables.
    d->remountTimeout = 0;
    d->remountAttempts = 0;
    d->checks = 0;
    d->firstImportDone = false;
    d->importsAllowed = true;
    d->activeProfile = newProfile;
    
    // Start the timer again.
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
  while(isRunning() || !d->importsAllowed)
  {
    QTest::qWait(TIMEOUT);
  }
  
  // Initialize an import
  import(true);
}

