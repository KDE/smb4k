/***************************************************************************
    smb4kwalletmanager  -  This is the wallet manager of Smb4K.
                             -------------------
    begin                : Sa Dez 27 2008
    copyright            : (C) 2008-2014 by Alexander Reinholdt
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
#include "smb4kwalletmanager_p.h"
#include "smb4ksettings.h"
#include "smb4kauthinfo.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"
#include "smb4knotification.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// Qt includes
#include <QtCore/QPointer>
#include <QtTest/QTest>

// KDE includes
#include <kglobal.h>
#include <kdebug.h>
#include <kapplication.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KWalletManagerStatic, p );



Smb4KWalletManager::Smb4KWalletManager( QObject *parent )
: QObject( parent ), d( new Smb4KWalletManagerPrivate )
{
  d->wallet = NULL;
  d->initialized = false;
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

      d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), (parent ? parent->winId() : 0), KWallet::Wallet::Asynchronous );
      connect( d->wallet, SIGNAL(walletOpened(bool)), this, SLOT(slotWalletOpened(bool)) );
    }
    else
    {
      // Do nothing. We already have a wallet.
    }
    
    // Wait until the wallet has been opened or the 
    // user canceled the dialog.
    while ( !d->initialized )
    {
      QTest::qWait( 250 );
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

    d->initialized = true;
    emit initialized();
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
      case Host:
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
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Share:
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
      authInfo->setUserName( map["Login"] );
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
      case Host:
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
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Share:
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


void Smb4KWalletManager::writeDefaultAuthInfo( Smb4KAuthInfo *authInfo )
{
  Q_ASSERT( authInfo );

  if ( useWalletSystem() && d->wallet )
  {
    // Write the default authentication information to the wallet.
    if ( !authInfo->userName().isEmpty() /* allow empty passwords */ )
    {
      QMap<QString,QString> map;
      map["Login"]    = authInfo->userName();
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

  // Get known logins if available and read the authentication
  // information.
  QMap<QString, QString> known_logins;

  switch ( networkItem->type() )
  {
    case Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

      if ( share )
      {
        QStringList users = Smb4KHomesSharesHandler::self()->homesUsers( share );

        for ( int i = 0; i < users.size(); ++i )
        {
          Smb4KShare *tmp_share = new Smb4KShare( *share );
          tmp_share->setLogin( users.at( i ) );

          // Read the authentication data for the share. If it does not
          // exist yet, login() and password() will be empty.
          readAuthInfo( tmp_share );
          known_logins.insert( tmp_share->login(), tmp_share->password() );

          delete tmp_share;
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
      readAuthInfo( networkItem );
      break;
    }
  }

  // Set up the password dialog and show it.
  QPointer<Smb4KPasswordDialog> dlg = new Smb4KPasswordDialog( networkItem, known_logins, parent );
 
  // Return value
  bool success = false;
  
  if ( dlg->exec() == Smb4KPasswordDialog::Accepted )
  {
    // Write the authentication information.
    writeAuthInfo( networkItem );
    success = true;
  }
  else
  {
    // Do nothing
  }

  delete dlg;

  return success;
}


bool Smb4KWalletManager::useWalletSystem() const
{
  return (KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet());
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
          authInfo->setUserName( map["Login"] );
          authInfo->setPassword( map["Password"] );
        }
        else
        {
          authInfo->setURL( entries.at( i ) );
          authInfo->setIP( map["IP Address"] );
          authInfo->setWorkgroupName( map["Workgroup"] );
          authInfo->setUserName( map["Login"] );
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

      if ( entries.at( i )->type() == Unknown )
      {
        // Default login
        map["Login"] = entries.at( i )->userName();
        map["Password"] = entries.at( i )->password();
        d->wallet->writeMap( "DEFAULT_LOGIN", map );
      }
      else
      {
        map["IP Address"] = entries.at( i )->ip();
        map["Workgroup"] = entries.at( i )->workgroupName();
        map["Login"] = entries.at( i )->userName();
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


bool Smb4KWalletManager::walletIsOpen() const
{
  return (d->wallet ? (useWalletSystem() && d->wallet->isOpen()) : false);
}


void Smb4KWalletManager::slotWalletOpened(bool success)
{
  if ( success )
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
      Smb4KNotification::credentialsNotAccessible();
    }
  }
  else
  {
    delete d->wallet;
    d->wallet = NULL;
    
    Smb4KNotification::openingWalletFailed(KWallet::Wallet::NetworkWallet());
  }

  d->initialized = true;
  emit initialized();
}


#include "smb4kwalletmanager.moc"
