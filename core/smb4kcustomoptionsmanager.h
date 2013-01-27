/***************************************************************************
    smb4kcustomoptionsmanager - Manage custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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
     * next time.
     * 
     * @param share     The share object
     */
    void addRemount( Smb4KShare *share );
    
    /**
     * Remove the share @p share from the list of shares that are to
     * be remounted the next time.
     * 
     * @param share     The share object
     */
    void removeRemount( Smb4KShare *share );
    
    /**
     * Removes all remounts from the list of custom options.
     */
    void clearRemounts();
    
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
  
  private:
    /**
     * Read custom options
     */
    void readCustomOptions();
    
    /**
     * Write custom options
     */
    void writeCustomOptions();

    /**
     * Pointer to Smb4KCustomOptionsManagerPrivate class
     */
    const QScopedPointer<Smb4KCustomOptionsManagerPrivate> d;
};

#endif
