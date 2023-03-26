/*
    smb4kprofilesmenu  -  The menu for the profiles

    SPDX-FileCopyrightText: 2014-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kprofilesmenu.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QAction>
#include <QStringList>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

Smb4KProfilesMenu::Smb4KProfilesMenu(QObject *parent)
    : KSelectAction(KDE::icon(QStringLiteral("system-users")), i18n("Profiles"), parent)
{
    QStringList profiles = Smb4KProfileManager::self()->profilesList();
    slotProfilesListChanged(profiles);

    setToolBarMode(KSelectAction::MenuMode);

    //
    // Connections
    //
    connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)), this, SLOT(slotActiveProfileChanged(QString)));
    connect(Smb4KProfileManager::self(), SIGNAL(profilesListChanged(QStringList)), this, SLOT(slotProfilesListChanged(QStringList)));
    connect(Smb4KProfileManager::self(), SIGNAL(profileUsageChanged(bool)), this, SLOT(slotProfileUsageChanged(bool)));
    connect(this, SIGNAL(textTriggered(QString)), this, SLOT(slotActionTriggered(QString)));
}

Smb4KProfilesMenu::~Smb4KProfilesMenu()
{
}

void Smb4KProfilesMenu::refreshMenu()
{
    //
    // Clear the select action
    //
    clear();

    //
    // Get the list of profiles and add all profiles to the select action
    //
    QStringList profiles = Smb4KProfileManager::self()->profilesList();

    for (const QString &profile : qAsConst(profiles)) {
        QAction *action = addAction(profile);

        if (action) {
            action->setEnabled(Smb4KProfileManager::self()->useProfiles());
        }
    }

    //
    // Enable the action if the user chose to use profiles
    //
    setEnabled(Smb4KProfileManager::self()->useProfiles());

    //
    // Set the current action
    //
    setCurrentAction(Smb4KProfileManager::self()->activeProfile());
}

void Smb4KProfilesMenu::slotActiveProfileChanged(const QString &newProfile)
{
    setCurrentAction(newProfile);
}

void Smb4KProfilesMenu::slotProfilesListChanged(const QStringList & /*profiles*/)
{
    refreshMenu();
}

void Smb4KProfilesMenu::slotProfileUsageChanged(bool use)
{
    QList<QAction *> actionList = actions();

    for (QAction *action : qAsConst(actionList)) {
        if (action) {
            action->setEnabled(use);
        }
    }

    setEnabled(use);
}

void Smb4KProfilesMenu::slotActionTriggered(const QString &name)
{
    Smb4KProfileManager::self()->setActiveProfile(name);
}
