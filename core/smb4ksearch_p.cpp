/***************************************************************************
    smb4ksearch_p  -  Private helper classes for Smb4KSearch class.
                             -------------------
    begin                : Mo Dez 22 2008
    copyright            : (C) 2008-2013 by Alexander Reinholdt
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

// application specific inludes
#include "smb4ksearch_p.h"
#include "smb4knotification.h"
#include "smb4kglobal.h"
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4ksettings.h"
#include "smb4kauthinfo.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"

// Qt includes
#include <QtCore/QTimer>
#include <QtCore/QLatin1String>

// KDE includes
#include <kstandarddirs.h>
#include <kshell.h>

using namespace Smb4KGlobal;


Smb4KSearchJob::Smb4KSearchJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_master( NULL ), m_parent_widget( NULL ), m_proc( NULL )
{
  setCapabilities( KJob::Killable );
}


Smb4KSearchJob::~Smb4KSearchJob()
{
  delete m_master;
}


void Smb4KSearchJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT(slotStartSearch()) );
}


void Smb4KSearchJob::setupSearch( const QString &string, Smb4KHost *master, QWidget *parentWidget )
{
  Q_ASSERT( !string.trimmed().isEmpty() );
  m_string = string;
  m_master = master ? new Smb4KHost( *master ) : NULL;
  m_parent_widget = parentWidget;
}


bool Smb4KSearchJob::doKill()
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


void Smb4KSearchJob::slotStartSearch()
{
  QStringList arguments;

  emit aboutToStart( m_string );

  // Find smbtree program.
  QString smbtree = KStandardDirs::findExe( "smbtree" );

  if ( smbtree.isEmpty() )
  {
    Smb4KNotification::commandNotFound("smbtree");
    emitResult();
    emit finished(m_string);
    return;
  }
  else
  {
    // Go ahead
  }
  
  //
  // Global Samba options
  //
  QMap<QString,QString> globalOptions = globalSambaOptions();

  // Lookup the custom options that are defined for the master browser.
  Smb4KCustomOptions *options = NULL;

  if ( m_master )
  {
    options = Smb4KCustomOptionsManager::self()->findOptions( m_master );
  }
  else
  {
    // Do nothing
  }

  // Compile the command
  arguments << smbtree;
  arguments << "-d2";

  // Kerberos
  if ( options )
  {
    switch ( options->useKerberos() )
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        arguments << "-k";
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        // No kerberos
        break;
      }
      case Smb4KCustomOptions::UndefinedKerberos:
      {
        if ( Smb4KSettings::useKerberos() )
        {
          arguments << "-k";
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
  }
  else
  {
    if ( Smb4KSettings::useKerberos() )
    {
      arguments << "-k";
    }
    else
    {
      // Do nothing
    }
  }

  // Machine account
  if ( Smb4KSettings::machineAccount() )
  {
    arguments << "-P";
  }
  else
  {
    // Do nothing
  }

  // Signing state
  switch ( Smb4KSettings::signingState() )
  {
    case Smb4KSettings::EnumSigningState::None:
    {
      break;
    }
    case Smb4KSettings::EnumSigningState::On:
    {
      arguments << "-S on";
      break;
    }
    case Smb4KSettings::EnumSigningState::Off:
    {
      arguments << "-S off";
      break;
    }
    case Smb4KSettings::EnumSigningState::Required:
    {
      arguments << "-S required";
      break;
    }
    default:
    {
      break;
    }
  }

  // Send broadcasts
  if ( Smb4KSettings::smbtreeSendBroadcasts() )
  {
    arguments << "-b";
  }
  else
  {
    // Do nothing
  }

  // Use Winbind CCache
  if ( Smb4KSettings::useWinbindCCache() )
  {
    arguments << "-C";
  }
  else
  {
    // Do nothing
  }

  // Use encryption
  if ( Smb4KSettings::encryptSMBTransport() )
  {
    arguments << "-e";
  }
  else
  {
    // Do nothing
  }

  if ( m_master && !m_master->login().isEmpty() )
  {
    arguments << QString( "-U %1" ).arg( m_master->login() );
  }
  else
  {
    arguments << "-U %";
  }
  
  //
  // Workaround problems with security fixes of April 12, 2016.
  //
  if (globalOptions.contains("client max protocol") && globalOptions.value("client max protocol").startsWith(QLatin1String("SMB")))
  {
    arguments << "--option='client max protocol=NT1'";
  }
  else
  {
    // Do nothing
  }

  m_proc = new Smb4KProcess( this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setEnv( "PASSWD", (m_master && !m_master->password().isEmpty()) ? m_master->password() : "", true );
  m_proc->setShellCommand( arguments.join( " " ) );
  
  qDebug() << arguments.join(" ");
  
  connect( m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadStandardOutput()) );
  connect( m_proc, SIGNAL(readyReadStandardError()), this, SLOT(slotReadStandardError()) );
  connect( m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)) );
  
  m_proc->start();
}


void Smb4KSearchJob::slotReadStandardOutput()
{
  QStringList stdOut = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( '\n', QString::SkipEmptyParts, Qt::CaseSensitive );

  // Process output from smbtree.
  QString workgroup_name;

  foreach ( const QString &line, stdOut )
  {
    if ( !line.contains( "added interface", Qt::CaseInsensitive ) &&
         !line.contains( "tdb(", Qt::CaseInsensitive ) &&
         !line.contains( "Got a positive", Qt::CaseInsensitive ) &&
         !line.contains( "error connecting", Qt::CaseInsensitive ) &&
         !line.isEmpty() )
    {
      if ( !line.contains( "\\" ) && !line.trimmed().isEmpty() )
      {
        workgroup_name = line.trimmed();
        continue;
      }
      else if ( line.count( "\\" ) == 3 )
      {
        QString unc = line.trimmed().section( '\t', 0, 0 ).trimmed().replace( "\\", "/" );
        QString comment = line.trimmed().section( '\t', 1, -1 ).trimmed();

        if ( unc.contains( m_string, Qt::CaseInsensitive ) )
        {
          Smb4KShare *share = new Smb4KShare();
          share->setURL( unc );
          share->setComment( comment );
          share->setWorkgroupName( workgroup_name );

          emit result( share );
          delete share;
        }
        else
        {
          // Do nothing
        }
        continue;
      }
      else
      {
        continue;
      }
    }
    else
    {
      continue;
    }
  }
}


void Smb4KSearchJob::slotReadStandardError()
{
  QString stdErr = QString::fromUtf8( m_proc->readAllStandardError(), -1 );

  // Remove unimportant warnings
  if ( stdErr.contains( "Ignoring unknown parameter" ) )
  {
    QStringList tmp = stdErr.split( '\n' );

    QMutableStringListIterator it( tmp );

    while ( it.hasNext() )
    {
      QString test = it.next();

      if ( test.trimmed().startsWith( QLatin1String( "Ignoring unknown parameter" ) ) )
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
    }

    stdErr = tmp.join( "\n" );
  }
  else
  {
    // Do nothing
  }

  // Process authentication errors:
  if ( stdErr.contains( "The username or password was not correct." ) ||
       stdErr.contains( "NT_STATUS_ACCOUNT_DISABLED" ) /* AD error */ ||
       stdErr.contains( "NT_STATUS_ACCESS_DENIED" ) ||
       stdErr.contains( "NT_STATUS_LOGON_FAILURE" ) )
  {
    m_proc->abort();
    emit authError( this );
  }
  else if ( stdErr.contains("NT_STATUS") )
  {
    Smb4KNotification::searchingFailed(m_string, stdErr);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSearchJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus status )
{
  switch ( status )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification::processError(m_proc->error());
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
  
  emitResult();
  emit finished(m_string);
}


#include "smb4ksearch_p.moc"

