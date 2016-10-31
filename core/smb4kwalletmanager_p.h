/***************************************************************************
    Private helper classes for the wallet manager of Smb4K.
                             -------------------
    begin                : Mo Dez 31 2012
    copyright            : (C) 2012-2016 by Alexander Reinholdt
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

#ifndef SMB4KWALLETMANAGER_P_H
#define SMB4KWALLETMANAGER_P_H

// application specific includes
#include "smb4kwalletmanager.h"
#include "smb4kbasicnetworkitem.h"

// KDE includes
#include <KWallet/KWallet>
#include <KWidgetsAddons/KPasswordDialog>


class Smb4KPasswordDialog : public KPasswordDialog
{
  Q_OBJECT
  
  public:
    Smb4KPasswordDialog(Smb4KBasicNetworkItem *networkItem,
                         const QMap<QString,QString> &knownLogins,
                         QWidget *parent = 0);
    virtual ~Smb4KPasswordDialog();

  protected Q_SLOTS:
    void slotGotUsernameAndPassword(const QString &user,
                                    const QString &pass,
                                    bool keep);

  private:
    Smb4KBasicNetworkItem *m_item;
};


class Smb4KWalletManagerPrivate
{
  public:
    KWallet::Wallet *wallet;
    bool initialized;
};


class Smb4KWalletManagerStatic
{
  public:
    Smb4KWalletManager instance;
};

#endif
