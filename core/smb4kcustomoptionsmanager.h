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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QWidget>

// KDE includes
#include <kdemacros.h>

// application specific includes
#include <smb4kcustomoptions.h>

// forward declarations
class Smb4KCustomOptionsManagerPrivate;
class Smb4KShare;

class KDE_EXPORT Smb4KCustomOptionsManager : public QObject
{
  Q_OBJECT
  
  friend class Smb4KCustomOptionsManagerPrivate;
  
  public:
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
     * Find custom options for the share @p share. If custom options for that share are not 
     * defined, but for the host that provides the share, the options for the host are 
     * returned. If neither is in the list, NULL is returned.
     *
     * If you set @p exactMatch to TRUE, NULL will be returned if the share is not in 
     * the list. Except in some special cases, you should not set @p exactMatch to true, 
     * because options that are defined for all shares provided by a certain host and
     * stored in a host-type custom options object are ignored then.
     * 
     * @returns the custom options for the share
     */
    Smb4KCustomOptions *findOptions( const Smb4KShare *share,
                                     bool exactMatch = false );
    
    /**
     * Find custom options for the host @p host. If no custom options for the host are
     * defined. NULL is returned.
     * 
     * @returns the custom options for the host
     */
    Smb4KCustomOptions *findOptions( const Smb4KHost *host );
    
    /**
     * Get the list of custom options. The list not only comprises of those items that
     * have custom options defined but also of those that are "only" to be remounted.
     * 
     * @returns the list of custom options objects.
     */
    const QList<Smb4KCustomOptions *> customOptions();
    
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
    
  protected slots:
    /**
     * Called when the application exits
     */
    void slotAboutToQuit();
  
  private:
    /**
     * Constructor
     */
    Smb4KCustomOptionsManager();
    
    /**
     * Destructor
     */
    ~Smb4KCustomOptionsManager();
    
    /**
     * Read custom options
     */
    void readCustomOptions();
    
    /**
     * Write custom options
     */
    void writeCustomOptions();
    
    /**
     * List of items for that custom options are defined
     */
    QList<Smb4KCustomOptions *> m_options;
};

#endif
