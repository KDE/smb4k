/*
    The configuration page for the authentication settings of Smb4K

    SPDX-FileCopyrightText: 2003-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEAUTHENTICATION_H
#define SMB4KCONFIGPAGEAUTHENTICATION_H

// Qt includes
#include <QWidget>

// KDE includes
#include <KLineEdit>
#include <KPasswordLineEdit>

/**
 * This is the configuration tab for the authentication settings
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KConfigPageAuthentication : public QWidget
{
    Q_OBJECT

public:
    /**
     * The constructor.
     *
     * @param parent          The parent widget
     */
    explicit Smb4KConfigPageAuthentication(QWidget *parent = nullptr);

    /**
     * The destructor.
     */
    virtual ~Smb4KConfigPageAuthentication();

    /**
     * Saves the current default login credentials to the secure storage.
     */
    void saveDefaultLoginCredentials();

    /**
     * Returns TRUE if the current default login credentials were changed.
     *
     * @returns TRUE if the current default login credentials changed.
     */
    bool defaultLoginCredentialsChanged();

Q_SIGNALS:
    /**
     * This signal is emitted every time the default login credentials of
     * the current profile changed.
     */
    void defaultLoginCredentialsModified();

protected Q_SLOTS:
    /**
     * This slot is called when the username was edited.
     *
     * @param userName        The default username
     */
    void slotDefaultUserNameEdited(const QString &userName);

    /**
     * This slot is called when the password was edited.
     *
     * @param password        The default password
     */
    void slotDefaultPasswordEdited(const QString &password);

private:
    void loadDefaultLoginCredentials();
    KLineEdit *m_defaultUserName;
    KPasswordLineEdit *m_defaultPassword;
    bool m_defaultCredentialsModified;
};

#endif
