/*
    The configuration page for the profiles

    SPDX-FileCopyrightText: 2014-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QMouseEvent>

// KDE includes
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
    m_currentProfileContainer = nullptr;

    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox *settingsBox = new QGroupBox(i18n("Settings"), this);
    QVBoxLayout *settingsBoxLayout = new QVBoxLayout(settingsBox);

    m_useProfiles = new QCheckBox(Smb4KSettings::self()->useProfilesItem()->label(), settingsBox);
    m_useProfiles->setObjectName(QStringLiteral("kcfg_UseProfiles"));

    settingsBoxLayout->addWidget(m_useProfiles);

    layout->addWidget(settingsBox);

    QGroupBox *profilesBox = new QGroupBox(i18n("Profiles"), this);
    QVBoxLayout *profilesBoxLayout = new QVBoxLayout(profilesBox);
    profilesBoxLayout->setContentsMargins(0, 0, 0, 0);

    m_profilesEditorWidget = new QWidget(profilesBox);
    m_profilesEditorWidget->setEnabled(Smb4KSettings::self()->useProfiles());

    QGridLayout *profilesEditorWidgetLayout = new QGridLayout(m_profilesEditorWidget);

    m_profilesInputLineEdit = new KLineEdit(m_profilesEditorWidget);
    m_profilesInputLineEdit->setClearButtonEnabled(true);

    profilesEditorWidgetLayout->addWidget(m_profilesInputLineEdit, 0, 0);

    m_profilesListWidget = new QListWidget(m_profilesEditorWidget);
    m_profilesListWidget->viewport()->installEventFilter(this);

    profilesEditorWidgetLayout->addWidget(m_profilesListWidget, 1, 0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, m_profilesEditorWidget);

    m_addButton = new QPushButton(KDE::icon(QStringLiteral("list-add")), i18n("Add"), buttonBox);
    buttonBox->addButton(m_addButton, QDialogButtonBox::ActionRole);

    m_editButton = new QPushButton(KDE::icon(QStringLiteral("edit-rename")), i18n("Edit"), buttonBox);
    m_editButton->setEnabled(false);
    buttonBox->addButton(m_editButton, QDialogButtonBox::ActionRole);

    m_removeButton = new QPushButton(KDE::icon(QStringLiteral("list-remove")), i18n("Remove"), buttonBox);
    m_removeButton->setEnabled(false);
    buttonBox->addButton(m_removeButton, QDialogButtonBox::ActionRole);

    profilesEditorWidgetLayout->addWidget(buttonBox, 0, 1, 2, 1);

    profilesBoxLayout->addWidget(m_profilesEditorWidget);

    layout->addWidget(profilesBox);

    QStringList profiles = Smb4KSettings::profilesList();

    for (const QString &profile : qAsConst(profiles)) {
        ProfileContainer p;
        p.initialName = profile;
        p.currentName = profile;
        p.removed = false;
        p.renamed = false;
        p.added = false;

        m_profiles << p;

        QListWidgetItem *profileItem = new QListWidgetItem(profile, m_profilesListWidget);
        profileItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
    }

    connect(m_useProfiles, &QCheckBox::toggled, this, &Smb4KConfigPageProfiles::slotProfileUsageChanged);
    connect(m_addButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotAddProfile);
    connect(m_editButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotEditProfile);
    connect(m_removeButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotRemoveProfile);

    connect(m_profilesListWidget, &QListWidget::itemSelectionChanged, this, &Smb4KConfigPageProfiles::slotEnableButtons);
    connect(m_profilesListWidget, &QListWidget::itemDoubleClicked, this, &Smb4KConfigPageProfiles::slotProfileDoubleClicked);
    connect(m_profilesListWidget, &QListWidget::itemChanged, this, &Smb4KConfigPageProfiles::slotProfileChanged);

    // connect(m_profilesInputLineEdit, &QLineEdit::editingFinished, this, &Smb4KConfigPageProfiles::slotProfileChanged);
}

Smb4KConfigPageProfiles::~Smb4KConfigPageProfiles()
{
}

void Smb4KConfigPageProfiles::applyChanges()
{
    if (m_profilesChanged) {
        QStringList profiles;

        for (int i = 0; i < m_profilesListWidget->count(); i++) {
            profiles << m_profilesListWidget->item(i)->text();
        }

        Smb4KSettings::setProfilesList(profiles);
        Smb4KSettings::self()->save();

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
                Smb4KProfileManager::self()->migrateProfile(p.initialName, p.currentName);
                it.value().initialName = p.currentName;
                it.value().renamed = false;
            }

            if (p.added) {
                it.value().initialName = p.currentName;
                it.value().added = false;
                it.value().renamed = false;
            }
        }

        m_profilesChanged = checkProfilesChanged();
    }
}

bool Smb4KConfigPageProfiles::profilesChanged() const
{
    return m_profilesChanged;
}

bool Smb4KConfigPageProfiles::checkProfilesChanged()
{
    bool changed = false;

    for (const ProfileContainer &p : qAsConst(m_profiles)) {
        if (p.added || p.removed || p.renamed) {
            changed = true;
            break;
        }
    }

    return changed;
}

ProfileContainer * Smb4KConfigPageProfiles::findProfileContainer(QListWidgetItem* profileItem)
{
    int index = 0;

    for (int i = 0; i < m_profiles.size(); i++) {
        if (m_profiles.at(i).currentName == profileItem->text()) {
            index = i;
            break;
        }
    }

    return &m_profiles[index];
}


bool Smb4KConfigPageProfiles::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_profilesListWidget->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent) {
                QListWidgetItem *profileItem = m_profilesListWidget->itemAt(mouseEvent->position().toPoint());

                if (!profileItem) {
                    m_profilesListWidget->setCurrentItem(nullptr);
                    m_profilesListWidget->clearSelection();
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        else {
            return false;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void Smb4KConfigPageProfiles::slotProfileUsageChanged(bool checked)
{
    m_profilesChanged = (checked != Smb4KSettings::useProfiles());
    m_profilesEditorWidget->setEnabled(checked);
}

void Smb4KConfigPageProfiles::slotAddProfile(bool checked)
{
    Q_UNUSED(checked);

    QString profile = m_profilesInputLineEdit->text();
    m_profilesInputLineEdit->clear();

    m_profilesListWidget->addItem(profile);

    ProfileContainer p;
    p.initialName = profile;
    p.currentName = profile;
    p.removed = false;
    p.renamed = false;
    p.added = true;

    m_profiles << p;

    m_profilesChanged = checkProfilesChanged();
}

void Smb4KConfigPageProfiles::slotEditProfile(bool checked)
{
    Q_UNUSED(checked);

    if (m_profilesListWidget->currentItem()) {
        slotProfileDoubleClicked(m_profilesListWidget->currentItem());
    }
}

void Smb4KConfigPageProfiles::slotRemoveProfile(bool checked)
{
    Q_UNUSED(checked);

    QString profile = m_profilesListWidget->currentItem()->text();
    delete m_profilesListWidget->currentItem();

    QMutableListIterator<ProfileContainer> it(m_profiles);

    while (it.hasNext()) {
        ProfileContainer p = it.next();

        if (p.initialName == profile || p.currentName == profile) {
            if (!p.added) {
                p.removed = true;
            } else {
                it.remove();
            }

            break;
        }
    }

    m_profilesChanged = checkProfilesChanged();
}

void Smb4KConfigPageProfiles::slotProfileDoubleClicked(QListWidgetItem* profileItem)
{
    if (profileItem) {
        m_currentProfileContainer = findProfileContainer(profileItem);
        m_profilesListWidget->setFocus();
        m_profilesListWidget->editItem(profileItem);
    }
}

void Smb4KConfigPageProfiles::slotProfileChanged(QListWidgetItem *profileItem)
{
    if (profileItem && m_currentProfileContainer) {
        m_currentProfileContainer->currentName = profileItem->text();
        m_currentProfileContainer->renamed = !(m_currentProfileContainer->initialName == profileItem->text());
        m_currentProfileContainer = nullptr;
    }

    m_profilesChanged = checkProfilesChanged();
}

void Smb4KConfigPageProfiles::slotEnableButtons()
{
    bool enable = (m_profilesListWidget->currentItem() && m_profilesListWidget->currentItem()->isSelected());

    m_editButton->setEnabled(Smb4KSettings::useProfiles() && enable);
    m_removeButton->setEnabled(Smb4KSettings::useProfiles() && enable);
}

