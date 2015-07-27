/***************************************************************************
    smb4knotification_p  -  These are the private helper classes of the 
    Smb4KNotification namespace.
                             -------------------
    begin                : So Jun 22 2014
    copyright            : (C) 2014-2015 by Alexander Reinholdt
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
#include "smb4knotification_p.h"

// KDE includes
#include <KIOWidgets/KRun>


Smb4KNotificationActionRunner::Smb4KNotificationActionRunner(QObject *parent)
: QObject(parent)
{
}


Smb4KNotificationActionRunner::~Smb4KNotificationActionRunner()
{
}


void Smb4KNotificationActionRunner::setMountpoint(const QUrl& mountpoint)
{
  m_mountpoint = mountpoint;
}


QUrl Smb4KNotificationActionRunner::mountpoint() const
{
  return m_mountpoint;
}


void Smb4KNotificationActionRunner::slotOpenShare()
{
  KRun::runUrl(m_mountpoint, "inode/directory", 0);
}

