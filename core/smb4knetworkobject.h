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

// application specific includes
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>
#include <QtGui/QIcon>

// KDE includes
#include <kdemacros.h>
#include <kurl.h>

// forward declaration
class Smb4KNetworkObjectPrivate;

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

  friend class Smb4KNetworkObjectPrivate;
  
  Q_ENUMS( Type )
  Q_PROPERTY( Type type READ type CONSTANT )
  Q_PROPERTY( QString workgroupName READ workgroupName CONSTANT )
  Q_PROPERTY( QString hostName READ hostName CONSTANT )
  Q_PROPERTY( QString shareName READ shareName CONSTANT )
  Q_PROPERTY( QIcon icon READ icon CONSTANT )
  Q_PROPERTY( QString comment READ comment CONSTANT )
  Q_PROPERTY( QUrl url READ url CONSTANT )
  Q_PROPERTY( bool isMounted READ isMounted CONSTANT )
  Q_PROPERTY( bool isPrinter READ isPrinter CONSTANT )
  Q_PROPERTY( QUrl mountpoint READ mountpoint CONSTANT )

  public:
    /**
     * Type enumeration
     */
    enum Type { Unknown = 0,
                Workgroup = 1,
                Host = 2,
                Share = 3 };
                
    /**
     * Constructor for a workgroup.
     */
    explicit Smb4KNetworkObject( Smb4KWorkgroup *workgroup, QObject *parent = 0 );

    /**
     * Constructor for a host.
     */
    explicit Smb4KNetworkObject( Smb4KHost *host, QObject *parent = 0 );

    /**
     * Constructor for a share.
     */
    explicit Smb4KNetworkObject( Smb4KShare *share, QObject *parent = 0 );

    /**
     * Empty constructor
     */
    explicit Smb4KNetworkObject( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KNetworkObject();

    /**
     * This function returns the type
     */
    Type type() const;

    /**
     * Returns the workgroup name
     *
     * @returns the workgroup name
     */
    QString workgroupName() const;

    /**
     * In case of a host or share, this function returns the name
     * of the host. In case of a workgroup the return value is an
     * empty string.
     *
     * @returns the host name or an empty string
     */
    QString hostName() const;

    /**
     * In case of a share, this function returns the name of the
     * share. In case of a workgroup or host the return value is an
     * empty string.
     *
     * @returns the share name or an empty string
     */
    QString shareName() const;

    /**
     * This function returns the icon of the network item.
     *
     * @returns the icon
     */
    QIcon icon() const;
    
    /**
     * This function returns the comment of a network item or an
     * empty string if there is no comment defined.
     * 
     * @returns the comment
     */
    QString comment() const;
    
    /**
     * This function returns the UNC/URL of this item.
     * 
     * Please note that a workgroup will have a UNC like smb://WORKGROUP,
     * so to discriminate it from a host, you need to check the type()
     * function as well.
     */
    KUrl url() const;
    
    /**
     * This function returns TRUE if the network item is a share and it is
     * mounted. Otherwise it returns FALSE.
     * 
     * @returns TRUE if the network item is mounted.
     */
    bool isMounted() const;
    
    /**
     * Updates the network item.
     * 
     * @param networkItem   The network item that needs to be updated
     */
    void update( Smb4KBasicNetworkItem *networkItem );
    
    /**
     * This function returns TRUE if the network item is a printer share.
     * Otherwise it returns FALSE,
     * 
     * @returns TRUE if the network item is a printer.
     */
    bool isPrinter() const;
    
    /**
     * This function returns the mountpoint of a mounted share or an empty 
     * string if the network item is not a share or the share is not mounted.
     * 
     * @returns the mount point of a share.
     */
    KUrl mountpoint() const;
    
  private:
    const QScopedPointer<Smb4KNetworkObjectPrivate> d;
};

#endif
