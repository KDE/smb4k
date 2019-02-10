/***************************************************************************
    This is the wallet manager of Smb4K.
                             -------------------
    begin                : Sa Dez 27 2008
    copyright            : (C) 2008-2019 by Alexander Reinholdt
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
#include <QPointer>
#include <QTest>
#include <QApplication>

// KDE includes

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KWalletManagerStatic, p);



Smb4KWalletManager::Smb4KWalletManager(QObject *parent)
: QObject(parent), d(new Smb4KWalletManagerPrivate)
{
  d->wallet = 0;
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
  if (useWalletSystem())
  {
    //
    // Get a pointer to the wallet, if we do not have one yet
    // 
    if (!d->wallet)
    {
      //
      // Open the wallet synchronously.
      // 
      d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), QApplication::activeWindow() ? QApplication::activeWindow()->winId() : 0);
      
      //
      // Check if the walled was opened successfully and set the
      // right folder, if it was
      // 
      if (d->wallet)
      {
        if (d->wallet->isOpen())
        {
          if (!d->wallet->hasFolder("Smb4K"))
          {
            d->wallet->createFolder("Smb4K");
            d->wallet->setFolder("Smb4K");
          }
          else
          {
            d->wallet->setFolder("Smb4K");
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
        d->wallet = 0;
        
        Smb4KNotification::openingWalletFailed(KWallet::Wallet::NetworkWallet());
      }
    }
  }
  else
  {
    if (d->wallet)
    {
      //
      // Close the wallet, if Smb4K is the only application that 
      // is using it. Thus, use force=false, otherwise it will definitely
      // be closed.
      //
      d->wallet->closeWallet(KWallet::Wallet::NetworkWallet(), false);
      
      //
      // Delete the wallet and set it to 0.
      // 
      delete d->wallet;
      d->wallet = 0;
    }
  }
  
  emit initialized();
}


void Smb4KWalletManager::readAuthInfo(const NetworkItemPtr &networkItem)
{
  //
  // Only do something, if the networkItem is not null
  // 
  if (networkItem)
  {
    //
    // Initialize the wallet manager. 
    // 
    init();
    
    //
    // Proceed if the wallet is open
    // 
    if (walletIsOpen())
    {
      //
      // Get the list of entries
      // 
      QStringList entryList = d->wallet->entryList();
      
      //
      // Create the map to store the authentication information
      // 
      QMap<QString, QString> authInfoMap;
      
      //
      // Now loop through the stored credentials and make a case insensitive comparison 
      // with the URL of the network item.
      // 
      for (const QString &entry : entryList)
      {
        if (networkItem->type() == Host)
        {
          if (QString::compare(entry, networkItem->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort), Qt::CaseInsensitive) == 0 ||
              QString::compare(entry, networkItem->url().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort), Qt::CaseInsensitive) == 0)
          {
            d->wallet->readMap(entry, authInfoMap);
            break;
          }
        }
        else if (networkItem->type() == Share)
        {
          //
          // Cast the network item. We need some share specific info
          // 
          SharePtr share = networkItem.staticCast<Smb4KShare>();
          
          if (share)
          {
            //
            // Process normal and 'homes' shares differently
            // 
            if (!share->isHomesShare())
            {
              //
              // Prefer the credentials for the share. Use the ones for the 
              // host as fallback.
              //
              if (QString::compare(entry, share->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort), Qt::CaseInsensitive) == 0 ||
                  QString::compare(entry, share->url().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort), Qt::CaseInsensitive) == 0)
              {
                d->wallet->readMap(entry, authInfoMap);
                break;
              }
              else if (QString::compare(entry, share->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath), Qt::CaseInsensitive) == 0 ||
                       QString::compare(entry, share->url().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath), Qt::CaseInsensitive) == 0)
              {
                d->wallet->readMap(entry, authInfoMap);
              }
            }
            else
            {
              //
              // Prefer the credentials for the share. Use the ones for the 
              // host as fallback.
              //
              if (QString::compare(entry, share->homeUrl().toString(QUrl::RemoveUserInfo|QUrl::RemovePort), Qt::CaseInsensitive) == 0 ||
                  QString::compare(entry, share->homeUrl().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort), Qt::CaseInsensitive) == 0)
              {
                d->wallet->readMap(entry, authInfoMap);
                break;
              }
              else if (QString::compare(entry, share->homeUrl().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath), Qt::CaseInsensitive) == 0 ||
                       QString::compare(entry, share->homeUrl().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath), Qt::CaseInsensitive) == 0)
              {
                d->wallet->readMap(entry, authInfoMap);
              }
            }
          }
        }
      }
      
      //
      // Set the authentication information
      // 
      if (!authInfoMap.isEmpty())
      {
        switch (networkItem->type())
        {
          case Host:
          {
            HostPtr host = networkItem.staticCast<Smb4KHost>();
            
            if (QString::compare(host->workgroupName(), authInfoMap.value("Workgroup"), Qt::CaseInsensitive) == 0)
            {
              host->setLogin(authInfoMap.value("Login"));
              host->setPassword(authInfoMap.value("Password"));
            }
            
            break;
          }
          case Share:
          {
            SharePtr share = networkItem.staticCast<Smb4KShare>();
            
            if (QString::compare(share->workgroupName(), authInfoMap.value("Workgroup"), Qt::CaseInsensitive) == 0)
            {
              share->setLogin(authInfoMap.value("Login"));
              share->setPassword(authInfoMap.value("Password"));
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
        //
        // In case the map is empty, set the default login, if it is to be used
        // 
        if (Smb4KSettings::useDefaultLogin())
        {
          d->wallet->readMap("DEFAULT_LOGIN", authInfoMap);
          
          switch (networkItem->type())
          {
            case Host:
            {
              HostPtr host = networkItem.staticCast<Smb4KHost>();
              host->setLogin(authInfoMap.value("Login"));
              host->setPassword(authInfoMap.value("Password"));
              
              break;
            }
            case Share:
            {
              SharePtr share = networkItem.staticCast<Smb4KShare>();
              share->setLogin(authInfoMap.value("Login"));
              share->setPassword(authInfoMap.value("Password"));
              
              break;
            }
            default:
            {
              break;
            }
          }
        }
      }
    }
  }
}


void Smb4KWalletManager::readDefaultAuthInfo(Smb4KAuthInfo *authInfo)
{
  if (authInfo)
  {
    //
    // Initialize the wallet manager.
    // 
    init();
    
    if (walletIsOpen())
    {
      //
      // Read the default authentication information from the
      // wallet.
      // 
      QMap<QString, QString> authInfoMap;
      d->wallet->readMap("DEFAULT_LOGIN", authInfoMap);
      
      if (!authInfoMap.isEmpty())
      {
        authInfo->setUserName(authInfoMap.value("Login"));
        authInfo->setPassword(authInfoMap.value("Password"));
      }
    }
  }
}


void Smb4KWalletManager::writeAuthInfo(const NetworkItemPtr &networkItem)
{
  if (networkItem)
  {
    //
    // Initialize the wallet manager.
    // 
    init();
    
    if (walletIsOpen())
    {
      //
      // Handle the network item according to its type
      // 
      switch (networkItem->type())
      {
        case Host:
        {
          //
          // Cast the network item
          // 
          HostPtr host = networkItem.staticCast<Smb4KHost>();
          
          if (host)
          {
            //
            // Write the authentication information to the wallet, if it
            // is not empty.
            // 
            if (!host->login().isEmpty() /* allow empty passwords */ && !host->hostName().isEmpty())
            {
              //
              // Create the map that carries the authentication information
              // 
              QMap<QString, QString> authInfoMap;
              
              //
              // Insert login and password
              // 
              authInfoMap.insert("Login", host->login());
              authInfoMap.insert("Password", host->password());
              
              //
              // Enter the workgroup, if it exists
              // 
              if (!host->workgroupName().isEmpty())
              {
                authInfoMap.insert("Workgroup", host->workgroupName());
              }
              
              //
              // Enter the IP address, if is exists
              // 
              if (host->hasIpAddress())
              {
                authInfoMap.insert("IP Address", host->ipAddress());
              }
              
              //
              // Write the entry to the wallet
              // 
              d->wallet->writeMap(host->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort), authInfoMap);
              d->wallet->sync();
            }
          }

          break;
        }
        case Share:
        {
          //
          // Cast the network item
          // 
          SharePtr share = networkItem.staticCast<Smb4KShare>();
          
          if (share)
          {
            //
            // Write the authentication information to the wallet, if it
            // is not empty.
            // 
            if (!share->login().isEmpty() /* allow empty passwords */ && !share->hostName().isEmpty())
            {
              //
              // Create the map that carries the authentication information
              // 
              QMap<QString, QString> authInfoMap;
              
              //
              // Insert login and password
              // 
              authInfoMap.insert("Login", share->login());
              authInfoMap.insert("Password", share->password());
              
              //
              // Enter the workgroup, if it exists
              // 
              if (!share->workgroupName().isEmpty())
              {
                authInfoMap.insert("Workgroup", share->workgroupName());
              }
              
              //
              // Enter the IP address, if is exists
              // 
              if (share->hasHostIpAddress())
              {
                authInfoMap.insert("IP Address", share->hostIpAddress());
              }
              
              //
              // Write the entry to the wallet
              // 
              if (!share->isHomesShare())
              {
                d->wallet->writeMap(share->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort), authInfoMap);
              }
              else
              {
                d->wallet->writeMap(share->homeUrl().toString(QUrl::RemoveUserInfo|QUrl::RemovePort), authInfoMap);
              }
            }
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
}


