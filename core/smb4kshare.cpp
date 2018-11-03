/***************************************************************************
    Smb4K's container class for information about a share.
                             -------------------
    begin                : Mo Jan 28 2008
    copyright            : (C) 2008-2017 by Alexander Reinholdt
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
#include "smb4kshare.h"
#include "smb4kauthinfo.h"

// Qt include
#include <QDir>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KIOCore/KIO/Global>
#include <KIconThemes/KIconLoader>
#include <KIOCore/KMountPoint>


class Smb4KSharePrivate
{
  public:
    QString workgroup;
    QString comment;
    QHostAddress ip;
    QString path;
    bool inaccessible;
    bool foreign;
    KUser user;
    KUserGroup group;
    qulonglong totalSpace;
    qulonglong freeSpace;
    qulonglong usedSpace;
    bool mounted;
    QString filesystem;
    Smb4KGlobal::NetworkItem shareType;
};


Smb4KShare::Smb4KShare(const QString &host, const QString &name)
: Smb4KBasicNetworkItem(Share), d(new Smb4KSharePrivate)
{
  d->inaccessible = false;
  d->foreign      = false;
  d->filesystem   = QString();
  d->user         = KUser(KUser::UseRealUserID);
  d->group        = KUserGroup(KUser::UseRealUserID);
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
  d->shareType    = FileShare;
  setHostName(host);
  setShareName(name);
  setShareIcon();
}


Smb4KShare::Smb4KShare(const QString &unc)
: Smb4KBasicNetworkItem(Share), d(new Smb4KSharePrivate)
{
  //
  // Set the private variables
  // 
  d->inaccessible = false;
  d->foreign      = false;
  d->filesystem   = QString();
  d->user         = KUser(KUser::UseRealUserID);
  d->group        = KUserGroup(KUser::UseRealUserID);
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
  d->shareType    = FileShare;
  
  //
  // Set the URL
  // 
  QUrl u;
  u.setUrl(unc, QUrl::TolerantMode);
  u.setScheme("smb");
  setUrl(u);
    
  //
  // Set the icon
  // 
  setShareIcon();
}


Smb4KShare::Smb4KShare(const Smb4KShare &s)
: Smb4KBasicNetworkItem(Share), d(new Smb4KSharePrivate)
{
  *d = *s.d;

  if (icon().isNull())
  {
    setShareIcon();
  }
  else
  {
    // Do nothing
  }
  
  if (url().isEmpty())
  {
    qWarning() << "URL of the share item is empty";
  }
  else
  {
    // Do nothing
  }
}



Smb4KShare::Smb4KShare()
: Smb4KBasicNetworkItem(Share), d(new Smb4KSharePrivate)
{
  //
  // Set the private variables
  // 
  d->inaccessible = false;
  d->foreign      = false;
  d->filesystem   = QString();
  d->user         = KUser(KUser::UseRealUserID);
  d->group        = KUserGroup(KUser::UseRealUserID);
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
  d->shareType    = FileShare;
  
  //
  // Set the URL
  //
  QUrl u;
  u.setScheme("smb");
  setUrl(u);
  
  //
  // Set the icon
  // 
  setShareIcon();
}


Smb4KShare::~Smb4KShare()
{
}


void Smb4KShare::setShareName(const QString &name)
{
  QUrl u = url();
  
  if (name.startsWith('/'))
  {
    u.setPath(name.trimmed());
  }
  else
  {
    u.setPath('/'+name.trimmed());
  }
  
  u.setScheme("smb");
  
  setUrl(u);
}


QString Smb4KShare::shareName() const
{
  if (url().path().startsWith('/'))
  {
    return url().path().remove(0, 1);
  }
  else
  {
    // Do nothing
  }

  return url().path();
}


void Smb4KShare::setHostName(const QString &hostName)
{
  QUrl u = url();
  u.setHost(hostName.trimmed());
  u.setScheme("smb");
  setUrl(u);
}


QString Smb4KShare::hostName() const
{
  return url().host().toUpper();
}


QString Smb4KShare::unc() const
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


QString Smb4KShare::homeUNC() const
{
  QString unc;

  if (isHomesShare() && !hostName().isEmpty() && !url().userName().isEmpty())
  {
    unc = QString("//%1/%2").arg(hostName()).arg(url().userName());
  }
  else
  {
    // Do nothing
  }
  
  return unc;
}


QUrl Smb4KShare::homeUrl() const
{
  QUrl u;
  
  if (isHomesShare() && !url().userName().isEmpty())
  {
    u = url();
    u.setPath('/'+url().userName(), QUrl::TolerantMode);
  }
  else
  {
    // Do nothing
  }
  
  return u;
}


QString Smb4KShare::hostUNC() const
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


QString Smb4KShare::displayString() const
{
  return i18n("%1 on %2", shareName(), hostName());
}


void Smb4KShare::setWorkgroupName(const QString &workgroup)
{
  d->workgroup = workgroup;
}


QString Smb4KShare::workgroupName() const
{
  return d->workgroup;
}


void Smb4KShare::setShareType(Smb4KGlobal::NetworkItem type)
{
  d->shareType = type;
  setShareIcon();
}


Smb4KGlobal::NetworkItem Smb4KShare::shareType() const
{
  return d->shareType;
}


QString Smb4KShare::shareTypeString() const
{
  QString typeString;
  
  switch (d->shareType)
  {
    case FileShare:
    {
      typeString = i18n("Disk");
      break;
    }
    case PrinterShare:
    {
      typeString = i18n("Printer");
      break;
    }
    case IpcShare:
    {
      typeString = i18n("IPC");
      break;
    }
    default:
    {
      break;
    }
  }
  
  return typeString;
}


void Smb4KShare::setComment(const QString &comment)
{
  d->comment = comment;
}


QString Smb4KShare::comment() const
{
  return d->comment;
}


void Smb4KShare::setHostIpAddress(const QString &ip)
{
  d->ip.setAddress(ip);
}


void Smb4KShare::setHostIpAddress(const QHostAddress& address)
{
  if (!address.isNull() && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
  {
    d->ip = address;
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KShare::hostIpAddress() const
{
  return d->ip.toString();
}


bool Smb4KShare::hasHostIpAddress() const
{
  return !d->ip.isNull();
}


bool Smb4KShare::isHidden() const
{
  return url().path().endsWith('$');
}


bool Smb4KShare::isPrinter() const
{
  return (d->shareType == PrinterShare);
}


void Smb4KShare::setPath(const QString &mountpoint)
{
  d->path = mountpoint;
}


QString Smb4KShare::path() const
{
  return d->path;
}


QString Smb4KShare::canonicalPath() const
{
  return (d->inaccessible ? d->path : QDir(d->path).canonicalPath());
}


void Smb4KShare::setInaccessible(bool in)
{
  d->inaccessible = in;
  setShareIcon();
}


bool Smb4KShare::isInaccessible() const
{
  return (d->mounted && d->inaccessible);
}


void Smb4KShare::setForeign(bool foreign)
{
  d->foreign = foreign;
  setShareIcon();
}


bool Smb4KShare::isForeign() const
{
  return (d->mounted && d->foreign);
}


QString Smb4KShare::fileSystemString() const
{
  if (!path().isEmpty() && d->filesystem.isEmpty())
  {
    KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath(path());
    d->filesystem = mp->mountType().toUpper();
  }
  else
  {
    // Do nothing
  }
  
  return d->filesystem;
}


void Smb4KShare::setUser(const KUser &user)
{
  d->user = user;
}


KUser Smb4KShare::user() const
{
  return d->user;
}


void Smb4KShare::setGroup(const KUserGroup &group)
{
  d->group = group;
}


KUserGroup Smb4KShare::group() const
{
  return d->group;
}


void Smb4KShare::setMounted(bool mounted)
{
  if (!isPrinter())
  {
    d->mounted = mounted;
    setShareIcon();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KShare::isMounted() const
{
  return d->mounted;
}


void Smb4KShare::setTotalDiskSpace(qulonglong size)
{
  d->totalSpace = size;
}


qulonglong Smb4KShare::totalDiskSpace() const
{
  return d->totalSpace;
}


QString Smb4KShare::totalDiskSpaceString() const
{
  return KIO::convertSize(d->totalSpace);
}


void Smb4KShare::setFreeDiskSpace(qulonglong size)
{
  d->freeSpace = size;
}


qulonglong Smb4KShare::freeDiskSpace() const
{
  return d->freeSpace;
}


QString Smb4KShare::freeDiskSpaceString() const
{
  return KIO::convertSize(d->freeSpace);
}


void Smb4KShare::setUsedDiskSpace(qulonglong size)
{
  d->usedSpace = size;
}


qulonglong Smb4KShare::usedDiskSpace() const
{
  return d->usedSpace;
}


QString Smb4KShare::usedDiskSpaceString() const
{
  return KIO::convertSize(d->usedSpace);
}


qreal Smb4KShare::diskUsage() const
{
  qreal used(usedDiskSpace());
  qreal total(totalDiskSpace());

  if (total > 0)
  {
    return used * 100 / total;
  }
  
  return 0;
}


QString Smb4KShare::diskUsageString() const
{
  return QString("%1 %").arg(diskUsage(), 0, 'f', 1);
}


bool Smb4KShare::equals(Smb4KShare *share, CheckFlags flag) const
{
  Q_ASSERT(share);

  switch (flag)
  {
    case Full:
    {
      // UNC
      if (QString::compare(unc(), share->unc(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Login
      if (QString::compare(login(), share->login()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Workgroup name
      if (QString::compare(workgroupName(), share->workgroupName(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Type
      if (d->shareType != share->shareType())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Comment
      if (QString::compare(comment(), share->comment()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // IP address
      if (QString::compare(hostIpAddress(), share->hostIpAddress()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Path
      if (QString::compare(path(), share->path()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Accessibility?
      if (isInaccessible() != share->isInaccessible())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Foreignness
      if (isForeign() != share->isForeign())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // UID
      if (user().userId().nativeId() != share->user().userId().nativeId())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // GID
      if (group().groupId().nativeId() != share->group().groupId().nativeId())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Total space
      if (totalDiskSpace() != share->totalDiskSpace())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Used space
      if (usedDiskSpace() != share->usedDiskSpace())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Mounted
      if (isMounted() != share->isMounted())
      {
        return false;
      }
      else
      {
        // Do nothing
      }
        
      break;
    }
    case NetworkOnly:
    {
      // UNC
      if (QString::compare(unc(), share->unc(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Login
      if (QString::compare(login(), share->login()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Workgroup name
      if (QString::compare(workgroupName(), share->workgroupName(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Type
      if (d->shareType != share->shareType())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Comment
      if (QString::compare(comment(), share->comment()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // IP address
      if (QString::compare(hostIpAddress(), share->hostIpAddress()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      break;
    }
    case MinimalNetworkOnly:
    {
      // UNC
      if (QString::compare(unc(), share->unc(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Workgroup name
      if (QString::compare(workgroupName(), share->workgroupName(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Type
      if (d->shareType != share->shareType())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case LocalOnly:
    {
      // UNC
      if (QString::compare(unc(), share->unc(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Login
      if (QString::compare(login(), share->login()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      // Path
      if (QString::compare(path(), share->path()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Accessibility?
      if (isInaccessible() != share->isInaccessible())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Foreignness
      if (isForeign() != share->isForeign())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // UID
      if (user().userId().nativeId() != share->user().userId().nativeId())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // GID
      if (group().groupId().nativeId() != share->group().groupId().nativeId())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Total space
      if (totalDiskSpace() != share->totalDiskSpace())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Used space
      if (usedDiskSpace() != share->usedDiskSpace())
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Mounted
      if (isMounted() != share->isMounted())
      {
        return false;
      }
      else
      {
        // Do nothing
      }
      
      break;
    }
    case MinimalLocalOnly:
    {
      // UNC
      if (QString::compare(unc(), share->unc(), Qt::CaseInsensitive) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      // Path
      if (QString::compare(path(), share->path()) != 0)
      {
        return false;
      }
      else
      {
        // Do nothing
      }

      break;
    }    
    default:
    {
      break;
    }
  }

  return true;
}


bool Smb4KShare::isEmpty(CheckFlags flag) const
{
  switch (flag)
  {
    case Full:
    {
      if (!url().isEmpty())
      {
        return false;
      }

      if (!d->workgroup.isEmpty())
      {
        return false;
      }

      if (!d->comment.isEmpty())
      {
        return false;
      }

      if (!d->ip.isNull())
      {
        return false;
      }

      if (!d->path.isEmpty())
      {
        return false;
      }

      if (!d->filesystem.isEmpty())
      {
        return false;
      }

      if (d->totalSpace > 0)
      {
        return false;
      }

      if (d->freeSpace > 0)
      {
        return false;
      }
      
      if (d->usedSpace > 0)
      {
        return false;
      }

      break;
    }
    case NetworkOnly:
    {
      if (!url().isEmpty())
      {
        return false;
      }

      if (!d->workgroup.isEmpty())
      {
        return false;
      }

      if (!d->comment.isEmpty())
      {
        return false;
      }

      if (!d->ip.isNull())
      {
        return false;
      }

      break;
    }
    case LocalOnly:
    {
      if (!d->path.isEmpty())
      {
        return false;
      }

      if (!d->filesystem.isEmpty())
      {
        return false;
      }

      if (d->totalSpace > 0)
      {
        return false;
      }

      if (d->freeSpace > 0)
      {
        return false;
      }
      
      if (d->usedSpace > 0)
      {
        return false;
      }

      break;
    }
    default:
    {
      break;
    }
  }

  return true;
}


void Smb4KShare::setMountData(Smb4KShare *share)
{
  Q_ASSERT(share);

  if (equals(share, MinimalNetworkOnly))
  {
    d->path         = share->path();
    d->inaccessible = share->isInaccessible();
    d->foreign      = share->isForeign();
    d->user         = share->user();
    d->group        = share->group();
    d->totalSpace   = share->totalDiskSpace();
    d->freeSpace    = share->freeDiskSpace();
    d->usedSpace    = share->usedDiskSpace();
    d->mounted      = share->isMounted();
    d->shareType    = share->shareType();
    setShareIcon();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::resetMountData()
{
  d->path.clear();
  d->inaccessible = false;
  d->foreign      = false;
  d->user         = KUser(KUser::UseRealUserID);
  d->group        = KUserGroup(KUser::UseRealUserID);
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
  d->shareType    = FileShare;
  setShareIcon();
}


bool Smb4KShare::isHomesShare() const
{
  return url().path().endsWith(QLatin1String("homes"));
}


void Smb4KShare::setPort(int port)
{
  QUrl u = url();
  u.setPort(port);
  setUrl(u);
}


int Smb4KShare::port() const
{
  return url().port();
}


void Smb4KShare::setAuthInfo(Smb4KAuthInfo *authInfo)
{
  // Avoid that the login is overwritten with an empty 
  // string if we have a homes share.
  if (!isHomesShare() || !authInfo->userName().isEmpty())
  {
    QUrl u = url();
    u.setUserName(authInfo->userName());
    u.setPassword(authInfo->password());
    setUrl(u);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::setLogin(const QString &login)
{
  // Avoid that the login is overwritten with an empty 
  // string if we have a homes share.
  if (!isHomesShare() || !login.isEmpty())
  {
    QUrl u = url();
    u.setUserName(login);
    setUrl(u);
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KShare::login() const
{
  return url().userName();
}


void Smb4KShare::setPassword(const QString &passwd)
{
  // Avoid that the password is overwritten with an empty 
  // string if we have a homes share.
  if (!isHomesShare() || !passwd.isEmpty())
  {
    QUrl u = url();
    u.setPassword(passwd);
    setUrl(u);
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KShare::password() const
{
  return url().password();
}


void Smb4KShare::setShareIcon()
{
  // We have three base icons: The remote folder, the locked folder
  // and the printer icon.
  if (!isPrinter())
  {
    // Overlays
    QStringList overlays;
    
    if (isMounted())
    {
      overlays << "emblem-mounted";
    }
    else
    {
      overlays << "";
    }

    if (isForeign())
    {
      overlays << "task-attention";
    }
    else
    {
      // Do nothing
    }

    // The icon
    QIcon icon;
    
    if (!isInaccessible())
    {
      icon = KDE::icon("folder-network", overlays);
    }
    else
    {
      icon = KDE::icon("folder-locked", overlays);
    }
    
    setIcon(icon);
  }
  else
  {
    setIcon(KDE::icon("printer"));
  }
}


void Smb4KShare::update(Smb4KShare* share)
{
  if (QString::compare(workgroupName(), share->workgroupName()) == 0 &&
      (QString::compare(unc(), share->unc()) == 0 || QString::compare(homeUNC(), share->homeUNC()) == 0))
  {
    setMountData(share);
    setUrl(share->url());
    setShareType(share->shareType());
    setComment(share->comment());
    setHostIpAddress(share->hostIpAddress());
  }
  else
  {
    // Do nothing
  }
}

