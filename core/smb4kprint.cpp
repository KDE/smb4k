/***************************************************************************
    smb4kprint  -  The (new) printing core class.
                             -------------------
    begin                : Son Feb 20 2011
    copyright            : (C) 2011-2015 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kprint.h"
#include "smb4kprint_p.h"
#include "smb4kshare.h"
#include "smb4knotification.h"
#include "smb4kauthinfo.h"
#include "smb4kwalletmanager.h"
#include "smb4kglobal.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>

using namespace Smb4KGlobal;


Q_GLOBAL_STATIC(Smb4KPrintStatic, p);


Smb4KPrint::Smb4KPrint(QObject *parent) : KCompositeJob(parent)
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


Smb4KPrint::~Smb4KPrint()
{
}


Smb4KPrint *Smb4KPrint::self()
{
  return &p->instance;
}


void Smb4KPrint::print(Smb4KShare *printer, QWidget *parent)
{
  // Check that we actually have a printer share
  if (!printer->isPrinter())
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  // Get the authentication information.
  Smb4KWalletManager::self()->readAuthInfo(printer);
  
  // Create a new job and add it to the subjobs 
  Smb4KPrintJob *job = new Smb4KPrintJob(this);
  job->setObjectName(QString("PrintJob_%1").arg(printer->unc()));
  job->setupPrinting(printer, parent);
  
  connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
  connect(job, SIGNAL(authError(Smb4KPrintJob*)), SLOT(slotAuthError(Smb4KPrintJob*)));
  connect(job, SIGNAL(aboutToStart(Smb4KShare*)), SIGNAL(aboutToStart(Smb4KShare*)));
  connect(job, SIGNAL(finished(Smb4KShare*)), SIGNAL(finished(Smb4KShare*)));
  
  addSubjob(job);
  
  job->start();
}


bool Smb4KPrint::isRunning()
{
  return hasSubjobs();
}


bool Smb4KPrint::isRunning(Smb4KShare *share)
{
  bool running = false;

  for (int i = 0; i < subjobs().size(); ++i)
  {
    if (QString::compare(QString("PrintJob_%1").arg(share->unc()), subjobs().at(i)->objectName()) == 0)
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


void Smb4KPrint::abortAll()
{
  for (int i = 0; i < subjobs().size(); ++i)
  {
    subjobs().at(i)->kill(KJob::EmitResult);
  }
}


void Smb4KPrint::abort(Smb4KShare *share)
{
  for (int i = 0; i < subjobs().size(); ++i)
  {
    if (QString::compare(QString("PrintJob_%1").arg(share->unc()), subjobs().at(i)->objectName()) == 0)
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


void Smb4KPrint::start()
{
  QTimer::singleShot(0, this, SLOT(slotStartJobs()));
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPrint::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KPrint::slotJobFinished(KJob *job)
{
  removeSubjob(job);
}


void Smb4KPrint::slotAuthError(Smb4KPrintJob *job)
{
  if (Smb4KWalletManager::self()->showPasswordDialog(job->printer(), job->parentWidget()))
  {
    print(job->printer(), job->parentWidget());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPrint::slotAboutToQuit()
{
  abortAll();
}

