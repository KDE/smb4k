/***************************************************************************
    smb4kprint_p  -  This file contains private helpers for the
    Smb4KPrint class
                             -------------------
    begin                : Fr Okt 31 2008
    copyright            : (C) 2008-2012 by Alexander Reinholdt
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

#ifndef SMB4KPRINT_P_H
#define SMB4KPRINT_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kprint.h"
#include "smb4kprocess.h"

// Qt includes
#include <QtGui/QPrinter>

// KDE includes
#include <kdialog.h>
#include <kurlrequester.h>
#include <knuminput.h>
#include <kurl.h>
#include <kjob.h>
#include <ktempdir.h>

// forward declarations
class Smb4KAuthInfo;
class Smb4KShare;


class Smb4KPrintJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * The constructor
     */
    Smb4KPrintJob( QObject *parent = 0 );
    
    /**
     * The destructor
     */
    ~Smb4KPrintJob();
    
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
     * Setup the printing process. You need to set the printer, the parent
     * widget is optional.
     * 
     * You must run this function before start() is called.
     * 
     * @param printer       The printer
     * 
     * @param parentWidget  The parent widget
     */
    void setupPrinting( Smb4KShare *printer,
                        QWidget *parentWidget = 0 );
    
    /**
     * Returns the printer share object
     * 
     * @returns the printer share object
     */
    Smb4KShare *printer() { return m_share; }
    
    /**
     * Returns the parent widget
     * 
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }
    
  signals:
    /**
     * Emitted when an authentication error happened.
     */
    void authError( Smb4KPrintJob *job );
    
    /**
     * Emitted when the printing is about to begin.
     */
    void aboutToStart( Smb4KShare *share );
    
    /**
     * Emitted after the printing finished.
     */
    void finished( Smb4KShare *share );
    
  protected:
    /**
     * Reimplemented from KJob. Kills the internal process and
     * then the job itself.
     */
    bool doKill();
    
  protected slots:
    void slotStartPrinting();
    void slotReadStandardOutput();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode, QProcess::ExitStatus status );
    
  private:
    bool m_started;
    Smb4KProcess *m_proc;
    Smb4KShare *m_share;
    QWidget *m_parent_widget;
    QString m_temp_dir;
};


class Smb4KPrintDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param share       The Smb4KShare item representing the printer.
     *
     * @param parent      The parent widget of this dialog.P
     */
    Smb4KPrintDialog( Smb4KShare *share,
                      QPrinter *printer,
                      QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KPrintDialog();
    
    /**
     * Returns the URL of the file that was chosen to be printed.
     * 
     * @returns the URL of the file
     */
    const KUrl &fileURL() { return m_url; }
    
    /**
     * Returns the QPrinter object
     * 
     * @returns the QPrinter object
     */
    QPrinter *printer() { return m_printer; }
    
  protected slots:
    /**
     * This slot is called when the User1 (i.e. the "Close") button
     * has been clicked.
     */
    void slotUser1Clicked();

    /**
     * This slot is called when the User2 (i.e. the "Print") button has been clicked.
     */
    void slotUser2Clicked();

    /**
     * This slot is being enabled if there is input text.
     *
     * @param text        The input text.
     */
    void slotInputValueChanged( const QString &text );

  private:
    /**
     * Set up the view.
     */
    void setupView( Smb4KShare *share );
    
    /**
     * The printer object
     */
    QPrinter *m_printer;

    /**
     * The url requester
     */
    KUrlRequester *m_file;

    /**
     * The copies input
     */
    KIntNumInput *m_copies;
    
    /**
     * The file URL
     */
    KUrl m_url;
};


class Smb4KPrintStatic
{
  public:
    Smb4KPrint instance;
};

#endif
