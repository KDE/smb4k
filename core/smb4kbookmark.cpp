/*
    This is the bookmark container for Smb4K (next generation).

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kbookmark.h"
#include "smb4kshare.h"

// Qt includes
#include <QDebug>
#include <QHostAddress>

// KDE includes
#include <KIconLoader>
#include <KLocalizedString>

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
    setShare(share);
    d->label = label;
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
    d->icon = KDE::icon(QStringLiteral("folder-network"));
}

Smb4KBookmark::~Smb4KBookmark()
{
}

void Smb4KBookmark::setShare(Smb4KShare *share) const
{
    if (!share->isHomesShare()) {
        d->url = share->url();
    } else {
        d->url = share->homeUrl();
    }

    d->workgroup = share->workgroupName();
    d->type = share->shareType();
    d->icon = KDE::icon(QStringLiteral("folder-network"));
    d->ip.setAddress(share->hostIpAddress());
}

void Smb4KBookmark::setWorkgroupName(const QString &workgroup) const
{
    d->workgroup = workgroup;
}

QString Smb4KBookmark::workgroupName() const
{
    return d->workgroup;
}

QString Smb4KBookmark::hostName() const
{
    return d->url.host().toUpper();
}

QString Smb4KBookmark::shareName() const
{
    if (d->url.path().startsWith(QStringLiteral("/"))) {
        return d->url.path().remove(0, 1);
    }

    return d->url.path();
}

void Smb4KBookmark::setHostIpAddress(const QString &ip) const
{
    d->ip.setAddress(ip);
}

QString Smb4KBookmark::hostIpAddress() const
{
    return d->ip.toString();
}

void Smb4KBookmark::setShareType(Smb4KGlobal::ShareType type) const
{
    d->type = type;
}

Smb4KGlobal::ShareType Smb4KBookmark::shareType() const
{
    return d->type;
}

void Smb4KBookmark::setLabel(const QString &label) const
{
    d->label = label;
}

QString Smb4KBookmark::label() const
{
    return d->label;
}

void Smb4KBookmark::setUserName(const QString &name) const
{
    d->url.setUserName(name);
}

QString Smb4KBookmark::userName() const
{
    return d->url.userName();
}

void Smb4KBookmark::setUrl(const QUrl &url) const
{
    d->url = url;
    d->url.setScheme(QStringLiteral("smb"));
}

QUrl Smb4KBookmark::url() const
{
    return d->url;
}

void Smb4KBookmark::setCategoryName(const QString &name) const
{
    d->category = name;
}

QString Smb4KBookmark::categoryName() const
{
    return d->category;
}

void Smb4KBookmark::setProfile(const QString &profile) const
{
    d->profile = profile;
}

QString Smb4KBookmark::profile() const
{
    return d->profile;
}

void Smb4KBookmark::setIcon(const QIcon &icon) const
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

Smb4KBookmark &Smb4KBookmark::operator=(const Smb4KBookmark &other)
{
    *d = *other.d;
    return *this;
}
