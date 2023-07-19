/*
    Configuration page for the custom settings

    SPDX-FileCopyrightText: 2013-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagecustomsettings.h"
#include "core/smb4kcustomoptions.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QDialogButtonBox>
#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>

// KDE includes
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageWidget>

using namespace Smb4KGlobal;

Smb4KConfigPageCustomSettings::Smb4KConfigPageCustomSettings(QWidget *parent)
    : QWidget(parent)
{
    m_itemToEdit = nullptr;
    m_customSettingsChanged = false;
    m_savingCustomSettings = false;

    QHBoxLayout *layout = new QHBoxLayout(this);

    QWidget *leftWidget = new QWidget(this);
    QVBoxLayout *leftWidgetLayout = new QVBoxLayout(leftWidget);
    leftWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_listWidget = new QListWidget(leftWidget);
    m_listWidget->setSelectionMode(QListWidget::SingleSelection);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listWidget->viewport()->installEventFilter(this);

    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &Smb4KConfigPageCustomSettings::slotEditCustomItem);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &Smb4KConfigPageCustomSettings::slotItemSelectionChanged);

    leftWidgetLayout->addWidget(m_listWidget);

    m_editorWidget = new Smb4KCustomSettingsEditorWidget(leftWidget);
    m_editorWidget->setVisible(false);

    connect(m_editorWidget, &Smb4KCustomSettingsEditorWidget::edited, this, &Smb4KConfigPageCustomSettings::slotCustomSettingsEdited);

    leftWidgetLayout->addWidget(m_editorWidget);

    //
    // The feedback message widget
    //
    KMessageWidget *messageWidget = new KMessageWidget(leftWidget);
    messageWidget->setCloseButtonVisible(true);
    messageWidget->setMessageType(KMessageWidget::Information);
    messageWidget->setIcon(KDE::icon(QStringLiteral("emblem-information")));
    messageWidget->setText(i18n("All fine."));
    messageWidget->setWordWrap(true);
    messageWidget->setVisible(false);

    leftWidgetLayout->addWidget(messageWidget);

    layout->addWidget(leftWidget);

    //
    // The push buttons
    //
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);

    m_resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    m_resetButton->setEnabled(false);

    m_editButton = buttonBox->addButton(i18n("Edit"), QDialogButtonBox::ActionRole);
    m_editButton->setIcon(KDE::icon(QStringLiteral("edit-rename")));
    m_editButton->setEnabled(false);

    m_removeButton = buttonBox->addButton(i18n("Remove"), QDialogButtonBox::ActionRole);
    m_removeButton->setIcon(KDE::icon(QStringLiteral("edit-delete")));
    m_removeButton->setEnabled(false);

    m_clearButton = buttonBox->addButton(i18n("Clear List"), QDialogButtonBox::ActionRole);
    m_clearButton->setIcon(KDE::icon(QStringLiteral("edit-clear")));
    m_clearButton->setEnabled(!Smb4KCustomOptionsManager::self()->customOptions(true).isEmpty());

    connect(m_resetButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotResetButtonClicked);
    connect(m_editButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotEditButtonClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotRemoveButtonClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotClearButtonClicked);

    layout->addWidget(buttonBox);

    loadCustomSettings();

    connect(this, &Smb4KConfigPageCustomSettings::customSettingsModified, this, &Smb4KConfigPageCustomSettings::slotEnableButtons);
}

Smb4KConfigPageCustomSettings::~Smb4KConfigPageCustomSettings()
{
}

bool Smb4KConfigPageCustomSettings::customSettingsChanged()
{
    return m_customSettingsChanged;
}

void Smb4KConfigPageCustomSettings::loadCustomSettings()
{
    if (m_savingCustomSettings) {
        return;
    }

    if (m_listWidget->count() != 0) {
        m_listWidget->clear();
    }

    QList<OptionsPtr> customOptions = Smb4KCustomOptionsManager::self()->customOptions(true);

    for (const OptionsPtr &option : qAsConst(customOptions)) {
        QVariant variant = QVariant::fromValue(*option.data());

        QListWidgetItem *item = new QListWidgetItem(option->displayString(), m_listWidget);
        item->setData(Qt::UserRole, variant);

        if (option->type() == Host) {
            item->setIcon(KDE::icon(QStringLiteral("network-server")));
        } else {
            item->setIcon(KDE::icon(QStringLiteral("folder-network")));
        }
    }

    m_listWidget->sortItems(Qt::AscendingOrder);

    m_customSettingsChanged = false;
    Q_EMIT customSettingsModified();
}

void Smb4KConfigPageCustomSettings::saveCustomSettings()
{
    if (m_customSettingsChanged) {
        if (m_itemToEdit) {
            OptionsPtr customSettings = m_editorWidget->getCustomSettings();

            if (customSettings) {
                Smb4KCustomOptions currentCustomSettings = m_itemToEdit->data(Qt::UserRole).value<Smb4KCustomOptions>();
                currentCustomSettings.update(customSettings.data());

                QVariant variant = QVariant::fromValue(currentCustomSettings);
                m_itemToEdit->setData(Qt::UserRole, variant);

                customSettings.clear();
            }
        }

        QList<OptionsPtr> customSettingsList;

        for (int i = 0; i < m_listWidget->count(); ++i) {
            OptionsPtr optionsPtr = OptionsPtr(new Smb4KCustomOptions(m_listWidget->item(i)->data(Qt::UserRole).value<Smb4KCustomOptions>()));

            if (optionsPtr) {
                customSettingsList << optionsPtr;
            }
        }

        m_savingCustomSettings = true;
        Smb4KCustomOptionsManager::self()->saveCustomOptions(customSettingsList);
        m_savingCustomSettings = false;

        m_customSettingsChanged = false;
        Q_EMIT customSettingsModified();
    }
}

bool Smb4KConfigPageCustomSettings::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_listWidget->viewport()) {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QPointF pos = m_listWidget->viewport()->mapFromGlobal(mouseEvent->globalPosition());
            QListWidgetItem *item = m_listWidget->itemAt(pos.toPoint());
#else
            QPoint pos = m_listWidget->viewport()->mapFromGlobal(mouseEvent->globalPos());
            QListWidgetItem *item = m_listWidget->itemAt(pos);
#endif

            m_editButton->setEnabled(item != nullptr);
            m_removeButton->setEnabled(item != nullptr);

            if (!item) {
                m_listWidget->clearSelection();
            }
        }
    }

    return QObject::eventFilter(obj, e);
}

void Smb4KConfigPageCustomSettings::slotItemSelectionChanged()
{
    if (m_editorWidget->isVisible()) {
        m_editorWidget->setVisible(false);

        OptionsPtr customSettings = m_editorWidget->getCustomSettings();

        if (customSettings) {
            Smb4KCustomOptions currentCustomSettings = m_itemToEdit->data(Qt::UserRole).value<Smb4KCustomOptions>();
            currentCustomSettings.update(customSettings.data());

            QVariant variant = QVariant::fromValue(currentCustomSettings);
            m_itemToEdit->setData(Qt::UserRole, variant);

            m_itemToEdit = nullptr;
            customSettings.clear();
        }

        m_editorWidget->clear();
    }
}

void Smb4KConfigPageCustomSettings::slotEditCustomItem(QListWidgetItem *item)
{
    OptionsPtr customSettings = OptionsPtr(new Smb4KCustomOptions(item->data(Qt::UserRole).value<Smb4KCustomOptions>()));
    m_editorWidget->setCustomSettings(customSettings);
    m_editorWidget->setVisible(true);
    m_itemToEdit = item;

    // if (optionsPtr) {
    //     if (!Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(optionsPtr, false)) {
    //         KMessageWidget *messageWidget = findChild<KMessageWidget *>();
    //
    //         if (messageWidget) {
    //             messageWidget->setText(i18n("The item %1 was removed, because all custom settings were reset.", item->text()));
    //
    //             if (!messageWidget->isVisible()) {
    //                 messageWidget->setVisible(true);
    //             }
    //         }
    //
    //         delete item;
    //     }
    //
    //     if (item) {
    //         QVariant variant = QVariant::fromValue(*optionsPtr.data());
    //         item->setData(Qt::UserRole, variant);
    //     }
    //
    //     m_customSettingsChanged = (optionsPtr->isChanged() || !item);
    //     Q_EMIT customSettingsModified();
    // }
}

void Smb4KConfigPageCustomSettings::slotEditButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    slotEditCustomItem(m_listWidget->currentItem());
}

void Smb4KConfigPageCustomSettings::slotRemoveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (m_listWidget->currentItem()) {
        delete m_listWidget->currentItem();
        m_listWidget->setCurrentItem(nullptr);
        m_customSettingsChanged = true;
        Q_EMIT customSettingsModified();
    }
}

void Smb4KConfigPageCustomSettings::slotClearButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    m_listWidget->clear();
    m_customSettingsChanged = true;
    Q_EMIT customSettingsModified();
}

void Smb4KConfigPageCustomSettings::slotResetButtonClicked(bool checked)
{
    // FIXME: Do not close the editor
    Q_UNUSED(checked);
    m_listWidget->clear();
    loadCustomSettings();
}

void Smb4KConfigPageCustomSettings::slotEnableButtons()
{
    m_resetButton->setEnabled(m_customSettingsChanged);
    m_clearButton->setEnabled(m_listWidget->count() != 0);
}

void Smb4KConfigPageCustomSettings::slotCustomSettingsEdited(bool changed)
{
    m_customSettingsChanged = changed;
    Q_EMIT customSettingsModified();
}
