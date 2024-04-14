/*
    This class provides a container for the authentication data.

    SPDX-FileCopyrightText: 2004-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KAUTHINFO_H
#define SMB4KAUTHINFO_H

// application specific includes
#include "smb4kbasicnetworkitem.h"
#include "smb4kcore_export.h"
#include "smb4kglobal.h"

// Qt includes
#include <QScopedPointer>
#include <QString>
#include <QUrl>

// forward declarations
class Smb4KAuthInfoPrivate;

using namespace Smb4KGlobal;

/**
 * This class provides a container for the authentication data.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class SMB4KCORE_EXPORT Smb4KAuthInfo
{
    friend class Smb4KAuthInfoPrivate;

public:
    /**
     * Constructor
     *
     * @param item      The network item
     */
    explicit Smb4KAuthInfo(Smb4KBasicNetworkItem *item);

    /**
     * The empty constructor.
     */
    Smb4KAuthInfo();

    /**
     * The copy constructor.
     *
     * @param info      The Smb4KAuthInfo object that will be copied.
     */
    Smb4KAuthInfo(const Smb4KAuthInfo &info);

    /**
     * The destructor
     */
    ~Smb4KAuthInfo();

    /**
     * Sets the user name.
     *
     * In case of a 'homes' share, this function will also set the share
     * name to @p username.
     *
     * @param username  The user name for the server/share
     */
    void setUserName(const QString &username);

    /**
     * Returns the username.
     *
     * @returns         The username
     */
    QString userName() const;

    /**
     * Sets the password.
     *
     * @param passwd    The password for the server/share
     */
    void setPassword(const QString &passwd);

    /**
     * Returns the password.
     */
    QString password() const;

    /**
     * Returns the type.
     *
     * @returns the type.
     */
    Smb4KGlobal::NetworkItem type() const;

    /**
     * Sets the URL of the share after some checks are passed.
     *
     * @param url             The URL of the network item
     */
    void setUrl(const QUrl &url);

    /**
     * Returns the URL of the network item
     *
     * @returns the URL
     */
    QUrl url() const;

    /**
     * Returns the display string. Prefer this over all other alternatives in your
     * GUI.
     * @returns the display string.
     */
    QString displayString() const;

private:
    /**
     * Pointer to Smb4KAuthInfoPrivate class
     */
    const QScopedPointer<Smb4KAuthInfoPrivate> d;
};

#endif
