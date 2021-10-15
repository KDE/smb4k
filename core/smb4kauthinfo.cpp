/*
    This class provides a container for the authentication data.

    SPDX-FileCopyrightText: 2004-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kauthinfo.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// Qt includes
#include <QDebug>
#include <QHostAddress>

// KDE includes
#include <KI18n/KLocalizedString>

class Smb4KAuthInfoPrivate
{
public:
    QUrl url;
    NetworkItem type;
    QHostAddress ip;
};

Smb4KAuthInfo::Smb4KAuthInfo(Smb4KBasicNetworkItem *item)
    : d(new Smb4KAuthInfoPrivate)
{
    d->type = item->type();

    switch (d->type) {
    case Host: {
        Smb4KHost *host = static_cast<Smb4KHost *>(item);

        if (host) {
            d->url = host->url();
            d->ip.setAddress(host->ipAddress());
        }

        break;
    }
    case Share: {
        Smb4KShare *share = static_cast<Smb4KShare *>(item);

        if (share) {
            d->url = !share->isHomesShare() ? share->homeUrl() : share->url();
            d->ip.setAddress(share->hostIpAddress());
        }

        break;
    }
    default: {
        break;
    }
    }
}

Smb4KAuthInfo::Smb4KAuthInfo()
    : d(new Smb4KAuthInfoPrivate)
{
    d->type = UnknownNetworkItem;
    d->url.clear();
    d->ip.clear();
}

Smb4KAuthInfo::Smb4KAuthInfo(const Smb4KAuthInfo &i)
    : d(new Smb4KAuthInfoPrivate)
{
    *d = *i.d;
}

Smb4KAuthInfo::~Smb4KAuthInfo()
{
}

void Smb4KAuthInfo::setUrl(const QUrl &url)
{
    d->url = url;
    d->url.setScheme("smb");

    //
    // Set the type
    //
    if (!d->url.path().remove('/').isEmpty()) {
        d->type = Share;

        //
        // Fix 'homes' URLs
        //
        if (d->url.path().remove('/') == "homes" && !d->url.userName().isEmpty()) {
            d->url.setPath(d->url.userName());
        }
    } else {
        d->type = Host;
    }
}

void Smb4KAuthInfo::setUrl(const QString &url)
{
    QUrl tempUrl(url, QUrl::TolerantMode);
    tempUrl.setScheme("smb");
    setUrl(tempUrl);
}

QUrl Smb4KAuthInfo::url() const
{
    return d->url;
}

void Smb4KAuthInfo::setUserName(const QString &username)
{
    d->url.setUserName(username);

    if (d->url.path().remove('/') == "homes") {
        d->url.setPath(username);
    }
}

QString Smb4KAuthInfo::userName() const
{
    return d->url.userName();
}

void Smb4KAuthInfo::setPassword(const QString &passwd)
{
    d->url.setPassword(passwd);
}

QString Smb4KAuthInfo::password() const
{
    return d->url.password();
}

Smb4KGlobal::NetworkItem Smb4KAuthInfo::type() const
{
    return d->type;
}

QString Smb4KAuthInfo::displayString() const
{
    //
    // Host name
    //
    QString hostName = d->url.host().toUpper();

    //
    // Return only the host name if the network item
    // has type Smb4Global:Host
    //
    if (d->type == Host) {
        return hostName;
    }

    //
    // Share name
    //
    QString shareName = d->url.path().remove('/');

    //
    // Return the full display string
    //
    return i18n("%1 on %2", shareName, hostName);
}
