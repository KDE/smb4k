/***************************************************************************
    smb4kprofilemanager  -  This class manages the profiles that were
    defined by the user.
                             -------------------
    begin                : Mi Aug 06 2014
    copyright            : (C) 2014 by Alexander Reinholdt
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

#ifndef SMB4KPROFILEMANAGER_H
#define SMB4KPROFILEMANAGER_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QWidget>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KProfileManagerPrivate;


class KDE_EXPORT Smb4KProfileManager : public QObject
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KProfileManager(QObject *parent = 0);
    
    /**
     * Destructor
     */
    virtual ~Smb4KProfileManager();
    
    /**
     * Returns a static pointer to this class.
     * 
     * @returns a static pointer to this class.
     */
    static Smb4KProfileManager *self();
    
    /**
     * Set the active profile if the use of profiles is enabled.
     * Otherwise the this function does nothing.
     * 
     * @param name        Name of the active profile.
     * 
     * @returns true if the active profile was changed.
     */
    bool setActiveProfile(const QString &name);
    
    /**
     * Return the currently active profile or an empty string if 
     * the use of profiles is disabled.
     * 
     * @returns the active profile.
     */
    QString activeProfile() const;
    
    /**
     * Returns the list of profiles or an empty string list if the 
     * the use of profiles is diabled.
     * 
     * @returns the list of profiles.
     */
    QStringList profilesList() const;
    
    /**
     * Returns if profiles should be used or not. This is basically 
     * a convenience function, since it just returns 
     * Smb4KSettings::useProfiles().
     * 
     * @returns true if profiles should be used.
     */
    bool useProfiles() const;
    
    /**
     * Migrate all entries in one profile to another.
     * 
     * @param from        The name of the old profile.
     * @param to          The name of the new profile.
     */
    void migrateProfile(const QString &from, const QString &to);
    
    /**
     * Remove a profile with all its entries.
     * 
     * @param name        The name of the profile.
     * @param parent      The parent widget for the profile migration
     *                    dialog.
     */
    void removeProfile(const QString &name, QWidget *parent = 0);
    
  Q_SIGNALS:
    /**
     * This signal is emittted when any of the settings related to 
     * profiles changed.
     */
    void settingsChanged();
    
    /**
     * This signal is emitted when all entries of one profile was migrated
     * to another one.
     * 
     * @param from        The old profile
     * @param to          The new profile
     */
    void profileMigrated(const QString &from, const QString &to);
    
    /**
     * This signal is emitted when a profile was removed.
     * 
     * @param profile     The removed profile
     */
    void removedProfile(const QString &profile);
    
  protected Q_SLOTS:
    /**
     * This slot is connected to the configChanged() signal of the
     * configuration object of the core.
     */
    void slotConfigChanged();
    
  private:
    /**
     * Pointer to Smb4KBookmarkHandlerPrivate class
     */
    const QScopedPointer<Smb4KProfileManagerPrivate> d;
};

#endif
