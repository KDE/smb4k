/***************************************************************************
    smb4kscanner_p  -  Private helper classes for the scanner
                             -------------------
    begin                : So Mai 22 2011
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSCANNER_P_H
#define SMB4KSCANNER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>

// KDE includes
#include <kjob.h>

// application specific includes
#include <smb4kscanner.h>
#include <smb4kprocess.h>
#include <smb4kworkgroup.h>
#include <smb4kauthinfo.h>

// forward declarations
class Smb4KHost;
class Smb4KShare;

class Smb4KLookupDomainsJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    Smb4KLookupDomainsJob( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KLookupDomainsJob();

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
     * Sets up the lookup job.
     *
     * @param parent      The parent widget
     */
    void setupLookup( QWidget *parent = 0 );

    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

    /**
     * Returns the list of discovered workgroups. You can use this function
     * after the finished() signal has been emitted to retrieve the complete
     * list of workgroups.
     *
     * @returns the list of workgroups
     */
    const QList<Smb4KWorkgroup> &workgroupsList() { return m_workgroups_list; }

  signals:
    /**
     * This signal is emitted when a lookup process is about
     * to start
     */
    void aboutToStart();

    /**
     * This signal is emitted when a lookup process finished
     */
    void finished();
    
    /**
     * Emits the list of discovered workgroups and domains
     */
    void workgroups( const QList<Smb4KWorkgroup> &list );

  protected:
    bool doKill();

  protected slots:
    void slotStartLookup();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );

  private:
    void processWorkgroups();
    bool m_started;
    QWidget *m_parent_widget;
    Smb4KProcess *m_proc;
    QList<Smb4KWorkgroup> m_workgroups_list;
};


class Smb4KQueryMasterJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    Smb4KQueryMasterJob( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KQueryMasterJob();

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
     * Sets up the lookup job. With the @p master argument you can 
     * define a custom master browser that should be queried. If 
     * @p master is empty, the current master browser is queried.
     *
     * @param master        The master browser's name or IP address
     *
     * @param parent        The parent widget
     */
    void setupLookup( const QString &master,
                      QWidget *parent = 0 );
    
    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

    /**
     * Returns the name of either the custom master browser, if one was
     * passed, or, in the case of an authentication error, the master
     * browser that needs authentication.
     *
     * @returns the custom master browser's name
     */
    const QString &masterBrowser() { return m_master_browser; }

    /**
     * Returns the list of discovered workgroups. You can use this function
     * after the finished() signal has been emitted to retrieve the complete
     * list of workgroups.
     *
     * @returns the list of workgroups
     */
    const QList<Smb4KWorkgroup> &workgroupsList() { return m_workgroups_list; }

  signals:
    /**
     * This signal is emitted when a lookup process is about
     * to start
     */
    void aboutToStart();

    /**
     * This signal is emitted when a lookup process finished
     */
    void finished();

    /**
     * Emits the list of discovered workgroups and domains
     */
    void workgroups( const QList<Smb4KWorkgroup> &list );

    /**
     * This signal is emitted when an authentication error
     * occurred.
     *
     * @param job           This job
     */
    void authError( Smb4KQueryMasterJob *job );

  protected:
    bool doKill();

  protected slots:
    void slotStartLookup();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
    
  private:
    void processWorkgroups();
    bool m_started;
    QWidget *m_parent_widget;
    QString m_master_browser;
    Smb4KProcess *m_proc;
    QList<Smb4KWorkgroup> m_workgroups_list;
};


class Smb4KScanBAreasJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    Smb4KScanBAreasJob( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KScanBAreasJob();

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
     * Sets up the scan job
     *
     * @param parent        The parent widget
     */
    void setupScan( QWidget *parent = 0 );

    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

    /**
     * Returns the list of discovered workgroups. You can use this function 
     * after the finished() signal has been emitted to retrieve the complete
     * list of workgroups.
     *
     * @returns the list of workgroups
     */
    const QList<Smb4KWorkgroup> &workgroupsList() { return m_workgroups_list; }

    /**
     * Returns the list of discovered hosts. You can use this function
     * after the finished() signal has been emitted to retrieve the complete
     * list of hosts.
     *
     * @returns the list of hosts
     */
    const QList<Smb4KHost> &hostsList() { return m_hosts_list; }

  signals:
    /**
     * This signal is emitted when a scan process is about
     * to start
     */
    void aboutToStart();

    /**
     * This signal is emitted when a scan process finished
     */
    void finished();

    /**
     * Emits the list of discovered workgroups and domains.
     * 
     * @param list        The list of workgroups
     */
    void workgroups( const QList<Smb4KWorkgroup> &list );

    /**
     * Emits the list of discovered hosts.
     *
     * @param list        The list of hosts
     */
    void hosts( const QList<Smb4KHost> &list );

  protected:
    bool doKill();

  protected slots:
    void slotStartScan();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );

  private:
    void processScan();
    bool m_started;
    QWidget *m_parent_widget;
    Smb4KProcess *m_proc;
    QList<Smb4KWorkgroup> m_workgroups_list;
    QList<Smb4KHost> m_hosts_list;
};


