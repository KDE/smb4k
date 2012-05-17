/***************************************************************************
    smb4kauthoptions  -  The configuration page for the authentication
    settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2012 by Alexander Reinholdt
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

#ifndef SMB4KAUTHOPTIONS_H
#define SMB4KAUTHOPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Application specific includes
#include "core/smb4kauthinfo.h"

// Qt includes
#include <QTableWidget>
#include <QList>
#include <QCheckBox>

// KDE includes
#include <ktabwidget.h>
#include <klistwidget.h>
#include <kpushbutton.h>
#include <kactioncollection.h>
#include <kactionmenu.h>

/**
 * This is the configuration tab for the authentication settings
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */


class Smb4KAuthOptions : public KTabWidget
{
  Q_OBJECT

  public:
    /**
     * Tabs enumeration
     */
    enum Tabs{ GeneralTab = 0,
               WalletEntriesTab = 1 };
               
    /**
     * The constructor.
     *
     * @param parent          The parent widget
     */
    Smb4KAuthOptions( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KAuthOptions();
    
    /**
     * Insert the list of authentication information entries into the internal
     * list of wallet entries. This function will not display the entries. You
     * need to call displayWalletEntries() for this.
     * 
     * @param entries       The list of entries
     */
    void insertWalletEntries( const QList<Smb4KAuthInfo *> &entries );
    
    /**
     * Get the - maybe modified - entries.
     * 
     * @returns the list of entries.
     */
    const QList<Smb4KAuthInfo *> &getWalletEntries() { return m_entries_list; }
    
    /**
     * Display the autentication information in the list widget.
     * Use this function AFTER setEntries() if necessary.
     */
    void displayWalletEntries();
    
    /**
     * Returns TRUE if the wallet entries are displayed and FALSE otherwise.
     * 
     * @returns TRUE if the wallet entries are displayed
     */
    bool walletEntriesDisplayed() { return m_entries_displayed; }
    
    /**
     * Returns TRUE if a removal in the list widget is undone and FALSE
     * otherwise.
     * 
     * @returns TRUE a removal is undown.
     */
    bool undoRemoval() { return m_undo_removal; }
    
    /**
     * Returns TRUE in the case the wallet entries might have changed. You need
     * to check this outside this widget, whether a change indeed occurred.
     * 
     * @returns TRUE if the wallet entries might have changed.
     */
    bool walletEntriesMaybeChanged() { return m_maybe_changed; }
    
  signals:
    /**
     * Emitted when the "Load" button is clicked.
     */
    void loadWalletEntries();
    
    /**
     * Emitted when the "Save" button is clicked.
     */
    void saveWalletEntries();
    
    /**
     * Emitted when the default login should be (re-)defined.
     */
    void setDefaultLogin();
    
    /**
     * This signal is emitted everytime the wallet entries potentially were 
     * modified by the user. When this signal is emitted, it does not necessarily 
     * mean that any wallet entry indeed changed. It only means that the user 
     * edited one.
     */
    void walletEntriesModified();
    
  protected:
    /**
     * Reimplemented.
     */
    bool eventFilter( QObject *object,
                      QEvent *event );
    
  protected slots:
    /**
     * This slot is called when the "Use wallet" check box is toggled.
     *
     * @param checked       TRUE if the check box is checked and otherwise
     *                      FALSE.
     */
    void slotKWalletButtonToggled( bool checked );

    /**
     * This slot is invoked when the "Default login" check box is toggled.
     *
     * @param checked       TRUE if the check box is checked and otherwise
     *                      FALSE.
     */
    void slotDefaultLoginToggled( bool checked );
    
    /**
     * This slot is connected to the "Details" button and shows the details
     * of the selected wallet entry.
     * 
     * @param checked       TRUE if the button is checked
     */
    void slotDetailsClicked( bool checked );
    
    /**
     * This slot is connected to the KListWidget::itemSelectionChanged() signal.
     * It unmarks and enables/disables the "Show details" checkbox and clears the
     * the details widget.
     */
    void slotItemSelectionChanged();
    
    /**
     * This slot is connected to the QTableWidget::cellChanged() signal and commits
     * changes the user applied to the entries to the internal list and enables the
     * "Undo" action. 
     *
     * @param row             The row of the cell that was changed
     * 
     * @param column          The column of the cell that was changed
     */
    void slotDetailsChanged( int row,
                             int column );
    
    /**
     * This slot is connected to the KListWidget::customContextMenuRequested()
     * signal and shows the menu in the list widget.
     */
    void slotShowListWidgetContextMenu( const QPoint &pos );
    
    /**
     * This slot is connected to the QTableWidget::customContextMenuRequested()
     * signal and shows the menu in the table widget.
     */
    void slotShowTableWidgetContextMenu( const QPoint &pos );
    
    /**
     * This slot is connected to the "Remove" action
     * 
     * @param checked         TRUE if the action is checked
     */
    void slotRemoveActionTriggered( bool checked );
    
    /**
     * This slot is connected to the "Clear List" action
     * 
     * @param checked         TRUE if the action is checked
     */
    void slotClearActionTriggered( bool checked );
    
    /**
     * This slot is connected to the "Undo" action in the list widget.
     * 
     * @param checked         TRUE if the action is checked
     */
    void slotUndoListActionTriggered( bool checked );
    
    /**
     * This slot is connected to the "Edit" action in the table widget.
     * 
     * @param checked         TRUE if the action is checked
     */
    void slotEditActionTriggered( bool checked );
    
    /**
     * This slot is connected to the "Undo" action in the table widget.
     * 
     * @param checked         TRUE if the action is checked
     */
    void slotUndoDetailsActionTriggered( bool checked );
    
    /**
     * This slot is connected to the "Save" button and resets all actions.
     * 
     * @param checked         TRUE if the action is checked
     */
    void slotSaveClicked( bool checked );
    
  private:
    void showDetails( Smb4KAuthInfo *authInfo );
    void clearDetails();
    KListWidget *m_entries_widget;
    KPushButton *m_load_button;
    KPushButton *m_save_button;
    QCheckBox *m_details_box;
    QTableWidget *m_details_widget;
    QList<Smb4KAuthInfo *> m_entries_list;
    bool m_entries_displayed;
    KActionCollection *m_collection;
    KActionMenu *m_entries_menu;
    KActionMenu *m_details_menu;
    Smb4KAuthInfo *m_auth_info;
    bool m_loading_details;
    bool m_default_login;
    bool m_undo_removal;
    bool m_maybe_changed;
};

#endif
