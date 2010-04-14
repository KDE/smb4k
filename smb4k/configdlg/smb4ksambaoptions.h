/***************************************************************************
    smb4ksambaoptions.cpp  -  This is the configuration page for the
    Samba settings of Smb4K
                             -------------------
    begin                : Mo Jan 26 2004
    copyright            : (C) 2004-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSAMBAOPTIONS_H
#define SMB4KSAMBAOPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QEvent>

// KDE includes
#include <ktabwidget.h>
#include <kactionmenu.h>
#include <kactioncollection.h>

// application specific includes
#include <core/smb4ksambaoptionsinfo.h>

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
    /**
     * This enumeration is used for the list view in the "Custom" tab.
     */
#ifndef Q_OS_FREEBSD
    enum Columns{ ItemName = 0,
                  Protocol = 1,
                  WriteAccess = 2,
                  Kerberos = 3,
                  UID = 4,
                  GID = 5,
                  SMBPort = 6,
                  FileSystemPort = 7 };
#else
    enum Columns{ ItemName = 0,
                  Protocol = 1,
                  Kerberos = 2,
                  UID = 3,
                  GID = 4,
                  SMBPort = 5 };
#endif

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
    void insertCustomOptions( const QList<Smb4KSambaOptionsInfo *> &list );

    /**
     * This function returns the list of custom option items that are currently
     * in the 'Custom Options' tree widget.
     *
     * @returns the list of custom option items.
     */
    const QList<Smb4KSambaOptionsInfo> &getCustomOptions() { return m_options_list; }
    
    /**
     * Returns TRUE if there may be changed custom settings. You must check if
     * this is indeed the case in you code.
     * 
     * @returns TRUE if custom settings may have changed.
     */
    bool customSettingsMaybeChanged() { return m_maybe_changed; }
    
  signals:
    /**
     * This signal is emitted everytime the custom settings potentially were 
     * modified by the user. When this signal is emitted, it does not necessarily 
     * mean that any custom setting changed. It only means that the user edited 
     * one option.
     */
    void customSettingsModified();

  protected:
    /**
     * Reimplemented from KTabWidget/QWidget. This function is used to close any edit
     * widget in the list widget when the user pressed somewhere where no item is
     * located.
     *
     * @param object              The object where the event occurred
     *
     * @param event               The event
     */
    bool eventFilter( QObject *object,
                      QEvent *e );

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
     * to edit the column where the user double clicked.
     *
     * @param item            The item that was double clicked.
     *
     * @param column          The column where the user double clicked the item
     *                        @p item.
     */
    void slotEditCustomItem( QTreeWidgetItem *item,
                             int column );

    /**
     * This slot is invoked when the text of a custom entry in the tree
     * widget was changed by choosing from a combo box.
     *
     * @param text            The new text for the protocol.
     */
    void slotCustomTextChanged( const QString &text );

    /**
     * This slot is invoked when an integer of a custom entry in the tree
     * widget was changed.
     *
     * @param value           The new value
     */
    void slotCustomIntValueChanged( int value );

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
    
  private:
    /**
     * Finds a custom options info in the private list. You need the @p unc
     * for this to work. Note that this function performs a case sensitive 
     * search.
     * 
     * @param unc             The UNC of the custom options info object
     */
    Smb4KSambaOptionsInfo *findInfo( const QString &unc );
    
    /**
     * Removes all edit widgets in the custom options tree widget.
     */
    void removeEditWidgets();
    
    /**
     * The tree widget for the custom options
     */
    QTreeWidget *m_custom_options;

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
    QList<Smb4KSambaOptionsInfo> m_options_list;
    
    /**
     * Is it possible that the custom settings changed?
     */
    bool m_maybe_changed;
};

#endif
