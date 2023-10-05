/*
    The configuration page for the authentication settings of Smb4K

    SPDX-FileCopyrightText: 2003-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpageauthentication.h"
#include "core/smb4ksettings.h"
#include "core/smb4kwalletmanager.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QVBoxLayout>

// KDE includes
#include <KIconLoader>
#include <KLocalizedString>
#include <KPasswordDialog>

Smb4KConfigPageAuthentication::Smb4KConfigPageAuthentication(QWidget *parent)
    : QWidget(parent)
{
    m_entriesLoaded = false;

    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox *settingsBox = new QGroupBox(i18n("Settings"), this);
    QVBoxLayout *settingsBoxLayout = new QVBoxLayout(settingsBox);

    QCheckBox *useWallet = new QCheckBox(Smb4KSettings::self()->useWalletItem()->label(), settingsBox);
    useWallet->setObjectName(QStringLiteral("kcfg_UseWallet"));

    connect(useWallet, &QCheckBox::toggled, this, &Smb4KConfigPageAuthentication::slotKWalletButtonToggled);

    settingsBoxLayout->addWidget(useWallet);

    m_useDefaultLogin = new QCheckBox(Smb4KSettings::self()->useDefaultLoginItem()->label(), settingsBox);
    m_useDefaultLogin->setObjectName(QStringLiteral("kcfg_UseDefaultLogin"));
    m_useDefaultLogin->setEnabled(false);

    connect(m_useDefaultLogin, &QCheckBox::toggled, this, &Smb4KConfigPageAuthentication::slotDefaultLoginToggled);

    settingsBoxLayout->addWidget(m_useDefaultLogin);

    layout->addWidget(settingsBox);

    QGroupBox *walletEntriesBox = new QGroupBox(i18n("Wallet Entries"), this);
    QVBoxLayout *walletEntriesBoxLayout = new QVBoxLayout(walletEntriesBox);

    m_walletEntriesEditor = new QWidget(walletEntriesBox);
    m_walletEntriesEditor->setEnabled(false);
    QHBoxLayout *walletEntriesEditorLayout = new QHBoxLayout(m_walletEntriesEditor);
    walletEntriesEditorLayout->setContentsMargins(0, 0, 0, 0);

    m_walletEntriesWidget = new QListWidget(m_walletEntriesEditor);
    m_walletEntriesWidget->setDragDropMode(QListWidget::NoDragDrop);
    m_walletEntriesWidget->setSelectionMode(QListWidget::SingleSelection);
    m_walletEntriesWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_walletEntriesWidget->viewport()->installEventFilter(this);

    connect(m_walletEntriesWidget, &QListWidget::itemDoubleClicked, this, &Smb4KConfigPageAuthentication::slotWalletItemDoubleClicked);

    walletEntriesEditorLayout->addWidget(m_walletEntriesWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, m_walletEntriesEditor);

    m_loadButton = buttonBox->addButton(i18n("Load"), QDialogButtonBox::ActionRole);
    m_loadButton->setIcon(KDE::icon(QStringLiteral("document-open")));

    connect(m_loadButton, &QPushButton::clicked, this, &Smb4KConfigPageAuthentication::slotLoadButtonClicked);

    m_saveButton = buttonBox->addButton(i18n("Save"), QDialogButtonBox::ActionRole);
    m_saveButton->setIcon(KDE::icon(QStringLiteral("document-save-all")));
    m_saveButton->setEnabled(false);

    connect(m_saveButton, &QPushButton::clicked, this, &Smb4KConfigPageAuthentication::slotSaveButtonClicked);

    m_editButton = buttonBox->addButton(i18n("Edit"), QDialogButtonBox::ActionRole);
    m_editButton->setIcon(KDE::icon(QStringLiteral("edit-rename")));
    m_editButton->setEnabled(false);

    connect(m_editButton, &QPushButton::clicked, this, &Smb4KConfigPageAuthentication::slotEditButtonClicked);

    m_removeButton = buttonBox->addButton(i18n("Remove"), QDialogButtonBox::ActionRole);
    m_removeButton->setIcon(KDE::icon(QStringLiteral("edit-delete")));
    m_removeButton->setEnabled(false);

    connect(m_removeButton, &QPushButton::clicked, this, &Smb4KConfigPageAuthentication::slotRemoveButtonClicked);

    m_clearButton = buttonBox->addButton(i18n("Clear"), QDialogButtonBox::ActionRole);
    m_clearButton->setIcon(KDE::icon(QStringLiteral("edit-clear-list")));
    m_clearButton->setEnabled(false);

    connect(m_clearButton, &QPushButton::clicked, this, &Smb4KConfigPageAuthentication::slotClearButtonClicked);

    m_resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    m_resetButton->setEnabled(false);

    connect(m_resetButton, &QPushButton::clicked, this, &Smb4KConfigPageAuthentication::slotResetButtonClicked);

    walletEntriesEditorLayout->addWidget(buttonBox);

    walletEntriesBoxLayout->addWidget(m_walletEntriesEditor);

    layout->addWidget(walletEntriesBox);

    connect(this, &Smb4KConfigPageAuthentication::walletEntriesModified, this, &Smb4KConfigPageAuthentication::slotEnableResetButton);
}

Smb4KConfigPageAuthentication::~Smb4KConfigPageAuthentication()
{
}

void Smb4KConfigPageAuthentication::loadLoginCredentials()
{
    m_entriesList = Smb4KWalletManager::self()->loginCredentialsList();

    Q_EMIT walletEntriesModified();

    m_walletEntriesWidget->clear();

    for (Smb4KAuthInfo *authInfo : qAsConst(m_entriesList)) {
        switch (authInfo->type()) {
        case UnknownNetworkItem: {
            QListWidgetItem *item = new QListWidgetItem(KDE::icon(QStringLiteral("dialog-password")), i18n("Default Login"), m_walletEntriesWidget);
            item->setData(Qt::UserRole, authInfo->url());
            break;
        }
        default: {
            QListWidgetItem *item = new QListWidgetItem(KDE::icon(QStringLiteral("dialog-password")), authInfo->displayString(), m_walletEntriesWidget);
            item->setData(Qt::UserRole, authInfo->url());
            break;
        }
        }
    }

    m_walletEntriesWidget->sortItems();

    m_entriesLoaded = true;

    m_saveButton->setEnabled(m_walletEntriesWidget->count() != 0);
    m_clearButton->setEnabled(m_walletEntriesWidget->count() != 0);
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
    return m_entriesLoaded;
}

bool Smb4KConfigPageAuthentication::loginCredentialsChanged()
{
    bool changed = false;

    if (m_entriesLoaded) {
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
    if (object == m_walletEntriesWidget->viewport()) {
        // If the user clicked on the viewport of the entries view, clear
        // the details widget and the "Details" button, if no item
        // is under the mouse.
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *event = static_cast<QMouseEvent *>(e);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QPointF pos = m_walletEntriesWidget->mapFromGlobal(event->globalPosition());
            QListWidgetItem *item = m_walletEntriesWidget->itemAt(pos.toPoint());
#else
            QPoint pos = m_walletEntriesWidget->mapFromGlobal(event->globalPos());
            QListWidgetItem *item = m_walletEntriesWidget->itemAt(pos);
#endif

            m_editButton->setEnabled(item != nullptr);
            m_removeButton->setEnabled(item != nullptr);

            if (!item) {
                m_walletEntriesWidget->clearSelection();
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
    m_useDefaultLogin->setEnabled(checked);
    m_walletEntriesEditor->setEnabled(checked);
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

                if (m_entriesLoaded) {
                    loadLoginCredentials();
                }
            } else {
                m_useDefaultLogin->setChecked(false);
            }
        }
    }
}

void Smb4KConfigPageAuthentication::slotLoadButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (!m_entriesLoaded) {
        loadLoginCredentials();
    }

    m_loadButton->setEnabled(false);
    m_walletEntriesWidget->setFocus();
}

void Smb4KConfigPageAuthentication::slotSaveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (m_entriesLoaded) {
        saveLoginCredentials();
    }

    m_editButton->setEnabled(false);
    m_removeButton->setEnabled(false);
    m_clearButton->setEnabled((m_walletEntriesWidget->count() != 0));

    m_walletEntriesWidget->clearSelection();

    Q_EMIT walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotEditButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    KPasswordDialog dlg(this, KPasswordDialog::ShowUsernameLine);

    if (m_walletEntriesWidget->currentItem()) {
        Smb4KAuthInfo *authInfo = nullptr;

        for (Smb4KAuthInfo *walletEntry : qAsConst(m_entriesList)) {
            // The following check also finds the default login, because it has an empty URL.
            if (m_walletEntriesWidget->currentItem()->data(Qt::UserRole).toUrl() == walletEntry->url()) {
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

    for (int i = 0; i < m_entriesList.size(); ++i) {
        // The following check also finds the default login, because it has an empty URL.
        if (m_walletEntriesWidget->currentItem()->data(Qt::UserRole).toUrl() == m_entriesList.at(i)->url()) {
            switch (m_entriesList.at(i)->type()) {
            case UnknownNetworkItem: {
                m_useDefaultLogin->setChecked(false);
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

    delete m_walletEntriesWidget->currentItem();

    m_clearButton->setEnabled((m_walletEntriesWidget->count() != 0));

    Q_EMIT walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotClearButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    while (m_walletEntriesWidget->count() != 0) {
        delete m_walletEntriesWidget->item(0);
    }

    while (!m_entriesList.isEmpty()) {
        delete m_entriesList.takeFirst();
    }

    m_clearButton->setEnabled(false);

    m_useDefaultLogin->setChecked(false);

    Q_EMIT walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotResetButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (m_entriesLoaded) {
        loadLoginCredentials();
    }

    Q_EMIT walletEntriesModified();

    m_clearButton->setEnabled((m_walletEntriesWidget->count() != 0));
}

void Smb4KConfigPageAuthentication::slotEnableResetButton()
{
    bool changed = loginCredentialsChanged();
    m_resetButton->setEnabled(changed);
}

void Smb4KConfigPageAuthentication::slotWalletItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);

    slotEditButtonClicked(false);
}
