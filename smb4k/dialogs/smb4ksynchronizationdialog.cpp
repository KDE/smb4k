/***************************************************************************
    smb4ksynchronizationdialog  -  The synchronization dialog of Smb4K
                             -------------------
    begin                : Sa Mai 19 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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

// Qt includes
#include <QGridLayout>

// KDE includes
#include <klocale.h>
#include <kguiitem.h>
#include <kdebug.h>
#include <kstandardguiitem.h>
#include <kurl.h>
#include <kurlcompletion.h>

// application specific includes
#include "smb4ksynchronizationdialog.h"
#include <core/smb4kshare.h>
#include <core/smb4ksynchronizationinfo.h>
#include <core/smb4ksettings.h>
#include <core/smb4ksynchronizer.h>

Smb4KSynchronizationDialog::Smb4KSynchronizationDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ), m_share( share )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  m_info = new Smb4KSynchronizationInfo();

  setCaption( i18n( "Synchronization" ) );
  setButtons( User3|User2|User1 );
  setDefaultButton( User2 );

  setButtonGuiItem( User1, KStandardGuiItem::close() );
  setButtonGuiItem( User2, KGuiItem( i18n( "Synchronize" ), "go-bottom",
                    i18n( "Synchronize the destination with the source" ) ) );
  setButtonGuiItem( User3, KGuiItem( i18n( "Swap Paths" ), "document-swap",
                    i18n( "Swap source and destination" ) ) );

  QWidget *main_widget      = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout  = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *source_label      = new QLabel( i18n( "Source:" ), main_widget );
  m_source                  = new KUrlRequester( main_widget );
  m_source->setUrl( KUrl( QString::fromUtf8( m_share->path() )+"/" ) );
  m_source->setMode( KFile::Directory | KFile::LocalOnly );
  m_source->lineEdit()->setSqueezedTextEnabled( true );
  m_source->completionObject()->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_source->completionObject()->setMode( KUrlCompletion::FileCompletion );
  m_source->setWhatsThis( i18n( "This is the source directory. The data that it contains is to be written "
    "to the destination directory." ) );
//   m_source->setToolTip( i18n( "The source directory" ) );

  QLabel *destination_label = new QLabel( i18n( "Destination:" ), main_widget );
  m_destination             = new KUrlRequester( main_widget );
  m_destination->setUrl( Smb4KSettings::rsyncPrefix() );
  m_destination->setMode( KFile::Directory | KFile::LocalOnly );
  m_destination->lineEdit()->setSqueezedTextEnabled( true );
  m_destination->completionObject()->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_destination->completionObject()->setMode( KUrlCompletion::FileCompletion );
  m_destination->setWhatsThis( i18n( "This is the destination directory. It will be updated with the data "
    "from the source directory." ) );
//   m_destination->setToolTip( i18n( "The destination directory" ) );

  m_current_file            = new KLineEdit( main_widget );
  m_current_file->setSqueezedTextEnabled( true );
  m_current_file->setReadOnly( true );
  m_current_file->setWhatsThis( i18n( "The file that is currently transferred is shown here." ) );
//   m_current_file->setToolTip( i18n( "The currently transferred file" ) );

  m_current_progress        = new QProgressBar( main_widget );
  m_current_progress->setRange( 0, 100 );
  m_current_progress->setEnabled( false );
  m_current_progress->setWhatsThis( i18n( "The progress of the current file transfer is shown here." ) );
//   m_current_progress->setToolTip( i18n( "The progress of the current file transfer" ) );

  m_total_progress          = new QProgressBar( main_widget );
  m_total_progress->setRange( 0, 100 );
  m_total_progress->setEnabled( false );
  m_total_progress->setWhatsThis( i18n( "The overall progress of the synchronization is shown here." ) );
//   m_total_progress->setToolTip( i18n( "The overall progress" ) );

  m_transfer_widget         = new QWidget( main_widget );

  QGridLayout *trans_layout = new QGridLayout( m_transfer_widget );
  trans_layout->setSpacing( 5 );
  trans_layout->setMargin( 0 );

  QLabel *file_label        = new QLabel( i18n( "Files transferred:" ), m_transfer_widget );
  m_transferred_files       = new QLabel( "0 / 0", m_transfer_widget );

  QLabel *rate_label        = new QLabel( i18n( "Transfer rate:" ), m_transfer_widget );
  m_transfer_rate           = new QLabel( "0.00 kB/s", m_transfer_widget );

  trans_layout->addWidget( file_label, 0, 0, 0 );
  trans_layout->addWidget( m_transferred_files, 0, 1, Qt::AlignRight );
  trans_layout->addWidget( rate_label, 1, 0, 0 );
  trans_layout->addWidget( m_transfer_rate, 1, 1, Qt::AlignRight );

  m_transfer_widget->setEnabled( false );

  layout->addWidget( source_label, 0, 0, 0 );
  layout->addWidget( m_source, 0, 1, 0 );
  layout->addWidget( destination_label, 1, 0, 0 );
  layout->addWidget( m_destination, 1, 1, 0 );
  layout->addWidget( m_current_file, 2, 0, 1, 2, 0 );
  layout->addWidget( m_current_progress, 3, 0, 1, 2, 0 );
  layout->addWidget( m_total_progress, 4, 0, 1, 2, 0 );
  layout->addWidget( m_transfer_widget, 5, 0, 2, 2, 0 );

  // Connections
  connect( this,                      SIGNAL( user1Clicked() ),
           this,                      SLOT( slotUser1Clicked() ) );

  connect( this,                      SIGNAL( user2Clicked() ),
           this,                      SLOT( slotUser2Clicked() ) );

  connect( this,                      SIGNAL( user3Clicked() ),
           this,                      SLOT( slotUser3Clicked() ) );

  connect( Smb4KSynchronizer::self(), SIGNAL( progress( Smb4KSynchronizationInfo * ) ),
           this,                      SLOT( slotProgress( Smb4KSynchronizationInfo * ) ) );

  connect( Smb4KSynchronizer::self(), SIGNAL( aboutToStart( Smb4KSynchronizationInfo * ) ),
           this,                      SLOT( slotSynchronizationAboutToStart( Smb4KSynchronizationInfo * ) ) );

  connect( Smb4KSynchronizer::self(), SIGNAL( finished( Smb4KSynchronizationInfo * ) ),
           this,                      SLOT( slotSynchronizationFinished( Smb4KSynchronizationInfo * ) ) );

  setMinimumSize( (sizeHint().width() > 350 ? sizeHint().width() : 350), sizeHint().height() );

  setInitialSize( QSize( minimumWidth(), minimumHeight() ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "SynchronizationDialog" );
  restoreDialogSize( group );
}


Smb4KSynchronizationDialog::~Smb4KSynchronizationDialog()
{
  // Do *not* delete the share object here.

  delete m_info;
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KSynchronizationDialog::slotUser1Clicked()
{
  // Abort or exit.
  if ( Smb4KSynchronizer::self()->isRunning( m_info ) )
  {
    Smb4KSynchronizer::self()->abort( m_info );
  }
  else
  {
    KConfigGroup group( Smb4KSettings::self()->config(), "SynchronizationDialog" );
    saveDialogSize( group, KConfigGroup::Normal );
    close();
  }
}


void Smb4KSynchronizationDialog::slotUser2Clicked()
{
  m_current_progress->setValue( 0 );
  m_total_progress->setValue( 0 );

  // Synchronize!
  m_info->setSourcePath( m_source->url().path() );
  m_info->setDestinationPath( m_destination->url().path() );

  Smb4KSynchronizer::self()->synchronize( m_info );
}


void Smb4KSynchronizationDialog::slotUser3Clicked()
{
  // Swap URLs.
  QString sourceURL = m_source->url().path();
  QString destinationURL = m_destination->url().path();

  m_source->setUrl( KUrl( destinationURL ) );
  m_destination->setUrl( KUrl( sourceURL ) );
}


void Smb4KSynchronizationDialog::slotProgress( Smb4KSynchronizationInfo *info )
{
  if ( m_info->equals( info ) )
  {
    if ( !info->text().isEmpty() )
    {
      m_current_file->setSqueezedText( info->text() );
    }
    else
    {
      // Do nothing
    }

    if ( info->currentProgress() != -1 )
    {
      m_current_progress->setValue( info->currentProgress() );
    }
    else
    {
      // Do nothing
    }

    if ( info->totalProgress() != -1 )
    {
      m_total_progress->setValue( info->totalProgress() );
    }
    else
    {
      // Do nothing
    }

    if ( info->totalFileNumber() != -1 && info->processedFileNumber() != -1 )
    {
      m_transferred_files->setText( QString( "%1 / %2" ).arg( info->processedFileNumber() )
                                                        .arg( info->totalFileNumber() ) );
    }
    else
    {
      // Do nothing
    }

    if ( !info->transferRate().isEmpty() )
    {
      m_transfer_rate->setText( info->transferRate() );
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


void Smb4KSynchronizationDialog::slotSynchronizationAboutToStart( Smb4KSynchronizationInfo *info )
{
  if ( m_info->equals( info ) )
  {
    // Disable the URL requesters but in a way, that the information
    // proviced in them is still readable:
    m_source->lineEdit()->setReadOnly( true );
    m_source->button()->setEnabled( false );

    m_destination->lineEdit()->setReadOnly( true );
    m_destination->button()->setEnabled( false );

    setButtonGuiItem( User1, KStandardGuiItem::cancel() );
    enableButton( User2, false );
    enableButton( User3, false );

    m_transfer_widget->setEnabled( true );
    m_current_progress->setEnabled( true );
    m_total_progress->setEnabled( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSynchronizationDialog::slotSynchronizationFinished( Smb4KSynchronizationInfo *info )
{
  if ( m_info->equals( info ) )
  {
    m_current_progress->setValue( 100 );
    m_total_progress->setValue( 100 );

    setButtonGuiItem( User1, KStandardGuiItem::close() );
  }
  else
  {
    // Do nothing
  }
}

#include "smb4ksynchronizationdialog.moc"
