/***************************************************************************
    smb4ksynchronizer_p  -  This file contains private helper classes for
    the Smb4KSynchronizer class.
                             -------------------
    begin                : Fr Okt 24 2008
    copyright            : (C) 2008-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QTimer>
#include <QGridLayout>
#include <QLabel>

// KDE includes
#include <kdebug.h>
#include <kprocess.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kurlcompletion.h>

// application specific includes
#include <smb4ksynchronizer_p.h>
#include <smb4knotification.h>
#include <smb4ksettings.h>
#include <smb4kglobal.h>

using namespace Smb4KGlobal;


Smb4KSyncJob::Smb4KSyncJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_share( NULL ), m_parent_widget( NULL ), m_proc( NULL )
{
  setCapabilities( KJob::Killable );
}


Smb4KSyncJob::~Smb4KSyncJob()
{
}


void Smb4KSyncJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT( slotStartSynchronization() ) );
}


void Smb4KSyncJob::setupSynchronization( Smb4KShare *share, QWidget *parent )
{
  Q_ASSERT( share );
  m_share = share;
  m_parent_widget = parent;
}


bool Smb4KSyncJob::doKill()
{
  if ( m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting) )
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KSyncJob::slotStartSynchronization()
{
  // Find rsync program.
  QString rsync = KStandardDirs::findExe( "rsync" );

  if ( rsync.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "rsync" );
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }

  if ( m_share )
  {
    // Show the user an URL input dialog.
    Smb4KSynchronizationDialog dlg( m_share, m_parent_widget );

    if ( dlg.exec() == KDialog::Accepted )
    {
      // Create the destination directory if it does not already exits.
      if ( !QFile::exists( dlg.destination().path() ) )
      {
        QDir sync_dir( dlg.destination().path() );

        if ( !sync_dir.mkpath( dlg.destination().path() ) )
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->mkdirFailed( sync_dir );
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

      m_src = dlg.source().path();
      m_dest = dlg.destination().path();
    }
    else
    {
      emitResult();
      return;
    }
    
    // Get the list of arguments
    QStringList arguments;
    arguments << "--progress";

    if ( Smb4KSettings::archiveMode() )
    {
      arguments << "--archive";
    }
    else
    {
      if ( Smb4KSettings::recurseIntoDirectories() )
      {
        arguments << "--recursive";
      }
      else
      {
        // Do nothing
      }

      if ( Smb4KSettings::preserveSymlinks() )
      {
        arguments << "--links";
      }
      else
      {
        // Do nothing
      }

      if ( Smb4KSettings::preservePermissions() )
      {
        arguments << "--perms";
      }
      else
      {
        // Do nothing
      }

      if ( Smb4KSettings::preserveTimes() )
      {
        arguments << "--times";
      }
      else
      {
        // Do nothing
      }

      if ( Smb4KSettings::preserveGroup() )
      {
        arguments << "--group";
      }
      else
      {
        // Do nothing
      }

      if ( Smb4KSettings::preserveOwner() )
      {
        arguments << "--owner";
      }
      else
      {
        // Do nothing
      }

      if ( Smb4KSettings::preserveDevicesAndSpecials() )
      {
        // Alias -D
        arguments << "--devices";
        arguments << "--specials";
      }
      else
      {
        // Do nothing
      }
    }

    if ( Smb4KSettings::relativePathNames() )
    {
      arguments << "--relative";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::omitDirectoryTimes() )
    {
      arguments << "--omit-dir-times";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::noImpliedDirectories() )
    {
      arguments << "--no-implied-dirs";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::updateTarget() )
    {
      arguments << "--update";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::updateInPlace() )
    {
      arguments << "--inplace";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::transferDirectories() )
    {
      arguments << "--dirs";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::transformSymlinks() )
    {
      arguments << "--copy-links";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::transformUnsafeSymlinks() )
    {
      arguments << "--copy-unsafe-links";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::ignoreUnsafeSymlinks() )
    {
      arguments << "--safe-links";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::preserveHardLinks() )
    {
      arguments << "--hard-links";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::keepDirectorySymlinks() )
    {
      arguments << "--keep-dirlinks";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::deleteExtraneous() )
    {
      arguments << "--delete";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::removeSourceFiles() )
    {
      arguments << "--remove-source-files";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::deleteBefore() )
    {
      arguments << "--delete-before";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::deleteDuring() )
    {
      arguments << "--delete-during";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::deleteAfter() )
    {
      arguments << "--delete-after";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::deleteExcluded() )
    {
      arguments << "--delete-excluded";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::ignoreErrors() )
    {
      arguments << "--ignore-errors";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::forceDirectoryDeletion() )
    {
      arguments << "--force";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::copyFilesWhole() )
    {
      arguments << "--whole-file";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::efficientSparseFileHandling() )
    {
      arguments << "--sparse";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::oneFileSystem() )
    {
      arguments << "--one-file-system";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::updateExisting() )
    {
      arguments << "--existing";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::ignoreExisting() )
    {
      arguments << "--ignore-existing";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::delayUpdates() )
    {
      arguments << "--delay-updates";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::compressData() )
    {
      arguments << "--compress";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::makeBackups() )
    {
      arguments << "--backup";

      if ( Smb4KSettings::useBackupDirectory() )
      {
        arguments << QString( "--backup-dir=%1" ).arg( Smb4KSettings::backupDirectory().path() );
      }
      else
      {
        // Do nothing
      }

      if ( Smb4KSettings::useBackupSuffix() )
      {
        arguments << QString( "--suffix=%1" ).arg( Smb4KSettings::backupSuffix() );
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

    if ( Smb4KSettings::useMaximumDelete() )
    {
      arguments << QString( "--max-delete=%1" ).arg( Smb4KSettings::maximumDeleteValue() );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useChecksum() )
    {
      arguments << "--checksum";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useBlockSize() )
    {
      arguments << QString( "--block-size=%1" ).arg( Smb4KSettings::blockSize() );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useChecksumSeed() )
    {
      arguments << QString( "--checksum-seed=%1" ).arg( Smb4KSettings::checksumSeed() );
    }
    else
    {
      // Do nothing
    }

    if ( !Smb4KSettings::customFilteringRules().isEmpty() )
    {
      arguments << Smb4KSettings::customFilteringRules().split( " " );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useMinimalTransferSize() )
    {
      arguments << QString( "--min-size=%1K" ).arg( Smb4KSettings::minimalTransferSize() );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useMaximalTransferSize() )
    {
      arguments << QString( "--max-size=%1K" ).arg( Smb4KSettings::maximalTransferSize() );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::keepPartial() )
    {
      arguments << " --partial";

      if ( Smb4KSettings::usePartialDirectory() )
      {
        arguments << QString( "--partial-dir=%1" ).arg( Smb4KSettings::partialDirectory().path() );
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

    if ( Smb4KSettings::useCVSExclude() )
    {
      arguments << "--cvs-exclude";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useFFilterRule() )
    {
      arguments << "-F";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useFFFilterRule() )
    {
      arguments << "-F";
      arguments << "-F";
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useExcludePattern() )
    {
      arguments << QString( "--exclude=%1" ).arg( Smb4KSettings::excludePattern() );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useExcludeFrom() )
    {
      arguments << QString( "--exclude-from=%1" ).arg( Smb4KSettings::excludeFrom().path() );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useIncludePattern() )
    {
      arguments << QString( "--include=%1" ).arg( Smb4KSettings::includePattern() );
    }
    else
    {
      // Do nothing
    }

    if ( Smb4KSettings::useIncludeFrom() )
    {
      arguments << QString( "--include-from=%1" ).arg( Smb4KSettings::includeFrom().path() );
    }
    else
    {
      // Do nothing
    }

    arguments << m_src.path();
    arguments << m_dest.path();

    emit aboutToStart( m_dest.path() );

    // Register the job with the job tracker
    jobTracker()->registerJob( this );
    connect( this, SIGNAL( result( KJob * ) ), jobTracker(), SLOT( unregisterJob( KJob * ) ) );

    // Send description to the GUI
    emit description( this, i18n( "Synchronizing" ),
                      qMakePair( i18n( "Source" ), m_src.path() ),
                      qMakePair( i18n( "Destination" ), m_dest.path() ) );
    
    // Dummy to show 0 %
    emitPercent( 0, 100 );

    m_proc = new Smb4KProcess( Smb4KProcess::Synchronize, this );
    m_proc->setOutputChannelMode( KProcess::SeparateChannels );
    m_proc->setProgram( rsync, arguments );

    connect( m_proc, SIGNAL( readyReadStandardOutput() ), SLOT( slotReadStandardOutput() ) );
    connect( m_proc, SIGNAL( readyReadStandardError() ),  SLOT( slotReadStandardError() ) );
    connect( m_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( slotProcessFinished( int, QProcess::ExitStatus ) ) );

    m_proc->start();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSyncJob::slotReadStandardOutput()
{
  QStringList stdout = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( "\n", QString::SkipEmptyParts );

  for ( int i = 0; i < stdout.size(); ++i )
  {
    if ( stdout.at( i )[0].isSpace() )
    {
      // Get the overall transfer progress
      if ( stdout.at( i ).contains( " to-check=" ) )
      {
        QString tmp = stdout.at( i ).section( " to-check=", 1, 1 ).section( ")", 0, 0 ).trimmed();

        bool success1 = true;
        bool success2 = true;

        qulonglong files = tmp.section( "/", 0, 0 ).trimmed().toLongLong( &success1 );
        qulonglong total = tmp.section( "/", 1, 1 ).trimmed().toLongLong( &success2 );

        if ( success1 && success2 )
        {
          setProcessedAmount( KJob::Files, total - files );
          setTotalAmount( KJob::Files, total );
          emitPercent( total - files, total );
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
      if ( stdout.at( i ).contains( "/s ", Qt::CaseSensitive ) )
      {
        bool success = true;
        
        double tmp_speed = stdout.at( i ).section( QRegExp( "../s" ), 0, 0 ).section( " ", -1 -1 ).trimmed().toDouble( &success );

        if ( success )
        {
          // MB == 1000000 B and kB == 1000 B per definitionem!
          if ( stdout.at( i ).contains( "MB/s" ) )
          {
            tmp_speed *= 1e6;
          }
          else if ( stdout.at( i ).contains( "kB/s" ) )
          {
            tmp_speed *= 1e3;
          }
          else
          {
            // Do nothing
          }

          ulong speed = (ulong)tmp_speed;
          emitSpeed( speed /* B/s */ );
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
    else
    {
      QString file = stdout.at( i ).trimmed();

      KUrl src_url = m_src;
      src_url.setFileName( file );
      src_url.cleanPath();

      KUrl dest_url = m_dest;
      dest_url.setFileName( file );
      dest_url.cleanPath();
      
      // Send description to the GUI
      emit description( this, i18n( "Synchronizing" ),
                        qMakePair( i18n( "Source" ), src_url.path() ),
                        qMakePair( i18n( "Destination" ), dest_url.path() ) );
    }
  }
}


