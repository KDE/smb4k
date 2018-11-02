/***************************************************************************
    This class retrieves all workgroups, servers and shares found on the 
    network neighborhood
                             -------------------
    begin                : So Mai 22 2011
    copyright            : (C) 2011-2018 by Alexander Reinholdt
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
#include "smb4khardwareinterface.h"

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
  d->timerId         = 0;
  
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
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


void Smb4KScanner::abortAll()
{
  QListIterator<KJob *> it(subjobs());
    
  while (it.hasNext())
  {
    it.next()->kill(KJob::EmitResult);
  }
}


void Smb4KScanner::start()
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




// void Smb4KScanner::lookupDomainMembers(WorkgroupPtr workgroup, QWidget *parent)
// {
//   Q_ASSERT(workgroup);
//   
//   Smb4KLookupDomainMembersJob *job = new Smb4KLookupDomainMembersJob(this);
//   job->setObjectName(QString("LookupDomainMembersJob_%1").arg(workgroup->workgroupName()));
//   job->setupLookup(workgroup, parent);
// 
//   connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
//   connect(job, SIGNAL(aboutToStart(WorkgroupPtr)), SLOT(slotAboutToStartHostsLookup(WorkgroupPtr)));
//   connect(job, SIGNAL(finished(WorkgroupPtr)), SLOT(slotHostsLookupFinished(WorkgroupPtr)));
//   connect(job, SIGNAL(hosts(WorkgroupPtr,QList<HostPtr>)), SLOT(slotHosts(WorkgroupPtr,QList<HostPtr>)));
//   connect(job, SIGNAL(authError(Smb4KLookupDomainMembersJob*)), SLOT(slotAuthError(Smb4KLookupDomainMembersJob*)));
// 
//   if (!hasSubjobs() && modifyCursor())
//   {
//     QApplication::setOverrideCursor(Qt::BusyCursor);
//   }
//   else
//   {
//     // Do nothing
//   }
// 
//   addSubjob(job);
// 
//   job->start();
// }


// void Smb4KScanner::lookupShares(HostPtr host, QWidget *parent)
// {
//   Q_ASSERT(host);
//   
//   Smb4KLookupSharesJob *job = new Smb4KLookupSharesJob(this);
//   job->setObjectName(QString("LookupSharesJob_%1").arg(host->hostName()));
//   job->setupLookup(host, parent);
//   
//   connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
//   connect(job, SIGNAL(aboutToStart(HostPtr)), SLOT(slotAboutToStartSharesLookup(HostPtr)));
//   connect(job, SIGNAL(finished(HostPtr)), SLOT(slotSharesLookupFinished(HostPtr)));
//   connect(job, SIGNAL(shares(HostPtr,QList<SharePtr>)), SLOT(slotShares(HostPtr,QList<SharePtr>)));
//   connect(job, SIGNAL(authError(Smb4KLookupSharesJob*)), SLOT(slotAuthError(Smb4KLookupSharesJob*)));
//   
//   if (!hasSubjobs() && modifyCursor())
//   {
//     QApplication::setOverrideCursor(Qt::BusyCursor);
//   }
//   else
//   {
//     // Do nothing
//   }
// 
//   addSubjob(job);
// 
//   job->start();
// }


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
//           lookupDomains();
          break;
        }
        case LookupDomainMembers:
        {
          for (const WorkgroupPtr &workgroup : workgroupsList())
          {
//             lookupDomainMembers(workgroup);
          }
          break;
        }
        case LookupShares:
        {
          for (const HostPtr &host : hostsList())
          {
//             lookupShares(host);
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
  if (!isRunning() && ((d->haveNewHosts && !hostsList().isEmpty()) || d->elapsedTimeIP >= 60000))
  {
    for (const HostPtr &host : hostsList())
    {
      if (!host->hasIpAddress())
      {
        Smb4KLookupIPAddressJob *job = new Smb4KLookupIPAddressJob(this);
        job->setObjectName(QString("LookupIPAddressJob_%1").arg(host->unc()));
        job->setupLookup(host, 0);

        connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
        connect(job, SIGNAL(ipAddress(HostPtr)), SLOT(slotProcessIPAddress(HostPtr)));

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


void Smb4KScanner::startScanning()
{
  if (Smb4KHardwareInterface::self()->isOnline())
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
//       lookupDomains(0);
    }

    // Start the timer in any case. Thus, we are able to switch
    // to periodic scanning seamlessly in the timerEvent() function.
    d->timerId = startTimer(TIMER_INTERVAL);
  }
  else
  {
    // Do nothing
  }
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
  //
  // Disconnect from Smb4KHardwareInterface.
  //
  disconnect(Smb4KHardwareInterface::self(), SIGNAL(networkConfigUpdated()), this, SLOT(slotStartJobs()));
  
  //
  // Start the scanning
  //
  startScanning();
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
  
  HostPtr masterBrowser;
  
  if (!job->masterBrowser().isEmpty())
  {
    masterBrowser->setIsMasterBrowser(true);

    if (QHostAddress(job->masterBrowser()).protocol() == QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      masterBrowser->setHostName(job->masterBrowser());
    }
    else
    {
      masterBrowser->setIpAddress(job->masterBrowser());
    }
    
    emit authError (masterBrowser, LookupDomains);
  }
  else
  {
    // Do nothing
  }

  if (Smb4KWalletManager::self()->showPasswordDialog(masterBrowser, job->parentWidget()))
  {
    // Start a query job with the returned master browser.
    Smb4KQueryMasterJob *job = new Smb4KQueryMasterJob(this);
    job->setObjectName("LookupDomainsJob");
    job->setupLookup(masterBrowser->hostName().isEmpty() ? masterBrowser->ipAddress() : masterBrowser->hostName(), job->parentWidget());

    connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
    connect(job, SIGNAL(aboutToStart()), SLOT(slotAboutToStartDomainsLookup()));
    connect(job, SIGNAL(finished()), SLOT(slotDomainsLookupFinished()));
    connect(job, SIGNAL(workgroups(QList<WorkgroupPtr>)), SLOT(slotWorkgroups(QList<WorkgroupPtr>)));
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

  WorkgroupPtr workgroup = findWorkgroup(job->workgroup()->workgroupName());
  HostPtr masterBrowser = findHost(job->workgroup()->masterBrowserName(), job->workgroup()->workgroupName());
  
  if (workgroup && masterBrowser)
  {
    emit authError(masterBrowser, LookupDomainMembers);
    
    if (Smb4KWalletManager::self()->showPasswordDialog(masterBrowser, job->parentWidget()))
    {
//       lookupDomainMembers(workgroup, job->parentWidget());
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
  
  HostPtr host = findHost(job->host()->hostName(), job->host()->workgroupName());
  
  if (host)
  {
    emit authError(host, LookupShares);
    
    if (Smb4KWalletManager::self()->showPasswordDialog(host, job->parentWidget()))
    {
//       lookupShares(host, job->parentWidget());
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
  NetworkItemPtr item = NetworkItemPtr(new Smb4KBasicNetworkItem());
  emit aboutToStart(item, LookupDomains);
}


void Smb4KScanner::slotDomainsLookupFinished()
{
  NetworkItemPtr item = NetworkItemPtr(new Smb4KBasicNetworkItem());
  emit finished(item, LookupDomains);
}


void Smb4KScanner::slotAboutToStartHostsLookup(const WorkgroupPtr &workgroup)
{
  emit aboutToStart(workgroup, LookupDomainMembers);
}


void Smb4KScanner::slotHostsLookupFinished(const WorkgroupPtr &workgroup)
{
  emit finished(workgroup, LookupDomainMembers);
}


void Smb4KScanner::slotAboutToStartSharesLookup(const HostPtr &host)
{
  emit aboutToStart(host, LookupShares);
}


void Smb4KScanner::slotSharesLookupFinished(const HostPtr &host)
{
  emit finished(host, LookupShares);
}


// void Smb4KScanner::slotWorkgroups(const QList<WorkgroupPtr> &workgroups_list)
// {
//   //
//   // Remove obsolete workgroups and their members
//   //
//   QListIterator<WorkgroupPtr> wIt(workgroupsList());
//   
//   while (wIt.hasNext())
//   {
//     WorkgroupPtr workgroup = wIt.next();
//     
//     bool found = false;
//     
//     for (const WorkgroupPtr &w : workgroups_list)
//     {
//       if (w->workgroupName() == workgroup->workgroupName())
//       {
//         found = true;
//         break;
//       }
//       else
//       {
//         continue;
//       }        
//     }
//     
//     if (!found)
//     {
//       QList<HostPtr> obsoleteHosts = workgroupMembers(workgroup);
//       QListIterator<HostPtr> hIt(obsoleteHosts);
//       
//       while (hIt.hasNext())
//       {
//         removeHost(hIt.next());
//       }
//       
//       removeWorkgroup(workgroup);
//     }
//     else
//     {
//       // Do nothing
//     }
//   }
//   
//   //
//   // Add new workgroups and update existing ones.
//   // 
//   for (const WorkgroupPtr &workgroup : workgroups_list)
//   {
//     if (!findWorkgroup(workgroup->workgroupName()))
//     {
//       addWorkgroup(workgroup);
//       
//       // Since this is a new workgroup, no master browser is present.
//       HostPtr masterBrowser = HostPtr(new Smb4KHost());
//       masterBrowser->setWorkgroupName(workgroup->workgroupName());
//       masterBrowser->setHostName(workgroup->masterBrowserName());
//       masterBrowser->setIP(workgroup->masterBrowserIP());
//       masterBrowser->setIsMasterBrowser(true);
//       
//       addHost(masterBrowser);
//     }
//     else
//     {
//       updateWorkgroup(workgroup);
//       
//       // Check if the master browser changed
//       QList<HostPtr> members = workgroupMembers(workgroup);
//       
//       for (const HostPtr &host : members)
//       {
//         if (workgroup->masterBrowserName() == host->hostName())
//         {
//           host->setIsMasterBrowser(true);
//           
//           if (!host->hasIP() && workgroup->hasMasterBrowserIP())
//           {
//             host->setIP(workgroup->masterBrowserIP());
//           }
//           else
//           {
//             // Do nothing
//           }          
//         }
//         else
//         {
//           host->setIsMasterBrowser(false);
//         }
//       }
//     }
//   }
//   
// //   emit workgroups();
// }


// void Smb4KScanner::slotHosts(const WorkgroupPtr &workgroup, const QList<HostPtr> &hosts_list)
// {
//   Q_ASSERT(workgroup);
//   
//   if (workgroup && !hosts_list.isEmpty())
//   {
//     //
//     // Remove obsolete hosts and their shares
//     //
//     QList<HostPtr> members = workgroupMembers(workgroup);
//     QListIterator<HostPtr> hIt(members);
//     
//     while (hIt.hasNext())
//     {
//       HostPtr host = hIt.next();
//       
//       bool found = false;
//       
//       for (const HostPtr &h : hosts_list)
//       {
//         if (h->workgroupName() == host->workgroupName() && h->hostName() == host->hostName())
//         {
//           found = true;
//           break;
//         }
//         else
//         {
//           continue;
//         }        
//       }
//       
//       if (!found)
//       {
//         QList<SharePtr> obsoleteShares = sharedResources(host);
//         QListIterator<SharePtr> sIt(obsoleteShares);
//         
//         while (sIt.hasNext())
//         {
//           removeShare(sIt.next());
//         }
//         
//         removeHost(host);
//       }
//       else
//       {
//         // Do nothing
//       }
//     }
//     
//     //
//     // Add new hosts and update existing ones
//     //
//     for (const HostPtr &host : hosts_list)
//     {
//       if (host->hostName() == workgroup->masterBrowserName())
//       {
//         host->setIsMasterBrowser(true);
//       }
//       else
//       {
//         host->setIsMasterBrowser(false);
//       }
//       
//       if (!findHost(host->hostName(), host->workgroupName()))
//       {
//         addHost(host);
//         d->haveNewHosts = true;
//       }
//       else
//       {
//         updateHost(host);
//       }
//     }
//     
//     emit hosts(workgroup);
//   }
//   else
//   {
//     // Do nothing
//   }
// }


void Smb4KScanner::slotShares(const HostPtr &host, const QList<SharePtr> &shares_list)
{
  Q_ASSERT(host);
  
  if (host && !shares_list.isEmpty())
  {
    //
    // Remove obsolete shares
    //
    QList<SharePtr> sharedRes = sharedResources(host);
    QListIterator<SharePtr> sIt(sharedRes);
    
    while (sIt.hasNext())
    {
      SharePtr share = sIt.next();
      
      bool found = false;
      
      for (const SharePtr &s : shares_list)
      {
        if (s->workgroupName() == share->workgroupName() && s->unc() == share->unc())
        {
          found = true;
          break;
        }
        else
        {
          continue;
        }
      }
      
      if (!found || 
          (share->isHidden() && !Smb4KSettings::detectHiddenShares()) ||
          (share->isPrinter() && !Smb4KSettings::detectPrinterShares()))
      {
        removeShare(share);
      }
      else
      {
        // Do nothing
      }
    }
    
    //
    // Add new shares and update existing ones
    //
    for (const SharePtr &share : shares_list)
    {
      //
      // Process only those shares that the user wants to see
      //
      if (share->isHidden() && !Smb4KSettings::detectHiddenShares())
      {
        continue;
      }
      else
      {
        // Do nothing
      }
      
      if (share->isPrinter() && !Smb4KSettings::detectPrinterShares())
      {
        continue;
      }
      else
      {
        // Do nothing
      }
      
      //
      // Add the host's IP address and authentication information
      //
      HostPtr host = findHost(share->hostName(), share->workgroupName());
      
      if (host)
      {
        share->setHostIpAddress(host->ipAddress());
        share->setLogin(host->login());
        share->setPassword(host->password());
      }
      else
      {
        // Do nothing
      }
      
      //
      // Add or update the shares
      //      
      if (!findShare(share->unc(), share->workgroupName()))
      {
        addShare(share);
      }
      else
      {
        updateShare(share);
      }
    }
    
//     emit shares(host);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KScanner::slotProcessIPAddress(const HostPtr &host)
{
  Q_ASSERT(host);
  
  //
  // Find the host in the hosts list
  //
  HostPtr knownHost = findHost(host->hostName(), host->workgroupName());

  //
  // Update the host, its shares, and, if applicable, the workgroup master browser's IP address
  //
  if (knownHost)
  {
    // Update host
    knownHost->setIpAddress(host->ipAddress());
    
    // Update shares
    QList<SharePtr> shares = sharedResources(knownHost);
    
    for (const SharePtr &share : shares)
    {
      share->setHostIpAddress(host->ipAddress());
    }
    
    // Update workgroup master browser's IP address
    if (knownHost->isMasterBrowser())
    {
      WorkgroupPtr workgroup = findWorkgroup(knownHost->workgroupName());
      
      if (workgroup && !workgroup->hasMasterBrowserIpAddress() && workgroup->masterBrowserName() == knownHost->hostName())
      {
        workgroup->setMasterBrowserIpAddress(knownHost->ipAddress());
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

  emit ipAddress(knownHost);
}


void Smb4KScanner::slotOnlineStateChanged(bool online)
{
  if (online)
  {
    //
    // Start the scanning of the network neighborhood
    //
    startScanning();
  }
  else
  {
    //
    // Abort all actions currently performed
    //
    abortAll();
    
    //
    // Kill the timer
    //
    killTimer(d->timerId);
    d->timerId = 0;
    
    //
    // Clear the list of periodic jobs, if necessary
    //
    if (Smb4KSettings::periodicScanning())
    {
      d->periodicJobs.clear();
    }
    else
    {
      // Do nothing
    }
  }
}

