/*
    This class provides the basic network item for the core library of
    Smb4K.
    -------------------
    begin                : Do Apr 2 2009
    SPDX-FileCopyrightText: 2009-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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

#ifndef SMB4KBASICNETWORKITEM_H
#define SMB4KBASICNETWORKITEM_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QIcon>
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
    ~Smb4KBasicNetworkItem();

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
     * Set the URL for this network item.
     *
     * @param url           The URL
     */
    void setUrl(const QUrl &url);

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
    void setDnsDiscovered(bool discovered);

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
    void setComment(const QString &comment);

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

private:
    const QScopedPointer<Smb4KBasicNetworkItemPrivate> d;
};

#endif
