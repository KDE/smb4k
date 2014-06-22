/***************************************************************************
    smb4knotification  -  This class provides notifications for Smb4K.
                             -------------------
    begin                : Son Jun 27 2010
    copyright            : (C) 2010-2014 by Alexander Reinholdt
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
#include "smb4knotification.h"
#include "smb4knotification_p.h"
#include "smb4ksettings.h"
#include "smb4kbookmark.h"
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <krun.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kdemacros.h>
#include <kauthactionreply.h>

using namespace KAuth;

// K_GLOBAL_STATIC(Smb4K

//
// Notifications
//

void Smb4KNotification::shareMounted(Smb4KShare* share)
{
  Q_ASSERT(share);
  
  if (share)
  {
    KNotification *notification = new KNotification("shareMounted");
    notification->setText(i18n( "<p>The share <b>%1</b> has been mounted to <b>%2</b>.</p>", 
                          share->unc(), share->path() ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, 
                            KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ));
    notification->setActions(QStringList( i18n( "Open" ) ));
    notification->setFlags(KNotification::CloseOnTimeout);
    
    Smb4KNotificationActionRunner *runner = new Smb4KNotificationActionRunner(notification);
    runner->setMountpoint(share->path());
    runner->connect(notification, SIGNAL(activated(uint)), SLOT(slotOpenShare()));
    
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::shareUnmounted(Smb4KShare* share)
{
  Q_ASSERT(share);
  
  if (share)
  {
    KNotification *notification = new KNotification("shareUnmounted");
    notification->setText(i18n( "<p>The share <b>%1</b> has been unmounted from <b>%2</b>.</p>", 
                          share->unc(), share->path() ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, 
                            KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ));
    notification->setFlags(KNotification::CloseOnTimeout);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::sharesMounted(int total, int actual)
{
  if (total != actual)
  {
    KNotification *notification = new KNotification("sharesMounted");
    notification->setText(i18np( "<p>%1 share out of %2 has been mounted.</p>", 
                                 "<p>%1 shares out of %2 have been mounted.</p>", actual, total ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "folder-remote", 
                            KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ));
    notification->setFlags(KNotification::CloseOnTimeout);
    notification->sendEvent();
  }
  else
  {
    KNotification *notification = new KNotification("sharesMounted");
    notification->setText(i18n( "<p>All shares have been mounted.</p>" ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "folder-remote", 
                            KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ));
    notification->setFlags(KNotification::CloseOnTimeout);
    notification->sendEvent();
  }
}


void Smb4KNotification::sharesUnmounted(int total, int actual)
{
  if (total != actual)
  {
    KNotification *notification = new KNotification("sharesUnmounted");
    notification->setText(i18np( "<p>%1 share out of %2 has been unmounted.</p>", 
                                 "<p>%1 shares out of %2 have been unmounted.</p>", actual, total ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, 
                            KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ));
    notification->setFlags(KNotification::CloseOnTimeout);
    notification->sendEvent();
  }
  else
  {
    KNotification *notification = new KNotification("sharesUnmounted");
    notification->setText(i18n( "<p>All shares have been unmounted.</p>" ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, 
                            KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ));
    notification->setFlags(KNotification::CloseOnTimeout);
    notification->sendEvent();
  }
}


//
// Warnings
//

void Smb4KNotification::openingWalletFailed(const QString& name)
{
  KNotification *notification = new KNotification("openingWalletFailed");
  notification->setText(i18n( "<p>Opening the wallet <b>%1</b> failed.</p>", name ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, 
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::CloseOnTimeout);
  notification->sendEvent();
}


void Smb4KNotification::credentialsNotAccessible()
{
  KNotification *notification = new KNotification("credentialsNotAccessible");
  notification->setText(i18n( "<p>The credentials stored in the wallet could not be accessed. "
                              "There is either no wallet available or it could not be opened.</p>" ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, 
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::CloseOnTimeout);
  notification->sendEvent();
}


void Smb4KNotification::mimetypeNotSupported(const QString& mimetype)
{
  KNotification *notification = new KNotification("mimetypeNotSupported");
  notification->setText(i18n( "<p>The mimetype <b>%1</b> is not supported for printing. "
                              "Please convert the file to PDF or Postscript and try again.</p>", mimetype ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, 
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::CloseOnTimeout);
  notification->sendEvent();
}


void Smb4KNotification::bookmarkExists(Smb4KBookmark* bookmark)
{
  Q_ASSERT(bookmark);
  
  if (bookmark)
  {
    KNotification *notification = new KNotification("bookmarkExists");
    notification->setText(i18n( "<p>The bookmark for share <b>%1</b> already exists and will be skipped.</p>",
                          bookmark->unc() ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, 
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::CloseOnTimeout);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::bookmarkLabelInUse(Smb4KBookmark* bookmark)
{
  Q_ASSERT(bookmark);
  
  if (bookmark)
  {
    KNotification *notification = new KNotification("bookmarkLabelInUse");
    notification->setText(i18n( "<p>The label <b>%1</b> of the bookmark for the share <b>%2</b> "
                                "is already being used and will automatically be renamed.</p>", 
                                bookmark->label(), bookmark->unc() ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0, 
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::CloseOnTimeout);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::emptyCustomMasterBrowser()
{
  KNotification *notification = new KNotification("emptyMasterBrowser");
  notification->setText(i18n( "The entry of the custom master browser is empty. Smb4K is going to "
                              "try to query the current master browser of your workgroup or domain instead." ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-warning", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::CloseOnTimeout);
  notification->sendEvent();
}


//
// Errors
//

void Smb4KNotification::retrievingDomainsFailed(const QString& err_msg)
{
  QString text;
  
  if ( !err_msg.isEmpty() )
  {
    text = i18n( "<p>Retrieving the list of available domains failed:</p><p><tt>%1</tt></p>", err_msg );
  }
  else
  {
    text = i18n( "<p>Retrieving the list of available domains failed.</p>" );
  }
  
  KNotification *notification = new KNotification("retrievingDomainsFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();
}


void Smb4KNotification::scanningBroadcastAreaFailed(const QString& err_msg)
{
  QString text;
  
  if ( !err_msg.isEmpty() )
  {
    text = i18n( "<p>Scanning the defined broadcast area(s) failed:</p><p><tt>%1</tt></p>", err_msg );
  }
  else
  {
    text = i18n( "<p>Scanning the defined broadcast area(s) failed.</p>" );
  }
  
  KNotification *notification = new KNotification("scanningBroadcastAreaFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();
}


void Smb4KNotification::retrievingHostsFailed(Smb4KWorkgroup* workgroup, const QString& err_msg)
{
  Q_ASSERT(workgroup);
  
  if (workgroup)
  {
    QString text;
    
    if ( !err_msg.isEmpty() )
    {
      text = i18n( "<p>Retrieving the list of hosts belonging to domain <b>%1</b> failed.</p><p><tt>%2</tt></p>", workgroup->workgroupName(), err_msg );
    }
    else
    {
      text = i18n( "<p>Retrieving the list of hosts belonging to domain <b>%1</b> failed.</p>", workgroup->workgroupName() );
    }

    KNotification *notification = new KNotification("retrievingHostsFailed");
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::retrievingSharesFailed(Smb4KHost* host, const QString& err_msg)
{
  Q_ASSERT(host);
  
  if (host)
  {
    QString text;
    
    if ( !err_msg.isEmpty() )
    {
      text = i18n( "<p>Retrieving the list of shares from <b>%1</b> failed:</p><p><tt>%2</tt></p>", host->hostName(), err_msg );
    }
    else
    {
      text = i18n( "<p>Retrieving the list of shares from <b>%1</b> failed.</p>", host->hostName() );
    }
    
    KNotification *notification = new KNotification("retrievingSharesFailed");
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::retrievingPreviewFailed(Smb4KShare* share, const QString& err_msg)
{
  Q_ASSERT(share);
  
  if (share)
  {
    QString text;
    
    if ( !err_msg.isEmpty() )
    {
      text =  i18n( "<p>Retrieving the preview of <b>%1</b> failed:</p><p><tt>%2</tt></p>", share->unc(), err_msg );
    }
    else
    {
      text =  i18n( "<p>Retrieving the preview of <b>%1</b> failed.</p>", share->unc() );
    }
    
    KNotification *notification = new KNotification("retrievingPreviewFailed");
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::mountingFailed(Smb4KShare* share, const QString& err_msg)
{
  Q_ASSERT(share);
  
  if (share)
  {
    QString text;
    
    if ( !err_msg.isEmpty() )
    {
      text = i18n( "<p>Mounting the share <b>%1</b> failed:</p><p><tt>%2</tt></p>", share->unc(), err_msg );
    }
    else
    {
      text = i18n( "<p>Mounting the share <b>%1</b> failed.</p>", share->unc() );
    }

    KNotification *notification = new KNotification("mountingFailed");
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::unmountingFailed(Smb4KShare* share, const QString& err_msg)
{
  Q_ASSERT(share);
  
  if (share)
  {
    QString text;
    
    if ( !err_msg.isEmpty() )
    {
      text = i18n( "<p>Unmounting the share <b>%1</b> from <b>%2</b> failed:</p><p><tt>%3</tt></p>", share->unc(), share->path(), err_msg );
    }
    else
    {
      text = i18n( "<p>Unmounting the share <b>%1</b> from <b>%2</b> failed.</p>", share->unc(), share->path() );
    }
    
    KNotification *notification = new KNotification("unmountingFailed");
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::unmountingNotAllowed(Smb4KShare* share)
{
  Q_ASSERT(share);
  
  if (share)
  {
    KNotification *notification = new KNotification("unmountingNotAllowed");
    notification->setText(i18n( "<p>You are not allowed to unmount the share <b>%1</b> from <b>%2</b>. "
                                "It is owned by the user <b>%3</b>.</p>", share->unc(), share->path(), share->owner() ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::printingFailed(Smb4KShare* printer, const QString& err_msg)
{
  Q_ASSERT(printer);
  
  if (printer)
  {
    QString text;
    
    if ( !err_msg.isEmpty() )
    {
      text = i18n( "<p>Printing on printer <b>%1</b> failed:</p><p><tt>%2</tt></p>", printer->unc(), err_msg );
    }
    else
    {
      text = i18n( "<p>Printing on printer <b>%1</b> failed.</p>", printer->unc() );
    }
    
    KNotification *notification = new KNotification("printingFailed");
    notification->setText(text);
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::synchronizationFailed(const KUrl& src, const KUrl& dest, const QString& err_msg)
{
  QString text;
  
  if ( !err_msg.isEmpty() )
  {
    text = i18n( "<p>Synchronizing <b>%1</b> with <b>%2</b> failed:</p><p><tt>%3</tt></p>", dest.path(), src.path(), err_msg );
  }
  else
  {
    text = i18n( "<p>Synchronizing <b>%1</b> with <b>%2</b> failed.</p>", dest.path(), src.path() );
  }
  
  KNotification *notification = new KNotification("synchronizationFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();  
}


void Smb4KNotification::searchingFailed(const QString& item, const QString& err_msg)
{
  QString text;
  
  if ( !err_msg.isEmpty() )
  {
    text = i18n( "<p>Searching the network neighborhood for the search item <b>%1</b> failed:</p><p><tt>%2</tt></p>", item, err_msg );
  }
  else
  {
    text = i18n( "<p>Searching the network neighborhood for the search item <b>%1</b> failed.</p>", item );
  }
  
  KNotification *notification = new KNotification("searchingFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();  
}


void Smb4KNotification::commandNotFound(const QString& command)
{
  KNotification *notification = new KNotification("commandNotFound");
  notification->setText(i18n( "<p>The command <b>%1</b> could not be found. Please check your installation.</p>", command ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent(); 
}


void Smb4KNotification::cannotBookmarkPrinter(Smb4KShare* share)
{
  Q_ASSERT(share);
  
  if (share && share->isPrinter())
  {
    KNotification *notification = new KNotification("cannotBookmarkPrinter");
    notification->setText(i18n( "<p>The share <b>%1</b> is a printer and cannot be bookmarked.</p>", share->unc() ));
    notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                            KIconLoader::DefaultState ));
    notification->setFlags(KNotification::Persistent);
    notification->sendEvent(); 
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::fileNotFound(const QString& fileName)
{
  KNotification *notification = new KNotification("fileNotFound");
  notification->setText(i18n( "<p>The file <b>%1</b> could not be found.</p>", fileName ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent(); 
}


void Smb4KNotification::openingFileFailed(const QFile& file)
{
  QString text;

  if ( !file.errorString().isEmpty() )
  {
    text = i18n( "<p>Opening the file <b>%1</b> failed:</p><p><tt>%2</tt></p>", file.fileName(), file.errorString() );
  }
  else
  {
    text = i18n( "<p>Opening the file <b>%1</b> failed.</p>", file.fileName() );
  }
  
  KNotification *notification = new KNotification("openingFileFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();
}



void Smb4KNotification::readingFileFailed(const QFile& file, const QString& err_msg)
{
  QString text;
  
  if ( !err_msg.isEmpty() )
  {
    text = i18n( "<p>Reading from file <b>%1</b> failed:</p><p><tt>%2</tt></p>", file.fileName(), err_msg );
  }
  else
  {
    if ( !file.errorString().isEmpty() )
    {
      text = i18n( "<p>Reading from file <b>%1</b> failed:</p><p><tt>%2</tt></p>", file.fileName(), file.errorString() );
    }
    else
    {
      text = i18n( "<p>Reading from file <b>%1</b> failed.</p>", file.fileName() );
    }
  }
  
  KNotification *notification = new KNotification("readingFileFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();
}


void Smb4KNotification::mkdirFailed(const QDir& dir)
{
  KNotification *notification = new KNotification("mkdirFailed");
  notification->setText(i18n( "<p>The following directory could not be created:</p><p><tt>%1</tt></p>", dir.absolutePath() ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent(); 
}


void Smb4KNotification::processError(QProcess::ProcessError error)
{
  QString text;

  switch ( error )
  {
    case QProcess::FailedToStart:
    {
      text = i18n( "<p>The process failed to start (error code: <tt>%1</tt>).</p>", error );
      break;
    }
    case QProcess::Crashed:
    {
      text = i18n( "<p>The process crashed (error code: <tt>%1</tt>).</p>", error );
      break;
    }
    case QProcess::Timedout:
    {
      text = i18n( "<p>The process timed out (error code: <tt>%1</tt>).</p>", error );
      break;
    }
    case QProcess::WriteError:
    {
      text = i18n( "<p>Could not write to the process (error code: <tt>%1</tt>).</p>", error );
      break;
    }
    case QProcess::ReadError:
    {
      text = i18n( "<p>Could not read from the process (error code: <tt>%1</tt>).</p>", error );
      break;
    }
    case QProcess::UnknownError:
    default:
    {
      text = i18n( "<p>The process reported an unknown error.</p>" );
      break;
    }
  }
  
  KNotification *notification = new KNotification("processError");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();
}


void Smb4KNotification::systemCallFailed(const QString& sys_call, int err_no)
{
  QString text;
  
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
    text = i18n( "<p>The system call <b>%1</b> failed:</p><p><tt>%2</tt></p>", sys_call, QString( msg ) );
  }
  else
  {
    text = i18n( "<p>The system call <b>%1</b> failed:</p><p><tt>%2</tt></p>", sys_call, QString( buf ) );
  }
#else
  if ( strerror_r( err_no, buf, len ) == 0 )
  {
    text = i18n( "<p>The system call <b>%1</b> failed:</p><p><tt>%2</tt></p>", sys_call, QString( buf ) );
  }
  else
  {
    // Do nothing
  }
#endif

  KNotification *notification = new KNotification("systemCallFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();
}


void Smb4KNotification::actionFailed(int err_code)
{
  QString text, err_msg;
  
  switch ( err_code )
  {
    case ActionReply::NoResponder:
    {
      err_msg = "NoResponder";
      break;
    }
    case ActionReply::NoSuchAction:
    {
      err_msg = "NoSuchAction";
      break;
    }
    case ActionReply::InvalidAction:
    {
      err_msg = "InvalidAction";
      break;
    }
    case ActionReply::AuthorizationDenied:
    {
      err_msg = "AuthorizationDenied";
      break;
    }
    case ActionReply::UserCancelled:
    {
      err_msg = "UserCancelled";
      break;
    }
    case ActionReply::HelperBusy:
    {
      err_msg = "HelperBusy";
      break;
    }
    case ActionReply::DBusError:
    {
      err_msg = "DBusError";
      break;
    }
    default:
    {
      break;
    }
  }

  if ( !err_msg.isEmpty() )
  {
    text = i18n( "<p>Executing an action with root privileges failed (error code:<tt>%1</tt>).</p>", err_msg );
  }
  else
  {
    text = i18n( "<p>Executing an action with root privileges failed.</p>" );
  }
  
  KNotification *notification = new KNotification("actionFailed");
  notification->setText(text);
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();  
}


void Smb4KNotification::invalidURLPassed()
{
  KNotification *notification = new KNotification("invalidURL");
  notification->setText(i18n( "<p>The URL that was passed is invalid.</p>" ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent(); 
}


void Smb4KNotification::emptyBroadcastAreas()
{
  KNotification *notification = new KNotification("emptyBroadcastAreas");
  notification->setText(i18n( "<p>There are no broadcast areas defined.</p>" ));
  notification->setPixmap(KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::NoGroup, 0,
                          KIconLoader::DefaultState ));
  notification->setFlags(KNotification::Persistent);
  notification->sendEvent();
}

