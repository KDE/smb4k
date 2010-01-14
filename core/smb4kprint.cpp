/***************************************************************************
    smb4kprint  -  The printing core class.
                             -------------------
    begin                : Tue Mar 30 2004
    copyright            : (C) 2004-2009 by Alexander Reinholdt
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
#include <kurl.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <kshell.h>
#include <kapplication.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

// application specific includes
#include <smb4kprint.h>
#include <smb4kdefs.h>
#include <smb4kcoremessage.h>
#include <smb4kglobal.h>
#include <smb4kauthinfo.h>
#include <smb4ksettings.h>
#include <smb4kshare.h>
#include <smb4kprint_p.h>
#include <smb4kwalletmanager.h>
#include <smb4kprintinfo.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KPrintPrivate, priv );


Smb4KPrint::Smb4KPrint()
: QObject()
{
  m_state = PRINT_STOP;

  connect( kapp, SIGNAL( aboutToQuit() ),
           this, SLOT( slotAboutToQuit() ) );
}


Smb4KPrint::~Smb4KPrint()
{
}


Smb4KPrint *Smb4KPrint::self()
{
  return &priv->instance;
}


void Smb4KPrint::print( Smb4KPrintInfo *printInfo )
{
  Q_ASSERT( printInfo );

  // Find the smbspool program
  QString smbspool = KStandardDirs::findExe( "smbspool" );

  if ( smbspool.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smbspool" );
    return;
  }
  else
  {
    // Go ahead
  }

  // Find the dvips program
  QString dvips = KStandardDirs::findExe( "dvips" );

  // Find the enscript program
  QString enscript = KStandardDirs::findExe( "enscript" );

  if ( QFile::exists( printInfo->filePath() ) )
  {
    // Get the authentication information.
    Smb4KAuthInfo authInfo( printInfo->printer() );
    Smb4KWalletManager::self()->readAuthInfo( &authInfo );

    // Set the authentication information for the printer.
    printInfo->setAuthInfo( &authInfo );

    // Set the temporary file for conversion purposes.
    KTemporaryFile temp_file;
    temp_file.setAutoRemove( true );
    temp_file.setSuffix( ".ps" );

    // Set up a KFileItem object to get the mimetype later on. It is
    // needed to be able to compile the command.
    KUrl url;
    url.setPath( printInfo->filePath() );
    KFileItem file_item = KFileItem( KFileItem::Unknown, KFileItem::Unknown, url, false );

    // Compile the command.
    QString command;

    if ( QString::compare( file_item.mimetype(), "application/postscript" ) == 0 ||
         QString::compare( file_item.mimetype(), "application/pdf" ) == 0 ||
         file_item.mimetype().startsWith( "image" ) )
    {
      // Nothing to do here. These mimetypes can be directly
      // printed.
    }
    else if ( QString::compare( file_item.mimetype(), "application/x-dvi" ) == 0 &&
              !dvips.isEmpty() )
    {
      // Create the temporary file.
      if ( !temp_file.open() )
      {
        Smb4KCoreMessage::error( ERROR_CREATING_TEMP_FILE, QString(), temp_file.errorString() );
        return;
      }
      else
      {
        // Do nothing
      }

      temp_file.close();

      command += dvips;
      command += " -Ppdf";
      command += " -o "+KShell::quoteArg( temp_file.fileName() );
      command += " "+KShell::quoteArg( printInfo->filePath() );
      command += " && ";
    }
    else if ( (file_item.mimetype().startsWith( "text" ) ||
              file_item.mimetype().startsWith( "message" ) ||
              QString::compare( file_item.mimetype(), "application/x-shellscript" ) == 0) &&
              !enscript.isEmpty() )
    {
      // Create the temporary file.
      if ( !temp_file.open() )
      {
        Smb4KCoreMessage::error( ERROR_CREATING_TEMP_FILE, QString(), temp_file.errorString() );
        return;
      }
      else
      {
        // Do nothing
      }

      temp_file.close();

      command += enscript;
      command += " --quiet";
      command += " --columns=1 --no-header --ps-level=2";
      command += " -o "+KShell::quoteArg( temp_file.fileName() );
      command += " "+KShell::quoteArg( printInfo->filePath() );
      command += " && ";
    }
    else
    {
      Smb4KCoreMessage::information( INFO_MIMETYPE_NOT_SUPPORTED, file_item.mimetype(), QString() );
      return;
    }

    command += smbspool;
    command += " 111";
    command += " "+KUser( getuid() ).loginName();
    command += " \"Smb4K print job\"";
    command += " "+QString( "%1" ).arg( printInfo->copies() );
    command += " \"\"";

    if ( temp_file.exists() )
    {
      command += " "+KShell::quoteArg( temp_file.fileName() );
    }
    else
    {
      command += " "+KShell::quoteArg( printInfo->filePath() );
    }

    // Start printing the file.
    if ( m_cache.size() == 0 )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      m_state = PRINT_START;
      emit stateChanged();
    }
    else
    {
      // Already running
    }

    emit aboutToStart( printInfo );

    PrintThread *thread = new PrintThread( printInfo, this );
    m_cache.insert( printInfo->filePath(), thread );

    connect( thread, SIGNAL( finished() ), this, SLOT( slotThreadFinished() ) );
    connect( thread, SIGNAL( authError( Smb4KPrintInfo * ) ), this, SLOT( slotAuthError( Smb4KPrintInfo* ) ) );

    if ( temp_file.exists() )
    {
      thread->setTempFilePath( temp_file.fileName() );
    }
    else
    {
      // Do nothing
    }

    thread->start();
    thread->print( &authInfo, command );
  }
  else
  {
    // Show error message an exit.
    Smb4KCoreMessage::error( ERROR_FILE_NOT_FOUND, printInfo->filePath(), QString() );
  }
}


void Smb4KPrint::abort( Smb4KPrintInfo *printInfo )
{
  Q_ASSERT( printInfo );

  PrintThread *thread = m_cache.object( printInfo->filePath() );

  if ( thread && thread->process() && (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
  {
    thread->process()->abort();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KPrint::isAborted( Smb4KPrintInfo *printInfo )
{
  Q_ASSERT( printInfo );

  PrintThread *thread = m_cache.object( printInfo->filePath() );
  return (thread && thread->process() && thread->process()->isAborted());
}


void Smb4KPrint::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QStringList keys = m_cache.keys();

    foreach ( const QString &key, keys )
    {
      PrintThread *thread = m_cache.object( key );

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


bool Smb4KPrint::isRunning( Smb4KPrintInfo *printInfo )
{
  Q_ASSERT( printInfo );

  PrintThread *thread = m_cache.object( printInfo->filePath() );
  return (thread && thread->process() && thread->process()->state() == KProcess::Running);
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPrint::slotAboutToQuit()
{
  abortAll();
}


void Smb4KPrint::slotAuthError( Smb4KPrintInfo *printInfo )
{
  Smb4KAuthInfo authInfo;
  authInfo.setWorkgroupName( printInfo->printer()->workgroupName() );
  authInfo.setUNC( printInfo->printer()->unc( QUrl::None ) );

  if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
  {
    // Retry the search.
    print( printInfo );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPrint::slotThreadFinished()
{
  QStringList keys = m_cache.keys();

  foreach ( const QString &key, keys )
  {
    PrintThread *thread = m_cache.object( key );

    if ( thread->isFinished() )
    {
      (void) m_cache.take( key );
      emit finished( thread->printInfo() );
      delete thread;
    }
    else
    {
      // Do not touch the thread
    }
  }

  if ( m_cache.size() == 0 )
  {
    m_state = PRINT_STOP;
    emit stateChanged();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Still running
  }
}

#include "smb4kprint.moc"
