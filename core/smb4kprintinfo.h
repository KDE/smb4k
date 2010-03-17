/***************************************************************************
    smb4kprintinfo  -  This is a container that carries information needed
    for printing.
                             -------------------
    begin                : Mo Apr 19 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
    email                : dustpuppy@mail.berlios.de
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

#ifndef SMB4KPRINTINFO_H
#define SMB4KPRINTINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>

// KDE includes
#include <kdemacros.h>

// application specific includes
#include <smb4kshare.h>

// forward declarations
class Smb4KAuthInfo;

/**
 * This class provides a container that holds all the info
 * that is needed to print a file.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KPrintInfo
{
  public:
    /**
     * The constructor.
     *
     * @param printer       The Smb4KShare item that represents the printer
     */
    Smb4KPrintInfo( Smb4KShare *printer );

    /**
     * Copy constructor
     *
     * @param info          The Smb4KPrintInfo object that is to be copied.
     */
    Smb4KPrintInfo( const Smb4KPrintInfo &info );

    /**
     * Empty constructor.
     */
    Smb4KPrintInfo() {}

    /**
     * The destructor.
     */
    ~Smb4KPrintInfo();

    /**
     * Returns the Smb4KShare object that represents the printer.
     * You need to use this to access the information about the printer.
     *
     * @returns the share object representing the share.
     */
    const Smb4KShare *printer() const { return &m_share; }

    /**
     * Set the Smb4KShare object that represents the printer. This function
     * does not check itself if the share is indeed a printer. You need to
     * do this yourself.
     *
     * @param share       The Smb4KShare object
     */
    void setPrinter( Smb4KShare *printer );

    /**
     * Returns the path of the file that is to be printed.
     *
     * @returns the path of the file.
     */
    const QString &filePath() const { return m_path; }

    /**
     * Sets the path to the file that is to be printed.
     *
     * @param path        The file's path
     */
    void setFilePath( const QString &path );

    /**
     * Returns the number of copies the user wants to have.
     */
    int copies() const { return m_copies; }

    /**
     * Sets the number of copies.
     */
    void setCopies( int num );

    /**
     * Returns TRUE if @p info has equal entries as this object. Otherwise FALSE
     * is returned.
     *
     * @returns TRUE is @p info equals this object.
     */
    bool equals( Smb4KPrintInfo *info ) const;
    
    /**
     * Operator to check if two infos are equal.
     */
    bool operator==( Smb4KPrintInfo info ) { return equals( &info ); }

    /**
     * Set the authentication information for the printer. This function will add
     * the authentication information to the URL of the printer. Any previous
     * user information will be overwritten.
     *
     * @param authInfo    The authentication information
     */
    void setAuthInfo( Smb4KAuthInfo *authInfo );

    /**
     * Return the full device URI of the remote printer. This is a convenience function.
     * You can also use printer()->unc() for this.
     *
     * The authentication for the URI can be set with the setAuthInfo() function.
     *
     * @returns the full URI of the printer.
     */
    QString deviceURI() { return m_share.unc( QUrl::None ); }

  private:
    /**
     * The share item
     */
    Smb4KShare m_share;

    /**
     * The path to the file to print.
     */
    QString m_path;

    /**
     * Holds the number of copies the user wants to have.
     */
    int m_copies;

    /**
     * The process id
     */
    int m_pid;
};

#endif

