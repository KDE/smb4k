/***************************************************************************
    smb4ksearchdialog_part  -  This Part encapsulates the search dialog
    of Smb4K.
                             -------------------
    begin                : Fr Jun 1 2007
    copyright            : (C) 2007-2013 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKSEARCHPART_H
#define SMB4KNETWORKSEARCHPART_H

// KDE includes
#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <klistwidget.h>
#include <kactionmenu.h>

// forward declarations
class Smb4KNetworkSearch;
class Smb4KShare;


/**
 * This is one of the parts of Smb4K. It contains the search dialog.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KNetworkSearchPart : public KParts::Part
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
     * @param args                A list of arguments. At the moment the following
     *                            arguments are recognized:
     *                            silent="true"|"false"
     */
    explicit Smb4KNetworkSearchPart( QWidget *parentWidget = 0,
                                     QObject *parent = 0,
                                     const QList<QVariant> &args = QList<QVariant>() );

    /**
     * The destructor.
     */
    virtual ~Smb4KNetworkSearchPart();

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
    void slotReceivedSearchResult( Smb4KShare *share );
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
     * This slot is connected to the Smb4KMounter::mounted() signal and marks
     * a share as mounted.
     * 
     * @param share               The share item
     */
    void slotShareMounted( Smb4KShare *share );
    
    /**
     * This slot is connected to the Smb4KMounter::unmounted() signal and unmarks
     * a share that was just unmounted.
     * 
     * @param share               The share item
     */
    void slotShareUnmounted( Smb4KShare *share );

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
     * This slot is invoked whent the Clear action is triggered. It clears the
     * combo box and the list widget and diables the actions.
     *
     * @param checked             TRUE if the action is checked.
     */
    void slotClearActionTriggered( bool checked );

    /**
     * This slot is invoked when the Mount action is triggered. It mounts the
     * selected share.
     *
     * @param checked             TRUE if the action is checked.
     */
    void slotMountActionTriggered( bool checked );

    /**
     * Change the state of the 'Mount'/'Unmount' dual action.
     *
     * @param active              TRUE if the action is in the active state.
     */
    void slotMountActionChanged( bool active );

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
    
    /**
     * This slot is called when the application is about to quit.
     */
    void slotAboutToQuit();
    
    /**
     * This slot is called if the icon size was changed.
     *
     * @param group               The icon group
     */
    void slotIconSizeChanged( int group );
    
    /**
     * This action is invoked when the Search/Abort dual action is triggered
     *
     * @param checked             TRUE if the action is checked
     */
    void slotSearchAbortActionTriggered( bool checked );
    
    /**
     * This action is invoked when the active state of the Search/Abort
     * dual action changed.
     * 
     * @param active              active or inactive
     */
    void slotSearchAbortActionChanged( bool active );
    
    /**
     * This slot is invoked when a mount or unmount finished. It is used
     * to switch the active state of the Mount/Unmount dual action.
     * 
     * @param share               The share
     * 
     * @param process             The kind of process
     */
    void slotMounterFinished( Smb4KShare *share, int process );

  private:
    /**
     * Set up actions
     */
    void setupActions();

    /**
     * This is the actual search dialog widget.
     */
    Smb4KNetworkSearch *m_widget;

    /**
     * The context action menu
     */
    KActionMenu *m_menu;

    /**
     * The menu title action
     */
    QAction *m_menu_title;
    
    /**
     * Emit status messages
     */
    bool m_silent;
};

#endif
