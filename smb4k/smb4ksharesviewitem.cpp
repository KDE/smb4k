/*
    The item for Smb4K's shares view.
    -------------------
    begin                : Di Dez 5 2006
    SPDX-FileCopyrightText: 2006-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
#include "smb4ksharesviewitem.h"
#include "core/smb4kshare.h"
#include "smb4ksharesview.h"

Smb4KSharesViewItem::Smb4KSharesViewItem(Smb4KSharesView *parent, const SharePtr &share)
    : QListWidgetItem(parent)
    , m_share(share)
{
    setFlags(flags() | Qt::ItemIsDropEnabled);
    setItemAlignment(parent->viewMode());

    setText(m_share->displayString());
    setIcon(m_share->icon());
}

Smb4KSharesViewItem::~Smb4KSharesViewItem()
{
}

void Smb4KSharesViewItem::update()
{
    setText(m_share->displayString());
    setIcon(m_share->icon());
}

void Smb4KSharesViewItem::setItemAlignment(QListView::ViewMode mode)
{
    switch (mode) {
    case QListView::IconMode: {
        setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        break;
    }
    case QListView::ListMode: {
        setTextAlignment(Qt::AlignAbsolute | Qt::AlignVCenter);
        break;
    }
    default: {
        break;
    }
    }
}
