/***************************************************************************
    smb4ksearchdialog_part  -  This Part encapsulates the search dialog
    of Smb4K.
                             -------------------
    begin                : Fr Jun 1 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
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

#ifndef SMB4KSEARCHDIALOGPART_H
#define SMB4KSEARCHDIALOGPART_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <klistwidget.h>
#include <kactionmenu.h>

// forward declarations
class Smb4KSearchDialog;
class Smb4KBasicNetworkItem;
class Smb4KShare;


/**
 * This is one of the parts of Smb4K. It contains the search dialog.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSearchDialogPart : public KParts::Part
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parentWidget        The parent widget
     *
     * @param parent              The parent object
     *
     * @param name                The name this object should have
     */
    Smb4KSearchDialogPart( QWidget *parentWidget = 0,
                           QObject *parent = 0,
                           const QStringList &args = QStringList() );

    /**
     * The destructor.
     */
    virtual ~Smb4KSearchDialogPart();

    /**
     * This function creates the KAboutData on request and returns it.
     *
     * @returns the KAboutData object.
     */
    static KAboutData *createAboutData();

  protected:
    /**
     * Reimplemented from KParts::Part.
     */
    void customEvent( QEvent *e );

  protected slots:
    /**
     * This slot retrieves the search result and puts it into the search
     * dialog.
     *
     * @param item                A Smb4KBasicNetworkItem object
     *
     * @param known               TRUE if item is known and FALSE otherwise
     */
    void slotReceivedSearchResult( Smb4KBasicNetworkItem *item,
                                   bool known );
    /**
     * This slot is connected to the Smb4KSearch::aboutToStart() signal.
     *
     * @param string              The search string for that the search is to
     *                            be performed.
     */
    void slotSearchAboutToStart( const QString &string );

    /**
     * This slot is connected to the Smb4KSearch::finished() signal.
     *
     * @param string              The search string for that the search was
     *                            performed.
     */
    void slotSearchFinished( const QString &string );

    /**
     * This slot is connected to the Smb4KScanner::hostListChanged() signal
     * and checks whether a host is already known, i.e. it is in the browser,
     * or if it is "new". If it is already known, this slot will change the icon
     * of the host item from the default one to one with a tick layed over.
     */
    void slotCheckItemIsKnown();

    /**
     * This slot is connected to the Smb4KMounter::updated() signal and will 
     * change the icon of the share item from the default one to the one that 
     * indicates that the item is mounted.
     * 
     * @param share               The share item
     */
    void slotMarkMountedShare( Smb4KShare *share );

    /**
     * This slot is invoked, when a user double clicks an item. It adds the item
     * to the list of known hosts (if necessary).
     *
     * @param item                The item that has been double clicked.
     */
    void slotItemDoubleClicked( QListWidgetItem *item );

    /**
     * This slot is invoked when the user pressed the return keyboard key. It
     * forwards the search request to the slotSearchTriggered() slot.
     */
    void slotReturnPressed();

    /**
     * This slot is invoked when the Search action is triggered. It disables some
     * widgets and starts the search.
     *
     * @param checked             TRUE if the action is checked.
     */
    void slotSearchActionTriggered( bool checked );

    /**
     * This slot is invoked whent the Clear action is triggered. It clears the
     * combo box and the list widget and diables the actions.
     *
     * @param checked             TRUE if the action is checked.
     */
    void slotClearActionTriggered( bool checked );

    /**
     * This slot is invoked when the Add action is triggered. It checks whether
     * the selected search item is valid and adds it to the global list of hosts
     * if necessary.
     *
     * @param checked             TRUE if the action is checked.
     */
    void slotAddActionTriggered( bool checked );

    /**
     * This slot is invoked when the Abort action is triggered. It enables and
     * disables some actions.
     *
     * @param checked             TRUE if the action is checked.
     */
    void slotAbortActionTriggered( bool checked );

    /**
     * This slot is invoked when the text in the combo box changed. It enables
     * or disables some actions.
     *
     * @param text                The text in the line edit of the combo box.
     */
    void slotComboBoxTextChanged( const QString &text );

    /**
     * This slot is invoked when the item selection changed. It enables and
     * disables some actions.
     */
    void slotItemSelectionChanged();

    /**
     * This slot is invoked, when the context menu of the search dialog is
     * requested.
     *
     * @param pos                 The postition where the context menu should
     *                            be opened.
     */
    void slotContextMenuRequested( const QPoint &pos );

  private:
    /**
     * Set up actions
     */
    void setupActions();

    /**
     * This is the actual search dialog widget.
     */
    Smb4KSearchDialog *m_widget;

    /**
     * The context action menu
     */
    KActionMenu *m_menu;

    /**
     * The menu title action
     */
    QAction *m_menu_title;
};

#endif
