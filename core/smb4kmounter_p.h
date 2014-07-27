/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2014 by Alexander Reinholdt
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

#ifndef SMB4KMOUNTER_P_H
#define SMB4KMOUNTER_P_H

// application specific includes
#include "smb4kmounter.h"
#include "smb4kshare.h"

// Qt includes
#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtGui/QCheckBox>

// KDE includes
#include <kdebug.h>
#include <klineedit.h>
#include <kdialog.h>
#include <kauth.h>
#include <kjob.h>

class Smb4KShare;

using namespace KAuth;


class Smb4KMountJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KMountJob( QObject* parent = 0 );
    
    /**
     * Destructor
     */
    ~Smb4KMountJob();
    
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
     * Set up the job for a single mount.
     *
     * You must run this function before start() is called.
     * 
     * @param share       The share that is to be mounted
     * 
     * @param parent      The parent widget
     */
    void setupMount( Smb4KShare *share,
                     QWidget *parent = 0 );
    
    /**
     * Set up the job for a bulk mount.
     * 
     * You must run this function before start() is called.
     * 
     * @param shares      The list of shares that are to be mounted
     * 
     * @param parent      The parent widget
     */
    void setupMount( const QList<Smb4KShare *> &shares, 
                     QWidget *parent = 0 );

    /**
     * Returns the list of shares for that authentication errors
     * were reported.
     *
     * @returns a list of shares with authentication errors
     */
    const QList<Smb4KShare *> &authErrors() { return m_auth_errors; }

    /**
     * Returns a list of shares for that a "bad share name" error
     * was reported. The have been corrected and the program should
     * retry.
     *
     * @returns a list of shares for that mounting should be retried
     */
    const QList<Smb4KShare *> &retries() { return m_retries; }

    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

  Q_SIGNALS:
    /**
     * This signal is emitted when shares are about to be mounted
     */
    void aboutToStart( const QList<Smb4KShare *> &shares );

    /**
     * This signal is emitted when all shares have been mounted
     */
    void finished( const QList<Smb4KShare *> &shares );

    /**
     * Emitted when an authentication error occurred
     */
    void authError( Smb4KMountJob *job );

    /**
     * Emitted if we should retry to mount a share
     */
    void retry( Smb4KMountJob *job );

    /**
     * Emitted when the share was actually mounted
     */
    void mounted( Smb4KShare *share );
    
  protected:
    /**
     * Reimplemented from KJob. Kills the internal process and
     * then the job itself.
     */
    bool doKill();

  protected Q_SLOTS:
    void slotStartMount();
    void slotActionFinished( ActionReply reply );
    void slotFinishJob();
    
  private:
    bool m_started;
    QList<Smb4KShare *> m_shares;
    QList<Smb4KShare *> m_auth_errors;
    QList<Smb4KShare *> m_retries;
    QWidget *m_parent_widget;
    bool createMountAction( Smb4KShare *share,
                            Action *action );
    int m_processed;
};


