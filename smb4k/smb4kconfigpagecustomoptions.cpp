/*
    The configuration page for the custom options

    SPDX-FileCopyrightText: 2013-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagecustomoptions.h"
#include "core/smb4kcustomoptions.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QDialogButtonBox>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KMessageWidget>

using namespace Smb4KGlobal;

Smb4KConfigPageCustomOptions::Smb4KConfigPageCustomOptions(QWidget *parent)
    : QWidget(parent)
{
    m_maybe_changed = false;

    //
    // Layout
    //
    QHBoxLayout *layout = new QHBoxLayout(this);

    //
    // Left widget comprising of list widget and message widget
    //
    QWidget *leftWidget = new QWidget(this);
    QVBoxLayout *leftWidgetLayout = new QVBoxLayout(leftWidget);
    leftWidgetLayout->setMargin(0);

    //
    // The list widget
    //
    QListWidget *optionsListWidget = new QListWidget(leftWidget);
    optionsListWidget->setObjectName("OptionsListWidget");
    optionsListWidget->setSelectionMode(QListWidget::SingleSelection);
    optionsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    optionsListWidget->viewport()->installEventFilter(this);

    connect(optionsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(slotEditCustomItem(QListWidgetItem *)));

    leftWidgetLayout->addWidget(optionsListWidget);

    //
    // The feedback message widget
    //
    KMessageWidget *messageWidget = new KMessageWidget(leftWidget);
    messageWidget->setCloseButtonVisible(true);
    messageWidget->setMessageType(KMessageWidget::Information);
    messageWidget->setIcon(KDE::icon("emblem-information"));
    messageWidget->setText(i18n("All fine."));
    messageWidget->setWordWrap(true);
    messageWidget->setVisible(false);

    leftWidgetLayout->addWidget(messageWidget);

    layout->addWidget(leftWidget);

    //
    // The push buttons
    //
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);

    QPushButton *resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    resetButton->setEnabled(false);
    resetButton->setObjectName("reset_button");

    QPushButton *editButton = buttonBox->addButton(i18n("Edit"), QDialogButtonBox::ActionRole);
    editButton->setIcon(KDE::icon("edit-rename"));
    editButton->setObjectName("edit_button");
    editButton->setEnabled(false);

    QPushButton *removeButton = buttonBox->addButton(i18n("Remove"), QDialogButtonBox::ActionRole);
    removeButton->setIcon(KDE::icon("edit-delete"));
    removeButton->setObjectName("remove_button");
    removeButton->setEnabled(false);

    QPushButton *clearButton = buttonBox->addButton(i18n("Clear List"), QDialogButtonBox::ActionRole);
    clearButton->setIcon(KDE::icon("edit-clear"));
    clearButton->setObjectName("clear_button");
    clearButton->setEnabled(!Smb4KCustomOptionsManager::self()->customOptions(true).isEmpty());

    connect(resetButton, SIGNAL(clicked(bool)), SLOT(slotResetButtonClicked(bool)));
    connect(editButton, SIGNAL(clicked(bool)), SLOT(slotEditButtonClicked(bool)));
    connect(removeButton, SIGNAL(clicked(bool)), SLOT(slotRemoveButtonClicked(bool)));
    connect(clearButton, SIGNAL(clicked(bool)), SLOT(slotClearButtonClicked(bool)));

    layout->addWidget(buttonBox);

    //
    // Load custom options
    //
    loadCustomOptions();

    //
    // Connection to enable/disable the reset button
    //
    connect(this, SIGNAL(customSettingsModified()), SLOT(slotEnableResetButton()));
}

Smb4KConfigPageCustomOptions::~Smb4KConfigPageCustomOptions()
{
}

bool Smb4KConfigPageCustomOptions::customSettingsMaybeChanged()
{
    return m_maybe_changed;
}

void Smb4KConfigPageCustomOptions::loadCustomOptions()
{
    //
    // Get the list widget and display the new options
    //
    QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");

    if (optionsListWidget) {
        // Clear the list widget
        while (optionsListWidget->count() != 0) {
            delete optionsListWidget->item(0);
        }

        // Display the new options
        QList<OptionsPtr> customOptions = Smb4KCustomOptionsManager::self()->customOptions(true);

        for (const OptionsPtr &option : qAsConst(customOptions)) {
            switch (option->type()) {
            case Host: {
                QListWidgetItem *item = new QListWidgetItem(KDE::icon("network-server"), option->displayString(), optionsListWidget, Host);
                item->setData(Qt::UserRole, option->url());
                break;
            }
            case Share: {
                QListWidgetItem *item = new QListWidgetItem(KDE::icon("folder-network"), option->displayString(), optionsListWidget, Share);
                item->setData(Qt::UserRole, option->url());
                break;
            }
            default: {
                break;
            }
            }
        }

        optionsListWidget->sortItems(Qt::AscendingOrder);
    }
}

void Smb4KConfigPageCustomOptions::saveCustomOptions()
{
    if (customSettingsMaybeChanged()) {
        Smb4KCustomOptionsManager::self()->saveCustomOptions();

        m_maybe_changed = false;

        // Do not emit walletEntriesModified() signal, because we do not
        // want to enable/disable the "Apply" button as well.
        slotEnableResetButton();
    }
}

void Smb4KConfigPageCustomOptions::resetCustomOptions()
{
    Smb4KCustomOptionsManager::self()->resetCustomOptions();
}

bool Smb4KConfigPageCustomOptions::eventFilter(QObject *obj, QEvent *e)
{
    QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");

    if (optionsListWidget) {
        if (obj == optionsListWidget->viewport()) {
            if (e->type() == QEvent::MouseButtonPress) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
                QPoint pos = optionsListWidget->viewport()->mapFromGlobal(mouseEvent->globalPos());
                QListWidgetItem *item = optionsListWidget->itemAt(pos);

                findChild<QPushButton *>("edit_button")->setEnabled(item != nullptr);
                findChild<QPushButton *>("remove_button")->setEnabled(item != nullptr);

                if (!item) {
                    optionsListWidget->clearSelection();
                }
            }
        }
    }

    return QObject::eventFilter(obj, e);
}

void Smb4KConfigPageCustomOptions::slotEditCustomItem(QListWidgetItem *item)
{
    QUrl itemUrl = item->data(Qt::UserRole).toUrl();

    OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(itemUrl);

    if (options) {
        if (!Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(options, false)) {
            KMessageWidget *messageWidget = findChild<KMessageWidget *>();

            if (messageWidget) {
                messageWidget->setText(i18n("The item %1 was removed, because all custom options were reset.", item->text()));

                if (!messageWidget->isVisible()) {
                    messageWidget->setVisible(true);
                }
            }

            delete item;
        }

        m_maybe_changed = true;
        emit customSettingsModified();
    }
}

void Smb4KConfigPageCustomOptions::slotEditButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");

    if (optionsListWidget) {
        slotEditCustomItem(optionsListWidget->currentItem());
    }
}

void Smb4KConfigPageCustomOptions::slotRemoveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");

    if (optionsListWidget) {
        QListWidgetItem *item = optionsListWidget->currentItem();

        if (item) {
            QUrl itemUrl = item->data(Qt::UserRole).toUrl();

            OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(itemUrl);

            if (options) {
                Smb4KCustomOptionsManager::self()->removeCustomOptions(options, false);
            }

            delete item;
            m_maybe_changed = true;
            emit customSettingsModified();
        }
    }

    findChild<QPushButton *>("clear_button")->setEnabled(optionsListWidget->count() != 0);
}

void Smb4KConfigPageCustomOptions::slotClearButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");

    if (optionsListWidget) {
        while (optionsListWidget->count() != 0) {
            QListWidgetItem *item = optionsListWidget->item(0);

            if (item) {
                QUrl itemUrl = item->data(Qt::UserRole).toUrl();

                OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(itemUrl);

                if (options) {
                    Smb4KCustomOptionsManager::self()->removeCustomOptions(options, false);
                }

                delete item;
            }
        }

        m_maybe_changed = true;
        emit customSettingsModified();
    }

    findChild<QPushButton *>("clear_button")->setEnabled(optionsListWidget->count() != 0);
}

void Smb4KConfigPageCustomOptions::slotResetButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    QListWidget *optionsListWidget = findChild<QListWidget *>("OptionsListWidget");

    if (optionsListWidget) {
        Smb4KCustomOptionsManager::self()->resetCustomOptions();
        loadCustomOptions();
    }

    m_maybe_changed = false;
    emit customSettingsModified();

    findChild<QPushButton *>("clear_button")->setEnabled(optionsListWidget->count() != 0);
}

void Smb4KConfigPageCustomOptions::slotEnableResetButton()
{
    QDialogButtonBox *buttonBox = findChild<QDialogButtonBox *>();

    if (buttonBox) {
        QPushButton *resetButton = buttonBox->button(QDialogButtonBox::Reset);

        if (resetButton) {
            resetButton->setEnabled(m_maybe_changed);
        }
    }
}
