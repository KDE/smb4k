/***************************************************************************
    smb4kwalletmanager  -  This is the wallet manager of Smb4K.
                             -------------------
    begin                : Sa Dez 27 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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
#include <QDesktopWidget>
#ifdef __FreeBSD__
#include <QFile>
#include <QDir>
#include <QTextCodec>
#include <QTextStream>
#endif

// KDE includes
#include <kglobal.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kpassworddialog.h>
#include <klocale.h>
#ifdef __FreeBSD__
#include <kprocess.h>
#include <kstandarddirs.h>
#endif

// application specific includes
#include <smb4kwalletmanager.h>
#include <smb4ksettings.h>
#include <smb4kauthinfo.h>
#include <smb4khomesshareshandler.h>
#ifdef __FreeBSD__
#include <smb4kcoremessage.h>
#include <smb4ksambaoptionshandler.h>
#endif
#include <smb4kglobal.h>

using namespace Smb4KGlobal;


class Smb4KWalletManagerPrivate
{
  public:
    Smb4KWalletManager instance;
};

K_GLOBAL_STATIC( Smb4KWalletManagerPrivate, priv );



Smb4KWalletManager::Smb4KWalletManager() : QObject()
{
  m_wallet = NULL;
  m_state = Unknown;
}


Smb4KWalletManager::~Smb4KWalletManager()
{
  // FIXME: Do we have to delete the wallet here?
}


Smb4KWalletManager *Smb4KWalletManager::self()
{
  return &priv->instance;
}


void Smb4KWalletManager::init( QWidget *parent, bool async )
{
  if ( KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet() )
  {
    // The wallet system is enabled and should be used,
    // so try to get the wallet.
    if ( !m_wallet )
    {
      int window_id = (parent ? parent->winId() : (kapp->activeWindow() ?
                      kapp->activeWindow()->winId() : kapp->desktop()->winId()));

      if ( async )
      {
        m_wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
                                                window_id,
                                                KWallet::Wallet::Asynchronous );

        connect( m_wallet, SIGNAL( walletOpened( bool ) ),
                 this,     SIGNAL( walletOpened( bool ) ) );

        connect( m_wallet, SIGNAL( walletOpened( bool ) ),
                 this,     SLOT( slotWalletOpened( bool ) ) );
      }
      else
      {
        m_wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
                                                window_id,
                                                KWallet::Wallet::Synchronous );

        if ( m_wallet )
        {
          setupFolder();
          m_state = UseWallet;
        }
        else
        {
          kDebug() << "Opening the wallet failed ..." << endl;
          m_state = Unknown;
        }

        emit initialized();
      }
    }
    else
    {
      // Do nothing. We already have a wallet.
    }
  }
  else
  {
    if ( m_wallet )
    {
      delete m_wallet;
      m_wallet = NULL;
    }
    else
    {
      // Do nothing
    }

    m_state = Smb4KSettings::rememberLogins() ?
              RememberAuthInfo :
              ForgetAuthInfo;

    emit initialized();
  }
}


void Smb4KWalletManager::setupFolder()
{
  if ( m_wallet && m_wallet->isOpen() )
  {
    if ( !m_wallet->hasFolder( "Smb4K" ) )
    {
      m_wallet->createFolder( "Smb4K" );
      m_wallet->setFolder( "Smb4K" );
    }
    else
    {
      m_wallet->setFolder( "Smb4K" );
    }
  }
  else
  {
    kDebug() << "No wallet or wallet not open ..." << endl;
  }
}


void Smb4KWalletManager::readAuthInfo( Smb4KAuthInfo *authInfo )
{
  Q_ASSERT( authInfo );

  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will do nothing.
  init( 0 );

  if ( m_wallet )
  {
    bool set_auth_info = false;

    // Read the authentication information.
    switch ( authInfo->type() )
    {
      case Smb4KAuthInfo::Host:
      {
        QMap<QString,QString> map;

        if ( m_wallet->hasEntry( authInfo->unc().toUpper() ) )
        {
          m_wallet->readMap( authInfo->unc().toUpper(), map );
        }
        else if ( m_wallet->hasEntry( authInfo->hostName().toUpper() ) )
        {
          m_wallet->readMap( authInfo->hostName().toUpper(), map );
          m_wallet->removeEntry( authInfo->hostName().toUpper() );
        }
        else
        {
          // Do nothing
        }

        // Only set the authentication information if the map is not empty and if
        // either the login was not set in authInfo or it is equal to map["Login"].
        if ( !map.isEmpty() &&
             (authInfo->login().isEmpty() || QString::compare( QString::fromUtf8( authInfo->login() ), map["Login"] ) == 0) )
        {
          // Do not set the authentication, if the workgroup does not match.
          if ( map["Workgroup"].isEmpty() || authInfo->workgroupName().isEmpty() ||
              QString::compare( map["Workgroup"], authInfo->workgroupName().toUpper() ) == 0 )
          {
            authInfo->setLogin( map["Login"] );
            authInfo->setPassword( map["Password"] );

            if ( !map["Workgroup"].isEmpty() && authInfo->workgroupName().isEmpty() )
            {
              authInfo->setWorkgroupName( map["Workgroup"] );
            }
            else
            {
              // Do nothing
            }

            set_auth_info = true;
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

        break;
      }
      case Smb4KAuthInfo::Share:
      {
        // Read the authentication information. Prefer the exact match,
        // but also look for an entry that was provided for the host.
        QMap<QString,QString> map;
        m_wallet->readMap( authInfo->unc().toUpper(), map );

        if ( map.isEmpty() )
        {
          m_wallet->readMap( authInfo->hostUNC().toUpper(), map );
        }
        else
        {
          // Do nothing
        }

        // Only set the authentication information the map is not empty and if
        // either the login was not set in authInfo or it is equal to map["Login"].
        if ( !map.isEmpty() &&
             (authInfo->login().isEmpty() || QString::compare( QString::fromUtf8( authInfo->login() ), map["Login"] ) == 0) )
        {
          if ( map["Workgroup"].isEmpty() || authInfo->workgroupName().isEmpty() ||
              QString::compare( map["Workgroup"], authInfo->workgroupName().toUpper() ) == 0 )
          {
            authInfo->setLogin( map["Login"] );
            authInfo->setPassword( map["Password"] );

            if ( !map["Workgroup"].isEmpty() && authInfo->workgroupName().isEmpty() )
            {
              authInfo->setWorkgroupName( map["Workgroup"] );
            }
            else
            {
              // Do nothing
            }

            set_auth_info = true;
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

        break;
      }
      case Smb4KAuthInfo::Default:
      {
        QMap<QString,QString> map;
        m_wallet->readMap( "DEFAULT_LOGIN", map );

        if ( !map.isEmpty() )
        {
          authInfo->setLogin( map["Login"] );
          authInfo->setPassword( map["Password"] );
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

    // If there was no matching authentication information, use the
    // default one, if it exists.
    if ( !set_auth_info && Smb4KSettings::useDefaultLogin() &&
         authInfo->type() != Smb4KAuthInfo::Default )
    {
      QMap<QString,QString> map;
      m_wallet->readMap( "DEFAULT_LOGIN", map );

      if ( !map.isEmpty() )
      {
        authInfo->setLogin( map["Login"] );
        authInfo->setPassword( map["Password"] );
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
  else
  {
    for ( int i = 0; i < m_list.size(); ++i )
    {
      if ( QString::compare( authInfo->unc().toUpper(), m_list.at( i )->unc().toUpper() ) == 0 )
      {
        // Exact match
        authInfo->setLogin( m_list.at( i )->login() );
        authInfo->setPassword( m_list.at( i )->password() );

        if ( authInfo->workgroupName().isEmpty() )
        {
          authInfo->setWorkgroupName( m_list.at( i )->workgroupName() );
        }
        else
        {
          // Do nothing
        }

        break;
      }
      else if ( QString::compare( authInfo->hostUNC().toUpper(), m_list.at( i )->hostUNC().toUpper() ) == 0 )
      {
        // The host is the same
        authInfo->setLogin( m_list.at( i )->login() );
        authInfo->setPassword( m_list.at( i )->password() );

        if ( authInfo->workgroupName().isEmpty() )
        {
          authInfo->setWorkgroupName( m_list.at( i )->workgroupName() );
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

    // Clear the list if necessary, i.e. if the wallet manager
    // should forget the previously used login(s).
    if ( !Smb4KSettings::rememberLogins() )
    {
      while ( !m_list.isEmpty() )
      {
        delete m_list.takeFirst();
      }
    }
    else
    {
      // Do nothing
    }
  }
#ifdef __FreeBSD__
  writeToConfigFile( authInfo );
#endif
}


void Smb4KWalletManager::writeAuthInfo( Smb4KAuthInfo *authInfo )
{
  Q_ASSERT( authInfo );

  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will just do nothing.
  init( 0 );

  if ( m_wallet )
  {
    // Write the authentication information to the wallet, if it
    // is not empty.
    QMap<QString,QString> map;

    if ( !authInfo->login().isEmpty() /* allow empty passwords */ )
    {
      map["Login"]    = authInfo->login();
      map["Password"] = authInfo->password();
    }
    else
    {
      return;
    }

    switch ( authInfo->type() )
    {
      case Smb4KAuthInfo::Default:
      {
        m_wallet->writeMap( "DEFAULT_LOGIN", map );
        break;
      }
      default:
      {
        if ( !authInfo->workgroupName().isEmpty() )
        {
          map["Workgroup"] = authInfo->workgroupName().toUpper();
        }
        else
        {
          // Do nothing
        }

        m_wallet->writeMap( authInfo->unc().toUpper(), map );
        break;
      }
    }

    m_wallet->sync();
  }
  else
  {
    // Clear the list if necessary, i.e. if the wallet manager
    // should forget the previously used login(s).
    if ( !Smb4KSettings::rememberLogins() )
    {
      while ( !m_list.isEmpty() )
      {
        delete m_list.takeFirst();
      }
    }
    else
    {
      // Do nothing
    }

    // We do not store a default login if no wallet is used.
    if ( authInfo->type() != Smb4KAuthInfo::Default )
    {
      m_list.append( new Smb4KAuthInfo( *authInfo ) );
    }
    else
    {
      // Do nothing
    }
  }

