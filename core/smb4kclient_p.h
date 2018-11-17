/***************************************************************************
    Private classes for the SMB client
                             -------------------
    begin                : So Oct 21 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KCLIENT_P_H
#define SMB4KCLIENT_P_H

// application specific includes
#include "smb4kclient.h"
#include "smb4kglobal.h"
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kfile.h"

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QUrl>
#include <QHostAddress>
#include <QDialog>
#include <QListWidgetItem>

// KDE includes
#include <KCoreAddons/KJob>


class Smb4KClientJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KClientJob(QObject *parent = 0);
    
    /**
     * Destructor
     */
    ~Smb4KClientJob();
    
    /**
     * Error enumeration
     * 
     * @param OutOfMemoryError    Out of memory
     * @param NullContextError    The passed SMBCCTX context was a null pointer
     * @param SmbConfError        The smb.conf file could not be read
     * @param AccessDeniedError   Permission denied
     * @param InvalidUrlError     The URL that was passed is invalid
     * @param NonExistentUrlError The URL does not exist
     * @param NoDirectoryError    The name is not a directory
     * @param NetPermittedError   The operation is not permitted
     * @param NotFoundError       The network item was not found
     */
    enum {
      OutOfMemoryError = UserDefinedError,
      NullContextError,
      SmbConfError,
      AccessDeniedError,
      InvalidUrlError,
      NonExistentUrlError,
      NoDirectoryError,
      NotPermittedError,
      NotFoundError,
      UnknownError
    };
    
    /**
     * Starts the job.
     */
    void start();
    
    /**
     * Set the basic network item
     */
    void setNetworkItem(NetworkItemPtr item);
    
    /**
     * Return the basic network item
     */
    NetworkItemPtr networkItem() const;
    
    /**
     * The authentication function for libsmbclient
     */
    void get_auth_data_fn(const char *server, 
                          const char *share,
                          char *workgroup,
                          int maxLenWorkgroup,
                          char *username,
                          int maxLenUsername,
                          char *password,
                          int maxLenPassword);
    /**
     * The list of workgroups that was discovered.
     */
    QList<WorkgroupPtr> workgroups();
    
    /**
     * The list of hosts that was discovered.
     */
    QList<HostPtr> hosts();
    
    /**
     * The list shares that was discovered.
     */
    QList<SharePtr> shares();
    
    /**
     * The ist of files and directories that were discovered.
     */
    QList<FilePtr> files();
    
    /**
     * The workgroup
     */
    QString workgroup();
    
  protected Q_SLOTS:
    void slotStartJob();
    
  private:
    QHostAddress lookupIpAddress(const QString &name);
    SMBCCTX *m_context;
    NetworkItemPtr m_item;
    QList<WorkgroupPtr> m_workgroups;
    QList<HostPtr> m_hosts;
    QList<SharePtr> m_shares;
    QList<FilePtr> m_files;
};


class Smb4KPreviewDialog : public QDialog
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    Smb4KPreviewDialog(const SharePtr &share, QWidget *parent = 0);
    
    /**
     * Destructor
     */
    ~Smb4KPreviewDialog();
    
    /**
     * Returns the share that is previewed
     * 
     * @returns the share
     */
    SharePtr share() const;
    
  Q_SIGNALS:
    /**
     * Request a preview
     */
    void requestPreview(NetworkItemPtr item);
    
    /**
     * Emitted when the dialog is about to close.
     */
    void aboutToClose(Smb4KPreviewDialog *dialog);
    
  protected Q_SLOTS:
    /**
     * Do last things before the dialog is closed
     */
    void slotClosingDialog();
    
    /**
     * Reload
     */
    void slotReloadActionTriggered();
    
    /**
     * Go up
     */
    void slotUpActionTriggered();
    
    /**
     * An URL was activated in the combo box
     * 
     * @param url         The activated URL
     */
    void slotUrlActivated(const QUrl &url);
    
    /**
     * Process the selected item
     * 
     * @param item        The activated item
     */
    void slotItemActivated(QListWidgetItem *item);
    
    /**
     * Initialize the preview
     */
    void slotInitializePreview();
    
    /**
     * Get the preview results and process them
     * 
     * @param list        The list of the preview results
     */
    void slotPreviewResults(const QList<FilePtr> &list);
    
    
  private:
    SharePtr m_share;
    NetworkItemPtr m_currentItem;
    QList<FilePtr> m_listing;
};


class Smb4KClientPrivate
{
  public:
    QList<Smb4KPreviewDialog *> previewDialogs;
};


class Smb4KClientStatic
{
  public:
    Smb4KClient instance;
};

#endif



