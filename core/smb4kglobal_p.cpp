/***************************************************************************
    These are the private helper classes of the Smb4KGlobal namespace.
                             -------------------
    begin                : Di Jul 24 2007
    copyright            : (C) 2007-2017 by Alexander Reinholdt
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
#include "smb4kglobal_p.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Qt includes
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <QCoreApplication>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QHostInfo>


Smb4KGlobalPrivate::Smb4KGlobalPrivate()
{
  onlyForeignShares = false;
  coreInitialized = false;
  m_sambaConfigMissing = false;
  
#ifdef Q_OS_LINUX
  whitelistedMountArguments << "dynperm";
  whitelistedMountArguments << "rwpidforward";
  whitelistedMountArguments << "hard";
  whitelistedMountArguments << "soft";
  whitelistedMountArguments << "noacl";
  whitelistedMountArguments << "cifsacl";
  whitelistedMountArguments << "backupuid";
  whitelistedMountArguments << "backupgid";
  whitelistedMountArguments << "ignorecase";
  whitelistedMountArguments << "nocase";
  whitelistedMountArguments << "nobrl";
  whitelistedMountArguments << "sfu";
  whitelistedMountArguments << "nounix";
  whitelistedMountArguments << "nouser_xattr";
  whitelistedMountArguments << "fsc";
  whitelistedMountArguments << "multiuser";
  whitelistedMountArguments << "actimeo";
  whitelistedMountArguments << "noposixpaths";
  whitelistedMountArguments << "posixpaths";
#endif
}


Smb4KGlobalPrivate::~Smb4KGlobalPrivate()
{
  // Clear the workgroup list.
  while (!workgroupsList.isEmpty())
  {
    workgroupsList.takeFirst().clear();
  }

  // Clear the host list.
  while (!hostsList.isEmpty())
  {
    hostsList.takeFirst().clear();
  }

  // Clear the list of mounted shares.
  while (!mountedSharesList.isEmpty())
  {
    mountedSharesList.takeFirst().clear();
  }

  // Clear the list of shares.
  while (!sharesList.isEmpty())
  {
    sharesList.takeFirst().clear();
  }
}


const QMap<QString,QString> &Smb4KGlobalPrivate::globalSambaOptions(bool read)
{
  if (read)
  {
    // Clear the options.
    m_sambaOptions.clear();
    
    // Now search the smb.conf file and read the [global] section
    // from it.
    // With the introduction of Samba 4, the smb.conf file might also
    // be named smb4.conf. Thus we need to search for two filenames.
    // Please note that the file named smb.conf will always be picked
    // if it exists.
    QStringList paths;
    paths << "/etc";
    paths << "/etc/samba";
    paths << "/usr/local/etc";
    paths << "/usr/local/etc/samba";
    
    QFile smbConf;
    QStringList contents;

    for (int i = 0; i < paths.size(); ++i)
    {
      QDir::setCurrent(paths.at(i));
      QFile file1("smb.conf");
      QFile file2("smb4.conf");
      
      if (file1.exists())
      {
        smbConf.setFileName(QDir::currentPath()+QDir::separator()+file1.fileName());
        break;
      }
      else if (file2.exists())
      {
        smbConf.setFileName(QDir::currentPath()+QDir::separator()+file2.fileName());
        break;
      }
      else
      {
        continue;
      }
    }
    
    if (smbConf.exists())
    {
      if (smbConf.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream ts(&smbConf);
        
        while (!ts.atEnd())
        {
          contents.append(ts.readLine(0));
        }
        
        smbConf.close();
      }
      else
      {
        Smb4KNotification::openingFileFailed(smbConf);
        return m_sambaOptions;
      }

      m_sambaConfigMissing = false;
    }
    else
    {
      if (!m_sambaConfigMissing)
      {
        Smb4KNotification::sambaConfigFileMissing();
        m_sambaConfigMissing = true;
        return m_sambaOptions;
      }
      else
      {
        // Do nothing
      }
    }
    
    // Now process the contents of the smb.conf file.
    if (!contents.isEmpty())
    {
      // Process the file contents.
      for (int i = contents.indexOf("[global]", 0); i < contents.size(); ++i)
      {
        if (i == -1)
        {
          // The smb.conf file does not contain a global section.
          break;
        }
        else
        {
          // Do nothing
        }
        
        if (contents.at(i).trimmed().startsWith('#') || contents.at(i).trimmed().startsWith(';'))
        {
          // This is a comment. We do not need it.
          continue;
        }
        else if (contents.at(i).trimmed().startsWith(QLatin1String("include")))
        {
          // Look for the include file and put its contents into the
          // m_sambaOptions map.
          QFile includeFile(contents.at(i).section('=', 1, 1).trimmed());

          if (includeFile.exists())
          {
            if (includeFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
              QTextStream ts(&includeFile);

              QString buffer;

              while(!ts.atEnd())
              {
                buffer = ts.readLine(0).trimmed();

                if (buffer.startsWith('#') || buffer.startsWith(';'))
                {
                  continue;
                }
                else
                {
                  QString key = buffer.section('=', 0, 0).trimmed().toLower();
                  m_sambaOptions[key] = buffer.section('=', 1, 1).trimmed().toUpper();
                  continue;
                }
              }
            }
            else
            {
              Smb4KNotification::openingFileFailed(includeFile);
              continue;
            }
          }
        }
        else if (contents.at(i).startsWith('[') && !contents.at(i).contains("[global]", Qt::CaseSensitive))
        {
          // We reached the end of the [global] section. Stop here.
          break;
        }
        else
        {
          // Put the entries of the [global] section into the m_sambaOptions
          // map.
          QString key = contents.at(i).section('=', 0, 0).trimmed().toLower();
          m_sambaOptions[key] = contents.at(i).section('=', 1, 1).trimmed().toUpper();
        }
      }
    }
    else
    {
      // Nothing to do
    }

    // Post-processing. Some values should be entered with their defaults, if they are
    // not already present.
    if (!m_sambaOptions.contains("netbios name"))
    {
      m_sambaOptions["netbios name"] = QHostInfo::localHostName().toUpper();
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
  
  return m_sambaOptions;
}


void Smb4KGlobalPrivate::setDefaultSettings()
{
  // Samba options that have to be dynamically imported from smb.conf:
  QMap<QString, QString> opts = globalSambaOptions(true);

  if (!opts["netbios name"].isEmpty())
  {
    Smb4KSettings::self()->netBIOSNameItem()->setDefaultValue(opts["netbios name"]);

    if (Smb4KSettings::netBIOSName().isEmpty())
    {
      Smb4KSettings::self()->netBIOSNameItem()->setDefault();
    }
  }

  if (!opts["workgroup"].isEmpty())
  {
    Smb4KSettings::self()->domainNameItem()->setDefaultValue(opts["workgroup"]);

    if (Smb4KSettings::domainName().isEmpty())
    {
      Smb4KSettings::self()->domainNameItem()->setDefault();
    }
  }

  if (!opts["socket options"].isEmpty())
  {
    Smb4KSettings::self()->socketOptionsItem()->setDefaultValue(opts["socket options"]);

    if (Smb4KSettings::socketOptions().isEmpty())
    {
      Smb4KSettings::self()->socketOptionsItem()->setDefault();
    }
  }

  if (!opts["netbios scope"].isEmpty())
  {
    Smb4KSettings::self()->netBIOSScopeItem()->setDefaultValue(opts["netbios scope"]);

    if (Smb4KSettings::netBIOSScope().isEmpty())
    {
      Smb4KSettings::self()->netBIOSScopeItem()->setDefault();
    }
  }

  if (!opts["name resolve order"].isEmpty())
  {
    Smb4KSettings::self()->nameResolveOrderItem()->setDefaultValue(opts["name resolve order"]);

    if (Smb4KSettings::nameResolveOrder().isEmpty())
    {
      Smb4KSettings::self()->nameResolveOrderItem()->setDefault();
    }
  }

  QHostAddress address(opts["interfaces"].section(' ', 0, 0));

  if (address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
  {
    Smb4KSettings::self()->broadcastAddressItem()->setDefaultValue(address.toString());

    if (Smb4KSettings::broadcastAddress().isEmpty())
    {
      Smb4KSettings::self()->broadcastAddressItem()->setDefault();
    }
  }
}


void Smb4KGlobalPrivate::makeConnections()
{
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}


void Smb4KGlobalPrivate::slotAboutToQuit()
{
  Smb4KSettings::self()->save();
}
