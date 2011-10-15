/***************************************************************************
    smb4ksearch_p  -  Private helper classes for Smb4KSearch class.
                             -------------------
    begin                : Mo Dez 22 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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

#ifndef SMB4KSEARCH_P_H
#define SMB4KSEARCH_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kjob.h>

// application specific includes
#include <smb4ksearch.h>
#include <smb4kprocess.h>
#include <smb4khost.h>

// forward declarations
class Smb4KShare;

class Smb4KSearchJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * The constructor
     */
    Smb4KSearchJob( QObject *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSearchJob();

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
     * Set up the search job. You need to set the search string, the
     * master browser and the parent widget are optional.
     *
     * You must run this function before start() is called.
     *
     * @param string          The search string
     *
     * @param master          The master browser
     *
     * @param parent          The parent widget
     */
    void setupSearch( const QString &string,
                      Smb4KHost *master = 0,
                      QWidget *parentWidget = 0 );

    /**
     * Returns the search string.
     *
     * @returns the search string
     */
    const QString &searchString() { return m_string; }

    /**
     * Returns the master browser object.
     *
     * @returns the master browser
     */
    Smb4KHost *masterBrowser() { return &m_master; }

    /**
     * Returns that parent widget.
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

  signals:
    /**
     * Emitted when the search is about to begin.
     */
    void aboutToStart( const QString &string );

    /**
     * Emitted after the search finished.
     */
    void finished( const QString &string );

    /**
     * Emitted when an authentication error happened.
     */
    void authError( Smb4KSearchJob *job );

    /**
     * Emitted with a search result
     */
    void result( Smb4KShare *share );

  protected:
    /**
     * Reimplemented from KJob. Kills the internal process and
     * then the job itself.
     */
    bool doKill();

  protected slots:
    void slotStartSearch();
    void slotReadStandardOutput();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode, QProcess::ExitStatus status );

  private:
    bool m_started;
    QString m_string;
    Smb4KHost m_master;
    QWidget *m_parent_widget;
    Smb4KProcess *m_proc;
};


class Smb4KSearchPrivate
{
  public:
    Smb4KSearchPrivate();
    ~Smb4KSearchPrivate();
    Smb4KSearch instance;
};

#endif
