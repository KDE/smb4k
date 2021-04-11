/***************************************************************************
    Smb4K's container class for information about a directory or file.
                             -------------------
    begin                : Sa Nov 10 2018
    copyright            : (C) 2018-2021 by Alexander Reinholdt
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

#ifndef SMB4KFILE_H
#define SMB4KFILE_H

// application specific includes
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QHostAddress>
#include <QScopedPointer>

class Smb4KFilePrivate;

class Q_DECL_EXPORT Smb4KFile : public Smb4KBasicNetworkItem
{
public:
    /**
     * Constructor
     *
     * @param url           The URL of the file or directory
     *
     * @param item          The type of the item (only Directory and File)
     */
    Smb4KFile(const QUrl &url, Smb4KGlobal::NetworkItem item);

    /**
     * Copy constructor
     *
     * @param file          The other Smb4KFile object
     */
    Smb4KFile(const Smb4KFile &file);

    /**
     * Destructor
     */
    ~Smb4KFile();

    /**
     * Sets the workgroup name to @p name.
     *
     * @param name          The workgroup name
     */
    void setWorkgroupName(const QString &name);

    /**
     * Returns the workgroup name.
     *
     * @returns the workgroup name.
     */
    QString workgroupName() const;

    /**
     * Returns the host's name.
     *
     * @returns the name of the host.
     */
    QString hostName() const;

    /**
     * Set the host's IP address to @p ip.
     *
     * @param ip            The IP address of the host
     */
    void setHostIpAddress(const QHostAddress &address);

    /**
     * Returns the host's IP address.
     *
     * @returns the IP address
     */
    QString hostIpAddress() const;

    /**
     * Returns TRUE if the host's IP address is set and FALSE otherwise.
     *
     * @returns TRUE if the host's IP address is set and FALSE otherwise.
     */
    bool hasHostIpAddress() const;

    /**
     * Returns the share's name.
     *
     * @returns the name of the share.
     */
    QString shareName() const;

    /**
     * Set the login name for the share where this file or directory is located to @p name.
     *
     * @param name          The login name
     */
    void setLogin(const QString &name);

    /**
     * Return the login name for the share where this file or directory is located.
     *
     * @returns the login name
     */
    QString login() const;

    /**
     * Set the password for the share where this file or directory is located to @p pass.
     *
     * @param pass          The password
     */
    void setPassword(const QString &pass);

    /**
     * Return the password for the share where this file or directory is located.
     *
     * @returns the password
     */
    QString password() const;

    /**
     * Returns TRUE if the network item is a directory and FALSE otherwise.
     *
     * @return TRUE if this item is a directory
     */
    bool isDirectory() const;

    /**
     * Returns the name of the file or directory.
     *
     * @returns the name of the file or directory.
     */
    QString name() const;

    /**
     * Returns TRUE if the file or directory is hidden and FALSE otherwise.
     *
     * @returns TRUE is the file or directory id hidden
     */
    bool isHidden() const;

private:
    const QScopedPointer<Smb4KFilePrivate> d;
};

#endif
