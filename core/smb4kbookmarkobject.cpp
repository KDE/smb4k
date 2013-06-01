/***************************************************************************
    smb4kbookmarkobject -  This class derives from QObject and
    encapsulates a bookmark item. It is for use with QtQuick.
                             -------------------
    begin                : Fr Mär 02 2012
    copyright            : (C) 2012 by Alexander Reinholdt
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
#include "smb4ksettings.h"

// KDE includes
#include "kicon.h"


class Smb4KBookmarkObjectPrivate
{
  public:
    QString workgroup;
    QString unc;
    KUrl url;
    QIcon icon;
    QString label;
    QString group;
    bool isGroup;
};


Smb4KBookmarkObject::Smb4KBookmarkObject(Smb4KBookmark* bookmark, QObject* parent)
: QObject(parent), d( new Smb4KBookmarkObjectPrivate )
{
  d->workgroup  = bookmark->workgroupName();
  d->unc        = bookmark->unc();
  d->url        = bookmark->url();
  d->icon       = bookmark->icon();
  d->label      = bookmark->label();
  d->group      = bookmark->group();
  d->isGroup    = false;
}


Smb4KBookmarkObject::Smb4KBookmarkObject(const QString& groupName, QObject* parent)
: QObject(parent), d( new Smb4KBookmarkObjectPrivate )
{
  d->icon       = KIcon("folder-favorites");
  d->group      = groupName;
  d->isGroup    = true;
}



Smb4KBookmarkObject::Smb4KBookmarkObject(QObject* parent)
: QObject(parent), d( new Smb4KBookmarkObjectPrivate )
{
  d->isGroup    = false;
}


Smb4KBookmarkObject::~Smb4KBookmarkObject()
{
}


QString Smb4KBookmarkObject::workgroupName() const
{
  return d->workgroup;
}


QString Smb4KBookmarkObject::unc() const
{
  return d->unc;
}


QString Smb4KBookmarkObject::label() const
{
  return d->label;
}


QString Smb4KBookmarkObject::description() const
{
  QString desc;
  
  if ( !d->isGroup )
  {
    if ( Smb4KSettings::showCustomBookmarkLabel() && !d->label.isEmpty() )
    {
      desc = d->label;
    }
    else
    {
      desc = d->unc;
    }
  }
  else
  {
    desc = d->group;
  }
  
  return desc;
}


KUrl Smb4KBookmarkObject::url() const
{
  return d->url;
}


QIcon Smb4KBookmarkObject::icon() const
{
  return d->icon;
}


QString Smb4KBookmarkObject::group() const
{
  return d->group;
}


bool Smb4KBookmarkObject::isGroup() const
{
  return d->isGroup;
}


#include "smb4kbookmarkobject.moc"