void Smb4KWalletManager::writeDefaultAuthInfo(Smb4KAuthInfo *authInfo)
{
  if (authInfo)
  {
    //
    // Initialize the wallet manager.
    // 
    init();
    
    if (walletIsOpen())
    {
      //
      // Write the default authentication information to the
      // wallet.
      // 
      if (!authInfo->userName().isEmpty() /* allow empty passwords */)
      {
        QMap<QString, QString> authInfoMap;
        authInfoMap.insert("Login", authInfo->userName());
        authInfoMap.insert("Password", authInfo->password());
        
        d->wallet->writeMap("DEFAULT_LOGIN", authInfoMap);
        d->wallet->sync();
      }
    }
  }
}


bool Smb4KWalletManager::showPasswordDialog(const NetworkItemPtr &networkItem)
{
  //
  // Define the return value here
  // 
  bool success = false;
  
  //
  // Check that the network item is not null
  // 
  if (networkItem)
  {
    //
    // Initialize the wallet manager. 
    // 
    init();
    
    //
    // Get the known logins (for homes shares) and read the authentication 
    // information.
    // 
    QMap<QString, QString> knownLogins;
    
    switch(networkItem->type())
    {
      case Share:
      {
        //
        // Cast the network item
        // 
        SharePtr share = networkItem.staticCast<Smb4KShare>();
        
        //
        // If the share is a 'homes' share, read the known logins
        // for that share.
        // 
        if (share && share->isHomesShare())
        {
          //
          // Get the known logins
          // 
          QStringList users = Smb4KHomesSharesHandler::self()->homesUsers(share);
          
          //
          // Read the authentication information for all known logins
          // 
          for (const QString &user : users)
          {
            //
            // Create a temp share
            // 
            SharePtr tempShare = share;
            
            // 
            // Set the login
            // 
            tempShare->setLogin(user);
            
            //
            // Read the authentication information
            // 
            readAuthInfo(tempShare);
            
            //
            // Save the authentication data in the map
            // 
            knownLogins.insert(tempShare->login(), tempShare->password());
            
            //
            // Clear the temp share
            // 
            tempShare.clear();
          }
        }

        break;
      }
      default:
      {
        readAuthInfo(networkItem);
        break;
      }
    }
    
    // 
    // Set up the password dialog and show it
    // 
    QPointer<Smb4KPasswordDialog> dlg = new Smb4KPasswordDialog(networkItem, knownLogins, QApplication::activeWindow());
    
    if (dlg->exec() == Smb4KPasswordDialog::Accepted)
    {
      // Write the authentication information.
      writeAuthInfo(networkItem);
      success = true;
    }

    delete dlg;
  }

  return success;
}


