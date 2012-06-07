/***************************************************************************
    smb4kwalletmanager  -  This is the wallet manager of Smb4K.
                             -------------------
    begin                : Sa Dez 27 2008
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
#include "smb4kwalletmanager.h"
#include "smb4ksettings.h"
#include "smb4kauthinfo.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"
#include "smb4knotification.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// Qt includes
#include <QtCore/QPointer>
#ifdef Q_OS_FREEBSD
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#endif
#include <QtGui/QDesktopWidget>

// KDE includes
#include <kglobal.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kpassworddialog.h>
#include <klocale.h>
#ifdef Q_OS_FREEBSD
#include <kprocess.h>
#include <kstandarddirs.h>
#endif

using namespace Smb4KGlobal;


class Smb4KWalletManagerPrivate
{
  public:
    KWallet::Wallet *wallet;
    Smb4KWalletManager::State state;
    QList<Smb4KAuthInfo *> list;
};


class Smb4KWalletManagerStatic
{
  public:
    Smb4KWalletManager instance;
};

K_GLOBAL_STATIC( Smb4KWalletManagerStatic, p );



Smb4KWalletManager::Smb4KWalletManager( QObject *parent )
: QObject( parent ), d( new Smb4KWalletManagerPrivate )
{
  d->wallet = NULL;
  d->state  = Unknown;
}


Smb4KWalletManager::~Smb4KWalletManager()
{
}


Smb4KWalletManager *Smb4KWalletManager::self()
{
  return &p->instance;
}


void Smb4KWalletManager::init()
{
  if ( KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet() )
  {
    // The wallet system is enabled and should be used,
    // so try to get the wallet.
    if ( !d->wallet )
    {
      // Get the main window as parent of the wallet.
      QWidget *parent = 0;
      QWidgetList top_level = kapp->topLevelWidgets();

      for ( int i = 0; i < top_level.size(); ++i )
      {
        if ( QString::compare( top_level.at( i )->metaObject()->className(), "Smb4KMainWindow" ) == 0 )
        {
          parent = top_level[i];
          break;
        }
        else
        {
          continue;
        }
      }

      d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
                                              parent ? parent->winId() : 0,
                                              KWallet::Wallet::Synchronous );

      if ( d->wallet )
      {
        setupFolder();
        d->state = UseWallet;
      }
      else
      {
        Smb4KNotification *notification = new Smb4KNotification( this );
        notification->openingWalletFailed( KWallet::Wallet::NetworkWallet() );

        d->state = Unknown;
      }

      emit initialized();
    }
    else
    {
      // Do nothing. We already have a wallet.
    }
  }
  else
  {
    if ( d->wallet )
    {
      delete d->wallet;
      d->wallet = NULL;
    }
    else
    {
      // Do nothing
    }

    d->state = Smb4KSettings::rememberLogins() ?
              RememberAuthInfo :
              ForgetAuthInfo;

    emit initialized();
  }
}


void Smb4KWalletManager::setupFolder()
{
  if ( d->wallet && d->wallet->isOpen() )
  {
    if ( !d->wallet->hasFolder( "Smb4K" ) )
    {
      d->wallet->createFolder( "Smb4K" );
      d->wallet->setFolder( "Smb4K" );
    }
    else
    {
      d->wallet->setFolder( "Smb4K" );
    }
  }
  else
  {
    Smb4KNotification *notification = new Smb4KNotification( this );
    notification->loginsNotAccessible();
  }
}


void Smb4KWalletManager::readAuthInfo( Smb4KBasicNetworkItem *networkItem )
{
  Q_ASSERT( networkItem );

  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will do nothing.
  init();

  if ( useWalletSystem() && d->wallet )
  {
    bool auth_info_set = false;

    switch ( networkItem->type() )
    {
      case Smb4KBasicNetworkItem::Host:
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );

        if ( host )
        {
          QMap<QString,QString> map;

          if ( d->wallet->hasEntry( host->unc().toUpper() ) )
          {
            d->wallet->readMap( host->unc().toUpper(), map );
          }
          else
          {
            // Do nothing
          }

          // Set the authentication information.
          if ( !map.isEmpty() && (host->workgroupName().isEmpty() || map["Workgroup"].isEmpty() ||
               QString::compare( host->workgroupName(), map.value( "Workgroup" ), Qt::CaseInsensitive ) == 0) )
          {
            if ( !host->login().isEmpty() )
            {
              if ( QString::compare( host->login(), map.value( "Login" ) ) == 0 )
              {
                host->setLogin( map.value( "Login" ) );
                host->setPassword( map.value( "Password" ) );
                auth_info_set = true;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              host->setLogin( map.value( "Login" ) );
              host->setPassword( map.value( "Password" ) );
              auth_info_set = true;
            }

            // In difference to the readAuthInfo() function that takes an Smb4KAuthInfo
            // object, do not set the workgroup or IP address here.
          }
          else
          {
            // Do nothing
          }

          // If no authentication information was set until now, check if
          // we have to set the default login then.
          if ( !auth_info_set && Smb4KSettings::useDefaultLogin() )
          {
            QMap<QString,QString> map;
            d->wallet->readMap( "DEFAULT_LOGIN", map );

            if ( !map.isEmpty() )
            {
              host->setLogin( map["Login"] );
              host->setPassword( map["Password"] );
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
#ifdef Q_OS_FREEBSD
          Smb4KAuthInfo authInfo( host );
          writeToConfigFile( &authInfo );
#endif
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Smb4KBasicNetworkItem::Share:
      {
        Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

        if ( share )
        {
          QMap<QString,QString> map;

          if ( !share->isHomesShare() )
          {
            if ( d->wallet->hasEntry( share->unc().toUpper() ) )
            {
              d->wallet->readMap( share->unc().toUpper(), map );
            }
            else
            {
              d->wallet->readMap( share->hostUNC().toUpper(), map );
            }
          }
          else
          {
            // Specify a user name if necessary. The overwrite argument
            // is set to FALSE here, so that no dialog is shown if a user
            // name has already been provided.
            Smb4KHomesSharesHandler::self()->specifyUser( share, false );

            if ( d->wallet->hasEntry( share->homeUNC().toUpper() ) )
            {
              d->wallet->readMap( share->homeUNC().toUpper(), map );
            }
            else
            {
              d->wallet->readMap( share->hostUNC().toUpper(), map );
            }
          }

          // Set the authentication information.
          if ( !map.isEmpty() && (share->workgroupName().isEmpty() || map["Workgroup"].isEmpty() ||
               QString::compare( share->workgroupName(), map.value( "Workgroup" ), Qt::CaseInsensitive ) == 0) )
          {
            if ( !share->login().isEmpty() )
            {
              if ( QString::compare( share->login(), map.value( "Login" ) ) == 0 )
              {
                share->setLogin( map.value( "Login" ) );
                share->setPassword( map.value( "Password" ) );
                auth_info_set = true;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              share->setLogin( map.value( "Login" ) );
              share->setPassword( map.value( "Password" ) );
              auth_info_set = true;
            }

            // In difference to the readAuthInfo() function that takes an Smb4KAuthInfo
            // object, do not set the workgroup or IP address here.
          }
          else
          {
            // Do nothing
          }

          // If no authentication information was set until now, check if
          // we have to set the default login then.
          if ( !auth_info_set && Smb4KSettings::useDefaultLogin() )
          {
            QMap<QString,QString> map;
            d->wallet->readMap( "DEFAULT_LOGIN", map );

            if ( !map.isEmpty() )
            {
              share->setLogin( map["Login"] );
              share->setPassword( map["Password"] );
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
#ifdef Q_OS_FREEBSD
          Smb4KAuthInfo authInfo( share );
          writeToConfigFile( &authInfo );
#endif
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
    switch ( networkItem->type() )
    {
      case Smb4KBasicNetworkItem::Host:
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );

        if ( host )
        {
          for ( int i = 0; i < d->list.size(); ++i )
          {
            if ( QString::compare( host->unc(), d->list.at( i )->unc(), Qt::CaseInsensitive ) == 0 &&
                 (host->workgroupName().isEmpty() || d->list.at( i )->workgroupName().isEmpty() ||
                 QString::compare( host->workgroupName(), d->list.at( i )->workgroupName(), Qt::CaseInsensitive ) == 0) )
            {
              if ( !host->login().isEmpty() )
              {
                if ( QString::compare( host->login(), d->list.at( i )->login() ) == 0 )
                {
                  host->setLogin( d->list.at( i )->login() );
                  host->setPassword( d->list.at( i )->password() );
                }
                else
                {
                  // Do nothing
                }
              }
              else
              {
                host->setLogin( d->list.at( i )->login() );
                host->setPassword( d->list.at( i )->password() );
              }
              break;
            }
            else
            {
              continue;
            }
          }
#ifdef Q_OS_FREEBSD
          Smb4KAuthInfo authInfo( host );
          writeToConfigFile( &authInfo );
#endif
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Smb4KBasicNetworkItem::Share:
      {
        Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

        if ( share )
        {
          QString unc;

          if ( !share->isHomesShare() )
          {
            unc = share->unc();
          }
          else
          {
            // Specify a user name if necessary. The overwrite argument
            // is set to FALSE here, so that no dialog is shown if a user
            // name has already been provided.
            Smb4KHomesSharesHandler::self()->specifyUser( share, false );

            unc = share->homeUNC();
          }

          for ( int i = 0; i < d->list.size(); ++i )
          {
            if ( QString::compare( unc, d->list.at( i )->unc(), Qt::CaseInsensitive ) == 0 &&
                 (share->workgroupName().isEmpty() || d->list.at( i )->workgroupName().isEmpty() ||
                 QString::compare( share->workgroupName(), d->list.at( i )->workgroupName(), Qt::CaseInsensitive ) == 0) )
            {
              // Exact match
              if ( !share->login().isEmpty() )
              {
                if ( QString::compare( share->login(), d->list.at( i )->login() ) == 0 )
                {
                  share->setLogin( d->list.at( i )->login() );
                  share->setPassword( d->list.at( i )->password() );
                }
                else
                {
                  // Do nothing
                }
              }
              else
              {
                share->setLogin( d->list.at( i )->login() );
                share->setPassword( d->list.at( i )->password() );
              }
              break;
            }
            else if ( QString::compare( share->hostUNC(), d->list.at( i )->hostUNC(), Qt::CaseInsensitive ) == 0 &&
                      (share->workgroupName().isEmpty() || d->list.at( i )->workgroupName().isEmpty() ||
                      QString::compare( share->workgroupName(), d->list.at( i )->workgroupName(), Qt::CaseInsensitive ) == 0) )
            {
              // The host is the same
              if ( !share->login().isEmpty() )
              {
                if ( QString::compare( share->login(), d->list.at( i )->login() ) == 0 )
                {
                  share->setLogin( d->list.at( i )->login() );
                  share->setPassword( d->list.at( i )->password() );
                }
                else
                {
                  // Do nothing
                }
              }
              else
              {
                share->setLogin( d->list.at( i )->login() );
                share->setPassword( d->list.at( i )->password() );
              }
              continue;
            }
            else
            {
              continue;
            }
          }
#ifdef Q_OS_FREEBSD
          Smb4KAuthInfo authInfo( share );
          writeToConfigFile( &authInfo );
#endif
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

    // Clear the list if necessary, i.e. if the wallet manager
    // should forget the previously used login(s).
    if ( !Smb4KSettings::rememberLogins() )
    {
      while ( !d->list.isEmpty() )
      {
        delete d->list.takeFirst();
      }
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KWalletManager::readDefaultAuthInfo( Smb4KAuthInfo *authInfo )
{
  Q_ASSERT( authInfo );

  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will just do nothing.
  init();

  // Since we do not store default authentication information
  // when no wallet is used, we only need to read from the wallet.
  if ( useWalletSystem() && d->wallet )
  {
    QMap<QString,QString> map;
    d->wallet->readMap( "DEFAULT_LOGIN", map );

    if ( !map.isEmpty() )
    {
      authInfo->setLogin( map["Login"] );
      authInfo->setPassword( map["Password"] );
    }
    else
    {
      // Do nothing
    }

    authInfo->useDefaultAuthInfo();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KWalletManager::writeAuthInfo( Smb4KBasicNetworkItem *networkItem )
{
  Q_ASSERT( networkItem );

  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will just do nothing.
  init();

  if ( useWalletSystem() && d->wallet )
  {
    switch ( networkItem->type() )
    {
      case Smb4KBasicNetworkItem::Host:
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );

        if ( host )
        {
          // Write the authentication information to the wallet, if it
          // is not empty.
          if ( !host->login().isEmpty() /* allow empty passwords */ && !host->hostName().isEmpty() )
          {
            QMap<QString,QString> map;
            map["Login"]    = host->login();
            map["Password"] = host->password();

            if ( !host->workgroupName().isEmpty() )
            {
              map["Workgroup"] = host->workgroupName().toUpper();
            }
            else
            {
              // Do nothing
            }

            if ( !host->ip().isEmpty() )
            {
              map["IP Address"] = host->ip();
            }
            else
            {
              // Do nothing
            }

            d->wallet->writeMap( host->unc().toUpper(), map );
            d->wallet->sync();
          }
          else
          {
            // Do nothing
          }

