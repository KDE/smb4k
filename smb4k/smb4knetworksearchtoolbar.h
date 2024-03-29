/*
    This class provides the network search toolbar.

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KNETWORKSEARCHTOOLBAR_H
#define SMB4KNETWORKSEARCHTOOLBAR_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QList>
#include <QListIterator>
#include <QToolBar>

// KDE includes
#include <KComboBox>
#include <KDualAction>

class Smb4KNetworkSearchToolBar : public QToolBar
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    Smb4KNetworkSearchToolBar(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KNetworkSearchToolBar();

    /**
     * Sets the focus to the search combo box
     */
    void prepareInput();

    /**
     * Set the active state
     *
     * @param active          The state
     */
    void setActiveState(bool active);

    /**
     * Set the search result
     *
     * @param list            The list of search results
     */
    void setSearchResults(const QList<SharePtr> &list);

    /**
     * Clear the search
     */
    void clearSearch();

    /**
     * Set the completion items. This function should be invoked before the
     * search toolbar is shown.
     *
     * @param items         The list of completion strings
     */
    void setCompletionItems(const QStringList &items);

    /**
     * Get the completion strings.
     *
     * @returns the completion strings
     */
    QStringList completionItems() const;

Q_SIGNALS:
    /**
     * Emitted when the search toolbar is to be closed (should be hidden)
     */
    void closeSearchBar();

    /**
     * Emitted when a search should be done
     */
    void search(const QString &item);

    /**
     * Emitted when a search should be stopped
     */
    void abort();

    /**
     * Emitted when either the up or down action was clicked
     */
    void jumpToResult(const QString &url);

    /**
     * Emitted when the search is cleared
     */
    void clearSearchResults();

protected Q_SLOTS:
    /**
     * Called when the return key was pressed
     */
    void slotReturnKeyPressed(const QString &text);

    /**
     * Called when the search dual action is toggled
     */
    void slotSearchActionTriggered();

    /**
     * Called when the close button was pressed
     */
    void slotCloseButtonPressed();

    /**
     * Called when the down action was triggered
     */
    void slotDownActionTriggered();

    /**
     * Called when the up action was triggered
     */
    void slotUpActionTriggered();

    /**
     * Called when the search is cleared
     */
    void slotClearSearch();

private:
    QStringList m_searchResults;
    QStringListIterator m_iterator;
    KComboBox *m_searchComboBox;
    KDualAction *m_searchAction;
    QAction *m_downAction;
    QAction *m_upAction;
    QAction *m_clearAction;
};

#endif
