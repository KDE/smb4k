/***************************************************************************
    This class retrieves all workgroups, servers and shares found on the 
    network neighborhood
                             -------------------
    begin                : So Mai 22 2011
    copyright            : (C) 2011-2016 by Alexander Reinholdt
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
#include "smb4kscanner.h"
#include "smb4kscanner_p.h"
#include "smb4ksettings.h"
#include "smb4kbasicnetworkitem.h"
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kauthinfo.h"
#include "smb4kwalletmanager.h"
#include "smb4knotification.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QUdpSocket>
#include <QTest>

using namespace Smb4KGlobal;

#define TIMER_INTERVAL 250

Q_GLOBAL_STATIC(Smb4KScannerStatic, p);



Smb4KScanner::Smb4KScanner(QObject *parent)
: KCompositeJob(parent), d(new Smb4KScannerPrivate)
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
  
  d->elapsedTimePS   = 0;
  d->elapsedTimeIP   = 0;
  d->scanningAllowed = true;
  d->haveNewHosts    = false;
  
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}


Smb4KScanner::~Smb4KScanner()
{
}


Smb4KScanner *Smb4KScanner::self()
{
  return &p->instance;
}


bool Smb4KScanner::isRunning()
{
  return hasSubjobs();
}


bool Smb4KScanner::isRunning(Smb4KGlobal::Process process, Smb4KBasicNetworkItem *item)
{
  bool running = false;

  switch (process)
  {
    case LookupDomains:
    {
      // We do not need a network item with this kind
      // of process. We'll just test if at least on job labeled
      // 'LookupDomainsJob' or 'ScanBAreasJob' is running.
      for (int i = 0; i < subjobs().size(); ++i)
      {
        if (QString::compare(subjobs().at(i)->objectName(), "LookupDomainsJob") == 0 ||
             QString::compare(subjobs().at(i)->objectName(), "ScanBAreasJob") == 0)
        {
          running = true;
          break;
        }
        else
        {
          continue;
        }
      }
      break;
    }
    case LookupDomainMembers:
    {
      if (item && item->type() == Workgroup)
      {
        // Only return TRUE if a job for the passed workgroup is running.
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>(item);
        
        if (workgroup)
        {
          for (int i = 0; i < subjobs().size(); ++i)
          {
            if (QString::compare(subjobs().at(i)->objectName(),
                 QString("LookupDomainMembersJob_%1").arg(workgroup->workgroupName()), Qt::CaseInsensitive) == 0)
            {
              running = true;
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupDomainMembersJob".
        for (int i = 0; i < subjobs().size(); ++i)
        {
          if (subjobs().at(i)->objectName().startsWith(QLatin1String("LookupDomainMembersJob")))
          {
            running = true;
            break;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    case LookupShares:
    {
      if (item && item->type() == Host)
      {
        // Only return TRUE if a job for the passed host is running.
        Smb4KHost *host = static_cast<Smb4KHost *>(item);
        
        if (host)
        {
          for (int i = 0; i < subjobs().size(); ++i)
          {
            if (QString::compare(subjobs().at(i)->objectName(),
                 QString("LookupSharesJob_%1").arg(host->hostName()), Qt::CaseInsensitive) == 0)
            {
              running = true;
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupSharesJob".
        for (int i = 0; i < subjobs().size(); ++i)
        {
          if (subjobs().at(i)->objectName().startsWith(QLatin1String("LookupSharesJob")))
          {
            running = true;
            break;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }

  return running;
}


void Smb4KScanner::abortAll()
{
  QListIterator<KJob *> it(subjobs());
    
  while (it.hasNext())
  {
    it.next()->kill(KJob::EmitResult);
  }
}


void Smb4KScanner::abort(Smb4KGlobal::Process process, Smb4KBasicNetworkItem *item)
{
  switch (process)
  {
    case LookupDomains:
    {
      // We do not need a network item with this kind
      // of process. We'll just kill all jobs labeled
      // 'LookupDomainsJob' and 'ScanBAreasJob'.
      for (int i = 0; i < subjobs().size(); ++i)
      {
        if (QString::compare(subjobs().at(i)->objectName(), "LookupDomainsJob") == 0 ||
             QString::compare(subjobs().at(i)->objectName(), "ScanBAreasJob") == 0)
        {
          subjobs().at(i)->kill(KJob::EmitResult);
          continue;
        }
        else
        {
          continue;
        }
      }
      break;
    }
    case LookupDomainMembers:
    {
      if (item && item->type() == Workgroup)
      {
        // Only kill a job if the workgroup matches.
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>(item);
        
        if (workgroup)
        {
          for (int i = 0; i < subjobs().size(); ++i)
          {
            if (QString::compare(subjobs().at(i)->objectName(),
                 QString("LookupDomainMembersJob_%1").arg(workgroup->workgroupName()), Qt::CaseInsensitive) == 0)
            {
              subjobs().at(i)->kill(KJob::EmitResult);
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupDomainMembersJob".
        for (int i = 0; i < subjobs().size(); ++i)
        {
          if (subjobs().at(i)->objectName().startsWith(QLatin1String("LookupDomainMembersJob")))
          {
            subjobs().at(i)->kill(KJob::EmitResult);
            continue;
          }
          else
          {
            continue;
          }
        }
      }
      break;
    }
    case LookupShares:
    {
      if (item && item->type() == Host)
      {
        // Only kill a job if the host matches
        Smb4KHost *host = static_cast<Smb4KHost *>(item);
        
        if (host)
        {
          for (int i = 0; i < subjobs().size(); ++i)
          {
            if (QString::compare(subjobs().at(i)->objectName(),
                 QString("LookupSharesJob_%1").arg(host->hostName()), Qt::CaseInsensitive) == 0)
            {
              subjobs().at(i)->kill(KJob::EmitResult);
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
          // Do nothing --- This should not happen.
        }
      }
      else
      {
        // If no item is defined, we just loop through the subjobs
        // and search for a "LookupSharesJob".
        for (int i = 0; i < subjobs().size(); ++i)
        {
          if (subjobs().at(i)->objectName().startsWith(QLatin1String("LookupSharesJob")))
          {
            subjobs().at(i)->kill(KJob::EmitResult);
            continue;
          }
          else
          {
            continue;
          }
        }
      }

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KScanner::start()
{
  // Avoid a race with QApplication and use 50 ms here.
  QTimer::singleShot(50, this, SLOT(slotStartJobs()));
}


void Smb4KScanner::lookupDomains(QWidget *parent)
{
  // Send Wake On LAN magic packages
  if (Smb4KSettings::enableWakeOnLAN())
  {
    QList<Smb4KCustomOptions *> wol_entries = Smb4KCustomOptionsManager::self()->wolEntries();
    
    if (!wol_entries.isEmpty())
    {
      Smb4KBasicNetworkItem item;
      emit aboutToStart(&item, WakeUp);
      
      QUdpSocket *socket = new QUdpSocket(this);
      
      for (int i = 0; i < wol_entries.size(); ++i)
      {
        if (wol_entries.at(i)->wolSendBeforeNetworkScan())
        {
          QHostAddress addr;
          
          if (!wol_entries.at(i)->ip().isEmpty())
          {
            addr.setAddress(wol_entries.at(i)->ip());
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
          QStringList parts = wol_entries.at(i)->macAddress().split(':', QString::SkipEmptyParts);
          
          for (int j = 0; j < 16; ++j)
          {
            for (int k = 0; k < parts.size(); ++k)
            {
              sequence.append(QChar(QString("0x%1").arg(parts.at(k)).toInt(0, 16)).toLatin1());
            }
          }
          
          socket->writeDatagram(sequence, addr, 9);
        }
        else
        {
          // Do nothing
        }
      }
      
      delete socket;
      
      // Wait the defined time
      int stop = 1000 * Smb4KSettings::wakeOnLANWaitingTime() / 250;
      int i = 0;
      
      while (i++ < stop)
      {
        QTest::qWait(250);
      }
      
      emit finished(&item, WakeUp);
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
  
  // Now look up the domains  
  if (Smb4KSettings::lookupDomains())
  {
    Smb4KLookupDomainsJob *job = new Smb4KLookupDomainsJob(this);
    job->setObjectName("LookupDomainsJob");
    job->setupLookup(parent);

    connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
    connect(job, SIGNAL(aboutToStart()), SLOT(slotAboutToStartDomainsLookup()));
    connect(job, SIGNAL(finished()), SLOT(slotDomainsLookupFinished()));
    connect(job, SIGNAL(workgroups(QList<Smb4KWorkgroup*>)), SLOT(slotWorkgroups(QList<Smb4KWorkgroup*>)));

    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }

    addSubjob(job);

    job->start();
  }
  else if (Smb4KSettings::queryCurrentMaster())
  {
    Smb4KQueryMasterJob *job = new Smb4KQueryMasterJob(this);
    job->setObjectName("LookupDomainsJob");
    job->setupLookup(QString(), parent);

    connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
    connect(job, SIGNAL(aboutToStart()), SLOT(slotAboutToStartDomainsLookup()));
    connect(job, SIGNAL(finished()), SLOT(slotDomainsLookupFinished()));
    connect(job, SIGNAL(workgroups(QList<Smb4KWorkgroup*>)), SLOT(slotWorkgroups(QList<Smb4KWorkgroup*>)));
    connect(job, SIGNAL(authError(Smb4KQueryMasterJob*)), SLOT(slotAuthError(Smb4KQueryMasterJob*)));

    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }

    addSubjob(job);

    job->start();
  }
  else if (Smb4KSettings::queryCustomMaster())
  {
    // If the custom master browser entry is empty, warn the user
    // and tell him/her that we are going to query the current master
    // browser instead.
    if (Smb4KSettings::customMasterBrowser().isEmpty())
    {
      Smb4KNotification::emptyCustomMasterBrowser();
    }
    else
    {
      // Do nothing
    }
    
    Smb4KQueryMasterJob *job = new Smb4KQueryMasterJob(this);
    job->setObjectName("LookupDomainsJob");
    job->setupLookup(Smb4KSettings::customMasterBrowser(), parent);

    connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
    connect(job, SIGNAL(aboutToStart()), SLOT(slotAboutToStartDomainsLookup()));
    connect(job, SIGNAL(finished()), SLOT(slotDomainsLookupFinished()));
    connect(job, SIGNAL(workgroups(QList<Smb4KWorkgroup*>)), SLOT(slotWorkgroups(QList<Smb4KWorkgroup*>)));
    connect(job, SIGNAL(authError(Smb4KQueryMasterJob*)), SLOT(slotAuthError(Smb4KQueryMasterJob*)));

    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }

    addSubjob(job);

    job->start();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KScanner::lookupDomainMembers(Smb4KWorkgroup *workgroup, QWidget *parent)
{
  Q_ASSERT(workgroup);

  Smb4KLookupDomainMembersJob *job = new Smb4KLookupDomainMembersJob(this);
  job->setObjectName(QString("LookupDomainMembersJob_%1").arg(workgroup->workgroupName()));
  job->setupLookup(workgroup, parent);

  connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
  connect(job, SIGNAL(aboutToStart(Smb4KWorkgroup*)), SLOT(slotAboutToStartHostsLookup(Smb4KWorkgroup*)));
  connect(job, SIGNAL(finished(Smb4KWorkgroup*)), SLOT(slotHostsLookupFinished(Smb4KWorkgroup*)));
  connect(job, SIGNAL(hosts(Smb4KWorkgroup*,QList<Smb4KHost*>)), SLOT(slotHosts(Smb4KWorkgroup*,QList<Smb4KHost*>)));
  connect(job, SIGNAL(authError(Smb4KLookupDomainMembersJob*)), SLOT(slotAuthError(Smb4KLookupDomainMembersJob*)));

  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::setOverrideCursor(Qt::BusyCursor);
  }
  else
  {
    // Do nothing
  }

  addSubjob(job);

  job->start();
}


void Smb4KScanner::lookupShares(Smb4KHost *host, QWidget *parent)
{
  Q_ASSERT(host);
  
  Smb4KLookupSharesJob *job = new Smb4KLookupSharesJob(this);
  job->setObjectName(QString("LookupSharesJob_%1").arg(host->hostName()));
  job->setupLookup(host, parent);
  
  connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
  connect(job, SIGNAL(aboutToStart(Smb4KHost*)), SLOT(slotAboutToStartSharesLookup(Smb4KHost*)));
  connect(job, SIGNAL(finished(Smb4KHost*)), SLOT(slotSharesLookupFinished(Smb4KHost*)));
  connect(job, SIGNAL(shares(Smb4KHost*,QList<Smb4KShare*>)), SLOT(slotShares(Smb4KHost*,QList<Smb4KShare*>)));
  connect(job, SIGNAL(authError(Smb4KLookupSharesJob*)), SLOT(slotAuthError(Smb4KLookupSharesJob*)));
  
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::setOverrideCursor(Qt::BusyCursor);
  }
  else
  {
    // Do nothing
  }

  addSubjob(job);

  job->start();
}


void Smb4KScanner::timerEvent(QTimerEvent */*e*/)
{
  // Periodic scanning
  if (Smb4KSettings::periodicScanning())
  {
    if (d->elapsedTimePS == 0)
    {
      // Fill the list of periodic jobs.
      // Make sure that the jobs do not accumulate when periodic 
      // scanning is inhibited, but that there is a list of jobs
      // that can immediately be executed when scanning is allowed 
      // again.
      if (d->scanningAllowed || d->periodicJobs.isEmpty())
      {
        d->periodicJobs << LookupDomains;
        d->periodicJobs << LookupDomainMembers;
        d->periodicJobs << LookupShares;
      }
      else
      {
        // Do nothing
      }
    }
    else if (d->elapsedTimePS >= (Smb4KSettings::scanInterval() * 60000 /* milliseconds */) )
    {
      // Reset interval.
      // To get the correct behavior, we need to set the time to -TIMER_INTERVAL!
      d->elapsedTimePS = -TIMER_INTERVAL;      
    }
    else
    {
      // Do nothing
    }

    // Periodic scanning is only done if there are jobs 
    // to process, there are no subjobs in the queue and 
    // scanning is allowed.
    if (!d->periodicJobs.isEmpty() && !hasSubjobs() && d->scanningAllowed)
    {
      // Get the process and initiate the periodic scanning.
      Process p = d->periodicJobs.takeFirst();

      switch (p)
      {
        case LookupDomains:
        {
          lookupDomains();
          break;
        }
        case LookupDomainMembers:
        {
          for (int i = 0; i < workgroupsList().size(); ++i)
          {
            lookupDomainMembers(workgroupsList()[i]);
          }
          break;
        }
        case LookupShares:
        {
          for (int i = 0; i < hostsList().size(); ++i)
          {
            lookupShares(hostsList()[i]);
          }
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

    d->elapsedTimePS += TIMER_INTERVAL;
  }
  else
  {
    // Periodic scanning is not enabled or has been disabled
    // during runtime. So, reset the interval, if necessary.
    if (d->elapsedTimePS != 0)
    {
      d->elapsedTimePS = 0;
    }
    else
    {
      // Do nothing
    }
  }

  // Look up IP addresses.
  // Only run this if there are no subjobs in the queue. Otherwise
  // there might be crashes in the GUI or other nasty stuff.
  // 
  // Checks are run either if new hosts were discovered or every
  // 60 seconds. The latter is done to retrieve IP addresses for
  // hosts for which the IP address was not discovered in the
  // first run.
  if (!hasSubjobs() && ((d->haveNewHosts && !hostsList().isEmpty()) || d->elapsedTimeIP >= 60000))
  {
    for (int i = 0; i < hostsList().size(); ++i)
    {
      if (!hostsList().at(i)->hasIP())
      {
        Smb4KLookupIPAddressJob *job = new Smb4KLookupIPAddressJob(this);
        job->setObjectName(QString("LookupIPAddressJob_%1").arg(hostsList().at(i)->unc()));
        job->setupLookup(hostsList().at(i), 0);

        connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
        connect(job, SIGNAL(ipAddress(Smb4KHost*)), SLOT(slotProcessIPAddress(Smb4KHost*)));

        addSubjob(job);

        job->start();
      }
      else
      {
        continue;
      }
    }

    d->haveNewHosts = false;
    d->elapsedTimeIP = -TIMER_INTERVAL;
  }
  else
  {
    // Do nothing
  }

  d->elapsedTimeIP += TIMER_INTERVAL;
}



/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KScanner::slotAboutToQuit()
{
  abortAll();
}


void Smb4KScanner::slotStartJobs()
{
  // If the user wants to have periodic scanning of the network
  // neighborhood, set it up here here.
  if (Smb4KSettings::periodicScanning())
  {
    // Fill list
    d->periodicJobs << LookupDomains;
    d->periodicJobs << LookupDomainMembers;
    d->periodicJobs << LookupShares;
  }
  else
  {
    lookupDomains(0);
  }

  // Start the timer in any case. Thus, we are able to switch
  // to periodic scanning seamlessly in the timerEvent() function.
  startTimer(TIMER_INTERVAL);
}


void Smb4KScanner::slotJobFinished(KJob *job)
{
  removeSubjob(job);

  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KScanner::slotAuthError(Smb4KQueryMasterJob *job)
{
  // Do not allow periodic scanning when an authentication
  // error occurred. We do not want to operate on a network
  // item that might get invalidated during periodic scanning.
  d->scanningAllowed = false;
  
  Smb4KHost master_browser;
  
  if (!job->masterBrowser().isEmpty())
  {
    master_browser.setIsMasterBrowser(true);

    if (QHostAddress(job->masterBrowser()).protocol() == QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      master_browser.setHostName(job->masterBrowser());
    }
    else
    {
      master_browser.setIP(job->masterBrowser());
    }
    
    emit authError (&master_browser, LookupDomains);
  }
  else
  {
    // Do nothing
  }

  if (Smb4KWalletManager::self()->showPasswordDialog(&master_browser, job->parentWidget()))
  {
    // Start a query job with the returned master browser.
    Smb4KQueryMasterJob *job = new Smb4KQueryMasterJob(this);
    job->setObjectName("LookupDomainsJob");
    job->setupLookup(master_browser.hostName().isEmpty() ? master_browser.ip() : master_browser.hostName(), job->parentWidget());

    connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
    connect(job, SIGNAL(aboutToStart()), SLOT(slotAboutToStartDomainsLookup()));
    connect(job, SIGNAL(finished()), SLOT(slotDomainsLookupFinished()));
    connect(job, SIGNAL(workgroups(QList<Smb4KWorkgroup*>)), SLOT(slotWorkgroups(QList<Smb4KWorkgroup*>)));
    connect(job, SIGNAL(authError(Smb4KQueryMasterJob*)), SLOT(slotAuthError(Smb4KQueryMasterJob*)));

    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else
    {
      // Do nothing
    }

    addSubjob(job);

    job->start();
  }
  else
  {
    // Do nothing
  }

  d->scanningAllowed = true;
}


void Smb4KScanner::slotAuthError(Smb4KLookupDomainMembersJob *job)
{
  // Do not allow periodic scanning when an authentication
  // error occurred. We do not want to operate on a network
  // item that might get invalidated during periodic scanning.
  d->scanningAllowed = false;

  Smb4KWorkgroup *workgroup = findWorkgroup(job->workgroup()->workgroupName());
  Smb4KHost *master_browser = findHost(job->workgroup()->masterBrowserName(), job->workgroup()->workgroupName());
  
  if (workgroup && master_browser)
  {
    emit authError(master_browser, LookupDomainMembers);
    
    if (Smb4KWalletManager::self()->showPasswordDialog(master_browser, job->parentWidget()))
    {
      lookupDomainMembers(workgroup, job->parentWidget());
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

  d->scanningAllowed = true;
}


void Smb4KScanner::slotAuthError(Smb4KLookupSharesJob *job)
{
  // Do not allow periodic scanning when an authentication
  // error occurred. We do not want to operate on a network
  // item that might get invalidated during periodic scanning.
  d->scanningAllowed = false;
  
  Smb4KHost *host = findHost(job->host()->hostName(), job->host()->workgroupName());
  
  if (host)
  {
    emit authError(host, LookupShares);
    
    if (Smb4KWalletManager::self()->showPasswordDialog(host, job->parentWidget()))
    {
      lookupShares(host, job->parentWidget());
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

  d->scanningAllowed = true;
}


void Smb4KScanner::slotAboutToStartDomainsLookup()
{
  Smb4KBasicNetworkItem item;
  emit aboutToStart(&item, LookupDomains);
}


void Smb4KScanner::slotDomainsLookupFinished()
{
  Smb4KBasicNetworkItem item;
  emit finished(&item, LookupDomains);
}


void Smb4KScanner::slotAboutToStartHostsLookup(Smb4KWorkgroup *workgroup)
{
  emit aboutToStart(workgroup, LookupDomainMembers);
}


void Smb4KScanner::slotHostsLookupFinished(Smb4KWorkgroup *workgroup)
{
  emit finished(workgroup, LookupDomainMembers);
}


void Smb4KScanner::slotAboutToStartSharesLookup(Smb4KHost *host)
{
  emit aboutToStart(host, LookupShares);
}


void Smb4KScanner::slotSharesLookupFinished(Smb4KHost *host)
{
  emit finished(host, LookupShares);
}


void Smb4KScanner::slotWorkgroups(const QList<Smb4KWorkgroup *> &workgroups_list)
{
  // The new workgroup list will be used as global workgroup list.
  // We do some checks and adjustments now, so that the host list 
  // is also correctly updated.
  if (!workgroups_list.isEmpty())
  {
    for (int i = 0; i < workgroups_list.size(); ++i)
    {
      Smb4KWorkgroup *workgroup = findWorkgroup(workgroups_list.at(i)->workgroupName());

      // Check if the master browser changed.
      if (workgroup)
      {
        if (QString::compare(workgroups_list.at(i)->masterBrowserName(), workgroup->masterBrowserName(), Qt::CaseInsensitive) != 0)
        {
          // Get the old master browser and reset the master browser flag.
          Smb4KHost *old_master_browser = findHost(workgroup->masterBrowserName(), workgroup->workgroupName());

          if (old_master_browser)
          {
            old_master_browser->setIsMasterBrowser(false);
          }
          else
          {
            // Do nothing
          }

          // Lookup new master browser and either set the master browser flag
          // or insert it if it does not exit yet.
          Smb4KHost *new_master_browser = findHost(workgroups_list.at(i)->masterBrowserName(), workgroups_list.at(i)->workgroupName());

          if (new_master_browser)
          {
            if (workgroups_list.at(i)->hasMasterBrowserIP())
            {
              new_master_browser->setIP(workgroups_list.at(i)->masterBrowserIP());
            }
            else
            {
              // Do nothing
            }

            new_master_browser->setIsMasterBrowser(true);
          }
          else
          {
            new_master_browser = new Smb4KHost();
            new_master_browser->setHostName(workgroups_list.at(i)->masterBrowserName());

            if (workgroups_list.at(i)->hasMasterBrowserIP())
            {
              new_master_browser->setIP(workgroups_list.at(i)->masterBrowserIP());
            }
            else
            {
              // Do nothing
            }

            new_master_browser->setWorkgroupName(workgroups_list.at(i)->workgroupName());
            new_master_browser->setIsMasterBrowser(true);

            addHost(new_master_browser);
          }
        }
        else
        {
          // Do nothing
        }

        removeWorkgroup(workgroup);
      }
      else
      {
        // Check if the master browser of the new workgroup list is by chance
        // already in the list of hosts. If it exists, set the master browser
        // flag, else insert it.
        Smb4KHost *new_master_browser = findHost(workgroups_list.at(i)->masterBrowserName(), workgroups_list.at(i)->workgroupName());

        if (new_master_browser)
        {
          if (workgroups_list.at(i)->hasMasterBrowserIP())
          {
            new_master_browser->setIP(workgroups_list.at(i)->masterBrowserIP());
          }
          else
          {
            // Do nothing
          }

          new_master_browser->setIsMasterBrowser(true);
        }
        else
        {
          new_master_browser = new Smb4KHost();
          new_master_browser->setHostName(workgroups_list.at(i)->masterBrowserName());

          if (workgroups_list.at(i)->hasMasterBrowserIP())
          {
            new_master_browser->setIP(workgroups_list.at(i)->masterBrowserIP());
          }
          else
          {
            // Do nothing
          }

          new_master_browser->setWorkgroupName(workgroups_list.at(i)->workgroupName());
          new_master_browser->setIsMasterBrowser(true);

          addHost(new_master_browser);
        }
      }
    }
    
    d->haveNewHosts = true;
  }
  else
  {
    // Do nothing
  }

  // The global workgroup list only contains obsolete workgroups now.
  // Remove all hosts belonging to those obsolete workgroups from the
  // host list and then also the workgroups themselves.
  while (!workgroupsList().isEmpty())
  {
    Smb4KWorkgroup *workgroup = workgroupsList().first();
    QList<Smb4KHost *> obsolete_hosts = workgroupMembers(workgroup);
    QListIterator<Smb4KHost *> h(obsolete_hosts);
    
    while (h.hasNext())
    {
      Smb4KHost *host = h.next();
      removeHost(host);
    }

    removeWorkgroup(workgroup);
  }

  // Add a copy of all workgroups to the global list.
  for (int i = 0; i < workgroups_list.size(); ++i)
  {
    addWorkgroup(new Smb4KWorkgroup(*workgroups_list.at(i)));
  }

  emit workgroups(workgroupsList());
}


void Smb4KScanner::slotHosts(const QList<Smb4KHost *> &hosts_list)
{
  slotHosts(0, hosts_list);
}


void Smb4KScanner::slotHosts(Smb4KWorkgroup *workgroup, const QList<Smb4KHost *> &hosts_list)
{
  if (!hosts_list.isEmpty())
  {
    // Copy any information we might need to the internal list and
    // remove the host from the global list. It will be added again
    // in an instant.
    for (int i = 0; i < hosts_list.size(); ++i)
    {
      Smb4KHost *host = findHost(hosts_list.at(i)->hostName(), hosts_list.at(i)->workgroupName());

      if (host)
      {
        // Set comment
        if (hosts_list.at(i)->comment().isEmpty() && !host->comment().isEmpty())
        {
          hosts_list[i]->setComment(host->comment());
        }
        else
        {
          // Do nothing
        }

        // Set the IP addresses
        if (!hosts_list.at(i)->hasIP() && host->hasIP())
        {
          hosts_list[i]->setIP(host->ip());
        }
        else
        {
          // Do nothing
        }

        removeHost(host);
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

  if (workgroup)
  {
    // Now remove all (obsolete) hosts of the scanned workgroup from
    // the global list as well as their shares.
    QList<Smb4KHost *> obsolete_hosts = workgroupMembers(workgroup);
    QListIterator<Smb4KHost *> h(obsolete_hosts);
    
    while (h.hasNext())
    {
      Smb4KHost *host = h.next();
      
      QList<Smb4KShare *> obsolete_shares = sharedResources(host);
      QListIterator<Smb4KShare *> s(obsolete_shares);
      
      while (s.hasNext())
      {
        Smb4KShare *share = s.next();
        removeShare(share);
      }
      
      removeHost(host);
    }
  }
  else
  {
    // If no workgroup was passed it means that we are doing an IP scan
    // or at least members of more than one workgroup were looked up. In
    // this case the global hosts list is considered to carry obsolete
    // host entries at this point. Remove them as well as their shares.
    while (!hostsList().isEmpty())
    {
      Smb4KHost *host = hostsList().first();

      QList<Smb4KShare *> obsolete_shares = sharedResources(host);
      QListIterator<Smb4KShare *> s(obsolete_shares);
      
      while (s.hasNext())
      {
        Smb4KShare *share = s.next();
        removeShare(share);
      }
      
      removeHost(host);
    }
  }
  
  // Add a copy of all hosts to the global list.
  for (int i = 0; i < hosts_list.size(); ++i)
  {
    addHost(new Smb4KHost(*hosts_list.at(i)));
  }
  
  d->haveNewHosts = true;

  if (workgroup)
  {
    QList<Smb4KHost *> workgroup_members = workgroupMembers(workgroup);
    emit hosts(workgroup, workgroup_members);
  }
  else
  {
    emit hosts(workgroup, hostsList());
  }
}


void Smb4KScanner::slotShares(Smb4KHost *host, const QList<Smb4KShare *> &shares_list)
{
  Q_ASSERT(host);
  
  if (host && !shares_list.isEmpty())
  {
    // Process list of shares:
    QList<Smb4KShare *> new_shares;
    
    // Copy all shares that the user wishes to retrieve and set the mount data
    // as well as the IP address (if not already present).
    for (int i = 0; i < shares_list.size(); ++i)
    {
      if (shares_list.at(i)->isPrinter() && !Smb4KSettings::detectPrinterShares())
      {
        continue;
      }
      else if (shares_list.at(i)->isHidden() && !Smb4KSettings::detectHiddenShares())
      {
        continue;
      }
      else
      {
        // Do nothing
      }
      
      // Check if the share has already been mounted.
      QList<Smb4KShare *> mounted_shares = findShareByUNC(shares_list.at(i)->unc());
      
      if (!mounted_shares.isEmpty())
      {
        // FIXME: We cannot honor Smb4KSettings::detectAllShares() here, because 
        // in case the setting is changed, there will be no automatic rescan
        // (in case of an automatic or periodical rescan that would be the 
        // favorable method...
        //
        // For now, we prefer the share mounted by the user or use the first
        // occurrence if he/she did not mount it.
        Smb4KShare *mounted_share = mounted_shares.first();
        
        for (int j = 0; j < mounted_shares.size(); ++j)
        {
          if (!mounted_shares.at(j)->isForeign())
          {
            mounted_share = mounted_shares[j];
            break;
          }
          else
          {
            continue;
          }
        }

        shares_list[i]->setMountData(mounted_share);
      }
      else
      {
        // Do nothing
      }      
 
      // Now set the IP address, if none could be retrieved by the
      // lookup job. 
      if (!shares_list.at(i)->hasHostIP())
      {
        Smb4KShare *s = findShare(shares_list.at(i)->unc(), shares_list.at(i)->workgroupName());
        
        if (s)
        {
          shares_list[i]->setHostIP(s->hostIP());
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
      
      new_shares << shares_list[i];
    }
    
    // Process the host:
    // Copy the authentication information.
    Smb4KHost *known_host = findHost(host->hostName(), host->workgroupName());
    
    if(known_host)
    {
      known_host->setLogin(host->login());
      known_host->setPassword(host->password());
    }
    else
    {
      // Do nothing
    }
    
    // Now remove all shares of this host from the global list.
    QList<Smb4KShare *> known_shares = sharedResources(known_host);
    QListIterator<Smb4KShare *> it(known_shares);
    
    while(it.hasNext())
    {
      Smb4KShare *s = it.next();
      removeShare(s);
    }
    
    // Add a copy of all desired shares to the global list.
    for (int i = 0; i < new_shares.size(); ++i)
    {
      addShare(new Smb4KShare(*new_shares[i]));
    }
    
    // Now emit the list of shared resources.
    QList<Smb4KShare *> shared_resources = sharedResources(host);
    emit shares(host, shared_resources);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KScanner::slotProcessIPAddress(Smb4KHost *host)
{
  Q_ASSERT(host);

  Smb4KHost *known_host = findHost(host->hostName(), host->workgroupName());

  if (known_host)
  {
    known_host->setIP(host->ip());
  }
  else
  {
    // Do nothing
  }

  emit ipAddress(known_host);
}