class Smb4KLookupDomainMembersJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    Smb4KLookupDomainMembersJob( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KLookupDomainMembersJob();

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
     * Sets up the lookup job.
     *
     * @param workgroup     The workgroup for that the hosts should be looked up
     *
     * @param parent        The parent widget
     */
    void setupLookup( Smb4KWorkgroup *workgroup,
                      QWidget *parent = 0 );

    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

    /**
     * Returns the workgroup for that the hosts were looked up.
     * Please note that this is a copy of the initial workgroup object.
     * 
     * @returns the workgroup
     */
    Smb4KWorkgroup *workgroup() { return &m_workgroup; }

    /**
     * Returns the list of discovered hosts. You can use this function
     * after the finished() signal has been emitted to retrieve the 
     * list of hosts discovered in the defined workgroup.
     *
     * @returns the list of hosts
     */
    const QList<Smb4KHost> &hostsList() { return m_hosts_list; }

  signals:
    /**
     * This signal is emitted when the lookup process is about to start.
     *
     * @param workgroup     The workgroup for that the hosts should be looked up
     */
    void aboutToStart( Smb4KWorkgroup *workgroup );

    /**
     * This signal is emitted when the lookup process finished.
     *
     * @param workgroup     The workgroup for that the hosts should be looked up
     */
    void finished( Smb4KWorkgroup *workgroup );

    /**
     * Emits the list of @p hosts that belong to a certain @p workgroup.
     *
     * @param workgroup   The workgroup
     *
     * @param list        The list of hosts
     */
    void hosts( Smb4KWorkgroup *workgroup,
                const QList<Smb4KHost> &list );
    
    /**
     * This signal is emitted when an authentication error
     * occurred.
     *
     * @param job           This job
     */
    void authError( Smb4KLookupDomainMembersJob *job );
    
  protected:
    bool doKill();

  protected slots:
    void slotStartLookup();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
  private:
    void processHosts();
    bool m_started;
    QWidget *m_parent_widget;
    Smb4KWorkgroup m_workgroup;
    Smb4KProcess *m_proc;
    QList<Smb4KHost> m_hosts_list;
    Smb4KHost m_master_browser;
};


class Smb4KLookupSharesJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    Smb4KLookupSharesJob( QObject* parent = 0 );
    
    /**
     * Destructor
     */
    ~Smb4KLookupSharesJob();
    
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
     * Sets up the lookup job.
     *
     * @param workgroup     The host that is to be asked for its shared resources
     *
     * @param parent        The parent widget
     */
    void setupLookup( Smb4KHost *host,
                      QWidget *parent = 0 );
    
    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }
    
    /**
     * Returns the host that is/was queried for its shared resources.
     * Please note that this is a copy of the initial host object.
     * 
     * @returns the host
     */
    Smb4KHost *host() { return &m_host; }
    
  signals:
    /**
     * This signal is emitted when the lookup process is about to start.
     *
     * @param host          The host that is queried for its shares
     */
    void aboutToStart( Smb4KHost *host );

    /**
     * This signal is emitted when the lookup process finished.
     *
     * @param host          The host that is queried for its shares
     */
    void finished( Smb4KHost *host );
    
    /**
     * This signal emits the list of shares that was retrieved from
     * the queried host.
     * 
     * @param host          The host
     *
     * @param list          The list of shares
     */
    void shares( Smb4KHost *host,
                 const QList<Smb4KShare> &list );
    
    /**
     * This signal is emitted when an authentication error
     * occurred.
     *
     * @param job           This job
     */
    void authError( Smb4KLookupSharesJob *job );
    
  protected:
    bool doKill();
    
  protected slots:
    void slotStartLookup();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
    
  private:
    void processShares();
    bool m_started;
    Smb4KHost m_host;
    QWidget *m_parent_widget;
    Smb4KProcess *m_proc;
    QList<Smb4KShare> m_shares_list;
};


class Smb4KLookupInfoJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    Smb4KLookupInfoJob( QObject *parent = 0 );
    
    /**
     * Destructor
     */
    ~Smb4KLookupInfoJob();
    
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
     * Sets up the lookup job.
     *
     * @param workgroup     The host that is to be asked for its shared resources
     *
     * @param parent        The parent widget
     */
    void setupLookup( Smb4KHost *host,
                      QWidget *parent = 0 );
    
    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }
    
  signals:
    /**
     * This signal is emitted when the lookup process is about to start.
     *
     * @param host          The host that is queried for the additional
     *                      information
     */
    void aboutToStart( Smb4KHost *host );

    /**
     * This signal is emitted when the lookup process finished.
     *
     * @param host          The host that was queried for the additional
     *                      information
     */
    void finished( Smb4KHost *host );
    
    /**
     * This signal is emitted when and if additional information was
     * found.
     *
     * @param host          The host that was queried for and now carries
     *                      additional information
     */
    void info( Smb4KHost *host );
    
  protected:
    bool doKill();
    
  protected slots:
    void slotStartLookup();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
    
  private:
    void processInfo();
    bool m_started;
    Smb4KHost m_host;
    QWidget *m_parent_widget;
    Smb4KProcess *m_proc;
};


class Smb4KScannerPrivate
{
  public:
    Smb4KScannerPrivate();
    ~Smb4KScannerPrivate();
    Smb4KScanner instance;
};

#endif
