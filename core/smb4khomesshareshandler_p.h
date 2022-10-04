/*
    Private helpers for the homes shares handler

    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KHOMESSHARESHANDLER_P_H
#define SMB4KHOMESSHARESHANDLER_P_H

// application specific includes
#include "smb4kglobal.h"
#include "smb4khomesshareshandler.h"

// Qt includes
#include <QDialog>
#include <QHostAddress>
#include <QPushButton>
#include <QStringList>

// KDE includes
#include <KCompletion/KComboBox>

class Smb4KHomesUsers
{
public:
    /**
     * Constructor
     */
    Smb4KHomesUsers(const SharePtr &share, const QStringList &users);

    /**
     * Copy constructor
     */
    Smb4KHomesUsers(const Smb4KHomesUsers &users);

    /**
     * Empty constructor
     */
    Smb4KHomesUsers();

    /**
     * Destructor
     */
    ~Smb4KHomesUsers();

    /**
     * Workgroup name
     */
    QString workgroupName() const;

    /**
     * Set workgroup name
     */
    void setWorkgroupName(const QString &name);

    /**
     * Host name
     */
    QString hostName() const;

    /**
     * Set host name
     */
    void setHostName(const QString &name);

    /**
     * Share name
     */
    QString shareName() const;

    /**
     * Set share name
     */
    void setShareName(const QString &name);

    /**
     * IP address
     */
    QString hostIP() const;

    /**
     * Set IP address
     */
    void setHostIP(const QString &ip);

    /**
     * User list
     */
    QStringList userList() const;

    /**
     * Set user list
     */
    void setUserList(const QStringList &users);

    /**
     * Profile
     */
    QString profile() const;

    /**
     * Set profile
     */
    void setProfile(const QString &profile);

private:
    QString m_workgroupName;
    QString m_hostName;
    QString m_shareName;
    QHostAddress m_hostIp;
    QStringList m_userList;
    QString m_profile;
};

class Smb4KHomesUserDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KHomesUserDialog(const SharePtr &share, QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KHomesUserDialog();

    /**
     * Set the user names
     */
    void setUserNames(const QStringList &users);

    /**
     * Get the user names
     */
    QStringList userNames();

    /**
     * Get the user name to use
     */
    QString userName()
    {
        return m_user_combo->currentText();
    }

protected Q_SLOTS:
    /**
     * Is connected to the textChanged() signal of the combo box
     * in the "Specify User" dialog.
     *
     * @param text        The text in the combo box
     */
    void slotTextChanged(const QString &text);

    /**
     * This slot is called if the User1 button, i.e. the "Clear" button
     * in the "Specify User" dialog has been clicked. It removes all
     * entries from the combo box.
     */
    void slotClearClicked();

    /**
     * This slot is called when the "OK" button is clicked.
     */
    void slotOkClicked();

    /**
     * This slot is used to add the input text to the completion object
     * of the input combo box.
     */
    void slotHomesUserEntered();

private:
    void setupView();
    QPushButton *m_clear_button;
    QPushButton *m_ok_button;
    QPushButton *m_cancel_button;
    KComboBox *m_user_combo;
    SharePtr m_share;
};

class Smb4KHomesSharesHandlerPrivate
{
public:
    QList<Smb4KHomesUsers *> homesUsers;
};

class Smb4KHomesSharesHandlerStatic
{
public:
    Smb4KHomesSharesHandler instance;
};

#endif
