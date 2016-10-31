/***************************************************************************
    This class derives from QObject and encapsulates a bookmark item. It
    is for use with QtQuick.
                             -------------------
    begin                : Fr Mai 11 2013
    copyright            : (C) 2013-2016 by Alexander Reinholdt
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
#include "smb4kbookmarkobject.h"

// KDE includes
#include <KIconThemes/KIconLoader>


class Smb4KBookmarkObjectPrivate
{
  public:
    QString workgroup;
    QUrl url;
    QIcon icon;
    QString label;
    QString group;
    bool isGroup;
    bool isMounted;
};


Smb4KBookmarkObject::Smb4KBookmarkObject(Smb4KBookmark* bookmark, QObject* parent)
: QObject(parent), d(new Smb4KBookmarkObjectPrivate)
{
  d->workgroup  = bookmark->workgroupName();
  d->url        = bookmark->url();
  d->icon       = bookmark->icon();
  d->label      = bookmark->label();
  d->group      = bookmark->groupName();
  d->isGroup    = false;
  d->isMounted  = false;
}


Smb4KBookmarkObject::Smb4KBookmarkObject(const QString& groupName, QObject* parent)
: QObject(parent), d(new Smb4KBookmarkObjectPrivate)
{
  d->icon       = KDE::icon("folder-favorites");
  d->group      = groupName;
  d->isGroup    = true;
  d->isMounted  = false;
}



Smb4KBookmarkObject::Smb4KBookmarkObject(QObject* parent)
: QObject(parent), d(new Smb4KBookmarkObjectPrivate)
{
  d->isGroup    = false;
  d->isMounted  = false;
}


Smb4KBookmarkObject::~Smb4KBookmarkObject()
{
}


QString Smb4KBookmarkObject::workgroupName() const
{
  return d->workgroup;
}


void Smb4KBookmarkObject::setWorkgroupName(const QString& name)
{
  d->workgroup = name;
  emit changed();
}


QString Smb4KBookmarkObject::unc() const
{
  QString path = (d->url.path().startsWith('/') ? d->url.path().remove(0, 1) : d->url.path());
  return QString("//%1/%2").arg(d->url.host().toUpper()).arg(path);
}


QString Smb4KBookmarkObject::label() const
{
  return d->label;
}


void Smb4KBookmarkObject::setLabel(const QString& label)
{
  d->label = label;
  emit changed();
}


QUrl Smb4KBookmarkObject::url() const
{
  return d->url;
}


void Smb4KBookmarkObject::setURL(const QUrl& url)
{
  d->url = url;
  emit changed();
}


QIcon Smb4KBookmarkObject::icon() const
{
  return d->icon;
}


void Smb4KBookmarkObject::setIcon(const QIcon& icon)
{
  d->icon = icon;
  emit changed();
}


QString Smb4KBookmarkObject::groupName() const
{
  return d->group;
}


void Smb4KBookmarkObject::setGroupName(const QString& name)
{
  d->group = name;
  emit changed();
}


bool Smb4KBookmarkObject::isGroup() const
{
  return d->isGroup;
}


void Smb4KBookmarkObject::setGroup(bool group)
{
  d->isGroup = group;
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

