/***************************************************************************
    This class provides the interface to the libsmbclient library.
                             -------------------
    begin                : Sa Oct 20 2018
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

#ifndef SMB4KCLIENT_H
#define SMB4KCLIENT_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QScopedPointer>

// KDE includes
#include <KCoreAddons/KCompositeJob>

// forward declarations
class Smb4KClientPrivate;
class Smb4KBasicNetworkItem;
class Smb4KClientJob;

class Q_DECL_EXPORT Smb4KClient : public KCompositeJob
{
  Q_OBJECT
  
  public:
    /**
     * The constructor
     */
    explicit Smb4KClient(QObject *parent = 0);
    
    /**
     * The destructor
     */
    ~Smb4KClient();
    
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KClient class.
     */
    static Smb4KClient *self();
    
    /**
     * This function starts the composite job
     */
    void start();
    
    /**
     * Returns TRUE, if jobs are running and FALSE otherwise
     * 
     * @returns TRUE if jobs are running
     */
    bool isRunning();
    
    /**
     * Aborts all subjobs
     */
    void abort();
    
    /**
     * This function starts the scan for all available workgroups and domains
     * on the network neighborhood.
     */
    void lookupDomains();
    
    /**
     * This function looks up all hosts in a certain domain or workgroup.
     * 
     * @param workgroup       The workgroup object
     */
    void lookupDomainMembers(const WorkgroupPtr &workgroup);
    
    /**
     * This function looks up all shared resources a certain @p host provides.
     * 
     * @param host            The host object
     */
    void lookupShares(const HostPtr &host);
    
    /**
     * This function looks up all files and directories present at the location
     * @p item points to. The network item must be of type Smb4KGlobal::Share or
     * Smb4KGlobal::Directory.
     * 
     * @param item            The network item object
     */
    void lookupFiles(const NetworkItemPtr &item);
    
    /**
     * This function opens the preview dialog for @p share.
     * 
     * @param share           The share object
     */
    void openPreviewDialog(const SharePtr &share);
    
  Q_SIGNALS:
    /**
     * This signal is emitted when the client starts its work.
     * 
     * @param item          The network item
     * @param type          The type of work
     */
    void aboutToStart(const NetworkItemPtr &item, int type);
    
    /**
     * This signal is emitted when the client finished its work.
     * 
     * @param item          The network item
     * @param type          The type of work
     */
    void finished(const NetworkItemPtr &item, int type);
    
    /**
     * Emitted when the requested list of workgroups was acquired
     */
    void workgroups();
    
    /**
     * Emitted when the requested list of workgroup members was acquired
     * 
     * @param workgroup     The workgroup that was queried
     */
    void hosts(const WorkgroupPtr &workgroup);
    
    /**
     * Emitted when the requested list of shares was acquired
     * 
     * @param host          The host that was queried
     */
    void shares(const HostPtr &host);
    
    /**
     * Emitted when the requested list of files and directories was acquired
     * 
     * @param list          The list of files and directories
     */
    void files(const QList<FilePtr> &list);
    
  protected Q_SLOTS:
    /**
     * Start the composite job
     */
    void slotStartJobs();
    
    /**
     * Called when a job finished
     */
    void slotJobFinished(KJob *job);
    
    /**
     * Called when the application is about to be closed
     */
    void slotAboutToQuit();
    
    /**
     * Start a network query
     */
    void slotStartNetworkQuery(NetworkItemPtr item);
    
  private:
    /**
     * Process errors
     */
    void processErrors(KJob *job);
    
    /**
     * Process the domains/workgroups retrieved from the network
     * 
     * @param job             The client job
     */
    void processWorkgroups(Smb4KClientJob *job);
    
    /**
     * Process the domain/workgroup members
     * 
     * @param job             The client job
     */
    void processHosts(Smb4KClientJob *job);
    
    /**
     * Process the shares
     * 
     * @param job             The client job
     */
    void processShares(Smb4KClientJob *job);
    
    /**
     * Process the files and directories
     * 
     * @param job             The client job
     */
    void processFiles(Smb4KClientJob *job);
      
    /**
     * Pointer to the Smb4KClientPrivate class
     */
    QScopedPointer<Smb4KClientPrivate> d;
};

#endif

