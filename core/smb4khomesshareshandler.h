/***************************************************************************
    smb4khomesshareshandler  -  This class handles the homes shares.
                             -------------------
    begin                : Do Aug 10 2006
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

#ifndef SMB4KHOMESSHARESHANDLER_H
#define SMB4KHOMESSHARESHANDLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QString>
#include <QStringList>
#include <QPair>
#include <QList>

// KDE includes
#include <kdialog.h>
#include <kdemacros.h>

// forward declarations
class Smb4KShare;
class Smb4KAuthInfo;


/**
 * This class belongs to the core of Smb4K and takes care of the
 * user names that are/were defined for a certain 'homes' share.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KHomesSharesHandler : public QObject
{
  Q_OBJECT

  friend class Smb4KHomesSharesHandlerPrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KHomesSharesHandler *self();

    /**
     * This function will open a dialog where the user has to define a
     * user name to access a 'homes' share. The name of @p share will be set accordingly.
     *
     * @param share       The share that is representing the homes share
     *
     * @param parent      The parent widget. If you go with the default value 0 and
     *                    @ref kapp->activeWindow() is not NULL, it will be used as parent.
     *
     * @returns TRUE if user has been chosen and FALSE otherwise.
     */
    bool specifyUser( Smb4KShare *share,
                      QWidget *parent = 0 );

    /**
     * Read and set the user names that are defined for the 'homes' share represented
     * by @p share. In most cases you do not need to use this function, because the 'homes'
     * share will already carry the list of users with it.
     *
     * If this share does not represent a homes share, this function will just return.
     *
     * @param share       The 'homes' share
     */
    void setHomesUsers( Smb4KShare *share );

    /**
     * Read and set the user names that are defined for the 'homes' share represented
     * by @p authInfo. In most cases you do not need to use this function, because the
     * authentication information will already carry the homes users.
     *
     * If this authentication information does not represent a homes share, this function
     * will just return.
     *
     * @param authInfo    The authentication information
     */
    void setHomesUsers( Smb4KAuthInfo *authInfo );

  protected slots:
    /**
     * Is connected to the textChanged() signal of the combo box
     * in the "Specify User" dialog.
     *
     * @param text        The text in the combo box
     */
    void slotTextChanged( const QString &text );

    /**
     * This slot is called if the User1 button, i.e. the "Clear" button
     * in the "Specify User" dialog has been clicked. It removes all
     * entries from the combo box.
     */
    void slotClearClicked();

  private:
    /**
     * The constructor
     */
    Smb4KHomesSharesHandler();

    /**
     * The destructor
     */
    ~Smb4KHomesSharesHandler();

    /**
     * Load the host and user names into a map.
     */
    void readUserNames();

    /**
     * Save the host and user names.
     */
    void writeUserNames();

    /**
     * Find a particular share in the list. This function takes @p share,
     * extracts the needed information and searches the list if a homes
     * share with the same workgroup and host name is defined. If it cannot
     * find such a share, NULL is returns and the share otherwise.
     *
     * @param share           The share that is searched
     *
     * @returns a pointer to the share carrying the needed information or NULL.
     */
    Smb4KShare *findShare( Smb4KShare *share );

    /**
     * Find a particular share in the list. This function takes @p authInfo,
     * extracts the needed information and searches the list if a homes
     * share with the same workgroup and host name is defined. If it cannot
     * find such a share, NULL is returns and the share otherwise.
     *
     * @param authInfo        The authentication information
     *
     * @returns a pointer to the share carrying the needed information or NULL.
     */
    Smb4KShare *findShare( Smb4KAuthInfo *authInfo );

    /**
     * This is the dialog that's shown the user when he/she has to
     * provide a user name.
     */
    KDialog *m_dlg;

    /**
     * This map carries all information about the homes shares.
     */
    QList<Smb4KShare> m_list;
};

#endif
