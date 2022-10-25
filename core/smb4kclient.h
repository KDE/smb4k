/*
    This class provides the interface to the libsmbclient library.

    SPDX-FileCopyrightText: 2018-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCLIENT_H
#define SMB4KCLIENT_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QScopedPointer>

// KDE includes
#include <KCoreAddons/KCompositeJob>
#include <KIOCore/KFileItem>

// forward declarations
class Smb4KClientPrivate;
class Smb4KBasicNetworkItem;
class Smb4KClientBaseJob;
class Smb4KPreviewDialog;
class Smb4KPrintDialog;

class Q_DECL_EXPORT Smb4KClient : public KCompositeJob
{
    Q_OBJECT

public:
    /**
     * The constructor
     */
    explicit Smb4KClient(QObject *parent = nullptr);

    /**
     * The destructor
     */
    ~Smb4KClient();

    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KClient class.
     */
    static Smb4KClient *self();

    /**
     * This function starts the composite job
     */
    void start() override;

    /**
     * Returns TRUE, if jobs are running and FALSE otherwise
     *
     * @returns TRUE if jobs are running
     */
    bool isRunning();

    /**
     * Aborts all subjobs
     */
    void abort();

    /**
     * This function starts the scan for all available workgroups and domains
     * on the network neighborhood.
     */
    void lookupDomains();

    /**
     * This function looks up all hosts in a certain domain or workgroup.
     *
     * @param workgroup       The workgroup object
     */
    void lookupDomainMembers(const WorkgroupPtr &workgroup);

    /**
     * This function looks up all shared resources a certain @p host provides.
     *
     * @param host            The host object
     */
    void lookupShares(const HostPtr &host);

    /**
     * This function looks up all files and directories present at the location
     * @p item points to. The network item must be of type Smb4KGlobal::Share or
     * Smb4KGlobal::Directory.
     *
     * @param item            The network item object
     */
    void lookupFiles(const NetworkItemPtr &item);

    /**
     * This function starts the printing of a file @p file to the printer share
     * @p printer.
     *
     * @param share           The printer share
     *
     * @param fileItem        The file item
     *
     * @param copies          Number of copies
     */
    void printFile(const SharePtr &share, const KFileItem &fileItem, int copies);

    /**
     * Perform a search on the entire network neighborhood
     *
     * @param item            The search item
     */
    void search(const QString &item);

    /**
     * This function opens the preview dialog for @p share.
     *
     * @param share           The share object
     */
    void openPreviewDialog(const SharePtr &share);

    /**
     * This function opens the print dialog for @p share.
     *
     * @param share           The share object (printer only)
     */
    void openPrintDialog(const SharePtr &share);

Q_SIGNALS:
    /**
     * This signal is emitted when the client starts its work.
     *
     * @param item          The network item
     * @param type          The type of work
     */
    void aboutToStart(const NetworkItemPtr &item, int type);

    /**
     * This signal is emitted when the client finished its work.
     *
     * @param item          The network item
     * @param type          The type of work
     */
    void finished(const NetworkItemPtr &item, int type);

    /**
     * Emitted when the requested list of workgroups was acquired
     */
    void workgroups();

    /**
     * Emitted when the requested list of workgroup members was acquired
     *
     * @param workgroup     The workgroup that was queried
     */
    void hosts(const WorkgroupPtr &workgroup);

    /**
     * Emitted when the requested list of shares was acquired
     *
     * @param host          The host that was queried
     */
    void shares(const HostPtr &host);

    /**
     * Emitted when the requested list of files and directories was acquired
     *
     * @param list          The list of files and directories
     */
    void files(const QList<FilePtr> &list);

    /**
     * Emitted when a search was done
     *
     * @param list          The list of search results
     */
    void searchResults(const QList<SharePtr> &list);

protected Q_SLOTS:
    /**
     * Start the composite job
     */
    void slotStartJobs();

    /**
     * React on changes of the online state
     */
    void slotOnlineStateChanged(bool online);

    /**
     * Called when a job finished. Reimplemented from KCompositeJob.
     */
    void slotResult(KJob *job) override;

    /**
     * Called when the application is about to be closed
     */
    void slotAboutToQuit();

    /**
     * Start a network query
     */
    void slotStartNetworkQuery(NetworkItemPtr item);

    /**
     * Called when a preview dialog closed
     */
    void slotPreviewDialogClosed(Smb4KPreviewDialog *dialog);

    /**
     * Called when a process should be aborted
     */
    void slotAbort();

    /**
     * Called when a file is about to be printed
     */
    void slotStartPrinting(const SharePtr &printer, const KFileItem &fileItem, int copies);

    /**
     * Called when a print dialog closed
     */
    void slotPrintDialogClosed(Smb4KPrintDialog *dialog);

private:
    /**
     * Process errors
     */
    void processErrors(Smb4KClientBaseJob *job);

    /**
     * Process the domains/workgroups retrieved from the network
     *
     * @param job             The client base job
     */
    void processWorkgroups(Smb4KClientBaseJob *job);

    /**
     * Process the domain/workgroup members
     *
     * @param job             The client job
     */
    void processHosts(Smb4KClientBaseJob *job);

    /**
     * Process the shares
     *
     * @param job             The client job
     */
    void processShares(Smb4KClientBaseJob *job);

    /**
     * Process the files and directories
     *
     * @param job             The client job
     */
    void processFiles(Smb4KClientBaseJob *job);

    /**
     * Pointer to the Smb4KClientPrivate class
     */
    QScopedPointer<Smb4KClientPrivate> d;
};

#endif
