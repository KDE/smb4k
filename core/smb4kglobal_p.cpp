/***************************************************************************
    These are the private helper classes of the Smb4KGlobal namespace.
                             -------------------
    begin                : Di Jul 24 2007
    copyright            : (C) 2007-2019 by Alexander Reinholdt
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
#include <QDirIterator>


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
  
  //
  // File system watcher for smb.conf file
  // 
  m_watcher = new QFileSystemWatcher(this);
  
  //
  // Connections
  // 
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
  connect(m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(slotSmbConfModified(QString)));
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
  if ((!m_sambaConfigMissing && m_sambaOptions.isEmpty()) || read)
  {
    // 
    // Clear the options.
    // 
    m_sambaOptions.clear();
    
    // 
    // Now search the smb.conf file.
    // 
    // With the introduction of Samba 4, the smb.conf file might also
    // be named smb4.conf. Thus we need to search for two filenames.
    // 
    QStringList paths;
    paths << "/etc";
    paths << "/usr/local/etc";
    paths << "/usr/pkg/etc";
    
    QStringList files;
    files << "smb.conf";
    files << "smb4.conf";
    
    QString result;
    
    for (const QString &path : paths)
    {
      QDirIterator it(path, files, QDir::Files, QDirIterator::Subdirectories);
      
      while (it.hasNext())
      {
        result = it.next();
      }
      
      if (!result.isEmpty())
      {
        break;
      }
    }
    
    //
    // Check if we found the file and read it. Otherwise show an error
    // message to the user.
    // 
    QStringList contents;
    
    if (!result.isEmpty())
    {
      QFile smbConf(result);
      
      if (smbConf.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream ts(&smbConf);
        
        while (!ts.atEnd())
        {
          contents << ts.readLine(0);
        }
        
        smbConf.close();
      }
      else
      {
        Smb4KNotification::openingFileFailed(smbConf);
        return m_sambaOptions;
      }
      
      //
      // Add the file to the file system watcher
      // 
      m_watcher->addPath(result);
      
      //
      // Tell the program that the config file was found
      // 
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
    }
    
    //
    // Process the contents of the smb.conf file
    // 
    if (!contents.isEmpty())
    {
      //
      // Jump to the [global] section and get the listed parameters
      // 
      for (int i = contents.indexOf("[global]", 0); i < contents.size(); ++i)
      {
        if (i == -1)
        {
          // 
          // No [global] section found. Stop.
          // 
          break;
        }
        else if (contents.at(i).trimmed().startsWith('#') || contents.at(i).trimmed().startsWith(';') || contents.at(i).trimmed().isEmpty())
        {
          // 
          // This is either a comment or an empty line. We do not want it.
          // 
          continue;
        }
        else if (contents.at(i).trimmed().startsWith(QLatin1String("include")))
        {
          //
          // This is an include file. Get its contents and insert it into the 
          // global Samba options map
          // 
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
                  // 
                  // This is either a comment or an empty line. We do not want it.
                  // 
                  continue;
                }
                else
                {
                  //
                  // This is an option. Put it into the global Samba options map
                  // 
                  QString key = contents.at(i).section('=', 0, 0).trimmed().toLower();
                  QString value = contents.at(i).section('=', 1, 1).trimmed().toUpper();
          
                  m_sambaOptions.insert(key, value);
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
        else if (contents.at(i).trimmed().startsWith('[') && !contents.at(i).contains("[global]", Qt::CaseSensitive))
        {
          //
          // A new section begins. Stop.
          // 
          break;
        }
        else
        {
          //
          // This is an option. Put it into the global Samba options map
          // 
          QString key = contents.at(i).section('=', 0, 0).trimmed().toLower();
          QString value = contents.at(i).section('=', 1, 1).trimmed().toUpper();
          
          m_sambaOptions.insert(key, value);
        }
      }
    }
  }
    
  return m_sambaOptions;
}


void Smb4KGlobalPrivate::slotAboutToQuit()
{
  Smb4KSettings::self()->save();
}


void Smb4KGlobalPrivate::slotSmbConfModified(const QString &/*file*/)
{
  globalSambaOptions(true);
}

