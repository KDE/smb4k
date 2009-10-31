/***************************************************************************
    smb4ksynchronizer_p  -  This file contains private helper classes for
    the Smb4KSynchronizer class.
                             -------------------
    begin                : Fr Okt 24 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

#ifndef SMB4KSYNCHRONIZER_P_H
#define SMB4KSYNCHRONIZER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QThread>

// application specific includes
#include <smb4ksynchronizer.h>
#include <smb4ksynchronizationinfo.h>
#include <smb4kprocess.h>


class SynchronizationThread : public QThread
{
  Q_OBJECT

  public:
    SynchronizationThread( QObject *parent = 0 );
    ~SynchronizationThread();
    void synchronize( Smb4KSynchronizationInfo *info,
                      const QString &command );
    Smb4KSynchronizationInfo *synchronizationInfo() { return m_info; }
    Smb4KProcess *process() { return m_proc; }

  signals:
    void progress( Smb4KSynchronizationInfo *info );

  protected slots:
    void slotProcessOutput();
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus );

  private:
    Smb4KSynchronizationInfo *m_info;
    Smb4KProcess *m_proc;
    QString m_stderr;
};


class Smb4KSynchronizerPrivate
{
  public:
    Smb4KSynchronizerPrivate();
    ~Smb4KSynchronizerPrivate();
    Smb4KSynchronizer instance;
};

#endif
