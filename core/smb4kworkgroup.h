/***************************************************************************
    smb4kworkgroup  -  Smb4K's container class for information about a
    workgroup.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KWORKGROUP_H
#define SMB4KWORKGROUP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>

// KDE includes
#include <kdemacros.h>

// application specific includes
#include <smb4kbasicnetworkitem.h>

/**
 * This class is a container that carries information about a workgroup or
 * domain found in the network neighborhood. It is part of the core classes
 * of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KWorkgroup : public Smb4KBasicNetworkItem
{
  public:
    /**
     * The default constructor. It takes the name of the workgroup as only
     * argument. You have to set all other information by the other functions
     * provided by this class.
     *
     * @param name            The name of the workgroup or domain.
     */
    Smb4KWorkgroup( const QString &name );

    /**
     * The copy contructor. This constructor takes another Smb4KWorkgroup item
     * as argument and copies its values.
     *
     * @param workgroup       The Smb4KWorkgroup item that is to be copied.
     */
    Smb4KWorkgroup( const Smb4KWorkgroup &workgroup );

    /**
     * The empty constructor. It does not take any argument and you have to
     * set all information by the other functions provided by this class.
     */
    Smb4KWorkgroup();

    /**
     * The destructor.
     */
    ~Smb4KWorkgroup();

    /**
     * Sets the name of the workgroup.
     *
     * @param name            The name of the workgroup
     */
    void setWorkgroupName( const QString &name );

    /**
     * This function returns the name of the workgroup.
     *
     * @returns the workgroup name.
     */
    const QString &workgroupName() const { return m_name; }

    /**
     * Set the master browser for the workgroup or domain. You have to
     * provide at least its name. Additionally, you can pass its IP
     * address if it's available. However, it will only be accepted
     * if it is compatible with either IPv4 or IPv6. The @p pseudo
     * parameter defines whether this is a real master browser or a
     * faked one, i.e. a pseudo master.
     *
     * @param masterName      The name of the master browser
     *
     * @param masterIP        The IP address of the master browser
     *
     * @param pseudoMaster    Set this to TRUE if this is a pseudo master.
     */
    void setMasterBrowser( const QString &masterName,
                           const QString &masterIP = QString(),
                           bool pseudoMaster = false );

    /**
     * Sets the name of the master browser of this workgroup or domain.
     *
     * @param masterName      The name of the master browser
     */
    void setMasterBrowserName( const QString &name );

    /**
     * Returns the name of the master browser of this workgroup or domain.
     *
     * @returns the name of the master browser.
     */
    const QString &masterBrowserName() const { return m_master_name; }

    /**
     * Set the IP address of the master browser. @p ip will only be accepted
     * if it is compatible with either IPv4 or IPv6.
     *
     * @param ip              The master browser's IP address
     */
    void setMasterBrowserIP( const QString &ip );

    /**
     * Returns the IP address of the master browser of this workgroup
     * or domain. If the IP address was not compatible with IPv4 and
     * IPv6 or if no IP address was supplied, an empty string is returned.
     *
     * @returns the IP address of the master browser or an empty string.
     */
    const QString &masterBrowserIP() const { return m_master_ip; }

    /**
     * Set @p pseudo to TRUE if the master browser of this workgroup is a
     * pseudo-master, i.e. the master browser indeed carries a browse list but you
     * could not figure out if it is the workgroup master browser.
     *
     * @param pseudo          Should be set to TRUE if the workgroup has a pseudo-master
     *                        browser.
     */
    void setHasPseudoMasterBrowser( bool pseudo );

    /**
     * Returns TRUE if the workgroup has a pseudo (i.e. a faked) master browser.
     * Otherwise it returns FALSE.
     *
     * @returns TRUE if the workgroup has a pseudo master browser.
     */
    bool hasPseudoMasterBrowser() const { return m_pseudo_master; }

    /**
     * Returns TRUE if the item is empty and FALSE otherwise. An item is not
     * empty if at least one string (workgroup name, master name, etc.) has been
     * set. A modified boolean will not be considered.
     *
     * @returns TRUE if the item is empty.
     */
    bool isEmpty() const;

    /**
     * Compare another Smb4KWorkgroup object with this one an return TRUE if both carry
     * the same data.
     *
     * @param workgroup       The Smb4KWorkgroup object that should be compared with this
     *                        one.
     *
     * @returns TRUE if the data that was compared is the same.
     */
    bool equals( Smb4KWorkgroup *workgroup ) const;
    
    /**
     * Operator to check if two items are equal.
     */
    bool operator==( Smb4KWorkgroup workgroup ) { return equals( &workgroup ); }
    
    /**
     * Returns TRUE if the workgroup/domain master browsers IP address is set and 
     * FALSE otherwise.
     *
     * @returns TRUE if the master browsers IP address is known.
     */
    bool hasMasterBrowserIP() const { return !m_master_ip.isEmpty(); }

  private:
    /**
     * The name of the workgroup.
     */
    QString m_name;

    /**
     * The master browser's name.
     */
    QString m_master_name;

    /**
     * The master browser's IP address.
     */
    QString m_master_ip;

    /**
     * Defines if this is a master browser.
     */
    bool m_pseudo_master;

    /**
     * This function checks if the given IP address is either
     * compatible with IPv4 or IPv6. If it is not, an empty string
     * is returned.
     *
     * @param ip              The IP address that needs to be checked.
     *
     * @returns the IP address or an empty string if the IP address
     * is not compatible with either IPv4 or IPv6.
     */
    const QString &ipIsValid( const QString &ip );
};

#endif
