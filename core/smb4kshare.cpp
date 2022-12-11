/*
    Smb4K's container class for information about a share.

    SPDX-FileCopyrightText: 2008-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kshare.h"
#include "smb4kauthinfo.h"

// Qt include
#include <QDir>
#include <QUrl>

// KDE includes
#include "kiconthemes_version.h"
#include <KI18n/KLocalizedString>
#include <KIOCore/KIO/Global>
#include <KIOCore/KMountPoint>
#include <KIconThemes/KIconLoader>

class Smb4KSharePrivate
{
public:
    QString workgroup;
    QHostAddress ip;
    QString path;
    bool inaccessible;
    bool foreign;
    KUser user;
    KUserGroup group;
    qint64 totalSpace;
    qint64 freeSpace;
    bool mounted;
    QString filesystem;
    Smb4KGlobal::ShareType shareType;
};

Smb4KShare::Smb4KShare(const QUrl &url)
    : Smb4KBasicNetworkItem(Share)
    , d(new Smb4KSharePrivate)
{
    //
    // Set the private variables
    //
    d->inaccessible = false;
    d->foreign = false;
    d->filesystem = QString();
    d->user = KUser(KUser::UseRealUserID);
    d->group = KUserGroup(KUser::UseRealUserID);
    d->totalSpace = -1;
    d->freeSpace = -1;
    d->mounted = false;
    d->shareType = FileShare;

    //
    // Set the URL
    //
    *pUrl = url;

    //
    // Set the icon
    //
    setShareIcon();
}

Smb4KShare::Smb4KShare(const Smb4KShare &s)
    : Smb4KBasicNetworkItem(Share)
    , d(new Smb4KSharePrivate)
{
    //
    // Copy the private variables
    //
    *d = *s.d;

    //
    // Set the icon if necessary
    //
    if (pIcon->isNull()) {
        setShareIcon();
    }
}

Smb4KShare::Smb4KShare()
    : Smb4KBasicNetworkItem(Share)
    , d(new Smb4KSharePrivate)
{
    //
    // Set the private variables
    //
    d->inaccessible = false;
    d->foreign = false;
    d->filesystem = QString();
    d->user = KUser(KUser::UseRealUserID);
    d->group = KUserGroup(KUser::UseRealUserID);
    d->totalSpace = -1;
    d->freeSpace = -1;
    d->mounted = false;
    d->shareType = FileShare;

    //
    // Set the URL
    //
    pUrl->setScheme(QStringLiteral("smb"));

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
    if (name.startsWith(QStringLiteral("/"))) {
        pUrl->setPath(name.trimmed());
    } else {
        pUrl->setPath(QStringLiteral("/") + name.trimmed());
    }

    pUrl->setScheme(QStringLiteral("smb"));
}

QString Smb4KShare::shareName() const
{
    return pUrl->path().remove(QStringLiteral("/"));
}

void Smb4KShare::setHostName(const QString &hostName)
{
    pUrl->setHost(hostName.trimmed());
    pUrl->setScheme(QStringLiteral("smb"));
}

QString Smb4KShare::hostName() const
{
    return pUrl->host().toUpper();
}

QUrl Smb4KShare::homeUrl() const
{
    QUrl u;

    if (isHomesShare() && !pUrl->userName().isEmpty()) {
        u = *pUrl;
        u.setPath(QStringLiteral("/") + pUrl->userName(), QUrl::TolerantMode);
    }

    return u;
}

QString Smb4KShare::displayString(bool showHomesShare) const
{
    if (showHomesShare && isHomesShare()) {
        return i18n("%1 on %2", homeUrl().path().remove(QStringLiteral("/")), hostName());
    }

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

void Smb4KShare::setShareType(Smb4KGlobal::ShareType type)
{
    d->shareType = type;
    setShareIcon();
}

Smb4KGlobal::ShareType Smb4KShare::shareType() const
{
    return d->shareType;
}

QString Smb4KShare::shareTypeString() const
{
    QString typeString;

    switch (d->shareType) {
    case FileShare: {
        typeString = i18n("Disk");
        break;
    }
    case PrinterShare: {
        typeString = i18n("Printer");
        break;
    }
    case IpcShare: {
        typeString = i18n("IPC");
        break;
    }
    default: {
        break;
    }
    }

    return typeString;
}

void Smb4KShare::setHostIpAddress(const QString &ip)
{
    d->ip.setAddress(ip);
}

void Smb4KShare::setHostIpAddress(const QHostAddress &address)
{
    if (!address.isNull() && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) {
        d->ip = address;
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
    return pUrl->path().endsWith(QStringLiteral("$"));
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
    if (!path().isEmpty() && d->filesystem.isEmpty()) {
        KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath(path());

        if (mp) {
            d->filesystem = mp->mountType().toUpper();
        }
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
    if (!isPrinter()) {
        d->mounted = mounted;
        setShareIcon();
    }
}

bool Smb4KShare::isMounted() const
{
    return d->mounted;
}

void Smb4KShare::setTotalDiskSpace(qint64 size)
{
    d->totalSpace = size;
}

qint64 Smb4KShare::totalDiskSpace() const
{
    return d->totalSpace;
}

QString Smb4KShare::totalDiskSpaceString() const
{
    return KIO::convertSize(static_cast<quint64>(d->totalSpace));
}

void Smb4KShare::setFreeDiskSpace(qint64 size)
{
    d->freeSpace = size;
}

qint64 Smb4KShare::freeDiskSpace() const
{
    return d->freeSpace;
}

QString Smb4KShare::freeDiskSpaceString() const
{
    return KIO::convertSize(static_cast<quint64>(d->freeSpace));
}

qint64 Smb4KShare::usedDiskSpace() const
{
    return (d->totalSpace - d->freeSpace);
}

QString Smb4KShare::usedDiskSpaceString() const
{
    return KIO::convertSize(static_cast<quint64>(usedDiskSpace()));
}

qreal Smb4KShare::diskUsage() const
{
    qreal usage = 0;
    
    if (d->totalSpace > 0) {
      qint64 used = d->totalSpace - d->freeSpace;
      usage = static_cast<qreal>(used) * 100 / static_cast<qreal>(d->totalSpace);
    }
    
    return usage;
}

QString Smb4KShare::diskUsageString() const
{
    return QString::number(diskUsage(), 'f', 1) + QStringLiteral(" %");
}

void Smb4KShare::setMountData(Smb4KShare *share)
{
    Q_ASSERT(share);

    if (QString::compare(url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                         share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                         Qt::CaseInsensitive)
            == 0
        && (share->workgroupName().isEmpty() || QString::compare(workgroupName(), share->workgroupName(), Qt::CaseInsensitive) == 0)) {
        d->path = share->path();
        d->inaccessible = share->isInaccessible();
        d->foreign = share->isForeign();
        d->user = share->user();
        d->group = share->group();
        d->totalSpace = share->totalDiskSpace();
        d->freeSpace = share->freeDiskSpace();
        d->mounted = share->isMounted();
        d->shareType = share->shareType();
        setShareIcon();
    }
}

void Smb4KShare::resetMountData()
{
    d->path.clear();
    d->inaccessible = false;
    d->foreign = false;
    d->user = KUser(KUser::UseRealUserID);
    d->group = KUserGroup(KUser::UseRealUserID);
    d->totalSpace = -1;
    d->freeSpace = -1;
    d->mounted = false;
    d->shareType = FileShare;
    setShareIcon();
}

bool Smb4KShare::isHomesShare() const
{
    return pUrl->path().endsWith(QStringLiteral("homes"));
}

void Smb4KShare::setPort(int port)
{
    pUrl->setPort(port);
}

int Smb4KShare::port() const
{
    return pUrl->port();
}

void Smb4KShare::setUserName(const QString &name)
{
    // Avoid that the login is overwritten with an empty
    // string if we have a homes share.
    if (!isHomesShare() || !name.isEmpty()) {
        pUrl->setUserName(name);
    }
}

QString Smb4KShare::userName() const
{
    return pUrl->userName();
}

void Smb4KShare::setPassword(const QString &passwd)
{
    // Avoid that the password is overwritten with an empty
    // string if we have a homes share.
    if (!isHomesShare() || !passwd.isEmpty()) {
        pUrl->setPassword(passwd);
    }
}

QString Smb4KShare::password() const
{
    return pUrl->password();
}

void Smb4KShare::setShareIcon()
{
    if (!isPrinter()) {
        // Overlays
        QStringList overlays;

        if (isInaccessible()) {
            overlays << QStringLiteral("emblem-locked");
        } else if (isForeign()) {
            overlays << QStringLiteral("emblem-warning");
        } else if (isMounted()) {
            overlays << QStringLiteral("emblem-mounted");
        }

        *pIcon = KDE::icon(QStringLiteral("folder-network"), overlays);
    } else {
        *pIcon = KDE::icon(QStringLiteral("printer"));
    }
}

void Smb4KShare::update(Smb4KShare *share)
{
    if (QString::compare(workgroupName(), share->workgroupName(), Qt::CaseInsensitive) == 0
        && (QString::compare(url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                             share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                             Qt::CaseInsensitive)
                == 0
            || QString::compare(homeUrl().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                share->homeUrl().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                Qt::CaseInsensitive)
                == 0)) {
        *pUrl = share->url();
        setMountData(share);
        setShareType(share->shareType());
        setComment(share->comment());
        setHostIpAddress(share->hostIpAddress());
    }
}
