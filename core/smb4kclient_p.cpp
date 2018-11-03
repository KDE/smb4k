/***************************************************************************
    Private classes for the SMB client
                             -------------------
    begin                : So Oct 21 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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
#include "smb4kclient_p.h"
#include "smb4ksettings.h"
#include "smb4kwalletmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"

// System includes
#include <string.h>
#include <errno.h>

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QAbstractSocket>

#define SMBC_DEBUG 1

using namespace Smb4KGlobal;


//
// Authentication function for libsmbclient
// 
static void get_auth_data_with_context_fn(SMBCCTX *context,
                                          const char *server, 
                                          const char *share,
                                          char *workgroup,
                                          int maxLenWorkgroup,
                                          char *username,
                                          int maxLenUsername,
                                          char *password,
                                          int maxLenPassword)
{
  if (context != nullptr)
  {
    Smb4KClientJob *job = static_cast<Smb4KClientJob *>(smbc_getOptionUserData(context));
    
    if (job)
    {
      job->get_auth_data_fn(server, share, workgroup, maxLenWorkgroup, username, maxLenUsername, password, maxLenPassword);
    }
    else
    {
      // Do nothing
    }
  }
}



//
// Client job
// 
Smb4KClientJob::Smb4KClientJob(QObject* parent) 
: KJob(parent)
{
}


Smb4KClientJob::~Smb4KClientJob()
{
  //
  // Clear the list of workgroups
  //
  while (!m_workgroups.isEmpty())
  {
    m_workgroups.takeFirst().clear();
  }
  
  //
  // Clear the list of hosts
  // 
  while (!m_hosts.isEmpty())
  {
    m_hosts.takeFirst().clear();
  }
  
  //
  // Clear the list of shares
  //
  while (!m_shares.isEmpty())
  {
    m_shares.takeFirst().clear();
  }
}


void Smb4KClientJob::start()
{
  QTimer::singleShot(50, this, SLOT(slotStartJob()));
}


void Smb4KClientJob::setNetworkItem(NetworkItemPtr item)
{
  m_item = item;
}


NetworkItemPtr Smb4KClientJob::networkItem() const
{
  return m_item;
}


void Smb4KClientJob::get_auth_data_fn(const char* server, const char* share, char* workgroup, int maxLenWorkgroup, char* username, int maxLenUsername, char* password, int maxLenPassword)
{
  //
  // Authentication
  // 
  switch (m_item->type())
  {
    case Network:
    {
      //
      // No authentication needed
      // 
      break;
    }
    case Workgroup:
    {
      //
      // Only request authentication data, if the master browsers require 
      // authentication data.
      // 
      if (Smb4KSettings::masterBrowsersRequireAuth())
      {
        if (QString::fromUtf8(server).toUpper() != QString::fromUtf8(workgroup).toUpper())
        {
          // 
          // This is the master browser. Create a host object for it.
          // 
          HostPtr host = HostPtr(new Smb4KHost());
          host->setWorkgroupName(QString::fromUtf8(workgroup));
          host->setHostName(QString::fromUtf8(server));
          
          // 
          // Get the authentication data
          // 
          Smb4KWalletManager::self()->readAuthInfo(host);
          
          // 
          // Copy the authentication data
          // 
          qstrncpy(username, host->login().toUtf8().data(), maxLenUsername);
          qstrncpy(password, host->password().toUtf8().data(), maxLenPassword);
        }
        else
        {
          // 
          // This is the workgroup name. Use anonymous login for it
          // 
          qstrncpy(username, "anonymous", sizeof("anonymous"));
        }
      }
      else
      {
        //
        // If no authentication data is required to log on to the master browsers,
        // use the anonymous login with an empty password.
        // 
        qstrncpy(username, "anonymous", sizeof("anonymous"));
        qstrncpy(password, "", sizeof(""));
      }
      
      break;
    }
    case Host:
    {
      //
      // Create a host object
      // 
      HostPtr host = HostPtr(new Smb4KHost());
      host->setWorkgroupName(QString::fromUtf8(workgroup));
      host->setHostName(QString::fromUtf8(server));
      
      //
      // Copy the workgroup name also to the private variable
      // 
      m_workgroup = QString::fromUtf8(workgroup);
      
      //
      // Get the authentication data
      //
      Smb4KWalletManager::self()->readAuthInfo(host);
          
      // 
      // Copy the authentication data
      // 
      qstrncpy(username, host->login().toUtf8().data(), maxLenUsername);
      qstrncpy(password, host->password().toUtf8().data(), maxLenPassword);
      
      //
      // Set the authentication data also for the network item
      // 
      QUrl url = m_item->url();
      url.setUserName(host->login());
      url.setPassword(host->password());
      m_item->setUrl(url);
      
      break;
    }
    case Share:
    {
      break;
    }
    default:
    {
      break;
    }
  }
}


QList<WorkgroupPtr> Smb4KClientJob::workgroups()
{
  return m_workgroups;
}


QList<HostPtr> Smb4KClientJob::hosts()
{
  return m_hosts;
}


QList<SharePtr> Smb4KClientJob::shares()
{
  return m_shares;
}


QString Smb4KClientJob::workgroup()
{
  QString workgroup;
  
  switch (m_item->type())
  {
    case Network:
    {
      break;
    }
    case Workgroup:
    {
      workgroup = m_item->url().host().toUpper();
      break;
    }
    default:
    {
      workgroup = m_workgroup;
      break;
    }
  }
  
  return workgroup;
}



QHostAddress Smb4KClientJob::lookupIpAddress(const QString& name)
{
  //
  // The IP address object
  // 
  QHostAddress ipAddress;
  
  // If the IP address is not to be determined for the local machine, we can use QHostInfo to
  // determine it. Otherwise we need to use QNetworkInterface for it.
  if (name == QHostInfo::localHostName().toUpper() || name == globalSambaOptions(true)["netbios name"] || name == Smb4KSettings::netBIOSName())
  {
    // FIXME: Do we need to honor 'interfaces' here?
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    
    // Get the IP address for the host. For the time being, prefer the 
    // IPv4 address over the IPv6 address.
    for (const QHostAddress &addr : addresses)
    {
      // We only use global addresses.
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
      if (addr.isGlobal())
#else
      if (!addr.isLoopback() && !addr.isMulticast())
#endif
      {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol)
        {
          ipAddress = addr;
          break;
        }
        else if (addr.protocol() == QAbstractSocket::IPv6Protocol)
        {
          // FIXME: Use the right address here.
          ipAddress = addr;
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
  }
  else
  {
    // Get the IP address
    QHostInfo hostInfo = QHostInfo::fromName(name);
            
    if (hostInfo.error() == QHostInfo::NoError)
    {
      // Get the IP address for the host. For the time being, prefer the 
      // IPv4 address over the IPv6 address.
      for (const QHostAddress &addr : hostInfo.addresses())
      {
        // We only use global addresses.
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        if (addr.isGlobal())
#else
        if (!addr.isLoopback() && !addr.isMulticast())
#endif
        {
          if (addr.protocol() == QAbstractSocket::IPv4Protocol)
          {
            ipAddress = addr;
            break;
          }
          else if (addr.protocol() == QAbstractSocket::IPv6Protocol)
          {
            // FIXME: Use the right address here.
            ipAddress = addr;
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
    }
    else
    {
      // Do nothing
    }
  }
  
  return ipAddress;
}


void Smb4KClientJob::slotStartJob()
{
  // 
  // Allocate new context
  // 
  m_context = smbc_new_context();
  
  if (!m_context)
  {
    int errorCode = errno;
    
    switch (errorCode)
    {
      case ENOMEM:
      {
        setError(OutOfMemoryError);
        break;
      }
      default:
      {
        setError(UnknownError);
        break;
      }
    }
    
    emitResult();
  }
  
  //
  // Get the custom options
  // 
  OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(m_item);
  
  // 
  // Set debug level
  // 
  smbc_setDebug(m_context, SMBC_DEBUG);
  
  //
  // Set the NetBIOS name and the workgroup to make connections
  // 
  switch (m_item->type())
  {
    case Network:
    {
      // 
      // We do not know about the servers and the domains/workgroups
      // present. So, do not set anything and use the default.
      // 
      break;
    }
    case Workgroup:
    {
      //
      // Set the NetBIOS name of the master browser and the workgroup to be queried
      // 
      WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
      smbc_setNetbiosName(m_context, workgroup->masterBrowserName().toUtf8().data());
      smbc_setWorkgroup(m_context, workgroup->workgroupName().toUtf8().data());
      break;
    }
    case Host:
    {
      //
      // Set both the NetBIOS name of the server and the workgroup to be queried
      // 
      HostPtr host = m_item.staticCast<Smb4KHost>();
      smbc_setNetbiosName(m_context, host->hostName().toUtf8().data());
      smbc_setWorkgroup(m_context, host->workgroupName().toUtf8().data());
      break;
    }
    default:
    {
      //
      // Set both the NetBIOS name of the server and the workgroup to be queried
      // 
      HostPtr host = findHost(m_item->url().host());
      smbc_setNetbiosName(m_context, host->hostName().toUtf8().data());
      smbc_setWorkgroup(m_context, host->workgroupName().toUtf8().data());
      break;
    }
  }

  //
  // Set auth callback function
  // 
  smbc_setFunctionAuthDataWithContext(m_context, get_auth_data_with_context_fn);
  
  //
  // Set the user data (this class)
  // 
  smbc_setOptionUserData(m_context, this);
  
  //
  // Set number of master browsers to be used
  // 
  smbc_setOptionBrowseMaxLmbCount(m_context, 0 /* all master browsers */);
  
  //
  // Set the encryption level
  // 
