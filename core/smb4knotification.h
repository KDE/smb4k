/***************************************************************************
    smb4knotification  -  This class provides notifications for Smb4K.static_cast<Smb4KHost *>( m_item )
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

// forward declarations
class Smb4KBookmark;
class Smb4KWorkgroup;
class Smb4KHost;


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
    
    /**
     * Tell the user that the program sudo could not be found and the 
     * the feature will be disabled.
     */
    void sudoNotFound();
    
    /**
     * Tell the user that the program kdesudo could not be found and the 
     * the feature will be disabled.
     */
    void kdesudoNotFound();
    
    /**
     * Tell the user that the mimetype is not supported and that he/she 
     * should convert the file.
     * 
     * @param mimetype  The mimetype
     */
    void mimetypeNotSupported( const QString &mimetype );
    
    /**
     * Tell the user that the label he/she chose for the bookmark is already 
     * being used and that it will be changed automatically.
     * 
     * @param bookmark  The bookmark object
     */
    void bookmarkLabelInUse( Smb4KBookmark *bookmark );
    
    //
    // Errors
    //
    
    /**
     * This error message is shown if the list of workgroups could not 
     * be retrieved.
     *
     * @param err_msg   The error message
     */
    void retrievingDomainsFailed( const QString &err_msg );
    
    /**
     * This error message is shown if the scanning of the broadcast 
     * areas failed.
     *
     * @param err_msg   The error message
     */
    void scanningBroadcastAreaFailed( const QString &err_msg );
    
    /**
     * This error message is shown if the list of servers could not 
     * be retrieved.
     * 
     * @param err_msg   The error message
     */
    void retrievingServersFailed( Smb4KWorkgroup *workgroup, const QString &err_msg );
    
    /**
     * This error message is shown if the list of shares could not
     * be retrieved.
     * 
     * @param host      The host object
     * 
     * @param err_msg   The error message
     */
    void retrievingSharesFailed( Smb4KHost *host, const QString &err_msg );
    
    /**
     * This error message is shown if the preview could not be
     * retrieved.
     * 
     * @param err_meg   The error message
     */
    void retrievingPreviewFailed( Smb4KShare *share, const QString &err_msg );
    
    /**
     * This error message is shown if the mounting of a share failed.
     * 
     * @param share     The share that was to be mounted
     * 
     * @param err_msg   The error message
     */
    void mountingFailed( Smb4KShare *share, const QString &err_msg );
    
    
  protected slots:
    /**
     * This slot is invoked when the notification is closed or ignored.
     */
    void slotNotificationClosed();
    
    /**
     * Opens the contents of a share in a file manager
     */
    void slotOpenShare();
    
    /**
     * Shows an error message
     */
    void slotShowErrorMessage();
    
  private:
    Smb4KShare m_share;
    QStringList m_err_msg;
};

#endif