bool Smb4KWalletManager::useWalletSystem() const
{
  return (KWallet::Wallet::isEnabled() && Smb4KSettings::useWallet());
}


QList<Smb4KAuthInfo *> Smb4KWalletManager::walletEntries()
{
  //
  // Initialize the wallet manager.
  // 
  init();
  
  //
  // Define the return value
  // 
  QList<Smb4KAuthInfo *> entries;
  
  //
  // Only read from the wallet if it is open
  // 
  if (walletIsOpen())
  {
    //
    // Get all entries from the wallet
    // 
    QStringList entryList = d->wallet->entryList();
    
    //
    // Process the entries
    // 
    for (const QString &entry : entryList)
    {
      //
      // Create a auth info object
      // 
      Smb4KAuthInfo *authInfo = new Smb4KAuthInfo();
      
      //
      // Read the authentication information from the wallet
      // 
      QMap<QString, QString> authInfoMap;
      d->wallet->readMap(entry, authInfoMap);
      
      //
      // Process the entry
      // 
      if (entry == "DEFAULT_LOGIN")
      {
        //
        // Default login
        // 
        authInfo->setUserName(authInfoMap.value("Login"));
        authInfo->setPassword(authInfoMap.value("Password"));
      }
      else
      {
        //
        // Entry for a specific URL
        // 
        authInfo->setUrl(entry);
        authInfo->setIpAddress(authInfoMap.value("IP Address"));
        authInfo->setWorkgroupName(authInfoMap.value("Workgroup"));
        authInfo->setUserName(authInfoMap.value("Login"));
        authInfo->setPassword(authInfoMap.value("Password"));
      }
      
      entries << authInfo;
    }
  }
  
  return entries;
}