#ifdef Q_OS_FREEBSD
          Smb4KAuthInfo authInfo( host );
          writeToConfigFile( &authInfo );
#endif
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Smb4KBasicNetworkItem::Share:
      {
        Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

        if ( share )
        {
          // Write the authentication information to the wallet, if it
          // is not empty.
          if ( !share->login().isEmpty() /* allow empty passwords */ && !share->hostName().isEmpty() )
          {
            QMap<QString,QString> map;
            map["Login"]    = share->login();
            map["Password"] = share->password();

            if ( !share->workgroupName().isEmpty() )
            {
              map["Workgroup"] = share->workgroupName().toUpper();
            }
            else
            {
              // Do nothing
            }

            if ( !share->hostIP().isEmpty() )
            {
              map["IP Address"] = share->hostIP();
            }
            else
            {
              // Do nothing
            }

            if ( !share->isHomesShare() )
            {
              d->wallet->writeMap( share->unc().toUpper(), map );
            }
            else
            {
              d->wallet->writeMap( share->homeUNC().toUpper(), map );
            }
            d->wallet->sync();
          }
          else
          {
            // Do nothing
          }

#ifdef Q_OS_FREEBSD
          Smb4KAuthInfo authInfo( share );
          writeToConfigFile( &authInfo );
#endif
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
    // Clear the list if necessary, i.e. if the wallet manager
    // should forget the previously used login(s).
    if ( !Smb4KSettings::rememberLogins() )
    {
      while ( !d->list.isEmpty() )
      {
        delete d->list.takeFirst();
      }
    }
    else
    {
      // Do nothing
    }

    // Now insert the new authentication information. Before we
    // do this, remove the old one, if one is present.
    switch ( networkItem->type() )
    {
      case Smb4KBasicNetworkItem::Host:
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );

        if ( host )
        {
          Smb4KAuthInfo authInfo( host );

          QMutableListIterator<Smb4KAuthInfo *> it( d->list );

          while ( it.hasNext() )
          {
            Smb4KAuthInfo *auth_info = it.next();

            if ( QString::compare( auth_info->unc(), authInfo.unc(), Qt::CaseInsensitive ) == 0 &&
                 (auth_info->workgroupName().isEmpty() || authInfo.workgroupName().isEmpty() ||
                 QString::compare( auth_info->workgroupName(), authInfo.workgroupName(), Qt::CaseInsensitive ) == 0) )
            {
              it.remove();
              break;
            }
            else
            {
              // Do nothing
            }
          }

          d->list << new Smb4KAuthInfo( authInfo );

#ifdef Q_OS_FREEBSD
          writeToConfigFile( &authInfo );
#endif
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Smb4KBasicNetworkItem::Share:
      {
        Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

        if ( share )
        {
          Smb4KAuthInfo authInfo( share );

          QMutableListIterator<Smb4KAuthInfo *> it( d->list );

          while ( it.hasNext() )
          {
            Smb4KAuthInfo *auth_info = it.next();

            if ( QString::compare( auth_info->unc(), authInfo.unc(), Qt::CaseInsensitive ) == 0 &&
                 (auth_info->workgroupName().isEmpty() || authInfo.workgroupName().isEmpty() ||
                 QString::compare( auth_info->workgroupName(), authInfo.workgroupName(), Qt::CaseInsensitive ) == 0) )
            {
              it.remove();
              break;
            }
            else
            {
              // Do nothing
            }
          }

          d->list << new Smb4KAuthInfo( authInfo );

#ifdef Q_OS_FREEBSD
          writeToConfigFile( &authInfo );
#endif
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
}


void Smb4KWalletManager::writeDefaultAuthInfo( Smb4KAuthInfo *authInfo )
{
  Q_ASSERT( authInfo );

  if ( useWalletSystem() && d->wallet )
  {
    // Write the default authentication information to the wallet
    if ( authInfo->type() == Smb4KAuthInfo::Default && !authInfo->login().isEmpty() /* allow empty passwords */ )
    {
      QMap<QString,QString> map;
      map["Login"]    = authInfo->login();
      map["Password"] = authInfo->password();
      d->wallet->writeMap( "DEFAULT_LOGIN", map );
      d->wallet->sync();
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


bool Smb4KWalletManager::showPasswordDialog( Smb4KBasicNetworkItem *networkItem, QWidget *parent )
{
  Q_ASSERT( networkItem );

  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will do nothing.
  init();

  // Return value
  bool success = false;

  // Read authentication information
  readAuthInfo( networkItem );

  // Set up the password dialog.
  QPointer<KPasswordDialog> dlg = new KPasswordDialog( parent, KPasswordDialog::ShowUsernameLine );

  switch ( networkItem->type() )
  {
    case Smb4KBasicNetworkItem::Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );

      if ( host )
      {
        dlg->setUsername( host->login() );
        dlg->setPassword( host->password() );
        dlg->setPrompt( i18n( "<qt>Please enter a username and a password for the host <b>%1</b>.</qt>", host->hostName() ) );

        // Execute the password dialog, retrieve the new authentication
        // information and save it.
        if ( (success = dlg->exec()) )
        {
          host->setLogin( dlg->username() );
          host->setPassword( dlg->password() );
          writeAuthInfo( host );
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
    case Smb4KBasicNetworkItem::Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

      if ( share )
      {
        // Get known logins in case we have a 'homes' share and the share
        // name has not yet been changed.
        QMap<QString,QString> logins;
        QStringList users = Smb4KHomesSharesHandler::self()->homesUsers( share );

        for ( int i = 0; i < users.size(); ++i )
        {
          Smb4KShare tmp_share( *share );
          tmp_share.setLogin( users.at( i ) );

          // Read the authentication data for the share. If it does not
          // exist yet, login() and password() will be empty.
          readAuthInfo( &tmp_share );
          logins.insert( tmp_share.login(), tmp_share.password() );
        }

        // Enter authentication information into the dialog
        if ( !logins.isEmpty() )
        {
          dlg->setKnownLogins( logins );
        }
        else
        {
          dlg->setUsername( share->login() );
          dlg->setPassword( share->password() );
        }

        if ( !share->isHomesShare() )
        {
          dlg->setPrompt( i18n( "<qt>Please enter a username and a password for the share <b>%1</b>.</qt>", share->unc() ) );
        }
        else
        {
          dlg->setPrompt( i18n( "<qt>Please enter a username and a password for the share <b>%1</b>.</qt>", share->homeUNC() ) );
        }

        // Execute the password dialog, retrieve the new authentication
        // information and save it.
        if ( (success = dlg->exec()) )
        {
          share->setLogin( dlg->username() );
          share->setPassword( dlg->password() );
          writeAuthInfo( share );
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
    default:
    {
      break;
    }
  }

  delete dlg;

  return success;
}


bool Smb4KWalletManager::useWalletSystem() const
{
  return (KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet());
}


Smb4KWalletManager::State Smb4KWalletManager::currentState() const
{
  return d->state;
}


QList<Smb4KAuthInfo *> Smb4KWalletManager::walletEntries()
{
  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will just do nothing.
  init();

  QList<Smb4KAuthInfo *> list;

  if ( useWalletSystem() && d->wallet )
  {
    QStringList entries = d->wallet->entryList();

    if ( !entries.isEmpty() )
    {
      for ( int i = 0; i < entries.size(); ++i )
      {
        Smb4KAuthInfo *authInfo = new Smb4KAuthInfo();

        QMap<QString,QString> map;
        d->wallet->readMap( entries.at( i ), map );

        if ( QString::compare( entries.at( i ), "DEFAULT_LOGIN" ) == 0 )
        {
          // Default login
          authInfo->useDefaultAuthInfo();
          authInfo->setLogin( map["Login"] );
          authInfo->setPassword( map["Password"] );
        }
        else
        {
          authInfo->setURL( entries.at( i ) );
          authInfo->setIP( map["IP Address"] );
          authInfo->setWorkgroupName( map["Workgroup"] );
          authInfo->setLogin( map["Login"] );
          authInfo->setPassword( map["Password"] );
        }

        list << authInfo;
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

  return list;
}


void Smb4KWalletManager::writeWalletEntries( const QList<Smb4KAuthInfo *> &entries )
{
  // Initialize the wallet manager. In case the wallet is already
  // set up, init() will just do nothing.
  init();

  if ( useWalletSystem() && d->wallet )
  {
    // Clear the wallet.
    QStringList entry_list = d->wallet->entryList();

    for ( int i = 0; i < entry_list.size(); ++i )
    {
      d->wallet->removeEntry( entry_list.at( i ) );
    }

    // Write the new entries to the wallet.
    for ( int i = 0; i < entries.size(); ++i )
    {
      QMap<QString,QString> map;

      if ( entries.at( i )->type() == Smb4KAuthInfo::Default )
      {
        // Default login
        map["Login"] = entries.at( i )->login();
        map["Password"] = entries.at( i )->password();
        d->wallet->writeMap( "DEFAULT_LOGIN", map );
      }
      else
      {
        map["IP Address"] = entries.at( i )->ip();
        map["Workgroup"] = entries.at( i )->workgroupName();
        map["Login"] = entries.at( i )->login();
        map["Password"] = entries.at( i )->password();
        d->wallet->writeMap( entries.at( i )->unc(), map );
      }
    }

    d->wallet->sync();
  }
  else
  {
    // Do nothing
  }
}


#ifdef Q_OS_FREEBSD

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
    Smb4KNotification *notification = new Smb4KNotification();
    notification->commandNotFound( "smbutil" );
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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->openingFileFailed( file );
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
    QMap<QString,QString> global_options = globalSambaOptions();

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
      default_section << "charsets="+client_charset+':'+server_charset;
    }
    else
    {
      // Do nothing
    }

    // WINS server
    QString wins_server = winsServer();

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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->processError( proc.error() );
      return;
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
        if ( contents.contains( '['+authInfo->hostName().toUpper()+':'+authInfo->login().toUpper()+']' ) )
        {
          index = contents.indexOf( '['+authInfo->hostName().toUpper()+':'+authInfo->login().toUpper()+']' );
        }
        else if ( contents.contains( '['+authInfo->hostName().toLower()+':'+authInfo->login().toLower()+']' ) )
        {
          index = contents.indexOf( '['+authInfo->hostName().toLower()+':'+authInfo->login().toLower()+']' );
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
            if ( contents.at( i ).startsWith( QLatin1String( "password" ), Qt::CaseInsensitive ) )
            {
              if ( QString::compare( contents.at( i ).section( '=', 1, 1 ).trimmed(), password ) != 0 &&
                   !authInfo->password().isEmpty() /* we do not want empty server passwords */ )
              {
                contents[i].replace( contents.at( i ).section( '=', 1, 1 ).trimmed(), password );
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

            contents << '['+authInfo->hostName().toUpper()+':'+authInfo->login().toUpper()+']';
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
        if ( contents.contains( '['+authInfo->hostName().toUpper()+']' ) )
        {
          index = contents.indexOf( '['+authInfo->hostName().toUpper()+']' );
        }
        else if ( contents.contains( '['+authInfo->hostName().toLower()+']' ) )
        {
          index = contents.indexOf( '['+authInfo->hostName().toLower()+']' );
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
            if ( contents.at( i ).startsWith( QLatin1String( "password" ), Qt::CaseInsensitive ) )
            {
              if ( QString::compare( contents.at( i ).section( '=', 1, 1 ).trimmed(), password ) != 0 &&
                   !authInfo->password().isEmpty() /* we do not want empty server passwords */ )
              {
                contents[i].replace( contents.at( i ).section( '=', 1, 1 ).trimmed(), password );
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

            contents << '['+authInfo->hostName().toUpper()+']';
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
        if ( contents.contains( '['+authInfo->hostName().toUpper()+':'+authInfo->login().toUpper()+':'+authInfo->shareName().toUpper()+']' ) )
        {
          index = contents.indexOf( '['+authInfo->hostName().toUpper()+':'+authInfo->login().toUpper()+':'+authInfo->shareName().toUpper()+']' );
        }
        else if ( contents.contains( '['+authInfo->hostName().toLower()+':'+authInfo->login().toLower()+':'+authInfo->shareName().toLower()+']' ) )
        {
          index = contents.indexOf( '['+authInfo->hostName().toLower()+':'+authInfo->login().toLower()+':'+authInfo->shareName().toLower()+']' );
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
            if ( contents.at( i ).startsWith( QLatin1String( "password" ), Qt::CaseInsensitive ) )
            {
              if ( QString::compare( contents.at( i ).section( '=', 1, 1 ).trimmed(), password ) != 0 &&
                   !authInfo->password().isEmpty() )
              {
                contents[i].replace( contents.at( i ).section( '=', 1, 1 ).trimmed(), password );
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

            contents << '['+authInfo->hostName().toUpper()+':'+authInfo->login().toUpper()+':'+authInfo->shareName().toUpper()+']';
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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->openingFileFailed( file );
      return;
    }
  }
  else
  {
    // Do nothing
  }
}

#endif


#include "smb4kwalletmanager.moc"
