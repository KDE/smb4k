/***************************************************************************
    smb4kcustomoptionsmanager - Manage custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2015 by Alexander Reinholdt
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

#ifndef SMB4KCUSTOMOPTIONSMANAGER_H
#define SMB4KCUSTOMOPTIONSMANAGER_H

// application specific includes
#include "smb4kcustomoptions.h"

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtGui/QWidget>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KCustomOptionsManagerPrivate;
class Smb4KShare;
class Smb4KBasicNetworkItem;
class Smb4KProfileManager;

/**
 * This classes manages the custom options that were defined
 * for a certain share or host.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class KDE_EXPORT Smb4KCustomOptionsManager : public QObject
{
  Q_OBJECT
  
  friend class Smb4KCustomOptionsManagerPrivate;
  friend class Smb4KProfileManager;
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KCustomOptionsManager( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KCustomOptionsManager();
    
    /**
     * Returns a static pointer to this class
     * 
     * @returns a static pointer to this class
     */
    static Smb4KCustomOptionsManager *self();
    
    /**
     * Add the share to the list of shares that are to be remounted
     * either only on next program start or always Smb4K is restarted.
     * 
     * @param share     The share object
     * 
     * @param always    If set to TRUE the share is always mounted 
     *                  when Smb4K is restarted.
     */
    void addRemount( Smb4KShare *share, bool always = false );
    
    /**
     * Remove the share @p share from the list of shares that are to be 
     * remounted. If @p force is set to TRUE, the share is removed even 
     * if it should always be removed (option is set to 
     * Smb4KCustomOptions::AlwaysRemount). Apart from that, the share is only 
     * removed when the option is set to Smb4KCustomOptions::DoRemount.
     * 
     * @param share     The share object
     * 
     * @param force     If set to TRUE, the share is removed regardless of the
     *                  remount setting.
     */
    void removeRemount( Smb4KShare *share, bool force = false );
    
    /**
     * Removes all remounts from the list of custom options. If @p force
     * is set to TRUE, even those are removed that should always be remounted.
     * 
     * @param force     If set to TRUE, even those shares are removed that should
     *                  always be remounted.
     */
    void clearRemounts( bool force );
    
    /**
     * Returns the list of shares that are to be remounted.
     * 
     * @returns the list of shares that are to be remounted
     */
    QList<Smb4KCustomOptions *> sharesToRemount();

    /**
     * Find custom options for the network item @p networkItem.
     * 
     * If the network item represents a share and custom options for it are not
     * defined, but for the host that provides the share, the options for the host
     * are returned. If neither is in the list, NULL is returned.
     *
     * If you set @p exactMatch to TRUE, NULL will be returned if the URL is not found. 
     * Except in some special cases, you should not set @p exactMatch to true,
     * because options that are defined for all shares provided by a certain host and
     * stored in a host-type custom options object are ignored then.
     * 
     * @param networkItem         The network item
     * @param exaxtMatch          If TRUE, only exact matches are returned
     *
     * @returns the custom options for the network item
     */
    Smb4KCustomOptions *findOptions( Smb4KBasicNetworkItem *networkItem,
                                     bool exactMatch = false );

    /**
     * Find custom options for the provided @p url.
     *
     * This function searches the list of custom options and compares the host entry
     * and, if applicable, the path (i.e. the share name). If an exact match was found, 
     * the corresponding custom options are returned.
     * 
     * @param url                 The network item's URL
     *
     * @returns the custom options
     */
    Smb4KCustomOptions *findOptions( const KUrl &url );
    
    /**
     * Get the list of custom options. By default, the list not only comprises of those 
     * items that have custom options defined but also of those that are "only" to be 
     * remounted. If @p optionsOnly is defined, only those entries are returned that have
     * custom options defined. Those that are only to be remounted won't be returned.
     * 
     * @param optionsOnly         Only return those entries that have custom options defined
     * 
     * @returns the list of custom options objects.
     */
    const QList<Smb4KCustomOptions *> customOptions( bool optionsOnly = false );
    
    /**
     * Replace all previously defined custom options with a list of new ones. If you
     * just want to change certain custom options, use the findOptions() functions.
     * 
     * @param options_list        The list of new or updated options
     */
    void replaceCustomOptions( const QList<Smb4KCustomOptions *> &options_list );
    
    /**
     * This function opens the custom options dialog.
     * 
     * @param item                The network item - either host or share
     * 
     * @param parent              The parent widget
     */
    void openCustomOptionsDialog( Smb4KBasicNetworkItem *item,
                                  QWidget *parent = 0 );
    
    /**
     * This function adds custom options for a single network item to the list
     * of options. If there already are options defined for that network item,
     * they are updated.
     * 
     * Please note that this function will store a copy of @p options and not 
     * the original object.
     * 
     * @param options             The custom options
     */
    void addCustomOptions( Smb4KCustomOptions *options );
    
    /**
     * This function removes custom options for a single network item from the
     * list of options.
     * 
     * @param options             The custom options
     */
    void removeCustomOptions( Smb4KCustomOptions *options );
    
    /**
     * This functions checks if the options object indeed carries custom
     * options. It does not test for the value of remount(), so you need 
     * do check it, too.
     * 
     * @param options       The custom options object
     */
    bool hasCustomOptions( Smb4KCustomOptions *options );
    
    /**
     * This function returns a list of custom option objects that have 
     * Wake-On-LAN features defined.
     * 
    * @returns a list of custom options objects with WOL features defined.
    */
    QList<Smb4KCustomOptions *> wolEntries() const;
    
  protected Q_SLOTS:
    /**
     * Called when the application exits
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
     * Read custom options
     */
    void readCustomOptions(QList<Smb4KCustomOptions *> *optionsList, bool allOptions);
    
    /**
     * This function writes the custom options to the disk. If @p listOnly is
     * set to TRUE, only the list that was passed will be written to the 
     * file replacing the existing custom options. If it is FALSE (the default),
     * the list will be merged with the existing custom options. 
     *
     * @param list          The (new) list of custom options that is to be 
     *                      written to the file.
     * @param listOnly      If TRUE only the passed list will be written to
     *                      the file.
     */
    void writeCustomOptions(const QList<Smb4KCustomOptions *> &list, bool listOnly = false);
    
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
     * Pointer to Smb4KCustomOptionsManagerPrivate class
     */
    const QScopedPointer<Smb4KCustomOptionsManagerPrivate> d;
};

#endif
