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
#include <errno.h>
// #include <sys/types.h>
#include <sys/stat.h>
// #include <unistd.h>

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QWindow>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KConfigGui/KWindowConfig>
#include <KIconThemes/KIconLoader>
#include <KIOWidgets/KUrlComboBox>
#include <KIO/Global>

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
  
  //
  // Clear the list of files and directories
  //
  while (!m_files.isEmpty())
  {
    m_files.takeFirst().clear();
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
      // The host object
      // 
      HostPtr host = m_item.staticCast<Smb4KHost>();
      
      //
      // Prefer the username (login) and password provided by the host item.
      // 
      if (!m_item->url().userInfo().isEmpty())
      {
        // 
        // Copy the authentication data
        // 
        qstrncpy(username, m_item->url().userName().toUtf8().data(), maxLenUsername);
        qstrncpy(password, m_item->url().password().toUtf8().data(), maxLenPassword);
      }
      else
      {
        //
        // Get the authentication data
        //
        Smb4KWalletManager::self()->readAuthInfo(host);
        
        // 
        // Copy the authentication data
        // 
        if (host->url().userInfo().isEmpty())
        {
          host->setLogin("anonymous");
          host->setPassword("");
        }
        else
        {
          // Do nothing
        }
        
        qstrncpy(username, host->login().toUtf8().data(), maxLenUsername);
        qstrncpy(password, host->password().toUtf8().data(), maxLenPassword);
      }
      
      break;
    }
    case Share:
    {
      // FIXME
      break;
    }
    case Directory:
    {
      // FIXME
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


QList<FilePtr> Smb4KClientJob::files()
{
  return m_files;
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
    case Host:
    {
      workgroup = m_item.staticCast<Smb4KHost>()->workgroupName();
      break;
    }
    case Share:
    {
      workgroup = m_item.staticCast<Smb4KShare>()->workgroupName();
      break;
    }
    case Directory:
    case File:
    {
      workgroup = m_item.staticCast<Smb4KFile>()->workgroupName();
      break;
    }
    default:
    {
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
        setErrorText(i18n("Out of memory"));
        break;
      }
      default:
      {
        setError(UnknownError);
        setErrorText(i18n("Unknown error"));
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
  // Set the user for making the connection
  // 
  if (!m_item->url().userName().isEmpty())
  {
    smbc_setUser(m_context, m_item->url().userName().toUtf8().data());
  }
  else
  {
    // Do nothing
  }
  
  //
  // Set the port
  // 
  if (options)
  {
    if (options->useSmbPort())
    {
      smbc_setPort(m_context, options->smbPort());
    }
    else
    {
      smbc_setPort(m_context, 0 /* use the default */);
    }
  }
  else
  {
    if (Smb4KSettings::useRemoteSmbPort())
    {
      smbc_setPort(m_context, Smb4KSettings::remoteSmbPort());
    }
    else
    {
      smbc_setPort(m_context, 0 /* use the default */);
    }
  }

  //
  // Set the user data (this class)
  // 
  smbc_setOptionUserData(m_context, this);
  
  //
  // Set number of master browsers to be used
  // 
  // TODO: Implement a setting asking the user if she/he is on a big network (0 -> 3)
  // 
  smbc_setOptionBrowseMaxLmbCount(m_context, 0 /* all master browsers */);
  
  //
  // Set the encryption level
  // 
  if (Smb4KSettings::useEncryptionLevel())
  {
    switch (Smb4KSettings::encryptionLevel())
    {
      case Smb4KSettings::EnumEncryptionLevel::None:
      {
        smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_NONE);
        break;
      }
      case Smb4KSettings::EnumEncryptionLevel::Request:
      {
        smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_REQUEST);
        break;
      }
      case Smb4KSettings::EnumEncryptionLevel::Require:
      {
        smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_REQUIRE);
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
  // Set auth callback function
  // 
  smbc_setFunctionAuthDataWithContext(m_context, get_auth_data_with_context_fn);
  
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
        setErrorText(i18n("Out of memory"));
        break;
      }
      case EBADF:
      {
        setError(NullContextError);
        setErrorText(i18n("NULL context given"));
        break;
      }
      case ENOENT:
      {
        setError(SmbConfError);
        setErrorText(i18n("The smb.conf file would not load"));
        break;
      }
      default:
      {
        setError(UnknownError);
        setErrorText(i18n("Unknown error"));
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
        setErrorText(i18n("Out of memory"));
        break;
      }
      case EACCES:
      {
        setError(AccessDeniedError);
        setErrorText(i18n("Permission denied"));
        break;
      }
      case EINVAL:
      {
        setError(InvalidUrlError);
        setErrorText(i18n("An invalid URL was passed"));
        break;
      }
      case ENOENT:
      {
        setError(NonExistentUrlError);
        setErrorText(i18n("The URL does not exist"));
        break;
      }
      case ENOTDIR:
      {
        setError(NoDirectoryError);
        setErrorText(i18n("Name is not a directory"));
        break;
      }
      case EPERM:
      {
        setError(NotPermittedError);
        // Is the error message correct? 
        setErrorText(i18n("The workgroup could not be found"));
        break;
      }
      case ENODEV:
      {
        setError(NotFoundError);
        setErrorText(i18n("The workgroup or server could not be found"));
        break;
      }
      default:
      {
        setError(UnknownError);
        setErrorText(i18n("Unknown error"));
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
          share->setWorkgroupName(m_item.staticCast<Smb4KHost>()->workgroupName());
          
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
          share->setWorkgroupName(m_item.staticCast<Smb4KHost>()->workgroupName());
          
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
          share->setWorkgroupName(m_item.staticCast<Smb4KHost>()->workgroupName());
          
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
        case SMBC_DIR:
        {
          //
          // Do not process '.' and '..' directories
          // 
          QString name = QString::fromUtf8(dirp->name);
          
          if (name != "." && name != "..")
          {
            //
            // Create the URL for the discovered item
            // 
            QUrl u = m_item->url();
            u.setPath(m_item->url().path()+QDir::separator()+QString::fromUtf8(dirp->name));
            
            //
            // We do not stat directories. Directly create the directory object
            // 
            FilePtr dir = FilePtr(new Smb4KFile(u, Directory));
            
            //
            // Set the workgroup name
            //
            dir->setWorkgroupName(m_item.staticCast<Smb4KShare>()->workgroupName());
            
            //
            // Set the authentication data
            // 
            dir->setLogin(m_item->url().userName(QUrl::FullyEncoded));
            dir->setPassword(m_item->url().password(QUrl::FullyEncoded));
            
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
              dir->setHostIpAddress(address);
              m_files << dir;
            }
            else
            {
              dir.clear();
            } 
          }
          else
          {
            // Do nothing
          }
          
          break;
        }
        case SMBC_FILE:
        {
          //
          // Create the URL for the discovered item
          // 
          QUrl u = m_item->url();
          u.setPath(m_item->url().path()+QDir::separator()+QString::fromUtf8(dirp->name));
            
          //
          // Create the directory object
          // 
          FilePtr dir = FilePtr(new Smb4KFile(u, File));
            
          //
          // Set the workgroup name
          //
          dir->setWorkgroupName(m_item.staticCast<Smb4KShare>()->workgroupName());
          
          //
          // Stat the file
          //
            
          //
          // Set the authentication data
          // 
          dir->setLogin(m_item->url().userName(QUrl::FullyEncoded));
          dir->setPassword(m_item->url().password(QUrl::FullyEncoded));
            
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
            dir->setHostIpAddress(address);
            m_files << dir;
          }
          else
          {
            dir.clear();
          } 
            
          break;
        }
        case SMBC_LINK:
        {
          qDebug() << dirp->name;
          qDebug() << dirp->comment;
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



Smb4KPreviewDialog::Smb4KPreviewDialog(const SharePtr& share, QWidget* parent) 
: QDialog(parent), m_share(share)
{
  // 
  // Dialog title
  // 
  setWindowTitle(i18n("Preview of %1", share->displayString()));
  
  //
  // Attributes
  // 
  setAttribute(Qt::WA_DeleteOnClose, true);
  
  //
  // Layout
  // 
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);
  
  //
  // The list widget
  // 
  QListWidget *listWidget = new QListWidget(this);
  listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(listWidget, SIGNAL(itemActivated(QListWidgetItem *)), SLOT(slotItemActivated(QListWidgetItem *)));
  
  layout->addWidget(listWidget, 0);
  
  //
  // Toolbar
  // Use QToolBar here with the settings suggested by the note provided in the 'Detailed Description'
  // section of KToolBar (https://api.kde.org/frameworks/kxmlgui/html/classKToolBar.html)
  // 
  QToolBar *toolBar = new QToolBar(this);
  toolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
  toolBar->setProperty("otherToolbar", true);
  
  // FIXME: Make this a double action
  QAction *reloadAction = toolBar->addAction(KDE::icon("view-refresh"), i18n("Reload"), this, SLOT(slotReloadActionTriggered()));
  reloadAction->setObjectName("reload_action");
  
  QAction *upAction =toolBar->addAction(KDE::icon("go-up"), i18n("Up"), this, SLOT(slotUpActionTriggered()));
  upAction->setObjectName("up_action");
  upAction->setEnabled(false);
  
  toolBar->addSeparator();
  
  KUrlComboBox *urlCombo = new KUrlComboBox(KUrlComboBox::Directories, toolBar);
  urlCombo->setEditable(false);
  toolBar->addWidget(urlCombo);
  connect(urlCombo, SIGNAL(urlActivated(QUrl)), this, SLOT(slotUrlActivated(QUrl)));
  
  layout->addWidget(toolBar, 0);
  
  //
  // Dialog button box
  // 
  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Horizontal);
  QPushButton *closeButton = buttonBox->addButton(QDialogButtonBox::Close);
  connect(closeButton, SIGNAL(clicked()), this, SLOT(slotClosingDialog()));
  
  layout->addWidget(buttonBox, 0);
  
  //
  // Set the dialog size
  // 
  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
  resize(windowHandle()->size()); // workaround for QTBUG-40584

  //
  // Start the preview
  // 
  m_currentItem = m_share;
  QTimer::singleShot(0, this, SLOT(slotInitializePreview()));
}


