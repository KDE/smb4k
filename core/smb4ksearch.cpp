/***************************************************************************
    This class does custom searches
                             -------------------
    begin                : Tue Mar 08 2011
    copyright            : (C) 2011-2017 by Alexander Reinholdt
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
#include "smb4ksearch.h"
#include "smb4ksearch_p.h"
#include "smb4kauthinfo.h"
#include "smb4ksettings.h"
#include "smb4kglobal.h"
#include "smb4kworkgroup.h"
#include "smb4kwalletmanager.h"
#include "smb4khomesshareshandler.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QApplication>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KSearchStatic, p);


Smb4KSearch::Smb4KSearch(QObject *parent)
: KCompositeJob(parent), d(new Smb4KSearchPrivate)
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
  
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}


Smb4KSearch::~Smb4KSearch()
{
}


Smb4KSearch *Smb4KSearch::self()
{
  return &p->instance;
}


void Smb4KSearch::search(const QString &string, QWidget *parent)
{
  //
  // Abort if the search string is empty
  // 
  if (string.trimmed().isEmpty())
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // Clear the previous search results
  // 
  if (!searchResults().isEmpty())
  {
    clearSearchResults();
  }
  else
  {
    // Do nothing
  }

  // 
  // Get authentication information for the master browser, if necessary
  // 
  HostPtr masterBrowser;

  if (Smb4KSettings::masterBrowsersRequireAuth())
  {
    // smbtree will be used and the master browser requires authentication. 
    // Lookup the authentication information for the master browser.
    WorkgroupPtr workgroup = findWorkgroup(Smb4KSettings::domainName());
    HostPtr host;

    if (workgroup)
    {
      host = findHost(workgroup->masterBrowserName(), workgroup->workgroupName());

      if (host)
      {
        // Copy host item
        masterBrowser = host;
        
        // Authentication information
        Smb4KWalletManager::self()->readAuthInfo(masterBrowser);
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

  // 
  // Create a new job and add it to the subjobs
  // 
  Smb4KSearchJob *job = new Smb4KSearchJob(this);
  job->setObjectName(QString("SearchJob_%1").arg(string));
  job->setupSearch(string, masterBrowser, parent);

  //
  // Connections
  // 
  connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
  connect(job, SIGNAL(authError(Smb4KSearchJob*)), SLOT(slotAuthError(Smb4KSearchJob*)));
  connect(job, SIGNAL(result(SharePtr)), SLOT(slotProcessSearchResult(SharePtr)));
  connect(job, SIGNAL(aboutToStart(QString)), SIGNAL(aboutToStart(QString)));
  connect(job, SIGNAL(finished(QString)), SIGNAL(finished(QString)));

  //
  // Modify the cursor, if wanted
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


bool Smb4KSearch::isRunning()
{
  return hasSubjobs();
}


bool Smb4KSearch::isRunning(const QString &string)
{
  bool running = false;

  for (int i = 0; i < subjobs().size(); ++i)
  {
    if (QString::compare(QString("SearchJob_%1").arg(string), subjobs().at(i)->objectName()) == 0)
    {
      running = true;
      break;
    }
    else
    {
      continue;
    }
  }

  return running;
}



void Smb4KSearch::abortAll()
{
  QListIterator<KJob *> it(subjobs());
    
  while (it.hasNext())
  {
    it.next()->kill(KJob::EmitResult);
  }
}


void Smb4KSearch::abort(const QString &string)
{
  for (int i = 0; i < subjobs().size(); ++i)
  {
    if (QString::compare(QString("SearchJob_%1").arg(string), subjobs().at(i)->objectName()) == 0)
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


void Smb4KSearch::start()
{
  QTimer::singleShot(0, this, SLOT(slotStartJobs()));
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSearch::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KSearch::slotJobFinished(KJob *job)
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


void Smb4KSearch::slotAuthError(Smb4KSearchJob *job)
{
  if (Smb4KWalletManager::self()->showPasswordDialog(job->masterBrowser(), job->parentWidget()))
  {
    search(job->searchString(), job->parentWidget());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSearch::slotProcessSearchResult(const SharePtr &share)
{
  if (share)
  {
    //
    // Add the IP address of the host, if it is necessary and known
    // 
    if (!share->hasHostIpAddress())
    {
      HostPtr host = findHost(share->hostName(), share->workgroupName());
      
      if (host)
      {
        share->setHostIpAddress(host->ipAddress());
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
    // Add the search result
    // 
    addSearchResult(share);

    //
    // Emit the search result
    // 
    emit result(share);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSearch::slotAboutToQuit()
{
  abortAll();
}

