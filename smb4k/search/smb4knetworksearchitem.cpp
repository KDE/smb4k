/***************************************************************************
    smb4ksearchdialogitem  -  This class is an enhanced version of a list
    box item for Smb4K.
                             -------------------
    begin                : So Jun 3 2007
    copyright            : (C) 2007-2015 by Alexander Reinholdt
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
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4knetworksearchitem.h"
#include "smb4knetworksearch.h"

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>


Smb4KNetworkSearchItem::Smb4KNetworkSearchItem(QListWidget *listWidget, Smb4KShare *share)
: QListWidgetItem(listWidget, Share)
{
  m_share = new Smb4KShare(*share);
  setupItem();
}


Smb4KNetworkSearchItem::Smb4KNetworkSearchItem(QListWidget *listWidget)
: QListWidgetItem(listWidget, Failure)
{
  m_share = 0;
  setupItem();
}


Smb4KNetworkSearchItem::~Smb4KNetworkSearchItem()
{
  delete m_share;
}


void Smb4KNetworkSearchItem::setupItem()
{
  switch(type())
  {
    case Share:
    {
      setText(m_share->unc());

      if (m_share->isMounted())
      {
        QStringList overlays;
        overlays.append("emblem-mounted");
        setIcon(KDE::icon("folder-remote", overlays));
      }
      else
      {
        setIcon(KDE::icon("folder-remote"));
      }

      break;
    }
    case Failure:
    {
      setText(i18n("The search returned no results."));
      setIcon(KDE::icon("dialog-error"));
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkSearchItem::update(Smb4KShare *share)
{
  m_share->setMounted(share->isMounted());
  m_share->setPath(share->path());
  m_share->setForeign(share->isForeign());

  setupItem();
}