Smb4KPreviewDialog::~Smb4KPreviewDialog()
{
  //
  // Clear the share
  //
  m_share.clear();
  
  //
  // Clear the current item
  //
  m_currentItem.clear();
  
  //
  // Clear the listing
  // 
  while (!m_listing.isEmpty())
  {
    m_listing.takeFirst().clear();
  }
}


SharePtr Smb4KPreviewDialog::share() const
{
  return m_share;
}


void Smb4KPreviewDialog::slotClosingDialog()
{
  //
  // Save the dialog size
  // 
  KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  
  //
  // Emit the aboutToClose() signal
  // 
  emit aboutToClose(this);
  
  // 
  // Close the dialog
  // 
  accept();
}


void Smb4KPreviewDialog::slotReloadActionTriggered()
{
  emit requestPreview(m_currentItem);
}


void Smb4KPreviewDialog::slotUpActionTriggered()
{
  //
  // Get the new URL
  // 
  QUrl u = KIO::upUrl(m_currentItem->url());
  
  //
  // Create a new network item object, if necessary and set the new current
  // item. Also, disable the "Up" action, if necessary.
  // 
  if (m_share->url().matches(u, QUrl::StripTrailingSlash))
  {
    m_currentItem = m_share;
    findChild<QAction *>("up_action")->setEnabled(false);
  }
  else if (m_share->url().path().length() < u.path().length())
  {
    FilePtr file = FilePtr(new Smb4KFile(u, Directory));
    m_currentItem = file;
  }
  else
  {
    return;
  }
  
  //
  // Emit the requestPreview() signal
  // 
  emit requestPreview(m_currentItem);
}


