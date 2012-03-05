/***************************************************************************
    smb4knetworkitemcontainer  -  This class derives from QObject and
    encapsulates the network items. It is for use with QtQuick.
                             -------------------
    begin                : Fr Mär 02 2012
    copyright            : (C) 2012 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKOBJECT_H
#define SMB4KNETWORKOBJECT_H

// Qt includes
#include <QObject>

// KDE includes
#include <kdemacros.h>

// application specific includes
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"

/**
 * This class derives from QObject and makes the main functions of the 
 * network items Smb4KWorkgroup, Smb4KHost, and Smb4KShare available. Its 
 * main purpose is to be used with QtQuick and Plasma.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */


class KDE_EXPORT Smb4KNetworkObject : public QObject
{
  Q_OBJECT

  Q_PROPERTY( int type READ type CONSTANT )
  Q_PROPERTY( QString workgroupName READ workgroupName CONSTANT )
  Q_PROPERTY( QString hostName READ hostName CONSTANT )
  Q_PROPERTY( QString shareName READ shareName CONSTANT )
  Q_PROPERTY( QIcon icon READ icon CONSTANT )

  public:
    /**
     * Type enumeration
     */
    enum Type { Workgroup,
                Host,
                Share,
                Unknown };
                
    /**
     * Constructor for a workgroup.
     */
    Smb4KNetworkObject( Smb4KWorkgroup *workgroup, QObject *parent = 0 );

    /**
     * Constructor for a host.
     */
    Smb4KNetworkObject( Smb4KHost *host, QObject *parent = 0 );

    /**
     * Constructor for a share.
     */
    Smb4KNetworkObject( Smb4KShare *share, QObject *parent = 0 );

    /**
     * Empty constructor
     */
    Smb4KNetworkObject( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KNetworkObject();

    /**
     * This function returns the type
     */
    int type() const { return m_type; }

    /**
     * Returns the workgroup name
     *
     * @returns the workgroup name
     */
    const QString workgroupName();

    /**
     * In case of a host or share, this function returns the name
     * of the host. In case of a workgroup the return value is an
     * empty string.
     *
     * @returns the host name or an empty string
     */
    const QString hostName();

    /**
     * In case of a share, this function returns the name of the
     * share. In case of a workgroup or host the return value is an
     * empty string.
     *
     * @returns the share name or an empty string
     */
    const QString shareName();

    /**
     * This function returns the icon of the network item.
     *
     * @returns the icon
     */
    const QIcon icon();
    
  private:
    Smb4KWorkgroup m_workgroup;
    Smb4KHost m_host;
    Smb4KShare m_share;
    int m_type;
};

#endif
