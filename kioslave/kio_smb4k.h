/***************************************************************************
    kio_smb4k  -  Main file for the KIO slave Smb4K provides.
                             -------------------
    begin                : Tue 12 Mai 2009
    copyright            : (C) 2009 by Alexander Reinholdt
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

#ifndef KIO_SMB4K_H
#define KIO_SMB4K_H

// Qt includes
#include <QString>

// KDE includes
#include <kio/global.h>
#include <kio/slavebase.h>

// application specific includes
#include <core/smb4kbasicnetworkitem.h>

class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;


class Smb4KSlave : public KIO::SlaveBase
{
  public:
    /**
     * Constructor
     */
    Smb4KSlave( const QByteArray &pool,
                const QByteArray &app );

    /**
     * Destructor.
     */
    virtual ~Smb4KSlave();

    /**
     * List the URL. Reimplemented from KIO::SlaveBase.
     */
    void listDir( const KUrl &url );

    /**
     * Stat the URL @p url. Reimplemented from KIO::SlaveBase.
     */
    void stat( const KUrl &url );

    void get( const KUrl &url );
    void special( const QByteArray &data );

  private:
    /**
     * The current network item
     */
    Smb4KBasicNetworkItem m_current_item;

    /**
     * The list of shares
     */
    QList<Smb4KShare *> m_shares;

    /**
     * Check the URL.
     *
     * @param url         The URL that is to be checked
     *
     * @returns the correct(ed) URL.
     */
    KUrl checkURL( const KUrl &url );

    /**
     * Returns the current network item. The incoming URL @p url is
     * inspected and the network item is returned according to the
     * findings.
     *
     * @param url         The current URL
     *
     * @returns the current network item.
     */
    Smb4KBasicNetworkItem currentNetworkItem( const KUrl &url );

    /**
     * Find an entry in the list of workgroups.
     *
     * Note: This function is needed, because we cannot use the global
     * workgroups list past a redirection.
     *
     * @param name        The name of the workgroup
     *
     * @returns the workgroup entry from the list.
     */
    Smb4KWorkgroup *findWorkgroup( const QString &name );

    /**
     * Find an entry in the list of hosts.
     *
     * Note: This function is needed, because we cannot use the global
     * hosts list past a redirection.
     *
     * @param name        The name of the host
     *
     * @param workgroup   The name of the host's workgroup/domain
     *
     * @returns the host entry from the list.
     */
    Smb4KHost *findHost( const QString &name,
                         const QString &workgroup = QString() );

    /**
     * Find an entry in the list of shares.
     *
     * @param name        The name of the share
     *
     * @param host        The name of the host
     *
     * @param workgroup   The name of the workgroup
     *
     * @returns the share entry from the list or NULL if it was not found.
     */
    Smb4KShare *findShare( const QString &name,
                           const QString &host,
                           const QString &workgroup = QString() );
};

#endif
