/***************************************************************************
    This is the global namespace for Smb4K.
                             -------------------
    begin                : Sa Apr 2 2005
    copyright            : (C) 2005-2017 by Alexander Reinholdt
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

#ifndef SMB4KGLOBAL_H
#define SMB4KGLOBAL_H

// Qt includes
#include <QMap>
#include <QString>
#include <QList>
#include <QEvent>
#include <QStringList>
#include <QSharedPointer>

// forward declarations
class Smb4KBasicNetworkItem;
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;
class Smb4KFile;
class Smb4KBookmark;
class Smb4KCustomOptions;

// type definitions
typedef QSharedPointer<Smb4KBasicNetworkItem> NetworkItemPtr;
typedef QSharedPointer<Smb4KWorkgroup> WorkgroupPtr;
typedef QSharedPointer<Smb4KHost> HostPtr;
typedef QSharedPointer<Smb4KShare> SharePtr;
typedef QSharedPointer<Smb4KFile> FilePtr;
typedef QSharedPointer<Smb4KBookmark> BookmarkPtr;
typedef QSharedPointer<Smb4KCustomOptions> OptionsPtr;

// Other definitions
#ifndef SMB4K_DEPRECATED
#define SMB4K_DEPRECATED __attribute__ ((__deprecated__))
#endif

#if !defined(Q_OS_LINUX) && !defined(Q_OS_FREEBSD) && !defined(Q_OS_NETBSD)
#warning Defining SMB4K_UNSUPPORTED_PLATFORM
#define SMB4K_UNSUPPORTED_PLATFORM
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
   * The Process enumeration.
   * 
   * @enum LookupDomains          Look up domains
   * @enum LookupDomainMembers    Look up those servers that belong to a domain/workgroup
   * @enum LookupShares           Look up shares on a server
   * @enum LookupFiles            Look up files and directories within a share
   * @enum WakeUp                 Send magic Wake-On-LAN packages
   * @enum PrintFile              Print a file
   * @enum NetworkSearch          Network search
   * @enum MountShare             Mount a share
   * @enum UnmountShare           Unmount a share
   * @enum NoProcess              No process
   */
  enum Process { 
    LookupDomains,
    LookupDomainMembers,
    LookupShares,
    LookupFiles,
    WakeUp,
    PrintFile,
    NetworkSearch,
    MountShare,
    UnmountShare,
    NoProcess };
                 
  /**
   * The enumeration to determine the type of a network item.
   * 
   * @enum Network                The network
   * @enum Workgroup              A workgroup
   * @enum Host                   A host
   * @enum Share                  A share
   * @enum Directory              A directory in a shared folder
   * @enum File                   A file in a shared folder
   * @enum UnknownNetworkItem     An unknown network item 
   */
  enum NetworkItem { 
    Network,
    Workgroup,
    Host,
    Share,
    Directory,
    File,
    UnknownNetworkItem };
    
  /**
   * The enumeration that determines the share type
   *    
   * @enum FileShare              a file share
   * @enum PrinterShare           a printer share
   * @enum IpcShare               an IPC share
   */
  enum ShareType {
    FileShare,
    PrinterShare,
    IpcShare,
  };
    
  /**
   * Use this function to initialize the core classes. Besides starting several
   * core classes such as the scanner (for an initial browse list) and the mounter
   * (for the import of all externally mounted shares), it also sets some default
   * values for some of the settings used to browse the network.
   * 
   * By setting the @p modifyCursor parameter to TRUE, you force the core classes
   * to set a busy cursor when they do something. Default is FALSE.
   * 
   * Setting @p initClasses to FALSE will avoid starting the core classes. This 
   * should only the used if you are starting the core classes in a different 
   * way (e. g. if you are starting them in the plasmoid via the Smb4KDeclarative
   * class).
   *
   * You should execute this function before starting your main application.
   */
  Q_DECL_EXPORT void initCore(bool modifyCursor = false, bool initClasses = true);

  /**
   * Aborts all actions that are run by the core classes and that can be aborted.
   */
  Q_DECL_EXPORT void abortCore();

  /**
   * Check if at least one of the core classes that use KJobs (scanner, mounter, etc.) 
   * is running.
   *
   * @returns TRUE if at least one of the core classes is doing something.
   */
  Q_DECL_EXPORT bool coreIsRunning();

  /**
   * Set the necessary default values.
   *
   * You only need to run this function if you do not use the initCore() function.
   * Check if the core has been initialized by the coreIsInitialized() function.
   */
  Q_DECL_EXPORT void setDefaultSettings();

  /**
   * Check if the core has been initialized through the initCore() function.
   *
   * @returns TRUE if the core has already been initialized.
   */
  Q_DECL_EXPORT bool coreIsInitialized();
  
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
   * This function returns the share with UNC @p unc located in the workgroup or
   * domain @p workgroup. If there is no such share, 0 is returned. The workgroup
   * entry may be empty.
   * @param unc           The UNC of the share
   * @param wokgroup      The workgroup
   * @returns the share that matches @p unc and optionally @p workgroup or 0.
   */
  Q_DECL_EXPORT SharePtr findShare(const QString &unc, const QString &workgroup = QString());

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
   * Find all mounts of a particular share with UNC @p unc on the system.
   *
   * This function will compare the incoming string with the UNC of each
   * mounted share to find all shares with the same UNC. For the comparison
   * the scheme/protocol, the user info, and the port will be stripped. You
   * can pass the UNC in any valid format. The function will internally
   * convert the string into a QUrl and work with that.
   *
   * @param unc         The UNC of the share
   *
   * @returns the complete list of mounts with the UNC @p unc.
   */
  Q_DECL_EXPORT QList<SharePtr> findShareByUNC(const QString &unc);

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
   * This functions adds a share to the search results.
   * 
   * @param share       The share item
   */
  Q_DECL_EXPORT bool addSearchResult(SharePtr share);
  
  /**
   * This function clears the list of search results.
   */
  Q_DECL_EXPORT void clearSearchResults();
  
  /**
   * This function returns the list of all search results
   * 
   * @returns the list of search results
   */
  Q_DECL_EXPORT QList<SharePtr> searchResults();
  
  /**
   * This enumeration determines with which application the mount point
   * of the current mounted share is to be opened.
   */
  enum OpenWith { FileManager,
                  Konsole };
  
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
   * Get the entries of the [global] section of the smb.conf file. By setting @p read 
   * to TRUE you can force the smb.conf file to be reread.
   * 
   * @returns the entries of the [global] section of the smb.conf file
   */
  Q_DECL_EXPORT const QMap<QString,QString> &globalSambaOptions(bool read = false);
  
  /**
   * Get the WINS server's name or IP address. Returns an empty string if there is no
   * WINS server is defined.
   * 
   * @returns the WINS server
   */
  Q_DECL_EXPORT const QString winsServer();
  
  /**
   * This function returns TRUE if the core classes should set a busy cursor when 
   * they are doing something.
   * 
   * @returns TRUE in case a busy cursor should be set.
   */
  Q_DECL_EXPORT bool modifyCursor();
  
#if defined(Q_OS_LINUX)
  /**
   * This list contains all whitelisted arguments for the mount.cifs binary and
   * is only present under the Linux operating system.
   */
  Q_DECL_EXPORT QStringList whitelistedMountArguments();
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
   * @returns the data location
   */
  Q_DECL_EXPORT const QString dataLocation();
};

#endif
