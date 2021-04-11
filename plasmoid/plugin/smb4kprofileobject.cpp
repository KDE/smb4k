/***************************************************************************
    This class derives from QObject and encapsulates a profile item/name.
    It is for use with QtQuick.
                             -------------------
    begin                : So 23 Nov 2014
    copyright            : (C) 2014-2021 by Alexander Reinholdt
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
    d->profileName = profileName;
    emit changed();
}

bool Smb4KProfileObject::isActiveProfile() const
{
    return d->activeProfile;
}

void Smb4KProfileObject::setActiveProfile(bool active)
{
    d->activeProfile = active;
    emit changed();
}
