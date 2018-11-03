/***************************************************************************
    This class provides the interface to the libsmbclient library.
                             -------------------
    begin                : Sa Oct 20 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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
#include "smb4kclient.h"
#include "smb4kclient_p.h"
#include "smb4khardwareinterface.h"
#include "smb4ksettings.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kbasicnetworkitem.h"
#include "smb4kglobal.h"

// Qt includes
#include <QUdpSocket>
#include <QHostAddress>
#include <QTest>
#include <QCoreApplication>

#define TIMER_INTERVAL 250

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KClientStatic, p);


Smb4KClient::Smb4KClient(QObject* parent) 
: KCompositeJob(parent), d(new Smb4KClientPrivate)
{
  //
  // Connections
  // 
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
}


Smb4KClient::~Smb4KClient()
{
}


Smb4KClient *Smb4KClient::self()
{
  return &p->instance;
}


void Smb4KClient::start()
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


bool Smb4KClient::isRunning()
{
  return hasSubjobs();
}


void Smb4KClient::abort()
{
  QListIterator<KJob *> it(subjobs());
    
  while (it.hasNext())
  {
    it.next()->kill(KJob::EmitResult);
  }
}


void Smb4KClient::lookupDomains()
{
  //
  // Send Wakeup-On-LAN packages
  // 
  if (Smb4KSettings::enableWakeOnLAN())
  {
    QList<OptionsPtr> wakeOnLanEntries = Smb4KCustomOptionsManager::self()->wakeOnLanEntries();
    
    if (!wakeOnLanEntries.isEmpty())
    {
      NetworkItemPtr item = NetworkItemPtr(new Smb4KBasicNetworkItem());
      emit aboutToStart(item, WakeUp);
      
      QUdpSocket *socket = new QUdpSocket(this);
      
      for (int i = 0; i < wakeOnLanEntries.size(); ++i)
      {
        if (wakeOnLanEntries.at(i)->wolSendBeforeNetworkScan())
        {
          QHostAddress addr;
          
          if (wakeOnLanEntries.at(i)->hasIpAddress())
          {
            addr.setAddress(wakeOnLanEntries.at(i)->ipAddress());
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
          QStringList parts = wakeOnLanEntries.at(i)->macAddress().split(':', QString::SkipEmptyParts);
          
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
      
      emit finished(item, WakeUp);
      item.clear();
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
  // Emit the aboutToStart() signal
  // 
  NetworkItemPtr item = NetworkItemPtr(new Smb4KBasicNetworkItem(Network));
  item->setUrl(QUrl("smb://"));
  emit aboutToStart(item, LookupDomains);
  
  // 
  // Create the job
  // 
  Smb4KClientJob *job = new Smb4KClientJob(this);
  job->setNetworkItem(item);
  
  connect(job, SIGNAL(result(KJob*)), this, SLOT(slotJobFinished(KJob*)));
  
  //
  // Clear the pointer
  // 
  item.clear();
  
  //
  // Set the busy cursor
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
  // Add the job to the subjobs
  //
  addSubjob(job);

  //
  // Start the job
  // 
  job->start();
}


void Smb4KClient::lookupDomainMembers(WorkgroupPtr workgroup)
{
  //
  // Emit the aboutToStart() signal
  // 
  emit aboutToStart(workgroup, LookupDomainMembers);
  
  // 
  // Create the job
  // 
  Smb4KClientJob *job = new Smb4KClientJob(this);
  job->setNetworkItem(workgroup);
  
  connect(job, SIGNAL(result(KJob*)), this, SLOT(slotJobFinished(KJob*)));
  
  //
  // Set the busy cursor
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
  // Add the job to the subjobs
  //
  addSubjob(job);

  //
  // Start the job
  // 
  job->start();
}


void Smb4KClient::lookupShares(HostPtr host)
{
  //
  // Emit the aboutToStart() signal
  // 
  emit aboutToStart(host, LookupShares);
  
  // 
  // Create the job
  // 
  Smb4KClientJob *job = new Smb4KClientJob(this);
  job->setNetworkItem(host);
  
  connect(job, SIGNAL(result(KJob*)), this, SLOT(slotJobFinished(KJob*)));
  
  //
  // Set the busy cursor
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
  // Add the job to the subjobs
  //
  addSubjob(job);

  //
  // Start the job
  // 
  job->start();  
}


void Smb4KClient::processErrors(KJob *job)
{
  switch (job->error())
  {
    case Smb4KClientJob::AccessDeniedError:
    {
      qDebug() << "Authentication error";
      break;
    }
    default:
    {
      break;
    }
  }
}



void Smb4KClient::processWorkgroups(Smb4KClientJob *job)
{
  //
  // Remove obsolete workgroups and their members
  //
  QListIterator<WorkgroupPtr> wIt(workgroupsList());
  
  while (wIt.hasNext())
  {
    WorkgroupPtr workgroup = wIt.next();
    
    bool found = false;
    
    for (const WorkgroupPtr &w : job->workgroups())
    {
      if (w->workgroupName() == workgroup->workgroupName())
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
      QList<HostPtr> obsoleteHosts = workgroupMembers(workgroup);
      QListIterator<HostPtr> hIt(obsoleteHosts);
      
      while (hIt.hasNext())
      {
        removeHost(hIt.next());
      }
      
      removeWorkgroup(workgroup);
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Add new workgroups and update existing ones.
  // 
  for (const WorkgroupPtr &workgroup : job->workgroups())
  {
    if (!findWorkgroup(workgroup->workgroupName()))
    {
      addWorkgroup(workgroup);
      
      // Since this is a new workgroup, no master browser is present.
      HostPtr masterBrowser = HostPtr(new Smb4KHost());
      masterBrowser->setWorkgroupName(workgroup->workgroupName());
      masterBrowser->setHostName(workgroup->masterBrowserName());
      masterBrowser->setIpAddress(workgroup->masterBrowserIpAddress());
      masterBrowser->setIsMasterBrowser(true);
      
      addHost(masterBrowser);
    }
    else
    {
      updateWorkgroup(workgroup);
      
      // Check if the master browser changed
      QList<HostPtr> members = workgroupMembers(workgroup);
      
      for (const HostPtr &host : members)
      {
        if (workgroup->masterBrowserName() == host->hostName())
        {
          host->setIsMasterBrowser(true);
          
          if (!host->hasIpAddress() && workgroup->hasMasterBrowserIpAddress())
          {
            host->setIpAddress(workgroup->masterBrowserIpAddress());
          }
          else
          {
            // Do nothing
          }          
        }
        else
        {
          host->setIsMasterBrowser(false);
        }
      }
    }
  }
  
  emit workgroups();
}


void Smb4KClient::processHosts(Smb4KClientJob *job)
{
  // 
  // Get the workgroup pointer
  // 
  WorkgroupPtr workgroup = job->networkItem().staticCast<Smb4KWorkgroup>();

  //
  // Remove obsolete workgroup members
  // 
  QList<HostPtr> members = workgroupMembers(workgroup);
  QListIterator<HostPtr> hIt(members);
    
  while (hIt.hasNext())
  {
    HostPtr host = hIt.next();
      
    bool found = false;
      
    for (const HostPtr &h : job->hosts())
    {
      if (h->workgroupName() == host->workgroupName() && h->hostName() == host->hostName())
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
      QList<SharePtr> obsoleteShares = sharedResources(host);
      QListIterator<SharePtr> sIt(obsoleteShares);
        
      while (sIt.hasNext())
      {
        removeShare(sIt.next());
      }
        
      removeHost(host);
    }
    else
    {
      // Do nothing
    }
  }
    
  //
  // Add new hosts and update existing ones
  //
  for (const HostPtr &host : job->hosts())
  {
    if (host->hostName() == workgroup->masterBrowserName())
    {
      host->setIsMasterBrowser(true);
    }
    else
    {
      host->setIsMasterBrowser(false);
    }
      
    if (!findHost(host->hostName(), host->workgroupName()))
    {
      addHost(host);
    }
    else
    {
      updateHost(host);
    }
  }
    
  emit hosts(workgroup);
}


void Smb4KClient::processShares(Smb4KClientJob *job)
{
  //
  // Get the host pointer
  // 
  HostPtr host = job->networkItem().staticCast<Smb4KHost>();

  //
  // Remove obsolete shares
  //
  QList<SharePtr> sharedRes = sharedResources(host);
  QListIterator<SharePtr> sIt(sharedRes);
    
  while (sIt.hasNext())
  {
    SharePtr share = sIt.next();
      
    bool found = false;
      
    for (const SharePtr &s : job->shares())
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
      
    if (!found || (share->isHidden() && !Smb4KSettings::detectHiddenShares()) || (share->isPrinter() && !Smb4KSettings::detectPrinterShares()))
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
  for (const SharePtr &share : job->shares())
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
    
  emit shares(host);
}


void Smb4KClient::slotStartJobs()
{
  //
  // Disconnect from Smb4KHardwareInterface::networkConfigUpdated() signal,
  // otherwise we get unwanted periodic scanning.
  //
  disconnect(Smb4KHardwareInterface::self(), SIGNAL(networkConfigUpdated()), this, SLOT(slotStartJobs()));
  
  //
  // Lookup domains as the first step
  // 
  lookupDomains();
}


void Smb4KClient::slotJobFinished(KJob *job)
{
  //
  // Get the client job
  // 
  Smb4KClientJob *clientJob = qobject_cast<Smb4KClientJob *>(job);
  
  //
  // Define a network item pointer and the process value for the 
  // finished() signal.
  // 
  NetworkItemPtr item;
  Smb4KGlobal::Process process;
  
  //
  // Get the result from the query and process it
  // 
  if (clientJob)
  {
    if (clientJob->error() == 0)
    {
      switch (clientJob->networkItem()->type())
      {
        case Network:
        {
          // Process the discovered workgroups
          processWorkgroups(clientJob);
          
          // Set the network item and the process value
          item = NetworkItemPtr(new Smb4KBasicNetworkItem(clientJob->networkItem()->type()));
          process = LookupDomains;
          
          break;
        }
        case Workgroup:
        {
          // Process the discovered workgroup members
          processHosts(clientJob);
          
          // Set the network item and the process value
          WorkgroupPtr workgroup = WorkgroupPtr(new Smb4KWorkgroup());
          workgroup->setWorkgroupName(clientJob->workgroup());
          item = workgroup.staticCast<Smb4KBasicNetworkItem>();
          process = LookupDomainMembers;
          
          break;
        }
        case Host:
        {
          // Process the discovered shares
          processShares(clientJob);
          
          // Set the network item and the process value
          HostPtr host = HostPtr(new Smb4KHost());
          host->setUrl(clientJob->networkItem()->url());
          host->setWorkgroupName(clientJob->workgroup());
          item = host.staticCast<Smb4KBasicNetworkItem>();
          process = LookupShares;
          
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
      processErrors(clientJob);
    }
  }
  else
  {
    // Do nothing
  }

  //
  // Remove the job
  //
  removeSubjob(job);
  
  //
  // Emit the finished signal
  // 
  finished(item, process);
  
  //
  // Clear the network item pointer
  // 
  item.clear();
  
  //
  // Restore the cursor
  //
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KClient::slotAboutToQuit()
{
  abort();
}





