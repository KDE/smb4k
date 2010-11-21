/***************************************************************************
    smb4ksearchdialog  -  The search dialog widget of Smb4K.
                             -------------------
    begin                : Sa Jun 2 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKSEARCH_H
#define SMB4KNETWORKSEARCH_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>
#include <QString>

// KDE includes
#include <klistwidget.h>
#include <kcombobox.h>
#include <kpushbutton.h>


/**
 * This is the search dialog. It enables the user to find servers,
 * that were not found by the regular network scan.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
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
    Smb4KNetworkSearch( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KNetworkSearch();

    /**
     * This function returns a pointer to the list view of this widget.
     *
     * @returns a pointer to the list view of this widget.
     */
    KListWidget *listWidget() { return m_list_widget; }

    /**
     * This function returns a pointer to the combo box.
     *
     * @returns a pointer to the combo box.
     */
    KComboBox *comboBox() { return m_combo; }

  private:
    /**
     * The list box of this widget
     */
    KListWidget *m_list_widget;

    /**
     * The search combo box.
     */
    KComboBox *m_combo;
};

#endif
