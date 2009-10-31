/***************************************************************************
    smb4kipaddressscanner_p  -  Private classes for the IP address scanner
    of Smb4K.
                             -------------------
    begin                : Mi Jan 28 2009
    copyright            : (C) 2009 by Alexander Reinholdt
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

#ifndef SMB4KIPADDRESSSCANNER_P_H
#define SMB4KIPADDRESSSCANNER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QThread>

// application specific includes
#include <smb4kipaddressscanner.h>
#include <smb4kbasicnetworkitem.h>
#include <smb4kprocess.h>

class IPScanThread : public QThread
{
  Q_OBJECT
  
  public:
    IPScanThread( Smb4KHost *host, 
                  QObject *parent = 0 );
    ~IPScanThread();
    void lookup( const QString &command );
    Smb4KHost *host() { return m_host; };
    Smb4KProcess *process() { return m_proc; }
    
  signals:
    void ipAddress( Smb4KHost *host );
    
  protected slots:
    void slotProcessOutput();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
                 
  private:
    Smb4KHost *m_host;
    Smb4KProcess *m_proc;
};


class Smb4KIPAddressScannerPrivate
{
  public:
    Smb4KIPAddressScannerPrivate();
    ~Smb4KIPAddressScannerPrivate();
    Smb4KIPAddressScanner instance;
};

#endif
