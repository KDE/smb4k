/***************************************************************************
    smb4kpreviewer_p  -  Private helper classes for Smb4KPreviewer class.
                             -------------------
    begin                : So Dez 21 2008
    copyright            : (C) 2008-2012 by Alexander Reinholdt
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
#include "smb4kpreviewer_p.h"
#include "smb4knotification.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kglobal.h"
#include "smb4ksettings.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"

// Qt includes
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QGridLayout>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kshell.h>

using namespace Smb4KGlobal;


Smb4KPreviewJob::Smb4KPreviewJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_share( NULL ), m_parent_widget( NULL ), m_proc( NULL )
{
}


Smb4KPreviewJob::~Smb4KPreviewJob()
{
}


void Smb4KPreviewJob::start()
{
  m_started = true;
  QTimer::singleShot( 0, this, SLOT(slotStartPreview()) );
}


void Smb4KPreviewJob::setupPreview( Smb4KShare *share, const KUrl &url, QWidget *parent )
{
  Q_ASSERT( share );
  m_share = share;
  m_url   = url;
  m_parent_widget = parent;
}


bool Smb4KPreviewJob::doKill()
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


void Smb4KPreviewJob::slotStartPreview()
{
  // Find the smbclient program
  QString smbclient = KStandardDirs::findExe( "smbclient" );

  if ( smbclient.isEmpty() )
  {
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "smbclient" );
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }

  // Start the preview process
  emit aboutToStart( m_share, m_url );

  // Get the path that has to be listed.
  QString path = m_url.path();
  
  if ( !m_share->isHomesShare() )
  {
    path.remove( m_share->shareName(), Qt::CaseInsensitive ); 
  }
  else
  {
    path.remove( m_share->login(), Qt::CaseInsensitive );
  }
  
  // Compile the command line arguments
  QStringList arguments;
  
  // smbclient command
  arguments << smbclient;

  // UNC
  arguments << m_share->unc();

  // Workgroup
  arguments << QString( "-W %1" ).arg( KShell::quoteArg( m_share->workgroupName() ) );

  // Directory
  arguments << QString( "-D %1" ).arg( KShell::quoteArg( path.isEmpty() ? "/" : path ) );

  // Command to perform (here: ls)
  arguments << QString( "-c %1" ).arg( KShell::quoteArg( "ls" ) );

  // IP address
  if ( !m_share->hostIP().isEmpty() )
  {
    arguments << QString( "-I %1" ).arg( m_share->hostIP() );
  }
  else
  {
    // Do nothing
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

  // Buffer size
  if ( Smb4KSettings::bufferSize() != 65520 )
  {
    arguments << QString( "-b %1" ).arg( Smb4KSettings::bufferSize() );
  }
  else
  {
    // Do nothing
  }

  // Get global Samba and custom options
  QMap<QString,QString> samba_options = globalSambaOptions();
  Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions( m_share );

  // Port
  if ( options && options->smbPort() != Smb4KSettings::remoteSMBPort() )
  {
    arguments << QString( "-p %1" ).arg( options->smbPort() );
  }
  else
  {
    arguments << QString( "-p %1" ).arg( Smb4KSettings::remoteSMBPort() );
  }

  // Kerberos
  if ( options )
  {
    switch ( options->useKerberos() )
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        arguments << " -k";
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

  // Resolve order
  if ( !Smb4KSettings::nameResolveOrder().isEmpty() &&
        QString::compare( Smb4KSettings::nameResolveOrder(), samba_options["name resolve order"] ) != 0 )
  {
    arguments << QString( "-R %1" ).arg( KShell::quoteArg( Smb4KSettings::nameResolveOrder() ) );
  }
  else
  {
    // Do nothing
  }

  // NetBIOS name
  if ( !Smb4KSettings::netBIOSName().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSName(), samba_options["netbios name"] ) != 0 )
  {
    arguments << QString( "-n %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
  }
  else
  {
    // Do nothing
  }

  // NetBIOS scope
  if ( !Smb4KSettings::netBIOSScope().isEmpty() &&
       QString::compare( Smb4KSettings::netBIOSScope(), samba_options["netbios scope"] ) != 0 )
  {
    arguments << QString( "-i %1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSScope() ) );
  }
  else
  {
    // Do nothing
  }

  // Socket options
  if ( !Smb4KSettings::socketOptions().isEmpty() &&
       QString::compare( Smb4KSettings::socketOptions(), samba_options["socket options"] ) != 0 )
  {
    arguments << QString( "-O %1" ).arg( KShell::quoteArg( Smb4KSettings::socketOptions() ) );
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

  if ( !m_share->login().isEmpty() )
  {
    arguments << QString( "-U %1" ).arg( m_share->login() );
  }
  else
  {
    arguments << "-U %";
  }

  m_proc = new Smb4KProcess( this );
  m_proc->setOutputChannelMode( KProcess::SeparateChannels );
  m_proc->setEnv( "PASSWD", m_share->password(), true );
  m_proc->setShellCommand( arguments.join( " " ) );

  connect( m_proc, SIGNAL(readyReadStandardOutput()), SLOT(slotReadStandardOutput()) );
  connect( m_proc, SIGNAL(readyReadStandardError()),  SLOT(slotReadStandardError()) );
  connect( m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(slotProcessFinished(int,QProcess::ExitStatus)) );

  m_proc->start();
}


void Smb4KPreviewJob::slotReadStandardOutput()
{
  QStringList list = QString::fromUtf8( m_proc->readAllStandardOutput(), -1 ).split( '\n', QString::SkipEmptyParts );
  QList<Item> items;

  foreach ( const QString &line, list )
  {
    if ( line.contains( "blocks of size" ) || line.contains( "Domain=[" ) )
    {
      continue;
    }
    else if ( line.contains( "NT_STATUS_ACCESS_DENIED", Qt::CaseSensitive ) ||
              line.contains( "NT_STATUS_LOGON_FAILURE", Qt::CaseSensitive ) )
    {
      // This might happen if a directory cannot be accessed due to missing
      // read permissions.
      emit authError( this );
      break;
    }
    else
    {
      QString entry = line;
      
      QString left = entry.trimmed().section( "     ", 0, -2 ).trimmed();
      QString right = entry.remove( left );

      QString name = left.section( "  ", 0, -2 ).trimmed().isEmpty() ?
                     left :
                     left.section( "  ", 0, -2 ).trimmed();

      QString dir_string = left.right( 3 ).trimmed();
      bool is_dir = (!dir_string.isEmpty() && dir_string.contains( "D" ));

      QString tmp_size = right.trimmed().section( "  ", 0, 0 ).trimmed();
      QString size;

      if ( tmp_size[0].isLetter() )
      {
        size = right.trimmed().section( "  ", 1, 1 ).trimmed();
      }
      else
      {
        size = tmp_size;
      }

      QString date = QDateTime::fromString( right.section( QString( " %1 " ).arg( size ), 1, 1 ).trimmed() ).toString();

      if ( !name.isEmpty() )
      {
        Item item;

        if ( is_dir )
        {
          if ( name.startsWith( '.' ) &&
              (QString::compare( name, "." ) != 0 && QString::compare( name, ".." ) != 0) )
          {
            item.first = HiddenDirectoryItem;
          }
          else
          {
            item.first = DirectoryItem;
          }
        }
        else
        {
          if ( name.startsWith( '.' ) )
          {
            item.first = HiddenFileItem;
          }
          else
          {
            item.first = FileItem;
          }
        }

        item.second["name"] = name;
        item.second["size"] = size;
        item.second["date"] = date;

        items << item;
      }
      else
      {
        continue;
      }
    }
  }

  emit preview( m_url, items );
}


void Smb4KPreviewJob::slotReadStandardError()
{
  QString stderr = QString::fromUtf8( m_proc->readAllStandardError(), -1 ).trimmed();

  // Remove DEBUG messages and the additional information
  // that smbclient unfortunately reports to stderr.
  QStringList err_msg = stderr.split( '\n', QString::SkipEmptyParts );

  QMutableStringListIterator it( err_msg );

  while ( it.hasNext() )
  {
    QString line = it.next();

    if ( line.contains( "DEBUG" ) )
    {
      it.remove();
    }
    else if ( line.trimmed().startsWith( QLatin1String( "Domain" ) ) || line.trimmed().startsWith( QLatin1String( "OS" ) ) )
    {
      it.remove();
    }
    else if ( line.trimmed().startsWith( "Ignoring unknown parameter" ) )
    {
      it.remove();
    }
    else
    {
      // Do nothing
    }
  }

  // Avoid reporting an error if the process was killed by calling the abort() function
  // or if only debug and other information was reported.
  if ( !m_proc->isAborted() && !err_msg.isEmpty() )
  {
    m_proc->abort();

    if ( stderr.contains( "NT_STATUS_LOGON_FAILURE" ) ||
         stderr.contains( "NT_STATUS_ACCESS_DENIED" ) )
    {
      // Authentication error
      emit authError( this );
    }
    else
    {
      if ( !err_msg.isEmpty() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->retrievingPreviewFailed( m_share, err_msg.join( "\n" ) );
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Go ahead
  }  
}


void Smb4KPreviewJob::slotProcessFinished( int /*exitCode*/, QProcess::ExitStatus status )
{
  switch ( status )
  {
    case QProcess::CrashExit:
    {
      if ( !m_proc->isAborted() )
      {
        Smb4KNotification *notification = new Smb4KNotification();
        notification->processError( m_proc->error() );;
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
  emit finished( m_share, m_url );
}



Smb4KPreviewDialog::Smb4KPreviewDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ), m_share( share ), m_iterator( QStringList() )
{
  if ( !share->isHomesShare() )
  {
    m_url = share->url();
  }
  else
  {
    m_url = share->homeURL();
  }
  
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Preview" ) );
  setButtons( Close );
  setDefaultButton( Close );

  // Set the IP address if necessary.
  if ( share->hostIP().isEmpty() )
  {
    Smb4KHost *host = findHost( share->hostName(), share->workgroupName() );
    share->setHostIP( host->ip() );
  }
  else
  {
    // Do nothing
  }

  setupView();

  connect( this,                   SIGNAL(closeClicked()),
           this,                   SLOT(slotCloseClicked()) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "PreviewDialog" );
  restoreDialogSize( group );

  QTimer::singleShot( 0, this, SLOT(slotRequestPreview()) );
}


Smb4KPreviewDialog::~Smb4KPreviewDialog()
{
}


void Smb4KPreviewDialog::setupView()
{
  // Main widget
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  m_view = new KListWidget( main_widget );
  m_view->setResizeMode( KListWidget::Adjust );
  m_view->setWrapping( true );
  m_view->setSortingEnabled( true );
  m_view->setWhatsThis( i18n( "The preview is displayed here." ) );
  int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
  m_view->setIconSize( QSize( icon_size, icon_size ) );

  KToolBar *toolbar = new KToolBar( main_widget, true, false );

  m_reload  = new KAction( KIcon( "view-refresh" ), i18n( "Reload" ), toolbar );
  m_reload->setEnabled( false );
  
  m_abort   = new KAction( KIcon( "process-stop" ), i18n( "Abort" ), toolbar );
  m_abort->setEnabled( false );
  
  m_back    = new KAction( KIcon( "go-previous" ), i18n( "Back" ), toolbar );
  m_back->setEnabled( false );
  
  m_forward = new KAction( KIcon( "go-next" ), i18n( "Forward" ), toolbar );
  m_forward->setEnabled( false );
  
  m_up      = new KAction( KIcon( "go-up" ), i18n( "Up" ), toolbar );
  m_up->setEnabled( false );
  
  m_combo   = new KHistoryComboBox( true, toolbar );
  m_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  m_combo->setEditable( false );
  m_combo->setWhatsThis( i18n( "The current UNC address is shown here. You can also choose one of "
    "the previously visited locations from the drop-down menu that will then be displayed in the "
    "view above." ) );

  toolbar->addAction( m_reload );
  toolbar->addAction( m_abort );
  toolbar->addAction( m_back );
  toolbar->addAction( m_forward );
  toolbar->addAction( m_up );
  toolbar->insertSeparator( toolbar->addWidget( m_combo ) );

  layout->addWidget( m_view, 0, 0, 0 );
  layout->addWidget( toolbar, 1, 0, 0 );

  connect( toolbar, SIGNAL(actionTriggered(QAction*)),
           this,      SLOT(slotActionTriggered(QAction*)) );

  connect( m_combo,   SIGNAL(activated(QString)),
           this,      SLOT(slotItemActivated(QString)) );

  connect( m_view,    SIGNAL(executed(QListWidgetItem*)),
           this,      SLOT(slotItemExecuted(QListWidgetItem*)) );

  connect( KGlobalSettings::self(), SIGNAL(iconChanged(int)),
           this,                    SLOT(slotIconSizeChanged(int)) );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPreviewDialog::slotActionTriggered( QAction *action )
{
  KAction *kaction = static_cast<KAction *>( action );

  if ( kaction )
  {
    if ( kaction == m_reload )
    {
      // Clear the history
      m_history.clear();
      
     // Request the preview
      slotRequestPreview();
    }
    else if ( kaction == m_abort )
    {
      // Emit signal to kill the preview job.
      emit abortPreview( m_share );
    }
    else if ( kaction == m_back )
    {
      // Get the current history if necessary,
      // shift one item back and request a preview.
      if ( m_history.isEmpty() )
      {
        m_history = m_combo->historyItems();
        m_iterator = QStringListIterator( m_history );
      }
      else
      {
        // Do nothing
      }

      if ( m_iterator.hasNext() )
      {
        // Jump behind the current item.
        QString location = m_iterator.next();

        if ( QString::compare( location, m_combo->currentText(), Qt::CaseInsensitive ) == 0 )
        {
          if ( m_iterator.hasNext() )
          {
            location = m_iterator.next();
            QString path = location.remove( m_share->unc(), Qt::CaseInsensitive );

            if ( !path.isEmpty() )
            {
              m_url.setPath( QString( "%1%2" ).arg( m_share->shareName() ).arg( path ) );
            }
            else
            {
              m_url.setPath( m_share->shareName() );
            }

            // Request the preview.
            slotRequestPreview();
          }
          else
          {
            m_back->setEnabled( false );
          }
        }
        else
        {
          QString path = location.remove( m_share->unc(), Qt::CaseInsensitive );

          if ( !path.isEmpty() )
          {
            m_url.setPath( QString( "%1%2" ).arg( m_share->shareName() ).arg( path ) );
          }
          else
          {
            m_url.setPath( m_share->shareName() );
          }

          // Request the preview.
          slotRequestPreview();
        }
      }
      else
      {
        // Do nothing
      }
    }
    else if ( kaction == m_forward )
    {
      // Shift one item forward an request a preview.
      if ( !m_history.isEmpty() && m_iterator.hasPrevious() )
      {
        // Jump in front of the current item
        QString location = m_iterator.previous();

        if ( QString::compare( location, m_combo->currentText(), Qt::CaseInsensitive ) == 0 )
        {
          // Now get the next location.
          if ( m_iterator.hasPrevious() )
          {
            location = m_iterator.previous();
            QString path = location.remove( m_share->unc(), Qt::CaseInsensitive );

            if ( !path.isEmpty() )
            {
              m_url.setPath( QString( "%1%2" ).arg( m_share->shareName() ).arg( path ) );
            }
            else
            {
              m_url.setPath( m_share->shareName() );
            }

            // Request the preview
            slotRequestPreview();
          }
          else
          {
            m_forward->setEnabled( false );
          }
        }
        else
        {
          QString path = location.remove( m_share->unc(), Qt::CaseInsensitive );

          if ( !path.isEmpty() )
          {
            m_url.setPath( QString( "%1%2" ).arg( m_share->shareName() ).arg( path ) );
          }
          else
          {
            m_url.setPath( m_share->shareName() );
          }

          // Request the preview
          slotRequestPreview();
        }
      }
      else
      {
        // Do nothing
      }
    }
    else if ( kaction == m_up )
    {
      QString test = QString( "//%1/%2" ).arg( m_url.host() ).arg( m_url.path(KUrl::RemoveTrailingSlash) );
      
      if ( QString::compare( m_share->unc(), test, Qt::CaseInsensitive ) != 0 )
      {
        // Clear the history
        m_history.clear();

        // Adjust the path
        // FIXME: Use KUrl::upUrl() here when we have found to adjust 
        // the path
        QString path = m_url.path();
        m_url.setPath( path.section( '/', 0, -2 ) );

        // Request the preview
        slotRequestPreview();
      }
      else
      {
        m_up->setEnabled( false );
      }
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


void Smb4KPreviewDialog::slotRequestPreview()
{
  // Set the current item
  KUrl u = m_url;
  u.setProtocol( QString() );
  u.setUserInfo( QString() );
  u.setPort( -1 );
  
  QString current = u.url().replace( u.host(), u.host().toUpper() );
    
  // Set the history
  QStringList history;

  if ( m_combo->historyItems().isEmpty() ||
       QString::compare( m_combo->historyItems().first(), current, Qt::CaseInsensitive ) != 0 )
  {
    history << current;
  }
  else
  {
    // Do nothing
  }
  
  history << m_combo->historyItems();

  m_combo->setHistoryItems( history, true );
  m_combo->setCurrentItem( current, false );

  // Clear the view
  m_view->clear();
  
  // Request the preview for the current URL
  emit requestPreview( m_share, m_url, parentWidget() );
}


void Smb4KPreviewDialog::slotDisplayPreview( const KUrl &url, const QList<Item> &contents )
{
  if ( m_url != url )
  {
    return;
  }
  else
  {
    // Do nothing
  }

  // Display the preview
  for ( int i = 0; i < contents.size(); ++i )
  {
    switch ( contents.at( i ).first )
    {
      case HiddenDirectoryItem:
      {
        // Honor the user's setting about hidden items. And do not show the '.' and '..' directories.
        if ( Smb4KSettings::previewHiddenItems() &&
             QString::compare( contents.at( i ).second.value( "name" ), "." ) != 0 &&
             QString::compare( contents.at( i ).second.value( "name" ), ".." ) != 0 )
        {
          QListWidgetItem *listItem = new QListWidgetItem( KIcon( "folder" ), contents.at( i ).second.value( "name" ), m_view, Directory );
          listItem->setData( Qt::UserRole, contents.at( i ).second.value( "name" ) );
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case DirectoryItem:
      {
        // Do not show the '.' and '..' directories.
        if ( QString::compare( contents.at( i ).second.value( "name" ), "." ) != 0 &&
             QString::compare( contents.at( i ).second.value( "name" ), ".." ) != 0 )
        {
          QListWidgetItem *listItem = new QListWidgetItem( KIcon( "folder" ), contents.at( i ).second.value( "name" ), m_view, Directory );
          listItem->setData( Qt::UserRole, contents.at( i ).second.value( "name" ) );
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case HiddenFileItem:
      {
        if ( Smb4KSettings::previewHiddenItems() )
        {
          KUrl url( contents.at( i ).second.value( "name" ) );
          QListWidgetItem *listItem = new QListWidgetItem( KIcon( KMimeType::iconNameForUrl( url, 0 ) ), contents.at( i ).second.value( "name" ), m_view, File );
          listItem->setData( Qt::UserRole, contents.at( i ).second.value( "name" ) );
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case FileItem:
      {
        KUrl url( contents.at( i ).second.value( "name" ) );
        QListWidgetItem *listItem = new QListWidgetItem( KIcon( KMimeType::iconNameForUrl( url, 0 ) ), contents.at( i ).second.value( "name" ), m_view, File );
        listItem->setData( Qt::UserRole, contents.at( i ).second.value( "name" ) );
        break;
      }
      default:
      {
        break;
      }
    }
  }

  // Enable/disable the back action.
  bool enable_back = (m_combo->historyItems().size() > 1 &&
                     (m_history.isEmpty() || m_iterator.hasNext()));
  m_back->setEnabled( enable_back );

  // Enable/disable the forward action.
  bool enable_forward = (!m_history.isEmpty() && m_iterator.hasPrevious());
  m_forward->setEnabled( enable_forward );

  // Enable/disable the up action.
  QString test = QString( "//%1/%2" ).arg( m_url.host() ).arg( m_url.path(KUrl::RemoveTrailingSlash) );
  bool enable_up = (QString::compare( m_share->unc(), test, Qt::CaseInsensitive ) != 0);
  m_up->setEnabled( enable_up );
}


void Smb4KPreviewDialog::slotAboutToStart( Smb4KShare *share, const KUrl &url )
{
  if ( share == m_share && url == m_url )
  {
    m_reload->setEnabled( false );
    m_abort->setEnabled( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotFinished( Smb4KShare *share, const KUrl &url )
{
  if ( share == m_share && url == m_url )
  {
    m_reload->setEnabled( true );
    m_abort->setEnabled( false );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotItemExecuted( QListWidgetItem *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Directory:
      {
        // Clear the history
        m_history.clear();
        
        if ( !Smb4KPreviewer::self()->isRunning( m_share ) )
        {
          QString old_path = m_url.path();
          m_url.setPath( QString( "%1/%2" ).arg( old_path ).arg( item->data( Qt::UserRole ).toString() ) );
          slotRequestPreview();
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
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotItemActivated( const QString &item )
{
  if ( !Smb4KPreviewer::self()->isRunning( m_share ) )
  {
    // Clear the history
    m_history.clear();

    KUrl u;
    u.setUrl( item, KUrl::TolerantMode );
    u.setProtocol( "smb" );
    
    m_url.setPath( u.path() );
    slotRequestPreview();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotCloseClicked()
{
  KConfigGroup group( Smb4KSettings::self()->config(), "PreviewDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
  emit aboutToClose( this );
}


void Smb4KPreviewDialog::slotIconSizeChanged( int group )
{
  switch ( group )
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
      m_view->setIconSize( QSize( icon_size, icon_size ) );
      break;
    }
    default:
    {
      break;
    }
  }
}


#include "smb4kpreviewer_p.moc"

