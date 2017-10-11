/***************************************************************************
    Private helper classes for the scanner
                             -------------------
    begin                : So Mai 22 2011
    copyright            : (C) 2011-2017 by Alexander Reinholdt
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

// application specific includes
#include "smb4kscanner.h"
#include "smb4kprocess.h"
#include "smb4kworkgroup.h"
#include "smb4kauthinfo.h"
#include "smb4kglobal.h"

// Qt includes
#include <QWidget>

// KDE includes
#include <KCoreAddons/KJob>

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
    explicit Smb4KLookupDomainsJob(QObject *parent = 0);

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
    void setupLookup(QWidget *parent = 0);

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
    const QList<WorkgroupPtr> &workgroupsList() { return m_workgroups_list; }

  Q_SIGNALS:
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
     * Emitted when workgroups/domains were found
     */
    void workgroups(const QList<WorkgroupPtr> &list);

  protected:
    bool doKill();

  protected Q_SLOTS:
    void slotStartLookup();
    void slotProcess1Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcess2Finished(int exitCode, QProcess::ExitStatus exitStatus);

  private:
    void startProcess1();
    void startProcess2(const QStringList &ipAddresses);
    void processErrors(const QString &stdErr);
    void processMasterBrowsers(const QString &stdOut);
    void processWorkgroups(const QString &stdOut);
    bool m_started;
    QWidget *m_parent_widget;
    Smb4KProcess *m_process1;
    Smb4KProcess *m_process2;
    QList<WorkgroupPtr> m_workgroups_list;
};


class Smb4KQueryMasterJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    explicit Smb4KQueryMasterJob(QObject *parent = 0);

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
    void setupLookup(const QString &master, QWidget *parent = 0);
    
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
    const QList<WorkgroupPtr> &workgroupsList() { return m_workgroups_list; }

  Q_SIGNALS:
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
     * Emitted when workgroups/domains were found
     */
    void workgroups(const QList<WorkgroupPtr> &list);

    /**
     * This signal is emitted when an authentication error
     * occurred.
     *
     * @param job           This job
     */
    void authError(Smb4KQueryMasterJob *job);

  protected:
    bool doKill();

  protected Q_SLOTS:
    void slotStartLookup();
    void slotProcess1Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcess2Finished(int exitCode, QProcess::ExitStatus exitStatus);
    
  private:
    void startProcess1();
    void startProcess2(const QString &ipAddress);
    void processErrors(const QString &stdErr);
    void processMasterBrowser(const QString &stdOut);
    void processWorkgroups(const QString &stdOut);
    bool m_started;
    QWidget *m_parent_widget;
    QString m_master_browser;
    Smb4KProcess *m_process1;
    Smb4KProcess *m_process2;
    QList<WorkgroupPtr> m_workgroups_list;
};


class Smb4KLookupDomainMembersJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    explicit Smb4KLookupDomainMembersJob(QObject *parent = 0);

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
    void setupLookup(const WorkgroupPtr &workgroup, QWidget *parent = 0);

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
    const WorkgroupPtr &workgroup() { return m_workgroup; }

    /**
     * Returns the list of discovered hosts. You can use this function
     * after the finished() signal has been emitted to retrieve the 
     * list of hosts discovered in the defined workgroup.
     *
     * @returns the list of hosts
     */
    const QList<HostPtr> &hostsList() { return m_hosts_list; }

  Q_SIGNALS:
    /**
     * This signal is emitted when the lookup process is about to start.
     *
     * @param workgroup     The workgroup for that the hosts should be looked up
     */
    void aboutToStart(const WorkgroupPtr &workgroup);

    /**
     * This signal is emitted when the lookup process finished.
     *
     * @param workgroup     The workgroup for that the hosts should be looked up
     */
    void finished(const WorkgroupPtr &workgroup);

    /**
     * Emitted when hosts were found
     */
    void hosts(const WorkgroupPtr &workgroup, const QList<HostPtr> &list);
    
    /**
     * This signal is emitted when an authentication error
     * occurred.
     *
     * @param job           This job
     */
    void authError(Smb4KLookupDomainMembersJob *job);
    
  protected:
    bool doKill();

  protected Q_SLOTS:
    void slotStartLookup();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
  private:
    void processErrors(const QString &stdErr);
    void processHosts(const QString &stdOut);
    bool m_started;
    QWidget *m_parent_widget;
    WorkgroupPtr m_workgroup;
    Smb4KProcess *m_process;
    QList<HostPtr> m_hosts_list;
    HostPtr m_master_browser;
};


class Smb4KLookupSharesJob : public KJob
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KLookupSharesJob(QObject* parent = 0);
    
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
    void setupLookup(const HostPtr &host, QWidget *parent = 0);
    
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
    const HostPtr &host() { return m_host; }
    
  Q_SIGNALS:
    /**
     * This signal is emitted when the lookup process is about to start.
     *
     * @param host          The host that is queried for its shares
     */
    void aboutToStart(const HostPtr &host);

    /**
     * This signal is emitted when the lookup process finished.
     *
     * @param host          The host that is queried for its shares
     */
    void finished(const HostPtr &host);
    
    /**
     * Emitted when shares were found
     */
    void shares(const HostPtr &host, const QList<SharePtr> &list);
    
    /**
     * This signal is emitted when an authentication error
     * occurred.
     *
     * @param job           This job
     */
    void authError(Smb4KLookupSharesJob *job);
    
  protected:
    bool doKill();
    
  protected Q_SLOTS:
    void slotStartLookup();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
  private:
    void processErrors(const QString &stdErr);
    void processShares(const QString &stdOut);
    bool m_started;
    HostPtr m_host;
    QWidget *m_parent_widget;
    Smb4KProcess *m_process;
    QList<SharePtr> m_shares_list;
};


class Smb4KLookupIPAddressJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    explicit Smb4KLookupIPAddressJob(QObject* parent = 0);

    /**
     * Destructor
     */
    ~Smb4KLookupIPAddressJob();

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
     * Sets up the lookup for a host
     *
     * @param host        The host for which the IP address should be
     *                    acquired
     *
     * @param parent      The parent widget
     */
    void setupLookup(const HostPtr &host, QWidget *parent = 0);

    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

    /**
     * Returns the host for which the additional information
     * was acquired.
     */
    const HostPtr &host() { return m_host; }

  Q_SIGNALS:
    /**
     * Is emitted when an IP address was found
     */
    void ipAddress(const HostPtr &host);

  protected:
    bool doKill();

  protected Q_SLOTS:
    void slotStartLookup();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  private:
    void useNmblookup(QStringList &command);
    void useNet(QStringList &command);
    void processNmblookupOutput();
    void processNetOutput();
    bool m_started;
    HostPtr m_host;
    QWidget *m_parent_widget;
    Smb4KProcess *m_process;
};


class Smb4KScannerPrivate
{
  public:
    // Elapsed time for periodic scanning
    int elapsedTimePS;
    // Elapsed time for IP address look-up
    int elapsedTimeIP;
    bool haveNewHosts;
    bool scanningAllowed;
    QList<Smb4KGlobal::Process> periodicJobs;
};


class Smb4KScannerStatic
{
  public:
    Smb4KScanner instance;
};

#endif
