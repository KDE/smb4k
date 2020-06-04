/***************************************************************************
    This class provides the interface to the libsmbclient library.
                             -------------------
    begin                : Sa Oct 20 2018
    copyright            : (C) 2018-2020 by Alexander Reinholdt
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

// application specific includes
#include "smb4kclient.h"
#include "smb4kclient_p.h"
#include "smb4khardwareinterface.h"
#include "smb4ksettings.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kbasicnetworkitem.h"
#include "smb4kglobal.h"
#include "smb4khomesshareshandler.h"
#include "smb4kwalletmanager.h"
#include "smb4knotification.h"

// Qt includes
#include <QUdpSocket>
#include <QHostAddress>
#include <QTest>
#include <QApplication>

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
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkSessionInitialized()), this, SLOT(slotStartJobs()));
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
  job->setProcess(LookupDomains);
  
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

  //
  // Add the job to the subjobs
  //
  addSubjob(job);

  //
  // Start the job
  // 
  job->start();
}


void Smb4KClient::lookupDomainMembers(const WorkgroupPtr &workgroup)
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
  job->setProcess(LookupDomainMembers);
  
  //
  // Set the busy cursor
  //
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::setOverrideCursor(Qt::BusyCursor);
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


void Smb4KClient::lookupShares(const HostPtr &host)
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
  job->setProcess(LookupShares);
  
  //
  // Set the busy cursor
  //
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::setOverrideCursor(Qt::BusyCursor);
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


void Smb4KClient::lookupFiles(const NetworkItemPtr &item)
{
  //
  // Check that the network item has the correct type and process it.
  // 
  if (item->type() == Share || item->type() == Directory)
  {
    //
    // Emit the aboutToStart() signal
    // 
    emit aboutToStart(item, LookupFiles);
    
    // 
    // Create the job
    // 
    Smb4KClientJob *job = new Smb4KClientJob(this);
    job->setNetworkItem(item);
    job->setProcess(LookupFiles);
    
    //
    // Set the busy cursor
    //
    if (!hasSubjobs() && modifyCursor())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
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
}


void Smb4KClient::printFile(const SharePtr& share, const KFileItem& fileItem, int copies)
{
  //
  // Emit the aboutToStart() signal
  // 
  emit aboutToStart(share, PrintFile);
  
  // 
  // Create the job
  // 
  Smb4KClientJob *job = new Smb4KClientJob(this);
  job->setNetworkItem(share);
  job->setPrintFileItem(fileItem);
  job->setPrintCopies(copies);
  job->setProcess(PrintFile);
  
  //
  // Set the busy cursor
  //
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::setOverrideCursor(Qt::BusyCursor);
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


void Smb4KClient::search(const QString& item)
{
  //
  // Create empty basic network item
  // 
  NetworkItemPtr networkItem = NetworkItemPtr(new Smb4KBasicNetworkItem());
  
  //
  // Emit the aboutToStart() signal
  // 
  emit aboutToStart(networkItem, NetworkSearch);
  
  //
  // Before doing the search, lookup all domains, servers and shares in the 
  // network neighborhood.
  // 
  lookupDomains();
  
  while(isRunning())
  {
    QTest::qWait(50);
  }
  
  for (const WorkgroupPtr &workgroup : workgroupsList())
  {
    lookupDomainMembers(workgroup);
    
    while(isRunning())
    {
      QTest::qWait(50);
    }
  }
  
  for (const HostPtr &host : hostsList())
  {
    lookupShares(host);
    
    while(isRunning())
    {
      QTest::qWait(50);
    }
  }
  
  //
  // Do the actual search
  // 
  QList<SharePtr> results;
  
  for (const SharePtr &share : sharesList())
  {
    if (share->shareName().contains(item, Qt::CaseInsensitive))
    {
      results << share;
    }
  }
  
  //
  // Emit the search results
  // 
  emit searchResults(results);
  
  //
  // Emit the finished() signal
  // 
  emit finished(networkItem, NetworkSearch);
}



void Smb4KClient::openPreviewDialog(const SharePtr &share)
{
  //
  // Printer share check
  // 
  if (share->isPrinter())
  {        
    return;
  }
      
  //
  // 'homes' share check
  //
  if (share->isHomesShare())
  {
    Smb4KHomesSharesHandler::self()->specifyUser(share, true);
  }
    
  //
  // Start the preview dialog
  // 
  // First, check if a preview dialog has already been set up for this share 
  // and reuse it, if possible.
  // 
  QPointer<Smb4KPreviewDialog> dlg = 0;
  
  for (Smb4KPreviewDialog *p : d->previewDialogs)
  {
    if (share == p->share())
    {
      dlg = p;
    }
  }
  
  //
  // If there was no preview dialog present, create a new one
  // 
  if (!dlg)
  {
    dlg = new Smb4KPreviewDialog(share, QApplication::activeWindow());
    d->previewDialogs << dlg;
    
    //
    // Connections
    // 
    connect(dlg, SIGNAL(requestPreview(NetworkItemPtr)), this, SLOT(slotStartNetworkQuery(NetworkItemPtr)));
    connect(dlg, SIGNAL(aboutToClose(Smb4KPreviewDialog*)), this, SLOT(slotPreviewDialogClosed(Smb4KPreviewDialog*)));
    connect(dlg, SIGNAL(requestAbort()), this, SLOT(slotAbort()));
    connect(this, SIGNAL(files(QList<FilePtr>)), dlg, SLOT(slotPreviewResults(QList<FilePtr>)));
    connect(this, SIGNAL(aboutToStart(NetworkItemPtr,int)), dlg, SLOT(slotAboutToStart(NetworkItemPtr,int)));
    connect(this, SIGNAL(finished(NetworkItemPtr,int)), dlg, SLOT(slotFinished(NetworkItemPtr,int)));
  }
  
  //
  // Show the preview dialog
  // 
  if (!dlg->isVisible())
  {
    dlg->setVisible(true);
  }
}


void Smb4KClient::openPrintDialog(const SharePtr& share)
{
  //
  // Printer share check
  // 
  if (!share->isPrinter())
  {        
    return;
  }
  
  //
  // Start the print dialog
  // 
  // First, check if a print dialog has already been set up for this share 
  // and reuse it, if possible.
  // 
  QPointer<Smb4KPrintDialog> dlg = 0;
  
  for (Smb4KPrintDialog *p : d->printDialogs)
  {
    if (share == p->share())
    {
      dlg = p;
    }
  }
  
  //
  // If there was no print dialog present, create a new one
  // 
  if (!dlg)
  {
    Smb4KWalletManager::self()->readAuthInfo(share);
    
    dlg = new Smb4KPrintDialog(share, QApplication::activeWindow());
    d->printDialogs << dlg;
    
    connect(dlg, SIGNAL(printFile(SharePtr,KFileItem,int)), this, SLOT(slotStartPrinting(SharePtr,KFileItem,int)));
    connect(dlg, SIGNAL(aboutToClose(Smb4KPrintDialog*)), this, SLOT(slotPrintDialogClosed(Smb4KPrintDialog*)));
  }
  
  //
  // Show the preview dialog
  // 
  if (!dlg->isVisible())
  {
    dlg->setVisible(true);
  }
}


void Smb4KClient::processErrors(Smb4KClientJob *job)
{
  switch (job->error())
  {
    case Smb4KClientJob::AccessDeniedError:
    {
      switch (job->networkItem()->type())
      {
        case Host:
        {
          if (Smb4KWalletManager::self()->showPasswordDialog(job->networkItem()))
          {
            lookupShares(job->networkItem().staticCast<Smb4KHost>());
          }
          
          break;
        }
        case Share:
        {
          if (Smb4KWalletManager::self()->showPasswordDialog(job->networkItem()))
          {
            if (job->process() == Smb4KGlobal::PrintFile)
            {
              printFile(job->networkItem().staticCast<Smb4KShare>(), job->printFileItem(), job->printCopies());
            }
            else
            {
              lookupFiles(job->networkItem().staticCast<Smb4KShare>());
            }
          }
          
          break;
        }
        case Directory:
        case File:
        {
          FilePtr file = job->networkItem().staticCast<Smb4KFile>();
          
          SharePtr share = SharePtr(new Smb4KShare());
          share->setWorkgroupName(file->workgroupName());
          share->setHostName(file->hostName());
          share->setShareName(file->shareName());
          share->setLogin(file->login());
          share->setPassword(file->password());
          
          if (Smb4KWalletManager::self()->showPasswordDialog(share))
          {
            file->setLogin(share->login());
            file->setPassword(share->password());
            
            lookupFiles(file);
          }

          break;
        }
        default:
        {
          qDebug() << "Authentication error. URL:" << job->networkItem()->url();
          break;
        }
      }
      
      break;
    }
    default:
    {
      Smb4KNotification::networkCommunicationFailed(job->errorText());
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
      if (s->workgroupName() == share->workgroupName() && s->url().matches(share->url(), QUrl::RemoveUserInfo|QUrl::RemovePort))
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
      
    if (share->isPrinter() && !Smb4KSettings::detectPrinterShares())
    {
      continue;
    }
      
    //
    // Add or update the shares
    //
    if (!findShare(share->url(), share->workgroupName()))
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


void Smb4KClient::processFiles(Smb4KClientJob *job)
{
  QList<FilePtr> list;
  
  for (const FilePtr &f : job->files())
  {
    if (f->isHidden() && !Smb4KSettings::previewHiddenItems())
    {
      continue;
    }
    
    list << f;
  }  
  
  emit files(list);
}


void Smb4KClient::slotStartJobs()
{
  //
  // Disconnect from Smb4KHardwareInterface
  //
  disconnect(Smb4KHardwareInterface::self(), SIGNAL(networkSessionInitialized()), this, SLOT(slotStartJobs()));

  //
  // Lookup domains as the first step
  // 
  if (Smb4KHardwareInterface::self()->isOnline())
  {
    lookupDomains();
  }
}


void Smb4KClient::slotResult(KJob *job)
{
  //
  // Get the client job
  // 
  Smb4KClientJob *clientJob = qobject_cast<Smb4KClientJob *>(job);
  
  //
  // Define a network item pointer and the process value for the 
  // finished() signal.
  // 
  NetworkItemPtr item = clientJob->networkItem();
  Smb4KGlobal::Process process = clientJob->process();
  
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
          break;
        }
        case Workgroup:
        {
          qDebug() << "Processing the hosts ...";
          // Process the discovered workgroup members
          processHosts(clientJob);
          break;
        }
        case Host:
        {
          // Process the discovered shares
          processShares(clientJob);
          break;
        }
        case Share:
        case Directory:
        {
          // Process the discoveres files and directories
          processFiles(clientJob);
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

  //
  // Remove the job
  //
  removeSubjob(job);
  
  //
  // Emit the finished signal
  // 
  emit finished(item, process);
  
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
}


void Smb4KClient::slotAboutToQuit()
{
  abort();
}


void Smb4KClient::slotStartNetworkQuery(NetworkItemPtr item)
{
  //
  // Look up files
  //
  lookupFiles(item);
}


void Smb4KClient::slotPreviewDialogClosed(Smb4KPreviewDialog *dialog)
{
  //
  // Remove the preview dialog from the list
  // 
  if (dialog)
  {
    // Find the dialog in the list and take it from the list.
    // It will automatically be deleted on close, so there is
    // no need to delete the dialog here.
    int i = d->previewDialogs.indexOf(dialog);
    d->previewDialogs.takeAt(i);
  }
}


void Smb4KClient::slotAbort()
{
  abort();
}


void Smb4KClient::slotStartPrinting(const SharePtr& printer, const KFileItem& fileItem, int copies)
{
  //
  // Start printing
  // 
  printFile(printer, fileItem, copies);
}


void Smb4KClient::slotPrintDialogClosed(Smb4KPrintDialog* dialog)
{
  //
  // Remove the print dialog from the list
  // 
  if (dialog)
  {
    // Find the dialog in the list and take it from the list.
    // It will automatically be deleted on close, so there is
    // no need to delete the dialog here.
    int i = d->printDialogs.indexOf(dialog);
    d->printDialogs.takeAt(i);
  }
}





