/*
    The configuration page for the profiles

    SPDX-FileCopyrightText: 2014-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpageprofiles.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>

// KDE includes
#include <KLineEdit>
#include <KLocalizedString>

struct ProfileContainer {
    QString initialName;
    QString currentName;
    bool removed;
    bool renamed;
    bool added;
};

Smb4KConfigPageProfiles::Smb4KConfigPageProfiles(QWidget *parent)
    : QWidget(parent)
{
    m_profilesChanged = false;

    QStringList profiles = Smb4KSettings::profilesList();

    for (const QString &profile : qAsConst(profiles)) {
        ProfileContainer p;
        p.initialName = profile;
        p.currentName = profile;
        p.removed = false;
        p.renamed = false;
        p.added = false;

        m_profiles << p;
    }

    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox *settingsBox = new QGroupBox(i18n("Settings"), this);
    QVBoxLayout *settingsBoxLayout = new QVBoxLayout(settingsBox);

    m_useProfiles = new QCheckBox(Smb4KSettings::self()->useProfilesItem()->label(), settingsBox);
    m_useProfiles->setObjectName(QStringLiteral("kcfg_UseProfiles"));

    settingsBoxLayout->addWidget(m_useProfiles);

    m_transferToFirstProfile = new QCheckBox(Smb4KSettings::self()->transferToFirstProfileItem()->label(), settingsBox);
    m_transferToFirstProfile->setObjectName(QStringLiteral("kcfg_TransferToFirstProfile"));

    settingsBoxLayout->addWidget(m_transferToFirstProfile);

    m_makeAllDataAvailable = new QCheckBox(Smb4KSettings::self()->makeAllDataAvailableItem()->label(), settingsBox);
    m_makeAllDataAvailable->setObjectName(QStringLiteral("kcfg_MakeAllDataAvailable"));

    settingsBoxLayout->addWidget(m_makeAllDataAvailable);

    layout->addWidget(settingsBox);

    QGroupBox *profilesBox = new QGroupBox(i18n("Profiles"), this);
    QVBoxLayout *profilesBoxLayout = new QVBoxLayout(profilesBox);

    m_profilesWidget = new KEditListWidget(profilesBox);
    m_profilesWidget->setObjectName(QStringLiteral("kcfg_ProfilesList"));
    m_profilesWidget->setEnabled(Smb4KSettings::self()->useProfiles());

    profilesBoxLayout->addWidget(m_profilesWidget);

    layout->addWidget(profilesBox);

    connect(m_useProfiles, &QCheckBox::toggled, this, &Smb4KConfigPageProfiles::slotProfileUsageChanged);
    connect(m_profilesWidget, &KEditListWidget::added, this, &Smb4KConfigPageProfiles::slotProfileAdded);
    connect(m_profilesWidget, &KEditListWidget::removed, this, &Smb4KConfigPageProfiles::slotProfileRemoved);
    connect(m_profilesWidget->lineEdit(), &QLineEdit::editingFinished, this, &Smb4KConfigPageProfiles::slotProfileChanged);
}

Smb4KConfigPageProfiles::~Smb4KConfigPageProfiles()
{
}

void Smb4KConfigPageProfiles::applyChanges()
{
    if (m_profilesChanged) {
        QMutableListIterator<ProfileContainer> it(m_profiles);

        while (it.hasNext()) {
            ProfileContainer p = it.next();

            if (!p.removed && !p.renamed && !p.added) {
                continue;
            }

            if (p.removed) {
                Smb4KProfileManager::self()->removeProfile(p.initialName);
                it.remove();
            }

            if (p.renamed && !p.added) {
                // When we just rename a profile, do not use the migration dialog
                Smb4KProfileManager::self()->migrateProfile(p.initialName, p.currentName);
                it.value().initialName = p.currentName;
                it.value().renamed = false;
            }

            if (p.added) {
                // We do not need to do anything here, because the new profile
                // will be saved by KConfig XT.
                it.value().initialName = p.currentName;
                it.value().added = false;
                it.value().renamed = false;
            }
        }

        // Migrate all data from the default profile to the first profile in the list
        if (m_useProfiles->isChecked() && m_transferToFirstProfile->isChecked() && m_useProfiles->isChecked() != Smb4KSettings::useProfiles()) {
            QString firstProfile = m_profilesWidget->items().first();
            Smb4KProfileManager::self()->migrateProfile(QStringLiteral(""), firstProfile);
        }

        // Migrate all data from all profiles to the default profile
        if (!m_useProfiles->isChecked() && m_makeAllDataAvailable->isChecked() && m_useProfiles->isChecked() != Smb4KSettings::useProfiles()) {
            Smb4KProfileManager::self()->migrateProfile(QStringLiteral("*"), QStringLiteral(""));
        }

        m_profilesChanged = false;
    }
}

bool Smb4KConfigPageProfiles::profilesChanged() const
{
    return m_profilesChanged;
}

void Smb4KConfigPageProfiles::slotProfileUsageChanged(bool checked)
{
    m_profilesChanged = (checked != Smb4KSettings::useProfiles());
    m_profilesWidget->setEnabled(checked);
}

void Smb4KConfigPageProfiles::slotProfileAdded(const QString &text)
{
    Q_UNUSED(text);

    ProfileContainer p;
    p.initialName = text;
    p.currentName = text;
    p.removed = false;
    p.renamed = false;
    p.added = true;

    m_profiles << p;

    m_profilesChanged = true;
}

void Smb4KConfigPageProfiles::slotProfileRemoved(const QString &text)
{
    for (int i = 0; i < m_profiles.size(); i++) {
        if (m_profiles.at(i).initialName == text || m_profiles.at(i).currentName == text) {
            m_profiles[i].removed = true;
            break;
        }
    }

    m_profilesChanged = true;
}

void Smb4KConfigPageProfiles::slotProfileChanged()
{
    QStringList listedProfiles = m_profilesWidget->items();
    int renamedIndex = -1;

    for (int i = 0; i < m_profiles.size(); i++) {
        if (!m_profiles.at(i).removed) {
            int index = listedProfiles.indexOf(m_profiles.at(i).currentName);

            if (index == -1) {
                renamedIndex = i;
                break;
            } else {
                listedProfiles.removeAt(index);
            }
        }
    }

    if (renamedIndex != -1) {
        QString newName = listedProfiles.first();
        m_profiles[renamedIndex].currentName = newName;
        m_profiles[renamedIndex].renamed = true;

        m_profilesChanged = true;
    }
}
