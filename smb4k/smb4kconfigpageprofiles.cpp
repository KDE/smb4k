/*
    The configuration page for the profiles

    SPDX-FileCopyrightText: 2014-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
#include <QVBoxLayout>

// KDE includes
#include <KCompletion/KLineEdit>
#include <KI18n/KLocalizedString>

Smb4KConfigPageProfiles::Smb4KConfigPageProfiles(QWidget *parent)
    : QWidget(parent)
{
    //
    // Layout
    //
    QVBoxLayout *layout = new QVBoxLayout(this);

    //
    // Profile Settings
    //
    QGroupBox *settingsBox = new QGroupBox(i18n("Settings"), this);
    QVBoxLayout *settingsBoxLayout = new QVBoxLayout(settingsBox);

    QCheckBox *useProfiles = new QCheckBox(Smb4KSettings::self()->useProfilesItem()->label(), settingsBox);
    useProfiles->setObjectName("kcfg_UseProfiles");

    settingsBoxLayout->addWidget(useProfiles);

    QCheckBox *useAssistant = new QCheckBox(Smb4KSettings::self()->useMigrationAssistantItem()->label(), settingsBox);
    useAssistant->setObjectName("kcfg_UseMigrationAssistant");

    settingsBoxLayout->addWidget(useAssistant);

    layout->addWidget(settingsBox);

    //
    // List of profiles
    //
    QGroupBox *profilesBox = new QGroupBox(i18n("Profiles"), this);
    QVBoxLayout *profilesBoxLayout = new QVBoxLayout(profilesBox);

    m_profiles = new KEditListWidget(profilesBox);
    m_profiles->setObjectName("kcfg_ProfilesList");
    m_profiles->setEnabled(Smb4KSettings::self()->useProfiles());

    profilesBoxLayout->addWidget(m_profiles);

    layout->addWidget(profilesBox);

    //
    // Connections
    //
    connect(useProfiles, SIGNAL(stateChanged(int)), this, SLOT(slotEnableWidget(int)));
    connect(m_profiles, SIGNAL(removed(QString)), this, SLOT(slotProfileRemoved(QString)));
    connect(m_profiles->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotProfileChanged()));
}

Smb4KConfigPageProfiles::~Smb4KConfigPageProfiles()
{
}

void Smb4KConfigPageProfiles::applyChanges()
{
    // Remove the profiles
    if (!m_removed.isEmpty()) {
        Smb4KProfileManager::self()->removeProfiles(m_removed);
        m_removed.clear();
    }

    // Rename the profiles
    if (!m_renamed.isEmpty()) {
        Smb4KProfileManager::self()->migrateProfiles(m_renamed);
        m_renamed.clear();
    }
}

void Smb4KConfigPageProfiles::slotEnableWidget(int state)
{
    switch (state) {
    case Qt::Unchecked: {
        m_profiles->setEnabled(false);
        break;
    }
    case Qt::Checked: {
        m_profiles->setEnabled(true);
        break;
    }
    default: {
        break;
    }
    }
}

void Smb4KConfigPageProfiles::slotProfileRemoved(const QString &name)
{
    // If the removed profile was renamed before, remove it from
    // the list.
    QMutableListIterator<QPair<QString, QString>> it(m_renamed);

    while (it.hasNext()) {
        QPair<QString, QString> entry = it.next();

        if (QString::compare(entry.first, name) == 0 || QString::compare(entry.second, name) == 0) {
            it.remove();
        }
    }

    m_removed << name;
}

void Smb4KConfigPageProfiles::slotProfileChanged()
{
    QStringList savedProfiles = Smb4KProfileManager::self()->profilesList();
    QStringList currentProfiles = m_profiles->items();

    if (savedProfiles.size() == currentProfiles.size()) {
        QMutableStringListIterator it(savedProfiles);

        while (it.hasNext()) {
            QString entry = it.next();
            int index = currentProfiles.indexOf(entry);

            if (index != -1) {
                currentProfiles.removeAt(index);
                it.remove();
            }
        }

        if (!savedProfiles.isEmpty() && !currentProfiles.isEmpty()) {
            // Take care that multiple renamings will have the correct
            // result.
            bool write = true;

            for (int i = 0; i < m_renamed.size(); ++i) {
                if (QString::compare(savedProfiles.first(), m_renamed.at(i).first, Qt::CaseSensitive) == 0) {
                    QPair<QString, QString> pair = static_cast<QPair<QString, QString>>(m_renamed.at(i));
                    pair.second = currentProfiles.first();
                    write = false;
                    break;
                }
            }

            // Write the renamed profile to the list, if necessary.
            if (write) {
                QPair<QString, QString> renamed(savedProfiles.first(), currentProfiles.first());
                m_renamed << renamed;
            }
        }
    }
}