#ifdef __FreeBSD__
  writeToConfigFile( authInfo );
#endif
}


bool Smb4KWalletManager::showPasswordDialog( Smb4KAuthInfo *authInfo, QWidget *parent )
{
  Q_ASSERT( authInfo );

  bool success = false;

  // Read the authentication information.
  readAuthInfo( authInfo );

  // Get known logins in case we have a 'homes' share and the share
  // name has not yet been changed.
  QMap<QString, QString> logins;

  if ( authInfo->isHomesShare() && QString::compare( authInfo->shareName(), "homes" ) == 0 )
  {
    if ( authInfo->homesUsers().isEmpty() )
    {
      // Normally, authInfo should already carry the homes users,
      // but we will check once more.
      Smb4KHomesSharesHandler::self()->setHomesUsers( authInfo );
    }
    else
    {
      // Do nothing
    }

    for ( int i = 0; i < authInfo->homesUsers().size(); ++i )
    {
      Smb4KAuthInfo user_auth_info( *authInfo );
      user_auth_info.setLogin( authInfo->homesUsers().at( i ) );

      // Read the authentication data for the share. If it does not
      // exist yet, user() and password() will be empty.
      readAuthInfo( &user_auth_info );

      logins.insert( QString::fromUtf8( user_auth_info.login() ),
                     QString::fromUtf8( user_auth_info.password() ) );
    }
  }
  else
  {
    // Do nothing
  }

  // Set up the password dialog.
  KPasswordDialog dlg( parent, KPasswordDialog::ShowUsernameLine );

  if ( !logins.isEmpty() )
  {
    dlg.setKnownLogins( logins );
  }
  else
  {
    dlg.setUsername( authInfo->login() );
    dlg.setPassword( authInfo->password() );
  }

  QString prompt;

  switch ( authInfo->type() )
  {
    case Smb4KAuthInfo::Host:
    {
      prompt = i18n( "<qt>Please enter a username and a password for the host %1.</qt>" ).arg( authInfo->hostName() );
      break;
    }
    case Smb4KAuthInfo::Share:
    {
      prompt = i18n( "<qt>Please enter a username and a password for the share %1.</qt>" ).arg( authInfo->unc() );
      break;
    }
    default:
    {
      prompt = i18n( "<qt>Please enter a username and a password below.</qt>" );
      break;
    }
  }

  dlg.setPrompt( prompt );

  // Execute the password dialog, retrieve the new authentication
  // information and save it.
  if ( (success = dlg.exec()) )
  {
    authInfo->setLogin( dlg.username() );
    authInfo->setPassword( dlg.password() );

    writeAuthInfo( authInfo );
  }
  else
  {
    // Do nothing
  }

  return success;
}


