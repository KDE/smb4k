/*
    This class provides the credentials manager used by Smb4K

    SPDX-FileCopyrightText: 2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCREDENTIALSMANAGER_H
#define SMB4KCREDENTIALSMANAGER_H

// application specific includes
#include "smb4kcore_export.h"
#include "smb4kglobal.h"

// Qt includes
#include <QObject>

// forward declarations
class Smb4KCredentialsManagerPrivate;

/**
 * This class manages the access to the credentials
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 4.0.0
 */

class SMB4KCORE_EXPORT Smb4KCredentialsManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KCredentialsManager(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KCredentialsManager();

    /**
     * This is a static pointer to this class.
     */
    static Smb4KCredentialsManager *self();

    /**
     * Read the login credentials for the given @p networkItem from
     * the secure storage.
     *
     * @param networkItem   The network item for which the login
     *                      credentials should be read.
     *
     * @returns TRUE if reading from the secure storage was successful.
     */
    bool readLoginCredentials(const NetworkItemPtr &networkItem);

    /**
     * Write the login credentials for the given @p networkItem to the
     * secure storage.
     *
     * @param networkItem   The network item for which the login
     *                      credentials should be saved.
     *
     * @returns TRUE if writing to the secure storage was successful.
     */
    bool writeLoginCredentials(const NetworkItemPtr &networkItem);

    /**
     * Read the default login @p credentials for the currently active
     * profile from the secure storage.
     *
     * @param username      The default username
     *
     * @param password      The default password
     *
     * @returns TRUE if reading from the secure storage was successful.
     */
    bool readDefaultLoginCredentials(QString *username, QString *password);

    /**
     * Write the default login credentials for the currently active
     * profile to the secure storage.
     *
     * @param username      The default username
     *
     * @param password      The default password
     *
     * @returns TRUE if writing to the secure storage was successful.
     */
    bool writeDefaultLoginCredentials(const QString &username, const QString &password);

    /**
     * This function returns TRUE if default credentials are defined and
     * FALSE otherwise.
     *
     * @returns TRUE if default credentials are defined.
     */
    bool hasDefaultCredentials() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the credentials for @param url were updated.
     *
     * @param QUrl          The URL for which the credentials were updated/added
     */
    void credentialsUpdated(const QUrl &url);

private:
    /**
     * Read login credentials from the secure storage.
     *
     * @param key           The key
     *
     * @param credentials   The credentials
     *
     * @returns QKeychain::NoError if everything worked out fine or an
     * error code if something went wrong.
     */
    int read(const QString &key, QString *credentials) const;

    /**
     * Write the login credentials to the secure storage.
     *
     * @param key           The key
     *
     * @param credentials   The credentials
     *
     * @returns QKeychain::NoError if everything worked out fine or an
     * error code if something went wrong.
     */
    int write(const QString &key, const QString &credentials) const;

    /**
     * Delete the login credentials from the secure storage.
     *
     * @param key           The key
     *
     * @returns QKeychain::NoError if everything worked out fine or an
     * error code if something went wrong.
     */
    int remove(const QString &key);

    /**
     * This function migrates the old credentials.
     */
    void migrate();

    /**
     * Pointer to the Smb4KWalletManagerPrivate class
     */
    const QScopedPointer<Smb4KCredentialsManagerPrivate> d;
};

#endif
