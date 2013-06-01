/***************************************************************************
    smb4kprint  -  The (new) printing core class.
                             -------------------
    begin                : Son Feb 20 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KPRINT_H
#define SMB4KPRINT_H

// Qt includes
#include <QtCore/QUrl>
#include <QtGui/QWidget>

// KDE specific includes
#include <kcompositejob.h>
#include <kdemacros.h>

// forward declarations
class Smb4KPrintPrivate;
class Smb4KShare;
class Smb4KPrintJob;

/**
 * This class provides an interface to printer shares on your
 * network neighborhood.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */


class KDE_EXPORT Smb4KPrint : public KCompositeJob
{
  Q_OBJECT
  
  Q_PROPERTY( bool running READ isRunning )
  
  friend class Smb4KPrintPrivate;
  
  public:
    /**
     * The constructor
     */
    explicit Smb4KPrint( QObject *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KPrint();
    
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KPrint class.
     */
    static Smb4KPrint *self();
    
    /**
     * Prints a file on a remote printer. All you need to do is to pass 
     * the Smb4KShare object that represents the printer to this function.
     * The print dialog will then be shown to determine the file you want
     * to be printed.
     * 
     * This function will just return if the share is not a printer.
     * 
     * @param printer       The Smb4KShare object representing the printer
     *
     * @param parent        The parent widget
     */
    void print( Smb4KShare *printer,
                QWidget *parent = 0 );
    
    /**
     * This function tells you whether print jobs are running
     * or not.
     *
     * @returns TRUE if at least one print job is running
     */
    bool isRunning();
    
    /**
     * With this function you can test whether a print job for a certain 
     * share @param share is already running.
     * 
     * @returns TRUE if a print job is already running
     */
    bool isRunning( Smb4KShare *share );
    
    /**
     * This function aborts all print jobs at once.
     */
    Q_INVOKABLE void abortAll();

    /**
     * This function aborts the printing to a certain printer share.
     *
     * @param share         The Smb4KShare object
     */
    void abort( Smb4KShare *share );
    
    /**
     * This function starts the composite job
     */
    Q_INVOKABLE void start();
    
    /**
     * This function takes a QUrl object and initiates the printing of a file
     * on a remote printer. 
     * 
     * Please note that this function only works with share objects that are 
     * already known. All others will be ignored.
     * 
     * @param url         The URL of the remote printer share
     */
    Q_INVOKABLE void print( const QUrl &url );
    
  Q_SIGNALS:
    /**
     * This signal is emitted when a job is started.
     *
     * @param printer      The remote printer
     */
    void aboutToStart( Smb4KShare *printer );

    /**
     * This signal is emitted when a job has finished.
     *
     * @param printer      The remote printer
     */
    void finished( Smb4KShare *printer );
    
  protected Q_SLOTS:
    /**
     * Invoked by start() function
     */
    void slotStartJobs();
    
    /**
     * Called when a job finished
     */
    void slotJobFinished( KJob *job );
    
    /**
     * Called when an authentication error occurred
     */
    void slotAuthError( Smb4KPrintJob *job );

    /**
     * Called when the application exits
     */
    void slotAboutToQuit();
};

#endif
