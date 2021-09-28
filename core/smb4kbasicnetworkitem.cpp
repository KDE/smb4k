/*
    This class provides the basic network item for the core library of
    Smb4K.
    -------------------
    begin                : Do Apr 2 2009
    SPDX-FileCopyrightText: 2009-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QDebug>
#include <QtGlobal>

using namespace Smb4KGlobal;

class Smb4KBasicNetworkItemPrivate
{
public:
    NetworkItem type;
    QIcon icon;
    QUrl url;
    bool dnsDiscovered;
    QString comment;
};

Smb4KBasicNetworkItem::Smb4KBasicNetworkItem(NetworkItem type)
    : d(new Smb4KBasicNetworkItemPrivate)
{
    //
    // Set the type
    //
    d->type = type;

    //
    // Initialize the dnsDiscovered variable
    //
    d->dnsDiscovered = false;

    //
    // Initialize the protected variables
    //
    pUrl = &d->url;
    pIcon = &d->icon;
    pComment = &d->comment;
}

Smb4KBasicNetworkItem::Smb4KBasicNetworkItem(const Smb4KBasicNetworkItem &item)
    : d(new Smb4KBasicNetworkItemPrivate)
{
    //
    // Copy the private variables
    //
    *d = *item.d;

    //
    // Initialize the protected variables
    //
    pUrl = &d->url;
    pIcon = &d->icon;
    pComment = &d->comment;
}

Smb4KBasicNetworkItem::~Smb4KBasicNetworkItem()
{
}

Smb4KGlobal::NetworkItem Smb4KBasicNetworkItem::type() const
{
    return d->type;
}

void Smb4KBasicNetworkItem::setIcon(const QIcon &icon)
{
    d->icon = icon;
}

QIcon Smb4KBasicNetworkItem::icon() const
{
    return d->icon;
}

void Smb4KBasicNetworkItem::setUrl(const QUrl &url)
{
    //
    // Check that the URL is valid
    //
    if (!url.isValid()) {
        return;
    }

    //
    // Do some checks depending on the type of the network item
    //
    switch (d->type) {
    case Network: {
        break;
    }
    case Workgroup:
    case Host: {
        //
        // Check that the host name is present and there is no path
        //
        if (url.host().isEmpty() || !url.path().isEmpty()) {
            return;
        }

        break;
    }
    case Share: {
        //
        // Check that the share name is present
        //
        if (url.path().isEmpty() || (url.path().size() == 1 && url.path().endsWith('/'))) {
            return;
        }

        break;
    }
    default: {
        break;
    }
    }

    //
    // Set the URL
    //
    d->url = url;

    //
    // Force the scheme
    //
    if (d->url.scheme() != "smb") {
        d->url.setScheme("smb");
    }
}

QUrl Smb4KBasicNetworkItem::url() const
{
    return d->url;
}

void Smb4KBasicNetworkItem::setDnsDiscovered(bool discovered)
{
    d->dnsDiscovered = discovered;
}

bool Smb4KBasicNetworkItem::dnsDiscovered() const
{
    return d->dnsDiscovered;
}

void Smb4KBasicNetworkItem::setComment(const QString &comment)
{
    d->comment = comment;
}

QString Smb4KBasicNetworkItem::comment() const
{
    return d->comment;
}

bool Smb4KBasicNetworkItem::hasUserInfo() const
{
    return !d->url.userInfo().isEmpty();
}
