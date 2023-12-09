/*
    Configuration page for the custom settings

    SPDX-FileCopyrightText: 2013-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagecustomsettings.h"
#include "core/smb4kcustomsettings.h"
#include "core/smb4kcustomsettingsmanager.h"
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
    m_messageWidget = new KMessageWidget(leftWidget);
    m_messageWidget->setCloseButtonVisible(true);
    m_messageWidget->setMessageType(KMessageWidget::Information);
    m_messageWidget->setIcon(KDE::icon(QStringLiteral("emblem-information")));
    m_messageWidget->setText(i18n("All fine."));
    m_messageWidget->setWordWrap(true);
    m_messageWidget->setVisible(false);

    leftWidgetLayout->addWidget(m_messageWidget);

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
    m_clearButton->setEnabled(!Smb4KCustomSettingsManager::self()->customSettings(true).isEmpty());

    connect(m_resetButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotResetButtonClicked);
    connect(m_editButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotEditButtonClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotRemoveButtonClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &Smb4KConfigPageCustomSettings::slotClearButtonClicked);

    layout->addWidget(buttonBox);

    loadCustomSettings();

    connect(this, &Smb4KConfigPageCustomSettings::customSettingsModified, this, &Smb4KConfigPageCustomSettings::slotEnableButtons);
    connect(Smb4KCustomSettingsManager::self(), &Smb4KCustomSettingsManager::updated, this, &Smb4KConfigPageCustomSettings::loadCustomSettings);
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

    QList<CustomSettingsPtr> customSettings = Smb4KCustomSettingsManager::self()->customSettings(true);

    for (const CustomSettingsPtr &option : qAsConst(customSettings)) {
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
            Smb4KCustomSettings customSettings = m_editorWidget->getCustomSettings();

            if (customSettings.hasOptions()) {
                Smb4KCustomSettings currentCustomSettings = m_itemToEdit->data(Qt::UserRole).value<Smb4KCustomSettings>();
                currentCustomSettings.update(&customSettings);

                QVariant variant = QVariant::fromValue(currentCustomSettings);
                m_itemToEdit->setData(Qt::UserRole, variant);
            } else {
                m_editorWidget->setVisible(false);
                m_editorWidget->clear();

                delete m_itemToEdit;
                m_itemToEdit = nullptr;

                setRemovalMessage(customSettings);

                if (!m_messageWidget->isVisible()) {
                    m_messageWidget->setVisible(true);
                }
            }
        }

        QList<CustomSettingsPtr> customSettingsList;

        for (int i = 0; i < m_listWidget->count(); ++i) {
            CustomSettingsPtr optionsPtr = CustomSettingsPtr(new Smb4KCustomSettings(m_listWidget->item(i)->data(Qt::UserRole).value<Smb4KCustomSettings>()));

            if (optionsPtr) {
                customSettingsList << optionsPtr;
            }
        }

        m_savingCustomSettings = true;
        Smb4KCustomSettingsManager::self()->saveCustomSettings(customSettingsList);
        m_savingCustomSettings = false;

        m_customSettingsChanged = false;
        Q_EMIT customSettingsModified();
    }
}

void Smb4KConfigPageCustomSettings::setRemovalMessage(const Smb4KCustomSettings &settings)
{
    m_messageWidget->setText(i18n("The item <b>%1</b> was removed, because all custom settings were reset.", settings.displayString()));
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

        Smb4KCustomSettings customSettings = m_editorWidget->getCustomSettings();

        if (customSettings.hasOptions()) {
            Smb4KCustomSettings currentCustomSettings = m_itemToEdit->data(Qt::UserRole).value<Smb4KCustomSettings>();
            currentCustomSettings.update(&customSettings);

            QVariant variant = QVariant::fromValue(currentCustomSettings);
            m_itemToEdit->setData(Qt::UserRole, variant);

            m_itemToEdit = nullptr;
        } else {
            delete m_itemToEdit;
            m_itemToEdit = nullptr;

            setRemovalMessage(customSettings);

            if (!m_messageWidget->isVisible()) {
                m_messageWidget->setVisible(true);
            }
        }

        m_editorWidget->clear();
    }
}

void Smb4KConfigPageCustomSettings::slotEditCustomItem(QListWidgetItem *item)
{
    m_editorWidget->setCustomSettings(item->data(Qt::UserRole).value<Smb4KCustomSettings>());
    m_editorWidget->setVisible(true);
    m_itemToEdit = item;
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
    m_messageWidget->setVisible(false);
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
