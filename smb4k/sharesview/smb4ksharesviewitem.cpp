/***************************************************************************
    smb4ksharesiconviewitem  -  The items for Smb4K's shares icon view.
                             -------------------
    begin                : Di Dez 5 2006
    copyright            : (C) 2006-2016 by Alexander Reinholdt
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
#include "smb4ksharesviewitem.h"
#include "smb4ksharesview.h"


Smb4KSharesViewItem::Smb4KSharesViewItem(Smb4KSharesView *parent, Smb4KShare *share)
: QListWidgetItem(parent)
{
  setFlags(flags() | Qt::ItemIsDropEnabled);
  setTextAlignment(Qt::AlignHCenter|Qt::AlignTop);

  m_share = new Smb4KShare(*share);
  
  m_tooltip = new Smb4KToolTip();
  m_tooltip->setup(Smb4KToolTip::SharesView, m_share);
  
  setText(m_share->displayString());
  setIcon(m_share->icon());
}


Smb4KSharesViewItem::~Smb4KSharesViewItem()
{
  delete m_share;
  delete m_tooltip;
}


void Smb4KSharesViewItem::update(Smb4KShare *share)
{
  m_share->setMountData(share);
  m_tooltip->update(Smb4KToolTip::SharesView, m_share);
  
  setText(m_share->displayString());
  setIcon(m_share->icon());
}


Smb4KToolTip* Smb4KSharesViewItem::tooltip()
{
  return m_tooltip;
}

