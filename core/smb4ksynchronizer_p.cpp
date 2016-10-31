/***************************************************************************
    This file contains private helper classes for the Smb4KSynchronizer 
    class.
                             -------------------
    begin                : Fr Okt 24 2008
    copyright            : (C) 2008-2016 by Alexander Reinholdt
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
#include "smb4ksynchronizer_p.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"
#include "smb4kglobal.h"
#include "smb4kshare.h"

// Qt includes
#include <QTimer>
#include <QPointer>
#include <QStandardPaths>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KCompletion/KLineEdit>
#include <KIOWidgets/KUrlCompletion>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KConfigGui/KWindowConfig>

using namespace Smb4KGlobal;


Smb4KSyncJob::Smb4KSyncJob(QObject *parent) : KJob(parent),
  m_started(false), m_share(0), m_parent_widget(0), m_process(0)
{
  setCapabilities(KJob::Killable);
  m_job_tracker = new KUiServerJobTracker(this);
}


Smb4KSyncJob::~Smb4KSyncJob()
{
}


void Smb4KSyncJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartSynchronization()));
}


void Smb4KSyncJob::setupSynchronization(Smb4KShare *share, QWidget *parent)
{
  Q_ASSERT(share);
  m_share = share;
  m_parent_widget = parent;
}


bool Smb4KSyncJob::doKill()
{
  if (m_process && m_process->state() != KProcess::NotRunning)
  {
    m_process->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KSyncJob::slotStartSynchronization()
{
  //
  // Find the shell command
  //
  QString rsync = QStandardPaths::findExecutable("rsync");

  if (rsync.isEmpty())
  {
    Smb4KNotification::commandNotFound("rsync");
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }
  
  //
  // The synchronization dialog
  //
  if (m_share)
  {
    // Show the user an URL input dialog.
    QPointer<Smb4KSynchronizationDialog> dlg = new Smb4KSynchronizationDialog(m_share, m_parent_widget);

    if (dlg->exec() == QDialog::Accepted)
    {
      // Create the destination directory if it does not already exits.
      if (!QFile::exists(dlg->destination().path()))
      {
        QDir syncDir(dlg->destination().path());

        if (!syncDir.mkpath(dlg->destination().path()))
        {
          Smb4KNotification::mkdirFailed(syncDir);
          emitResult();
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
      
      // Make sure that we have got the trailing slash present.
      // rsync is very picky regarding it.
      m_src = dlg->source();
      m_dest = dlg->destination();

      delete dlg;
    }
    else
    {
      delete dlg;
      emitResult();
      return;
    }
  }
  else
  {
    emitResult();
    return;
  }
  
  //
  // The command
  //
  QStringList command;
  command << rsync;
  command << "--progress";
  
  if (Smb4KSettings::archiveMode())
  {
    command << "--archive";
  }
  else
  {
    if (Smb4KSettings::recurseIntoDirectories())
    {
      command << "--recursive";
    }
    else
    {
      // Do nothing
    }

    if (Smb4KSettings::preserveSymlinks())
    {
      command << "--links";
    }
    else
    {
      // Do nothing
    }

    if (Smb4KSettings::preservePermissions())
    {
      command << "--perms";
    }
    else
    {
      // Do nothing
    }

    if (Smb4KSettings::preserveTimes())
    {
      command << "--times";
    }
    else
    {
      // Do nothing
    }

    if (Smb4KSettings::preserveGroup())
    {
      command << "--group";
    }
    else
    {
      // Do nothing
    }

    if (Smb4KSettings::preserveOwner())
    {
      command << "--owner";
    }
    else
    {
      // Do nothing
    }

    if (Smb4KSettings::preserveDevicesAndSpecials())
    {
      // Alias -D
      command << "--devices";
      command << "--specials";
    }
    else
    {
      // Do nothing
    }
  }
  
  if (Smb4KSettings::relativePathNames())
  {
    command << "--relative";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::omitDirectoryTimes())
  {
    command << "--omit-dir-times";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::noImpliedDirectories())
  {
    command << "--no-implied-dirs";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::updateTarget())
  {
    command << "--update";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::updateInPlace())
  {
    command << "--inplace";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::transferDirectories())
  {
    command << "--dirs";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::transformSymlinks())
  {
    command << "--copy-links";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::transformUnsafeSymlinks())
  {
    command << "--copy-unsafe-links";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::ignoreUnsafeSymlinks())
  {
    command << "--safe-links";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::preserveHardLinks())
  {
    command << "--hard-links";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::keepDirectorySymlinks())
  {
    command << "--keep-dirlinks";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::deleteExtraneous())
  {
    command << "--delete";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::removeSourceFiles())
  {
    command << "--remove-source-files";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::deleteBefore())
  {
    command << "--delete-before";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::deleteDuring())
  {
    command << "--delete-during";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::deleteAfter())
  {
    command << "--delete-after";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::deleteExcluded())
  {
    command << "--delete-excluded";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::ignoreErrors())
  {
    command << "--ignore-errors";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::forceDirectoryDeletion())
  {
    command << "--force";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::copyFilesWhole())
  {
    command << "--whole-file";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::efficientSparseFileHandling())
  {
    command << "--sparse";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::oneFileSystem())
  {
    command << "--one-file-system";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::updateExisting())
  {
    command << "--existing";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::ignoreExisting())
  {
    command << "--ignore-existing";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::delayUpdates())
  {
    command << "--delay-updates";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::compressData())
  {
    command << "--compress";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::makeBackups())
  {
    command << "--backup";

    if (Smb4KSettings::useBackupDirectory())
    {
      command << QString("--backup-dir=%1").arg(Smb4KSettings::backupDirectory().path());
    }
    else
    {
      // Do nothing
    }

    if (Smb4KSettings::useBackupSuffix())
    {
      command << QString("--suffix=%1").arg(Smb4KSettings::backupSuffix());
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
  
  if (Smb4KSettings::useMaximumDelete())
  {
    command << QString("--max-delete=%1").arg(Smb4KSettings::maximumDeleteValue());
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useChecksum())
  {
    command << "--checksum";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useBlockSize())
  {
    command << QString("--block-size=%1").arg(Smb4KSettings::blockSize());
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useChecksumSeed())
  {
    command << QString("--checksum-seed=%1").arg(Smb4KSettings::checksumSeed());
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::keepPartial())
  {
    command << " --partial";

    if (Smb4KSettings::usePartialDirectory())
    {
      command << QString("--partial-dir=%1").arg(Smb4KSettings::partialDirectory().path());
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

  if (Smb4KSettings::useCVSExclude())
  {
    command << "--cvs-exclude";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useFFilterRule())
  {
    command << "-F";
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useFFFilterRule())
  {
    command << "-F";
    command << "-F";
  }
  else
  {
    // Do nothing
  }
  
  if (Smb4KSettings::useExcludePattern())
  {
    command << QString("--exclude=%1").arg(Smb4KSettings::excludePattern());
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useExcludeFrom())
  {
    command << QString("--exclude-from=%1").arg(Smb4KSettings::excludeFrom().path());
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useIncludePattern())
  {
    command << QString("--include=%1").arg(Smb4KSettings::includePattern());
  }
  else
  {
    // Do nothing
  }

  if (Smb4KSettings::useIncludeFrom())
  {
    command << QString("--include-from=%1").arg(Smb4KSettings::includeFrom().path());
  }
  else
  {
    // Do nothing
  }
  
  // Make sure that the trailling slash is present. rsync is very 
  // picky regarding it.
  QString source = m_src.path() + (!m_src.path().endsWith('/') ? "/" : "");
  QString destination = m_dest.path() + (!m_dest.path().endsWith('/') ? "/" : "");
    
  command << source;
  command << destination;
  
  //
  // The job tracker
  //
  m_job_tracker->registerJob(this);
  connect(this, SIGNAL(result(KJob*)), m_job_tracker, SLOT(unregisterJob(KJob*)));

  //
  // The process
  //
  m_process = new Smb4KProcess(this);
  m_process->setOutputChannelMode(KProcess::SeparateChannels);
  m_process->setProgram(command);
  
  connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(slotReadStandardOutput()));
  connect(m_process, SIGNAL(readyReadStandardError()),  SLOT(slotReadStandardError()));
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
  
  // Start the synchronization process
  emit aboutToStart(m_dest.path()); 

  // Send description to the GUI
  emit description(this, i18n("Synchronizing"),
                   qMakePair(i18n("Source"), source),
                   qMakePair(i18n("Destination"), destination));
     
  // Dummy to show 0 %
  emitPercent(0, 100);
  
  m_process->start();
}


void Smb4KSyncJob::slotReadStandardOutput()
{
  QStringList stdOut = QString::fromUtf8(m_process->readAllStandardOutput(), -1).split('\n', QString::SkipEmptyParts);

  for (int i = 0; i < stdOut.size(); ++i)
  {
    if (stdOut.at(i)[0].isSpace())
    {
      // Get the overall transfer progress
      if (stdOut.at(i).contains(" to-check="))
      {
        QString tmp = stdOut.at(i).section(" to-check=", 1, 1).section(')', 0, 0).trimmed();

        bool success1 = true;
        bool success2 = true;

        qulonglong files = tmp.section('/', 0, 0).trimmed().toLongLong(&success1);
        qulonglong total = tmp.section('/', 1, 1).trimmed().toLongLong(&success2);

        if (success1 && success2)
        {
          setProcessedAmount(KJob::Files, total - files);
          setTotalAmount(KJob::Files, total);
          emitPercent(total - files, total);
        }
        else
        {
          // Do nothing
        }
      }
      else if (stdOut.at(i).contains(" to-chk="))
      {
        // Make Smb4K work with rsync >= 3.1.
        QString tmp = stdOut.at(i).section(" to-chk=", 1, 1).section(')', 0, 0).trimmed();

        bool success1 = true;
        bool success2 = true;

        qulonglong files = tmp.section('/', 0, 0).trimmed().toLongLong(&success1);
        qulonglong total = tmp.section('/', 1, 1).trimmed().toLongLong(&success2);

        if (success1 && success2)
        {
          setProcessedAmount(KJob::Files, total - files);
          setTotalAmount(KJob::Files, total);
          emitPercent(total - files, total);
        }
        else
        {
          // Do nothing
        }
      }
      else if (stdOut.at(i).contains(" ir-chk="))
      {
        // Make Smb4K work with rsync >= 3.1.
        QString tmp = stdOut.at(i).section(" ir-chk=", 1, 1).section(')', 0, 0).trimmed();

        bool success1 = true;
        bool success2 = true;

        qulonglong files = tmp.section('/', 0, 0).trimmed().toLongLong(&success1);
        qulonglong total = tmp.section('/', 1, 1).trimmed().toLongLong(&success2);

        if (success1 && success2)
        {
          setProcessedAmount(KJob::Files, total - files);
          setTotalAmount(KJob::Files, total);
          emitPercent(total - files, total);
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // No overall transfer progress can be determined
      }

      // Get transfer rate
      if (stdOut.at(i).contains("/s ", Qt::CaseSensitive))
      {
        bool success = true;
        
        double tmp_speed = stdOut.at(i).section(QRegExp("../s"), 0, 0).section(' ', -1 -1).trimmed().toDouble(&success);

        if (success)
        {
          // MB == 1000000 B and kB == 1000 B per definitionem!
          if (stdOut.at(i).contains("MB/s"))
          {
            tmp_speed *= 1e6;
          }
          else if (stdOut.at(i).contains("kB/s"))
          {
            tmp_speed *= 1e3;
          }
          else
          {
            // Do nothing
          }

          ulong speed = (ulong)tmp_speed;
          emitSpeed(speed /* B/s */);
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // No transfer rate available
      }
    }
    else if (!stdOut.at(i).contains("sending incremental file list"))
    {
      QString file = stdOut.at(i).trimmed();

      QUrl src_url = m_src;
      src_url.setPath(QDir::cleanPath(src_url.path() + '/' + file));

      QUrl dest_url = m_dest;
      dest_url.setPath(QDir::cleanPath(dest_url.path() + '/' + file));
      
      // Send description to the GUI
      emit description(this, i18n("Synchronizing"),
                       qMakePair(i18n("Source"), src_url.path()),
                       qMakePair(i18n("Destination"), dest_url.path()));
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KSyncJob::slotReadStandardError()
{
  QString stdErr = QString::fromUtf8(m_process->readAllStandardError(), -1).trimmed();

  // Avoid reporting an error if the process was killed by calling the abort() function.
  if (!m_process->isAborted() && (stdErr.contains("rsync error:") && !stdErr.contains("(code 23)")
       /*ignore "some files were not transferred" error*/))
  {
    m_process->abort();
    Smb4KNotification::synchronizationFailed(m_src, m_dest, stdErr);
  }
  else
  {
    // Go ahead
  }
}


void Smb4KSyncJob::slotProcessFinished(int, QProcess::ExitStatus status)
{
  // Dummy to show 100 %
  emitPercent(100, 100);
  
  // Handle error.
  switch (status)
  {
    case QProcess::CrashExit:
    {
      if (!m_process->isAborted())
      {
        Smb4KNotification::processError(m_process->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }

  // Finish job
  emitResult();
  emit finished(m_dest.path());
}



Smb4KSynchronizationDialog::Smb4KSynchronizationDialog(Smb4KShare *share, QWidget *parent)
: QDialog(parent), m_share(share)
{
  setWindowTitle(i18n("Synchronization"));
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_swap_button = buttonBox->addButton(i18n("Swap Paths"), QDialogButtonBox::ActionRole);
  m_swap_button->setToolTip(i18n("Swap source and destination"));
  m_synchronize_button = buttonBox->addButton(i18n("Synchronize"), QDialogButtonBox::ActionRole);
  m_synchronize_button->setToolTip(i18n("Synchronize the destination with the source"));
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_synchronize_button->setDefault(true);

  QGridLayout *layout = new QGridLayout(this);
  layout->setSpacing(5);

  QLabel *pixmap = new QLabel(this);
  QPixmap sync_pix = KDE::icon("folder-sync").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(sync_pix);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *description = new QLabel(i18n("Please provide the source and destination "
                                        "directory for the synchronization."), this);
  description->setWordWrap(true);
  description->setAlignment(Qt::AlignBottom);

  QUrl src_url  = QUrl(QDir::cleanPath(m_share->path()));
  QUrl dest_url = QUrl(QDir::cleanPath(QString("%1/%2/%3").arg(Smb4KSettings::rsyncPrefix().path())
                       .arg(m_share->hostName()).arg(m_share->shareName())));

  QLabel *source_label = new QLabel(i18n("Source:"), this);
  m_source = new KUrlRequester(this);
  m_source->setUrl(src_url);
  m_source->setMode(KFile::Directory | KFile::LocalOnly);
  m_source->lineEdit()->setSqueezedTextEnabled(true);
  m_source->completionObject()->setCompletionMode(KCompletion::CompletionPopupAuto);
  m_source->completionObject()->setMode(KUrlCompletion::FileCompletion);
  m_source->setWhatsThis(i18n("This is the source directory. The data that it contains is to be written "
    "to the destination directory."));

  QLabel *destination_label = new QLabel(i18n("Destination:"), this);
  m_destination = new KUrlRequester(this);
  m_destination->setUrl(dest_url);
  m_destination->setMode(KFile::Directory | KFile::LocalOnly);
  m_destination->lineEdit()->setSqueezedTextEnabled(true);
  m_destination->completionObject()->setCompletionMode(KCompletion::CompletionPopupAuto);
  m_destination->completionObject()->setMode(KUrlCompletion::FileCompletion);
  m_destination->setWhatsThis(i18n("This is the destination directory. It will be updated with the data "
    "from the source directory."));

  layout->addWidget(pixmap, 0, 0, 0);
  layout->addWidget(description, 0, 1, Qt::AlignBottom);
  layout->addWidget(source_label, 1, 0, 0);
  layout->addWidget(m_source, 1, 1, 0);
  layout->addWidget(destination_label, 2, 0, 0);
  layout->addWidget(m_destination, 2, 1, 0);
  layout->addWidget(buttonBox, 3, 0, 1, 2, 0);

  // Connections
  connect(m_cancel_button, SIGNAL(clicked()), SLOT(slotCancelClicked()));
  connect(m_synchronize_button, SIGNAL(clicked()), SLOT(slotSynchronizeClicked()));
  connect(m_swap_button, SIGNAL(clicked()), SLOT(slotSwapPathsClicked()));

  setMinimumSize((sizeHint().width() > 350 ? sizeHint().width() : 350), sizeHint().height());

  KConfigGroup group(Smb4KSettings::self()->config(), "SynchronizationDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
}


Smb4KSynchronizationDialog::~Smb4KSynchronizationDialog()
{
}


const QUrl Smb4KSynchronizationDialog::source()
{
  return m_source->url();
}


const QUrl Smb4KSynchronizationDialog::destination()
{
  return m_destination->url();
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KSynchronizationDialog::slotCancelClicked()
{
  reject();
}


void Smb4KSynchronizationDialog::slotSynchronizeClicked()
{
  KConfigGroup group(Smb4KSettings::self()->config(), "SynchronizationDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  accept();
}


void Smb4KSynchronizationDialog::slotSwapPathsClicked()
{
  // Swap URLs.
  QString sourceURL = m_source->url().path();
  QString destinationURL = m_destination->url().path();

  m_source->setUrl(QUrl(destinationURL));
  m_destination->setUrl(QUrl(sourceURL));
}

