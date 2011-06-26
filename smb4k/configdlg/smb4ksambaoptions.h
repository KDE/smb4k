/***************************************************************************
    smb4ksambaoptions.cpp  -  This is the configuration page for the
    Samba settings of Smb4K
                             -------------------
    begin                : Mo Jan 26 2004
    copyright            : (C) 2004-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSAMBAOPTIONS_H
#define SMB4KSAMBAOPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QList>
#include <QEvent>
#include <QCloseEvent>
#include <QListWidgetItem>
#include <QLabel>
#include <QCheckBox>

// KDE includes
#include <ktabwidget.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <klistwidget.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kcombobox.h>

// application specific includes
#include <smb4kcustomoptions.h>

/**
 * This class manages the configuration dialog of the options
 * that can be passed to smbmount and other programs of the
 * Samba software suite.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */


class Smb4KSambaOptions : public KTabWidget
{
  Q_OBJECT

  public:
    enum Tabs{ GeneralTab = 0,
               MountingTab = 1,
               ClientProgramsTab = 2,
               CustomOptionsTab = 3 };

    /**
     * The constructor.
     *
     * @param parent            The parent widget
     *
     * @param name              This widget's name
     */
    Smb4KSambaOptions( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KSambaOptions();

    /**
     * This enumeration marks the type of the custom options item
     * in the tree widget.
     */
    enum ItemType{ Host,
                   Share };

    /**
     * This function inserts a list of custom option items into the list widget
     * of the 'Custom Options' tab.
     *
     * @param list              The list with Smb4KSambaOptionsInfo objects
     */
    void insertCustomOptions( const QList<Smb4KCustomOptions *> &list );

    /**
     * This function returns the list of custom option items that are currently
     * in the 'Custom Options' tree widget.
     *
     * @returns the list of custom option items.
     */
    const QList<Smb4KCustomOptions *> getCustomOptions();
    
    /**
     * Returns TRUE if there may be changed custom settings. You must check if
     * this is indeed the case in you code.
     * 
     * @returns TRUE if custom settings may have changed.
     */
    bool customSettingsMaybeChanged() { return m_maybe_changed; }
    
  protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter( QObject *obj,
                      QEvent *e );
    
  signals:
    /**
     * This signal is emitted everytime the custom settings potentially were 
     * modified by the user. When this signal is emitted, it does not necessarily 
     * mean that any custom setting changed. It only means that the user edited 
     * one option.
     */
    void customSettingsModified();
    
    /**
     * This signal is emitted when the settings should be reloaded.
     */
    void reloadCustomSettings();

  protected slots:
    /**
     * Sets the new general user ID.
     *
     * @param action              The action that represents the new user.
     */
    void slotNewUserTriggered( QAction *action );

    /**
     * Sets the new general group ID.
     *
     * @param action              The action that represents the new group.
     */
    void slotNewGroupTriggered( QAction *action );

    /**
     * This slot is invoked when an item is double clicked. It is used
     * to edit the item the user double clicked.
     *
     * @param item            The item that was double clicked.
     */
    void slotEditCustomItem( QListWidgetItem *item );

    /**
     * This slot is invoked when the selection in the custom tree widget
     * changed.
     */
    void slotItemSelectionChanged();

    /**
     * This slot is invoked when the custom context menu for the custom
     * options widget is requested.
     *
     * @param pos             The position where the context menu was requested.
     */
    void slotCustomContextMenuRequested( const QPoint &pos );

    /**
     * This slot is connected to the "Edit" action found in the context menu of
     * the custom options tab. It is called when this action is triggered.
     * 
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotEditActionTriggered( bool );
    
    /**
     * This slot is connected to the "Remove" action found in the context menu
     * of the custom options tab. It is called when this action is triggered.
     *
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotRemoveActionTriggered( bool );
    
    /**
     * This slot is connected to the "Clear List" action found in the context
     * menu of the custom options tab. It is called when this action is triggered.
     * 
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotClearActionTriggered( bool );
    
    /**
     * This slot is connected to the "Undo" action found in the context
     * menu of the custom options tab. It is called when this action is triggered.
     * 
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotUndoActionTriggered( bool );
    
    /**
     * This slot is called when a value was changed.
     */
    void slotEntryChanged();
    
  private:
    /**
     * Finds a custom options info in the private list. You need the @p url
     * for this to work. 
     * 
     * @param url             The URL of the custom options info object
     */
    Smb4KCustomOptions *findOptions( const QUrl &url );
    
    /**
     * Populate the editors with the current settings and enable the
     * widget.
     */
    void populateEditors( Smb4KCustomOptions *options );
    
    /**
     * Clear the editors and disable the widget. 
     */
    void clearEditors();
    
    /**
     * Commit changes 
     */
    void commitChanges();
    
    /**
     * The list widget for the custom options
     */
    KListWidget *m_custom_options;
    
    /**
     * The edit widgets
     */
    QWidget *m_editors;
    
    /**
     * The UNC of the item
     */
    KLineEdit *m_unc_address;
    
    /**
     * SMB Port
     */
    KIntNumInput *m_smb_port;

#ifndef Q_OS_FREEBSD
    /**
     * Filesystem port
     */
    KIntNumInput *m_fs_port;

    /**
     * Write access
     */
    KComboBox *m_write_access;
#endif
    
    /**
     * Protocol hint
     */
    KComboBox *m_protocol_hint;
    
    /**
     * UID
     */
    KComboBox *m_user_id;
    
    /**
     * GID
     */
    KComboBox *m_group_id;
    
    /**
     * Kerberos
     */
    QCheckBox *m_kerberos;

    /**
     * The action menu
     */
    KActionMenu *m_menu;

    /**
     * The action collection
     */
    KActionCollection *m_collection;
    
    /**
     * The list of custom Samba options
     */
    QList<Smb4KCustomOptions *> m_options_list;
    
    /**
     * The current custom options object
     */
    Smb4KCustomOptions m_current_options;
    
    /**
     * Is it possible that the custom settings changed?
     */
    bool m_maybe_changed;
    
    /**
     * List cleared or item removed?
     */
    bool m_removed;
};

#endif
