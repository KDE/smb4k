/*
    Smb4K's container class for information about a directory or file.

    SPDX-FileCopyrightText: 2018-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

class Smb4KFilePrivate
{
public:
    QString workgroupName;
    QHostAddress ip;
};

Smb4KFile::Smb4KFile(const QUrl &url, Smb4KGlobal::NetworkItem type)
    : Smb4KBasicNetworkItem(type)
    , d(new Smb4KFilePrivate)
{
    *pUrl = url;

    if (type == Directory) {
        *pIcon = KDE::icon("folder");
    } else {
        *pIcon = KDE::icon(KIO::iconNameForUrl(url));
    }
}

Smb4KFile::Smb4KFile(const Smb4KFile &file)
    : Smb4KBasicNetworkItem(file.type())
    , d(new Smb4KFilePrivate)
{
    *d = *file.d;
}

Smb4KFile::~Smb4KFile()
{
}

void Smb4KFile::setWorkgroupName(const QString &name)
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

void Smb4KFile::setHostIpAddress(const QHostAddress &address)
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
    return pUrl->path().section('/', 1, 1);
}

void Smb4KFile::setUserName(const QString &name)
{
    pUrl->setUserName(name);
}

QString Smb4KFile::userName() const
{
    return pUrl->userName();
}

void Smb4KFile::setPassword(const QString &pass)
{
    pUrl->setPassword(pass);
}

QString Smb4KFile::password() const
{
    return pUrl->password();
}

bool Smb4KFile::isDirectory() const
{
    return (type() == Directory);
}

QString Smb4KFile::name() const
{
    QString name;

    switch (type()) {
    case Directory: {
        name = pUrl->path().section(QDir::separator(), -1, -1);
        break;
    }
    case File: {
        name = pUrl->fileName();
        break;
    }
    default: {
        break;
    }
    }

    return name;
}

bool Smb4KFile::isHidden() const
{
    return name().startsWith('.');
}
