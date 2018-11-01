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
#include "smb4kglobal.h"

// System includes
#include <string.h>

// Samba includes
#include <libsmbclient.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QHostAddress>


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
: KJob(parent), m_type(Smb4KGlobal::UnknownNetworkItem)
{
}


Smb4KClientJob::~Smb4KClientJob()
{
}


void Smb4KClientJob::start()
{
  QTimer::singleShot(50, this, SLOT(slotStartJob()));
}


void Smb4KClientJob::setUrl(const QUrl& url)
{
  m_url = url;
}


void Smb4KClientJob::setType(Smb4KGlobal::NetworkItem type)
{
  m_type = type;
}


void Smb4KClientJob::get_auth_data_fn(const char* server, const char* share, char* workgroup, int maxLenWorkgroup, char* username, int maxLenUsername, char* password, int maxLenPassword)
{
  switch (m_type)
  {
    case Smb4KGlobal::Network:
    {
      // No authentication needed.
      break;
    }
    case Smb4KGlobal::Workgroup:
    {
      break;
    }
    case Smb4KGlobal::Host:
    {
      break;
    }
    case Smb4KGlobal::Share:
    {
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KClientJob::slotStartJob()
{
  // 
  // Allocate new context
  // 
  m_context = smbc_new_context();
  
  if (!m_context)
  {
    qWarning() << "Could not allocate new context";
    emitResult();
  }
  
  // 
  // Set mandatory options
  // 
  smbc_setDebug(m_context, 1);

  smbc_setFunctionAuthDataWithContext(m_context, get_auth_data_with_context_fn);
  smbc_setOptionUserData(m_context, this);
  smbc_setOptionBrowseMaxLmbCount(m_context, 0 /* all master browsers */);
  
  // FIXME: Use settings
  smbc_setOptionUseKerberos(m_context, 1);
  smbc_setOptionFallbackAfterKerberos(m_context, 1);
  smbc_setOptionDebugToStderr(m_context, 1);
  
  // 
  // Initialize context with the previously defined options
  // 
  if (!smbc_init_context(m_context))
  {
    smbc_free_context(m_context, 0);
    qWarning() << "Could not initialize context";
    emitResult();
  }
  
  smbc_set_context(m_context);
  
  // 
  // Read the given URL
  // 
  int dirfd;
  struct smbc_dirent *dirp = nullptr;
  
  dirfd = smbc_opendir(m_url.toString().toLocal8Bit());
  
  if (dirfd < 0)
  {
    qWarning() << "Failure";
    emitResult();
  }
  else
  {
    while ((dirp = smbc_readdir(dirfd)) != nullptr)
    {
      switch (dirp->smbc_type)
      {
        case SMBC_WORKGROUP:
        {
          // Create a workgroup pointer
          Smb4KWorkgroup *workgroup = new Smb4KWorkgroup();
          
          // Set the workgroup name
          QString workgroupName = QString::fromUtf8(dirp->name);
          workgroup->setWorkgroupName(workgroupName);
          
          // Set the master browser
          QString masterBrowserName = QString::fromUtf8(dirp->comment);
          workgroup->setMasterBrowserName(masterBrowserName);

          // Get the IP address of the master browser
          if (workgroup->masterBrowserName() == QHostInfo::localHostName().toUpper() ||
              workgroup->masterBrowserName() == Smb4KGlobal::globalSambaOptions()["netbios name"].toUpper() ||
              workgroup->masterBrowserName() == Smb4KSettings::netBIOSName().toUpper())
          {
            // This is the local machine. We need to get the right active network interface.
            // The code block here was inspired by 
            // https://stackoverflow.com/questions/48296737/correct-way-to-look-up-hostnames-in-qt-with-qhostinfo-or-qdnslookup
            QString ipAddress;
            QList<QHostAddress> ipAddresses = QNetworkInterface::allAddresses();
            
            for (const QHostAddress &addr : ipAddresses)
            {
              // FIXME
            }
          }
          else
          {
            QHostInfo hostInfo = QHostInfo::fromName(workgroup->masterBrowserName());
            
            // FIXME
            qDebug() << hostInfo.addresses();
          }
          
          m_workgroups << workgroup;
          
          break;
        }
        default:
        {
          break;
        }
      }
        
      qDebug() << "Name: " << dirp->name;
      qDebug() << "Comment: " << dirp->comment;
    }
    
    smbc_closedir(dirfd);
  }
  
  smbc_free_context(m_context, 0);
  
  qDebug() << "Size of workgroups list: " << m_workgroups.size();
  
  emitResult();
}





