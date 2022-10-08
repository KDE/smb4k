/*
    This class derives from QObject and encapsulates a profile item/name.
    It is for use with QtQuick.

    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kprofileobject.h"

class Smb4KProfileObjectPrivate
{
public:
    QString profileName;
    bool activeProfile;
};

Smb4KProfileObject::Smb4KProfileObject(QObject *parent)
    : QObject(parent)
    , d(new Smb4KProfileObjectPrivate)
{
    d->profileName = QString();
    d->activeProfile = false;
}

Smb4KProfileObject::~Smb4KProfileObject()
{
}

QString Smb4KProfileObject::profileName() const
{
    return d->profileName;
}

void Smb4KProfileObject::setProfileName(const QString &profileName)
{
    if (d->profileName != profileName) {
        d->profileName = profileName;
        Q_EMIT changed();
    }
}

bool Smb4KProfileObject::isActiveProfile() const
{
    return d->activeProfile;
}

void Smb4KProfileObject::setActiveProfile(bool active)
{
    if (d->activeProfile != active) {
        d->activeProfile = active;
        Q_EMIT changed();
    }
}
