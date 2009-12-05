/***************************************************************************
    smb4ksambaoptionshandler  -  This class handles the Samba options.
                             -------------------
    begin                : So Mai 14 2006
    copyright            : (C) 2006-2008 by Alexander Reinholdt
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
     * This function returns the line of arguments for the 'smbclient' program.
     * The arguments are spcific to the share that is defined by @p share. You have
     * to provide the name of the shares - as always - in the form //HOST/SHARE.
     *
     * @param share             The name of the share.
     *
     * @returns a list of arguments for use with the 'smbclient' program.
     */
    const QString smbclientOptions( Smb4KShare *share = NULL );

    /**
     * This function returns the "global" options for nmblookup, i.e. the domain
     * the client is in, if Kerberos should be used, etc.
     *
     * @param with_broadcast    Return the global broadcast address if defined.
     *
     * @returns a string with the "global" options for nmblookup
     */
    const QString nmblookupOptions( bool with_broadcast = true );

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
     * This enumeration is for use with the netOptions() function. It tells which
     * command to use.
     */
    enum NetCommand { Share,
                      ServerDomain,
                      LookupHost,
                      LookupMaster,
                      Domain };

    /**
     * This function returns the options for the net command. It only knows
     * host related actions. If you need to scan for workgroup masters, etc.,
     * use the function below.
     *
     * Known values for @p command: Share, LookupHost, LookupMaster
     *
     * @param command           One of the entries of the NetCommand enumeration.
     *
     * @param host              The Smb4KHost item.
     *
     * @returns the list of arguments for the net command or an empty string if
     * an error occurred.
     */
    const QString netOptions( NetCommand command,
                              Smb4KHost *host );

    /**
     * This is an overloaded function and behaves essentially the same way as the
     * one above. It returns the options for the net command and only knows
     * workgroup related actions. If you need to scan for shares, etc.,use the
     * function above.
     *
     * Known values for @p command: LookupHost, LookupMaster
     *
     * @param command           One of the entries of the NetCommand enumeration.
     *
     * @param workgroup         The Smb4KWorkgroup item.
     *
     * @returns the list of arguments for the net command or an empty string if
     * an error occurred.
     */
    const QString netOptions( NetCommand command,
                              Smb4KWorkgroup *workgroup );

    /**
     * This is an overloaded function and behaves essentially the same way as the
     * others above. It returns the options for the net command. In contrast to the
     * other functions it only takes one argument.
     *
     * Known values for @p command: ServerDomain, Domain
     *
     * @param command          One of the entries of the NetCommand enumeration.
     *
     * @returns the list of arguments for the net command or an empty string if
     * an error occurred.
     */
    const QString netOptions( NetCommand command );

    /**
     * Find a share in the list.
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
     * If you provide @p exactMatch, NULL will be returned if item is not in the list.
     * Otherwise this function returns the closest match if available.
     *
     * @param share             The share
     *
     * @param exactMatch        The UNC of the share is matched exactly.
     *
     * @returns                 the network item.
     */
    Smb4KSambaOptionsInfo *findItem( Smb4KHost *share,
                                     bool exactMatch = false );

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
     * This function returns the options for the smbtree program. Optionally,
     * you can provide the workgroup item that contains the current workgroup
     * master browser, so that options defined for it can be used instead of
     * the global ones.
     *
     * @param workgroup         The name of the local workgroup master browser.
     *
     * @returns the options for smbtree.
     */
    const QString smbtreeOptions( Smb4KWorkgroup *workgroup = NULL );

    /**
     * This function updates the list of custom options.
     *
     * @param list              The list that is used for the update.
     */
    void updateCustomOptions( const QList<Smb4KSambaOptionsInfo *> &list );

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
     * This function searches a particular UNC in the list. If this item is a share
     * and it is not found, @p exactMatch determines if NULL or the values of the item that
     * matches @p item closest are returned (i.e. the host, or another share that's located
     * on the host). In most cases you want @p exactMatch to be FALSE.
     * Please note: Do not delete the pointer that's returned by this function or you will
     * remove an item from the list!
     *
     * @param unc               The name of the network item.
     *
     * @param exactMatch        The name has to match exactly the result that's returned.
     *
     * @returns                 The Smb4KSambaOptionsInfo object associated with the network item.
     */
//     Smb4KSambaOptionsInfo *find_item( const QString &item, bool exactMatch = false );

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

    /**
     * Check whether the item has entries that deviate from the global settings and return
     * TRUE if this is the case. Please note that this function ignores the remount flag.
     *
     * @param item              The item that is to be checked.
     *
     * @returns TRUE if the item contains entries that deviate from the global settings and
     * FALSE otherwise.
     */
    void has_custom_options( Smb4KSambaOptionsInfo *item );
};


#endif
