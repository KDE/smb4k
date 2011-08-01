/***************************************************************************
    smb4khomesshareshandler  -  This class handles the homes shares.
                             -------------------
    begin                : Do Aug 10 2006
    copyright            : (C) 2006-2011 by Alexander Reinholdt
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

#ifndef SMB4KHOMESSHARESHANDLER_H
#define SMB4KHOMESSHARESHANDLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QString>
#include <QStringList>
#include <QPair>
#include <QList>

// KDE includes
#include <kdialog.h>
#include <kdemacros.h>

// forward declarations
class Smb4KShare;
class Smb4KAuthInfo;
class Smb4KHomesUsers;
class Smb4KHomesSharesHandlerPrivate;


/**
 * This class belongs to the core of Smb4K and takes care of the
 * user names that are/were defined for a certain 'homes' share.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class KDE_EXPORT Smb4KHomesSharesHandler : public QObject
{
  Q_OBJECT

  friend class Smb4KHomesSharesHandlerPrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KHomesSharesHandler *self();

    /**
     * This function will open a dialog where the user has to define a
     * user name to access a 'homes' share. You also can define if the 
     * user name should be overwritten in case one has already been set
     * (default is TRUE).
     *
     * In case that a new username is set by this function, the password
     * is cleared.
     *
     * @param share       The share that is representing the homes share
     * 
     * @param overwrite   Overwrite user name or not
     *
     * @param parent      The parent widget
     *
     * @returns TRUE if user has been chosen and FALSE otherwise.
     */
    bool specifyUser( Smb4KShare *share, 
                      bool overwrite = true, 
                      QWidget *parent = 0 );
    
    /**
     * Return the list of users defined for a certain homes share.
     * 
     * @param share       The share
     * 
     * @returns a list of users
     */
    QStringList homesUsers( Smb4KShare *share );
    
  protected slots:
    /**
     * Called when the application goes down
     */
    void slotAboutToQuit();

  private:
    /**
     * The constructor
     */
    Smb4KHomesSharesHandler();

    /**
     * The destructor
     */
    ~Smb4KHomesSharesHandler();

    /**
     * Load the host and user names into a map.
     */
    void readUserNames();

    /**
     * Save the host and user names.
     */
    void writeUserNames();
    
    /**
     * Find the homes user for a specific share
     */
    void findHomesUsers( Smb4KShare *share, 
                         QStringList *users );
    
    /**
     * Add user to a homes share
     */
    void addHomesUsers( Smb4KShare *share,
                        QStringList *users );
    
    /**
     * The list of shares with homes users
     */
    QList<Smb4KHomesUsers> m_homes_users;
};

#endif