void Smb4KWalletManager::writeWalletEntries(const QList<Smb4KAuthInfo *> &entries)
{
  //
  // Initialize the wallet manager.
  // 
  init();
  
  //
  // Write the list if the wallet is open
  // 
  if (walletIsOpen())
  {
    //
    // First clear the wallet
    // 
    QStringList entryList = d->wallet->entryList();
    
    for (const QString &entry : entryList)
    {
      d->wallet->removeEntry(entry);
    }
    
    //
    // Now write the new entries to the wallet
    // 
    for (Smb4KAuthInfo *authInfo : entries)
    {
      QMap<QString, QString> authInfoMap;
      
      if (authInfo->type() == UnknownNetworkItem)
      {
        //
        // Default login
        // 
        authInfoMap.insert("Login", authInfo->userName());
        authInfoMap.insert("Password", authInfo->password());
        
        //
        // Write the default authentication information to the wallet
        // 
        d->wallet->writeMap("DEFAULT_LOGIN", authInfoMap);
      }
      else
      {
        //
        // Authentication information for a specific URL
        // 
        authInfoMap.insert("IP Address", authInfo->ipAddress());
        authInfoMap.insert("Workgroup", authInfo->workgroupName());
        authInfoMap.insert("Login", authInfo->userName());
        authInfoMap.insert("Password", authInfo->password());
        
        //
        // Write the authentication information to the wallet
        // 
        d->wallet->writeMap(authInfo->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort), authInfoMap);
      }
      
      //
      // Sync the entries to disk
      // 
      d->wallet->sync();
    }
  }
}


bool Smb4KWalletManager::walletIsOpen() const
{
  return (d->wallet ? (useWalletSystem() && d->wallet->isOpen()) : false);
}

