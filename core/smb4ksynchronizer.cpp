/***************************************************************************
    smb4ksynchronizer  -  This is the synchronizer of Smb4K.
                             -------------------
    begin                : Mo Jul 4 2005
    copyright            : (C) 2005-2009 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// KDE includes
#include <kdebug.h>
#include <kapplication.h>
#include <kshell.h>
#include <kstandarddirs.h>

// application specific includes
#include <smb4ksynchronizer.h>
#include <smb4kdefs.h>
#include <smb4kcoremessage.h>
#include <smb4kglobal.h>
#include <smb4ksynchronizationinfo.h>
#include <smb4ksynchronizer_p.h>
#include <smb4ksettings.h>
#include <smb4knotification.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KSynchronizerPrivate, priv );



Smb4KSynchronizer::Smb4KSynchronizer() : QObject()
{
  m_state = SYNCHRONIZER_STOP;

  connect( kapp,   SIGNAL( aboutToQuit() ),
           this,   SLOT( slotAboutToQuit() ) );
}


Smb4KSynchronizer::~Smb4KSynchronizer()
{
}


Smb4KSynchronizer *Smb4KSynchronizer::self()
{
  return &priv->instance;
}


void Smb4KSynchronizer::synchronize( Smb4KSynchronizationInfo *info )
{
  Q_ASSERT( info );

  // Find rsync program.
  QString rsync = KStandardDirs::findExe( "rsync" );

  if ( rsync.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "rsync" );
    return;
  }
  else
  {
    // Go ahead
  }

  // Check that the synchronization is not already running for the
  // given mount point.
  if ( findChild<Smb4KProcess *>( info->sourcePath() ) )
  {
    // FIXME: Show message
    kDebug() << "FIXME: Show error/information message..." << endl;
    return;
  }
  else
  {
    // Go ahead
  }

  // Compile the command
  QString command;
  command += rsync;
  command += " --progress";

  if ( Smb4KSettings::archiveMode() )
  {
    command += " --archive";
  }
  else
  {
    command += Smb4KSettings::recurseIntoDirectories() ? " --recursive" : "";
    command += Smb4KSettings::preserveSymlinks() ? " --links" : "";
    command += Smb4KSettings::preservePermissions() ? " --perms" : "";
    command += Smb4KSettings::preserveTimes() ? " --times" : "";
    command += Smb4KSettings::preserveGroup() ? " --group" : "";
    command += Smb4KSettings::preserveOwner() ? " --owner" : "";
    command += Smb4KSettings::preserveDevicesAndSpecials() ? " --devices --specials" /* alias -D */ : "";
  }

  command += Smb4KSettings::relativePathNames() ? " --relative" : "";
  command += Smb4KSettings::omitDirectoryTimes() ? " --omit-dir-times" : "";
  command += Smb4KSettings::noImpliedDirectories() ? " --no-implied-dirs" : "";
  command += Smb4KSettings::updateTarget() ? " --update" : "";
  command += Smb4KSettings::updateInPlace() ? " --inplace" : "";
  command += Smb4KSettings::transferDirectories() ? " --dirs" : "";
  command += Smb4KSettings::transformSymlinks() ? " --copy-links" : "";
  command += Smb4KSettings::transformUnsafeSymlinks() ? " --copy-unsafe-links" : "";
  command += Smb4KSettings::ignoreUnsafeSymlinks() ? " --safe-links" : "";
  command += Smb4KSettings::preserveHardLinks() ? " --hard-links" : "";
  command += Smb4KSettings::keepDirectorySymlinks() ? " --keep-dirlinks" : "";
  command += Smb4KSettings::deleteExtraneous() ? " --delete" : "";
  command += Smb4KSettings::removeSourceFiles() ? " --remove-source-files" : "";
  command += Smb4KSettings::deleteBefore() ? " --delete-before" : "";
  command += Smb4KSettings::deleteDuring() ? " --delete-during" : "";
  command += Smb4KSettings::deleteAfter() ? " --delete-after" : "";
  command += Smb4KSettings::deleteExcluded() ? " --delete-excluded" : "";
  command += Smb4KSettings::ignoreErrors() ? " --ignore-errors" : "";
  command += Smb4KSettings::forceDirectoryDeletion() ? " --force" : "";
  command += Smb4KSettings::copyFilesWhole() ? " --whole-file" : "";
  command += Smb4KSettings::efficientSparseFileHandling() ? " --sparse" : "";
  command += Smb4KSettings::oneFileSystem() ? " --one-file-system" : "";
  command += Smb4KSettings::updateExisting() ? " --existing" : "";
  command += Smb4KSettings::ignoreExisting() ? " --ignore-existing" : "";
  command += Smb4KSettings::delayUpdates() ? " --delay-updates" : "";
  command += Smb4KSettings::compressData() ? " --compress" : "";

  if ( Smb4KSettings::makeBackups() )
  {
    command += " --backup";
    command += Smb4KSettings::useBackupDirectory() ? " --backup-dir="+Smb4KSettings::backupDirectory().path() : "";
    command += Smb4KSettings::useBackupSuffix() ? " --suffix="+Smb4KSettings::backupSuffix() : "";
  }
  else
  {
    // Do nothing
  }

  command += Smb4KSettings::useMaximumDelete() ? " --max-delete="+QString( "%1" ).arg( Smb4KSettings::maximumDeleteValue() ) : "";
  command += Smb4KSettings::useChecksum() ? " --checksum" : "";
  command += Smb4KSettings::useBlockSize() ? " --block-size="+QString( "%1" ).arg( Smb4KSettings::blockSize() ) : "";
  command += Smb4KSettings::useChecksumSeed() ? " --checksum-seed="+QString( "%1" ).arg( Smb4KSettings::checksumSeed() ) : "";
  command += !Smb4KSettings::customFilteringRules().isEmpty() ? " "+Smb4KSettings::customFilteringRules() : "";
  command += Smb4KSettings::useMinimalTransferSize() ? " --min-size="+QString( "%1" ).arg( Smb4KSettings::minimalTransferSize() )+"K" : "";
  command += Smb4KSettings::useMaximalTransferSize() ? " --max-size="+QString( "%1" ).arg( Smb4KSettings::maximalTransferSize() )+"K" : "";

  if ( Smb4KSettings::keepPartial() )
  {
    command += " --partial";
    command += Smb4KSettings::usePartialDirectory() ? " --partial-dir="+Smb4KSettings::partialDirectory().path() : "";
  }
  else
  {
    // Do nothing
  }

  command += Smb4KSettings::useCVSExclude() ? " --cvs-exclude" : "";
  command += Smb4KSettings::useFFilterRule() ? " -F" : "";
  command += Smb4KSettings::useFFFilterRule() ? " -F -F" : "";
  command += Smb4KSettings::useExcludePattern() ? " --exclude="+Smb4KSettings::excludePattern() : "";
  command += Smb4KSettings::useExcludeFrom() ? " --exclude-from="+Smb4KSettings::excludeFrom().path() : "";
  command += Smb4KSettings::useIncludePattern() ? " --include="+Smb4KSettings::includePattern() : "";
  command += Smb4KSettings::useIncludeFrom() ? " --include-from="+Smb4KSettings::includeFrom().path() : "";
  command += " "+KShell::quoteArg( info->sourcePath() );
  command += " "+KShell::quoteArg( info->destinationPath() );

  // Start the synchronization.
  if ( m_cache.size() == 0 )
  {
    m_state = SYNCHRONIZER_START;
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( info );

  SynchronizationThread *thread = new SynchronizationThread( this );
  m_cache.insert( info->sourcePath(), thread );

  connect( thread, SIGNAL( finished() ), this, SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( progress( Smb4KSynchronizationInfo * ) ), this, SIGNAL( progress( Smb4KSynchronizationInfo * ) ) );

  thread->start();
  thread->synchronize( info, command );
}


