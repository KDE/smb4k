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

#ifndef SMB4KCLIENT_P_H
#define SMB4KCLIENT_P_H

// application specific includes
#include "smb4kclient.h"
#include "smb4kglobal.h"
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QUrl>
#include <QHostAddress>

// KDE includes
#include <KCoreAddons/KJob>


class Smb4KClientJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KClientJob(QObject *parent = 0);
    
    /**
     * Destructor
     */
    ~Smb4KClientJob();
    
    /**
     * Error enumeration
     * 
     * @param OutOfMemoryError  Out of memory
     * @param NullContextError  The passed SMBCCTX context was a null pointer
     * @param SmbConfError      The smb.conf file could not be read
     * @param AccessDeniedError Permission denied
     * @param InvalidUrlError   The URL that was passed is invalid
     * @param UrlExistError     The URL does not exist
     * @param NoDirectoryError  The name is not a directory
     * @param NetPermittedError The operation is not permitted
     * @param NotFoundError     The network item was not found
     */
    enum {
      OutOfMemoryError = UserDefinedError,
      NullContextError,
      SmbConfError,
      AccessDeniedError,
      InvalidUrlError,
      UrlExistError,
      NoDirectoryError,
      NotPermittedError,
      NotFoundError,
      UnknownError
    };
    
    /**
     * Starts the job.
     */
    void start();
    
    /**
     * Set the basic network item
     */
    void setNetworkItem(NetworkItemPtr item);
    
    /**
     * Return the basic network item
     */
    NetworkItemPtr networkItem() const;
    
    /**
     * The authentication function for libsmbclient
     */
    void get_auth_data_fn(const char *server, 
                          const char *share,
                          char *workgroup,
                          int maxLenWorkgroup,
                          char *username,
                          int maxLenUsername,
                          char *password,
                          int maxLenPassword);
    /**
     * The list of workgroups that was discovered.
     */
    QList<WorkgroupPtr> workgroups();
    
    /**
     * The list of hosts that was discovered.
     */
    QList<HostPtr> hosts();
    
    /**
     * The list shares that was discovered.
     */
    QList<SharePtr> shares();
    
    /**
     * The workgroup
     */
    QString workgroup();
    
  protected Q_SLOTS:
    void slotStartJob();
    
  private:
    QHostAddress lookupIpAddress(const QString &name);
    SMBCCTX *m_context;
    NetworkItemPtr m_item;
    QList<WorkgroupPtr> m_workgroups;
    QList<HostPtr> m_hosts;
    QList<SharePtr> m_shares;
    QString m_workgroup;
};


class Smb4KClientPrivate
{
};


class Smb4KClientStatic
{
  public:
    Smb4KClient instance;
};

#endif



