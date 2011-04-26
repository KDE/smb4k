/***************************************************************************
    smb4khomesshareshandler_p  -  Private helpers for the homes shares
    handler.
                             -------------------
    begin                : Mo Apr 11 2011
    copyright            : (C) 2011 by Alexander Reinholdt
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
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

#ifndef SMB4KHOMESSHARESHANDLER_P_H
#define SMB4KHOMESSHARESHANDLER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kcombobox.h>

// application specific includes
#include <smb4khomesshareshandler.h>
#include <smb4kshare.h>


class Smb4KHomesUsers
{
  public:
    /**
     * Constructor
     */
    Smb4KHomesUsers( const Smb4KShare &share,
                     const QStringList &users );
    
    /**
     * Copy constructor
     */
    Smb4KHomesUsers( const Smb4KHomesUsers &users );
    
    /**
     * Empty constructor
     */
    Smb4KHomesUsers();
    
    /**
     * Destructor
     */
    ~Smb4KHomesUsers();
    
    /**
     * Share
     */
    Smb4KShare share;
    
    /**
     * User list
     */
    QStringList users;
};


class Smb4KHomesUserDialog : public KDialog
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    Smb4KHomesUserDialog( QWidget *parent = 0 );
    
    /**
     * Destructor
     */
    ~Smb4KHomesUserDialog();
    
    /**
     * Set the user names
     */
    void setUserNames( const QStringList &users );
    
    /**
     * Get the user names
     */
    QStringList userNames();
    
    /**
     * Get the user name to use
     */
    QString login() { return m_user_combo->currentText(); }
    
  protected slots:
    /**
     * Is connected to the textChanged() signal of the combo box
     * in the "Specify User" dialog.
     *
     * @param text        The text in the combo box
     */
    void slotTextChanged( const QString &text );

    /**
     * This slot is called if the User1 button, i.e. the "Clear" button
     * in the "Specify User" dialog has been clicked. It removes all
     * entries from the combo box.
     */
    void slotClearClicked();
    
    /**
     * This slot is called when the "OK" button is clicked.
     */
    void slotOkClicked();
    
    /**
     * This slot is used to add the input text to the completion object
     * of the input combo box.
     */
    void slotHomesUserEntered();
    
  private:
    void setupView();
    KComboBox *m_user_combo;
};


class Smb4KHomesSharesHandlerPrivate
{
  public:
    Smb4KHomesSharesHandlerPrivate();
    ~Smb4KHomesSharesHandlerPrivate();
    Smb4KHomesSharesHandler instance;
};

#endif
