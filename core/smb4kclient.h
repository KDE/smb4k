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

// Qt includes
#include <QScopedPointer>

// KDE includes
#include <KCoreAddons/KCompositeJob>

// forward declarations
class Smb4KClientPrivate;
class Smb4KBasicNetworkItem;

class Smb4KClient : public KCompositeJob
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
     * This function starts the scan for all available domains
     * on the network.
     */
    void lookupDomains();
    
  Q_SIGNALS:
    /**
     * This signal is emitted when the client starts its work.
     * 
     * @param item          The network item
     * @param type          The type of work
     */
    void aboutToStart(Smb4KBasicNetworkItem *item, int type);
    
  protected Q_SLOTS:
    /**
     * Start the composite job
     */
    void slotStartJobs();
    
    /**
     * Called when a job finished
     */
    void slotJobFinished(KJob *job);
    
  private:
    /**
     * Pointer to the Smb4KClientPrivate class
     */
    QScopedPointer<Smb4KClientPrivate> d;
};

#endif

