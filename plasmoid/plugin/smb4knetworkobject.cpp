/*
    This class derives from QObject and encapsulates the network items.
    It is for use with QtQuick.

    SPDX-FileCopyrightText: 2012-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knetworkobject.h"
#include "core/smb4kglobal.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kworkgroup.h"

// Qt includes
#include <QDebug>

using namespace Smb4KGlobal;

class Smb4KNetworkObjectPrivate
{
public:
    QString workgroup;
    QUrl url;
    int type;
    int parentType;
    QString comment;
    bool mounted;
    QUrl mountpoint;
    bool printer;
    bool isMaster;
    bool inaccessible;
};

Smb4KNetworkObject::Smb4KNetworkObject(Smb4KBasicNetworkItem *networkItem, QObject *parent)
    : QObject(parent)
    , d(new Smb4KNetworkObjectPrivate)
{
    switch (networkItem->type()) {
    case Smb4KGlobal::Workgroup: {
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>(networkItem);

        if (workgroup) {
            d->workgroup = workgroup->workgroupName();
            d->url = workgroup->url();
            d->mounted = false;
            d->inaccessible = false;
            d->printer = false;
            d->isMaster = false;
            setType(Workgroup);
        }

        break;
    }
    case Smb4KGlobal::Host: {
        Smb4KHost *host = static_cast<Smb4KHost *>(networkItem);

        if (host) {
            d->workgroup = host->workgroupName();
            d->url = host->url();
            d->comment = host->comment();
            d->mounted = false;
            d->inaccessible = false;
            d->printer = false;
            d->isMaster = host->isMasterBrowser();
            setType(Host);
        }

        break;
    }
    case Smb4KGlobal::Share: {
        Smb4KShare *share = static_cast<Smb4KShare *>(networkItem);

        if (share) {
            d->workgroup = share->workgroupName();
            d->url = share->url();
            d->comment = share->comment();
            d->mounted = share->isMounted();
            d->inaccessible = share->isInaccessible();
            d->printer = share->isPrinter();
            d->isMaster = false;
            d->mountpoint = QUrl::fromLocalFile(share->path());
            setType(Share);
        }

        break;
    }
    default: {
        if (networkItem->url().toString() == "smb://") {
            d->url = networkItem->url();
            d->mounted = false;
            d->inaccessible = false;
            d->printer = false;
            d->isMaster = false;
            setType(Network);
        } else {
            d->mounted = false;
            d->inaccessible = false;
            d->printer = false;
            d->isMaster = false;
            setType(Unknown);
        }
        break;
    }
    }
}

Smb4KNetworkObject::Smb4KNetworkObject(QObject *parent)
    : QObject(parent)
    , d(new Smb4KNetworkObjectPrivate)
{
    d->url.setUrl("smb://", QUrl::TolerantMode);
    d->mounted = false;
    d->inaccessible = false;
    d->printer = false;
    d->isMaster = false;
    setType(Network);
}

Smb4KNetworkObject::~Smb4KNetworkObject()
{
}

Smb4KNetworkObject::NetworkItem Smb4KNetworkObject::type() const
{
    return static_cast<Smb4KNetworkObject::NetworkItem>(d->type);
}

Smb4KNetworkObject::NetworkItem Smb4KNetworkObject::parentType() const
{
    return static_cast<Smb4KNetworkObject::NetworkItem>(d->parentType);
}

void Smb4KNetworkObject::setType(NetworkItem type)
{
    d->type = type;

    switch (type) {
    case Host: {
        d->parentType = Workgroup;
        break;
    }
    case Share: {
        d->parentType = Host;
        break;
    }
    default: {
        d->parentType = Network;
        break;
    }
    }
    emit changed();
}

QString Smb4KNetworkObject::workgroupName() const
{
    return d->workgroup;
}

void Smb4KNetworkObject::setWorkgroupName(const QString &name)
{
    d->workgroup = name;
    emit changed();
}

QString Smb4KNetworkObject::hostName() const
{
    return d->url.host().toUpper();
}

void Smb4KNetworkObject::setHostName(const QString &name)
{
    d->url.setHost(name);
    emit changed();
}

bool Smb4KNetworkObject::isMasterBrowser() const
{
    return d->isMaster;
}

void Smb4KNetworkObject::setMasterBrowser(bool master)
{
    if (type() == Host) {
        d->isMaster = master;
        emit changed();
    }
}

QString Smb4KNetworkObject::shareName() const
{
    // Since users might come up with very weird share names,
    // we are careful and do not use QString::remove("/"), but
    // only remove preceding and trailing slashes.
    QString share_name = d->url.path();

    if (share_name.startsWith('/')) {
        share_name.remove(0, 1);
    }

    if (share_name.endsWith('/')) {
        share_name.remove(share_name.size() - 1, 1);
    }

    return share_name;
}

void Smb4KNetworkObject::setShareName(const QString &name)
{
    d->url.setPath(name);
    emit changed();
}

QString Smb4KNetworkObject::name() const
{
    QString name;

    switch (d->type) {
    case Workgroup: {
        name = workgroupName();
        break;
    }
    case Host: {
        name = hostName();
        break;
    }
    case Share: {
        name = shareName();
        break;
    }
    default: {
        break;
    }
    }

    return name;
}

QString Smb4KNetworkObject::comment() const
{
    return d->comment;
}

void Smb4KNetworkObject::setComment(const QString &comment)
{
    d->comment = comment;
    emit changed();
}

QUrl Smb4KNetworkObject::url() const
{
    return d->url;
}

QUrl Smb4KNetworkObject::parentUrl() const
{
    // Do not use QUrl::upUrl() here, because it produces
    // an URL like this: smb://HOST/Share/../ and we do not
    // want that.
    QUrl parent_url;
    parent_url.setUrl("smb://");

    switch (d->type) {
    case Host: {
        parent_url.setHost(d->workgroup);
        break;
    }
    case Share: {
        parent_url.setHost(d->url.host());
        break;
    }
    default: {
        break;
    }
    }

    return parent_url;
}

void Smb4KNetworkObject::setUrl(const QUrl &url)
{
    d->url = url;
    emit changed();
}

bool Smb4KNetworkObject::isMounted() const
{
    return d->mounted;
}

void Smb4KNetworkObject::setMounted(bool mounted)
{
    d->mounted = mounted;
    emit changed();
}

void Smb4KNetworkObject::update(Smb4KBasicNetworkItem *networkItem)
{
    if (d->type == Workgroup && networkItem->type() == Smb4KGlobal::Workgroup) {
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>(networkItem);

        if (workgroup) {
            // Check that we update with the correct item.
            if (QString::compare(workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive) == 0) {
                d->workgroup = workgroup->workgroupName();
                d->url = workgroup->url();
                d->type = Workgroup;
                d->mounted = false;
                d->inaccessible = false;
                d->printer = false;
            }
        }
    } else if (d->type == Host && networkItem->type() == Smb4KGlobal::Host) {
        Smb4KHost *host = static_cast<Smb4KHost *>(networkItem);

        if (host) {
            // Check that we update with the correct item.
            if (QString::compare(workgroupName(), host->workgroupName(), Qt::CaseInsensitive) == 0
                && QString::compare(hostName(), host->hostName(), Qt::CaseInsensitive) == 0) {
                d->workgroup = host->workgroupName();
                d->url = host->url();
                d->comment = host->comment();
                d->type = Host;
                d->mounted = false;
                d->inaccessible = false;
                d->printer = false;
            }
        }
    } else if (d->type == Share && networkItem->type() == Smb4KGlobal::Share) {
        Smb4KShare *share = static_cast<Smb4KShare *>(networkItem);

        if (share) {
            // Check that we update with the correct item.
            if (QString::compare(workgroupName(), share->workgroupName(), Qt::CaseInsensitive) == 0
                && QString::compare(hostName(), share->hostName(), Qt::CaseInsensitive) == 0
                && QString::compare(shareName(), share->shareName(), Qt::CaseInsensitive) == 0) {
                d->workgroup = share->workgroupName();
                d->url = share->url();
                d->comment = share->comment();
                d->type = Share;
                d->mounted = share->isMounted();
                d->inaccessible = share->isInaccessible();
                d->printer = share->isPrinter();
                d->mountpoint.setUrl(share->path(), QUrl::TolerantMode);
                d->mountpoint.setScheme("file");
            }
        }
    } else {
        d->type = Network;
    }

    emit changed();
}

bool Smb4KNetworkObject::isPrinter() const
{
    return d->printer;
}

void Smb4KNetworkObject::setPrinter(bool printer)
{
    d->printer = printer;
    emit changed();
}

QUrl Smb4KNetworkObject::mountpoint() const
{
    return d->mountpoint;
}

void Smb4KNetworkObject::setMountpoint(const QUrl &mountpoint)
{
    d->mountpoint = mountpoint;
    emit changed();
}

bool Smb4KNetworkObject::isInaccessible() const
{
    return (d->mounted && d->inaccessible);
}

void Smb4KNetworkObject::setInaccessible(bool inaccessible)
{
    d->inaccessible = inaccessible;
}
