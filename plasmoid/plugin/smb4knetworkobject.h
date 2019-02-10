/***************************************************************************
    This class derives from QObject and encapsulates the network items. 
    It is for use with QtQuick.
                             -------------------
    begin                : Fr Mär 02 2012
    copyright            : (C) 2012-2019 by Alexander Reinholdt
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
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QUrl>
#include <QIcon>

// forward declaration
class Smb4KNetworkObjectPrivate;

using namespace Smb4KGlobal;

/**
 * This class derives from QObject and makes the main functions of the 
 * network items Smb4KWorkgroup, Smb4KHost, and Smb4KShare available. Its 
 * main purpose is to be used with QtQuick and Plasma.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */


class Q_DECL_EXPORT Smb4KNetworkObject : public QObject
{
  Q_OBJECT
  Q_PROPERTY(NetworkItem type READ type WRITE setType NOTIFY changed)
  Q_PROPERTY(NetworkItem parentType READ parentType CONSTANT)
  Q_PROPERTY(QString workgroupName READ workgroupName WRITE setWorkgroupName NOTIFY changed)
  Q_PROPERTY(QString hostName READ hostName WRITE setHostName NOTIFY changed)
  Q_PROPERTY(QString shareName READ shareName WRITE setShareName NOTIFY changed)
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY changed)
  Q_PROPERTY(QUrl url READ url WRITE setURL NOTIFY changed)
  Q_PROPERTY(QUrl parentURL READ parentURL CONSTANT)
  Q_PROPERTY(bool isMounted READ isMounted WRITE setMounted NOTIFY changed)
  Q_PROPERTY(bool isPrinter READ isPrinter WRITE setPrinter NOTIFY changed)
  Q_PROPERTY(QUrl mountpoint READ mountpoint WRITE setMountpoint NOTIFY changed)
  Q_PROPERTY(bool isMasterBrowser READ isMasterBrowser WRITE setMasterBrowser NOTIFY changed)
  Q_PROPERTY(bool isInaccessible READ isInaccessible WRITE setInaccessible NOTIFY changed)
  
  Q_ENUM(NetworkItem)
  
  friend class Smb4KNetworkObjectPrivate;
  
  public:
    /**
     * Constructor for a workgroup.
     */
    explicit Smb4KNetworkObject(Smb4KWorkgroup *workgroup, QObject *parent = 0);

    /**
     * Constructor for a host.
     */
    explicit Smb4KNetworkObject(Smb4KHost *host, QObject *parent = 0);

    /**
     * Constructor for a share.
     */
    explicit Smb4KNetworkObject(Smb4KShare *share, QObject *parent = 0);

    /**
     * Empty constructor
     */
    explicit Smb4KNetworkObject(QObject *parent = 0);

    /**
     * Destructor
     */
    ~Smb4KNetworkObject();

    /**
     * This function returns the type.
     * 
     * @returns the type
     */
    NetworkItem type() const;
    
    /**
     * This function returns the type of the parent of this item. In case of
     * the type() function returning Unknown, this function will do the same,
     * otherwise the type of the level above is returned.
     * 
     * @returns the parent's type
     */
    NetworkItem parentType() const;
    
    /**
     * Set the type of the network item.
     * 
     * @param type        The type
     */
    void setType(NetworkItem type);

    /**
     * Returns the workgroup name.
     *
     * @returns the workgroup name
     */
    QString workgroupName() const;
    
    /**
     * Set the workgroup name for this network item.
     * 
     * @param name        The workgroup name
     */
    void setWorkgroupName(const QString &name);

    /**
     * In case of a host or share, this function returns the name
     * of the host. In case of a workgroup the return value is an
     * empty string.
     *
     * @returns the host name or an empty string
     */
    QString hostName() const;
    
    /**
     * Set the host name for this network item.
     * 
     * @param name        The host name
     */
    void setHostName(const QString &name);
    
    /**
     * Returns TRUE if this network object represents a master browser
     * and FALSE otherwise.
     * @returns TRUE if the network object is a master browser
     */
    bool isMasterBrowser() const;
    
    /**
     * Set this network object to be a master browser. This function
     * only does something, if type() returns Host.
     * 
     * @param master      Set to TRUE, if the network item is a master 
     *                    browser
     */
    void setMasterBrowser(bool master);

    /**
     * In case of a share, this function returns the name of the
     * share. In case of a workgroup or host the return value is an
     * empty string.
     *
     * @returns the share name or an empty string
     */
    QString shareName() const;
    
    /**
     * Set the share name for this network item.
     * 
     * @param name        The share name
     */
    void setShareName(const QString &name);
    
    /**
     * This is a convenience function that returns the name of the 
     * item depending of its type.
     * 
     * @returns the name depending of the type
     */
    QString name() const;

    /**
     * This function returns the comment of a network item or an
     * empty string if there is no comment defined.
     * 
     * @returns the comment
     */
    QString comment() const;
    
    /**
     * Set the comment for this network item.
     * 
     * @param comment     The comment
     */
    void setComment(const QString &comment);
    
    /**
     * This function returns the UNC/URL of this item.
     * 
     * Please note that a workgroup will have a UNC like smb://WORKGROUP,
     * so to discriminate it from a host, you need to check the type()
     * function as well.
     * 
     * @returns the item's URL
     */
    QUrl url() const;
    
    /**
     * Return the URL of the parent item.
     * 
     * @returns the item's parent URL
     */
    QUrl parentURL() const;
    
    /**
     * Set the URL of this network item.
     * 
     * @param url         The URL
     */
    void setURL(const QUrl &url);
    
    /**
     * This function returns TRUE if the network item is a share and it is
     * mounted. Otherwise it returns FALSE.
     * 
     * @returns TRUE if the network item is mounted.
     */
    bool isMounted() const;
    
    /**
     * Mark this network item as mounted. This is only reasonable with a share.
     * 
     * @param mounted     Should be TRUE if the network item is mounted
     */
    void setMounted(bool mounted);
    
    /**
     * Updates the network item.
     * 
     * @param networkItem   The network item that needs to be updated
     */
    void update(Smb4KBasicNetworkItem *networkItem);
    
    /**
     * This function returns TRUE if the network item is a printer share.
     * Otherwise it returns FALSE,
     * 
     * @returns TRUE if the network item is a printer.
     */
    bool isPrinter() const;
    
    /**
     * Mark this network item as printer. This is only reasonable with a share.
     * 
     * @param printer     Should be TRUE if the network item is a printer
     */
    void setPrinter(bool printer);
    
    /**
     * This function returns the mountpoint of a mounted share or an empty 
     * string if the network item is not a share or the share is not mounted.
     * 
     * @returns the mount point of a share.
     */
    QUrl mountpoint() const;
    
    /**
     * Set the mountpoint for this network item. This is only reasonable with a
     * share.
     * 
     * @param mountpoint  The mountpoint
     */
    void setMountpoint(const QUrl &url);
    
    /**
     * Returns TRUE if the network item is a share that is mounted and became
     * inaccessible. Otherwise this function returns FALSE.
     * @returns TRUE is the mounted share is inaccessible
     */
    bool isInaccessible() const;
    
    /**
     * Mark this network item as inaccessible. This is only reasonable with a 
     * mounted share.
     * @param inaccessible  Should be TRUE if the mounted share is inaccessible
     */
    void setInaccessible(bool inaccessible);
    
  Q_SIGNALS:
    /**
     * This signal is emitted when the network item changed.
     */
    void changed();
    
  private:
    const QScopedPointer<Smb4KNetworkObjectPrivate> d;
};

#endif
