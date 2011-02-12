/***************************************************************************
    smb4kglobal_p  -  This is the private helper class of the Smb4KGlobal
    namespace.
                             -------------------
    begin                : Di Jul 24 2007
    copyright            : (C) 2007-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KGLOBAL_P_H
#define SMB4KGLOBAL_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kuiserverjobtracker.h>

// Qt includes
#include <QList>
#include <QMap>

// application specific includes
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kshare.h>

/**
 * This class is a private helper for the Smb4KGlobal namespace.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KGlobalPrivate
{
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
    QList<Smb4KWorkgroup *> workgroupsList;

    /**
     * This is the global host list.
     */
    QList<Smb4KHost *> hostsList;

    /**
     * This is global list of mounted shares.
     */
    QList<Smb4KShare *> mountedSharesList;

    /**
     * This is the global list of shares.
     */
    QList<Smb4KShare *> sharesList;

    /**
     * The job tracker for KJobs
     */
    KUiServerJobTracker jobTracker;
};

#endif
