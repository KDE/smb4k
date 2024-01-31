/*
    This is the global namespace for Smb4K.

    SPDX-FileCopyrightText: 2005-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KGLOBAL_H
#define SMB4KGLOBAL_H

// application specific includes
#include "smb4kbasicnetworkitem.h"
#include "smb4kbookmark.h"
#include "smb4kcustomsettings.h"
#include "smb4kfile.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

// Qt includes
#include <QEvent>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

// type definitions
typedef QSharedPointer<Smb4KBasicNetworkItem> NetworkItemPtr;
typedef QSharedPointer<Smb4KWorkgroup> WorkgroupPtr;
typedef QSharedPointer<Smb4KHost> HostPtr;
typedef QSharedPointer<Smb4KShare> SharePtr;
typedef QSharedPointer<Smb4KFile> FilePtr;
typedef QSharedPointer<Smb4KBookmark> BookmarkPtr;
typedef QSharedPointer<Smb4KCustomSettings> CustomSettingsPtr;

// Other definitions
#ifndef SMB4K_DEPRECATED
#define SMB4K_DEPRECATED __attribute__((__deprecated__))
#endif

/**
 * This is the global namespace. It provides access to the global lists
 * of workgroups, hosts and shares, to the global settings of the Samba
 * configuration and much more.
 *
 * @author    Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

namespace Smb4KGlobal
{
/**
 * This function returns the global list of workgroups that were discovered by
 * Smb4K. Use this if you want to access and modify the list with your code.
 *
 * @returns the global list of known workgroups.
 */
Q_DECL_EXPORT const QList<WorkgroupPtr> &workgroupsList();

/**
 * This function returns the workgroup or domain that matches the name @p name or
 * NULL if there is no such workgroup.
 *
 * @returns a pointer to the workgroup with name @p name.
 */
Q_DECL_EXPORT WorkgroupPtr findWorkgroup(const QString &name);

/**
 * This function takes a workgroup @p workgroup, checks whether it is already
 * in the list of domains and adds it to the list if necessary. It returns TRUE
 * if the workgroup was added and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param workgroup   The workgroup item
 *
 * @returns TRUE if the workgroup was added and FALSE otherwise.
 */
Q_DECL_EXPORT bool addWorkgroup(WorkgroupPtr workgroup);

/**
 * This function takes a workgroup @p workgroup, and updates the respective workgroup
 * in the global list, if it exists. It returns TRUE on success and FALSE otherwise.
 * If you want to add a workgroup to the global list, use @see addWorkgroup().
 *
 * Please prefer this function over per class solutions.
 *
 * @returns TRUE if the workgroup was updated and FALSE otherwise
 */
Q_DECL_EXPORT bool updateWorkgroup(WorkgroupPtr workgroup);

/**
 * This function removes a workgroup @p workgroup from the list of domains. The
 * pointer that is passed to this function will be deleted. You won't be able
 * to use it afterwards. This function returns TRUE if the workgroup was removed
 * and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param workgroup   The workgroup item
 *
 * @returns TRUE if the workgroup was removed and FALSE otherwise.
 */
Q_DECL_EXPORT bool removeWorkgroup(WorkgroupPtr workgroup);

/**
 * This function clears the global list of workgroups.
 */
Q_DECL_EXPORT void clearWorkgroupsList();

/**
 * This function returns the global list of hosts that were discovered by
 * Smb4K. Use this if you want to access and modify the list with your code.
 *
 * @returns the global list of known hosts.
 */
Q_DECL_EXPORT const QList<HostPtr> &hostsList();

/**
 * This function returns the host matching the name @p name or NULL if there is no
 * such host. The name of the host is mandatory. The workgroup may be empty, but
 * should be given, because this will speed up the search process.
 *
 * @param name          The name of the host
 *
 * @param workgroup     The workgroup where the host is located
 *
 * @returns an Smb4KHost item of NULL if none was found that matches @p name.
 */
Q_DECL_EXPORT HostPtr findHost(const QString &name, const QString &workgroup = QString());

