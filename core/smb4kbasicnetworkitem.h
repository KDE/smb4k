/*
    This class provides the basic network item for the core library of
    Smb4K.

    SPDX-FileCopyrightText: 2009-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KBASICNETWORKITEM_H
#define SMB4KBASICNETWORKITEM_H

// application specific includes
#include "smb4kglobalenums.h"

// Qt includes
#include <QIcon>
#include <QMetaType>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

// forward declarations
class Smb4KBasicNetworkItemPrivate;

/**
 * This is the basic class from which all other network item classes
 * are derived.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class Q_DECL_EXPORT Smb4KBasicNetworkItem
{
public:
    /**
     * The constructor
     */
    explicit Smb4KBasicNetworkItem(Smb4KGlobal::NetworkItem type = Smb4KGlobal::UnknownNetworkItem);

    /**
     * The copy constructor
     */
    Smb4KBasicNetworkItem(const Smb4KBasicNetworkItem &item);

    /**
     * The destructor
     */
    virtual ~Smb4KBasicNetworkItem();

    /**
     * This function sets the type of the basic network item.
     *
     * @param type          The type of the network item
     */
    void setType(Smb4KGlobal::NetworkItem type) const;

    /**
     * This function returns the type of the basic network
     * item.
     *
     * @returns the type.
     */
    Smb4KGlobal::NetworkItem type() const;

    /**
     * This function sets the icon of the network item.
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
     * Set the URL for this network item.
     *
     * @param url           The URL
     */
    void setUrl(const QUrl &url) const;

    /**
     * Return the URL for this network item.
     *
     * @returns the URL
     */
    QUrl url() const;

    /**
     * Set @p discovered to TRUE if this network item was discovered using
     * the DNS-SD service.
     *
     * @param discovered    Set this to TRUE if the network item was discovered
     *                      using the DNS-SD service
     */
    void setDnsDiscovered(bool discovered) const;

    /**
     * Return TRUE if the network item was discovered using the DNS-SD
     * service and FALSE otherwise.
     *
     * @return TRUE if discovered by DNS-SD.
     */
    bool dnsDiscovered() const;

    /**
     * Set the comment for this network item.
     *
     * @param comment       The comment
     */
    void setComment(const QString &comment) const;

    /**
     * Return the comment for this network item.
     *
     * @returns the comment
     */
    QString comment() const;

    /**
     * Returms true, if the URL carries user information
     */
    bool hasUserInfo() const;

    /**
     * Copy assignment operator
     */
    Smb4KBasicNetworkItem &operator=(const Smb4KBasicNetworkItem &other);

protected:
    /**
     * Expose a pointer to the private URL variable.
     */
    QUrl *pUrl;

    /**
     * Expose a pointer to the private icon variable.
     */
    QIcon *pIcon;

    /**
     * Expose a pointer to the private comment variable.
     */
    QString *pComment;

    /**
     * Export a pointer to the private type variable
     */
    Smb4KGlobal::NetworkItem *pType;

private:
    const QScopedPointer<Smb4KBasicNetworkItemPrivate> d;
};

Q_DECLARE_METATYPE(Smb4KBasicNetworkItem)

#endif
