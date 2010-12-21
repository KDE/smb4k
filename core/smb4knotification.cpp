/***************************************************************************
    smb4knotification  -  This class provides notifications for Smb4K.
                             -------------------
    begin                : Son Jun 27 2010
    copyright            : (C) 2010 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.org
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
#include <klocale.h>
#include <kiconloader.h>
#include <krun.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kdemacros.h>

// system includes
#include <string.h>

// application specific includes
#include <smb4knotification.h>
#include <smb4ksettings.h>
#include <smb4kbookmark.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kprintinfo.h>
#include <smb4ksynchronizationinfo.h>


Smb4KNotification::Smb4KNotification( QObject *parent )
: QObject( parent )
{
}


Smb4KNotification::~Smb4KNotification()
{
}


void Smb4KNotification::shareMounted( Smb4KShare* share )
{
  Q_ASSERT( share );

  if ( Smb4KSettings::self()->showNotifications() )
  {
    m_share = *share;

    KNotification *notification = new KNotification( "shareMounted", KNotification::CloseOnTimeout );
    notification->setText( i18n( "The share <b>%1</b> has been mounted to <b>%2</b>." ).arg( share->unc() )
                          .arg( QString::fromUtf8( share->canonicalPath() ) ) );
    notification->setActions( QStringList( i18n( "Open" ) ) );
    notification->setPixmap( KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ) );

    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotOpenShare() ) );
    connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::shareUnmounted( Smb4KShare* share )
{
  Q_ASSERT( share );

  if ( Smb4KSettings::self()->showNotifications() )
  {
    KNotification *notification = KNotification::event( "shareUnmounted",
                                  i18n( "The share <b>%1</b> has been unmounted from <b>%2</b>." ).arg( share->unc() ).arg( QString::fromUtf8( share->path() ) ),
                                  KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ) );
    connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::sharesRemounted( int total, int actual )
{
  if ( Smb4KSettings::self()->showNotifications() )
  {
    if ( total != actual )
    {
      KNotification *notification = KNotification::event( "sharesRemounted",
                                    i18np( "%1 share out of %2 has been remounted.", "%1 shares out of %2 have been remounted.", actual, total ),
                                    KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ) );
      connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
    }
    else
    {
      KNotification *notification = KNotification::event( "sharesRemounted", i18n( "All shares have been remounted." ),
                                    KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ) );
      connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::allSharesUnmounted( int total, int actual )
{
  if ( Smb4KSettings::self()->showNotifications() )
  {
    if ( total != actual )
    {
      KNotification *notification = KNotification::event( "allSharesUnmounted",
                                    i18np( "%1 share out of %2 has been unmounted.", "%1 shares out of %2 have been unmounted.", actual, total ),
                                    KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ) );
      connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
    }
    else
    {
      KNotification *notification = KNotification::event( "allSharesUnmounted", i18n( "All shares have been unmounted." ),
                                    KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ) );
      connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
    }
  }
  else
  {
    // Do nothing
  }
}


//
// Warnings
//


void Smb4KNotification::openingWalletFailed( const QString &name )
{
  KNotification *notification = KNotification::event( "openingWalletFailed",
                                i18n( "Opening the wallet <b>%1</b> failed." ).arg( name ),
                                KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


void Smb4KNotification::loginsNotAccessible()
{
  KNotification *notification = KNotification::event( "loginsNotAccessible",
                                i18n( "The logins stored in the wallet could not be accessed. There is either no wallet available or it could not be opened." ),
                                KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


void Smb4KNotification::mimetypeNotSupported( const QString &mt )
{
  KNotification *notification = KNotification::event( "mimetypeNotSupported",
                                i18n( "The mimetype <b>%1</b> is not supported for printing. "
                                      "Please convert the file to PDF or Postscript and try again." ).arg( mt ),
                                KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


void Smb4KNotification::bookmarkLabelInUse( Smb4KBookmark *bookmark )
{
  KNotification *notification = KNotification::event( "bookmarkLabelInUse",
                                i18n( "The label <b>%1</b> of the bookmark for the share <b>%2</b> "
                                      "is already being used and will automatically be renamed." )
                                .arg( bookmark->label() ).arg( bookmark->unc() ),
                                KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


//
// Errors
//


void Smb4KNotification::retrievingDomainsFailed( const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "retrievingDomainsFailed", KNotification::Persistent );
  notification->setText( i18n( "Retrieving the list of available domains failed." ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::scanningBroadcastAreaFailed( const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "scanningBroadcastAreaFailed", KNotification::Persistent );
  notification->setText( i18n( "Scanning the defined broadcast area(s) failed." ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::retrievingServersFailed( Smb4KWorkgroup* workgroup, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "retrievingServersFailed", KNotification::Persistent );
  notification->setText( i18n( "Retrieving the list of servers belonging to domain <b>%1</b> failed." )
                         .arg( workgroup->workgroupName() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::retrievingSharesFailed( Smb4KHost *host, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "retrievingSharesFailed", KNotification::Persistent );
  notification->setText( i18n( "Retrieving the list of shares from <b>%1</b> failed." ).arg( host->hostName() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::retrievingPreviewFailed( Smb4KShare *share, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "retrievingPreviewFailed", KNotification::Persistent );
  notification->setText( i18n( "Retrieving the preview of <b>%1</b> failed." ).arg( share->unc() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::mountingFailed( Smb4KShare *share, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "mountingFailed", KNotification::Persistent );
  notification->setText( i18n( "Mounting the share <b>%1</b> failed." ).arg( share->unc() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::unmountingFailed( Smb4KShare *share, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "unmountingFailed", KNotification::Persistent );
  notification->setText( i18n( "Unmounting the share <b>%1</b> from <b>%2</b> failed." )
                         .arg( share->unc() ).arg( QString::fromUtf8( share->path() ) ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::unmountingNotAllowed( Smb4KShare *share )
{
  KNotification *notification = KNotification::event( "unmountingNotAllowed",
                                i18n( "You are not allowed to unmount the share <b>%1</b> from <b>%2</b>. "
                                      "It is owned by the user <b>%3</b>." )
                                .arg( share->unc() ).arg( QString::fromUtf8( share->path() ) ).arg( share->owner() ),
                                KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


void Smb4KNotification::printingFailed( Smb4KPrintInfo *info, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "printingFailed", KNotification::Persistent );
  notification->setText( i18n( "Printing file <b>%1</b> on printer <b>%2</b> failed." )
                         .arg( info->filePath() ).arg( info->printer()->unc() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::synchronizationFailed( Smb4KSynchronizationInfo *info, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "synchronizationFailed", KNotification::Persistent );
  notification->setText( i18n( "Synchronizing <b>%1</b> with <b>%2</b> failed." )
                         .arg( info->destinationPath() ).arg( info->sourcePath() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::searchingFailed( const QString &item, const QString &err_msg )
{
  m_err_msg = err_msg.split( "\n" );

  KNotification *notification = new KNotification( "searchingFailed", KNotification::Persistent );
  notification->setText( i18n( "Searching the network neighborhood for <b>%1</b> failed." ).arg( item ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::commandNotFound( const QString &command )
{
  KNotification *notification = KNotification::event( "commandNotFound",
                                i18n( "The command <b>%1</b> could not be found." )
                                .arg( command ),
                                KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


void Smb4KNotification::cannotBookmarkPrinter( Smb4KShare *share )
{
  if ( share->isPrinter() )
  {
    KNotification *notification = KNotification::event( "cannotBookmarkPrinter",
                                  i18n( "The share <b>%1</b> is a printer and cannot be bookmarked." )
                                  .arg( share->unc() ),
                                  KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
    connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::fileNotFound( const QString &fileName )
{
  KNotification *notification = KNotification::event( "fileNotFound",
                                i18n( "The file <b>%1</b> could not be found." )
                                .arg( fileName ),
                                KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


void Smb4KNotification::openingFileFailed( const QFile &file )
{
  m_err_msg = file.errorString().split( "\n" );

  KNotification *notification = new KNotification( "openingFileFailed", KNotification::Persistent );
  notification->setText( i18n( "Opening the file <b>%1</b> failed." ).arg( file.fileName() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::readingFileFailed( const QFile& file, const QString& err_msg )
{
  if ( !err_msg.isEmpty() )
  {
    m_err_msg = err_msg.split( "\n" );
  }
  else
  {
    m_err_msg = file.errorString().split( "\n" );
  }

  KNotification *notification = new KNotification( "readingFileFailed", KNotification::Persistent );
  notification->setText( i18n( "Reading from file <b>%1</b> failed." ).arg( file.fileName() ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}



void Smb4KNotification::mkdirFailed( const QDir &dir )
{
  KNotification *notification = KNotification::event( "mkdirFailed",
                                i18n( "The directory <b>%1</b> could not be created." )
                                .arg( dir.absolutePath() ),
                                KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );
  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );
}


void Smb4KNotification::missingPrograms( const QStringList &programs )
{
  m_err_msg = programs;

  KNotification *notification = new KNotification( "missingPrograms", KNotification::Persistent );
  notification->setText( i18n( "Some required programs could not be found." ) );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "List" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::processError( QProcess::ProcessError error )
{
  KNotification *notification = new KNotification( "processError", KNotification::Persistent );

  switch ( error )
  {
    case QProcess::FailedToStart:
    {
      notification->setText( i18n( "The process failed to start (error code: %1).", error ) );
      break;
    }
    case QProcess::Crashed:
    {
      notification->setText( i18n( "The process crashed (error code: %1).", error ) );
      break;
    }
    case QProcess::Timedout:
    {
      notification->setText( i18n( "The process timed out (error code: %1).", error ) );
      break;
    }
    case QProcess::WriteError:
    {
      notification->setText( i18n( "Could not write to the process (error code: %1).", error ) );
      break;
    }
    case QProcess::ReadError:
    {
      notification->setText( i18n( "Could not read from the process (error code: %1).", error ) );
      break;
    }
    case QProcess::UnknownError:
    default:
    {
      notification->setText( i18n( "The process reported an unknown error." ) );
      break;
    }
  }

  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::systemCallFailed( const QString &sys_call, int err_no )
{
  int len = 100;
  char buf[100];
  buf[0] = '\0';
#ifndef Q_OS_FREEBSD
  // This is thread safe.
  const char *msg;

  msg = strerror_r( err_no, buf, len );

  if ( buf[0] == '\0' )
  {
    // Buffer was not used
    m_err_msg += QString( msg );
  }
  else
  {
    m_err_msg += QString( buf );
  }
#else
  if ( strerror_r( err_no, buf, len ) == 0 )
  {
    m_err_msg += QString( buf );
  }
  else
  {
    // Do nothing
  }
#endif

  KNotification *notification = new KNotification( "systemCallFailed", KNotification::Persistent );

  if ( !sys_call.isEmpty() )
  {
    notification->setText( i18n( "The system call <b>%1</b> failed.", sys_call ) );
  }
  else
  {
    notification->setText( i18n( "A system call failed." ) );
  }

  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  if ( !m_err_msg.isEmpty() )
  {
    notification->setActions( QStringList( i18n( "Error Message" ) ) );
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotShowErrorMessage() ) );
  }
  else
  {
    // Do nothing
  }

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


void Smb4KNotification::actionFailed( AuthActions action, const QString &err_msg )
{
  KNotification *notification = new KNotification( "actionFailed", KNotification::Persistent );
  QString notification_text;

  switch ( action )
  {
    case MountAction:
    {
      notification->setText( i18n( "The execution of the mount action failed. The error code was: <tt>%1</tt>" ).arg( err_msg ) );
      break;
    }
    case UnmountAction:
    {
      notification->setText( i18n( "The execution of the unmount action failed. The error code was: <tt>%1</tt>" ).arg( err_msg ) );
    }
    default:
    {
      break;
    }
  }

  notification->setText( notification_text );
  notification->setPixmap( KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0, KIconLoader::DefaultState ) );

  connect( notification, SIGNAL( closed() ), this, SLOT( slotNotificationClosed() ) );

  notification->sendEvent();
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KNotification::slotNotificationClosed()
{
  delete this;
}


void Smb4KNotification::slotOpenShare()
{
  KRun::runUrl( KUrl( m_share.canonicalPath() ), "inode/directory", 0 );
}


void Smb4KNotification::slotShowErrorMessage()
{
  KMessageBox::errorList( 0, i18n( "The following error message was reported:" ), m_err_msg );
}



#include "smb4knotification.moc"
