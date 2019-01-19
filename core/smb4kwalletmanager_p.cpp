/***************************************************************************
    Private helper classes for the wallet manager of Smb4K.
                             -------------------
    begin                : Mo Dez 31 2012
    copyright            : (C) 2012-2019 by Alexander Reinholdt
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

// application specific includes
#include "smb4kwalletmanager_p.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"

// Qt includes
#include <QDebug>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>

using namespace Smb4KGlobal;


Smb4KPasswordDialog::Smb4KPasswordDialog(const NetworkItemPtr &networkItem, const QMap<QString,QString> &knownLogins, QWidget* parent)
: KPasswordDialog(parent, KPasswordDialog::ShowUsernameLine)
{
  m_item = networkItem;
  
  switch (m_item->type())
  {
    case Host:
    {
      HostPtr host = m_item.staticCast<Smb4KHost>();

      if (host)
      {
        setUsername(host->login());
        setPassword(host->password());
        setPrompt(i18n("<qt>Please enter a username and a password for the host <b>%1</b>.</qt>", host->hostName()));
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Share:
    {
      SharePtr share = m_item.staticCast<Smb4KShare>();

      if (share)
      {
        // Enter authentication information into the dialog
        if (!knownLogins.isEmpty())
        {
          setKnownLogins(knownLogins);
        }
        else
        {
          setUsername(share->login());
          setPassword(share->password());
        }

        if (!share->isHomesShare())
        {
          setPrompt(i18n("<qt>Please enter a username and a password for the share <b>%1</b>.</qt>", share->displayString()));
        }
        else
        {
          setPrompt(i18n("<qt>Please enter a username and a password for the share <b>%1 on %2</b>.</qt>", share->displayString(true)));
        }
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }

  connect(this, SIGNAL(gotUsernameAndPassword(QString,QString,bool)), SLOT(slotGotUsernameAndPassword(QString,QString,bool)));
}


Smb4KPasswordDialog::~Smb4KPasswordDialog()
{
}


void Smb4KPasswordDialog::slotGotUsernameAndPassword(const QString &user, const QString &pass, bool /*keep*/)
{
  switch (m_item->type())
  {
    case Host:
    {
      HostPtr host = m_item.staticCast<Smb4KHost>();

      if (host)
      {
        host->setLogin(user);
        host->setPassword(pass);
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Share:
    {
      SharePtr share = m_item.staticCast<Smb4KShare>();

      if (share)
      {
        share->setLogin(user);
        share->setPassword(pass);
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }
}

