/***************************************************************************
    kio_smb4k  -  Main file for the KIO slave Smb4K provides.
                             -------------------
    begin                : Tue 12 Mai 2009
    copyright            : (C) 2009-2010 by Alexander Reinholdt
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
#include <QObject>

// KDE includes
#include <kio/global.h>
#include <kio/slavebase.h>

// application specific includes
#include <core/smb4kbasicnetworkitem.h>

class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;
class Smb4KSlavePrivate;


class KIO_EXPORT Smb4KSlave : public QObject, public KIO::SlaveBase
{
  Q_OBJECT
  
  friend class Smb4KSlavePrivate;
  
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

  private:
    Q_PRIVATE_SLOT( m_priv, void slotWorkgroups( const QList<Smb4KWorkgroup *> & ) )
    Q_PRIVATE_SLOT( m_priv, void slotHosts( Smb4KWorkgroup *, const QList<Smb4KHost *> & ) )
    Q_PRIVATE_SLOT( m_priv, void slotShares( Smb4KHost *, const QList<Smb4KShare *> & ) )
    Q_PRIVATE_SLOT( m_priv, void slotScannerFinished( Smb4KBasicNetworkItem *, int ) )
    
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
     * Pointer to the private helper class.
     */
    Smb4KSlavePrivate *const m_priv;
};

#endif
