/***************************************************************************
    smb4knotification_p  -  These are the private helper classes of the 
    Smb4KNotification namespace.
                             -------------------
    begin                : So Jun 22 2014
    copyright            : (C) 2014 by Alexander Reinholdt
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

#ifndef SMB4KNOTIFICATION_P_H
#define SMB4KNOTIFICATION_P_H

// Qt includes
#include <QtCore/QObject>

// KDE includes
#include <kurl.h>


class Smb4KNotificationActionRunner : public QObject
{
  Q_OBJECT
  
  public:
    Smb4KNotificationActionRunner(QObject *parent = 0);
    ~Smb4KNotificationActionRunner();
    void setMountpoint(const KUrl &mountpoint);
    KUrl mountpoint() const;
    
  public Q_SLOTS:
    void slotOpenShare();
    
  private:
    KUrl m_mountpoint;
};


#endif
