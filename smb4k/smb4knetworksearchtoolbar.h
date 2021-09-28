/*
    This class provides the network search toolbar.
    -------------------
    begin                : Su Dec 23 2018
    SPDX-FileCopyrightText: 2018-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
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

class Smb4KNetworkSearchToolBar : public QToolBar
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    Smb4KNetworkSearchToolBar(QWidget *parent = 0);

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
     * Set the completion strings. This function should be invoked before the
     * search toolbar is shown.
     *
     * @param strings         The list of completion strings
     */
    void setCompletionStrings(const QStringList &strings);

    /**
     * Get the completion strings.
     *
     * @returns the completion strings
     */
    QStringList completionStrings() const;

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
    void slotReturnKeyPressed();

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
    /**
     * The search results
     */
    QStringList m_searchResults;

    /**
     * String list iterator
     */
    QStringListIterator m_iterator;
};

#endif
