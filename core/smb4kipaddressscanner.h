/***************************************************************************
    smb4kipaddressscanner  -  This class scans for IP addresses.
                             -------------------
    begin                : Fri Mar 18 2011
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
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KIPADDRESSSCANNER_H
#define SMB4KIPADDRESSSCANNER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kcompositejob.h>
#include <kdemacros.h>

// application specific includes
#include <smb4kworkgroup.h>
#include <smb4khost.h>

// forward declarations
class Smb4KIPAddressScannerPrivate;

class KDE_EXPORT Smb4KIPAddressScanner : public KCompositeJob
{
  Q_OBJECT

  friend class Smb4KIPAddressScannerPrivate;

  public:
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KPrint class.
     */
    static Smb4KIPAddressScanner *self();

    /**
     * Starts a IP address lookup for all hosts currently present 
     * in the global hosts list. If @p force is TRUE, the all IP 
     * addresses will be looked up regardless whether the host already
     * has got one.
     * 
     * @param force         Force the lookup
     *
     * @param parent        The parent widget
     */
    void lookup( bool force = false,
                 QWidget *parent = 0 );

    /**
     * Get the IP address for the master browser of @p workgroup. This is
     * a convenience function and only searches Smb4KGlobal::hostList()
     * for the right host and copies the IP address. If you want to do an
     * IP address lookup, use the lookup() function.
     *
     * @param host          The Smb4KHost object
     */
    void getIPAddress( Smb4KWorkgroup *workgroup );
    
    /**
     * Get the IP address for @p host. This is a convenience function and
     * only searches  Smb4KGlobal::hostList() for the right host and copies
     * the IP address to @p host. If you want to do an IP address lookup, use
     * the lookup() function.
     *
     * @param host          The Smb4KHost object
     */
    void getIPAddress( Smb4KHost *host );

    /**
     * This function tells you if at least one IP address lookup job is
     * currently running.
     *
     * @returns TRUE if at least one lookup job is running
     */
    bool isRunning();

    /**
     * This function aborts all searches at once.
     */
    void abortAll();

    /**
     * This function starts the composite job
     */
    void start();

  signals:
    /**
     * This signal is emitted when a lookup process is about to be started.
     * It passes the host to the receiver.
     *
     * @param host          The host
     */
    void aboutToStart( Smb4KHost *host );

    /**
     * This signal is emitted when a lookup process has finished. It passes
     * the host to the receiver.
     *
     * @param host          The host
     */
    void finished( Smb4KHost *host );

    /**
     * This signal is emitted when an IP address was successfully looked
     * up.
     *
     * @param host          The host
     */
    void ipAddress( Smb4KHost *host );

  protected slots:
    /**
     * Invoked by start() function
     */
    void slotStartJobs();

    /**
     * Processes the host if it is a master browser.
     */
    void slotProcessIPAddress( Smb4KHost *host );

    /**
     * Invoked by KJob::result() signal
     */
    void slotJobFinished( KJob *job );

    /**
     * Called when the program is about to quit
     */
    void slotAboutToQuit();

  private:
    /**
     * Constructor
     */
    Smb4KIPAddressScanner();

    /**
     * Destructor
     */
    ~Smb4KIPAddressScanner();
};

#endif
