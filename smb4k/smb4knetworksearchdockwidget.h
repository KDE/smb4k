/***************************************************************************
    The network search widget dock widget 
                             -------------------
    begin                : Mon Apr 30 2018
    copyright            : (C) 2018 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KNETWORKSEARCHDOCKWIDGET_H
#define SMB4KNETWORKSEARCHDOCKWIDGET_H

// application specific includes
#include "smb4knetworksearch.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QDockWidget>

// KDE includes
#include <KWidgetsAddons/KActionMenu>
#include <KXmlGui/KActionCollection>

using namespace Smb4KGlobal;

class Smb4KNetworkSearchDockWidget : public QDockWidget
{
  Q_OBJECT
  
  public:
    /**
    * Constructor
    */
    Smb4KNetworkSearchDockWidget(const QString &title, QWidget *parent = 0);
    
    /**
    * Destructor
    */
    ~Smb4KNetworkSearchDockWidget();
    
    /**
     * Load settings
     */
    void loadSettings();

    /**
     * Save settings
     */
    void saveSettings();
    
    /**
     * Returns the action collection of this dock widget
     * @returns the action collection
     */
    KActionCollection *actionCollection();
    
  protected Q_SLOTS:
    /**
     * This slot is invoked when the user pressed the return keyboard key. It
     * forwards the search request to the slotSearchTriggered() slot.
     */
    void slotReturnPressed();
    
    /**
     * This slot is invoked when the text in the combo box changed. It enables
     * or disables some actions.
     * @param text                The text in the line edit of the combo box.
     */
    void slotComboBoxTextChanged(const QString &text);
    
    /**
     * This slot is invoked when the user activated an item in the network
     * search list widget.
     * @param item                The item that was activated.
     */
    void slotItemActivated(QListWidgetItem *item);
    
    /**
     * This slot is invoked when the item selection changed. It enables and
     * disables some actions.
     */
    void slotItemSelectionChanged();
    
    /**
     * This slot is invoked, when the context menu of the search dialog is
     * requested.
     * @param pos                 The position where the context menu should
     *                            be opened.
     */
    void slotContextMenuRequested(const QPoint &pos);
    
    /**
     * This slot retrieves the search result and puts it into the search
     * dialog.
     * @param item                A share item
     */
    void slotReceivedSearchResult(const SharePtr &share);
    
    /**
     * This slot is connected to the Smb4KSearch::aboutToStart() signal.
     * @param string              The search string for that the search is to
     *                            be performed.
     */
    void slotSearchAboutToStart(const QString &string);

    /**
     * This slot is connected to the Smb4KSearch::finished() signal.
     * @param string              The search string for that the search was
     *                            performed.
     */
    void slotSearchFinished(const QString &string);
    
    /**
     * This slot is connected to the Smb4KMounter::mounted() signal and marks
     * a share as mounted.
     * @param share               The share item
     */
    void slotShareMounted(const SharePtr &share);
    
    /**
     * This slot is connected to the Smb4KMounter::unmounted() signal and unmarks
     * a share that was just unmounted.
     * @param share               The share item
     */
    void slotShareUnmounted(const SharePtr &share);
    
    /**
     * This action is invoked when the Search/Abort dual action is triggered
     * @param checked             TRUE if the action is checked
     */
    void slotSearchAbortActionTriggered(bool checked);
    
    /**
     * This action is invoked when the active state of the Search/Abort
     * dual action changed.
     * @param active              active or inactive
     */
    void slotSearchAbortActionChanged(bool active);
    
    /**
     * This slot is invoked when the Clear action is triggered. It clears the
     * combo box and the list widget and disables the actions.
     * @param checked             TRUE if the action is checked.
     */
    void slotClearActionTriggered(bool checked);
    
    /**
     * This slot is connected to the 'Add Bookmark' action. It lets you add a share 
     * to the bookmarks.
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotBookmarkActionTriggered(bool checked);
    
    /**
     * This slot is invoked when the Mount/Unmount action is triggered. It 
     * mounts/unmounts the selected share.
     * @param checked             TRUE if the action is checked.
     */
    void slotMountActionTriggered(bool checked);

    /**
     * Change the state of the 'Mount'/'Unmount' dual action.
     * @param active              TRUE if the action is in the active state.
     */
    void slotMountActionChanged(bool active);
    
    /**
     * This slot is called if the icon size was changed.
     * @param group               The icon group
     */
    void slotIconSizeChanged(int group);
    
  private:
    /**
     * Set up the actions
     */
    void setupActions();
    
    /**
     * The network search widget
     */
    Smb4KNetworkSearch *m_networkSearch;

    /**
     * Action collection
     */
    KActionCollection *m_actionCollection;
    
    /**
     * The context menu
     */
    KActionMenu *m_contextMenu;
};

#endif
