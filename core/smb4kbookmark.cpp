/*
    This is the bookmark container for Smb4K (next generation).

    SPDX-FileCopyrightText: 2008-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kbookmark.h"
#include "smb4kshare.h"

// Qt includes
#include <QDebug>
#include <QHostAddress>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

class Smb4KBookmarkPrivate
{
public:
    QUrl url;
    QString workgroup;
    QHostAddress ip;
    QString label;
    QString category;
    QString profile;
    QIcon icon;
    Smb4KGlobal::ShareType type;
};

Smb4KBookmark::Smb4KBookmark(Smb4KShare *share, const QString &label)
    : d(new Smb4KBookmarkPrivate)
{
    if (!share->isHomesShare()) {
        d->url = share->url();
    } else {
        d->url = share->homeUrl();
    }

    d->workgroup = share->workgroupName();
    d->type = share->shareType();
    d->label = label;
    d->icon = KDE::icon("folder-network");
    d->ip.setAddress(share->hostIpAddress());
}

Smb4KBookmark::Smb4KBookmark(const Smb4KBookmark &b)
    : d(new Smb4KBookmarkPrivate)
{
    *d = *b.d;
}

Smb4KBookmark::Smb4KBookmark()
    : d(new Smb4KBookmarkPrivate)
{
    d->type = FileShare;
    d->icon = KDE::icon("folder-network");
}

Smb4KBookmark::~Smb4KBookmark()
{
}

void Smb4KBookmark::setWorkgroupName(const QString &workgroup)
{
    d->workgroup = workgroup;
}

QString Smb4KBookmark::workgroupName() const
{
    return d->workgroup;
}

void Smb4KBookmark::setHostName(const QString &host)
{
    d->url.setHost(host);
    d->url.setScheme("smb");
}

QString Smb4KBookmark::hostName() const
{
    return d->url.host().toUpper();
}

void Smb4KBookmark::setShareName(const QString &share)
{
    d->url.setPath(share);
}

QString Smb4KBookmark::shareName() const
{
    if (d->url.path().startsWith('/')) {
        return d->url.path().remove(0, 1);
    }

    return d->url.path();
}

void Smb4KBookmark::setHostIpAddress(const QString &ip)
{
    d->ip.setAddress(ip);
}

QString Smb4KBookmark::hostIpAddress() const
{
    return d->ip.toString();
}

void Smb4KBookmark::setShareType(Smb4KGlobal::ShareType type)
{
    d->type = type;
}

Smb4KGlobal::ShareType Smb4KBookmark::shareType() const
{
    return d->type;
}

void Smb4KBookmark::setLabel(const QString &label)
{
    d->label = label;
}

QString Smb4KBookmark::label() const
{
    return d->label;
}

void Smb4KBookmark::setLogin(const QString &login)
{
    d->url.setUserName(login);
}

QString Smb4KBookmark::login() const
{
    return d->url.userName();
}

void Smb4KBookmark::setUrl(const QUrl &url)
{
    d->url = url;
    d->url.setScheme("smb");
}

void Smb4KBookmark::setUrl(const QString &url)
{
    d->url.setUrl(url, QUrl::TolerantMode);
    d->url.setScheme("smb");
}

QUrl Smb4KBookmark::url() const
{
    return d->url;
}

void Smb4KBookmark::setCategoryName(const QString &name)
{
    d->category = name;
}

QString Smb4KBookmark::categoryName() const
{
    return d->category;
}

void Smb4KBookmark::setProfile(const QString &profile)
{
    d->profile = profile;
}

QString Smb4KBookmark::profile() const
{
    return d->profile;
}

void Smb4KBookmark::setIcon(const QIcon &icon)
{
    d->icon = icon;
}

QIcon Smb4KBookmark::icon() const
{
    return d->icon;
}

QString Smb4KBookmark::displayString() const
{
    return i18n("%1 on %2", shareName(), hostName());
}
