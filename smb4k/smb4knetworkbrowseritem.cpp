/*
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.

    SPDX-FileCopyrightText: 2007-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knetworkbrowseritem.h"
#include "core/smb4kglobal.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kworkgroup.h"

// Qt includes
#include <QApplication>
#include <QBrush>
#include <QDebug>

using namespace Smb4KGlobal;

Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem(QTreeWidget *parent, const NetworkItemPtr &item)
    : QTreeWidgetItem(parent, item->type())
    , m_item(item)
{
    switch (m_item->type()) {
    case Workgroup: {
        WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
        setText(Network, workgroup->workgroupName());
        setIcon(Network, workgroup->icon());
        break;
    }
    case Host: {
        HostPtr host = m_item.staticCast<Smb4KHost>();
        setText(Network, host->hostName());
        setText(IP, host->ipAddress());
        setText(Comment, host->comment());

        if (host->isMasterBrowser()) {
            for (int i = 0; i < columnCount(); ++i) {
                QBrush brush(Qt::darkBlue);
                setForeground(i, brush);
            }
        }

        setIcon(Network, host->icon());
        break;
    }
    case Share: {
        SharePtr share = m_item.staticCast<Smb4KShare>();
        setText(Network, share->shareName());
        setText(Type, share->shareTypeString());
        setText(Comment, share->comment());

        if (!share->isPrinter() && share->isMounted()) {
            for (int i = 0; i < columnCount(); ++i) {
                QFont f = font(i);
                f.setItalic(true);
                setFont(i, f);
            }
        }

        setIcon(Network, share->icon());
        break;
    }
    default: {
        break;
    }
    }
}

Smb4KNetworkBrowserItem::Smb4KNetworkBrowserItem(QTreeWidgetItem *parent, const NetworkItemPtr &item)
    : QTreeWidgetItem(parent, item->type())
    , m_item(item)
{
    switch (m_item->type()) {
    case Workgroup: {
        WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
        setText(Network, workgroup->workgroupName());
        setIcon(Network, workgroup->icon());
        break;
    }
    case Host: {
        HostPtr host = m_item.staticCast<Smb4KHost>();
        setText(Network, host->hostName());
        setText(IP, host->ipAddress());
        setText(Comment, host->comment());

        if (host->isMasterBrowser()) {
            for (int i = 0; i < columnCount(); ++i) {
                QBrush brush(Qt::darkBlue);
                setForeground(i, brush);
            }
        }

        setIcon(Network, host->icon());
        break;
    }
    case Share: {
        SharePtr share = m_item.staticCast<Smb4KShare>();
        setText(Network, share->shareName());
        setText(Type, share->shareTypeString());
        setText(Comment, share->comment());

        if (!share->isPrinter() && share->isMounted()) {
            for (int i = 0; i < columnCount(); ++i) {
                QFont f = font(i);
                f.setItalic(true);
                setFont(i, f);
            }
        }

        setIcon(Network, share->icon());
        break;
    }
    default: {
        break;
    }
    }
}

Smb4KNetworkBrowserItem::~Smb4KNetworkBrowserItem()
{
}

WorkgroupPtr Smb4KNetworkBrowserItem::workgroupItem()
{
    if (!m_item || (m_item->type() != Workgroup)) {
        return WorkgroupPtr();
    }

    return m_item.staticCast<Smb4KWorkgroup>();
}

HostPtr Smb4KNetworkBrowserItem::hostItem()
{
    if (!m_item || (m_item->type() != Host)) {
        return HostPtr();
    }

    return m_item.staticCast<Smb4KHost>();
}

SharePtr Smb4KNetworkBrowserItem::shareItem()
{
    if (!m_item || (m_item->type() != Share)) {
        return SharePtr();
    }

    return m_item.staticCast<Smb4KShare>();
}

const NetworkItemPtr &Smb4KNetworkBrowserItem::networkItem()
{
    return m_item;
}

void Smb4KNetworkBrowserItem::update()
{
    switch (m_item->type()) {
    case Host: {
        HostPtr host = m_item.staticCast<Smb4KHost>();

        // Adjust the item's color.
        if (host->isMasterBrowser()) {
            for (int i = 0; i < columnCount(); ++i) {
                QBrush brush(Qt::darkBlue);
                setForeground(i, brush);
            }
        } else {
            for (int i = 0; i < columnCount(); ++i) {
                QBrush brush = QApplication::palette().text();
                setForeground(i, brush);
            }
        }

        // Set the IP address
        setText(IP, host->ipAddress());

        // Set the comment
        setText(Comment, host->comment());
        break;
    }
    case Share: {
        SharePtr share = m_item.staticCast<Smb4KShare>();

        // Set the comment
        setText(Comment, share->comment());

        // Set the icon
        setIcon(Network, share->icon());

        // Set the font
        for (int i = 0; i < columnCount(); ++i) {
            QFont f = font(i);
            f.setItalic(share->isMounted());
            setFont(i, f);
        }

        break;
    }
    default: {
        break;
    }
    }
}
