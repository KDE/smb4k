/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
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

#ifndef SMB4KMOUNTER_P_H
#define SMB4KMOUNTER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QThread>
#include <QString>
#include <QList>

// KDE includes
#include <kdebug.h>
#include <kprocess.h>

// system includes
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __FreeBSD__
#include <sys/statfs.h>
#include <unistd.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif
#include <errno.h>

// application specific includes
#include <smb4kshare.h>
#include <smb4kmounter.h>
#include <smb4kauthinfo.h>


class CheckThread : public QThread
{
  Q_OBJECT

  public:
    CheckThread( Smb4KShare *share,
                 QObject *parent = 0 );
    ~CheckThread();
    void check();

  private:
    Smb4KShare *m_share;
};


class Smb4KMounterPrivate
{
  public:
    Smb4KMounterPrivate();
    ~Smb4KMounterPrivate();
    Smb4KMounter instance;
    void setAboutToQuit();
    bool aboutToQuit() { return m_quit; }
    void setHardwareReason( bool hardware );
    bool hardwareReason() { return m_hardware; }

  private:
    bool m_quit;
    bool m_hardware;
};

#endif
