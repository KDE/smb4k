/***************************************************************************
    This is the bookmark container for Smb4K (next generation).
                             -------------------
    begin                : So Jun 8 2008
    copyright            : (C) 2008-2018 by Alexander Reinholdt
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
    QString group;
    QString profile;
    QIcon icon;
    Smb4KGlobal::NetworkItem type;
};


Smb4KBookmark::Smb4KBookmark(Smb4KShare *share, const QString &label)
: d(new Smb4KBookmarkPrivate)
{
  if (!share->isHomesShare())
  {
    d->url = share->url();
  }
  else
  {
    d->url = share->homeUrl();
  }

  d->workgroup = share->workgroupName();
  d->type      = share->shareType();
  d->label     = label;
  d->icon      = KDE::icon("folder-network");
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


void Smb4KBookmark::setWorkgroupName( const QString &workgroup )
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
  if (d->url.path().startsWith('/'))
  {
    return d->url.path().remove(0, 1);
  }
  else
  {
    // Do nothing
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


void Smb4KBookmark::setShareType(Smb4KGlobal::NetworkItem type)
{
  d->type = type;
}


Smb4KGlobal::NetworkItem Smb4KBookmark::shareType() const
{
  return d->type;
}


QString Smb4KBookmark::unc() const
{
  QString unc;
  
  if (!hostName().isEmpty() && !shareName().isEmpty())
  {
    unc = QString("//%1/%2").arg(hostName()).arg(shareName());
  }
  else 
  {
    // Do nothing
  }
  
  return unc;
}


QString Smb4KBookmark::hostUNC() const
{
  QString unc;
  
  if (!hostName().isEmpty())
  {
    unc = QString("//%1").arg(hostName());
  }
  else
  {
    // Do nothing
  }
  
  return unc;
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


void Smb4KBookmark::setGroupName(const QString &name)
{
  d->group = name;
}


QString Smb4KBookmark::groupName() const
{
  return d->group;
}


void Smb4KBookmark::setProfile(const QString &profile)
{
  d->profile = profile;
}


QString Smb4KBookmark::profile() const
{
  return d->profile;
}


bool Smb4KBookmark::equals(Smb4KBookmark *bookmark) const
{
  // URL
  if (d->url != bookmark->url())
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Workgroup
  if (QString::compare(d->workgroup, bookmark->workgroupName(), Qt::CaseInsensitive) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // IP address
  if (QString::compare(d->ip.toString(), bookmark->hostIpAddress()) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Share type
  if (d->type != bookmark->shareType())
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Label
  if (QString::compare(d->label, bookmark->label()) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Group
  if (QString::compare(d->group, bookmark->groupName()) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Profile
  if (QString::compare(d->profile, bookmark->profile()) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // The icon is not used here.

  return true;
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