void Smb4KPreviewDialog::slotUrlActivated(const QUrl &url)
{
  //
  // Get the full authentication information. This is needed, since the combo
  // box only returns sanitized URLs, i.e. without password, etc.
  // 
  QUrl u = url;
  u.setUserName(m_share->login());
  u.setPassword(m_share->password());

  //
  // Create a network item
  // 
  NetworkItemPtr item;
  
  if (m_share->url().matches(u, QUrl::StripTrailingSlash))
  {
    item = m_share;
  }
  else
  {
    item = FilePtr(new Smb4KFile(u, Directory));
  }
  
  //
  // Set the current item
  // 
  m_currentItem = item;
  
  //
  // Emit the requestPreview() signal
  // 
  emit requestPreview(m_currentItem);
}


void Smb4KPreviewDialog::slotItemActivated(QListWidgetItem *item)
{
  //
  // Only process the item if it represents a directory
  // 
  if (item && item->type() == Directory)
  {
    //
    // Find the file item, make it the current one and emit the requestPreview() 
    // signal.
    // 
    for (const FilePtr &f : m_listing)
    {
      if (item->data(Qt::UserRole).toUrl().matches(f->url(), QUrl::None))
      {
        m_currentItem = f;
        emit requestPreview(m_currentItem);
        break;
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


void Smb4KPreviewDialog::slotInitializePreview()
{
  emit requestPreview(m_currentItem);
}


void Smb4KPreviewDialog::slotPreviewResults(const QList<FilePtr> &list)
{
  //
  // Clear the internal listing
  // 
  while (!m_listing.isEmpty())
  {
    m_listing.takeFirst().clear();
  }  
  
  //
  // Copy the list into the private variable
  //
  m_listing = list;
  
  //
  // Get the list widget
  // 
  QListWidget *listWidget = findChild<QListWidget *>();
  
  //
  // Clear the list widget
  //
  listWidget->clear();
  
  //
  // Insert the new listing
  // 
  if (listWidget)
  {
    for (const FilePtr &f : list)
    {
      QListWidgetItem *item = new QListWidgetItem(f->icon(), f->name(), listWidget, f->isDirectory() ? Directory : File);
      item->setData(Qt::UserRole, f->url());
    }
  }
  else
  {
    // Do nothing
  }
  
  //
  // Sort the list widget
  // 
  listWidget->sortItems();
  
  //
  // Add the URL to the combo box and show it. Omit duplicates.
  // 
  KUrlComboBox *urlCombo = findChild<KUrlComboBox *>();
  QStringList urls = urlCombo->urls();
  urls << m_currentItem->url().toString();
  urlCombo->setUrls(urls);
  urlCombo->setUrl(m_currentItem->url());
  
  //
  // Enable / disable the "Up" action
  //
  findChild<QAction *>("up_action")->setEnabled(!m_share->url().matches(m_currentItem->url(), QUrl::StripTrailingSlash));
}

