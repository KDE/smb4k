/*
    This class derives from QObject and encapsulates a bookmark item. It
    is for use with QtQuick.

    SPDX-FileCopyrightText: 2013-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KBOOKMARKOBJECT_H
#define SMB4KBOOKMARKOBJECT_H

// application specific includes
#include "core/smb4kbookmark.h"

// Qt includes
#include <QIcon>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

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
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY changed)
    Q_PROPERTY(QString hostIpAddress READ hostIpAddress WRITE setHostIpAddress NOTIFY changed)

public:
    /**
     * Constructor
     */
    explicit Smb4KBookmarkObject(Smb4KBookmark *bookmark, QObject *parent = nullptr);

    /**
     * Constructor for a bookmark group
     */
    explicit Smb4KBookmarkObject(const QString &categoryName, QObject *parent = nullptr);

    /**
     * Empty constructor
     */
    explicit Smb4KBookmarkObject(QObject *parent = nullptr);

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
     * Returns the user name for the bookmarked share.
     *
     * @returns the user name
     */
    QString userName() const;

    /**
     * Set the user name for the bookmarked share.
     *
     * @param name      The user name
     */
    void setUserName(const QString &name);

    /**
     * Returns the IP address of the host
     *
     * @returns the IP address of the host
     */
    QString hostIpAddress() const;

    /**
     * Set the IP address of the host
     *
     * @param ip        The IP address
     */
    void setHostIpAddress(const QString &ip);

    /**
     * Get the icon of the network bookmark.
     *
     * @returns the icon
     */
    Q_INVOKABLE QIcon icon() const;

Q_SIGNALS:
    /**
     * This signal is emitted if something changed.
     */
    void changed();

private:
    const QScopedPointer<Smb4KBookmarkObjectPrivate> d;
};

#endif
