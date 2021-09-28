/*
    This class derives from QObject and encapsulates a bookmark item. It
    is for use with QtQuick.
    -------------------
    begin                : Fr Mai 11 2013
    SPDX-FileCopyrightText: 2013-2021 Alexander Reinholdt
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
#include "smb4kbookmarkobject.h"

// Qt includes
#include <QHostAddress>

// KDE includes
#include <KIconThemes/KIconLoader>

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
    QHostAddress hostIP;
};

Smb4KBookmarkObject::Smb4KBookmarkObject(Smb4KBookmark *bookmark, QObject *parent)
    : QObject(parent)
    , d(new Smb4KBookmarkObjectPrivate)
{
    d->workgroup = bookmark->workgroupName();
    d->url = bookmark->url();
    d->label = bookmark->label();
    d->category = bookmark->categoryName();
    d->login = bookmark->login();
    d->isCategory = false;
    d->isMounted = false;
    d->hostIP.setAddress(bookmark->hostIpAddress());
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
    d->workgroup = name;
    emit changed();
}

QString Smb4KBookmarkObject::hostName() const
{
    return d->url.host().toUpper();
}

QString Smb4KBookmarkObject::shareName() const
{
    return d->url.path().remove('/');
}

QString Smb4KBookmarkObject::label() const
{
    return d->label;
}

void Smb4KBookmarkObject::setLabel(const QString &label)
{
    d->label = label;
    emit changed();
}

QUrl Smb4KBookmarkObject::url() const
{
    return d->url;
}

void Smb4KBookmarkObject::setUrl(const QUrl &url)
{
    d->url = url;
    emit changed();
}

QString Smb4KBookmarkObject::categoryName() const
{
    return d->category;
}

void Smb4KBookmarkObject::setCategoryName(const QString &name)
{
    d->category = name;
    emit changed();
}

bool Smb4KBookmarkObject::isCategory() const
{
    return d->isCategory;
}

void Smb4KBookmarkObject::setCategory(bool category)
{
    d->isCategory = category;
    emit changed();
}

bool Smb4KBookmarkObject::isMounted() const
{
    return d->isMounted;
}

void Smb4KBookmarkObject::setMounted(bool mounted)
{
    d->isMounted = mounted;
    emit changed();
}

QString Smb4KBookmarkObject::login() const
{
    return d->login;
}

void Smb4KBookmarkObject::setLogin(const QString &name)
{
    d->login = name;
    emit changed();
}

QString Smb4KBookmarkObject::hostIP() const
{
    return d->hostIP.toString();
}

void Smb4KBookmarkObject::setHostIP(const QString &ip)
{
    if (d->hostIP.setAddress(ip)) {
        emit changed();
    }
}
