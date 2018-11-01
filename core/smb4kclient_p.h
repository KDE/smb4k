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

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QUrl>

// KDE includes
#include <KCoreAddons/KJob>


class Smb4KClientJob : public KJob
{
  Q_OBJECT
  
  public:
    explicit Smb4KClientJob(QObject *parent = 0);
    ~Smb4KClientJob();
    void start();
    void setUrl(const QUrl &url);
    void setType(Smb4KGlobal::NetworkItem type);
    void get_auth_data_fn(const char *server, 
                          const char *share,
                          char *workgroup,
                          int maxLenWorkgroup,
                          char *username,
                          int maxLenUsername,
                          char *password,
                          int maxLenPassword);
    
  protected Q_SLOTS:
    void slotStartJob();
    
  private:
    SMBCCTX *m_context;
    QUrl m_url;
    Smb4KGlobal::NetworkItem m_type;
    QList<Smb4KWorkgroup *> m_workgroups;
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



