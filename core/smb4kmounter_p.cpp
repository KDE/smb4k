/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2015 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kmounter_p.h"
#include "smb4ksettings.h"
#include "smb4knotification.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_freebsd.h"
#elif defined(Q_OS_SOLARIS)
#include "smb4kmountsettings_solaris.h"
#endif

// Qt includes
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

// KDE includes
#include <kdiskfreespaceinfo.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmountpoint.h>
#include <kshell.h>
#include <KUser>
       

using namespace Smb4KGlobal;


Smb4KMountJob::Smb4KMountJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_processed( 0 )
{
  setCapabilities( KJob::Killable );
}


Smb4KMountJob::~Smb4KMountJob()
{
  while ( !m_shares.isEmpty() )
  {
    delete m_shares.takeFirst();
  }

  while ( !m_auth_errors.isEmpty() )
  {
    delete m_auth_errors.takeFirst();
  }

  while ( !m_retries.isEmpty() )
  {
    delete m_retries.takeFirst();
  }
}


void Smb4KMountJob::start()
{
  m_started = true;
  QTimer::singleShot( 50, this, SLOT(slotStartMount()) );
}


void Smb4KMountJob::setupMount( Smb4KShare *share, QWidget *parent )
{
  Q_ASSERT( share );
  m_shares << new Smb4KShare( *share );
  m_parent_widget = parent;
}


void Smb4KMountJob::setupMount( const QList<Smb4KShare*> &shares, QWidget *parent )
{
  QListIterator<Smb4KShare *> it( shares );

  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();
    Q_ASSERT( share );
    m_shares << new Smb4KShare( *share );
  }

  m_parent_widget = parent;
}


bool Smb4KMountJob::createMountAction(Smb4KShare *share, Action *action)
{
  Q_ASSERT(share);
  Q_ASSERT(action);
  
  if (!share || !action)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Map that holds the mount command, etc.
  QMap<QString, QVariant> mount_args;

  // Fill the map and create the mountpoint upon success.
  if (!fillArgs(share, mount_args))
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  action->setName("net.sourceforge.smb4k.mounthelper.mount");
  action->setHelperID("net.sourceforge.smb4k.mounthelper");
  
  // Add the mount arguments (i. e. mount command, options,
  // mountpoint, unc, ...)
  QMapIterator<QString, QVariant> it(mount_args);
  
  while (it.hasNext())
  {
    it.next();
    action->addArgument(it.key(), it.value());
  }
  
  if (!share->isHomesShare())
  {
    action->addArgument("mh_url", static_cast<QUrl>(share->url()));
  }
  else
  {
    action->addArgument("mh_url", static_cast<QUrl>(share->homeURL()));
  }  

  action->addArgument("mh_workgroup", share->workgroupName());
  action->addArgument("mh_comment", share->comment());
  action->addArgument("mh_ip", share->hostIP());

  // The path to the Kerberos ticket is stored - if it exists - in the
  // KRB5CCNAME environment variable. By default, the ticket is located
  // at /tmp/krb5cc_[uid]. So, if the environment variable does not exist,
  // but the cache file is there, try to use it.
  if (QProcessEnvironment::systemEnvironment().contains("KRB5CCNAME"))
  {
    action->addArgument("mh_krb5ticket", QProcessEnvironment::systemEnvironment().value("KRB5CCNAME", ""));
  }
  else
  {
    QString ticket = QString("/tmp/krb5cc_%1").arg(KUser(KUser::UseRealUserID).uid());
    
    if (QFile::exists(ticket))
    {
      action->addArgument("mh_krb5ticket", "FILE:"+ticket);
    }
    else
    {
      // Do nothing
    }
  }
  
  return true;
}


