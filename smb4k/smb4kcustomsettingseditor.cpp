/*
 *  Editor dialog for the custom settings
 *
 *  SPDX-FileCopyrightText: 2023-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kcustomsettingseditor.h"
#include "core/smb4kcustomsettingsmanager.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksettings.h"
#include "smb4khomesuserdialog.h"

// Qt includes
#include <QDialogButtonBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGroup>
#include <KLocalizedString>
#include <KWindowConfig>

Smb4KCustomSettingsEditor::Smb4KCustomSettingsEditor(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Custom Settings Editor"));
    setAttribute(Qt::WA_DeleteOnClose);

    m_customSettings = nullptr;
    m_savingCustomSettings = false;
    m_changedCustomSettings = false;

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *descriptionWidget = new QWidget(this);
    QHBoxLayout *descriptionWidgetLayout = new QHBoxLayout(descriptionWidget);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("settings-configure")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignVCenter);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionPixmap);

    m_descriptionText = new QLabel(descriptionWidget);
    m_descriptionText->setWordWrap(true);
    m_descriptionText->setAlignment(Qt::AlignVCenter);
    m_descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_descriptionText->setText(i18n("No network item was set."));

    descriptionWidgetLayout->addWidget(m_descriptionText);

    layout->addWidget(descriptionWidget);
    layout->addSpacing(layout->spacing());

    m_editorWidget = new Smb4KCustomSettingsEditorWidget(this);
    connect(m_editorWidget, &Smb4KCustomSettingsEditorWidget::edited, this, &Smb4KCustomSettingsEditor::slotCustomSettingsEdited);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_restoreButton = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
    m_resetButton = buttonBox->addButton(QDialogButtonBox::Reset);

    m_saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    m_saveButton->setEnabled(false);
    m_saveButton->setShortcut(QKeySequence::Save);
    m_saveButton->setDefault(true);

    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_cancelButton->setShortcut(QKeySequence::Cancel);

    connect(m_restoreButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditor::slotRestoreDefaultCustomSettings);
    connect(m_resetButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditor::slotResetCustomSettings);
    connect(m_saveButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditor::slotSaveCustomSettings);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditor::reject);

    layout->addWidget(m_editorWidget);
    layout->addWidget(buttonBox);

    connect(Smb4KCustomSettingsManager::self(), &Smb4KCustomSettingsManager::updated, this, &Smb4KCustomSettingsEditor::slotCustomSettingsUpdated);

    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    create();

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("CustomSettingsDialog"));
    QSize dialogSize;

    if (dialogGroup.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    m_editorWidget->loadCompletionItems();
}

Smb4KCustomSettingsEditor::~Smb4KCustomSettingsEditor()
{
}

bool Smb4KCustomSettingsEditor::setNetworkItem(NetworkItemPtr networkItem)
{
    Q_ASSERT(networkItem);

    if (networkItem) {
        switch (networkItem->type()) {
        case Host: {
            HostPtr host = networkItem.staticCast<Smb4KHost>();
            m_descriptionText->setText(i18n("Define custom settings for host <b>%1</b> and all the shares it provides.", host->hostName()));

            m_customSettings = Smb4KCustomSettingsManager::self()->findCustomSettings(host);

            if (!m_customSettings) {
                m_customSettings = CustomSettingsPtr(new Smb4KCustomSettings(host.data()));
                m_customSettings->setProfile(Smb4KProfileManager::self()->activeProfile());
            }

            m_editorWidget->setCustomSettings(*m_customSettings.data());
            break;
        }
        case Share: {
            SharePtr share = networkItem.staticCast<Smb4KShare>();

            if (!share->isPrinter()) {
                if (share->isHomesShare()) {
                    QPointer<Smb4KHomesUserDialog> homesUserDialog = new Smb4KHomesUserDialog(this);
                    bool proceed = false;

                    if (homesUserDialog->setShare(share)) {
                        // We want to get a return value here, so we use exec()
                        if (homesUserDialog->exec() == QDialog::Accepted) {
                            proceed = true;
                        }
                    }

                    delete homesUserDialog;

                    if (!proceed) {
                        return false;
                    }
                }

                m_descriptionText->setText(i18n("Define custom settings for share <b>%1</b>.", share->displayString(true)));
                m_customSettings = Smb4KCustomSettingsManager::self()->findCustomSettings(share);

                if (!m_customSettings) {
                    m_customSettings = CustomSettingsPtr(new Smb4KCustomSettings(share.data()));
                    m_customSettings->setProfile(Smb4KProfileManager::self()->activeProfile());

                    // Get rid of the 'homes' share
                    if (share->isHomesShare()) {
                        m_customSettings->setUrl(share->homeUrl());
                    }
                }

                m_editorWidget->setCustomSettings(*m_customSettings.data());
            }

            break;
        }
        default: {
            break;
        }
        }

        m_restoreButton->setEnabled(m_customSettings->hasCustomSettings(true));
    }

    return true;
}

void Smb4KCustomSettingsEditor::slotRestoreDefaultCustomSettings()
{
    Smb4KCustomSettings defaultCustomSettings;
    defaultCustomSettings.setUrl(m_customSettings->url());
    defaultCustomSettings.setIpAddress(m_customSettings->ipAddress());
    defaultCustomSettings.setWorkgroupName(m_customSettings->workgroupName());

    m_customSettings->update(&defaultCustomSettings);
    m_editorWidget->setCustomSettings(*m_customSettings.data());
    m_restoreButton->setEnabled(false);
}

void Smb4KCustomSettingsEditor::slotResetCustomSettings()
{
    // The data stored in m_customSettings is untouched until the user
    // saves the modifications and the dialog is closed. So, we can
    // use it to restore the default values.
    m_editorWidget->setCustomSettings(*m_customSettings.data());
    m_resetButton->setEnabled(false);
}

void Smb4KCustomSettingsEditor::slotSaveCustomSettings()
{
    CustomSettingsPtr tempCustomSettings = CustomSettingsPtr(new Smb4KCustomSettings(m_editorWidget->getCustomSettings()));
    m_customSettings.swap(tempCustomSettings);

    m_savingCustomSettings = true;
    Smb4KCustomSettingsManager::self()->addCustomSettings(m_customSettings);
    m_savingCustomSettings = false;

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("CustomSettingsDialog"));
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    m_editorWidget->saveCompletionItems();

    accept();
}

void Smb4KCustomSettingsEditor::slotCustomSettingsEdited(bool changed)
{
    m_restoreButton->setEnabled(!m_editorWidget->hasDefaultCustomSettings());
    m_resetButton->setEnabled(changed);
    m_saveButton->setEnabled(changed);

    m_changedCustomSettings = changed;
}

void Smb4KCustomSettingsEditor::slotCustomSettingsUpdated()
{
    if (!m_savingCustomSettings) {
        CustomSettingsPtr customSettings = Smb4KCustomSettingsManager::self()->findCustomSettings(m_customSettings->url());

        // Only reload existing custom settings, because only those could have
        // been changed externally.
        if (customSettings && !m_changedCustomSettings) {
            m_customSettings = customSettings;
            m_editorWidget->setCustomSettings(*m_customSettings.data());
        }
    }
}
