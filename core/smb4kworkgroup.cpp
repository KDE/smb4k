/*
    Smb4K's container class for information about a workgroup.

    SPDX-FileCopyrightText: 2008-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kworkgroup.h"
#include "smb4kglobal.h"

// Qt includes
#include <QAbstractSocket>
#include <QUrl>

// KDE includes
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

class Smb4KWorkgroupPrivate
{
public:
    QUrl masterURL;
    QHostAddress masterIP;
};

Smb4KWorkgroup::Smb4KWorkgroup(const QString &name)
    : Smb4KBasicNetworkItem(Workgroup)
    , d(new Smb4KWorkgroupPrivate)
{
    //
    // Set the URL of the workgroup
    //
    pUrl->setScheme(QStringLiteral("smb"));
    pUrl->setHost(name);

    //
    // Set the icon
    //
    *pIcon = KDE::icon(QStringLiteral("network-workgroup"));
}

Smb4KWorkgroup::Smb4KWorkgroup(const Smb4KWorkgroup &w)
    : Smb4KBasicNetworkItem(Workgroup)
    , d(new Smb4KWorkgroupPrivate)
{
    // Copy the private variables
    //
    *d = *w.d;

    //
    // Set the icon if necessary
    //
    if (pIcon->isNull()) {
        *pIcon = KDE::icon(QStringLiteral("network-workgroup"));
    }
}

Smb4KWorkgroup::Smb4KWorkgroup()
    : Smb4KBasicNetworkItem(Workgroup)
    , d(new Smb4KWorkgroupPrivate)
{
    //
    // Set the URL
    //
    pUrl->setScheme(QStringLiteral("smb"));

    //
    // Set the icon
    //
    *pIcon = KDE::icon(QStringLiteral("network-workgroup"));
}

Smb4KWorkgroup::~Smb4KWorkgroup()
{
}

void Smb4KWorkgroup::setWorkgroupName(const QString &name)
{
    pUrl->setHost(name);
    pUrl->setScheme(QStringLiteral("smb"));
}

QString Smb4KWorkgroup::workgroupName() const
{
    return pUrl->host().toUpper();
}

void Smb4KWorkgroup::setMasterBrowserName(const QString &name)
{
    d->masterURL.setHost(name);
    d->masterURL.setScheme(QStringLiteral("smb"));
}

QString Smb4KWorkgroup::masterBrowserName() const
{
    return d->masterURL.host().toUpper();
}

bool Smb4KWorkgroup::hasMasterBrowser() const
{
    return !d->masterURL.host().isEmpty();
}

void Smb4KWorkgroup::setMasterBrowserIpAddress(const QString &ip)
{
    d->masterIP.setAddress(ip);
}

void Smb4KWorkgroup::setMasterBrowserIpAddress(const QHostAddress &address)
{
    if (!address.isNull() && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) {
        d->masterIP = address;
    }
}

QString Smb4KWorkgroup::masterBrowserIpAddress() const
{
    return d->masterIP.toString();
}

bool Smb4KWorkgroup::hasMasterBrowserIpAddress() const
{
    return !d->masterIP.isNull();
}

void Smb4KWorkgroup::update(Smb4KWorkgroup *workgroup)
{
    if (QString::compare(workgroupName(), workgroup->workgroupName()) == 0) {
        setMasterBrowserName(workgroup->masterBrowserName());
        setMasterBrowserIpAddress(workgroup->masterBrowserIpAddress());
    }
}
