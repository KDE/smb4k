/***************************************************************************
    smb4kpreviewer  -  This class queries a remote share for a preview
                             -------------------
    begin                : Mo Mai 28 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
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
#include <QCoreApplication>
#include <QDesktopWidget>

// KDE includes
#include <kapplication.h>
#include <kdebug.h>
#include <kshell.h>
#include <kstandarddirs.h>

// application specific includes
#include <smb4kpreviewer.h>
#include <smb4kpreviewitem.h>
#include <smb4kdefs.h>
#include <smb4kglobal.h>
#include <smb4kauthinfo.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4ksambaoptionsinfo.h>
#include <smb4kcoremessage.h>
#include <smb4kshare.h>
#include <smb4khomesshareshandler.h>
#include <smb4kpreviewer_p.h>
#include <smb4kwalletmanager.h>
#include <smb4kprocess.h>
#include <smb4ksettings.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KPreviewerPrivate, priv );


Smb4KPreviewer::Smb4KPreviewer() : QObject()
{
}


Smb4KPreviewer::~Smb4KPreviewer()
{
  // Do not delete m_item here, because it is owned
  // by a different object.
}


Smb4KPreviewer *Smb4KPreviewer::self()
{
  return &priv->instance;
}


void Smb4KPreviewer::preview( Smb4KPreviewItem *item )
{
  Q_ASSERT( item );

  // Find the smbclient program
  QString smbclient = KStandardDirs::findExe( "smbclient" );

  if ( smbclient.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smbclient" );
    return;
  }
  else
  {
    // Go ahead
  }

  // Process homes shares.
  if( item->share()->isHomesShare() )
  {
    QWidget *parent = 0;

    if ( kapp )
    {
      if ( kapp->activeWindow() )
      {
        parent = kapp->activeWindow();
      }
      else
      {
        parent = kapp->desktop();
      }
    }
    else
    {
      // Do nothing
    }

    if ( !Smb4KHomesSharesHandler::self()->specifyUser( item->share(), parent ) )
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

  // Get the authentication information.
  Smb4KAuthInfo authInfo( item->share() );
  Smb4KWalletManager::self()->readAuthInfo( &authInfo );

  // Compile the command.
  //
  // Here are some things to remember:
  // (a) Do not convert the path to local 8 bit. It won't work with umlauts or other
  //     special characters.
  // (b) Do not pass the path unquoted, or you'll get a NT_STATUS_OBJECT_NAME_NOT_FOUND
  //     error message in the case the path is empty.
  QString command;
  command += smbclient;
  command += " "+KShell::quoteArg( item->share()->unc() );
  command += " -W "+KShell::quoteArg( item->share()->workgroupName() );
  command += " -D "+KShell::quoteArg( item->path() );
  command += " -c "+KShell::quoteArg( "ls" );

  if ( !item->share()->hostIP().isEmpty() )
  {
    command += " -I "+item->share()->hostIP();
  }
  else
  {
    // Do nothing
  }
  
  // Machine account
  command += Smb4KSettings::machineAccount() ? " -P" : "";
  
  // Signing state
  switch ( Smb4KSettings::signingState() )
  {
    case Smb4KSettings::EnumSigningState::None:
    {
      break;
    }
    case Smb4KSettings::EnumSigningState::On:
    {
      command += " -S on";
      break;
    }
    case Smb4KSettings::EnumSigningState::Off:
    {
      command += " -S off";
      break;
    }
    case Smb4KSettings::EnumSigningState::Required:
    {
      command += " -S required";
      break;
    }
    default:
    {
      break;
    }
  }
  
  // Buffer size
  command += Smb4KSettings::bufferSize() != 65520 ? QString( " -b %1" ).arg( Smb4KSettings::bufferSize() ) : "";
  
  // Get global Samba and custom options
  QMap<QString,QString> samba_options = Smb4KSambaOptionsHandler::self()->globalSambaOptions();
  Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( item->share() );
  
  // Port
  command += (info && info->smbPort() != -1) ? QString( " -p %1" ).arg( info->smbPort() ) : 
             QString( " -p %1" ).arg( Smb4KSettings::remoteSMBPort() );
             
  // Kerberos
  if ( info )
  {
    switch ( info->useKerberos() )
    {
      case Smb4KSambaOptionsInfo::UseKerberos:
      {
        command += " -k";
        break;
      }
      case Smb4KSambaOptionsInfo::NoKerberos:
      {
        // No kerberos 
        break;
      }
      case Smb4KSambaOptionsInfo::UndefinedKerberos:
      {
        command += Smb4KSettings::useKerberos() ? " -k" : "";
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    command += Smb4KSettings::useKerberos() ? " -k" : "";
  }
  
  // Resolve order
  command += (!Smb4KSettings::nameResolveOrder().isEmpty() &&
             QString::compare( Smb4KSettings::nameResolveOrder(), samba_options["name resolve order"] ) != 0) ?
             QString( " -R %1" ).arg( KShell::quoteArg( Smb4KSettings::nameResolveOrder() ) ) : "";
             
  // NetBIOS name
  command += (!Smb4KSettings::netBIOSName().isEmpty() &&
             QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0) ?
             QString( " -n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) ) : "";
             
  // NetBIOS scope
  command += (!Smb4KSettings::netBIOSScope().isEmpty() &&
             QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0) ?
             QString( " -i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) ) : "";
  
  // Socket options
  command += (!Smb4KSettings::socketOptions().isEmpty() &&
             QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0) ?
             QString( " -O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) ) : "";

  if ( !authInfo.login().isEmpty() )
  {
    command += " -U "+KShell::quoteArg( authInfo.login() );
  }
  else
  {
    command += " -U %";
  }

  // Start the synchronization.
  if ( m_cache.size() == 0 )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    m_state = SYNCHRONIZER_START;
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( item );

  PreviewThread *thread = new PreviewThread( item, this );
  m_cache.insert( item->location(), thread );

  connect( thread, SIGNAL( finished() ), this, SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( result( Smb4KPreviewItem * ) ), this, SIGNAL( result( Smb4KPreviewItem * ) ) );
  connect( thread, SIGNAL( authError( Smb4KPreviewItem * ) ), this, SLOT( slotAuthError( Smb4KPreviewItem* ) ) );

  thread->start();
  thread->preview( &authInfo, command );
}


bool Smb4KPreviewer::isRunning( Smb4KPreviewItem *item )
{
  Q_ASSERT( item );

  PreviewThread *thread = m_cache.object( item->location() );
  return (thread && thread->process() && thread->process()->state() == KProcess::Running);
}


void Smb4KPreviewer::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QStringList keys = m_cache.keys();

    foreach ( const QString &key, keys )
    {
      PreviewThread *thread = m_cache.object( key );

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


void Smb4KPreviewer::abort( Smb4KPreviewItem *item )
{
  Q_ASSERT( item );

  PreviewThread *thread = m_cache.object( item->location() );

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


bool Smb4KPreviewer::isAborted( Smb4KPreviewItem *item )
{
  Q_ASSERT( item );

  PreviewThread *thread = m_cache.object( item->location() );
  return (thread && thread->process() && thread->process()->isAborted());
}


/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPreviewer::slotAuthError( Smb4KPreviewItem *item )
{
  Smb4KAuthInfo authInfo;
  authInfo.setWorkgroupName( item->share()->workgroupName() );
  authInfo.setUNC( item->share()->unc( QUrl::None ) );

  if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
  {
    // Retry the preview.
    preview( item );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewer::slotThreadFinished()
{
  QStringList keys = m_cache.keys();

  foreach ( const QString &key, keys )
  {
    PreviewThread *thread = m_cache.object( key );

    if ( thread->isFinished() )
    {
      (void) m_cache.take( key );
      emit finished( thread->previewItem() );
      delete thread;
    }
    else
    {
      // Do not touch the thread
    }
  }

  if ( m_cache.size() == 0 )
  {
    m_state = PREVIEWER_STOP;
    emit stateChanged();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Still running
  }
}

#include "smb4kpreviewer.moc"
