/***************************************************************************
    smb4ksambaoptionshandler  -  This class handles the Samba options.
                             -------------------
    begin                : So Mai 14 2006
    copyright            : (C) 2006-2010 by Alexander Reinholdt
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

#ifndef SMB4KSAMBAOPTIONSHANDLER_H
#define SMB4KSAMBAOPTIONSHANDLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QMap>
#include <QList>

// KDE includes
#include <kdemacros.h>

// application includes
#include <smb4kshare.h>

// forward declarations
class Smb4KSambaOptionsInfo;
class Smb4KWorkgroup;
class Smb4KHost;


/**
 * This class belongs to the core classes of Smb4K and handles the global
 * and the custom Samba options.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KSambaOptionsHandler : public QObject
{
  Q_OBJECT

  friend class Smb4KSambaOptionsHandlerPrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KSambaOptionsHandler *self();

    /**
     * Retrieve the list of shares that have custom options defined.
     * You will only get those options that have different ports etc.
     * defined but also all shares that are to be remounted.
     *
     * @returns the list of all shares that have custom options defined.
     */
    const QList<Smb4KSambaOptionsInfo *> &customOptionsList() { return m_list; }

    /**
     * Get only those shares that are to be remounted.
     *
     * @returns a list of those shares that are to be remounted.
     */
    QList<Smb4KSambaOptionsInfo *> sharesToRemount();

    /**
     * This function adds the share @p share to the ones that should be remounted
     * after the restart of the application or after the computer woke up from a
     * sleep state.
     *
     * @param share             The share that is to be remounted.
     */
    void addRemount( Smb4KShare *share );

    /**
     * This function removes the share @p share from the ones that should be remounted.
     *
     * @param share             The share that should not be remounted anymore.
     */
    void removeRemount( Smb4KShare *share );

    /**
     * Remove all remounts from the list of custom options.
     */
    void clearRemounts();

    /**
     * Commit the whole list of shares with custom options to the configuration
     * file. You should call this if you exit the application.
     */
    void sync();

    /**
     * This function returns the options defined in the global section of the smb.conf
     * file. All option names have been converted to lower case and you can find each
     * entry by providing the option name in lowercase (!) as key.
     *
     * @returns a list of the options defined in smb.conf.
     */
    const QMap<QString,QString> &globalSambaOptions();

    /**
     * This function returns the WINS server the system is using.
     *
     * @returns the name or IP of the WINS server
     */
    const QString &winsServer();

    /**
     * Find a share in the list. If custom options for that share are not defined, but
     * for the host that carries the share, the options for the host are returned.
     *
     * If you provide @p exactMatch, NULL will be returned if item is not in the list.
     * Otherwise this function returns the closest match (the host) if available.
     *
     * @param share             The share
     *
     * @param exactMatch        The UNC of the share is matched exactly.
     *
     * @returns                 the network item.
     */
    Smb4KSambaOptionsInfo *findItem( Smb4KShare *share,
                                     bool exactMatch = false );

    /**
     * Find a host in the list.
     *
     * @param host              The share
     *
     * @returns                 the network item.
     */
    Smb4KSambaOptionsInfo *findItem( Smb4KHost *host );

    /**
     * Add a new Smb4KSambaOptionsInfo object to the list of custom options. If the item already exists,
     * the old options will be replaced by the new ones.
     *
     * @param info              The Smb4KSambaOptionsInfo object
     *
     * @param sync              If TRUE, the list is sync'ed with the config file.
     */
    void addItem( Smb4KSambaOptionsInfo *info,
                  bool sync );

    /**
     * Remove a host item from the list.
     *
     * @param host              The host that is to be removed.
     *
     * @param sync              If TRUE, the list is sync'ed with the config file.
     */
    void removeItem( Smb4KHost *host,
                     bool sync );

    /**
     * Remove a share item from the list.
     *
     * @param share             The host that is to be removed.
     *
     * @param sync              If TRUE, the list is sync'ed with the config file.
     */
    void removeItem( Smb4KShare *share,
                     bool sync );

    /**
     * This function updates the list of custom options.
     *
     * @param list              The list that is used for the update.
     */
    void updateCustomOptions( const QList<Smb4KSambaOptionsInfo *> &list );
    
    /**
     * Check whether the item has entries that deviate from the global settings and return
     * TRUE if this is the case. Please note that this function ignores the remount flag.
     *
     * @param info              The options info item that is to be checked.
     *
     * @returns TRUE if the item contains entries that deviate from the global settings and
     * FALSE otherwise.
     */
    bool hasCustomOptions( Smb4KSambaOptionsInfo *info );

  private:
    /**
     * The constructor
     */
    Smb4KSambaOptionsHandler();

    /**
     * The destructor
     */
    ~Smb4KSambaOptionsHandler();

    /**
     * The list of network items that have custom options defined.
     */
    QList<Smb4KSambaOptionsInfo *> m_list;

    /**
     * Read the options from the configuration file.
     */
    void readCustomOptions();

    /**
     * Write the options to the configuration file.
     */
    void writeCustomOptions();

    /**
     * This function reads the entries of the global section of Samba's configuration
     * file smb.conf and puts them into a map.
     */
    void read_smb_conf();

    /**
     * This map carries the options defined in the [global] section of Samba's configuration
     * file smb.conf. You can access a certain value by providing the lower case option name
     * as key.
     */
    QMap<QString,QString> m_samba_options;

    /**
     * The WINS server
     */
    QString m_wins_server;
};


#endif
