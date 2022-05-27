/*
    The configuration page for the authentication settings of Smb4K

    SPDX-FileCopyrightText: 2003-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpageauthentication.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QAction>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KCollapsibleGroupBox>

Smb4KConfigPageAuthentication::Smb4KConfigPageAuthentication(QWidget *parent)
    : QWidget(parent)
{
    m_entries_displayed = false;
    m_maybe_changed = false;

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

    QCheckBox *defaultAuth = new QCheckBox(Smb4KSettings::self()->useDefaultLoginItem()->label(), settingsBox);
    defaultAuth->setObjectName("kcfg_UseDefaultLogin");

    connect(defaultAuth, SIGNAL(toggled(bool)), this, SLOT(slotDefaultLoginToggled(bool)));

    settingsBoxLayout->addWidget(defaultAuth);

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
    QGridLayout *walletEntriesEditorLayout = new QGridLayout(walletEntriesEditor);
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

    //     // Add action
    //     QAction *addAction = new QAction(KDE::icon("list-add"), i18n("Add"), walletEntriesWidget);
    //     addAction->setObjectName("AddAction");
    //     addAction->setEnabled(false);
    //     // FIXME: Connection
    //     qDebug() << "Make add action work";
    //     walletEntriesWidget->addAction(addAction);
    //
    //     // Add Default action
    //     QAction *addDefaultAction = new QAction(KDE::icon("list-add"), i18n("Add Default"), walletEntriesWidget);
    //     addDefaultAction->setObjectName("AddDefaultAction");
    //     addDefaultAction->setEnabled(false);
    //     // FIXME: Connection
    //     qDebug() << "Make add default action work";
    //     walletEntriesWidget->addAction(addDefaultAction);

    // Edit action
    QAction *editAction = new QAction(KDE::icon("edit-rename"), i18n("Edit"), walletEntriesWidget);
    editAction->setObjectName("EditAction");
    editAction->setEnabled(false);
    connect(editAction, SIGNAL(triggered(bool)), this, SLOT(slotEditClicked()));
    walletEntriesWidget->addAction(editAction);

    // Remove action
    QAction *removeAction = new QAction(KDE::icon("edit-delete"), i18n("Remove"), walletEntriesWidget);
    removeAction->setObjectName("RemoveAction");
    removeAction->setEnabled(false);
    connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(slotRemoveClicked()));
    walletEntriesWidget->addAction(removeAction);

    // Clear action
    QAction *clearAction = new QAction(KDE::icon("edit-clear-list"), i18n("Clear"), walletEntriesWidget);
    clearAction->setObjectName("ClearAction");
    clearAction->setEnabled(false);
    connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(slotClearClicked()));
    walletEntriesWidget->addAction(clearAction);

    connect(walletEntriesWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));

    walletEntriesEditorLayout->addWidget(walletEntriesWidget, 0, 0, 7, 1);

    //
    // Load button
    //
    QPushButton *loadButton = new QPushButton(walletEntriesEditor);
    loadButton->setObjectName("LoadButton");
    loadButton->setText(i18n("Load"));
    loadButton->setIcon(KDE::icon("document-open"));
    loadButton->setWhatsThis(i18n("The login information that was stored by Smb4K will be loaded from the wallet."));

    connect(loadButton, SIGNAL(clicked(bool)), this, SIGNAL(loadWalletEntries()));

    walletEntriesEditorLayout->addWidget(loadButton, 0, 1);

    //
    // Save button
    //
    QPushButton *saveButton = new QPushButton(walletEntriesEditor);
    saveButton->setObjectName("SaveButton");
    saveButton->setText(i18n("Save"));
    saveButton->setIcon(KDE::icon("document-save-all"));
    saveButton->setWhatsThis(i18n("All modifications you applied are saved to the wallet."));
    saveButton->setEnabled(false);

    connect(saveButton, SIGNAL(clicked(bool)), this, SIGNAL(saveWalletEntries()));
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveClicked(bool)));

    walletEntriesEditorLayout->addWidget(saveButton, 1, 1);
    walletEntriesEditorLayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 2, 1);

    //
    // The details widget
    //
    KCollapsibleGroupBox *detailsBox = new KCollapsibleGroupBox(walletEntriesEditor);
    detailsBox->setObjectName("DetailsBox");
    detailsBox->setTitle(i18n("Details"));
    detailsBox->setEnabled(false);
    QVBoxLayout *detailsBoxLayout = new QVBoxLayout(detailsBox);

    QTableWidget *detailsWidget = new QTableWidget(detailsBox);
    detailsWidget->setObjectName("DetailsWidget");
    detailsWidget->horizontalHeader()->setVisible(false);
    detailsWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    detailsWidget->verticalHeader()->setVisible(false);
    detailsWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    detailsWidget->viewport()->installEventFilter(this);

    detailsBoxLayout->addWidget(detailsWidget, 0);

    walletEntriesEditorLayout->addWidget(detailsBox, 3, 1);
    walletEntriesEditorLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 4, 1);

    walletEntriesBoxLayout->addWidget(walletEntriesEditor, 0);

    layout->addWidget(walletEntriesBox);

    //
    // Adjustments
    //
    slotKWalletButtonToggled(useWallet->isChecked());
    slotDefaultLoginToggled(defaultAuth->isChecked());

    //
    // Set focus
    //
    loadButton->setFocus();
}

Smb4KConfigPageAuthentication::~Smb4KConfigPageAuthentication()
{
}

void Smb4KConfigPageAuthentication::insertLoginCredentials(const QList<Smb4KAuthInfo *> &list)
{
    //
    // Insert the list of authentication information
    //
    m_entriesList = list;

    //
    // Reset the changed flag, since we are (re)loading the information
    //
    m_maybe_changed = false;
    emit walletEntriesModified();

    //
    // Get the list wirdget
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
            (void)new QListWidgetItem(KDE::icon("dialog-password"), i18n("Default Login"), walletEntriesWidget);
            break;
        }
        default: {
            (void)new QListWidgetItem(KDE::icon("dialog-password"), authInfo->displayString(), walletEntriesWidget);
            break;
        }
        }
    }

    //
    // Sort the entries
    //
    walletEntriesWidget->sortItems();

    //
    // Set the display flag to true
    //
    m_entries_displayed = true;

    //
    // Enable buttons and actions
    //
    findChild<QPushButton *>("SaveButton")->setEnabled(walletEntriesWidget->count() != 0);
    findChild<QAction *>("ClearAction")->setEnabled(walletEntriesWidget->count() != 0);
}

const QList<Smb4KAuthInfo *> &Smb4KConfigPageAuthentication::getLoginCredentials()
{
    return m_entriesList;
}

bool Smb4KConfigPageAuthentication::loginCredentialsDisplayed()
{
    return m_entries_displayed;
}

bool Smb4KConfigPageAuthentication::loginCredentialsMaybeChanged()
{
    return m_maybe_changed;
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

            if (!walletEntriesWidget->itemAt(pos)) {
                clearDetails();
                walletEntriesWidget->clearSelection();
                findChild<QAction *>("EditAction")->setEnabled(false);
                findChild<QAction *>("RemoveAction")->setEnabled(false);
            }
        }

        return walletEntriesWidget->viewport()->eventFilter(object, e);
    }

    return QWidget::eventFilter(object, e);
}

void Smb4KConfigPageAuthentication::loadDetails(Smb4KAuthInfo *authInfo)
{
    //
    // Get the widgets
    //
    QTableWidget *detailsWidget = findChild<QTableWidget *>("DetailsWidget");
    KCollapsibleGroupBox *detailsGroupBox = findChild<KCollapsibleGroupBox *>("DetailsBox");
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Fill the details table widget with the information
    //
    switch (authInfo->type()) {
    case Host:
    case Share: {
        detailsWidget->setColumnCount(2);
        detailsWidget->setRowCount(4);

        QTableWidgetItem *entry_label = new QTableWidgetItem(i18n("Entry"));
        entry_label->setFlags(entry_label->flags() & Qt::ItemIsEditable);
        entry_label->setForeground(palette().text());

        QTableWidgetItem *entry = new QTableWidgetItem(authInfo->displayString());
        entry->setFlags(entry->flags() & Qt::ItemIsEditable);
        entry->setForeground(palette().text());

        QTableWidgetItem *login_label = new QTableWidgetItem(i18n("Username"));
        login_label->setFlags(login_label->flags() & Qt::ItemIsEditable);
        login_label->setForeground(palette().text());

        QTableWidgetItem *password_label = new QTableWidgetItem(i18n("Password"));
        password_label->setFlags(password_label->flags() & Qt::ItemIsEditable);
        password_label->setForeground(palette().text());

        detailsWidget->setItem(0, 0, entry_label);
        detailsWidget->setItem(0, 1, entry);
        detailsWidget->setItem(1, 0, login_label);
        detailsWidget->setItem(1, 1, new QTableWidgetItem(authInfo->userName()));
        detailsWidget->setItem(2, 0, password_label);
        detailsWidget->setItem(2, 1, new QTableWidgetItem(authInfo->password()));

        break;
    }
    default: {
        detailsWidget->setColumnCount(2);
        detailsWidget->setRowCount(3);

        QTableWidgetItem *entry_label = new QTableWidgetItem(i18n("Entry"));
        entry_label->setFlags(entry_label->flags() & Qt::ItemIsEditable);
        entry_label->setForeground(palette().text());

        QTableWidgetItem *entry = new QTableWidgetItem(i18n("Default Login"));
        entry->setFlags(entry->flags() & Qt::ItemIsEditable);
        entry->setForeground(palette().text());

        QTableWidgetItem *login_label = new QTableWidgetItem(i18n("Username"));
        login_label->setFlags(login_label->flags() & Qt::ItemIsEditable);
        login_label->setForeground(palette().text());

        QTableWidgetItem *password_label = new QTableWidgetItem(i18n("Password"));
        password_label->setFlags(password_label->flags() & Qt::ItemIsEditable);
        password_label->setForeground(palette().text());

        detailsWidget->setItem(0, 0, entry_label);
        detailsWidget->setItem(0, 1, entry);
        detailsWidget->setItem(1, 0, login_label);
        detailsWidget->setItem(1, 1, new QTableWidgetItem(authInfo->userName()));
        detailsWidget->setItem(2, 0, password_label);
        detailsWidget->setItem(2, 1, new QTableWidgetItem(authInfo->password()));

        break;
    }
    }

    //
    // Connect signals
    //
    connect(detailsWidget, SIGNAL(cellChanged(int, int)), this, SLOT(slotDetailsChanged(int, int)));

    //
    // Enable the details box
    //
    detailsGroupBox->setEnabled(!walletEntriesWidget->selectedItems().isEmpty());
}

void Smb4KConfigPageAuthentication::clearDetails()
{
    //
    // Get the widgets
    //
    QTableWidget *detailsWidget = findChild<QTableWidget *>("DetailsWidget");
    KCollapsibleGroupBox *detailsGroupBox = findChild<KCollapsibleGroupBox *>("DetailsBox");
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Disconnect signals
    //
    disconnect(detailsWidget, SIGNAL(cellChanged(int, int)), this, SLOT(slotDetailsChanged(int, int)));

    //
    // Collapse the details box and disable it.
    //
    detailsGroupBox->setExpanded(false);
    detailsGroupBox->setEnabled(!walletEntriesWidget->selectedItems().isEmpty());

    //
    // Clear the table widget
    //
    if (detailsWidget->rowCount() != 0 && detailsWidget->columnCount() != 0) {
        detailsWidget->clear();
        detailsWidget->setRowCount(0);
        detailsWidget->setColumnCount(0);
    }
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
        emit setDefaultLogin();
    }
}

void Smb4KConfigPageAuthentication::slotItemSelectionChanged()
{
    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Clear details widget
    //
    clearDetails();

    //
    // Get the authentication information and load its
    // details into the details widget
    //
    if (walletEntriesWidget->currentItem()) {
        for (Smb4KAuthInfo *authInfo : qAsConst(m_entriesList)) {
            if (walletEntriesWidget->currentItem()->text() == authInfo->displayString()
                || (walletEntriesWidget->currentItem()->text() == i18n("Default Login") && authInfo->type() == UnknownNetworkItem)) {
                loadDetails(authInfo);
                break;
            }
        }

        // Enable actions
        findChild<QAction *>("EditAction")->setEnabled(true);
        findChild<QAction *>("RemoveAction")->setEnabled(true);
    }
}

void Smb4KConfigPageAuthentication::slotDetailsChanged(int row, int column)
{
    //
    // Get the widget
    //
    QTableWidget *detailsWidget = findChild<QTableWidget *>("DetailsWidget");

    //
    // Find the right authentication information and pass the modifications
    //
    for (Smb4KAuthInfo *authInfo : qAsConst(m_entriesList)) {
        if (QString::compare(detailsWidget->item(0, 1)->text(), authInfo->displayString()) == 0
            || (QString::compare(detailsWidget->item(0, 1)->text(), i18n("Default Login")) == 0 && authInfo->type() == UnknownNetworkItem)) {
            switch (authInfo->type()) {
            case Host:
            case Share: {
                if (column == 1) {
                    switch (row) {
                    case 1: // Login
                    {
                        authInfo->setUserName(detailsWidget->item(row, column)->text());
                        break;
                    }
                    case 2: // Password
                    {
                        authInfo->setPassword(detailsWidget->item(row, column)->text());
                        break;
                    }
                    default: {
                        break;
                    }
                    }
                }

                break;
            }
            default: {
                if (column == 1) {
                    switch (row) {
                    case 1: // Login
                    {
                        authInfo->setUserName(detailsWidget->item(row, column)->text());
                        break;
                    }
                    case 2: // Password
                    {
                        authInfo->setPassword(detailsWidget->item(row, column)->text());
                        break;
                    }
                    default: {
                        break;
                    }
                    }
                }

                break;
            }
            }

            break;
        }
    }

    //
    // Tell the program that the authentication information may be changed
    // and emit the appropriate signal
    //
    m_maybe_changed = true;
    emit walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotEditClicked()
{
    //
    // Get the widgets
    //
    KCollapsibleGroupBox *detailsGroupBox = findChild<KCollapsibleGroupBox *>("DetailsBox");
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    if (walletEntriesWidget->currentItem()) {
        //
        // Since the details have been loaded to the details widget already
        // by slotItemSelectionChanged(), only open the details widget here.
        //
        if (!detailsGroupBox->isExpanded()) {
            detailsGroupBox->setExpanded(true);
        }
    }
}

void Smb4KConfigPageAuthentication::slotRemoveClicked()
{
    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Clear the details widget
    //
    clearDetails();

    //
    // Remove the appropriate entry from the list of authentication information
    //
    for (int i = 0; i < m_entriesList.size(); ++i) {
        if (QString::compare(walletEntriesWidget->currentItem()->text(), m_entriesList.at(i)->displayString()) == 0
            || (QString::compare(walletEntriesWidget->currentItem()->text(), i18n("Default Login")) == 0
                && m_entriesList.at(i)->type() == UnknownNetworkItem)) {
            switch (m_entriesList.at(i)->type()) {
            case UnknownNetworkItem: {
                QCheckBox *default_login = findChild<QCheckBox *>("kcfg_UseDefaultLogin");
                default_login->setChecked(false);
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
    findChild<QAction *>("ClearAction")->setEnabled((walletEntriesWidget->count() != 0));

    //
    // Tell the program that the authentication information may be changed
    // and emit the appropriate signal
    //
    m_maybe_changed = true;
    emit walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotClearClicked()
{
    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Clear the details widget
    //
    clearDetails();

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
    findChild<QAction *>("ClearAction")->setEnabled(false);

    //
    // Uncheck the Default Login checkbox
    //
    findChild<QCheckBox *>("kcfg_UseDefaultLogin")->setChecked(false);

    //
    // Tell the program that the authentication information may be changed
    // and emit the appropriate signal
    //
    m_maybe_changed = true;
    emit walletEntriesModified();
}

void Smb4KConfigPageAuthentication::slotSaveClicked(bool /*checked*/)
{
    //
    // Get the list widget
    //
    QListWidget *walletEntriesWidget = findChild<QListWidget *>("WalletEntriesWidget");

    //
    // Disable buttons
    //
    findChild<QAction *>("EditAction")->setEnabled(false);
    findChild<QAction *>("RemoveAction")->setEnabled(false);
    findChild<QAction *>("ClearAction")->setEnabled((walletEntriesWidget->count() != 0));

    //
    // Clear the selection in the list view
    //
    walletEntriesWidget->clearSelection();

    //
    // Tell the program that the authentication information may be changed
    // and emit the appropriate signal
    //
    m_maybe_changed = false;
    emit walletEntriesModified();
}
