/***************************************************************************
    smb4kipaddressscanner  -  This class scans for IP addresses. It
    belongs to the core classes of Smb4K.
                             -------------------
    begin                : Di Apr 22 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

#ifndef SMB4KIPADDRESSSCANNER_H
#define SMB4KIPADDRESSSCANNER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QList>
#include <QCache>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KHost;
class IPScanThread;

/**
 * This class scans for IP addresses of hosts found in the network.
 * It either takes a single host or a list of hosts and initiates
 * the scan by invoking private scan threads.
 *
 * This class is a very simple helper class. It does not have the
 * ability to stop a scan once it was started. But since this class
 * is supposed to run in the background, this is not needed.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KIPAddressScanner : public QObject
{
  Q_OBJECT

  friend class Smb4KIPAddressScannerPrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KIPAddressScanner *self();

    /**
     * Lookup the IP address of a certain @p host. If @p host already has an
     * IP address set, this function will do nothing. If the IP address is already
     * known, it will set it. Otherwise the @p host will be processed. If @p wait
     * is set to true, the function will wait until the IP address was determined
     * and will return afterwards. The advantage it, that you can immediately work
     * with the IP address. However, in most cases you do not want this function
     * to block the rest of the program. By default, the function returns after the
     * lookup process was started and you need to connect to the @see ipAddress()
     * signal to retrieve the result.
     *
     * @param host            The Smb4KHost item for which the IP address is to be
     *                        looked up.
     *
     * @param wait            Set to TRUE, the function will wait until the IP address
     *                        was retrieved.
     */
    void lookup( Smb4KHost *host,
                 bool wait = false );

    /**
     * Lookup the IP addresses of the hosts that are stored in @p list. This
     * function loops through the list of hosts and passes each item to the
     * @see lookup() function above. If you set @p wait to TRUE, you can immediately
     * work with the IP addresses after the function returned. However, this might
     * freeze your program for the time that is needed to find all IP addresses.
     *
     * @param list            The list of Smb4KHost items for which the IP address
     *                        are to be looked up.
     *
     * @param wait            Set to TRUE, the IP addresses will immediately filled
     *                        into the list.
     */
    void lookup( const QList<Smb4KHost *> &list,
                 bool wait = false );
                 
    /**
     * This function returns TRUE if a thread is running that looks up the IP address
     * for @p host.
     *
     * @param host          The host item
     *
     * @returns             TRUE if the IP address for @p host is currently being
     *                      looked up.
     */
    bool isRunning( Smb4KHost *host );
                 
    /**
     * This function reports if the IP scanner is running or not.
     *
     * @returns             TRUE if the scanner is running and FALSE otherwise.
     */
    bool isRunning() { return !m_cache.isEmpty(); }

  signals:
    /**
     * This signal is emitted, when an IP address was found for a host. It passed
     * the associated Smb4KHost object.
     *
     * @param host            The Smb4KHost object that carries the discovered
     *                        IP address.
     */
    void ipAddress( Smb4KHost *host );

  protected slots:
    /**
     * This slot is called when the application is about to closed.
     */
    void slotAboutToQuit();
    
    /**
     * This slot is called when an IP address was discovered and emitted by
     * the lookup thread.
     *
     * @param host            The Smb4KHost item that carries the IP address
     */
    void slotProcessIPAddress( Smb4KHost *host );
    
    /**
     * This slot is called when a thread finished.
     */
    void slotThreadFinished();

  private:
    /**
     * The constructor
     */
    Smb4KIPAddressScanner();

    /**
     * The destructor
     */
    ~Smb4KIPAddressScanner();

    /**
     * The cache that holds the threads.
     */
    QCache<QString, IPScanThread> m_cache;
};

#endif