bool Smb4KWalletManager::useWalletSystem()
{
  return (KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet());
}


#ifdef __FreeBSD__

void Smb4KWalletManager::writeToConfigFile( Smb4KAuthInfo *authInfo )
{
  Q_ASSERT( authInfo );

  // Only write the authentication information if it is not empty.
  if ( authInfo->login().isEmpty() /* allow empty passwords */ )
  {
    return;
  }
  else
  {
    // Do nothing
  }

  // Find smbutil program
  QString smbutil = KStandardDirs::findExe( "smbutil" );

  if ( smbutil.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smbutil" );
    return;
  }
  else
  {
    // Go ahead
  }

  QStringList contents;

  // Open the config file.
  QFile file( QDir::homePath()+QDir::separator()+".nsmbrc" );

  if ( file.exists() )
  {
    if ( file.open( QIODevice::ReadOnly|QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, we'll keep
      // it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      while ( !ts.atEnd() )
      {
        contents << ts.readLine().trimmed();
      }

      file.close();
    }
    else
    {
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, file.fileName(), file.errorString() );
      return;
    }
  }
  else
  {
    // Do nothing
  }

  // Check if we have to write something.
  bool write = false;

  // If the default section is missing, add it to the config file.
  if ( !contents.contains( "[default]", Qt::CaseInsensitive ) )
  {
    // Get the global Samba options.
    QMap<QString,QString> global_options = Smb4KSambaOptionsHandler::self()->globalSambaOptions();

    // Set up the default section.
    QStringList default_section;
    default_section << "[default]";

    // Local and remote character set
    QString client_charset, server_charset;

    switch ( Smb4KSettings::clientCharset() )
    {
      case Smb4KSettings::EnumClientCharset::default_charset:
      {
        client_charset = global_options["unix charset"].toLower();
        break;
      }
      default:
      {
        Q_ASSERT( !Smb4KSettings::self()->clientCharsetItem()->label().isEmpty() );
        client_charset = Smb4KSettings::self()->clientCharsetItem()->label();
        break;
      }
    }

    switch ( Smb4KSettings::serverCodepage() )
    {
      case Smb4KSettings::EnumServerCodepage::default_codepage:
      {
        server_charset = global_options["dos charset"].toLower(); // maybe empty
        break;
      }
      default:
      {
        Q_ASSERT( !Smb4KSettings::self()->serverCodepageItem()->label().isEmpty() );
        server_charset = Smb4KSettings::self()->serverCodepageItem()->label();
        break;
      }
    }

    if ( !client_charset.isEmpty() && !server_charset.isEmpty() )
    {
      default_section << "charsets="+client_charset+":"+server_charset;
    }
    else
    {
      // Do nothing
    }

    // WINS server
    QString wins_server = Smb4KSambaOptionsHandler::self()->winsServer();

    if ( !wins_server.isEmpty() )
    {
      default_section << "nbns="+wins_server;
    }
    else
    {
      // Do nothing
    }

    // Workgroup/domain
    QString domain;

    if ( !Smb4KSettings::domainName().isEmpty() )
    {
      domain = Smb4KSettings::domainName();
    }
    else
    {
      domain = global_options["workgroup"];
    }

    if ( !domain.isEmpty() )
    {
      default_section << "workgroup="+domain;
    }
    else
    {
      // Do nothing
    }

    // The rest of the possible options we leave up to the user.

    default_section << "";

    // Prepend the default section to the contents of ~/.nsmbrc.
    contents = default_section+contents;

    write = true;
  }
  else
  {
    // We won't touch the default section when it exists.
  }

  // Encrypt the password that was passed by authInfo.
  QString password;

  KProcess proc;
  proc.setShellCommand( smbutil+" crypt "+authInfo->password() );
  proc.setOutputChannelMode( KProcess::SeparateChannels );

  switch ( proc.execute() )
  {
    case -2:
    case -1:
    {
      Smb4KCoreMessage::processError( ERROR_PROCESS_ERROR, proc.error() );
      break;
    }
    default:
    {
      password = QString::fromUtf8( proc.readAllStandardOutput(), -1 ).trimmed();
      break;
    }
  }

  // Check the entry that corresponds to the data passed by authInfo.
  switch ( authInfo->type() )
  {
    case Smb4KAuthInfo::Host:
    {
      int index = -1;

      if ( !authInfo->login().isEmpty() )
      {
        if ( contents.contains( "["+authInfo->hostName().toUpper()+":"+authInfo->login().toUpper()+"]" ) )
        {
          index = contents.indexOf( "["+authInfo->hostName().toUpper()+":"+authInfo->login().toUpper()+"]" );
        }
        else if ( contents.contains( "["+authInfo->hostName().toLower()+":"+authInfo->login().toLower()+"]" ) )
        {
          index = contents.indexOf( "["+authInfo->hostName().toLower()+":"+authInfo->login().toLower()+"]" );
        }
        else
        {
          // Do nohing
        }

        if ( index != -1 )
        {
          // The entry is in the list.
          for ( int i = index; i < contents.size(); ++i )
          {
            if ( contents.at( i ).startsWith( "password", Qt::CaseInsensitive ) )
            {
              if ( QString::compare( contents.at( i ).section( "=", 1, 1 ).trimmed(), password ) != 0 &&
                   !authInfo->password().isEmpty() /* we do not want empty server passwords */ )
              {
                contents[i].replace( contents.at( i ).section( "=", 1, 1 ).trimmed(), password );
                write = true;
              }
              else
              {
                // Do nothing
              }

              break;
            }
            else if ( contents.at( i ).isEmpty() )
            {
              break;
            }
            else
            {
              continue;
            }
          }
        }
        else
        {
          // The entry is not in the list. Append it, if a password is
          // available.
          if ( !authInfo->password().isEmpty() )
          {
            if ( !contents.last().trimmed().isEmpty() )
            {
              contents << "";
            }
            else
            {
              // Do nothing
            }

            contents << "["+authInfo->hostName().toUpper()+":"+authInfo->login().toUpper()+"]";
            contents << "password="+password;
            contents << "workgroup="+authInfo->workgroupName();
            contents << "";

            write = true;
          }
          else
          {
            // Do nothing
          }
        }
      }
      else
      {
        if ( contents.contains( "["+authInfo->hostName().toUpper()+"]" ) )
        {
          index = contents.indexOf( "["+authInfo->hostName().toUpper()+"]" );
        }
        else if ( contents.contains( "["+authInfo->hostName().toLower()+"]" ) )
        {
          index = contents.indexOf( "["+authInfo->hostName().toLower()+"]" );
        }
        else
        {
          // Do nothing
        }

        if ( index != -1 )
        {
          // The entry is in the list.
          for ( int i = index; i < contents.size(); ++i )
          {
            if ( contents.at( i ).startsWith( "password", Qt::CaseInsensitive ) )
            {
              if ( QString::compare( contents.at( i ).section( "=", 1, 1 ).trimmed(), password ) != 0 &&
                   !authInfo->password().isEmpty() /* we do not want empty server passwords */ )
              {
                contents[i].replace( contents.at( i ).section( "=", 1, 1 ).trimmed(), password );
                write = true;
              }
              else
              {
                // Do nothing
              }

              break;
            }
            else if ( contents.at( i ).isEmpty() )
            {
              break;
            }
            else
            {
              continue;
            }
          }
        }
        else
        {
          // The entry is not in the list. Append it, if a password is
          // available.
          if ( !authInfo->password().isEmpty() )
          {
            if ( !contents.last().trimmed().isEmpty() )
            {
              contents << "";
            }
            else
            {
              // Do nothing
            }

            contents << "["+authInfo->hostName().toUpper()+"]";
            contents << "password="+password;
            contents << "workgroup="+authInfo->workgroupName();
            contents << "";
            write = true;
          }
          else
          {
            // Do nothing
          }
        }
      }

      break;
    }
    case Smb4KAuthInfo::Share:
    {
      int index = -1;

      if ( !authInfo->login().isEmpty() )
      {
        // The server is not in the file.
        if ( contents.contains( "["+authInfo->hostName().toUpper()+":"+authInfo->login().toUpper()+":"+authInfo->shareName().toUpper()+"]" ) )
        {
          index = contents.indexOf( "["+authInfo->hostName().toUpper()+":"+authInfo->login().toUpper()+":"+authInfo->shareName().toUpper()+"]" );
        }
        else if ( contents.contains( "["+authInfo->hostName().toLower()+":"+authInfo->login().toLower()+":"+authInfo->shareName().toLower()+"]" ) )
        {
          index = contents.indexOf( "["+authInfo->hostName().toLower()+":"+authInfo->login().toLower()+":"+authInfo->shareName().toLower()+"]" );
        }
        else
        {
          // Do nohing
        }

        if ( index != -1 )
        {
          // The entry is in the list.
          for ( int i = index; i < contents.size(); ++i )
          {
            if ( contents.at( i ).startsWith( "password", Qt::CaseInsensitive ) )
            {
              if ( QString::compare( contents.at( i ).section( "=", 1, 1 ).trimmed(), password ) != 0 &&
                   !authInfo->password().isEmpty() )
              {
                contents[i].replace( contents.at( i ).section( "=", 1, 1 ).trimmed(), password );
                write = true;
              }
              else
              {
                // Do nothing
              }

              break;
            }
            else if ( contents.at( i ).isEmpty() )
            {
              break;
            }
            else
            {
              continue;
            }
          }
        }
        else
        {
          // The entry is not in the list. Append it, if a password is
          // available.
          if ( !authInfo->password().isEmpty() )
          {
            if ( !contents.last().trimmed().isEmpty() )
            {
              contents << "";
            }
            else
            {
              // Do nothing
            }

            contents << "["+authInfo->hostName().toUpper()+":"+authInfo->login().toUpper()+":"+authInfo->shareName().toUpper()+"]";
            contents << "password="+password;
            contents << "workgroup="+authInfo->workgroupName();
            contents << "";

            write = true;
          }
          else
          {
            // Do nothing
          }
        }
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

  if ( write )
  {
    if ( file.open( QIODevice::WriteOnly|QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, we'll keep
      // it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      ts << contents.join( "\n" );

      file.close();
    }
    else
    {
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, file.fileName(), file.errorString() );
      return;
    }
  }
  else
  {
    // Do nothing
  }
}

#endif


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KWalletManager::slotWalletOpened( bool success )
{
  if ( success )
  {
    setupFolder();
    m_state = UseWallet;
  }
  else
  {
    kDebug() << "Opening the wallet failed ..." << endl;
    m_state = Unknown;
  }

  emit initialized();
}


#include "smb4kwalletmanager.moc"