//   smbc_setOptionSmbEncryptionLevel(m_context, );
  
  //
  // Set the usage of the winbind ccache
  // 
  smbc_setOptionUseCCache(m_context, Smb4KSettings::useWinbindCCache());
  
  //
  // Set usage of Kerberos
  // 
  if (options)
  {
    smbc_setOptionUseKerberos(m_context, options->useKerberos());
  }
  else
  {
    smbc_setOptionUseKerberos(m_context, Smb4KSettings::useKerberos());
  }
  
  smbc_setOptionFallbackAfterKerberos(m_context, 1);
  
  //
  // Set the channel for debug output
  // 
  smbc_setOptionDebugToStderr(m_context, 1);
  
  // 
  // Initialize context with the previously defined options
  // 
  if (!smbc_init_context(m_context))
  {
    int errorCode = errno;
    
    switch (errorCode)
    {
      case ENOMEM:
      {
        setError(OutOfMemoryError);
        break;
      }
      case EBADF:
      {
        setError(NullContextError);
        break;
      }
      case ENOENT:
      {
        setError(SmbConfError);
        break;
      }
      default:
      {
        setError(UnknownError);
      }
    }

    smbc_free_context(m_context, 0);

    emitResult();
  }
  
  //
  // Set the new context
  // 
  (void)smbc_set_context(m_context);
  
  // 
  // Read the given URL
  // 
  int dirfd;
  struct smbc_dirent *dirp = nullptr;
  
  dirfd = smbc_opendir(m_item->url().toString().toUtf8().data());
  
  if (dirfd < 0)
  {
    int errorCode = errno;
    
    switch (errorCode)
    {
      case ENOMEM:
      {
        setError(OutOfMemoryError);
        break;
      }
      case EACCES:
      {
        setError(AccessDeniedError);
        break;
      }
      case EINVAL:
      {
        setError(InvalidUrlError);
        break;
      }
      case ENOENT:
      {
        setError(UrlExistError);
        break;
      }
      case ENOTDIR:
      {
        setError(NoDirectoryError);
        break;
      }
      case EPERM:
      {
        setError(NotPermittedError);
        break;
      }
      case ENODEV:
      {
        setError(NotFoundError);
        break;
      }
      default:
      {
        setError(UnknownError);
      }
    }

    emitResult();
  }
  else
  {
    //
    // Get the entries of the "directory"
    // 
    while ((dirp = smbc_readdir(dirfd)) != nullptr)
    {
      switch (dirp->smbc_type)
      {
        case SMBC_WORKGROUP:
        {
          // 
          // Create a workgroup pointer
          // 
          WorkgroupPtr workgroup = WorkgroupPtr(new Smb4KWorkgroup());
          
          // 
          // Set the workgroup name
          // 
          QString workgroupName = QString::fromUtf8(dirp->name);
          workgroup->setWorkgroupName(workgroupName);
          
          // 
          // Set the master browser
          // 
          QString masterBrowserName = QString::fromUtf8(dirp->comment);
          workgroup->setMasterBrowserName(masterBrowserName);
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(masterBrowserName);
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip the
          // workgroup and delete the pointer.
          // 
          if (!address.isNull())
          {
            workgroup->setMasterBrowserIpAddress(address);
            m_workgroups << workgroup;
          }
          else
          {
            workgroup.clear();
          }
          
          break;
        }
        case SMBC_SERVER:
        {
          // 
          // Create a host pointer
          // 
          HostPtr host = HostPtr(new Smb4KHost());
          
          // 
          // Set the workgroup name
          // 
          host->setWorkgroupName(m_item->url().host());
          
          // 
          // Set the host name
          // 
          QString hostName = QString::fromUtf8(dirp->name);
          host->setHostName(hostName);
          
          // 
          // Set the comment
          // 
          QString comment = QString::fromUtf8(dirp->comment);
          host->setComment(comment);
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(hostName);
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            host->setIpAddress(address);
            m_hosts << host;
          }
          else
          {
            host.clear();
          } 
          
          break;
        }
        case SMBC_FILE_SHARE:
        {
          //
          // Create a share pointer
          // 
          SharePtr share = SharePtr(new Smb4KShare());
          
          //
          // Set the workgroup name
          // 
          share->setWorkgroupName(m_workgroup);
          
          //
          // Set the host name
          // 
          share->setHostName(m_item->url().host());
          
          //
          // Set the share name
          // 
          share->setShareName(QString::fromUtf8(dirp->name));
          
          // 
          // Set the comment
          // 
          share->setComment(QString::fromUtf8(dirp->comment));
          
          //
          // Set share type
          // 
          share->setShareType(FileShare);
          
          //
          // Set the authentication data
          // 
          share->setLogin(m_item->url().userName(QUrl::FullyEncoded));
          share->setPassword(m_item->url().password(QUrl::FullyEncoded));
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(m_item->url().host());
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            share->setHostIpAddress(address);
            m_shares << share;
          }
          else
          {
            share.clear();
          } 
          
          break;
        }
        case SMBC_PRINTER_SHARE:
        {
          //
          // Create a share pointer
          // 
          SharePtr share = SharePtr(new Smb4KShare());
          
          //
          // Set the workgroup name
          // 
          share->setWorkgroupName(m_workgroup);
          
          //
          // Set the host name
          // 
          share->setHostName(m_item->url().host());
          
          //
          // Set the share name
          // 
          share->setShareName(QString::fromUtf8(dirp->name));
          
          // 
          // Set the comment
          // 
          share->setComment(QString::fromUtf8(dirp->comment));
          
          //
          // Set share type
          // 
          share->setShareType(PrinterShare);
          
          //
          // Set the authentication data
          // 
          share->setLogin(m_item->url().userName(QUrl::FullyEncoded));
          share->setPassword(m_item->url().password(QUrl::FullyEncoded));
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(m_item->url().host());
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            share->setHostIpAddress(address);
            m_shares << share;
          }
          else
          {
            share.clear();
          } 
          
          break;
        }
        case SMBC_IPC_SHARE:
        {
          //
          // Create a share pointer
          // 
          SharePtr share = SharePtr(new Smb4KShare());
          
          //
          // Set the workgroup name
          // 
          share->setWorkgroupName(m_workgroup);
          
          //
          // Set the host name
          // 
          share->setHostName(m_item->url().host());
          
          //
          // Set the share name
          // 
          share->setShareName(QString::fromUtf8(dirp->name));
          
          // 
          // Set the comment
          // 
          share->setComment(QString::fromUtf8(dirp->comment));
          
          //
          // Set share type
          // 
          share->setShareType(IpcShare);
          
          //
          // Set the authentication data
          // 
          share->setLogin(m_item->url().userName(QUrl::FullyEncoded));
          share->setPassword(m_item->url().password(QUrl::FullyEncoded));
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(m_item->url().host());
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            share->setHostIpAddress(address);
            m_shares << share;
          }
          else
          {
            share.clear();
          } 
          
          break;
        }
        default:
        {
          qDebug() << "Need to process network item " << dirp->name;
          break;
        }
      }
    }
    
    smbc_closedir(dirfd);
  }
  
  smbc_free_context(m_context, 0);
  
  emitResult();
}

