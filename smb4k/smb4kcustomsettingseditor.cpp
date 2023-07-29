/*
 *  Editor dialog for the custom settings
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kcustomsettingseditor.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4khomesshareshandler.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGroup>
#include <KLocalizedString>
#include <KWindowConfig>

Smb4KCustomSettingsEditor::Smb4KCustomSettingsEditor(QWidget *parent)
    : QDialog(parent)
{
    m_customSettings = nullptr;

    setWindowTitle(i18n("Custom Settings"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *descriptionWidget = new QWidget(this);
    QHBoxLayout *descriptionWidgetLayout = new QHBoxLayout(descriptionWidget);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("media-mount")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignVCenter);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionPixmap);

    m_descriptionText = new QLabel(this);
    m_descriptionText->setWordWrap(true);
    m_descriptionText->setAlignment(Qt::AlignVCenter);
    m_descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(m_descriptionText);

    layout->addWidget(descriptionWidget);

    m_editorWidget = new Smb4KCustomSettingsEditorWidget(this);
    connect(m_editorWidget, &Smb4KCustomSettingsEditorWidget::edited, this, &Smb4KCustomSettingsEditor::slotCustomSettingsEdited);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_resetButton = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
    m_saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    m_saveButton->setEnabled(false);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(m_resetButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditor::slotRestoreDefaultsClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditor::slotSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KCustomSettingsEditor::slotCancelClicked);

    layout->addWidget(m_editorWidget);
    layout->addWidget(buttonBox);

    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
    QSize dialogSize;

    // FIXME: Insert completion objects?

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KCustomSettingsEditor::~Smb4KCustomSettingsEditor()
{
}

bool Smb4KCustomSettingsEditor::setNetworkItem(NetworkItemPtr networkItem)
{
    Q_ASSERT(networkItem);

    bool customSettingsSet = false;

    if (networkItem) {
        switch (networkItem->type()) {
        case Host: {
            HostPtr host = networkItem.staticCast<Smb4KHost>();
            m_descriptionText->setText(i18n("Define custom settings for host <b>%1</b> and all the shares it provides.", host->hostName()));

            m_customSettings = Smb4KCustomOptionsManager::self()->findOptions(host);

            if (!m_customSettings) {
                m_customSettings = OptionsPtr(new Smb4KCustomOptions(host.data()));
                m_customSettings->setProfile(Smb4KProfileManager::self()->activeProfile());
            }

            m_editorWidget->setCustomSettings(*m_customSettings.data());
            customSettingsSet = true;

            break;
        }
        case Share: {
            SharePtr share = networkItem.staticCast<Smb4KShare>();
            m_descriptionText->setText(i18n("Define custom settings for share <b>%1</b>.", share->displayString()));

            if (!share->isPrinter()) {
                if (share->isHomesShare()) {
                    if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true)) {
                        return customSettingsSet;
                    }
                }

                m_customSettings = Smb4KCustomOptionsManager::self()->findOptions(share);

                if (!m_customSettings) {
                    m_customSettings = OptionsPtr(new Smb4KCustomOptions(share.data()));
                    m_customSettings->setProfile(Smb4KProfileManager::self()->activeProfile());

                    // Get rid of the 'homes' share
                    if (share->isHomesShare()) {
                        m_customSettings->setUrl(share->homeUrl());
                    }
                }

                m_editorWidget->setCustomSettings(*m_customSettings.data());
                customSettingsSet = true;
            }

            break;
        }
        default: {
            break;
        }
        }
    }

    return customSettingsSet;
}

void Smb4KCustomSettingsEditor::slotRestoreDefaultsClicked()
{
    Smb4KCustomOptions defaultCustomSettings;
    // FIXME: Set the necessary parameters
    m_editorWidget->setCustomSettings(defaultCustomSettings);
}

void Smb4KCustomSettingsEditor::slotSaveClicked()
{
    OptionsPtr tempCustomSettings = OptionsPtr(new Smb4KCustomOptions(m_editorWidget->getCustomSettings()));
    m_customSettings.swap(tempCustomSettings);
    Smb4KCustomOptionsManager::self()->addCustomOptions(m_customSettings, true);

    KConfigGroup group(Smb4KSettings::self()->config(), "CustomOptionsDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    // FIXME: Save completion objects?

    accept();
}

void Smb4KCustomSettingsEditor::slotCancelClicked()
{
    reject();
}

void Smb4KCustomSettingsEditor::slotCustomSettingsEdited(bool changed)
{
    m_saveButton->setEnabled(changed);
}
