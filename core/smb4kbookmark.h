/***************************************************************************
    smb4kbookmark  -  This is the bookmark container for Smb4K (next
    generation).
                             -------------------
    begin                : So Jun 8 2008
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

#ifndef SMB4KBOOKMARK_H
#define SMB4KBOOKMARK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>
#include <QUrl>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KShare;

/**
 * This is the container class for bookmarks in Smb4K. It is a complete
 * rewrite of the previous class and comes with several improvements.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KBookmark
{
  public:
    /**
     * The constructor.
     *
     * @param share           The share for which the bookmark should
     *                        be created.
     *
     * @param label           The optional bookmark label.
     */
    Smb4KBookmark( Smb4KShare *share,
                   const QString &label = QString() );

    /**
     * The copy constructor.
     *
     * @param bookmark        The bookmark that should be copied.
     */
    Smb4KBookmark( const Smb4KBookmark &bookmark );

    /**
     * The empty constructor.
     */
    Smb4KBookmark();

    /**
     * The destructor
     */
    ~Smb4KBookmark();

    /**
     * Set the workgroup name.
     *
     * @param workgroup       The workgroup where the share is located.
     */
    void setWorkgroupName( const QString &workgroup );

    /**
     * Returns the workgroup/domain name.
     *
     * @returns the workgroup/domain name.
     */
    const QString &workgroupName() const { return m_workgroup; }

    /**
     * Set the host name.
     *
     * @param host            The host where the share is located.
     */
    void setHostName( const QString &host );

    /**
     * Returns the host name.
     *
     * @returns the host name.
     */
    QString hostName() const { return m_url.host().toUpper(); }

    /**
     * Set the share name.
     *
     * @param share           The share name
     */
    void setShareName( const QString &share );

    /**
     * Returns the share name.
     *
     * @returns the share name.
     */
    QString shareName() const;

    /**
     * Set the host's IP address.
     *
     * @param ip              The host's IP address
     */
    void setHostIP( const QString &ip );

    /**
     * Returns the host's IP address.
     *
     * @returns the host's IP address.
     */
    const QString &hostIP() const { return m_ip; }

    /**
     * Set the share's type.
     *
     * @param type            The type of the share.
     */
    void setTypeString( const QString &type );

    /**
     * Returns the share's type.
     *
     * @returns the type of the share.
     */
    const QString &typeString() const { return m_type; }

    /**
     * This function sets the UNC (Uniform Naming Convention string). This
     * has to conform to the following scheme: [smb:]//[USER@]HOST/SHARE.
     *
     * The UNC may contain the protocol, i.e. "smb://". If a wrong protocol or a mal-
     * formatted UNC is passed, this function will return immediately without doing
     * anything.
     *
     * @param unc           The UNC of the share/bookmark
     */
    void setUNC( const QString &unc );

    /**
     * Returns the UNC in the form [smb:]//[USER@]HOST[:PORT]/SHARE depending on
     * the format specified by @p options.
     *
     * @returns the UNC.
     */
    QString unc( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                   QUrl::RemoveUserInfo|
                                                   QUrl::RemovePort ) const;

    /**
     * Returns the host's UNC in the form [smb:]//[USER@]HOST[:PORT] depending on
     * the format specified by @p options.
     *
     * @returns the UNC of the host.
     */
    QString hostUNC( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                       QUrl::RemoveUserInfo|
                                                       QUrl::RemovePort ) const;

    /**
     * Set the (optional) the bookmark's label.
     *
     * @param label           The bookmark's label
     */
    void setLabel( const QString &label );

    /**
     * Returns the bookmark's label.
     *
     * @returns the bookmark's label.
     */
    const QString &label() const { return m_label; }

    /**
     * Set the profile for which this custom options are to be used.
     *
     * @param name              The name of the profile.
     */
    void setProfile( const QString &name );

    /**
     * Return the name of the profile for which the options are to be
     * used.
     *
     * @returns the name of the profile for which the options should be
     * used.
     */
    const QString &profile() const { return m_profile; }

    /**
     * Sets the login that is used to mount this share.
     *
     * @param login           The login
     */
    void setLogin( const QString &login );

    /**
     * Returns the login that is used to mount this share.
     *
     * @returns the login.
     */
    QString login() const { return m_url.userName(); }
    
    /**
     * Compare another Smb4KBookmark object with this one an return TRUE if both carry
     * the same data.
     *
     * @param bookmark        The Smb4KBookmark object that should be compared with this
     *                        one.
     *
     * @returns TRUE if the data that was compared is the same.
     */
    bool equals( Smb4KBookmark *bookmark ) const;
    
    /**
     * Operator to check if two authentication informations are equal.
     */
    bool operator==( Smb4KBookmark bookmark ) { return equals( &bookmark ); }

  private:
    /**
     * The URL
     */
    QUrl m_url;

    /**
     * Workgroup
     */
    QString m_workgroup;

    /**
     * Host IP address
     */
    QString m_ip;

    /**
     * Type
     */
    QString m_type;

    /**
     * Label
     */
    QString m_label;

    /**
     * Profile
     */
    QString m_profile;

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