class Smb4KUnmountJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KUnmountJob( QObject* parent = 0 );
    
    /**
     * Destructor
     */
    ~Smb4KUnmountJob();
    
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
     * Set up the job for a single unmount.
     *
     * You must run this function before start() is called.
     * 
     * @param share       The share that is to be unmounted
     * 
     * @param force       Force unmounting
     * 
     * @param silent      Perform the unmounting silently. No reporting
     *
     * @param synchron    Start the job synchronously. slotStartUnmount() is
     *                    called directly.
     * 
     * @param parent      The parent widget
     */
    void setupUnmount( Smb4KShare *share,
                       bool force,
                       bool silent,
                       bool synchron,
                       QWidget *parent = 0 );
    
    /**
     * Set up the job for a bulk unmount.
     * 
     * You must run this function before start() is called.
     * 
     * @param shares      The list of shares that are to be unmounted
     * 
     * @param force       Force unmounting
     * 
     * @param silent      Perform the unmounting silently. No reporting
     *
     * @param synchron    Start the job synchronously. slotStartUnmount() is
     *                    called directly.
     * 
     * @param parent      The parent widget
     */
    void setupUnmount( const QList<Smb4KShare *> &shares,
                       bool force,
                       bool silent,
                       bool synchron,
                       QWidget *parent = 0 );
    
  Q_SIGNALS:
    /**
     * This signal is emitted when shares are about to be unmounted
     */
    void aboutToStart( const QList<Smb4KShare *> &shares );

    /**
     * This signal is emitted when all shares have been unmounted
     */
    void finished( const QList<Smb4KShare *> &shares );

    /**
     * Emitted when the share was actually mounted
     */
    void unmounted( Smb4KShare *share );
    
  protected:
    /**
     * Reimplemented from KJob. Kills the internal process and
     * then the job itself.
     */
    bool doKill();

  protected Q_SLOTS:
    void slotStartUnmount();
    void slotActionFinished( ActionReply reply );
    void slotFinishJob();
    
  private:
    bool m_started;
    bool m_force;
    bool m_silent;
    bool m_synchron;
    QList<Smb4KShare *> m_shares;
    QWidget *m_parent_widget;
    bool createUnmountAction( Smb4KShare *share,
                              Action *action );
    int m_processed;
};


class Smb4KMountDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent      The parent widget
     */
    explicit Smb4KMountDialog( Smb4KShare *share,
                               QWidget *parent = 0 );
    /**
     * The destructor.
     */
    ~Smb4KMountDialog();

    /**
     * Returns if the share should be bookmarked or not.
     *
     * @returns TRUE if the share should be bookmarked
     */
    bool bookmarkShare() { return m_bookmark->isChecked(); }
    
    /**
     * Returns if the user input is valid or not.
     * 
     * @returns TRUE if the user input is valid.
     */
    bool validUserInput() { return m_valid; }

  protected Q_SLOTS:
    /**
     * This slot is activated if the OK button has been clicked.
     */
    void slotOkClicked();

    /**
     * This slot is activated if the Cancel button has been clicked.
     */
    void slotCancelClicked();

    /**
     * This slot is being enabled if there is input text.
     *
     * @param text        The input text.
     */
    void slotChangeInputValue( const QString &text );

    /**
     * This slot is used for making text completion for the share edit line
     * work.
     */
    void slotShareNameEntered();

    /**
     * This slot is used for making text completion for the IP edit line
     * work.
     */
    void slotIPEntered();

    /**
     * This slot is used for making text completion for the workgroup edit
     * line work.
     */
    void slotWorkgroupEntered();

  private:
    /**
     * This function sets up the view.
     */
    void setupView();

    /**
     * The line edit where the user has to enter the share.
     */
    KLineEdit *m_share_input;

    /**
     * The line edit where the user can enter the IP address.
     */
    KLineEdit *m_ip_input;

    /**
     * The line edit where the user can enter the workgroup.
     */
    KLineEdit *m_workgroup_input;

    /**
     * This checkbox determines whether the share should be added to the
     * bookmarks.
     */
    QCheckBox *m_bookmark;

    /**
     * The share that is passed to the mounter.
     */
    Smb4KShare *m_share;
    
    /**
     * Valid user input?
     */
    bool m_valid;
};


class Smb4KMounterPrivate
{
  public:
    int importTimeout;
    int remountTimeout;
    int remountAttempts;
    int checks;
    Smb4KMountDialog *dialog;
    QList<Smb4KShare *> importedShares;
    QList<Smb4KShare *> retries;
    QList<Smb4KShare *> remounts;
    QStringList obsoleteMountpoints;
    bool aboutToQuit;
    bool hardwareReason;
    bool firstImportDone;
    bool importsAllowed;
};


class Smb4KMounterStatic
{
  public:
    Smb4KMounter instance;
};

#endif
