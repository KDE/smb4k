/*
    This class provides the network search toolbar.

    SPDX-FileCopyrightText: 2018-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knetworksearchtoolbar.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QDebug>

// KDE includes
#include <KComboBox>
#include <KDualAction>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>

Smb4KNetworkSearchToolBar::Smb4KNetworkSearchToolBar(QWidget *parent)
    : QToolBar(parent)
    , m_iterator(QStringList())
{
    //
    // Set up tool bar
    //
    // Use the settings suggested by the note provided in the 'Detailed Description'
    // section of KToolBar (https://api.kde.org/frameworks/kxmlgui/html/classKToolBar.html)
    //
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setProperty("otherToolbar", true);

    //
    // The Close action
    //
    QAction *closeAction = new QAction(this);
    closeAction->setObjectName(QStringLiteral("CloseAction"));
    closeAction->setIcon(KDE::icon(QStringLiteral("window-close")));
    closeAction->setText(i18n("Close"));

    connect(closeAction, SIGNAL(triggered(bool)), this, SLOT(slotCloseButtonPressed()));

    addAction(closeAction);

    //
    // The search combo box
    //
    KComboBox *comboBox = new KComboBox(true, this);
    comboBox->setObjectName(QStringLiteral("SearchCombo"));
    comboBox->lineEdit()->setPlaceholderText(i18n("Search string"));
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    comboBox->setCompletionMode(KCompletion::CompletionPopupAuto);

    connect(comboBox, SIGNAL(returnPressed(QString)), this, SLOT(slotReturnKeyPressed(QString)));
    // FIXME: Add a connection to the clearSearch() slot

    addWidget(comboBox);

    //
    // The search dual action
    //
    KDualAction *searchAction = new KDualAction(this);
    searchAction->setObjectName(QStringLiteral("SearchAction"));
    searchAction->setInactiveIcon(KDE::icon(QStringLiteral("search")));
    searchAction->setInactiveText(i18n("Search"));
    searchAction->setActiveIcon(KDE::icon(QStringLiteral("process-stop")));
    searchAction->setActiveText(i18n("Stop"));
    searchAction->setAutoToggle(false);

    connect(searchAction, SIGNAL(triggered(bool)), this, SLOT(slotSearchActionTriggered()));

    addAction(searchAction);

    //
    // Go one item down action
    //
    QAction *downAction = new QAction(this);
    downAction->setObjectName(QStringLiteral("DownAction"));
    downAction->setIcon(KDE::icon(QStringLiteral("go-down-search")));
    downAction->setText(i18n("Item Down"));
    downAction->setEnabled(false);

    connect(downAction, SIGNAL(triggered(bool)), this, SLOT(slotDownActionTriggered()));

    addAction(downAction);

    //
    // Go one item up action
    //
    QAction *upAction = new QAction(this);
    upAction->setObjectName(QStringLiteral("UpAction"));
    upAction->setIcon(KDE::icon(QStringLiteral("go-up-search")));
    upAction->setText(i18n("Item Up"));
    upAction->setEnabled(false);

    connect(upAction, SIGNAL(triggered(bool)), this, SLOT(slotUpActionTriggered()));

    addAction(upAction);

    /**
     * Clear the search
     */
    QAction *clearAction = new QAction(this);
    clearAction->setObjectName(QStringLiteral("ClearAction"));
    clearAction->setIcon(KDE::icon(QStringLiteral("edit-clear-all")));
    clearAction->setText(i18n("Clear"));
    clearAction->setEnabled(false);

    connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(slotClearSearch()));

    addAction(clearAction);
}

Smb4KNetworkSearchToolBar::~Smb4KNetworkSearchToolBar()
{
}

void Smb4KNetworkSearchToolBar::prepareInput()
{
    //
    // Get the search combo box
    //
    KComboBox *comboBox = findChild<KComboBox *>(QStringLiteral("SearchCombo"));

    //
    // Set the keyboard focus to the lineedit
    //
    comboBox->lineEdit()->setFocus();
}

void Smb4KNetworkSearchToolBar::setActiveState(bool active)
{
    //
    // Get the search dual action and set the active state
    //
    KDualAction *searchAction = findChild<KDualAction *>(QStringLiteral("SearchAction"));
    searchAction->setActive(active);

    //
    // Get the search combo box and disable/enable it
    //
    KComboBox *comboBox = findChild<KComboBox *>(QStringLiteral("SearchCombo"));
    comboBox->setEnabled(!active);
}

