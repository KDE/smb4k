/***************************************************************************
    This is the bookmark container for Smb4K (next generation).
                             -------------------
    begin                : So Jun 8 2008
    copyright            : (C) 2008-2021 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARK_H
#define SMB4KBOOKMARK_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QIcon>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

// forward declarations
class Smb4KShare;
class Smb4KBookmarkPrivate;

/**
 * This is the container class for bookmarks in Smb4K. It is a complete
 * rewrite of the previous class and comes with several improvements.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KBookmark
{
    friend class Smb4KBookmarkPrivate;

public:
    /**
     * The constructor.
     *
     * @param share           The share for which the bookmark should
     *                        be created.
     *
     * @param label           The optional bookmark label.
     */
    explicit Smb4KBookmark(Smb4KShare *share, const QString &label = QString());

    /**
     * The copy constructor.
     *
     * @param bookmark        The bookmark that should be copied.
     */
    Smb4KBookmark(const Smb4KBookmark &bookmark);

    /**
     * The empty constructor.
     */
    Smb4KBookmark();

    /**
     * The destructor
     */
    ~Smb4KBookmark();

    /**
     * Set the workgroup name.
     *
     * @param workgroup       The workgroup where the share is located.
     */
    void setWorkgroupName(const QString &workgroup);

    /**
     * Returns the workgroup/domain name.
     *
     * @returns the workgroup/domain name.
     */
    QString workgroupName() const;

    /**
     * Set the host name.
     *
     * @param host            The host where the share is located.
     */
    void setHostName(const QString &host);

    /**
     * Returns the host name.
     *
     * @returns the host name.
     */
    QString hostName() const;

    /**
     * Set the share name.
     *
     * @param share           The share name
     */
    void setShareName(const QString &share);

    /**
     * Returns the share name.
     *
     * @returns the share name.
     */
    QString shareName() const;

    /**
     * Set the host's IP address.
     *
     * @param ip              The host's IP address
     */
    void setHostIpAddress(const QString &ip);

    /**
     * Returns the host's IP address.
     *
     * @returns the host's IP address.
     */
    QString hostIpAddress() const;

    /**
     * Set the share's type.
     *
     * @param type            The type of the share.
     */
    void setShareType(Smb4KGlobal::ShareType type);

    /**
     * Returns the share's type.
     *
     * @returns the type of the share.
     */
    Smb4KGlobal::ShareType shareType() const;

    /**
     * Set the (optional) bookmark label.
     *
     * @param label           The bookmark's label
     */
    void setLabel(const QString &label);

    /**
     * Returns the bookmark's label.
     *
     * @returns the bookmark's label.
     */
    QString label() const;

    /**
     * Sets the login that is used to mount this share.
     *
     * @param login           The login
     */
    void setLogin(const QString &login);

    /**
     * Returns the login that is used to mount this share.
     *
     * @returns the login.
     */
    QString login() const;

    /**
     * Sets the URL of the share after some checks are passed.
     *
     * @param url             The URL of the network item
     */
    void setUrl(const QUrl &url);

    /**
     * Sets the URL of the share.
     *
     * @param url             The URL of the network item
     */
    void setUrl(const QString &url);

    /**
     * Returns the URL of this bookmark.
     *
     * @returns the URL
     */
    QUrl url() const;

    /**
     * Set the category this bookmark belongs to.
     *
     * @param name            The category name
     */
    void setCategoryName(const QString &name);

    /**
     * Returns the name of the category this bookmark is in.
     *
     * @returns the category name
     */
    QString categoryName() const;

    /**
     * Sets the profile this bookmark belongs to. The profile is meant
     * to distinguish between several network environments, like home
     * and work, and is not an alternative to the categories.
     *
     * @param profile         The profile name
     */
    void setProfile(const QString &profile);

    /**
     * Returns the name of the profile this bookmark belongs to.
     *
     * @returns the profile name
     */
    QString profile() const;

    /**
     * This function sets the icon of the bookmark.
     *
     * @param icon          The icon
     */
    void setIcon(const QIcon &icon);

    /**
     * This function returns the icon of the network item. By default, it
     * is the null icon. You must set the appropriate icon either in
     * a class that inherits this one or from somewhere else.
     *
     * @returns the network item's icon.
     */
    QIcon icon() const;

    /**
     * Returns the display string. Prefer this over all other alternatives in your
     * GUI.
     * @returns the display string.
     */
    QString displayString() const;

private:
    const QScopedPointer<Smb4KBookmarkPrivate> d;
};

#endif
