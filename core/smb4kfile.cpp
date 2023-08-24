/*
    Smb4K's container class for information about a directory or file.

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kfile.h"
#include "smb4kglobal.h"

// Qt includes
#include <QDebug>
#include <QDir>

// KDE includes
#include <KIO/Global>
#include <KIconLoader>

using namespace Smb4KGlobal;

class Smb4KFilePrivate
{
public:
    QString workgroupName;
    QHostAddress ip;
    bool isDirectory;
};

Smb4KFile::Smb4KFile(const QUrl &url)
    : Smb4KBasicNetworkItem(Smb4KGlobal::FileOrDirectory)
    , d(new Smb4KFilePrivate)
{
    *pUrl = url;
    *pIcon = KDE::icon(KIO::iconNameForUrl(url));
    d->isDirectory = false;
}

Smb4KFile::Smb4KFile(const Smb4KFile &file)
    : Smb4KBasicNetworkItem(file)
    , d(new Smb4KFilePrivate)
{
    *d = *file.d;
}

Smb4KFile::Smb4KFile()
    : Smb4KBasicNetworkItem(Smb4KGlobal::FileOrDirectory)
    , d(new Smb4KFilePrivate)
{
    d->isDirectory = false;
}

Smb4KFile::~Smb4KFile()
{
}

void Smb4KFile::setWorkgroupName(const QString &name) const
{
    d->workgroupName = name;
}

QString Smb4KFile::workgroupName() const
{
    return d->workgroupName;
}

QString Smb4KFile::hostName() const
{
    return pUrl->host().toUpper();
}

void Smb4KFile::setHostIpAddress(const QHostAddress &address) const
{
    if (!address.isNull() && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) {
        d->ip = address;
    }
}

QString Smb4KFile::hostIpAddress() const
{
    return d->ip.toString();
}

bool Smb4KFile::hasHostIpAddress() const
{
    return !d->ip.isNull();
}

QString Smb4KFile::shareName() const
{
    return pUrl->path().section(QStringLiteral("/"), 1, 1);
}

void Smb4KFile::setUserName(const QString &name) const
{
    pUrl->setUserName(name);
}

QString Smb4KFile::userName() const
{
    return pUrl->userName();
}

void Smb4KFile::setPassword(const QString &pass) const
{
    pUrl->setPassword(pass);
}

QString Smb4KFile::password() const
{
    return pUrl->password();
}

void Smb4KFile::setDirectory(bool directory) const
{
    d->isDirectory = directory;
    *pIcon = KDE::icon(QStringLiteral("folder"));
}

bool Smb4KFile::isDirectory() const
{
    return d->isDirectory;
}

QString Smb4KFile::name() const
{
    QString name;

    if (d->isDirectory) {
        name = pUrl->path().section(QDir::separator(), -1, -1);
    } else {
        name = pUrl->fileName();
    }

    return name;
}

bool Smb4KFile::isHidden() const
{
    return name().startsWith(QStringLiteral("."));
}

Smb4KFile &Smb4KFile::operator=(const Smb4KFile &other)
{
    *d = *other.d;
    return *this;
}