void Smb4KNetworkSearchToolBar::clearSearch()
{
    //
    // Clear the list of search results
    //
    m_searchResults.clear();

    //
    // Clear the combo box
    //
    KComboBox *comboBox = findChild<KComboBox *>(QStringLiteral("SearchCombo"));
    comboBox->clear();
    comboBox->clearEditText();

    //
    // Get the down action and disable it
    //
    QAction *downAction = findChild<QAction *>(QStringLiteral("DownAction"));
    downAction->setEnabled(!m_searchResults.isEmpty());

    //
    // Get the up action and disable it
    //
    QAction *upAction = findChild<QAction *>(QStringLiteral("UpAction"));
    upAction->setEnabled(!m_searchResults.isEmpty());

    //
    // Get the clear action and disable it
    //
    QAction *clearAction = findChild<QAction *>(QStringLiteral("ClearAction"));
    clearAction->setEnabled(!m_searchResults.isEmpty());

    //
    // Emit the clearSearchResults() signal
    //
    Q_EMIT clearSearchResults();
}

void Smb4KNetworkSearchToolBar::setSearchResults(const QList<SharePtr> &list)
{
    //
    // Set the new list
    //
    for (const SharePtr &share : list) {
        m_searchResults << share->url().toString();
    }

    //
    // Sort the search results
    //
    m_searchResults.sort();

    //
    // Set the iterator
    //
    m_iterator = m_searchResults;

    //
    // Get the down action and enable it
    //
    QAction *downAction = findChild<QAction *>(QStringLiteral("DownAction"));
    downAction->setEnabled(!m_searchResults.isEmpty());

    //
    // Get the up action and enable it
    //
    QAction *upAction = findChild<QAction *>(QStringLiteral("UpAction"));
    upAction->setEnabled(!m_searchResults.isEmpty());

    //
    // Get the clear action and enable it
    //
    QAction *clearAction = findChild<QAction *>(QStringLiteral("ClearAction"));
    clearAction->setEnabled(!m_searchResults.isEmpty());
}

void Smb4KNetworkSearchToolBar::setCompletionItems(const QStringList &items)
{
    //
    // Get the input combo box
    //
    KComboBox *comboBox = findChild<KComboBox *>(QStringLiteral("SearchCombo"));

    //
    // Set the completion strings
    //
    comboBox->completionObject()->setItems(items);
}

QStringList Smb4KNetworkSearchToolBar::completionItems() const
{
    //
    // Get the input combo box
    //
    KComboBox *comboBox = findChild<KComboBox *>(QStringLiteral("SearchCombo"));

    //
    // Return the completion strings
    //
    return comboBox->completionObject()->items();
}

void Smb4KNetworkSearchToolBar::slotReturnKeyPressed(const QString &text)
{
    if (!text.isEmpty()) {
        KComboBox *comboBox = findChild<KComboBox *>(QStringLiteral("SearchCombo"));

        if (comboBox) {
            comboBox->completionObject()->addItem(text);
            Q_EMIT search(text);
        }
    }
}

void Smb4KNetworkSearchToolBar::slotSearchActionTriggered()
{
    //
    // Get the search dual action
    //
    KDualAction *searchAction = findChild<KDualAction *>(QStringLiteral("SearchAction"));

    //
    // Initialize a search if the action is in active state and abort
    // the search if it is in inactive state
    //
    if (!searchAction->isActive()) {
        KComboBox *comboBox = findChild<KComboBox *>(QStringLiteral("SearchCombo"));

        if (!comboBox->currentText().isEmpty()) {
            //
            // Add the search item to the completion object
            //
            comboBox->completionObject()->addItem(comboBox->currentText());

            //
            // Emit the search signal
            //
            Q_EMIT search(comboBox->currentText());
        }
    } else {
        //
        // Emit the abort signal
        //
        Q_EMIT abort();
    }
}

void Smb4KNetworkSearchToolBar::slotCloseButtonPressed()
{
    //
    // Clear the search toolbar
    //
    clearSearch();

    //
    // Emit the abort signal
    //
    Q_EMIT abort();

    //
    // Emit the close signal
    //
    Q_EMIT closeSearchBar();
}

void Smb4KNetworkSearchToolBar::slotDownActionTriggered()
{
    if (m_iterator.hasNext()) {
        //
        // Get the URL of the item
        //
        QString url = m_iterator.next();

        //
        // Emit the jumpToResult() signal
        //
        Q_EMIT jumpToResult(url);
    }
}

void Smb4KNetworkSearchToolBar::slotUpActionTriggered()
{
    if (m_iterator.hasPrevious()) {
        //
        // Get the URL of the item
        //
        QString url = m_iterator.previous();

        //
        // Emit the jumpToResult() signal
        //
        Q_EMIT jumpToResult(url);
    }
}

void Smb4KNetworkSearchToolBar::slotClearSearch()
{
    clearSearch();
}
