/***************************************************************************
    smb4kpreviewitem  -  A container for previews of a remote share
                             -------------------
    begin                : Mo Mai 28 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
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

#ifndef SMB4KPREVIEWITEM_H
#define SMB4KPREVIEWITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>
#include <QPair>
#include <QList>
#include <QUrl>

// KDE includes
#include <kdemacros.h>

// application specific includes
#include <smb4kshare.h>

// forward declarations
class Smb4KAuthInfo;

typedef QPair<int, QString> ContentsItem;

/**
 * This class provides a container for the preview of the contents of a remote
 * SMB share.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KPreviewItem
{
  public:
    /**
     * The default constructor.
     *
     * @param share             The share for that a preview should be collected.
     *
     * @param path              The path for that the preview should be collected.
     */
    Smb4KPreviewItem( Smb4KShare *share,
                      const QString &path = "/" );

    /**
     * The empty constructor.
     */
    Smb4KPreviewItem() {}

    /**
     * The destructor.
     */
    ~Smb4KPreviewItem();

    /**
     * Return the share item. You need to use this function if you want
     * to access information about the share. Please note, that this share
     * item is merely a copy and that all changes you apply to it get lost
     * after the preview item was deleted.
     *
     * @returns the Smb4KShare item.
     */
    Smb4KShare *share() { return &m_share; }

    /**
     * Set the share item.
     *
     * @param share             The Smb4KShare object
     */
    void setShare( Smb4KShare *share );

    /**
     * Return the path that is to be previewed.
     */
    QString path() const;

    /**
     * Set the path for which the preview should be compiled. The root directory
     * has to be set as "/". If you forget this, it will be set for you.
     *
     * @param path              The path
     *
     * @note As soon as this function is used, the list of files and directories
     * will be cleared.
     */
    void setPath( const QString &path );

    /**
     * Returns the current location. The format is depending on the formatting
     * options @p options that are passed. By default, the remote path is shown
     * without user information, port and protocol (scheme).
     *
     * This function can be used for displaying in a preview dialog or for checks.
     *
     * @returns                 The current location/remote path
     */
    QString location( QUrl::FormattingOptions options =
                      QUrl::RemoveScheme|
                      QUrl::RemoveUserInfo|
                      QUrl::RemovePort ) const;

    /**
     * This enumeration is used for the contents. It determines if
     * an item is a file, a hidden file, a directory, or a hidden
     * directory.
     */
    enum Contents { File,
                    HiddenFile,
                    Directory,
                    HiddenDirectory };

    /**
     * Returns the contents of the location.
     *
     * @returns a map of (hidden) files and (hidden) directories.
     */
    const QList<ContentsItem> &contents() const { return m_contents; }

    /**
     * Add a file or directory to the contents.
     *
     * @param item              A ContentsItem object. This is a QPair<int,QString>
     *                          with the integer being a value from the Contents
     *                          enumeration and the string being the full path of
     *                          the file or directory.
     *
     * @see Smb4KPreviewItem::setPath() or Smb4KPreviewItem::clearContents() for how
     * the list of files and directories is cleared.
     */
    void addContents( const ContentsItem &item );

    /**
     * Clears the contents.
     */
    void clearContents();

    /**
     * This function returns TRUE if the current directory is the root directory of
     * the share and FALSE otherwise.
     *
     * @returns TRUE if the current path is the root path.
     */
    bool isRootDirectory();

    /**
     * Compare another Smb4KPreviewItem object @p item with this one. If the item
     * has the same contents, TRUE is returned. At the moment, only the location
     * string is compared.
     *
     * @returns TRUE if @p item is equal to this object.
     */
    bool equals( Smb4KPreviewItem *item );

    /**
     * Set the authentication information for the share that is to be previewed.
     * This function will add the authentication information to the URL of the share.
     * Any previous user information will be overwritten.
     *
     * @param authInfo    The authentication information
     */
    void setAuthInfo( Smb4KAuthInfo *authInfo );

  private:
    /**
     * The share item
     */
    Smb4KShare m_share;

    /**
     * The path
     */
    QString m_path;

    /**
     * This map stores the contents of the current
     * location.
     */
    QList<ContentsItem> m_contents;
};

#endif
