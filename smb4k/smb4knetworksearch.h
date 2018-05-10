/***************************************************************************
    smb4ksearchdialog  -  The search dialog widget of Smb4K.
                             -------------------
    begin                : Sa Jun 2 2007
    copyright            : (C) 2007-2018 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKSEARCH_H
#define SMB4KNETWORKSEARCH_H

// Qt includes
#include <QString>
#include <QWidget>
#include <QListWidget>

// KDE includes
#include <KCompletion/KComboBox>
#include <KXmlGui/KToolBar>


/**
 * This is the search dialog. It enables the user to find servers,
 * that were not found by the regular network scan.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */


class Smb4KNetworkSearch : public QWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent      The parent widget
     */
    
    explicit Smb4KNetworkSearch(QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KNetworkSearch();

    /**
     * This function returns a pointer to the list view of this widget.
     *
     * @returns a pointer to the list view of this widget.
     */
    QListWidget *listWidget() { return m_list_widget; }

    /**
     * This function returns a pointer to the combo box.
     *
     * @returns a pointer to the combo box.
     */
    KComboBox *comboBox() { return m_combo; }
    
    /**
     * This function returns a pointer to the tool bar right to the
     * search line that can be populated with actions.
     * 
     * @returns a pointer to the tool bar.
     */
    KToolBar *toolBar() { return m_toolbar; }
    
  private:
    /**
     * The list box of this widget
     */
    QListWidget *m_list_widget;

    /**
     * The search combo box.
     */
    KComboBox *m_combo;
    
    /**
     * The tool bar that can be populated with actions
     */
    KToolBar *m_toolbar;
};

#endif
