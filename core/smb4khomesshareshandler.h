/***************************************************************************
    This class handles the homes shares
                             -------------------
    begin                : Do Aug 10 2006
    copyright            : (C) 2006-2019 by Alexander Reinholdt
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
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

#ifndef SMB4KHOMESSHARESHANDLER_H
#define SMB4KHOMESSHARESHANDLER_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QScopedPointer>
#include <QDialog>


// forward declarations
class Smb4KAuthInfo;
class Smb4KHomesUsers;
class Smb4KHomesSharesHandlerPrivate;
class Smb4KProfileManager;


/**
 * This class belongs to the core of Smb4K and takes care of the
 * user names that are/were defined for a certain 'homes' share.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KHomesSharesHandler : public QObject
{
  Q_OBJECT

  friend class Smb4KHomesSharesHandlerPrivate;
  friend class Smb4KProfileManager;

  public:
    /**
     * The constructor
     */
    explicit Smb4KHomesSharesHandler(QObject *parent = 0);

    /**
     * The destructor
     */
    ~Smb4KHomesSharesHandler();
    
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
     * @returns TRUE if user has been chosen and FALSE otherwise.
     */
    bool specifyUser(const SharePtr &share, bool overwrite = true);
    
    /**
     * Return the list of users defined for a certain homes share.
     * 
     * @param share       The share
     * 
     * @returns a list of users
     */
    QStringList homesUsers(const SharePtr &share);
    
  protected Q_SLOTS:
    /**
     * Called when the application goes down
     */
    void slotAboutToQuit();
    
    /**
     * This slot is called if the active profile changed.
     * 
     * @param activeProfile   The name of the active profile
     */
    void slotActiveProfileChanged(const QString &activeProfile);
    
  private:
    /**
     * Load the host and user names into a map.
     */
    const QList<Smb4KHomesUsers *> readUserNames(bool readAll);

    /**
     * This function writes the homes user entries to the disk. If @p listOnly 
     * is set to TRUE, only the list that was passed will be written to the 
     * file replacing the existing homes user entries. If it is FALSE (the 
     * default), the list will be merged with the existing homes user entries. 
     *
     * @param list          The (new) list of homes user entries that is to be 
     *                      written.
     * @param listOnly      If TRUE only the passed list will be written to
     *                      the file.
     */
    void writeUserNames(const QList<Smb4KHomesUsers *> &list, bool listOnly = false);
    
    /**
     * Find the homes user for a specific share
     */
    const QStringList findHomesUsers(const SharePtr &share);
    
    /**
     * Add user to a homes share
     */
    void addHomesUsers(const SharePtr &share, const QStringList &users);
    
    /**
     * Migrates one profile to another.
     * 
     * This function is meant to be used by the profile manager.
     * 
     * @param from        The name of the old profile.
     * @param to          The name of the new profile.
     */
    void migrateProfile(const QString &from, const QString &to);
    
    /**
     * Removes a profile from the list of profiles.
     * 
     * This function is meant to be used by the profile manager.
     * 
     * @param name        The name of the profile.
     */
    void removeProfile(const QString &name);

    /**
     * Pointer to the Smb4KHomesSharesHandlerPrivate class
     */
    const QScopedPointer<Smb4KHomesSharesHandlerPrivate> d;
};

#endif