/**
 * This function takes a host @p host, checks whether it is already
 * in the list of hosts and adds it to the list if necessary. It returns TRUE
 * if the host was added and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param host          The host item
 *
 * @returns TRUE if the host was added and FALSE otherwise.
 */
Q_DECL_EXPORT bool addHost(HostPtr host);

/**
 * This function takes an host @p host, and updates the respective host
 * in the global list, if it exists. It returns TRUE on success and FALSE otherwise.
 * If you want to add an host to the global list, use @see addHost().
 *
 * Please prefer this function over per class solutions.
 *
 * @returns TRUE if the host was updated and FALSE otherwise
 */
Q_DECL_EXPORT bool updateHost(HostPtr host);

/**
 * This function removes a host @p host from the list of hosts. The
 * pointer that is passed to this function will be deleted. You won't
 * be able to use it afterwards. This function returns TRUE if the host was removed
 * and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param host          The host item
 *
 * @returns TRUE if the host was removed and FALSE otherwise.
 */
Q_DECL_EXPORT bool removeHost(HostPtr host);

/**
 * This function clears the global list of hosts.
 */
Q_DECL_EXPORT void clearHostsList();

/**
 * This function returns all hosts that belong to the workgroup or domain
 * @p workgroup.
 *
 * Please favor this function over per class solutions.
 *
 * @param workgroup   The workgroup for that the list should be returned.
 *
 * @returns the list of hosts belonging to the workgroup or domain @param workgroup.
 */
Q_DECL_EXPORT QList<HostPtr> workgroupMembers(WorkgroupPtr workgroup);

/**
 * This function returns the list of shares that were discovered by Smb4K.
 * Use this if you want to access and modify the list with your code.
 *
 * @returns the global list of known shares.
 */
Q_DECL_EXPORT const QList<SharePtr> &sharesList();

/**
 * This function returns the share with URL @p url located in the workgroup or
 * domain @p workgroup. If there is no such share, 0 is returned. The workgroup
 * entry may be empty.
 *
 * @param url           The URL of the share
 *
 * @param workgroup     The workgroup
 *
 * @returns the share that matches @p url and optionally @p workgroup or 0.
 */
Q_DECL_EXPORT SharePtr findShare(const QUrl &url, const QString &workgroup = QString());

/**
 * This function takes a share @p share, checks whether it is already
 * in the list of shares and adds it to the list if necessary. It returns TRUE
 * if the share was added and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param share         The share item
 *
 * @returns TRUE if the share was added and FALSE otherwise.
 */
Q_DECL_EXPORT bool addShare(SharePtr share);

/**
 * This function takes a share @p share, and updates the respective share
 * in the global list, if it exists. It returns TRUE on success and FALSE otherwise.
 * If you want to add a share to the global list, use @see addShare().
 *
 * Please prefer this function over per class solutions.
 *
 * @returns TRUE if the share was updated and FALSE otherwise
 */
Q_DECL_EXPORT bool updateShare(SharePtr share);

/**
 * This function removes a share @p share from the list of shares. The
 * pointer that is passed to this function will be deleted. You won't
 * be able to use it afterwards. This function returns TRUE if the share was removed
 * and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param share         The share item
 *
 * @returns TRUE if the share was removed and FALSE otherwise.
 */
Q_DECL_EXPORT bool removeShare(SharePtr share);

/**
 * This function clears the global list of shares.
 */
Q_DECL_EXPORT void clearSharesList();

/**
 * This function returns the list of shares that is provided by one specific host
 * @p host.
 *
 * Please favor this function over per class solutions.
 *
 * @param host          The host where the shares are located
 *
 * @returns the list of shares that are provided by the host @p host.
 */
Q_DECL_EXPORT QList<SharePtr> sharedResources(HostPtr host);

/**
 * This function returns the global list of mounted shares that were discovered by
 * Smb4K. Use this if you want to access and modify the list with your code.
 *
 * @returns the global list of known mounted shares.
 */
Q_DECL_EXPORT const QList<SharePtr> &mountedSharesList();

/**
 * Find a mounted share by its path (i.e. mount point).
 *
 * @returns the share that is mounted to @p path.
 */
