/*
    Private helper classes for the wallet manager of Smb4K.
    -------------------
    begin                : Mo Dez 31 2012
    SPDX-FileCopyrightText: 2012-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KWALLETMANAGER_P_H
#define SMB4KWALLETMANAGER_P_H

// application specific includes
#include "smb4kbasicnetworkitem.h"
#include "smb4kwalletmanager.h"

// KDE includes
#include <KWallet/KWallet>
#include <KWidgetsAddons/KPasswordDialog>

class Smb4KPasswordDialog : public KPasswordDialog
{
    Q_OBJECT

public:
    Smb4KPasswordDialog(const NetworkItemPtr &networkItem, const QMap<QString, QString> &knownLogins, QWidget *parent = 0);
    virtual ~Smb4KPasswordDialog();

protected Q_SLOTS:
    void slotGotUsernameAndPassword(const QString &user, const QString &pass, bool keep);

private:
    NetworkItemPtr m_item;
};

class Smb4KWalletManagerPrivate
{
public:
    KWallet::Wallet *wallet;
};

class Smb4KWalletManagerStatic
{
public:
    Smb4KWalletManager instance;
};

#endif
