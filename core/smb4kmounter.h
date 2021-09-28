/*
    The core class that mounts the shares.
    -------------------
    begin                : Die Jun 10 2003
    SPDX-FileCopyrightText: 2003-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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

#ifndef SMB4KMOUNTER_H
#define SMB4KMOUNTER_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QFile>
#include <QMap>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QVariant>

// KDE includes
#include <KCoreAddons/KCompositeJob>

// forward declarations
class Smb4KShare;
class Smb4KAuthInfo;
class Smb4KMountDialog;
class Smb4KMountJob;
class Smb4KUnmountJob;
class Smb4KMounterPrivate;

/**
 * This is one of the core classes of Smb4K. It manages the mounting
 * and unmounting of remote Samba/Windows shares. Additionally it maintains a
 * list of all mounts with SMBFS and CIFS file system that are present
 * on the system.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KMounter : public KCompositeJob
{
    Q_OBJECT

    friend class Smb4KMounterPrivate;

public:
    /**
     * The constructor.
     */
    explicit Smb4KMounter(QObject *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KMounter();

    /**
     * Returns a static pointer to this class.
     */
    static Smb4KMounter *self();

    /**
     * Aborts all running processes.
     */
    void abort();

    /**
     * This function attempts to mount a share.
     *
     * @param share       The Smb4KShare object that is representing the share.
     */
    void mountShare(const SharePtr &share);

    /**
     * Mounts a list of shares at once.
     *
     * @param shares      The list of shares
     */
    void mountShares(const QList<SharePtr> &shares);

    /**
     * This function attempts to unmount a share. With the parameter @p silent you
     * can suppress any error messages.
     *
     * @param share       The share object that should be unmounted.
     *
     * @param silent      Determines whether this function should emit an error code in
     *                    case of an error. The default value is FALSE.
     */
    void unmountShare(const SharePtr &share, bool silent = false);

    /**
     * This function attempts to unmount a list of shares. With the parameter @p silent
     * you can suppress any error messages.
     *
     * @param shares      The list of shares that is to be unmounted
     *
     * @param silent      Determines whether this function should emit an error code in
     *                    case of an error. The default value is FALSE.
     */
    void unmountShares(const QList<SharePtr> &shares, bool silent = false);

    /**
     * Unmounts all shares at once. This is a convenience function. It calls
     * unmountShares() to unmount all currently mounted shares.
     *
     * @param silent      Determines whether this function should emit an error code in
     *                    case of an error. The default value is FALSE.
     */
    void unmountAllShares(bool silent);

    /**
     * Mount a share via a mount dialog. The mount dialog is opened and you have
     * to enter the UNC and optionally the workgroup and IP address.
     */
    void openMountDialog();

    /**
     * This function reports if the mounter is running or not.
     *
     * @returns TRUE if the mounter is running and FALSE otherwise.
     */
    bool isRunning();

    /**
     * This function starts the composite job
     */
    void start() override;

Q_SIGNALS:
    /**
     * This signal is emitted whenever a share item was updated.
     *
     * @param share             The share item that was just updated.
     */
    void updated(const SharePtr &share);

    /**
     * This signal is emitted when a share has successfully been mounted.
     *
     * @param share             The share that was just mounted.
     */
    void mounted(const SharePtr &share);

    /**
     * This signal is emitted after a share has successfully been unmounted.
     *
     * @param share            The share that was unmounted.
     */
    void unmounted(const SharePtr &share);

    /**
     * This signal is emitted when a process is about to start.
     * @param process           The kind of process
     */
    void aboutToStart(int process);

    /**
     * This signal is emitted when a process finished.
     * @param process           The kind of process
     */
    void finished(int process);

    /**
     * This signal is emitted every time a share was added to or removed
     * from the list of shares. In contrast to the mounted() and unmounted()
     * signals, this signal is emitted at the end of the modification of
     * the list (the unmount() signal is emitted before the share is actually
     * removed from the list).
     *
     * If you need to know if the contents of a specific share has been changed,
     * you need to connect to the updated() signal.
     */
    void mountedSharesListChanged();

protected:
    /**
     * Reimplemented from QObject to process the queue.
     *
     * @param event             The QTimerEvent event
     */
    void timerEvent(QTimerEvent *event) override;

protected Q_SLOTS:
    /**
     * Starts the composite job
     */
    void slotStartJobs();

    /**
     * This slot is called by the QApplication::aboutToQuit() signal.
     * Is does everything that has to be done before the program
     * really exits.
     */
    void slotAboutToQuit();

    /**
     * This slot is called when the online status changed. It is used
     * to initialize network actions when the network becomes available.
     * @param online          TRUE if online otherwise FALSE
     */
    void slotOnlineStateChanged(bool online);

    /**
     * Called when a mounted share has been stat'ed.
     *
     * @param job             The KIO::StatJob
     */
    void slotStatResult(KJob *job);

    /**
     * This slot is invoked when the active profile is about to be changed
     */
    void slotAboutToChangeProfile();

    /**
     * This slot is called when the active profile changed.
     */
    void slotActiveProfileChanged(const QString &newProfile);

    /**
     * This slot is called when a profile was migrated.
     *
     * @param from            The old profile
     * @param to              The new profile
     */
    void slotProfileMigrated(const QString &from, const QString &to);

    /**
     * This slot is called whenever a network share is mounted or
     * unmounted.
     */
    void slotTriggerImport();

    /**
     * This slot is called whenever the configuration changed. It is used
     * to trigger the importing of shares when certain settings changed.
     */
    void slotConfigChanged();

private:
    /**
     * Trigger the remounting of shares. If the parameter @p fill_list is
     * set to true, the internal list should be populated with the shares
     * that are scheduled for a remount.
     *
     * @param fill_list       Fill the internal list with shares that are
     *                        to be remounted.
     */
    void triggerRemounts(bool fill_list);

    /**
     * Imports mounted shares.
     */
    void import(bool checkInaccessible);

    /**
     * Save all shares that need to be remounted.
     */
    void saveSharesForRemount();

    /**
     * Fill the mount action arguments into a map.
     */
    bool fillMountActionArgs(const SharePtr &share, QVariantMap &mountArgs);

    /**
     * Fill the unmount action arguments into a map.
     */
    bool fillUnmountActionArgs(const SharePtr &share, bool force, bool silent, QVariantMap &unmountArgs);

    /**
     * Check the size, accessibility, ids, etc. of the share(s)
     */
    void check(const SharePtr &share);

    /**
     * Pointer to the Smb4KMounterPrivate class.
     */
    const QScopedPointer<Smb4KMounterPrivate> d;
};

#endif
