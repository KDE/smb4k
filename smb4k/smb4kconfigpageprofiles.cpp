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
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMouseEvent>
#include <QPointer>
#include <QVBoxLayout>

// KDE includes
#include <KLocalizedString>

struct ProfileContainer {
    QString initialName;
    QString currentName;
    bool removed;
    bool renamed;
    bool added;
    bool active;
    bool moved;
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

    m_removeButton = new QPushButton(KDE::icon(QStringLiteral("edit-delete")), i18n("Remove"), buttonBox);
    m_removeButton->setEnabled(false);
    buttonBox->addButton(m_removeButton, QDialogButtonBox::ActionRole);

    m_setActiveButton = new QPushButton(KDE::icon(QStringLiteral("checkmark")), i18n("Set Active"), buttonBox);
    m_setActiveButton->setEnabled(false);
    buttonBox->addButton(m_setActiveButton, QDialogButtonBox::ActionRole);

    m_upButton = new QPushButton(KDE::icon(QStringLiteral("arrow-up")), i18n("Move Up"), buttonBox);
    m_upButton->setEnabled(false);
    buttonBox->addButton(m_upButton, QDialogButtonBox::ActionRole);

    m_downButton = new QPushButton(KDE::icon(QStringLiteral("arrow-down")), i18n("Move Down"), buttonBox);
    m_downButton->setEnabled(false);
    buttonBox->addButton(m_downButton, QDialogButtonBox::ActionRole);

    m_editButton = new QPushButton(KDE::icon(QStringLiteral("edit-rename")), i18n("Rename"), buttonBox);
    m_editButton->setEnabled(false);
    buttonBox->addButton(m_editButton, QDialogButtonBox::ActionRole);

    m_resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    m_resetButton->setEnabled(false);

    profilesEditorWidgetLayout->addWidget(buttonBox, 0, 1, 2, 1);

    profilesBoxLayout->addWidget(m_profilesEditorWidget);

    layout->addWidget(profilesBox);

    loadProfiles();

    connect(m_useProfiles, &QCheckBox::toggled, this, &Smb4KConfigPageProfiles::slotProfileUsageChanged);
    connect(m_addButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotAddProfile);
    connect(m_editButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotEditProfile);
    connect(m_removeButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotRemoveProfile);
    connect(m_upButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotMoveProfileUp);
    connect(m_downButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotMoveProfileDown);
    connect(m_setActiveButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotSetProfileActive);
    connect(m_resetButton, &QPushButton::clicked, this, &Smb4KConfigPageProfiles::slotResetProfiles);

    // FIXME: Use double clicking for setting active profile? Or single click over the checkbox?
    connect(m_profilesListWidget, &QListWidget::itemChanged, this, &Smb4KConfigPageProfiles::slotProfileChanged);
    connect(m_profilesListWidget, &QListWidget::currentRowChanged, this, &Smb4KConfigPageProfiles::slotEnableButtons);
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

        QMutableListIterator<ProfileContainer> it(m_profiles);

        while (it.hasNext()) {
            ProfileContainer p = it.next();

            if (p.active) {
                Smb4KProfileManager::self()->setActiveProfile(p.currentName);
            }

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

            it.value().moved = false;
        }

        checkProfilesChanged();
    }
}

bool Smb4KConfigPageProfiles::profilesChanged() const
{
    return m_profilesChanged;
}

void Smb4KConfigPageProfiles::loadProfiles()
{
    if (m_profilesListWidget->count() != 0) {
        m_profilesListWidget->clear();
    }

    QStringList profiles = Smb4KSettings::profilesList();

    for (const QString &profile : qAsConst(profiles)) {
        ProfileContainer p;
        p.initialName = profile;
        p.currentName = profile;
        p.removed = false;
        p.renamed = false;
        p.added = false;
        p.active = (profile == Smb4KProfileManager::self()->activeProfile());
        p.moved = false;

        m_profiles << p;

        QListWidgetItem *profileItem = new QListWidgetItem(profile, m_profilesListWidget);
        profileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        profileItem->setCheckState(p.active ? Qt::Checked : Qt::Unchecked);
    }

    m_profilesChanged = (m_useProfiles->isChecked() != Smb4KSettings::useProfiles());
}

void Smb4KConfigPageProfiles::checkProfilesChanged()
{
    for (const ProfileContainer &p : qAsConst(m_profiles)) {
        if (p.added || p.removed || p.renamed || (p.active && p.currentName != Smb4KProfileManager::self()->activeProfile()) || p.moved) {
            m_profilesChanged = true;
            m_resetButton->setEnabled(true);
            Q_EMIT profilesModified();
            break;
        }
    }
}

