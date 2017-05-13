/***************************************************************************
    This is the global namespace for Smb4K.
                             -------------------
    begin                : Sa Apr 2 2005
    copyright            : (C) 2005-2016 by Alexander Reinholdt
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
#include "smb4kscanner.h"
#include "smb4kmounter.h"
#include "smb4kprint.h"
#include "smb4ksynchronizer.h"
#include "smb4kpreviewer.h"
#include "smb4ksearch.h"

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


const QEvent::Type Smb4KGlobal::Smb4KEvent::LoadSettings =        (QEvent::Type)QEvent::registerEventType();
const QEvent::Type Smb4KGlobal::Smb4KEvent::SetFocus =            (QEvent::Type)QEvent::registerEventType();
const QEvent::Type Smb4KGlobal::Smb4KEvent::ScanNetwork =         (QEvent::Type)QEvent::registerEventType();
const QEvent::Type Smb4KGlobal::Smb4KEvent::AddBookmark =         (QEvent::Type)QEvent::registerEventType();
const QEvent::Type Smb4KGlobal::Smb4KEvent::MountOrUnmountShare = (QEvent::Type)QEvent::registerEventType();


Smb4KGlobal::Smb4KEvent::Smb4KEvent(QEvent::Type type): QEvent(type)
{
}


Smb4KGlobal::Smb4KEvent::~Smb4KEvent()
{
}



void Smb4KGlobal::initCore(bool modifyCursor, bool initClasses)
{
  // Should the core use a busy cursor?
  p->modifyCursor = modifyCursor;
  
  // Set default values for some settings.
  p->setDefaultSettings();
  
  // Initialize the necessary core classes
  if (initClasses)
  {
    Smb4KScanner::self()->start();
    Smb4KMounter::self()->start();
  }
  else
  {
    // Do nothing
  }

  p->makeConnections();
  p->coreInitialized = true;
}


void Smb4KGlobal::abortCore()
{
  Smb4KScanner::self()->abortAll();
  Smb4KMounter::self()->abortAll();
  Smb4KPrint::self()->abortAll();
  Smb4KSynchronizer::self()->abortAll();
  Smb4KPreviewer::self()->abortAll();
  Smb4KSearch::self()->abortAll();
}


bool Smb4KGlobal::coreIsRunning()
{
  return (Smb4KScanner::self()->isRunning() ||
          Smb4KMounter::self()->isRunning() ||
          Smb4KPrint::self()->isRunning() ||
          Smb4KSynchronizer::self()->isRunning() ||
          Smb4KPreviewer::self()->isRunning() ||
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


const QList<Smb4KWorkgroup *> &Smb4KGlobal::workgroupsList()
{
  return p->workgroupsList;
}


Smb4KWorkgroup *Smb4KGlobal::findWorkgroup(const QString &name)
{
  Smb4KWorkgroup *workgroup = 0;

  mutex.lock();
  
  for (Smb4KWorkgroup *w : p->workgroupsList)
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


bool Smb4KGlobal::addWorkgroup(Smb4KWorkgroup *workgroup)
{
  Q_ASSERT(workgroup);

  bool added = false;

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

  return added;
}


bool Smb4KGlobal::removeWorkgroup(Smb4KWorkgroup *workgroup)
{
  Q_ASSERT(workgroup);

  bool removed = false;

  mutex.lock();

  int index = p->workgroupsList.indexOf(workgroup);

  if (index != -1)
  {
    // The workgroup was found. Remove it.
    delete p->workgroupsList.takeAt(index);
    removed = true;
  }
  else
  {
    // Try harder to find the workgroup.
    Smb4KWorkgroup *wg = findWorkgroup(workgroup->workgroupName());

    if (wg)
    {
      index = p->workgroupsList.indexOf(wg);

      if (index != -1)
      {
        delete p->workgroupsList.takeAt(index);
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

    delete workgroup;
  }

  mutex.unlock();

  return removed;
}


void Smb4KGlobal::clearWorkgroupsList()
{
  mutex.lock();

  while (!p->workgroupsList.isEmpty())
  {
    delete p->workgroupsList.takeFirst();
  }

  mutex.unlock();
}


const QList<Smb4KHost *> &Smb4KGlobal::hostsList()
{
  return p->hostsList;
}


Smb4KHost *Smb4KGlobal::findHost(const QString &name, const QString &workgroup)
{
  Smb4KHost *host = 0;

  mutex.lock();
  
  for (Smb4KHost *h : p->hostsList)
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


bool Smb4KGlobal::addHost(Smb4KHost *host)
{
  Q_ASSERT(host);

  bool added = false;

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

  return added;
}


bool Smb4KGlobal::removeHost(Smb4KHost *host)
{
  Q_ASSERT(host);

  bool removed = false;

  mutex.lock();

  int index = p->hostsList.indexOf(host);

  if (index != -1)
  {
    // The host was found. Remove it.
    delete p->hostsList.takeAt(index);
    removed = true;
  }
  else
  {
    // Try harder to find the host.
    Smb4KHost *h = findHost(host->hostName(), host->workgroupName());

    if (h)
    {
      index = p->hostsList.indexOf(h);

      if (index != -1)
      {
        delete p->hostsList.takeAt(index);
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

    delete host;
  }

  mutex.unlock();

  return removed;
}


void Smb4KGlobal::clearHostsList()
{
  mutex.lock();

  while (!p->hostsList.isEmpty())
  {
    delete p->hostsList.takeFirst();
  }

  mutex.unlock();
}


QList<Smb4KHost *> Smb4KGlobal::workgroupMembers(Smb4KWorkgroup *workgroup)
{
  QList<Smb4KHost *> hosts;

  mutex.lock();
  
  for (Smb4KHost *h : p->hostsList)
  {
    if (QString::compare(h->workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive) == 0)
    {
      hosts += h;
    }
    else
    {
      // Do nothing
    }
  }

  mutex.unlock();

  return hosts;
}


const QList<Smb4KShare *> &Smb4KGlobal::sharesList()
{
  return p->sharesList;
}


Smb4KShare *Smb4KGlobal::findShare(const QString& unc, const QString& workgroup)
{
  Smb4KShare *share = 0;
  
  mutex.lock();

  for (Smb4KShare *s : p->sharesList)
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



bool Smb4KGlobal::addShare(Smb4KShare *share)
{
  Q_ASSERT(share);

  bool added = false;
  
  if (share)
  {
    mutex.lock();

    if (!findShare(share->unc(), share->workgroupName()))
    {
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


bool Smb4KGlobal::removeShare(Smb4KShare *share)
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
      delete p->sharesList.takeAt(index);
      removed = true;
    }
    else
    {
      // Try harder to find the share.
      Smb4KShare *s = findShare(share->unc(), share->workgroupName());

      if (s)
      {
        index = p->sharesList.indexOf(s);

        if (index != -1)
        {
          delete p->sharesList.takeAt(index);
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

      delete share;
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
    delete p->sharesList.takeFirst();
  }

  mutex.unlock();
}


QList<Smb4KShare *> Smb4KGlobal::sharedResources(Smb4KHost *host)
{
  QList<Smb4KShare *> shares;

  mutex.lock();
  
  for (Smb4KShare *s : p->sharesList)
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


const QList<Smb4KShare *> &Smb4KGlobal::mountedSharesList()
{
  return p->mountedSharesList;
}


Smb4KShare* Smb4KGlobal::findShareByPath(const QString &path)
{
  Smb4KShare *share = 0;

  mutex.lock();

  if (!path.isEmpty() && !p->mountedSharesList.isEmpty())
  {
    for (Smb4KShare *s : p->mountedSharesList)
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


QList<Smb4KShare *> Smb4KGlobal::findShareByUNC(const QString &unc)
{
  QList<Smb4KShare *> shares;

  mutex.lock();

  if (!unc.isEmpty() && !p->mountedSharesList.isEmpty())
  {
    for (Smb4KShare *s : p->mountedSharesList)
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


QList<Smb4KShare*> Smb4KGlobal::findInaccessibleShares()
{
  QList<Smb4KShare *> inaccessibleShares;

  mutex.lock();
  
  for (Smb4KShare *s : p->mountedSharesList)
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


bool Smb4KGlobal::addMountedShare(Smb4KShare *share)
{
  Q_ASSERT(share);

  bool added = false;

  if (share)
  {
    mutex.lock();

    if (!findShareByPath(share->path()))
    {
      p->mountedSharesList.append(share);
      added = true;

      p->onlyForeignShares = true;
    
      for (Smb4KShare *s : p->mountedSharesList)
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


bool Smb4KGlobal::updateMountedShare(Smb4KShare* share)
{
  Q_ASSERT(share);
  
  bool updated = false;
  
  if (share)
  {
    Smb4KShare *mountedShare = findShareByPath(share->path());
    
    if (mountedShare)
    {
      // Update share
      mountedShare->setMountData(share);
      updated = true;
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
  
  return updated;
}


bool Smb4KGlobal::removeMountedShare(Smb4KShare *share)
{
  Q_ASSERT(share);

  bool removed = false;
  
  if (share)
  {
    mutex.lock();
  
    // Find the share by its path and remove it.
    QMutableListIterator<Smb4KShare *> it(p->mountedSharesList);
    
    while (it.hasNext())
    {
      Smb4KShare *s = it.next();
      
      if (QString::compare(s->path(), share->path(), Qt::CaseInsensitive) == 0 ||
          QString::compare(s->canonicalPath(), share->canonicalPath(), Qt::CaseInsensitive) == 0)
      {
        it.remove();
        removed = true;
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    for (Smb4KShare *s : p->mountedSharesList)
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
    
    delete share;
    
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


void Smb4KGlobal::openShare(Smb4KShare *share, OpenWith openWith)
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
