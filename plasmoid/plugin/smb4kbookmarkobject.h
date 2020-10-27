/***************************************************************************
    This class derives from QObject and encapsulates a bookmark item. It
    is for use with QtQuick.
                             -------------------
    begin                : Fr Mai 11 2013
    copyright            : (C) 2013-2020 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKOBJECT_H
#define SMB4KBOOKMARKOBJECT_H

// application specific includes
#include "core/smb4kbookmark.h"

// Qt includes
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QUrl>
#include <QIcon>

// forward declarations
class Smb4KBookmarkObjectPrivate;

/**
 * This class derives from QObject and makes the main functions of a 
 * bookmark available. Its main purpose is to be used with QtQuick 
 * and Plasma.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */

class Q_DECL_EXPORT Smb4KBookmarkObject : public QObject
{
  Q_OBJECT
  
  friend class Smb4KBookmarkObjectPrivate;
  
  Q_PROPERTY(QString workgroupName READ workgroupName WRITE setWorkgroupName NOTIFY changed)
  Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY changed)
  Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY changed)
  Q_PROPERTY(QString categoryName READ categoryName WRITE setCategoryName NOTIFY changed)
  Q_PROPERTY(bool isCategory READ isCategory WRITE setCategory NOTIFY changed)
  Q_PROPERTY(bool isMounted READ isMounted WRITE setMounted NOTIFY changed)
  Q_PROPERTY(QString hostName READ hostName CONSTANT)
  Q_PROPERTY(QString shareName READ shareName CONSTANT)
  Q_PROPERTY(QString login READ login WRITE setLogin NOTIFY changed)
  Q_PROPERTY(QString hostIP READ hostIP WRITE setHostIP NOTIFY changed)
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KBookmarkObject(Smb4KBookmark *bookmark, QObject *parent = 0);
    
    /**
     * Constructor for a bookmark group
     */
    explicit Smb4KBookmarkObject(const QString &categoryName, QObject *parent = 0);
    
    /**
     * Empty constructor
     */
    explicit Smb4KBookmarkObject(QObject *parent = 0);
    
    /**
     * Destructor
     */
    virtual ~Smb4KBookmarkObject();
    
    /**
     * This function returns the workgroup where the bookmarked
     * share is located.
     * 
     * @returns the workgroup name
     */
    QString workgroupName() const;
    
    /**
     * Set the workgroup of the bookmark object.
     * 
     * @param name      The workgroup name
     */
    void setWorkgroupName(const QString &name);
    
    /**
     * This function returns the name of the host where the bookmarked
     * share is located.
     * @returns the host name
     */
    QString hostName() const;
    
    /**
     * This function returns the name of the bookmarked share.
     * @returns the share name
     */    
    QString shareName() const;
    
    /**
     * This function returns the optional label of the bookmarked
     * share.
     *
     * @returns the label
     */
    QString label() const;
    
    /**
     * Set the label of the bookmarked share.
     * 
     * @param label     The label
     */
    void setLabel(const QString &label);
    
    /**
     * This function returns the URL of this bookmark.
     * 
     * @returns the URL
     */
    QUrl url() const;
    
    /**
     * Set the URL of this bookmark.
     * 
     * @param url       The URL
     */
    void setUrl(const QUrl &url);
    
    /**
     * This function returns the name of the group the bookmark is
     * in.
     * 
     * @returns the group
     */
    QString categoryName() const;
    
    /**
     * Set the name of the group this bookmark is in.
     * 
     * @param name      The group name
     */
    void setCategoryName(const QString &name);
    
    /**
     * This function returns TRUE if this object represents a bookmark
     * group.
     * 
     * @returns TRUE if this object is a bookmark group
     */
    bool isCategory() const;
    
    /**
     * For a bookmark category @p category has to be set to TRUE.
     * 
     * @param category  TRUE for a bookmark group and FALSE otherwise
     */
    void setCategory(bool category);
    
    /**
     * Returns TRUE if the share that is represented by this bookmark 
     * is mounted.
     * 
     * @returns TRUE if the bookmarked share is mounted
     */
    bool isMounted() const;
    
    /**
     * For a share that is mounted set this to TRUE.
     * 
     * @param mounted   Set to TRUE for a mounted share
     */
    void setMounted(bool mounted);
    
    /**
     * Returns the login for the bookmarked share.
     * 
     * @returns the login name
     */
    QString login() const;
    
    /**
     * Set the login for the bookmarked share.
     * 
     * @param name      The login name
     */
    void setLogin(const QString &name);
    
    /**
     * Returns the IP address of the host
     *
     * @returns the IP address of the host
     */
    QString hostIP() const;
    
    /**
     * Set the IP address of the host
     * 
     * @param ip        The IP address
     */
    void setHostIP(const QString &ip);
    
  Q_SIGNALS:
    /**
     * This signal is emitted if something changed.
     */
    void changed();
    
  private:
    const QScopedPointer<Smb4KBookmarkObjectPrivate> d;
};

#endif