#if defined(Q_OS_LINUX)
//
// Linux arguments
//
bool Smb4KMountJob::fillArgs(Smb4KShare *share, QMap<QString, QVariant>& map)
{
  // Find the mount program.
  QString mount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  for (int i = 0; i < paths.size(); ++i)
  {
    mount = KGlobal::dirs()->findExe("mount.cifs", paths.at(i));

    if (!mount.isEmpty())
    {
      map.insert("mh_command", mount);
      break;
    }
    else
    {
      continue;
    }
  }

  if (mount.isEmpty())
  {
    Smb4KNotification::commandNotFound("mount.cifs");
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Mount arguments.
  QMap<QString, QString> global_options = globalSambaOptions();
  Smb4KCustomOptions *options  = Smb4KCustomOptionsManager::self()->findOptions(share);

  // Set some settings for the share.
  share->setFileSystem(Smb4KShare::CIFS);
  
  if (options)
  {
    share->setPort(options->fileSystemPort() != Smb4KMountSettings::remoteFileSystemPort() ?
                   options->fileSystemPort() : Smb4KMountSettings::remoteFileSystemPort());
  }
  else
  {
    share->setPort(Smb4KMountSettings::remoteFileSystemPort());
  }
  
  // Compile the list of arguments, that is added to the
  // mount command via "-o ...".
  QStringList args_list;
  
  // Workgroup or domain
  if (!share->workgroupName().trimmed().isEmpty())
  {
    args_list << QString("domain=%1").arg(KShell::quoteArg(share->workgroupName()));
  }
  else
  {
    // Do nothing
  }
  
  // Host IP
  if (!share->hostIP().trimmed().isEmpty())
  {
    args_list << QString("ip=%1").arg(share->hostIP());
  }
  else
  {
    // Do nothing
  }
  
  // User name
  if (!share->login().isEmpty())
  {
    args_list << QString("username=%1").arg(share->login());
  }
  else
  {
    args_list << "guest";
  }
  
  // Client's and server's NetBIOS name
  // According to the manual page, this is only needed when port 139
  // is used. So, we only pass the NetBIOS name in that case.
  if (Smb4KMountSettings::remoteFileSystemPort() == 139 || (options && options->fileSystemPort() == 139))
  {
    // The client's NetBIOS name.
    if (!Smb4KSettings::netBIOSName().isEmpty())
    {
      args_list << QString("netbiosname=%1").arg(KShell::quoteArg(Smb4KSettings::netBIOSName()));
    }
    else
    {
      if (!global_options["netbios name"].isEmpty())
      {
        args_list << QString("netbiosname=%1").arg(KShell::quoteArg(global_options["netbios name"]));
      }
      else
      {
        // Do nothing
      }
    }

    // The server's NetBIOS name.
    // ('servern' is a synonym for 'servernetbiosname')
    args_list << QString("servern=%1").arg(KShell::quoteArg( share->hostName()));
  }
  else
  {
    // Do nothing
  }
  
  // UID
  args_list << QString("uid=%1").arg(options ? options->uid() : (K_UID)Smb4KMountSettings::userID().toInt());
  
  // Force user
  if (Smb4KMountSettings::forceUID())
  {
    args_list << "forceuid";
  }
  else
  {
    // Do nothing
  }
  
  // GID
  args_list << QString("gid=%1").arg(options ? options->gid() : (K_GID)Smb4KMountSettings::groupID().toInt());
  
  // Force GID
  if (Smb4KMountSettings::forceGID())
  {
    args_list << "forcegid";
  }
  else
  {
    // Do nothing
  }
  
  // Client character set
  switch (Smb4KMountSettings::clientCharset())
  {
    case Smb4KMountSettings::EnumClientCharset::default_charset:
    {
      if (!global_options["unix charset"].isEmpty())
      {
        args_list << QString("iocharset=%1").arg(global_options["unix charset"].toLower());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      args_list << QString("iocharset=%1")
                   .arg(Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label);
      break;
    }
  }
  
  // Port. 
  args_list << QString("port=%1").arg(share->port());
  
  // Write access
  if (options)
  {
    switch (options->writeAccess())
    {
      case Smb4KCustomOptions::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KCustomOptions::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        switch (Smb4KMountSettings::writeAccess())
        {
          case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
          {
            args_list << "rw";
            break;
          }
          case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
          {
            args_list << "ro";
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
    }
  }
  else
  {
    switch (Smb4KMountSettings::writeAccess())
    {
      case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  // File mask
  if (!Smb4KMountSettings::fileMask().isEmpty())
  {
    args_list << QString("file_mode=%1").arg(Smb4KMountSettings::fileMask());
  }
  else
  {
    // Do nothing
  }

  // Directory mask
  if (!Smb4KMountSettings::directoryMask().isEmpty())
  {
    args_list << QString("dir_mode=%1").arg(Smb4KMountSettings::directoryMask());
  }
  else
  {
    // Do nothing
  }
  
  // Permission checks
  if (Smb4KMountSettings::permissionChecks())
  {
    args_list << "perm";
  }
  else
  {
    args_list << "noperm";
  }
  
  // Client controls IDs
  if (Smb4KMountSettings::clientControlsIDs())
  {
    args_list << "setuids";
  }
  else
  {
    args_list << "nosetuids";
  }
  
  // Server inode numbers
  if (Smb4KMountSettings::serverInodeNumbers())
  {
    args_list << "serverino";
  }
  else
  {
    args_list << "noserverino";
  }
  
  // Cache mode
  switch (Smb4KMountSettings::cacheMode())
  {
    case Smb4KMountSettings::EnumCacheMode::None:
    {
      args_list << "cache=none";
      break;
    }
    case Smb4KMountSettings::EnumCacheMode::Strict:
    {
      args_list << "cache=strict";
      break;
    }
    case Smb4KMountSettings::EnumCacheMode::Loose:
    {
      args_list << "cache=loose";
      break;
    }
    default:
    {
      break;
    }
  }
  
  // Translate reserved characters
  if (Smb4KMountSettings::translateReservedChars())
  {
    args_list << "mapchars";
  }
  else
  {
    args_list << "nomapchars";
  }
  
  // Locking
  if (Smb4KMountSettings::noLocking())
  {
    args_list << "nolock";
  }
  else
  {
    // Do nothing
  }
  
  // Security mode
  if (options)
  {
    switch (options->securityMode())
    {
      case Smb4KCustomOptions::NoSecurityMode:
      {
        args_list << "sec=none";
        break;
      }
      case Smb4KCustomOptions::Krb5:
      {
        args_list << "sec=krb5";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).uid());
        break;
      }
      case Smb4KCustomOptions::Krb5i:
      {
        args_list << "sec=krb5i";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).uid());
        break;
      }
      case Smb4KCustomOptions::Ntlm:
      {
        args_list << "sec=ntlm";
        break;
      }
      case Smb4KCustomOptions::Ntlmi:
      {
        args_list << "sec=ntlmi";
        break;
      }
      case Smb4KCustomOptions::Ntlmv2:
      {
        args_list << "sec=ntlmv2";
        break;
      }
      case Smb4KCustomOptions::Ntlmv2i:
      {
        args_list << "sec=ntlmv2i";
        break;
      }
      case Smb4KCustomOptions::Ntlmssp:
      {
        args_list << "sec=ntlmssp";
        break;
      }
      case Smb4KCustomOptions::Ntlmsspi:
      {
        args_list << "sec=ntlmsspi";
        break;
      }
      default:
      {
        // Smb4KCustomOptions::DefaultSecurityMode
        break;
      }
    }
  }
  else
  {
    switch (Smb4KMountSettings::securityMode())
    {
      case Smb4KMountSettings::EnumSecurityMode::None:
      {
        args_list << "sec=none";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5:
      {
        args_list << "sec=krb5";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).uid());
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5i:
      {
        args_list << "sec=krb5i";
        args_list << QString("cruid=%1").arg(KUser(KUser::UseRealUserID).uid());
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlm:
      {
        args_list << "sec=ntlm";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
      {
        args_list << "sec=ntlmi";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
      {
        args_list << "sec=ntlmv2";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
      {
        args_list << "sec=ntlmv2i";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
      {
        args_list << "sec=ntlmssp";
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
      {
        args_list << "sec=ntlmsspi";
        break;
      }
      default:
      {
        // Smb4KSettings::EnumSecurityMode::Default,
        break;
      }
    }
  }
  
  // SMB protocol version
  switch (Smb4KMountSettings::smbProtocolVersion())
  {
    case Smb4KMountSettings::EnumSmbProtocolVersion::OnePointZero:
    {
      args_list << "vers=1.0";
      break;
    }
    case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointZero:
    {
      args_list << "vers=2.0";
      break;
    }
    case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointOne:
    {
      args_list << "vers=2.1";
      break;
    }
    case Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointZero:
    {
      args_list << "vers=3.0";
      break;
    }
    default:
    {
      break;
    }
  }
    
  // Global custom options provided by the user
  if (!Smb4KMountSettings::customCIFSOptions().isEmpty())
  {
    // SECURITY: Only pass those arguments to mount.cifs that do not pose
    // a potential security risk and that have not already been defined.
    //
    // This is, among others, the proper fix to the security issue reported
    // by Heiner Markert (aka CVE-2014-2581).
    QStringList whitelist = whitelistedMountArguments();
    QStringList list = Smb4KMountSettings::customCIFSOptions().split(',', QString::SkipEmptyParts);
    QMutableStringListIterator it(list);
    
    while (it.hasNext())
    {
      QString arg = it.next().section("=", 0, 0);
      
      if (!whitelist.contains(arg))
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
      
      args_list += list;
    }
  }
  else
  {
    // Do nothing
  }
  
  // Mount options
  QStringList mh_options;
  mh_options << "-o";
  mh_options << args_list.join(",");
  map.insert("mh_options", mh_options);
  
  // Mount point
  map.insert("mh_mountpoint", share->canonicalPath());
  
  // UNC
  map.insert("mh_unc", !share->isHomesShare() ? share->unc() : share->homeUNC());;
  
  return true;
}
#elif  defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD arguments
//
bool Smb4KMountJob::fillArgs(Smb4KShare *share, QMap<QString, QVariant>& map)
{
  // Find the mount program.
  QString mount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  for (int i = 0; i < paths.size(); ++i)
  {
    mount = KGlobal::dirs()->findExe("mount_smbfs", paths.at(i));

    if (!mount.isEmpty())
    {
      map.insert("mh_command", mount);
      break;
    }
    else
    {
      continue;
    }
  }

  if (mount.isEmpty())
  {
    Smb4KNotification::commandNotFound("mount_smbfs");
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Mount arguments.
  QMap<QString, QString> global_options = globalSambaOptions();
  Smb4KCustomOptions *options  = Smb4KCustomOptionsManager::self()->findOptions(share);

  // Set some settings for the share.
  share->setFileSystem(Smb4KShare::SMBFS);
  
  // Compile the list of arguments.
  QStringList args_list;
  
  // Workgroup
  if (!share->workgroupName().isEmpty())
  {
    args_list << "-W";
    args_list << KShell::quoteArg(share->workgroupName());
  }
  else
  {
    // Do nothing
  }
  
  // Host IP
  if (!share->hostIP().isEmpty())
  {
    args_list << "-I";
    args_list << share->hostIP();
  }
  else
  {
    // Do nothing
  }
  
  // UID
  if (options)
  {
    args_list << "-u";
    args_list << QString("%1").arg(options->uid());
  }
  else
  {
    args_list << "-u";
    args_list << QString("%1").arg((K_UID)Smb4KMountSettings::userID().toInt());
  }
  
  // GID
  if (options)
  {
    args_list << "-g";
    args_list << QString("%1").arg(options->gid());
  }
  else
  {
    args_list << "-g";
    args_list << QString("%1").arg((K_GID)Smb4KMountSettings::groupID().toInt());
  }
  
  // Character sets for the client and server
  QString client_charset, server_charset;

  switch (Smb4KMountSettings::clientCharset())
  {
    case Smb4KMountSettings::EnumClientCharset::default_charset:
    {
      client_charset = global_options["unix charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      client_charset = Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label;
      break;
    }
  }

  switch (Smb4KMountSettings::serverCodepage())
  {
    case Smb4KMountSettings::EnumServerCodepage::default_codepage:
    {
      server_charset = global_options["dos charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      server_charset = Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::serverCodepage()).label;
      break;
    }
  }

  if (!client_charset.isEmpty() && !server_charset.isEmpty())
  {
    args_list << "-E";
    args_list << QString("%1:%2").arg(client_charset, server_charset);
  }
  else
  {
    // Do nothing
  }
  
  // File mask
  if (!Smb4KMountSettings::fileMask().isEmpty())
  {
    args_list << "-f";
    args_list << QString("%1").arg(Smb4KMountSettings::fileMask());
  }
  else
  {
    // Do nothing
  }

  // Directory mask
  if (!Smb4KMountSettings::directoryMask().isEmpty())
  {
    args_list << "-d";
    args_list << QString("%1").arg(Smb4KMountSettings::directoryMask());
  }
  else
  {
    // Do nothing
  }
  
  // User name
  if (!share->login().isEmpty())
  {
    args_list << "-U";
    args_list << QString("%1").arg(share->login());
  }
  else
  {
    args_list << "-N";
  }
  
  // Mount options
  map.insert("mh_options", args_list);
  
  // Mount point
  map.insert("mh_mountpoint", share->canonicalPath());
  
  // UNC
  map.insert("mh_unc", !share->isHomesShare() ? share->unc() : share->homeUNC());;
  
  return true;
}
#elif defined(Q_OS_SOLARIS)
//
// Solaris/illumos
//
bool Smb4KMountJob::fillArgs(Smb4KShare *share, QMap<QString, QVariant> &map)
{
  qDebug() << "FIXME: Implement Smb4KMountJob::fillArgs() under Solaris.";
  
 // Find the mount program.
  QString mount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  for (int i = 0; i < paths.size(); ++i)
  {
    mount = KGlobal::dirs()->findExe("mount", paths.at(i));

    if (!mount.isEmpty())
    {
      map.insert("mh_command", mount);
      break;
    }
    else
    {
      continue;
    }
  }

  if (mount.isEmpty())
  {
    Smb4KNotification::commandNotFound("mount");
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Mount arguments.
  Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions(share);
  
  // Set some settings for the share.
  share->setFileSystem(Smb4KShare::SMBFS);
  
  // Compile the list of arguments.
  QStringList arguments, args_list;
  
  // File system
  arguments << "-F";
  arguments << "smbfs";
  
  // Arguments to be used with -o:
  
  // File permissions
  if (!Smb4KMountSettings::fileMask().isEmpty())
  {
    args_list << QString("fileperms=%1").arg(Smb4KMountSettings::fileMask());
  }
  else
  {
    // Do nothing
  }

  // Directory permissions
  if (!Smb4KMountSettings::directoryMask().isEmpty())
  {
    args_list << QString("dirperms=%1").arg(Smb4KMountSettings::directoryMask());
  }
  else
  {
    // Do nothing
  }
  
  // UID
  args_list << QString("uid=%1").arg(options ? options->uid() : (K_UID)Smb4KMountSettings::userID().toInt());
  
  // GID
  args_list << QString("gid=%1").arg(options ? options->gid() : (K_GID)Smb4KMountSettings::groupID().toInt());
  
  // Set the 'noprompt' option if there is no auth info.
  if (share->login().isEmpty())
  {
    args_list << "noprompt";
  }
  else
  {
    // Do nothing
  }
  
  // Allow execution of programs.
  if (Smb4KMountSettings::programExecution())
  {
    args_list << "exec";
  }
  else
  {
    args_list << "noexec";
  }
  
  // Allow non-blocking mandatory locking semantics
  if (Smb4KMountSettings::nonBlockingMandatoryLocking())
  {
    args_list << "nbmand";
  }
  else
  {
    args_list << "nonbmand";
  }
  
  // Read-write or read-only access
 if (options)
  {
    switch (options->writeAccess())
    {
      case Smb4KCustomOptions::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KCustomOptions::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        switch (Smb4KMountSettings::writeAccess())
        {
          case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
          {
            args_list << "rw";
            break;
          }
          case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
          {
            args_list << "ro";
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
    }
  }
  else
  {
    switch (Smb4KMountSettings::writeAccess())
    {
      case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
      {
        args_list << "rw";
        break;
      }
      case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
      {
        args_list << "ro";
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  // Setuid or setgid execution
  if (Smb4KMountSettings::suidExecution())
  {
    args_list << "suid";
  }
  else
  {
    args_list << "nosuid";
  }
  
  arguments << "-o";
  arguments << args_list.join(",");
  
  // Mount options
  map.insert("mh_options", arguments);
  
  // Mount point
  map.insert("mh_mountpoint", share->canonicalPath());
  
  // UNC
  map.insert("mh_unc", !share->isHomesShare() ? share->fullUNC() : share->fullHomeUNC());;
  
  qDebug() << map;
  
  return true;
}
#else
//
// Dummy 
//
bool Smb4KMountJob::fillArgs(Smb4KShare *, QMap<QString, QVariant>&)
{
  qWarning() << "Smb4KMountJob::fillArgs() is not implemented!";
  qWarning() << "Mounting under this operating system is not supported...";
  return false;
}
#endif


bool Smb4KMountJob::doKill()
{
  Action( "net.sourceforge.smb4k.mounthelper.mount" ).stop();
  return KJob::doKill();
}


void Smb4KMountJob::slotStartMount()
{
  QList<Action> actions;
  QMutableListIterator<Smb4KShare *> it(m_shares);

  while (it.hasNext())
  {
    Smb4KShare *share = it.next();
    Action mountAction;
    
    // Create the mountpoint.
    QString mountpoint;
    mountpoint += Smb4KSettings::mountPrefix().path();
    mountpoint += QDir::separator();
    mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->hostName().toLower() : share->hostName());
    mountpoint += QDir::separator();

    if (!share->isHomesShare())
    {
      mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->shareName().toLower() : share->shareName());
    }
    else
    {
      mountpoint += (Smb4KSettings::forceLowerCaseSubdirs() ? share->login().toLower() : share->login());
    }
    
    // Get the permissions that should be used for creating the
    // mount prefix and all its subdirectories. 
    // Please note that the actual permissions of the mount points
    // are determined by the mount utility.
    QFile::Permissions permissions;
    KUrl parentDirectory;
    
    if (QFile::exists(Smb4KSettings::mountPrefix().path()))
    {
      parentDirectory = Smb4KSettings::mountPrefix();
    }
    else
    {
      KUrl u(Smb4KSettings::mountPrefix());
      parentDirectory = u.upUrl();
    }
    
    QFile f(parentDirectory.path());
    permissions = f.permissions();
    
    QDir dir(mountpoint);

    if (!dir.mkpath(dir.path()))
    {
      share->setPath("");
      Smb4KNotification::mkdirFailed(dir);
      return;
    }
    else
    {
      KUrl u(dir.path());
      
      while (!parentDirectory.equals(u, KUrl::CompareWithoutTrailingSlash))
      {
        QFile(u.path()).setPermissions(permissions);
        u = u.upUrl();
      }
    }
  
    share->setPath(QDir::cleanPath(mountpoint));

    // Create the mount action.
    if (createMountAction(share, &mountAction))
    {
      connect( mountAction.watcher(), SIGNAL(actionPerformed(ActionReply)),
               this, SLOT(slotActionFinished(ActionReply)) );

      actions << mountAction;
    }
    else
    {
      // The creation of the mount action failed. Remove the
      // mountpoint again.
      dir.rmdir(dir.canonicalPath());
      if (dir.cdUp())
      {
        dir.rmdir(dir.canonicalPath());
      }
      else
      {
        // Do nothing
      }
    }
  }

  if (!actions.isEmpty())
  {
    emit aboutToStart(m_shares);
    Action::executeActions(actions, NULL, "net.sourceforge.smb4k.mounthelper");
  }
  else
  {
    // No aboutToStart() signal should have been emitted,
    // so there is no need to emit a finished() signal.
    emitResult();
  }
}


void Smb4KMountJob::slotActionFinished(ActionReply reply)
{
  // Count the processed actions.
  m_processed++;

  if (reply.succeeded())
  {
    QMutableListIterator<Smb4KShare *> it(m_shares);

    while( it.hasNext() )
    {
      Smb4KShare *share = it.next();

      // Check if the mount process reported an error
      QString errorMsg(reply.data()["mh_error_message"].toString().trimmed());

      if (QString::compare(share->canonicalPath(), reply.data()["mh_mountpoint"].toString()) == 0 && !errorMsg.isEmpty())
      {
#if defined(Q_OS_LINUX)
        if (errorMsg.contains("mount error 13") || errorMsg.contains("mount error(13)") /* authentication error */)
        {
          m_auth_errors << new Smb4KShare(*share);
        }
        else if ((errorMsg.contains("mount error 6") || errorMsg.contains("mount error(6)")) /* bad share name */ &&
                  share->shareName().contains("_", Qt::CaseSensitive))
        {
          QString share_name = share->shareName();
          share->setShareName(share_name.replace('_', ' '));
          m_retries << new Smb4KShare(*share);
        }
        else if (errorMsg.contains("mount error 101") || errorMsg.contains("mount error(101)") /* network unreachable */)
        {
          qDebug() << "Network unreachable ..." << endl;
        }
        else if (errorMsg.contains("Unable to find suitable address."))
        {
          // Swallow this
        }
        else
        {
          Smb4KNotification::mountingFailed(share, errorMsg);
        }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
        if (errorMsg.contains("Authentication error"))
        {
          m_auth_errors << new Smb4KShare(*share);
        }
        else if (errorMsg.contains("Permission denied"))
        {
          m_auth_errors << new Smb4KShare(*share);
        }
        else
        {
          Smb4KNotification::mountingFailed(share, errorMsg);
        }
#else
	qWarning() << "Smb4KMountJob::slotActionFinished(): Error handling not implemented!";
	Smb4KNotification::mountingFailed(share, errorMsg);
#endif
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // The auth action failed. Report this.
    if (reply.type() == ActionReply::KAuthError)
    {
      Smb4KNotification::actionFailed(reply.errorCode());
    }
    else
    {
      Smb4KNotification::actionFailed();
    }
  }

  if (m_processed == m_shares.size())
  {
    // Emit the necessary signals.
    if (!m_auth_errors.isEmpty())
    {
      emit authError(this);
    }
    else
    {
      // Do nothing
    }

    if (!m_retries.isEmpty())
    {
      emit retry(this);
    }
    else
    {
      // Do nothing
    }
    
    // Give the operating system some time to process the mounts
    // before we invoke KMountPoint::currentMountPoints().
    QTimer::singleShot(500, this, SLOT(slotFinishJob()));
  }
}


void Smb4KMountJob::slotFinishJob()
{
  QMutableListIterator<Smb4KShare *> it( m_shares );
  Smb4KShare *share = NULL;

  while ( it.hasNext() )
  {
    share = it.next();

    // Check which share has been mounted and emit the mounted() signal
    // if appropriate.
    if ( !share->isMounted() )
    {
      KMountPoint::List mount_points = KMountPoint::currentMountPoints( KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions );

      for ( int i = 0; i < mount_points.size(); ++i )
      {
        if ( QString::compare( mount_points.at( i )->mountPoint(), share->path() ) == 0 ||
             QString::compare( mount_points.at( i )->mountPoint(), share->canonicalPath() ) == 0 )
        {
          share->setIsMounted( true );
          emit mounted( share );
          break;
        }
        else
        {
          continue;
        }
      }
    }
    else
    {
      // Do nothing
    }
  }

  // Emit result() signal and tell the job to finish.
  emitResult();
  emit finished( m_shares );
}


Smb4KUnmountJob::Smb4KUnmountJob( QObject *parent ) : KJob( parent ),
  m_started( false ), m_parent_widget( NULL ), m_processed( 0 )
{
  setCapabilities( KJob::Killable );
}


Smb4KUnmountJob::~Smb4KUnmountJob()
{
  while ( !m_shares.isEmpty() )
  {
    delete m_shares.takeFirst();
  }
}


void Smb4KUnmountJob::start()
{
  m_started = true;
  
  if ( !m_synchron )
  {
    QTimer::singleShot( 50, this, SLOT(slotStartUnmount()) );
  }
  else
  {
    slotStartUnmount();
  }
}


void Smb4KUnmountJob::setupUnmount( Smb4KShare *share, bool force, bool silent, bool synchron, QWidget *parent )
{
  Q_ASSERT( share );
  m_shares << new Smb4KShare( *share );
  m_force = force;
  m_silent = silent;
  m_synchron = synchron;
  m_parent_widget = parent;
}


void Smb4KUnmountJob::setupUnmount( const QList<Smb4KShare *> &shares, bool force, bool silent, bool synchron, QWidget* parent )
{
  QListIterator<Smb4KShare *> it( shares );

  while ( it.hasNext() )
  {
    Smb4KShare *share = it.next();
    Q_ASSERT( share );
    m_shares << new Smb4KShare( *share );
  }

  m_force = force;
  m_silent = silent;
  m_synchron = synchron;
  m_parent_widget = parent;
}


bool Smb4KUnmountJob::createUnmountAction(Smb4KShare *share, Action *action)
{
  Q_ASSERT(share);
  Q_ASSERT(action);
  
  if (!share || !action)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Find the umount program.
  QString umount;
  QStringList paths;
  paths << "/bin";
  paths << "/sbin";
  paths << "/usr/bin";
  paths << "/usr/sbin";
  paths << "/usr/local/bin";
  paths << "/usr/local/sbin";

  for ( int i = 0; i < paths.size(); ++i )
  {
    umount = KGlobal::dirs()->findExe("umount", paths.at(i));

    if (!umount.isEmpty())
    {
      break;
    }
    else
    {
      continue;
    }
  }

  if (umount.isEmpty() && !m_silent)
  {
    Smb4KNotification::commandNotFound("umount");
    return false;
  }
  else
  {
    // Do nothing
  }

  QStringList options;
#if defined(Q_OS_LINUX)
  if (m_force)
  {
    options << "-l"; // lazy unmount
  }
  else
  {
    // Do nothing
  }
#endif

  action->setName("net.sourceforge.smb4k.mounthelper.unmount");
  action->setHelperID("net.sourceforge.smb4k.mounthelper");
  action->addArgument("mh_command", umount);
  action->addArgument("mh_url", share->url().url());
  action->addArgument("mh_mountpoint", share->canonicalPath());
  action->addArgument("mh_options", options);

  return true;
}


bool Smb4KUnmountJob::doKill()
{
  Action( "net.sourceforge.smb4k.mounthelper.unmount" ).stop();
  return KJob::doKill();
}


void Smb4KUnmountJob::slotStartUnmount()
{
  QList<Action> actions;
  QMutableListIterator<Smb4KShare *> it(m_shares);

  while (it.hasNext())
  {
    Smb4KShare *share = it.next();
    Action unmountAction;

    if (createUnmountAction(share, &unmountAction))
    {
      connect(unmountAction.watcher(), SIGNAL(actionPerformed(ActionReply)),
              this, SLOT(slotActionFinished(ActionReply)));

      actions << unmountAction;
    }
    else
    {
      // Do nothing
    }
  }

  if (!actions.isEmpty())
  {
    emit aboutToStart(m_shares);
    Action::executeActions(actions, NULL, "net.sourceforge.smb4k.mounthelper");
  }
  else
  {
    // No aboutToStart() signal should have been emitted,
    // so there is no need to emit a finished() signal.
    emitResult();
  }
}


void Smb4KUnmountJob::slotActionFinished(ActionReply reply)
{
  m_processed++;

  if (reply.succeeded())
  {
    QMutableListIterator<Smb4KShare *> it(m_shares);

    while(it.hasNext())
    {
      Smb4KShare *share = it.next();

      // Check if the unmount process reported an error
      QString errorMsg(reply.data()["mh_error_message"].toString().trimmed());

      if (QString::compare(share->canonicalPath(), reply.data()["mh_mountpoint"].toString()) == 0 && 
          !errorMsg.isEmpty() && !m_silent)
      {
        Smb4KNotification::unmountingFailed(share, errorMsg);
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // The auth action failed. Report this.
    if (!m_silent)
    {
      if (reply.type() == ActionReply::KAuthError)
      {
        Smb4KNotification::actionFailed(reply.errorCode());
      }
      else
      {
        Smb4KNotification::actionFailed();
      }
    }
    else
    {
      // Do nothing
    }
  }

  if (m_processed == m_shares.size())
  {
    // Give the operating system some time to process the unmounts
    // before we invoke KMountPoint::currentMountPoints(). It seems
    // that we need at least 500 ms, so that even slow systems have
    // the opportunity to unregister the mounts.
    QTimer::singleShot(500, this, SLOT(slotFinishJob()));
  }
}


void Smb4KUnmountJob::slotFinishJob()
{
  QMutableListIterator<Smb4KShare *> it(m_shares);
  Smb4KShare *share = NULL;

  while (it.hasNext())
  {
    share = it.next();

    // Check if the share has been unmounted, emit the unmounted()
    // signal and remove the mounpoint if appropriate.
    if (share->isMounted())
    {
      KMountPoint::List mount_points = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
      bool mountpoint_found = false;

      for (int i = 0; i < mount_points.size(); ++i)
      {
        if (QString::compare(mount_points.at(i)->mountPoint(), share->path()) == 0 ||
            QString::compare(mount_points.at(i)->mountPoint(), share->canonicalPath()) == 0)
        {
          mountpoint_found = true;
          break;
        }
        else
        {
          continue;
        }
      }

      if (!mountpoint_found)
      {
        share->setIsMounted(false);
        emit unmounted(share);
        
        // It is essential that the mountpoint is removed AFTER
        // the unmount() signal is emitted, because otherwise
        // Smb4KShare::canonicalPath() would return an empty string
        // and the checks in Smb4KMounter::slotShareUnmounted() would 
        // fail.
        QDir dir(share->canonicalPath());
        dir.rmdir(dir.canonicalPath());
        
        if (dir.cdUp())
        {
          dir.rmdir(dir.canonicalPath());
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing
    }
  }

  // Emit result() signal and tell the job to finish.
  emitResult();
  emit finished(m_shares);
}



Smb4KMountDialog::Smb4KMountDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ), m_share( share ), m_valid( true )
{
  setCaption( i18n( "Mount Share" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  setupView();

  connect( this, SIGNAL(okClicked()), SLOT(slotOkClicked()) );
  connect( this, SIGNAL(cancelClicked()), SLOT(slotCancelClicked()) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  restoreDialogSize( group );

  m_share_input->completionObject()->setItems( group.readEntry( "ShareNameCompletion", QStringList() ) );
  m_ip_input->completionObject()->setItems( group.readEntry( "IPAddressCompletion", QStringList() ) );
  m_workgroup_input->completionObject()->setItems( group.readEntry( "WorkgroupCompletion", QStringList() ) );
}


Smb4KMountDialog::~Smb4KMountDialog()
{
}


void Smb4KMountDialog::setupView()
{
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QVBoxLayout *layout = new QVBoxLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QWidget *description = new QWidget( main_widget );

  QHBoxLayout *desc_layout = new QHBoxLayout( description );
  desc_layout->setSpacing( 5 );
  desc_layout->setMargin( 0 );

  QLabel *pixmap = new QLabel( description );
  QPixmap mount_pix = KIcon( "view-form", KIconLoader::global(), QStringList( "emblem-mounted" ) ).pixmap( KIconLoader::SizeHuge );
  pixmap->setPixmap( mount_pix );
  pixmap->setAlignment( Qt::AlignBottom );

  QLabel *label = new QLabel( i18n( "Enter the location (UNC address) and optionally the IP address and "
                                    "workgroup to mount a share." ), description );
  label->setWordWrap( true );
  label->setAlignment( Qt::AlignBottom );

  desc_layout->addWidget( pixmap, 0 );
  desc_layout->addWidget( label, Qt::AlignBottom );

  QWidget *edit_widget = new QWidget( main_widget );

  QGridLayout *edit_layout = new QGridLayout( edit_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *shareLabel = new QLabel( i18n( "UNC Address:" ), edit_widget );
  m_share_input = new KLineEdit( edit_widget );
  m_share_input->setWhatsThis( i18n( "The Uniform Naming Convention (UNC) address "
    "describes the location of the share. It has the following syntax: "
    "//[USER@]HOST/SHARE. The username is optional." ) );
//   m_share_input->setToolTip( i18n( "The UNC address of the share" ) );
  m_share_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_share_input->setClearButtonShown( true );
  m_share_input->setMinimumWidth( 200 );
  m_share_input->setFocus();

  QLabel *addressLabel = new QLabel( i18n( "IP Address:" ), edit_widget );
  m_ip_input = new KLineEdit( edit_widget);
  m_ip_input->setWhatsThis( i18n( "The Internet Protocol (IP) address identifies the "
    "host in the network and indicates where it is. It has two valid formats, the one "
    "known as IP version 4 (e.g. 192.168.2.11) and the version 6 format "
    "(e.g. 2001:0db8:85a3:08d3:1319:8a2e:0370:7334)." ) );
//   m_ip_input->setToolTip( i18n( "The IP address of the host where the share is located" ) );
  m_ip_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_ip_input->setClearButtonShown( true );
  m_ip_input->setMinimumWidth( 200 );

  QLabel *workgroupLabel = new QLabel( i18n( "Workgroup:" ), edit_widget );
  m_workgroup_input = new KLineEdit( edit_widget );
  m_workgroup_input->setWhatsThis( i18n( "The workgroup or domain identifies the "
    "peer-to-peer computer network the host is located in." ) );
//   m_workgroup_input->setToolTip( i18n( "The workgroup where the host is located" ) );
  m_workgroup_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_workgroup_input->setClearButtonShown( true );
  m_workgroup_input->setMinimumWidth( 200 );

  edit_layout->addWidget( shareLabel, 0, 0, 0 );
  edit_layout->addWidget( m_share_input, 0, 1, 0 );
  edit_layout->addWidget( addressLabel, 1, 0, 0 );
  edit_layout->addWidget( m_ip_input, 1, 1, 0 );
  edit_layout->addWidget( workgroupLabel, 2, 0, 0 );
  edit_layout->addWidget( m_workgroup_input, 2, 1, 0 );

  m_bookmark = new QCheckBox( i18n( "Add this share to the bookmarks" ), main_widget );
  m_bookmark->setWhatsThis( i18n( "If you tick this checkbox, the share will be bookmarked "
    "and you can access it e.g. through the \"Bookmarks\" menu entry in the main window." ) );
//   m_bookmark->setToolTip( i18n( "Add this share to the bookmarks" ) );

  layout->addWidget( description, Qt::AlignBottom );
  layout->addWidget( edit_widget, 0 );
  layout->addWidget( m_bookmark, 0 );

  slotChangeInputValue( m_share_input->text() );

  // Connections
  connect( m_share_input,     SIGNAL(textChanged(QString)) ,
           this,              SLOT(slotChangeInputValue(QString)) );

  connect( m_share_input,     SIGNAL(editingFinished()),
           this,              SLOT(slotShareNameEntered()) );

  connect( m_ip_input,        SIGNAL(editingFinished()),
           this,              SLOT(slotIPEntered()) );

  connect( m_workgroup_input, SIGNAL(editingFinished()),
           this,              SLOT(slotWorkgroupEntered()) );
}


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMountDialog::slotChangeInputValue( const QString& _test)
{
  enableButtonOk( !_test.isEmpty() );
}


void Smb4KMountDialog::slotOkClicked()
{
  if ( !m_share_input->text().trimmed().isEmpty() )
  {
    KUrl url;

    // Take care of Windows-like UNC addresses:
    if ( m_share_input->text().trimmed().startsWith( QLatin1String( "\\" ) ) )
    {
      QString unc = m_share_input->text();
      unc.replace( "\\", "/" );
      url.setUrl( unc, KUrl::TolerantMode );
    }
    else
    {
      url.setUrl( m_share_input->text().trimmed(), KUrl::TolerantMode );
    }

    url.setProtocol( "smb" );

    if ( url.isValid() && url.hasHost() && url.hasPath() && !url.path().endsWith( '/' ) )
    {
      m_share->setURL( url );
      m_share->setWorkgroupName( m_workgroup_input->text().trimmed() );
      m_share->setHostIP( m_ip_input->text().trimmed() );
    }
    else
    {
      Smb4KNotification::invalidURLPassed();
      m_valid = false;
    }
  }
  else
  {
    // Do nothing
  }

  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
  group.writeEntry( "ShareNameCompletion", m_share_input->completionObject()->items() );
  group.writeEntry( "IPAddressCompletion", m_ip_input->completionObject()->items() );
  group.writeEntry( "WorkgroupCompletion", m_workgroup_input->completionObject()->items() );
}


void Smb4KMountDialog::slotCancelClicked()
{
  Smb4KMounter::self()->abort( m_share );
}


void Smb4KMountDialog::slotShareNameEntered()
{
  KCompletion *completion = m_share_input->completionObject();
  KUrl url( m_share_input->userText() );
  url.setProtocol( "smb" );

  if ( url.isValid() && !url.isEmpty() )
  {
    completion->addItem( m_share_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotIPEntered()
{
  KCompletion *completion = m_ip_input->completionObject();

  if ( !m_ip_input->userText().isEmpty() )
  {
    completion->addItem( m_ip_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotWorkgroupEntered()
{
  KCompletion *completion = m_workgroup_input->completionObject();

  if ( !m_workgroup_input->userText().isEmpty() )
  {
    completion->addItem( m_workgroup_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


#include "smb4kmounter_p.moc"