bool Smb4KSynchronizer::isRunning( Smb4KSynchronizationInfo *info )
{
  Q_ASSERT( info );

  SynchronizationThread *thread = m_cache.object( info->sourcePath() );
  return (thread && thread->process() && thread->process()->state() == KProcess::Running);
}


void Smb4KSynchronizer::abort( Smb4KSynchronizationInfo *info )
{
  Q_ASSERT( info );

  SynchronizationThread *thread = m_cache.object( info->sourcePath() );

  if ( thread && thread->process() &&
       (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
  {
    thread->process()->abort();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KSynchronizer::isAborted( Smb4KSynchronizationInfo *info )
{
  Q_ASSERT( info );

  SynchronizationThread *thread = m_cache.object( info->sourcePath() );
  return (thread && thread->process() && thread->process()->isAborted());
}


void Smb4KSynchronizer::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QStringList keys = m_cache.keys();

    foreach ( const QString &key, keys )
    {
      SynchronizationThread *thread = m_cache.object( key );

      if ( thread->process() && (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
      {
        thread->process()->abort();
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // priv has already been deleted
  }
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSynchronizer::slotAboutToQuit()
{
  abortAll();
}


void Smb4KSynchronizer::slotThreadFinished()
{
  QStringList keys = m_cache.keys();

  foreach ( const QString &key, keys )
  {
    SynchronizationThread *thread = m_cache.object( key );

    if ( thread->isFinished() )
    {
      (void) m_cache.take( key );
      emit finished( thread->synchronizationInfo() );
      delete thread;
    }
    else
    {
      // Do not touch the thread
    }
  }

  if ( m_cache.size() == 0 )
  {
    m_state = SYNCHRONIZER_STOP;
    emit stateChanged();
  }
  else
  {
    // Still running
  }
}

#include "smb4ksynchronizer.moc"
