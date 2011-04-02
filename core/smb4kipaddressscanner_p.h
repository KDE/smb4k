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
#include <QWidget>

// KDE includes
#include <kjob.h>

// application specific includes
#include <smb4kipaddressscanner.h>
#include <smb4kprocess.h>

class Smb4KIPLookupJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    Smb4KIPLookupJob( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KIPLookupJob();

    /**
     * Returns TRUE if the job has been started and FALSE otherwise
     *
     * @returns TRUE if the job has been started
     */
    bool isStarted() { return m_started; }

    /**
     * Starts the job
     */
    void start();

    /**
     * Set up the lookup job. You need to set the host, the
     * parent widget is optional.
     *
     * You must run this function before start() is called.
     *
     * @param host            The host
     *
     * @param parent          The parent widget
     */
    void setupLookup( Smb4KHost *host,
                      QWidget *parentWidget = 0 );

  signals:
    /**
     * This signal is emitted when a lookup process is about to be started.
     * It passes the host to the receiver.
     *
     * @param host          The host
     */
    void aboutToStart( Smb4KHost *host );

    /**
     * This signal is emitted when a lookup process has finished. It passes
     * the host to the receiver.
     *
     * @param host          The host
     */
    void finished( Smb4KHost *host );

    /**
     * This signal is emitted when an IP address was successfully looked
     * up.
     *
     * @param host          The host
     */
    void ipAddress( Smb4KHost *host );

  protected:
    bool doKill();

  protected slots:
    void slotStartLookup();
    void slotReadStandardOutput();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode, QProcess::ExitStatus status );

  private:
    bool m_started;
    Smb4KHost *m_host;
    QWidget *m_parent_widget;
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
