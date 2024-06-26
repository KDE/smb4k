/*
    Smb4K's container class for information about a host.

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4khost.h"

// Qt includes
#include <QDebug>
#include <QStringList>
#include <QUrl>

// KDE includes
#include <KIconLoader>

using namespace Smb4KGlobal;

class Smb4KHostPrivate
{
public:
    QString workgroup;
    QHostAddress ip;
    bool isMaster;
};

Smb4KHost::Smb4KHost(const QUrl &url)
    : Smb4KBasicNetworkItem(Host)
    , d(new Smb4KHostPrivate)
{
    d->isMaster = false;
    *pIcon = KDE::icon(QStringLiteral("network-server"));
    *pUrl = url;
}

Smb4KHost::Smb4KHost(const Smb4KHost &host)
    : Smb4KBasicNetworkItem(host)
    , d(new Smb4KHostPrivate)
{
    *d = *host.d;

    if (pIcon->isNull()) {
        *pIcon = KDE::icon(QStringLiteral("network-server"));
    }
}

Smb4KHost::Smb4KHost()
    : Smb4KBasicNetworkItem(Host)
    , d(new Smb4KHostPrivate)
{
    d->isMaster = false;
    *pIcon = KDE::icon(QStringLiteral("network-server"));
}

Smb4KHost::~Smb4KHost()
{
}

void Smb4KHost::setHostName(const QString &name)
{
    pUrl->setHost(name);
    pUrl->setScheme(QStringLiteral("smb"));
}

QString Smb4KHost::hostName() const
{
    return pUrl->host().toUpper();
}

void Smb4KHost::setWorkgroupName(const QString &workgroup)
{
    d->workgroup = workgroup.toUpper();
}

QString Smb4KHost::workgroupName() const
{
    return d->workgroup;
}

void Smb4KHost::setIpAddress(const QString &ip)
{
    d->ip.setAddress(ip);
}

void Smb4KHost::setIpAddress(const QHostAddress &address)
{
    if (!address.isNull() && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) {
        d->ip = address;
    }
}

QString Smb4KHost::ipAddress() const
{
    return d->ip.toString();
}

bool Smb4KHost::hasIpAddress() const
{
    return !d->ip.isNull();
}

void Smb4KHost::setIsMasterBrowser(bool master)
{
    d->isMaster = master;
}

bool Smb4KHost::isMasterBrowser() const
{
    return d->isMaster;
}

void Smb4KHost::setUserName(const QString &name)
{
    pUrl->setUserName(name);
}

QString Smb4KHost::userName() const
{
    return pUrl->userName();
}

void Smb4KHost::setPassword(const QString &passwd)
{
    pUrl->setPassword(passwd);
}

QString Smb4KHost::password() const
{
    return pUrl->password();
}

void Smb4KHost::setPort(int port)
{
    pUrl->setPort(port);
}

int Smb4KHost::port() const
{
    return pUrl->port();
}

void Smb4KHost::update(Smb4KHost *host)
{
    if (QString::compare(workgroupName(), host->workgroupName()) == 0 && QString::compare(hostName(), host->hostName()) == 0) {
        *pUrl = host->url();
        setComment(host->comment());
        setIsMasterBrowser(host->isMasterBrowser());

        // Do not kill the already discovered IP address
        if (!hasIpAddress() && host->hasIpAddress()) {
            setIpAddress(host->ipAddress());
        }
    }
}

Smb4KHost &Smb4KHost::operator=(const Smb4KHost &other)
{
    *d = *other.d;
    return *this;
}
