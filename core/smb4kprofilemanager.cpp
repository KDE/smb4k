/*
    This class manages the profiles that were defined by the user.

    SPDX-FileCopyrightText: 2014-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kprofilemanager.h"
#include "smb4kbookmarkhandler.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4khomesshareshandler.h"
#include "smb4ksettings.h"

// Qt includes
#include <QApplication>
#include <QPointer>

class Smb4KProfileManagerPrivate
{
public:
    QString activeProfile;
    QStringList profiles;
    bool useProfiles;
};

class Smb4KProfileManagerStatic
{
public:
    Smb4KProfileManager instance;
};

Q_GLOBAL_STATIC(Smb4KProfileManagerStatic, p);

Smb4KProfileManager::Smb4KProfileManager(QObject *parent)
    : QObject(parent)
    , d(new Smb4KProfileManagerPrivate)
{
    d->useProfiles = Smb4KSettings::useProfiles();

    if (d->useProfiles) {
        d->profiles = Smb4KSettings::profilesList();
        d->activeProfile = !Smb4KSettings::activeProfile().isEmpty() ? Smb4KSettings::activeProfile() : d->profiles.first();
    } else {
        d->profiles.clear();
        d->activeProfile.clear();
    }

    connect(Smb4KSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));
}

Smb4KProfileManager::~Smb4KProfileManager()
{
}

Smb4KProfileManager *Smb4KProfileManager::self()
{
    return &p->instance;
}

void Smb4KProfileManager::setActiveProfile(const QString &name)
{
    if (d->useProfiles) {
        if (name != d->activeProfile) {
            Q_EMIT aboutToChangeProfile();
            d->activeProfile = name;
            Smb4KSettings::setActiveProfile(d->activeProfile);
            Q_EMIT activeProfileChanged(d->activeProfile);
        }
    } else {
        if (!d->activeProfile.isEmpty()) {
            Q_EMIT aboutToChangeProfile();
            d->activeProfile.clear();
            Smb4KSettings::setActiveProfile(d->activeProfile);
            Q_EMIT activeProfileChanged(d->activeProfile);
        }
    }

    Smb4KSettings::self()->save();
}

QString Smb4KProfileManager::activeProfile() const
{
    return d->activeProfile;
}

QStringList Smb4KProfileManager::profilesList() const
{
    return d->useProfiles ? d->profiles : QStringList();
}

bool Smb4KProfileManager::useProfiles() const
{
    return d->useProfiles;
}

void Smb4KProfileManager::migrateProfile(const QString &oldName, const QString &newName)
{
    if (oldName == QStringLiteral("*")) {
        // All profiles to new profile name
        for (int i = 0; i < d->profiles.size(); i++) {
            QString tempOldName = d->profiles.at(i);
            d->profiles[i] = newName;
            Q_EMIT profileMigrated(tempOldName, newName);
        }
        setActiveProfile(newName);
    } else {
        // Old profile name to new profile name
        for (int i = 0; i < d->profiles.size(); i++) {
            if (d->profiles.at(i) == oldName) {
                d->profiles[i] = newName;
            }
        }

        Q_EMIT profileMigrated(oldName, newName);

        if (d->activeProfile == oldName) {
            setActiveProfile(newName);
        }
    }

    Smb4KSettings::setProfilesList(d->profiles);
    Q_EMIT profilesListChanged(d->profiles);
}

void Smb4KProfileManager::removeProfile(const QString &name)
{
    int index = d->profiles.indexOf(name);

    if (index != -1) {
        d->profiles.removeAt(index);
    }

    Q_EMIT profileRemoved(name);

    if (name == d->activeProfile) {
        setActiveProfile(!d->profiles.isEmpty() ? d->profiles.first() : QString());
    }

    // NOTE: Since this function is called BEFORE the new profiles list
    // is stored by the configuration dialog, we need to set the new list
    // here, because we are emitting the signal and we cannot be sure
    // that the user always uses Smb4KProfileManager::profilesList()
    // instead of Smb4KSettings::profilesList().
    Smb4KSettings::setProfilesList(d->profiles);

    Q_EMIT profilesListChanged(d->profiles);
}

void Smb4KProfileManager::slotConfigChanged()
{
    // FIXME: Do we need to emit the signals here?
    if (d->useProfiles != Smb4KSettings::useProfiles()) {
        d->useProfiles = Smb4KSettings::useProfiles();
        Q_EMIT profileUsageChanged(d->useProfiles);
    }

    if (d->profiles != Smb4KSettings::profilesList()) {
        d->profiles = Smb4KSettings::profilesList();
        Q_EMIT profilesListChanged(d->profiles);
    }

    if (!Smb4KSettings::activeProfile().isEmpty() && d->profiles.contains(Smb4KSettings::activeProfile())) {
        setActiveProfile(Smb4KSettings::activeProfile());
    } else {
        setActiveProfile(d->profiles.first());
    }
}
