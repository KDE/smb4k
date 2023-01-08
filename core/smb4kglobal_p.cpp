/***************************************************************************
    These are the private helper classes of the Smb4KGlobal namespace.
                             -------------------
    begin                : Di Jul 24 2007
    copyright            : (C) 2007-2020 by Alexander Reinholdt
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

// application specific includes
#include "smb4kglobal_p.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <QCoreApplication>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QHostInfo>
#include <QDirIterator>


Smb4KGlobalPrivate::Smb4KGlobalPrivate()
{
  onlyForeignShares = false;
  coreInitialized = false;
  
#ifdef Q_OS_LINUX
  allowedMountArguments << "dynperm";
  allowedMountArguments << "rwpidforward";
  allowedMountArguments << "hard";
  allowedMountArguments << "soft";
  allowedMountArguments << "noacl";
  allowedMountArguments << "cifsacl";
  allowedMountArguments << "backupuid";
  allowedMountArguments << "backupgid";
  allowedMountArguments << "ignorecase";
  allowedMountArguments << "nocase";
  allowedMountArguments << "nobrl";
  allowedMountArguments << "sfu";
  allowedMountArguments << "nounix";
  allowedMountArguments << "nouser_xattr";
  allowedMountArguments << "fsc";
  allowedMountArguments << "multiuser";
  allowedMountArguments << "actimeo";
  allowedMountArguments << "noposixpaths";
  allowedMountArguments << "posixpaths";
#endif
  
  //
  // Create and init the SMB context and read the NetBIOS and
  // workgroup name of this machine.
  //
  SMBCCTX *smbContext= smbc_new_context();
  
  if (smbContext)
  {
    smbContext = smbc_init_context(smbContext);
      
    if (!smbContext)
    {
      smbc_free_context(smbContext, 1);
    }
    else
    {
      machineNetbiosName = QString::fromUtf8(smbc_getNetbiosName(smbContext)).toUpper();
      machineWorkgroupName = QString::fromUtf8(smbc_getWorkgroup(smbContext)).toUpper();
    }
  }
  
  //
  // Free the SMB context
  // 
  smbc_free_context(smbContext, 1);
  
  //
  // Connections
  // 
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}


Smb4KGlobalPrivate::~Smb4KGlobalPrivate()
{
  // 
  // Clear the workgroup list
  // 
  while (!workgroupsList.isEmpty())
  {
    workgroupsList.takeFirst().clear();
  }

  // 
  // Clear the host list
  // 
  while (!hostsList.isEmpty())
  {
    hostsList.takeFirst().clear();
  }

  // 
  // Clear the list of mounted shares
  // 
  while (!mountedSharesList.isEmpty())
  {
    mountedSharesList.takeFirst().clear();
  }

  // 
  // Clear the list of shares
  // 
  while (!sharesList.isEmpty())
  {
    sharesList.takeFirst().clear();
  }
}


void Smb4KGlobalPrivate::slotAboutToQuit()
{
  Smb4KSettings::self()->save();
}

