/*
    The configuration page for the authentication settings of Smb4K

    SPDX-FileCopyrightText: 2003-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpageauthentication.h"
#include "core/smb4ksettings.h"
#include "core/smb4kwalletmanager.h"

// Qt includes
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KPasswordDialog>

Smb4KConfigPageAuthentication::Smb4KConfigPageAuthentication(QWidget *parent)
    : QWidget(parent)
{
    m_entries_loaded = false;

    //
    // Layout
    //
    QVBoxLayout *layout = new QVBoxLayout(this);

    //
    // Settings
    //
    QGroupBox *settingsBox = new QGroupBox(i18n("Settings"), this);
    QVBoxLayout *settingsBoxLayout = new QVBoxLayout(settingsBox);

    QCheckBox *useWallet = new QCheckBox(Smb4KSettings::self()->useWalletItem()->label(), settingsBox);
    useWallet->setObjectName("kcfg_UseWallet");

    connect(useWallet, SIGNAL(toggled(bool)), this, SLOT(slotKWalletButtonToggled(bool)));

    settingsBoxLayout->addWidget(useWallet);

    QCheckBox *useDefaultLogin = new QCheckBox(Smb4KSettings::self()->useDefaultLoginItem()->label(), settingsBox);
    useDefaultLogin->setObjectName("kcfg_UseDefaultLogin");

    connect(useDefaultLogin, SIGNAL(toggled(bool)), this, SLOT(slotDefaultLoginToggled(bool)));

    settingsBoxLayout->addWidget(useDefaultLogin);

    layout->addWidget(settingsBox);

    //
    // Wallet entries widget
    //
    QGroupBox *walletEntriesBox = new QGroupBox(i18n("Wallet Entries"), this);
    QVBoxLayout *walletEntriesBoxLayout = new QVBoxLayout(walletEntriesBox);

    //
    // Wallet Entries editor
    //
    QWidget *walletEntriesEditor = new QWidget(walletEntriesBox);
    walletEntriesEditor->setObjectName("WalletEntriesEditor");
    QHBoxLayout *walletEntriesEditorLayout = new QHBoxLayout(walletEntriesEditor);
    walletEntriesEditorLayout->setMargin(0);

    //
    // The list view
    //
    QListWidget *walletEntriesWidget = new QListWidget(walletEntriesEditor);
    walletEntriesWidget->setObjectName("WalletEntriesWidget");
    walletEntriesWidget->setDragDropMode(QListWidget::NoDragDrop);
    walletEntriesWidget->setSelectionMode(QListWidget::SingleSelection);
    walletEntriesWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    walletEntriesWidget->viewport()->installEventFilter(this);

    connect(walletEntriesWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(slotWalletItemDoubleClicked(QListWidgetItem *)));

    walletEntriesEditorLayout->addWidget(walletEntriesWidget);

    //
    // The button box
    //
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, walletEntriesBox);

    //
    // Load button
    //
    QPushButton *loadButton = buttonBox->addButton(i18n("Load"), QDialogButtonBox::ActionRole);
    loadButton->setIcon(KDE::icon("document-open"));
    loadButton->setObjectName("load_button");

    connect(loadButton, SIGNAL(clicked(bool)), SLOT(slotLoadButtonClicked(bool)));

    //
    // Save button
    //
    QPushButton *saveButton = buttonBox->addButton(i18n("Save"), QDialogButtonBox::ActionRole);
    saveButton->setIcon(KDE::icon("document-save-all"));
    saveButton->setObjectName("save_button");
    saveButton->setEnabled(false);

    connect(saveButton, SIGNAL(clicked(bool)), SLOT(slotSaveButtonClicked(bool)));

    //
    // Edit button
    //
    QPushButton *editButton = buttonBox->addButton(i18n("Edit"), QDialogButtonBox::ActionRole);
    editButton->setIcon(KDE::icon("edit-rename"));
    editButton->setObjectName("edit_button");
    editButton->setEnabled(false);

    connect(editButton, SIGNAL(clicked(bool)), SLOT(slotEditButtonClicked(bool)));

    //
    // Remove button
    //
    QPushButton *removeButton = buttonBox->addButton(i18n("Remove"), QDialogButtonBox::ActionRole);
    removeButton->setIcon(KDE::icon("edit-delete"));
    removeButton->setObjectName("remove_button");
    removeButton->setEnabled(false);

    connect(removeButton, SIGNAL(clicked(bool)), SLOT(slotRemoveButtonClicked(bool)));

    //
    // Clear button
    //
    QPushButton *clearButton = buttonBox->addButton(i18n("Clear"), QDialogButtonBox::ActionRole);
    clearButton->setIcon(KDE::icon("edit-clear-list"));
    clearButton->setObjectName("clear_button");
    clearButton->setEnabled(false);

    connect(clearButton, SIGNAL(clicked(bool)), SLOT(slotClearButtonClicked(bool)));

    //
    // Reset button
    //
    QPushButton *resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    resetButton->setObjectName("reset_button");
    resetButton->setEnabled(false);

    walletEntriesEditorLayout->addWidget(buttonBox);

    walletEntriesBoxLayout->addWidget(walletEntriesEditor);

    layout->addWidget(walletEntriesBox);

    //
    // Adjustments
    //
    slotKWalletButtonToggled(useWallet->isChecked());
    slotDefaultLoginToggled(useDefaultLogin->isChecked());

    //
    // Connection to enable/disable the reset button
    //
    connect(this, SIGNAL(walletEntriesModified()), SLOT(slotEnableResetButton()));
}

Smb4KConfigPageAuthentication::~Smb4KConfigPageAuthentication()
{
}

void Smb4KConfigPageAuthentication::loadLoginCredentials()
{
    //
    // Insert the list of authentication information
    //
    m_entriesList = Smb4KWalletManager::self()->loginCredentialsList();

    //
    // Reset the changed flag, since we are (re)loading the information
    //
    Q_EMIT walletEntriesModified();

    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Clear the list widget
    //
    walletEntriesWidget->clear();

    //
    // Insert the authentication information entries into the
    // list widget
    //
    for (Smb4KAuthInfo *authInfo : qAsConst(m_entriesList)) {
        switch (authInfo->type()) {
        case UnknownNetworkItem: {
            QListWidgetItem *item = new QListWidgetItem(KDE::icon("dialog-password"), i18n("Default Login"), walletEntriesWidget);
            item->setData(Qt::UserRole, authInfo->url());
            break;
        }
        default: {
            QListWidgetItem *item = new QListWidgetItem(KDE::icon("dialog-password"), authInfo->displayString(), walletEntriesWidget);
            item->setData(Qt::UserRole, authInfo->url());
            break;
        }
        }
    }

    //
    // Sort the entries
    //
    walletEntriesWidget->sortItems();

    //
    // Set the loaded flag to true
    //
    m_entries_loaded = true;

    //
    // Enable buttons
    //
    findChild<QPushButton *>("save_button")->setEnabled(walletEntriesWidget->count() != 0);
    findChild<QPushButton *>("clear_button")->setEnabled(walletEntriesWidget->count() != 0);
}

void Smb4KConfigPageAuthentication::saveLoginCredentials()
{
    if (loginCredentialsChanged()) {
        Smb4KWalletManager::self()->writeLoginCredentialsList(m_entriesList);

        // Do not emit walletEntriesModified() signal, because we do not
        // want to enable/disable the "Apply" button as well.
        slotEnableResetButton();
    }
}

bool Smb4KConfigPageAuthentication::loginCredentialsLoaded()
{
    return m_entries_loaded;
}

bool Smb4KConfigPageAuthentication::loginCredentialsChanged()
{
    bool changed = false;

    if (m_entries_loaded) {
        QList<Smb4KAuthInfo *> savedLoginCredentials = Smb4KWalletManager::self()->loginCredentialsList();

        if (savedLoginCredentials.size() != m_entriesList.size()) {
            changed = true;
        } else {
            for (Smb4KAuthInfo *oldEntry : qAsConst(savedLoginCredentials)) {
                for (Smb4KAuthInfo *newEntry : qAsConst(m_entriesList)) {
                    if (oldEntry->url().matches(newEntry->url(), QUrl::RemoveUserInfo | QUrl::RemovePort)) {
                        changed = (oldEntry->url().userInfo() != newEntry->url().userInfo());
                        break;
                    }
                }

                if (changed) {
                    break;
                }
            }
        }
    }

    return changed;
}

bool Smb4KConfigPageAuthentication::eventFilter(QObject *object, QEvent *e)
{
    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Process the events in the list widget
    //
    if (object == walletEntriesWidget->viewport()) {
        // If the user clicked on the viewport of the entries view, clear
        // the details widget and the "Details" button, if no item
        // is under the mouse.
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *event = static_cast<QMouseEvent *>(e);
            QPoint pos = walletEntriesWidget->mapFromGlobal(event->globalPos());
            QListWidgetItem *item = walletEntriesWidget->itemAt(pos);

            findChild<QPushButton *>("edit_button")->setEnabled(item != nullptr);
            findChild<QPushButton *>("remove_button")->setEnabled(item != nullptr);

            if (!item) {
                walletEntriesWidget->clearSelection();
            }
        }
    }

    return QWidget::eventFilter(object, e);
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigPageAuthentication::slotKWalletButtonToggled(bool checked)
{
    findChild<QCheckBox *>("kcfg_UseDefaultLogin")->setEnabled(checked);
    findChild<QWidget *>("WalletEntriesEditor")->setEnabled(checked);
}

void Smb4KConfigPageAuthentication::slotDefaultLoginToggled(bool checked)
{
    if (checked && !Smb4KSettings::useDefaultLogin()) {
        if (!Smb4KWalletManager::self()->hasDefaultCredentials()) {
            Smb4KAuthInfo authInfo;
            // If there are no default credentials, we do not need to read them.
            KPasswordDialog dlg(this, KPasswordDialog::ShowUsernameLine);
            dlg.setPrompt(i18n("Enter the default login information."));
            dlg.setUsername(authInfo.userName());
            dlg.setPassword(authInfo.password());

            if (dlg.exec() == KPasswordDialog::Accepted) {
                authInfo.setUserName(dlg.username());
                authInfo.setPassword(dlg.password());

                Smb4KWalletManager::self()->writeLoginCredentials(&authInfo);

                if (m_entries_loaded) {
                    loadLoginCredentials();
                }
            } else {
                findChild<QCheckBox *>("kcfg_UseDefaultLogin")->setChecked(false);
            }
        }
    }
}

void Smb4KConfigPageAuthentication::slotLoadButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (!m_entries_loaded) {
        loadLoginCredentials();
    }

    findChild<QPushButton *>("load_button")->setEnabled(false);
    findChild<QListWidget *>()->setFocus();
}

void Smb4KConfigPageAuthentication::slotSaveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (m_entries_loaded) {
        saveLoginCredentials();
    }

    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Disable buttons
    //
    findChild<QPushButton *>("edit_button")->setEnabled(false);
    findChild<QPushButton *>("remove_button")->setEnabled(false);
    findChild<QPushButton *>("clear_button")->setEnabled((walletEntriesWidget->count() != 0));

    //
    // Clear the selection in the list view
    //
    walletEntriesWidget->clearSelection();

    //
    // Tell the program that the authentication information may be changed
    // and emit the appropriate signal
    //
    Q_EMIT walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotEditButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    KPasswordDialog dlg(this, KPasswordDialog::ShowUsernameLine);

    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    if (walletEntriesWidget->currentItem()) {
        Smb4KAuthInfo *authInfo = nullptr;

        for (Smb4KAuthInfo *walletEntry : qAsConst(m_entriesList)) {
            // The following check also finds the default login, because it has an empty URL.
            if (walletEntriesWidget->currentItem()->data(Qt::UserRole).toUrl() == walletEntry->url()) {
                if (walletEntry->type() != Smb4KGlobal::UnknownNetworkItem) {
                    dlg.setPrompt(i18n("Set the username and password for wallet entry %1.", walletEntry->displayString()));
                } else {
                    dlg.setPrompt(i18n("Set the username and password for the default login."));
                }
                dlg.setUsername(walletEntry->userName());
                dlg.setPassword(walletEntry->password());

                authInfo = walletEntry;

                break;
            }
        }

        if (authInfo) {
            if (dlg.exec() == KPasswordDialog::Accepted) {
                authInfo->setUserName(dlg.username());
                authInfo->setPassword(dlg.password());

                Q_EMIT walletEntriesModified();
            }
        }
    }
}

void Smb4KConfigPageAuthentication::slotRemoveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Remove the appropriate entry from the list of authentication information
    //
    for (int i = 0; i < m_entriesList.size(); ++i) {
        // The following check also finds the default login, because it has an empty URL.
        if (walletEntriesWidget->currentItem()->data(Qt::UserRole).toUrl() == m_entriesList.at(i)->url()) {
            switch (m_entriesList.at(i)->type()) {
            case UnknownNetworkItem: {
                QCheckBox *useDefaultLogin = findChild<QCheckBox *>("kcfg_UseDefaultLogin");
                useDefaultLogin->setChecked(false);
                break;
            }
            default: {
                break;
            }
            }

            delete m_entriesList.takeAt(i);
            break;
        } else {
            continue;
        }
    }

    //
    // Remove the current item
    //
    delete walletEntriesWidget->currentItem();

    //
    // Enable actions
    //
    findChild<QPushButton *>("clear_button")->setEnabled((walletEntriesWidget->count() != 0));

    //
    // Tell the program that the authentication information may be changed
    // and emit the appropriate signal
    //
    Q_EMIT walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotClearButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Remove all entries from the view
    //
    while (walletEntriesWidget->count() != 0) {
        delete walletEntriesWidget->item(0);
    }

    //
    // Remove all entries from the list off authentication information
    //
    while (!m_entriesList.isEmpty()) {
        delete m_entriesList.takeFirst();
    }

    //
    // Enabled widgets
    //
    findChild<QPushButton *>("clear_button")->setEnabled(false);

    //
    // Uncheck the Default Login checkbox
    //
    findChild<QCheckBox *>("kcfg_UseDefaultLogin")->setChecked(false);

    //
    // Tell the program that the authentication information may be changed
    // and emit the appropriate signal
    //
    Q_EMIT walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotResetButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (m_entries_loaded) {
        loadLoginCredentials();
    }

    Q_EMIT walletEntriesModified();

    findChild<QPushButton *>("clear_button")->setEnabled((findChild<QListWidget *>("WalletEntriesWidget")->count() != 0));
}

void Smb4KConfigPageAuthentication::slotEnableResetButton()
{
    QDialogButtonBox *buttonBox = findChild<QDialogButtonBox *>();

    if (buttonBox) {
        QPushButton *resetButton = buttonBox->button(QDialogButtonBox::Reset);

        if (resetButton) {
            bool changed = loginCredentialsChanged();
            resetButton->setEnabled(changed);
        }
    }
}

void Smb4KConfigPageAuthentication::slotWalletItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);

    slotEditButtonClicked(false);
}
