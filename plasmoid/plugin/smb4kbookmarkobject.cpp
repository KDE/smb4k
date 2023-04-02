/*
    This class derives from QObject and encapsulates a bookmark item. It
    is for use with QtQuick.

    SPDX-FileCopyrightText: 2013-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kbookmarkobject.h"

// Qt includes
#include <QHostAddress>

// KDE includes
#include <KIconLoader>

class Smb4KBookmarkObjectPrivate
{
public:
    QString workgroup;
    QUrl url;
    QString label;
    QString category;
    QString login;
    bool isCategory;
    bool isMounted;
    QHostAddress hostIp;
};

Smb4KBookmarkObject::Smb4KBookmarkObject(Smb4KBookmark *bookmark, QObject *parent)
    : QObject(parent)
    , d(new Smb4KBookmarkObjectPrivate)
{
    d->workgroup = bookmark->workgroupName();
    d->url = bookmark->url();
    d->label = bookmark->label();
    d->category = bookmark->categoryName();
    d->login = bookmark->userName();
    d->isCategory = false;
    d->isMounted = false;
    d->hostIp.setAddress(bookmark->hostIpAddress());
}

Smb4KBookmarkObject::Smb4KBookmarkObject(const QString &categoryName, QObject *parent)
    : QObject(parent)
    , d(new Smb4KBookmarkObjectPrivate)
{
    d->category = categoryName;
    d->isCategory = true;
    d->isMounted = false;
}

Smb4KBookmarkObject::Smb4KBookmarkObject(QObject *parent)
    : QObject(parent)
    , d(new Smb4KBookmarkObjectPrivate)
{
    d->isCategory = false;
    d->isMounted = false;
}

Smb4KBookmarkObject::~Smb4KBookmarkObject()
{
}

QString Smb4KBookmarkObject::workgroupName() const
{
    return d->workgroup;
}

void Smb4KBookmarkObject::setWorkgroupName(const QString &name)
{
    if (d->workgroup != name) {
        d->workgroup = name;
        Q_EMIT changed();
    }
}

QString Smb4KBookmarkObject::hostName() const
{
    return d->url.host().toUpper();
}

QString Smb4KBookmarkObject::shareName() const
{
    return d->url.path().remove(QStringLiteral("/"));
}

QString Smb4KBookmarkObject::label() const
{
    return d->label;
}

void Smb4KBookmarkObject::setLabel(const QString &label)
{
    if (d->label != label) {
        d->label = label;
        Q_EMIT changed();
    }
}

QUrl Smb4KBookmarkObject::url() const
{
    return d->url;
}

void Smb4KBookmarkObject::setUrl(const QUrl &url)
{
    if (!d->url.matches(url, QUrl::None)) {
        d->url = url;
        Q_EMIT changed();
    }
}

QString Smb4KBookmarkObject::categoryName() const
{
    return d->category;
}

void Smb4KBookmarkObject::setCategoryName(const QString &name)
{
    if (d->category != name) {
        d->category = name;
        Q_EMIT changed();
    }
}

bool Smb4KBookmarkObject::isCategory() const
{
    return d->isCategory;
}

void Smb4KBookmarkObject::setCategory(bool category)
{
    if (d->isCategory != category) {
        d->isCategory = category;
        Q_EMIT changed();
    }
}

bool Smb4KBookmarkObject::isMounted() const
{
    return d->isMounted;
}

void Smb4KBookmarkObject::setMounted(bool mounted)
{
    if (d->isMounted != mounted) {
        d->isMounted = mounted;
        Q_EMIT changed();
    }
}

QString Smb4KBookmarkObject::userName() const
{
    return d->login;
}

void Smb4KBookmarkObject::setUserName(const QString &name)
{
    if (d->login != name) {
        d->login = name;
        Q_EMIT changed();
    }
}

QString Smb4KBookmarkObject::hostIpAddress() const
{
    return d->hostIp.toString();
}

void Smb4KBookmarkObject::setHostIpAddress(const QString &ip)
{
    if (d->hostIp.toString() != ip) {
        d->hostIp.setAddress(ip);
        Q_EMIT changed();
    }
}
