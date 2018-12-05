/***************************************************************************
    This file contains private helpers for the Smb4KPrint class
                             -------------------
    begin                : Fr Okt 31 2008
    copyright            : (C) 2008-2017 by Alexander Reinholdt
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

// application specific includes
#include "smb4kprint.h"
#include "smb4kprocess.h"
#include "smb4kglobal.h"

// Qt includes
#include <QTemporaryDir>
#include <QPrinter>
#include <QDialog>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>

// KDE includes
#include <KCoreAddons/KJob>
#include <KIOWidgets/KUrlRequester>

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
    explicit Smb4KPrintJob(QObject *parent = 0);
    
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
    void setupPrinting(const SharePtr &printer, QWidget *parentWidget = 0);
    
    /**
     * Returns the printer share object
     * 
     * @returns the printer share object
     */
    const SharePtr &printer() { return m_share; }
    
    /**
     * Returns the parent widget
     * 
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }
    
  Q_SIGNALS:
    /**
     * Emitted when an authentication error happened.
     */
    void authError(Smb4KPrintJob *job);
    
    /**
     * Emitted when the printing is about to begin.
     */
    void aboutToStart(const SharePtr &share);
    
    /**
     * Emitted after the printing finished.
     */
    void finished(const SharePtr &share);
    
  protected:
    /**
     * Reimplemented from KJob. Kills the internal process and
     * then the job itself.
     */
    bool doKill();
    
  protected Q_SLOTS:
    void processErrors(const QString &stdErr);
    void processOutput(const QString &stdOut);
    void slotStartPrinting();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus status);
    
  private:
    bool m_started;
    Smb4KProcess *m_process;
    SharePtr m_share;
    QWidget *m_parent_widget;
    QTemporaryDir m_tempDir;
    
};


// class Smb4KPrintDialog : public QDialog
// {
//   Q_OBJECT
// 
//   public:
//     /**
//      * The constructor.
//      *
//      * @param share       The Smb4KShare item representing the printer.
//      * 
//      * @param printer     The QPrinter object
//      *
//      * @param parent      The parent widget of this dialog.
//      */
//     Smb4KPrintDialog(const SharePtr &share, QPrinter *printer, QWidget *parent = 0);
// 
//     /**
//      * The destructor
//      */
//     ~Smb4KPrintDialog();
//     
//     /**
//      * Returns the URL of the file that was chosen to be printed.
//      * 
//      * @returns the URL of the file
//      */
//     const QUrl &fileURL() { return m_url; }
//     
//     /**
//      * Returns the QPrinter object
//      * 
//      * @returns the QPrinter object
//      */
//     QPrinter *printer() { return m_printer; }
//     
//   protected Q_SLOTS:
//     void slotPrintClicked();
//     void slotCancelClicked();
//     void slotOptionsClicked();
//     void slotInputValueChanged(const QString &text);
// 
//   private:
//     /**
//      * Set up the view.
//      */
//     void setupView(const SharePtr &share);
//     
//     /**
//      * The Print button
//      */
//     QPushButton *m_print_button;
//     
//     /**
//      * The Cancel button
//      */
//     QPushButton *m_cancel_button;
//     
//     /**
//      * The Options button
//      */
//     QPushButton *m_optionsButton;
//     
//     /**
//      * The Details widget
//      */
//     QGroupBox *m_options_box;
//     
//     /**
//      * The printer object
//      */
//     QPrinter *m_printer;
// 
//     /**
//      * The url requester
//      */
//     KUrlRequester *m_file;
// 
//     /**
//      * The copies input
//      */
//     QSpinBox *m_copies;
//     
//     /**
//      * The file URL
//      */
//     QUrl m_url;
// };


class Smb4KPrintStatic
{
  public:
    Smb4KPrint instance;
};

#endif
