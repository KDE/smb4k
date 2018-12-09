/***************************************************************************
    This is the global namespace for Smb4K.
                             -------------------
    begin                : Sa Apr 2 2005
    copyright            : (C) 2005-2017 by Alexander Reinholdt
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
#include "smb4kglobal.h"
#include "smb4kglobal_p.h"
#include "smb4knotification.h"
#include "smb4kmounter.h"
#include "smb4ksynchronizer.h"
#include "smb4ksearch.h"
#include "smb4kclient.h"

// Qt includes
#include <QMutex>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>

// KDE includes
#include <KCoreAddons/KShell>
#include <KIOWidgets/KRun>

Q_GLOBAL_STATIC(Smb4KGlobalPrivate, p);
QMutex mutex(QMutex::Recursive /* needed to avoid dead-locks */);


void Smb4KGlobal::initCore(bool modifyCursor, bool initClasses)
{
  if (!p->coreInitialized)
  {
    // 
    // Busy cursor
    // 
    p->modifyCursor = modifyCursor;
    
    // 
    // Set default values
    // 
    p->setDefaultSettings();
    
    // 
    // Initialize the necessary core classes
    // 
    if (initClasses)
    {
      Smb4KClient::self()->start();
      Smb4KMounter::self()->start();
    }
    else
    {
      // Do nothing
    }

    p->makeConnections();
    p->coreInitialized = true;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KGlobal::abortCore()
{
  Smb4KClient::self()->abort();
  Smb4KMounter::self()->abortAll();
  Smb4KSynchronizer::self()->abortAll();
  Smb4KSearch::self()->abortAll();
}


bool Smb4KGlobal::coreIsRunning()
{
  return (Smb4KClient::self()->isRunning() ||
          Smb4KMounter::self()->isRunning() ||
          Smb4KSynchronizer::self()->isRunning() ||
          Smb4KSearch::self()->isRunning());
}


void Smb4KGlobal::setDefaultSettings()
{
  p->setDefaultSettings();
}


bool Smb4KGlobal::coreIsInitialized()
{
  return p->coreInitialized;
}


const QList<WorkgroupPtr> &Smb4KGlobal::workgroupsList()
{
  return p->workgroupsList;
}


WorkgroupPtr Smb4KGlobal::findWorkgroup(const QString &name)
{
  WorkgroupPtr workgroup;

  mutex.lock();
  
  for (const WorkgroupPtr &w : p->workgroupsList)
  {
    if (QString::compare(w->workgroupName(), name, Qt::CaseInsensitive) == 0)
    {
      workgroup = w;
      break;
    }
    else
    {
      // Do nothing
    }
  }

  mutex.unlock();

  return workgroup;
}


bool Smb4KGlobal::addWorkgroup(WorkgroupPtr workgroup)
{
  Q_ASSERT(workgroup);
  
  bool added = false;

  if (workgroup)
  {
    mutex.lock();

    if (!findWorkgroup(workgroup->workgroupName()))
    {
      p->workgroupsList.append(workgroup);
      added = true;
    }
    else
    {
      // Do nothing
    }

    mutex.unlock();
  }
  else
  {
    // Do nothing
  }

  return added;
}


bool Smb4KGlobal::updateWorkgroup(WorkgroupPtr workgroup)
{
  Q_ASSERT(workgroup);
  
  bool updated = false;
  
  if (workgroup)
  {
    mutex.lock();
    
    WorkgroupPtr existingWorkgroup = findWorkgroup(workgroup->workgroupName());
    
    if (existingWorkgroup)
    {
      existingWorkgroup->update(workgroup.data());
      updated = true;
    }
    else
    {
      // Do nothing
    }
    
    mutex.unlock();
  }
  else
  {
    // Do nothing
  }
  
  return updated;
}



bool Smb4KGlobal::removeWorkgroup(WorkgroupPtr workgroup)
{
  Q_ASSERT(workgroup);

  bool removed = false;
  
  if (workgroup)
  {
    mutex.lock();

    int index = p->workgroupsList.indexOf(workgroup);

    if (index != -1)
    {
      // The workgroup was found. Remove it.
      p->workgroupsList.takeAt(index).clear();
      removed = true;
    }
    else
    {
      // Try harder to find the workgroup.
      WorkgroupPtr wg = findWorkgroup(workgroup->workgroupName());

      if (wg)
      {
        index = p->workgroupsList.indexOf(wg);

        if (index != -1)
        {
          p->workgroupsList.takeAt(index).clear();
          removed = true;
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

      workgroup.clear();
    }

    mutex.unlock();
  }
  else
  {
    // Do nothing
  }

  return removed;
}


void Smb4KGlobal::clearWorkgroupsList()
{
  mutex.lock();

  while (!p->workgroupsList.isEmpty())
  {
    p->workgroupsList.takeFirst().clear();
  }

  mutex.unlock();
}


const QList<HostPtr> &Smb4KGlobal::hostsList()
{
  return p->hostsList;
}


HostPtr Smb4KGlobal::findHost(const QString &name, const QString &workgroup)
{
  HostPtr host;

  mutex.lock();
  
  for (const HostPtr &h : p->hostsList)
  {
    if ((workgroup.isEmpty() || QString::compare(h->workgroupName(), workgroup, Qt::CaseInsensitive) == 0) &&
        QString::compare(h->hostName(), name, Qt::CaseInsensitive) == 0)
    {
      host = h;
      break;
    }
    else
    {
      // Do nothing
    }
  }

  mutex.unlock();

  return host;
}


bool Smb4KGlobal::addHost(HostPtr host)
{
  Q_ASSERT(host);
  
  bool added = false;

  if (host)
  {
    mutex.lock();

    if (!findHost(host->hostName(), host->workgroupName()))
    {
      p->hostsList.append(host);
      added = true;
    }
    else
    {
      // Do nothing
    }

    mutex.unlock();
  }
  else
  {
    // Do nothing
  }

  return added;
}


bool Smb4KGlobal::updateHost(HostPtr host)
{
  Q_ASSERT(host);
  
  bool updated = false;
  
  if (host)
  {
    mutex.lock();
    
    HostPtr existingHost = findHost(host->hostName(), host->workgroupName());
    
    if (existingHost)
    {
      existingHost->update(host.data());
      updated = true;
    }
    else
    {
      // Do nothing
    }
    
    mutex.unlock();
  }
  else
  {
    // Do nothing
  }
  
  return updated;
}



bool Smb4KGlobal::removeHost(HostPtr host)
{
  Q_ASSERT(host);

  bool removed = false;
  
  if (host)
  {
    mutex.lock();

    int index = p->hostsList.indexOf(host);

    if (index != -1)
    {
      // The host was found. Remove it.
      p->hostsList.takeAt(index).clear();
      removed = true;
    }
    else
    {
      // Try harder to find the host.
      HostPtr h = findHost(host->hostName(), host->workgroupName());

      if (h)
      {
        index = p->hostsList.indexOf(h);

        if (index != -1)
        {
          p->hostsList.takeAt(index).clear();
          removed = true;
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

      host.clear();
    }

    mutex.unlock();
  }
  else
  {
    // Do nothing
  }

  return removed;
}


void Smb4KGlobal::clearHostsList()
{
  mutex.lock();

  while (!p->hostsList.isEmpty())
  {
    p->hostsList.takeFirst().clear();
  }

  mutex.unlock();
}


QList<HostPtr> Smb4KGlobal::workgroupMembers(WorkgroupPtr workgroup)
{
  QList<HostPtr> hosts;

  mutex.lock();
  
  for (const HostPtr &h : p->hostsList)
  {
    if (QString::compare(h->workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive) == 0)
    {
      hosts << h;
    }
    else
    {
      // Do nothing
    }
  }

  mutex.unlock();

  return hosts;
}


const QList<SharePtr> &Smb4KGlobal::sharesList()
{
  return p->sharesList;
}


SharePtr Smb4KGlobal::findShare(const QString& unc, const QString& workgroup)
{
  SharePtr share;
  
  mutex.lock();

  for (const SharePtr &s : p->sharesList)
  {
    if (QString::compare(s->unc(), unc, Qt::CaseInsensitive) == 0 &&
        (workgroup.isEmpty() || QString::compare(s->workgroupName(), workgroup, Qt::CaseInsensitive) == 0))
    {
      share = s;
      break;
    }
    else
    {
      // Do nothing
    }
  }
  
  mutex.unlock();
  
  return share;
}



bool Smb4KGlobal::addShare(SharePtr share)
{
  Q_ASSERT(share);

  bool added = false;
  
  if (share)
  {
    mutex.lock();
    
    //
    // Add the share
    //
    if (!findShare(share->unc(), share->workgroupName()))
    {
      // 
      // Set the share mounted
      // Only honor shares that are owned by the user
      // 
      QList<SharePtr> mountedShares = findShareByUNC(share->unc());
      
      if (!mountedShares.isEmpty())
      {
        for (const SharePtr &s : mountedShares)
        {
          if (!s->isForeign())
          {
            share->setMountData(s.data());
            break;
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        // Do nothing
      }  
      
      // 
      // Add it
      // 
      p->sharesList.append(share);
      added = true;
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

  mutex.unlock();

  return added;
}


bool Smb4KGlobal::updateShare(SharePtr share)
{
  Q_ASSERT(share);
  
  bool updated = false;
  
  if (share)
  {
    mutex.lock();
    
    //
    // Updated the share
    //
    SharePtr existingShare = findShare(share->unc(), share->workgroupName());
    
    if (existingShare)
    {
      // 
      // Set the share mounted
      // Only honor shares that are owned by the user
      // 
      QList<SharePtr> mountedShares = findShareByUNC(share->unc());
      
      if (!mountedShares.isEmpty())
      {
        for (const SharePtr &s : mountedShares)
        {
          if (!s->isForeign())
          {
            share->setMountData(s.data());
            break;
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        // Do nothing
      }
      
      // 
      // Update it
      // 
      existingShare->update(share.data());
      updated = true;
    }
    else
    {
      // Do nothing
    }
    
    mutex.unlock();
  }
  else
  {
    // Do nothing
  }
  
  return updated;
}



bool Smb4KGlobal::removeShare(SharePtr share)
{
  Q_ASSERT(share);

  bool removed = false;
  
  if (share)
  {
    mutex.lock();

    int index = p->sharesList.indexOf(share);

    if (index != -1)
    {
      // The share was found. Remove it.
      p->sharesList.takeAt(index).clear();
      removed = true;
    }
    else
    {
      // Try harder to find the share.
      SharePtr s = findShare(share->unc(), share->workgroupName());

      if (s)
      {
        index = p->sharesList.indexOf(s);

        if (index != -1)
        {
          p->sharesList.takeAt(index).clear();
          removed = true;
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

      share.clear();
    }

    mutex.unlock();
  }
  else
  {
    // Do nothing
  }

  return removed;
}


void Smb4KGlobal::clearSharesList()
{
  mutex.lock();

  while (!p->sharesList.isEmpty())
  {
    p->sharesList.takeFirst().clear();
  }

  mutex.unlock();
}


QList<SharePtr> Smb4KGlobal::sharedResources(HostPtr host)
{
  QList<SharePtr> shares;

  mutex.lock();
  
  for (const SharePtr &s : p->sharesList)
  {
    if (QString::compare(s->hostName(), host->hostName(), Qt::CaseInsensitive) == 0 &&
        QString::compare(s->workgroupName(), host->workgroupName(), Qt::CaseInsensitive) == 0)
    {
      shares += s;
    }
    else
    {
      // Do nothing
    }
  }

  mutex.unlock();

  return shares;
}


const QList<SharePtr> &Smb4KGlobal::mountedSharesList()
{
  return p->mountedSharesList;
}


SharePtr Smb4KGlobal::findShareByPath(const QString &path)
{
  SharePtr share;

  mutex.lock();

  if (!path.isEmpty() && !p->mountedSharesList.isEmpty())
  {
    for (const SharePtr &s : p->mountedSharesList)
    {
      if (QString::compare(s->path(), path, Qt::CaseInsensitive) == 0 ||
          QString::compare(s->canonicalPath(), path, Qt::CaseInsensitive) == 0)
      {
        share = s;
        break;
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

  mutex.unlock();

  return share;
}


QList<SharePtr> Smb4KGlobal::findShareByUNC(const QString &unc)
{
  QList<SharePtr> shares;

  mutex.lock();

  if (!unc.isEmpty() && !p->mountedSharesList.isEmpty())
  {
    for (const SharePtr &s : p->mountedSharesList)
    {
      if (QString::compare(s->unc(), unc, Qt::CaseInsensitive) == 0)
      {
        shares += s;
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

  mutex.unlock();

  return shares;
}


QList<SharePtr> Smb4KGlobal::findInaccessibleShares()
{
  QList<SharePtr> inaccessibleShares;

  mutex.lock();
  
  for (const SharePtr &s : p->mountedSharesList)
  {
    if (s->isInaccessible())
    {
      inaccessibleShares += s;
    }
    else
    {
      // Do nothing
    }
  }

  mutex.unlock();

  return inaccessibleShares;
}


bool Smb4KGlobal::addMountedShare(SharePtr share)
{
  Q_ASSERT(share);

  bool added = false;

  if (share)
  {
    mutex.lock();

    //
    // Copy the mount data to the network share and the search results.
    // Only honor shares that were mounted by the user.
    //
    if (!share->isForeign())
    {
      // Network shares
      SharePtr networkShare = findShare(share->unc(), share->workgroupName());
      
      if (networkShare)
      {
        networkShare->setMountData(share.data());
      }
      else
      {
        // Do nothing
      }
      
      // Search results
      for (SharePtr s : p->searchResults)
      {
        if (share->unc() == s->unc())
        {
          s->setMountData(share.data());
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

    if (!findShareByPath(share->path()))
    {
      //
      // Check if we have to add a workgroup name and/or IP address
      //
      HostPtr networkHost = findHost(share->hostName(), share->workgroupName());
      
      if (networkHost)
      {
        // Set the IP address
        if (!share->hasHostIpAddress() || networkHost->ipAddress() != share->hostIpAddress())
        {
          share->setHostIpAddress(networkHost->ipAddress());
        }
        else
        {
          // Do nothing
        }
        
        // Set the workgroup name
        if (share->workgroupName().isEmpty())
        {
          share->setWorkgroupName(networkHost->workgroupName());
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
      
      p->mountedSharesList.append(share);
      added = true;

      p->onlyForeignShares = true;
    
      for (const SharePtr &s : p->mountedSharesList)
      {
        if (!s->isForeign())
        {
          p->onlyForeignShares = false;
          break;
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

    mutex.unlock();
  }
  else
  {
    // Do nothing
  }

  return added;
}


bool Smb4KGlobal::updateMountedShare(SharePtr share)
{
  Q_ASSERT(share);
  
  bool updated = false;
  
  if (share)
  {
    mutex.lock();
    
    //
    // Copy the mount data to the network share (needed for unmounting from the network browser)
    // Only honor shares that were mounted by the user.
    //
    if (!share->isForeign())
    {
      SharePtr networkShare = findShare(share->unc(), share->workgroupName());
      
      if (networkShare)
      {
        networkShare->setMountData(share.data());
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
    
    SharePtr mountedShare = findShareByPath(share->path());
    
    if (mountedShare)
    {
      //
      // Check if we have to add a workgroup name and/or IP address
      //
      HostPtr networkHost = findHost(share->hostName(), share->workgroupName());
      
      if (networkHost)
      {
        // Set the IP address
        if (!share->hasHostIpAddress() || networkHost->ipAddress() != share->hostIpAddress())
        {
          share->setHostIpAddress(networkHost->ipAddress());
        }
        else
        {
          // Do nothing
        }
        
        // Set the workgroup name
        if (share->workgroupName().isEmpty())
        {
          share->setWorkgroupName(networkHost->workgroupName());
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
      // Update share
      //
      mountedShare->setMountData(share.data());
      updated = true;
    }
    else
    {
      // Do nothing
    }
    
    mutex.unlock();
  }
  else
  {
    // Do nothing
  }
  
  return updated;
}


bool Smb4KGlobal::removeMountedShare(SharePtr share)
{
  Q_ASSERT(share);

  bool removed = false;
  
  if (share)
  {
    mutex.lock();
    
    //
    // Reset the mount data for the network share and the
    // search result
    // 
    if (!share->isForeign())
    {
      // Network share
      SharePtr networkShare = findShare(share->unc(), share->workgroupName());
      
      if (networkShare)
      {
        networkShare->resetMountData();
      }
      else
      {
        // Do nothing
      }
      
      // Search result
      for (SharePtr searchResult : searchResults())
      {
        if (searchResult->unc() == share->unc())
        {
          searchResult->resetMountData();
          break;
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

    //
    // Remove the mounted share
    // 
    int index = p->mountedSharesList.indexOf(share);

    if (index != -1)
    {
      // The share was found. Remove it.
      p->mountedSharesList.takeAt(index).clear();
      removed = true;
    }
    else
    {
      // Try harder to find the share.
      SharePtr s = findShareByPath(share->isInaccessible() ? share->path() : share->canonicalPath());

      if (s)
      {
        index = p->mountedSharesList.indexOf(s);

        if (index != -1)
        {
          p->mountedSharesList.takeAt(index).clear();
          removed = true;
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

      share.clear();
    }
    
    for (const SharePtr &s : p->mountedSharesList)
    {
      if (!s->isForeign())
      {
        p->onlyForeignShares = false;
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    mutex.unlock();
  }
  else
  {
    // Do nothing
  }
  
  return removed;
}


bool Smb4KGlobal::onlyForeignMountedShares()
{
  return p->onlyForeignShares;
}


bool Smb4KGlobal::addSearchResult(SharePtr share)
{
  bool added = false;
  
  if (share)
  {
    mutex.lock();
    
    //
    // Check if the share is already mounted. Ignore foreign shares
    // for that.
    // 
    QList<SharePtr> mountedShares = findShareByUNC(share->unc());
    
    if (!mountedShares.isEmpty())
    {
      for (const SharePtr &mountedShare : mountedShares)
      {
        if (!mountedShare->isForeign())
        {
          share->setMountData(mountedShare.data());
          break;
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
    
    //
    // Add the search result.
    // 
    p->searchResults.append(share);
    added = true;
    mutex.unlock();
  }
  else
  {
    // Do nothing
  }
  
  return added;
}


void Smb4KGlobal::clearSearchResults()
{
  mutex.lock();
  
  while (!p->searchResults.isEmpty())
  {
    p->searchResults.takeFirst().clear();
  }
  
  mutex.unlock();
}


QList<SharePtr> Smb4KGlobal::searchResults()
{
  return p->searchResults;
}



void Smb4KGlobal::openShare(SharePtr share, OpenWith openWith)
{
  if (!share || share->isInaccessible())
  {
    return;
  }

  switch (openWith)
  {
    case FileManager:
    {
      QUrl url = QUrl::fromLocalFile(share->canonicalPath());

      (void) new KRun(url, 0);

      break;
    }
    case Konsole:
    {
      QString konsole = QStandardPaths::findExecutable("konsole");

      if (konsole.isEmpty())
      {
        Smb4KNotification::commandNotFound("konsole");
      }
      else
      {
        KRun::runCommand(konsole+" --workdir "+KShell::quoteArg(share->canonicalPath()), 0);
      }

      break;
    }
    default:
    {
      break;
    }
  }
}


const QMap<QString,QString> &Smb4KGlobal::globalSambaOptions(bool read)
{
  return p->globalSambaOptions(read);
}


const QString Smb4KGlobal::winsServer()
{
  QMap<QString,QString> global_opts = p->globalSambaOptions(false);
  QString wins_server;
  
  if (global_opts.contains("wins server"))
  {
    wins_server = global_opts.value("wins server");
  }
  else
  {
    if (global_opts.contains("wins support") &&
         (QString::compare(global_opts.value("wins support"), "yes", Qt::CaseInsensitive) == 0 ||
          QString::compare(global_opts.value("wins support"), "true", Qt::CaseInsensitive) == 0))
    {
      wins_server = "127.0.0.1";
    }
    else
    {
      // Do nothing
    }
  }
  
  return wins_server;
}


bool Smb4KGlobal::modifyCursor()
{
  return p->modifyCursor;
}


#if defined(Q_OS_LINUX)
QStringList Smb4KGlobal::whitelistedMountArguments()
{
  return p->whitelistedMountArguments;
}
#endif


const QString Smb4KGlobal::findMountExecutable()
{
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

#if defined(Q_OS_LINUX)
  return QStandardPaths::findExecutable("mount.cifs", paths);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
  return QStandardPaths::findExecutable("mount_smbfs", paths);
#else
  return QString();
#endif
}


const QString Smb4KGlobal::findUmountExecutable()
{
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  return QStandardPaths::findExecutable("umount", paths);
}


const QString Smb4KGlobal::dataLocation()
{
  return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+QDir::separator()+"smb4k";
}

