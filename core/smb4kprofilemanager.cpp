/*
    This class manages the profiles that were defined by the user.

    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kprofilemanager.h"
#include "smb4kbookmarkhandler.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4khomesshareshandler.h"
#include "smb4kprofilemanager_p.h"
#include "smb4ksettings.h"

// Qt includes
#include <QApplication>
#include <QPointer>
#include <QTest>

Q_GLOBAL_STATIC(Smb4KProfileManagerStatic, p);

//
// NOTE: Do not invoke writeConfig() here, because this will/might
// trigger the configChanged() signal which can lead to unwanted
// effects.
//

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
    //
    // Check if the active profile is going to be changed. If so,
    // notify the program so that things can be done before the
    // profile is actually changed.
    //
    bool change = false;

    if (d->useProfiles) {
        if (name != d->activeProfile) {
            emit aboutToChangeProfile();
            change = true;
        }
    } else {
        if (!d->activeProfile.isEmpty()) {
            emit aboutToChangeProfile();
            change = true;
        }
    }

    //
    // Now change the profile
    //
    if (change) {
        d->activeProfile = d->useProfiles ? name : QString();
        Smb4KSettings::setActiveProfile(d->activeProfile);
        emit activeProfileChanged(d->activeProfile);
    }
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

void Smb4KProfileManager::migrateProfile(const QString &from, const QString &to)
{
    QList<QPair<QString, QString>> list;
    list << QPair<QString, QString>(from, to);
    migrateProfiles(list);
}

void Smb4KProfileManager::migrateProfiles(const QList<QPair<QString, QString>> &list)
{
    if (d->useProfiles || (list.size() == 1 && list.first().second.isEmpty())) {
        for (int i = 0; i < list.size(); ++i) {
            QString from = list.at(i).first;
            QString to = list.at(i).second;

            if (!to.isEmpty()) {
                // Migrate one/the default profile to another one.
                // First exchange the old profile.
                for (int j = 0; j < d->profiles.size(); ++j) {
                    if (QString::compare(from, d->profiles.at(j), Qt::CaseSensitive) == 0) {
                        d->profiles.replace(j, to);
                        break;
                    }
                }

                // Migrate profiles.
                Smb4KBookmarkHandler::self()->migrateProfile(from, to);
                Smb4KCustomOptionsManager::self()->migrateProfile(from, to);
                Smb4KHomesSharesHandler::self()->migrateProfile(from, to);
                emit migratedProfile(from, to);

                // In case the active profile was modified, rename it according
                // the value passed.
                if (QString::compare(from, d->activeProfile, Qt::CaseSensitive) == 0) {
                    setActiveProfile(to);
                }
            } else {
                // Migrate all profiles to the default one.
                for (int j = 0; j < d->profiles.size(); ++j) {
                    Smb4KBookmarkHandler::self()->migrateProfile(d->profiles.at(j), to);
                    Smb4KCustomOptionsManager::self()->migrateProfile(d->profiles.at(j), to);
                    Smb4KHomesSharesHandler::self()->migrateProfile(d->profiles.at(j), to);
                    emit migratedProfile(d->profiles.at(i), to);
                }
            }
        }

        Smb4KSettings::setProfilesList(d->profiles);
        emit profilesListChanged(d->profiles);
    }
}

void Smb4KProfileManager::removeProfile(const QString &name)
{
    QStringList list;
    list << name;
    removeProfiles(list);
}

void Smb4KProfileManager::removeProfiles(const QStringList &list)
{
    if (d->useProfiles) {
        for (int i = 0; i < list.size(); ++i) {
            QString name = list.at(i);

            // First remove the profile from the list.
            QMutableStringListIterator it(d->profiles);

            while (it.hasNext()) {
                QString entry = it.next();

                if (QString::compare(name, entry, Qt::CaseSensitive) == 0) {
                    it.remove();
                    break;
                }
            }

            if (!d->profiles.isEmpty()) {
                // Ask the user if he/she wants to migrate the entries
                // of the removed profile to another one.
                if (Smb4KSettings::useMigrationAssistant()) {
                    QPointer<Smb4KProfileMigrationDialog> dlg = new Smb4KProfileMigrationDialog(QStringList(name), d->profiles, QApplication::activeWindow());

                    if (dlg->exec() == QDialog::Accepted) {
                        migrateProfile(dlg->from(), dlg->to());
                    }

                    delete dlg;
                }
            }

            // Remove the profile.
            Smb4KBookmarkHandler::self()->removeProfile(name);
            Smb4KCustomOptionsManager::self()->removeProfile(name);
            Smb4KHomesSharesHandler::self()->removeProfile(name);
            emit removedProfile(name);

            // Set a new active profile if the user removed the current one.
            if (QString::compare(name, d->activeProfile, Qt::CaseSensitive) == 0) {
                setActiveProfile(!d->profiles.isEmpty() ? d->profiles.first() : QString());
            }
        }

        Smb4KSettings::setProfilesList(d->profiles);
        emit profilesListChanged(d->profiles);
    }
}

void Smb4KProfileManager::slotConfigChanged()
{
    bool usageChanged = false;

    //
    // Check if the usage of profiles changed
    //
    if (d->useProfiles != Smb4KSettings::useProfiles()) {
        d->useProfiles = Smb4KSettings::useProfiles();
        emit profileUsageChanged(d->useProfiles);
        usageChanged = true;
    }

    //
    // Updated the list of profiles
    //
    if (d->profiles != Smb4KSettings::profilesList()) {
        d->profiles = Smb4KSettings::profilesList();
        emit profilesListChanged(d->profiles);
    }

    //
    // Migrate profiles.
    // Profiles are only migrated, if the usage changed and the
    // user chose to use the migration assistant.
    //
    if (usageChanged && Smb4KSettings::useMigrationAssistant()) {
        QStringList from, to;

        if (d->useProfiles) {
            // Since the setting changed, the use of profiles was
            // switched off before. So, ask the user if he/she wants
            // to migrate the default profile to any other one.
            // Therefore, from needs to get one empty entry (default
            // profile) and to has to be d->profiles.
            from << QString();
            to << d->profiles;
        } else {
            // Here it is vice versa: Ask the user if he/she wants to
            // migrate all profiles to the default profile. Therefore,
            // set from to d->profiles and to to empty.
            from << d->profiles;
            to << QString();
        }

        // Now, launch the migration dialog.
        QPointer<Smb4KProfileMigrationDialog> dlg = new Smb4KProfileMigrationDialog(from, to, QApplication::activeWindow());

        if (dlg->exec() == QDialog::Accepted) {
            migrateProfile(dlg->from(), dlg->to());
        }

        delete dlg;
    }

    //
    // Set the active profile
    //
    if (!Smb4KSettings::activeProfile().isEmpty() && d->profiles.contains(Smb4KSettings::activeProfile())) {
        setActiveProfile(Smb4KSettings::activeProfile());
    } else {
        setActiveProfile(d->profiles.first());
    }
}
