/***************************************************************************
    smb4kpreviewdialog.h  -  The preview dialog of Smb4K
                             -------------------
    begin                : Fre Jul 4 2003
    copyright            : (C) 2003-2008 by Alexander Reinholdt
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

#ifndef SMB4KPREVIEWDIALOG_H
#define SMB4KPREVIEWDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QStringList>

// KDE includes
#include <klistwidget.h>
#include <kdialog.h>
#include <ktoolbar.h>
#include <kcombobox.h>
#include <kdemacros.h>
#include <kaction.h>

// forward declarations
class Smb4KShare;
class Smb4KPreviewItem;

/**
 * This is the preview dialog of Smb4K. You can preview remote
 * shares with it.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KPreviewDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * This is the constructor of the preview dialog.
     *
     * @param share         The Smb4KShare object.
     *
     * @param parent        The parent of this widget
     */
    Smb4KPreviewDialog( Smb4KShare *share,
                        QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KPreviewDialog();

    /**
     * This function will request the preview. You should run it before
     * you show the dialog, but it is not mandatory.
     */
    void getPreview();

  protected slots:
    /**
     * This slot receives the results of the attempt to generate
     * a preview.
     *
     * @param item          The Smb4KPreviewItem for which a preview
     *                      was generated.
     */
    void slotReceivedData( Smb4KPreviewItem *item );

    /**
     * This slot is called when the preview process is about to start.
     *
     * @param item          The Smb4KPreviewItem for which the preview
     *                      is about to be generated.
     */
    void slotAboutToStart( Smb4KPreviewItem *item );

    /**
     * This slot is called when the preview process finished.
     *
     * @param item          The Smb4KPreviewItem for which the preview
     *                      was generated.
     */
    void slotFinished( Smb4KPreviewItem *item );

    /**
     * Is called, if an item has been executed.
     *
     * @param item          The item that has been exected.
     */
    void slotItemExecuted( QListWidgetItem *item );

    /**
     * This slot is called when the "Reload" action has been triggered.
     *
     * @param checked       TRUE if the action is checked.
     */
    void slotReloadActionTriggered( bool checked );

    /**
     * This slot is called when the "Abort" action has been triggered.
     *
     * @param checked       THRUE if the action is checked.
     */
    void slotAbortActionTriggered( bool checked );

    /**
     * This slot is called when the "Back" action has been triggered.
     *
     * @param checked       TRUE if the action is checked.
     */
    void slotBackActionTriggered( bool checked );

    /**
     * This slot is called when the "Forward" action has been triggered.
     *
     * @param checked       TRUE if the action is checked.
     */
    void slotForwardActionTriggered( bool checked );

    /**
     * This slot is called when the "Up" action has been triggered.
     *
     * @param checked       TRUE if the action is checked.
     */
    void slotUpActionTriggered( bool checked );

    /**
     * Is called, if an item in the combo box is activated.
     */
    void slotItemActivated( const QString &item );

    /**
     * This slot is called when the close button was clicked.
     */
    void slotCloseClicked();

  private:
    /**
     * Enumeration for the buttons.
     */
    enum ButtonID{ Reload,
                   Abort,
                   Up,
                   Back,
                   Forward,
                   Combo,
                   None };

    /**
     * Enumeration for the items in the list view.
     */
    enum ItemType{ File = 1000,
                   Directory = 1001 };

    /**
     * The current button id
     */
    int m_button_id;

    /**
     * Sets up the file view.
     */
    void setupView();

    /**
     * The icon view.
     */
    KListWidget *m_view;

    /**
     * The toolbar.
     */
    KToolBar *m_toolbar;

    /**
     * The combo box.
     */
    KComboBox *m_combo;

    /**
     * The private Smb4KPreviewItem object
     */
    Smb4KPreviewItem *m_item;

    /**
     * This list holds the history of the session.
     */
    QStringList m_history;

    /**
     * The index of the current item in the history.
     */
    int  m_current_index;

    /**
     * Reload action
     */
    KAction *m_reload;

    /**
     * Abort action
     */
    KAction *m_abort;

    /**
     * Back action
     */
    KAction *m_back;

    /**
     * Forward action
     */
    KAction *m_forward;

    /**
     * Up action
     */
    KAction *m_up;
};
#endif