ProfileContainer *Smb4KConfigPageProfiles::findProfileContainer(QListWidgetItem *profileItem)
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

bool Smb4KConfigPageProfiles::eventFilter(QObject *watched, QEvent *event)
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
        } else {
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

    if (!m_profilesInputLineEdit->text().isEmpty()) {
        QString profile = m_profilesInputLineEdit->text();
        m_profilesInputLineEdit->clear();

        m_profilesListWidget->addItem(profile);

        ProfileContainer p;
        p.initialName = profile;
        p.currentName = profile;
        p.removed = false;
        p.renamed = false;
        p.added = true;
        p.active = false;
        p.moved = false;

        m_profiles << p;

        checkProfilesChanged();
    }
}

void Smb4KConfigPageProfiles::slotEditProfile(bool checked)
{
    Q_UNUSED(checked);

    if (m_profilesListWidget->currentItem()) {
        m_currentProfileContainer = findProfileContainer(m_profilesListWidget->currentItem());
        m_profilesListWidget->setFocus();
        m_profilesListWidget->editItem(m_profilesListWidget->currentItem());
    }
}

void Smb4KConfigPageProfiles::slotSetProfileActive(bool checked)
{
    Q_UNUSED(checked);

    for (int i = 0; i < m_profilesListWidget->count(); i++) {
        QListWidgetItem *item = m_profilesListWidget->item(i);

        if (item->checkState() == Qt::Checked) {
            item->setCheckState(Qt::Unchecked);

            ProfileContainer *p = findProfileContainer(item);
            p->active = false;
        }
    }

    if (m_profilesListWidget->currentItem()) {
        m_currentProfileContainer = findProfileContainer(m_profilesListWidget->currentItem());
        m_profilesListWidget->setFocus();
        m_profilesListWidget->currentItem()->setCheckState(Qt::Checked);

        m_setActiveButton->setEnabled(false);
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

    checkProfilesChanged();
}

void Smb4KConfigPageProfiles::slotMoveProfileUp(bool checked)
{
    Q_UNUSED(checked);

    int currentRow = m_profilesListWidget->currentRow();
    QListWidgetItem *itemToMove = m_profilesListWidget->takeItem(currentRow);
    m_profilesListWidget->insertItem(currentRow - 1, itemToMove);
    m_profilesListWidget->setCurrentItem(itemToMove);

    ProfileContainer *p = findProfileContainer(itemToMove);

    if (p) {
        p->moved = true;
    }

    checkProfilesChanged();
}

void Smb4KConfigPageProfiles::slotMoveProfileDown(bool checked)
{
    Q_UNUSED(checked);

    int currentRow = m_profilesListWidget->currentRow();
    QListWidgetItem *itemToMove = m_profilesListWidget->takeItem(currentRow);
    m_profilesListWidget->insertItem(currentRow + 1, itemToMove);
    m_profilesListWidget->setCurrentItem(itemToMove);

    ProfileContainer *p = findProfileContainer(itemToMove);

    if (p) {
        p->moved = true;
    }

    checkProfilesChanged();
}

void Smb4KConfigPageProfiles::slotProfileChanged(QListWidgetItem *profileItem)
{
    if (profileItem && m_currentProfileContainer) {
        m_currentProfileContainer->currentName = profileItem->text();
        m_currentProfileContainer->renamed = !(m_currentProfileContainer->initialName == profileItem->text());
        m_currentProfileContainer->active = (profileItem->checkState() == Qt::Checked);
        m_currentProfileContainer = nullptr;
    }

    checkProfilesChanged();
}

void Smb4KConfigPageProfiles::slotResetProfiles(bool checked)
{
    Q_UNUSED(checked);

    loadProfiles();
    checkProfilesChanged();

    m_resetButton->setEnabled(false);

    Q_EMIT profilesModified();
}

void Smb4KConfigPageProfiles::slotEnableButtons(int row)
{
    m_editButton->setEnabled((row != -1));
    m_removeButton->setEnabled((row != -1));
    m_setActiveButton->setEnabled((row != -1) && (m_profilesListWidget->item(row)->checkState() == Qt::Unchecked));

    m_upButton->setEnabled((row > 0));
    m_downButton->setEnabled((row > -1 && row + 1 < m_profilesListWidget->count()));
}
