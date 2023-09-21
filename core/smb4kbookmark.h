/*
    This is the bookmark container for Smb4K (next generation).

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KBOOKMARK_H
#define SMB4KBOOKMARK_H

// application specific includes
#include "smb4kglobalenums.h"

// Qt includes
#include <QIcon>
#include <QScopedPointer>
#include <QString>
#include <QUrl>
#include <QMetaType>

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
     * Set the share item.
     *
     * @param share           The share
     */
    void setShare(Smb4KShare *share) const;

    /**
     * Set the workgroup name.
     *
     * @param workgroup       The workgroup where the share is located.
     */
    void setWorkgroupName(const QString &workgroup) const;

    /**
     * Returns the workgroup/domain name.
     *
     * @returns the workgroup/domain name.
     */
    QString workgroupName() const;

    /**
     * Returns the host name.
     *
     * @returns the host name.
     */
    QString hostName() const;

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
    void setHostIpAddress(const QString &ip) const;

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
    void setShareType(Smb4KGlobal::ShareType type) const;

    /**
     * Returns the share's type.
     *
     * @returns the type of the share.
     */
    Smb4KGlobal::ShareType shareType() const;

    /**
     * Set the (optional) bookmark label.
     *
     * @param label         The bookmark's label
     */
    void setLabel(const QString &label) const;

    /**
     * Returns the bookmark's label.
     *
     * @returns the bookmark's label.
     */
    QString label() const;

    /**
     * Sets the user name that is used to mount this share.
     *
     * @param name          The user name
     */
    void setUserName(const QString &name) const;

    /**
     * Returns the user name that is used to mount this share.
     *
     * @returns the user name.
     */
    QString userName() const;

    /**
     * Sets the URL of the share after some checks are passed.
     *
     * @param url             The URL of the network item
     */
    void setUrl(const QUrl &url) const;

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
    void setCategoryName(const QString &name) const;

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
    void setProfile(const QString &profile) const;

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
    void setIcon(const QIcon &icon) const;

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

    /**
     * Copy assignment operator
     */
    Smb4KBookmark &operator=(const Smb4KBookmark &other);

private:
    QScopedPointer<Smb4KBookmarkPrivate> d;
};

Q_DECLARE_METATYPE(Smb4KBookmark)

#endif