Q_DECL_EXPORT SharePtr findShareByPath(const QString &path);

/**
 * Find all mounts of a particular share with URL @p url on the system.
 *
 * This function will compare the incoming URL with the URL of each
 * mounted share to find all shares with the same URL. For the comparison
 * the user info and the port will be stripped.
 *
 * @param url         The URL of the share
 *
 * @returns the complete list of mounts with the URL @p url.
 */
Q_DECL_EXPORT QList<SharePtr> findShareByUrl(const QUrl &url);

/**
 * This function returns the list of inaccessible shares.
 *
 * @returns the list of inaccessible shares.
 */
Q_DECL_EXPORT QList<SharePtr> findInaccessibleShares();

/**
 * This function takes a mounted share @p share, checks whether it is
 * already in the list of mounted shares and adds it to the list if
 * necessary. It returns TRUE if the share was added and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param share       The share item
 *
 * @returns TRUE if the share was added and FALSE otherwise.
 */
Q_DECL_EXPORT bool addMountedShare(SharePtr share);

/**
 * This function takes a mounted share @p share and updates the share that
 * is already present in the internal list.
 *
 * @param share       The share item
 * @returns TRUE if a share was found and updated. Returns FALSE otherwise.
 */
Q_DECL_EXPORT bool updateMountedShare(SharePtr share);

/**
 * This function removes a mounted share @p share from the list of mounted
 * shares. The pointer that is passed to this function will be deleted.
 * You won't be able to use it afterwards. This function returns TRUE if
 * the share was removed and FALSE otherwise.
 *
 * Please prefer this function over per class solutions.
 *
 * @param share       The share item
 *
 * @returns TRUE if the share was removed and FALSE otherwise.
 */
Q_DECL_EXPORT bool removeMountedShare(SharePtr share);

/**
 * This function returns TRUE if only shares are present that are owned by
 * other users and FALSE otherwise.
 *
 * @returns TRUE if there are only shares that are owned by other users.
 */
Q_DECL_EXPORT bool onlyForeignMountedShares();

/**
 * This enumeration determines with which application the mount point
 * of the current mounted share is to be opened.
 */
enum OpenWith { FileManager, Konsole };

/**
 * Open the mount point of a share. Which application is used is determined by
 * the value of @p openWith and - maybe - by settings that were defined by the
 * user.
 *
 * @param share         The share that is to be opened.
 *
 * @param openWith      Integer of type Smb4KCore::OpenWith. Determines with which
 *                      application the share should be opened.
 */
Q_DECL_EXPORT void openShare(SharePtr share, OpenWith openWith = FileManager);

/**
 * Get the NetBIOS name of this computer.
 *
 * @returns the NetBIOS name
 */
Q_DECL_EXPORT const QString machineNetbiosName();

/**
 * Get the name of the workgroup or domain this computer is in.
 *
 * @returns the workgroup or domain
 */
Q_DECL_EXPORT const QString machineWorkgroupName();

/**
 * This function returns TRUE if the core classes should set a busy cursor when
 * they are doing something.
 *
 * @returns TRUE in case a busy cursor should be set.
 */
Q_DECL_EXPORT bool modifyCursor();

#if defined(Q_OS_LINUX)
/**
 * This list contains all allowed arguments for the mount.cifs binary and
 * is only present under the Linux operating system.
 *
 * @returns the list of allowed mount arguments
 */
Q_DECL_EXPORT QStringList allowedMountArguments();
#endif

/**
 * Find the mount executable on the system.
 *
 * @returns the path of the mount executable.
 */
Q_DECL_EXPORT const QString findMountExecutable();

/**
 * Find the umount executable on the system.
 *
 * @returns the path of the umount executable.
 */
Q_DECL_EXPORT const QString findUmountExecutable();

/**
 * This function returns the directory where data is to be placed.
 *
 * @returns the data location
 */
Q_DECL_EXPORT const QString dataLocation();

/**
 * Wait the given @p time until proceeding. This wait function is non-blocking.
 *
 * @param time          The waiting time in msec
 */
Q_DECL_EXPORT void wait(int time);
};

#endif
