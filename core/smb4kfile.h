/*
    Smb4K's container class for information about a directory or file.

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KFILE_H
#define SMB4KFILE_H

// application specific includes
#include "smb4kbasicnetworkitem.h"
#include "smb4kcore_export.h"

// Qt includes
#include <QHostAddress>
#include <QScopedPointer>

class Smb4KFilePrivate;

class SMB4KCORE_EXPORT Smb4KFile : public Smb4KBasicNetworkItem
{
public:
    /**
     * Constructor
     *
     * @param url           The URL of the file or directory
     */
    Smb4KFile(const QUrl &url);

    /**
     * Copy constructor
     *
     * @param file          The other Smb4KFile object
     */
    Smb4KFile(const Smb4KFile &file);

    /**
     * Empty constructor
     */
    Smb4KFile();

    /**
     * Destructor
     */
    ~Smb4KFile();

    /**
     * Sets the workgroup name to @p name.
     *
     * @param name          The workgroup name
     */
    void setWorkgroupName(const QString &name) const;

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
    void setHostIpAddress(const QHostAddress &address) const;

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
     * Set the user name for the share where this file or directory is located to @p name.
     *
     * @param name          The user name
     */
    void setUserName(const QString &name) const;

    /**
     * Return the user name for the share where this file or directory is located.
     *
     * @returns the user name
     */
    QString userName() const;

    /**
     * Set the password for the share where this file or directory is located to @p pass.
     *
     * @param pass          The password
     */
    void setPassword(const QString &pass) const;

    /**
     * Return the password for the share where this file or directory is located.
     *
     * @returns the password
     */
    QString password() const;

    /**
     * Indicate that this network item is a directory.
     *
     * @param directory     Set this to TRUE if this is a directory
     */
    void setDirectory(bool directory) const;

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

    /**
     * Copy assignment operator
     */
    Smb4KFile &operator=(const Smb4KFile &other);

private:
    const QScopedPointer<Smb4KFilePrivate> d;
};

Q_DECLARE_METATYPE(Smb4KFile)

#endif
