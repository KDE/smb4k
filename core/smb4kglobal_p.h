/***************************************************************************
    These are the private helper classes of the Smb4KGlobal namespace.
                             -------------------
    begin                : Di Jul 24 2007
    copyright            : (C) 2007-2017 by Alexander Reinholdt
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

#ifndef SMB4KGLOBAL_P_H
#define SMB4KGLOBAL_P_H

// application specific includes
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// Qt includes
#include <QList>
#include <QMap>
#include <QObject>
#include <QSharedPointer>

/**
 * This class is a private helper for the Smb4KGlobal namespace.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KGlobalPrivate : public QObject
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    Smb4KGlobalPrivate();

    /**
     * Destructor
     */
    ~Smb4KGlobalPrivate();

    /**
     * This is the global workgroup list.
     */
    QList<QSharedPointer<Smb4KWorkgroup>> workgroupsList;

    /**
     * This is the global host list.
     */
    QList<QSharedPointer<Smb4KHost>> hostsList;

    /**
     * This is global list of mounted shares.
     */
    QList<QSharedPointer<Smb4KShare>> mountedSharesList;

    /**
     * This is the global list of shares.
     */
    QList<QSharedPointer<Smb4KShare>> sharesList;
    
    /**
     * This list contains the search results.
     */
    QList<QSharedPointer<Smb4KShare>> searchResults;
    
    /**
     * The global options defined in smb.conf
     */
    const QMap<QString,QString> &globalSambaOptions(bool read);

    /**
     * Boolean that is TRUE when only foreign shares
     * are in the list of mounted shares
     */
    bool onlyForeignShares;

    /**
     * Set default values for some settings
     */
    void setDefaultSettings();

    /**
     * Make connections
     */
    void makeConnections();

    /**
     * Boolean that is TRUE if the core classes have
     * been initialized
     */
    bool coreInitialized;
    
    /**
     * Boolean that determines if the core classes should set
     * a busy cursor when they are doing something or not.
     */
    bool modifyCursor;
    
#ifdef Q_OS_LINUX
    /**
     * This list contains all whitelisted arguments for the mount.cifs binary and
     * is only present under the Linux operating system.
     */
    QStringList whitelistedMountArguments;
#endif

  protected Q_SLOTS:
    /**
     * This slot does last things before the application quits
     */
    void slotAboutToQuit();
    
  private:
    QMap<QString,QString> m_sambaOptions;
    bool m_sambaConfigMissing;
};

#endif
