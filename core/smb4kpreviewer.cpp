/***************************************************************************
    This class queries a remote share for a preview
                             -------------------
    begin                : Sa Mär 05 2011
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kpreviewer.h"
#include "smb4kpreviewer_p.h"
#include "smb4kwalletmanager.h"
#include "smb4kshare.h"
#include "smb4kauthinfo.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KPreviewerStatic, p);


Smb4KPreviewer::Smb4KPreviewer(QObject *parent)
: KCompositeJob(parent), d(new Smb4KPreviewerPrivate)
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


Smb4KPreviewer::~Smb4KPreviewer()
{
}


Smb4KPreviewer *Smb4KPreviewer::self()
{
  return &p->instance;
}


void Smb4KPreviewer::preview(Smb4KShare *share, QWidget *parent)
{
  if (share->isPrinter())
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  // Process homes shares.
  if(share->isHomesShare())
  {
    if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true, parent))
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
    // Do nothing
  }

  // Check if a preview dialog has already been set up 
  // for this share and reuse it, if appropriate.
  Smb4KPreviewDialog *dlg = 0;
  
  for (int i = 0; i < d->dialogs.size(); ++i)
  {
    if (share == d->dialogs.at(i)->share())
    {
      dlg = d->dialogs.at(i);
    }
    else
    {
      // Do nothing
    }
  }

  if (!dlg)
  {
    // Create the preview dialog..
    dlg = new Smb4KPreviewDialog(share, parent);
    connect(dlg,  SIGNAL(aboutToClose(Smb4KPreviewDialog*)),
            this, SLOT(slotDialogClosed(Smb4KPreviewDialog*)));
    connect(dlg,  SIGNAL(requestPreview(Smb4KShare*,QUrl,QWidget*)),
            this, SLOT(slotAcquirePreview(Smb4KShare*,QUrl,QWidget*)));
    connect(this, SIGNAL(aboutToStart(Smb4KShare*,QUrl)),
            dlg,  SLOT(slotAboutToStart(Smb4KShare*,QUrl)));
    connect(this, SIGNAL(finished(Smb4KShare*,QUrl)),
            dlg,  SLOT(slotFinished(Smb4KShare*,QUrl)));
    connect(dlg,  SIGNAL(abortPreview(Smb4KShare*)),
            this, SLOT(slotAbortPreview(Smb4KShare*)));
    d->dialogs.append(dlg);
  }
  else
  {
    // Do nothing
  }
  
  if (!dlg->isVisible())
  {
    dlg->setVisible(true);
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KPreviewer::isRunning()
{
  return hasSubjobs();
}


bool Smb4KPreviewer::isRunning(Smb4KShare *share)
{
  bool running = false;
  QString unc;
  
  if (!share->isHomesShare())
  {
    unc = share->unc();
  }
  else
  {
    unc = share->homeUNC();
  }

  for (int i = 0; i < subjobs().size(); ++i)
  {
    if (QString::compare(QString("PreviewJob_%1").arg(unc), subjobs().at(i)->objectName()) == 0)
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


void Smb4KPreviewer::abortAll()
{
  QListIterator<KJob *> it(subjobs());
    
  while (it.hasNext())
  {
    it.next()->kill(KJob::EmitResult);
  }
}


void Smb4KPreviewer::abort(Smb4KShare *share)
{
  QString unc;
  
  if (!share->isHomesShare())
  {
    unc = share->unc();
  }
  else
  {
    unc = share->homeUNC();
  }
  
  for (KJob *job : subjobs())
  {
    if (QString::compare(QString("PreviewJob_%1").arg(unc), job->objectName()) == 0)
    {
      job->kill(KJob::EmitResult);
      break;
    }
    else
    {
      continue;
    }
  }
}


void Smb4KPreviewer::start()
{
  QTimer::singleShot(0, this, SLOT(slotStartJobs()));
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPreviewer::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KPreviewer::slotJobFinished(KJob *job)
{
  disconnect(job);
  removeSubjob(job);
}


void Smb4KPreviewer::slotAuthError(Smb4KPreviewJob *job)
{
  // To avoid a crash here because after the password dialog closed
  // the job is gone, immediately get the needed data.
  Smb4KShare *share = job->share();
  QWidget *parent   = job->parentWidget();
  QUrl location     = job->location();
  
  if (Smb4KWalletManager::self()->showPasswordDialog(share, parent))
  {
    slotAcquirePreview(share, location, parent);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewer::slotDialogClosed(Smb4KPreviewDialog *dialog)
{
  if (dialog)
  {
    // Find the dialog in the list and take it from the list.
    // It will automatically be deleted on close, so there is
    // no need to delete the dialog here.
    int i = d->dialogs.indexOf(dialog);
    d->dialogs.takeAt(i);
  }
  else
  {
    qDebug() << "Dialog already gone.";
  }
}


void Smb4KPreviewer::slotAcquirePreview(Smb4KShare *share, const QUrl &url, QWidget *parent)
{
  // Get the authentication information
  Smb4KWalletManager::self()->readAuthInfo(share);
  
  // Create a new job and add it to the subjobs
  Smb4KPreviewJob *job = new Smb4KPreviewJob(this);
  
  if (!share->isHomesShare())
  {
    job->setObjectName(QString("PreviewJob_%1").arg(share->unc()));
  }
  else
  {
    job->setObjectName(QString("PreviewJob_%1").arg(share->homeUNC()));
  }
  
  job->setupPreview(share, url, parent);

  connect(job,  SIGNAL(result(KJob*)),
          this, SLOT(slotJobFinished(KJob*)));
  connect(job,  SIGNAL(authError(Smb4KPreviewJob*)),
          this, SLOT(slotAuthError(Smb4KPreviewJob*)));
  connect(job,  SIGNAL(aboutToStart(Smb4KShare*,QUrl)),
          this, SIGNAL(aboutToStart(Smb4KShare*,QUrl)));
  connect(job,  SIGNAL(finished(Smb4KShare*,QUrl)),
          this, SIGNAL(finished(Smb4KShare*,QUrl)));

  // Get the preview dialog, so that the result of the query
  // can be sent.
  Smb4KPreviewDialog *dlg = 0;

  for (int i = 0; i < d->dialogs.size(); ++i)
  {
    if (d->dialogs.at(i) && d->dialogs.at(i)->share() == share)
    {
      dlg = d->dialogs[i];
      break;
    }
    else
    {
      continue;
    }
  }
  
  if (dlg)
  {
    connect(job, SIGNAL(preview(QUrl,QList<Smb4KPreviewFileItem>)),
            dlg, SLOT(slotDisplayPreview(QUrl,QList<Smb4KPreviewFileItem>)));
  }
  else
  {
    // Do nothing
  }

  addSubjob(job);

  job->start();
}


void Smb4KPreviewer::slotAbortPreview(Smb4KShare *share)
{
  abort(share);
}


void Smb4KPreviewer::slotAboutToQuit()
{
  abortAll();
}