void Smb4KSyncJob::slotReadStandardError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  // Avoid reporting an error if the process was killed by calling the abort() function.
  if ( !m_proc->isAborted() && (stderr.contains( "rsync error:" ) && !stderr.contains( "(code 23)" )
       /*ignore "some files were not transferred" error*/) )
  {
    m_proc->abort();
    Smb4KNotification *notification = new Smb4KNotification();
    notification->synchronizationFailed( m_src, m_dest, stderr );
  }
  else
  {
    // Go ahead
  }
}


void Smb4KSyncJob::slotProcessFinished( int, QProcess::ExitStatus status )
{
  // Dummy to show 100 %
  emitPercent( 100, 100 );
  
  // Handle error.
  switch ( status )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );
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
  emit finished( m_dest.path() );
}



Smb4KSynchronizationDialog::Smb4KSynchronizationDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ), m_share( share )
{
  setCaption( i18n( "Synchronization" ) );
  setButtons( User3|User2|User1 );
  setDefaultButton( User2 );

  setButtonGuiItem( User1, KStandardGuiItem::cancel() );
  setButtonGuiItem( User2, KGuiItem( i18n( "Synchronize" ), "go-bottom",
                    i18n( "Synchronize the destination with the source" ) ) );
  setButtonGuiItem( User3, KGuiItem( i18n( "Swap Paths" ), "document-swap",
                    i18n( "Swap source and destination" ) ) );

  QWidget *main_widget      = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout  = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *pixmap            = new QLabel( main_widget );
  QPixmap sync_pix          = KIcon( "folder-sync" ).pixmap( KIconLoader::SizeHuge );
  pixmap->setPixmap( sync_pix );

  QLabel *description       = new QLabel( i18n( "Please provide the source and destination "
                                                "directory for the synchronization." ), main_widget );
  description->setWordWrap( true );

  KUrl src_url  = KUrl( QString::fromUtf8( m_share->path() )+"/" );
  src_url.cleanPath();
  KUrl dest_url = KUrl( QString( "%1/%2/%3" ).arg( Smb4KSettings::rsyncPrefix().path() )
                                             .arg( m_share->hostName() )
                                             .arg( m_share->shareName() ) );
  dest_url.cleanPath();
  

  QLabel *source_label      = new QLabel( i18n( "Source:" ), main_widget );
  m_source                  = new KUrlRequester( main_widget );
  m_source->setUrl( src_url );
  m_source->setMode( KFile::Directory | KFile::LocalOnly );
  m_source->lineEdit()->setSqueezedTextEnabled( true );
  m_source->completionObject()->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_source->completionObject()->setMode( KUrlCompletion::FileCompletion );
  m_source->setWhatsThis( i18n( "This is the source directory. The data that it contains is to be written "
    "to the destination directory." ) );

  QLabel *destination_label = new QLabel( i18n( "Destination:" ), main_widget );
  m_destination             = new KUrlRequester( main_widget );
  m_destination->setUrl( dest_url );
  m_destination->setMode( KFile::Directory | KFile::LocalOnly );
  m_destination->lineEdit()->setSqueezedTextEnabled( true );
  m_destination->completionObject()->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_destination->completionObject()->setMode( KUrlCompletion::FileCompletion );
  m_destination->setWhatsThis( i18n( "This is the destination directory. It will be updated with the data "
    "from the source directory." ) );

  layout->addWidget( pixmap, 0, 0, 0 );
  layout->addWidget( description, 0, 1, Qt::AlignBottom );
  layout->addWidget( source_label, 1, 0, 0 );
  layout->addWidget( m_source, 1, 1, 0 );
  layout->addWidget( destination_label, 2, 0, 0 );
  layout->addWidget( m_destination, 2, 1, 0 );

  // Connections
  connect( this,                      SIGNAL( user1Clicked() ),
           this,                      SLOT( slotUser1Clicked() ) );

  connect( this,                      SIGNAL( user2Clicked() ),
           this,                      SLOT( slotUser2Clicked() ) );

  connect( this,                      SIGNAL( user3Clicked() ),
           this,                      SLOT( slotUser3Clicked() ) );

  setMinimumSize( (sizeHint().width() > 350 ? sizeHint().width() : 350), sizeHint().height() );

  setInitialSize( QSize( minimumWidth(), minimumHeight() ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "SynchronizationDialog" );
  restoreDialogSize( group );
}


Smb4KSynchronizationDialog::~Smb4KSynchronizationDialog()
{
}


const KUrl Smb4KSynchronizationDialog::source()
{
  return m_source->url();
}


const KUrl Smb4KSynchronizationDialog::destination()
{
  return m_destination->url();
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KSynchronizationDialog::slotUser1Clicked()
{
  KConfigGroup group( Smb4KSettings::self()->config(), "SynchronizationDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
  close();
}


void Smb4KSynchronizationDialog::slotUser2Clicked()
{
  accept();
}


void Smb4KSynchronizationDialog::slotUser3Clicked()
{
  // Swap URLs.
  QString sourceURL = m_source->url().path();
  QString destinationURL = m_destination->url().path();

  m_source->setUrl( KUrl( destinationURL ) );
  m_destination->setUrl( KUrl( sourceURL ) );
}


#include "smb4ksynchronizer_p.moc"

