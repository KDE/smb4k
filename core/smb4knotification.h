/***************************************************************************
    smb4knotification  -  This class provides notifications for Smb4K.
                             -------------------
    begin                : Son Jun 27 2010
    copyright            : (C) 2010 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.org
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

#ifndef SMB4KNOTIFICATION_H
#define SMB4KNOTIFICATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>

// KDE includes
#include <knotification.h>
#include <kdemacros.h>

// application specific includes
#include <smb4kshare.h>

class KDE_EXPORT Smb4KNotification : public QObject
{
  Q_OBJECT
  
  public:
    /**
     * The constructor
     */
    Smb4KNotification( QObject *parent = 0 );
    
    /**
     * The destructor
     */
    ~Smb4KNotification();
    
    //
    // Notifications
    // 
    
    /**
     * Notify the user that a share has been mounted.
     * 
     * @param share     The share that has been mounted
     */
    void shareMounted( Smb4KShare *share );
    
    /**
     * Notify the user that a share has been unmounted.
     *
     * @param share     The share that has been unmounted
     */
    void shareUnmounted( Smb4KShare *share );
    
    /**
     * Notify the user that shares have been remounted.
     * 
     * @param total     The number of remounts that were scheduled
     * 
     * @param actual    The number of remounts that were actually 
     *                  mounted.
     */
    void sharesRemounted( int total, int actual );
    
    /**
     * Notify the user that all shares have been unmounted at once.
     * 
     * @param total     The number of unmounts that were scheduled
     * 
     * @param actual    The number of unmounts that actually finished 
     *                  successfully.
     */
    void allSharesUnmounted( int total, int actual );
    
    //
    // Warnings
    //
    
    /**
     * Warn the user that the wallet could not be opened.
     * 
     * @param name      The name of the wallet
     */
    void openingWalletFailed( const QString &name );
    
    /**
     * Warn the user that the logins stored in the wallet could not
     * be accessed.
     */
    void loginsNotAccessible();
    
  protected slots:
    /**
     * This slot is invoked when the notification is closed or ignored.
     */
    void slotNotificationClosed();
    
    /**
     * Opens the contents of a share in a file manager
     */
    void slotOpenShare();
    
  private:
    Smb4KShare m_share;
};

#endif
