/*
    This file provides the enumerations of Smb4KGlobal

    SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KGLOBALENUMS_H
#define SMB4KGLOBALENUMS_H

namespace Smb4KGlobal
{
/**
 * The Process enumeration.
 *
 * @enum LookupDomains          Look up domains
 * @enum LookupDomainMembers    Look up those servers that belong to a domain/workgroup
 * @enum LookupShares           Look up shares on a server
 * @enum LookupFiles            Look up files and directories within a share
 * @enum WakeUp                 Send magic Wake-On-LAN packets
 * @enum PrintFile              Print a file
 * @enum NetworkSearch          Network search
 * @enum MountShare             Mount a share
 * @enum UnmountShare           Unmount a share
 * @enum NoProcess              No process
 */
enum Process { LookupDomains, LookupDomainMembers, LookupShares, LookupFiles, WakeUp, PrintFile, NetworkSearch, MountShare, UnmountShare, NoProcess };

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
enum NetworkItem { Network, Workgroup, Host, Share, Directory, File, UnknownNetworkItem };

/**
 * The enumeration that determines the share type
 *
 * @enum FileShare              a file share
 * @enum PrinterShare           a printer share
 * @enum IpcShare               an IPC share
 */
enum ShareType { FileShare, PrinterShare, IpcShare };
};

#endif
