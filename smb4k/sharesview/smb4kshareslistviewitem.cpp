/***************************************************************************
    smb4kshareslistviewitem  -  The shares list view item class of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
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
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kshareslistviewitem.h"
#include "smb4kshareslistview.h"

// Qt includes
#include <QtGui/QPixmap>
#include <QtGui/QBrush>

// KDE includes
#include <KI18n/KLocalizedString>


Smb4KSharesListViewItem::Smb4KSharesListViewItem(Smb4KSharesListView *parent, Smb4KShare *share, bool mountpoint)
: QTreeWidgetItem(parent), m_mountpoint(mountpoint)
{
  setFlags(flags() | Qt::ItemIsDropEnabled);

  // Copy share object
  m_share = new Smb4KShare(*share);
  
  m_tooltip   = new Smb4KToolTip();
  m_tooltip->setup(Smb4KToolTip::SharesView, m_share);
  
  // Set up the text.
  if (!m_mountpoint)
  {
    setText(Item, m_share->unc());
  }
  else
  {
    setText(Item, m_share->path());
  }

  setText(Owner, QString("%1 - %2").arg(m_share->user().loginName()).arg(m_share->group().name()));

#if defined(Q_OS_LINUX)
  switch (m_share->fileSystem())
  {
    case Smb4KShare::CIFS:
    {
      if (!m_share->login().isEmpty())
      {
        setText(Login, m_share->login());
      }
      else
      {
        setText(Login, i18n("unknown"));
      }
      break;
    }
    default:
    {
      setText(Login, "-");
      break;
    }
  }
#endif

  setText(FileSystem, m_share->fileSystemString().toUpper());
  setText(Used, m_share->usedDiskSpaceString());
  setText(Free, m_share->freeDiskSpaceString());
  setText(Total, m_share->totalDiskSpaceString());
  setText(Usage, m_share->diskUsageString());

  // Alignment
  setTextAlignment(Used, Qt::AlignRight|Qt::AlignVCenter);
  setTextAlignment(Free, Qt::AlignRight|Qt::AlignVCenter);
  setTextAlignment(Total, Qt::AlignRight|Qt::AlignVCenter);
  setTextAlignment(Usage, Qt::AlignRight|Qt::AlignVCenter);

  setIcon(Item, m_share->icon());
}


Smb4KSharesListViewItem::~Smb4KSharesListViewItem()
{
  delete m_share;
  delete m_tooltip;
}


void Smb4KSharesListViewItem::setShowMountPoint(bool show)
{
  m_mountpoint = show;
  update(m_share);
}


void Smb4KSharesListViewItem::update(Smb4KShare *share)
{
  delete m_share;
  m_share = new Smb4KShare(*share);
  
  m_tooltip->update(Smb4KToolTip::SharesView, m_share);
  
  // Set up the text.
  if (!m_mountpoint)
  {
    setText(Item, m_share->unc());
  }
  else
  {
    setText(Item, m_share->path());
  }

  setText(Owner, QString("%1 - %2").arg(m_share->user().loginName()).arg(m_share->group().name()));

#if defined(Q_OS_LINUX)
  switch (m_share->fileSystem())
  {
    case Smb4KShare::CIFS:
    {
      if (!m_share->login().isEmpty())
      {
        setText(Login, m_share->login());
      }
      else
      {
        setText(Login, i18n("unknown"));
      }
      break;
    }
    default:
    {
      setText(Login, "-");
      break;
    }
  }
#endif

  setText(FileSystem, m_share->fileSystemString().toUpper());
  setText(Used, m_share->usedDiskSpaceString());
  setText(Free, m_share->freeDiskSpaceString());
  setText(Total, m_share->totalDiskSpaceString());
  setText(Usage, m_share->diskUsageString());

  setIcon(Item, m_share->icon());
}


Smb4KToolTip* Smb4KSharesListViewItem::tooltip()
{
  return m_tooltip;
}


