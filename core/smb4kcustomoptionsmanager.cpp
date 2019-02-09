/***************************************************************************
    Manage custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2019 by Alexander Reinholdt
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
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptionsmanager_p.h"
#include "smb4kcustomoptions.h"
#include "smb4khomesshareshandler.h"
#include "smb4knotification.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4ksettings.h"
#include "smb4kglobal.h"
#include "smb4kprofilemanager.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QApplication>
#include <QPointer>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KCustomOptionsManagerStatic, p);


Smb4KCustomOptionsManager::Smb4KCustomOptionsManager(QObject *parent)
: QObject(parent), d(new Smb4KCustomOptionsManagerPrivate)
{
  // First we need the directory.
  QString path = dataLocation();
  
  QDir dir;
  
  if (!dir.exists(path))
  {
    dir.mkpath(path);
  }
  
  readCustomOptions();
  
  // Connections
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), 
          this, SLOT(slotAboutToQuit()));
}


Smb4KCustomOptionsManager::~Smb4KCustomOptionsManager()
{
}


Smb4KCustomOptionsManager *Smb4KCustomOptionsManager::self()
{
  return &p->instance;
}


#if !defined(SMB4K_UNSUPPORTED_PLATFORM)
void Smb4KCustomOptionsManager::addRemount(const SharePtr &share, bool always)
{
  if (share)
  {
    OptionsPtr options = findOptions(share, true);
    
    if (options)
    {
      // If the options are already in the list, check if the share is
      // always to be remounted. If so, ignore the 'always' argument
      // and leave that option untouched.
      if (options->remount() != Smb4KCustomOptions::RemountAlways)
      {
        options->setRemount(always ? Smb4KCustomOptions::RemountAlways : Smb4KCustomOptions::RemountOnce);
      }
    }
    else
    {
      options = OptionsPtr(new Smb4KCustomOptions(share.data()));
      options->setProfile(Smb4KProfileManager::self()->activeProfile());
      options->setRemount(always ? Smb4KCustomOptions::RemountAlways : Smb4KCustomOptions::RemountOnce);
      d->options << options;
    }
    
    writeCustomOptions();
  }
}


void Smb4KCustomOptionsManager::removeRemount(const SharePtr &share, bool force)
{
  if (share)
  {
    OptionsPtr options = findOptions(share, true);
    
    if (options)
    {
      if (options->remount() == Smb4KCustomOptions::RemountOnce)
      {
        options->setRemount(Smb4KCustomOptions::RemountNever);
      }
      else if (options->remount() == Smb4KCustomOptions::RemountAlways && force)
      {
        options->setRemount(Smb4KCustomOptions::RemountNever);
      }     
    }
    
    writeCustomOptions();
  }
}


void Smb4KCustomOptionsManager::clearRemounts(bool force)
{
  //
  // List of relevant custom options
  //
  QList<OptionsPtr> options = customOptions(false);
  
  //
  // Remove the remount flags
  //
  for (const OptionsPtr &o : options)
  {
    if (o->type() == Share)
    {
      if (o->remount() == Smb4KCustomOptions::RemountOnce)
      {
        o->setRemount(Smb4KCustomOptions::RemountNever);
      }
      else if (o->remount() == Smb4KCustomOptions::RemountAlways && force)
      {
        o->setRemount(Smb4KCustomOptions::RemountNever);
      }
    }
  }
  
  //
  // Write the options
  //
  writeCustomOptions();
}


QList<OptionsPtr> Smb4KCustomOptionsManager::sharesToRemount()
{
  //
  // List of relevant custom options
  //
  QList<OptionsPtr> options = customOptions(false);
  
  //
  // List of remounts
  //
  QList<OptionsPtr> remounts;
  
  //
  // Get the list of remounts
  //
  for (const OptionsPtr &o : options)
  {
    if (o->remount() == Smb4KCustomOptions::RemountOnce)
    {
      remounts << o;
    }
    else if (o->remount() == Smb4KCustomOptions::RemountAlways)
    {
      remounts << o;
    }
  }
  
  //
  // Return relevant options
  //
  return remounts;
}
#endif


OptionsPtr Smb4KCustomOptionsManager::findOptions(const NetworkItemPtr &networkItem, bool exactMatch)
{
  //
  // The options that are to be returned
  //
  OptionsPtr options;
  
  //
  // Get the list of options
  // 
  QList<OptionsPtr> optionsList = customOptions(false);
  
  //
  // Only do something if the list of options is not empty.
  // 
  if (!optionsList.isEmpty())
  {
    switch (networkItem->type())
    {
      case Host:
      {
        HostPtr host = networkItem.staticCast<Smb4KHost>();
        
        if (host)
        {
          for (const OptionsPtr &o : optionsList)
          {
            // In case of a host, there can only be an exact match.
            if (QString::compare(host->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                                 o->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                                 Qt::CaseInsensitive) == 0 ||
                (host->url().isEmpty() && host->ipAddress() == o->ipAddress()))
            {
              options = o;
              break;
            }
          }
        }
        break;
      }
      case Share:
      {
        SharePtr share = networkItem.staticCast<Smb4KShare>();
        
        if (share)
        {
          for (const OptionsPtr &o : optionsList)
          {
            if (QString::compare(share->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                                 o->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                                 Qt::CaseInsensitive) == 0)
            {
              options = o;
              break;
            }
            else if (!exactMatch && o->type() == Host && 
                     QString::compare(share->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath),
                                      o->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath),
                                      Qt::CaseInsensitive) == 0)
            {
              options = o;
            }
          }
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  return options;
}


OptionsPtr Smb4KCustomOptionsManager::findOptions(const QUrl &url)
{
  //
  // The options that are to be returned
  //
  OptionsPtr options;
  
  //
  // Search the options for the given URL
  //
  if (url.isValid() && url.scheme() == "smb")
  {
    //
    // Get the relevant options
    //
    QList<OptionsPtr> optionsList = customOptions(false);    
    
    //
    // Get the options
    //
    for (const OptionsPtr &o : optionsList)
    {
      if (o->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::StripTrailingSlash) == 
          url.toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::StripTrailingSlash))
      {
        options = o;
        break;
      }
    }
  }
  
  //
  // Return the options
  //
  return options;  
}


void Smb4KCustomOptionsManager::readCustomOptions()
{
  //
  // Clear the list of options
  //
  while (!d->options.isEmpty())
  {
    d->options.takeFirst().clear();
  }
  
  //
  // Set the XML file
  //
  QFile xmlFile(dataLocation()+QDir::separator()+"custom_options.xml");

  if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QXmlStreamReader xmlReader(&xmlFile);

    while (!xmlReader.atEnd())
    {
      xmlReader.readNext();

      if (xmlReader.isStartElement())
      {
        if (xmlReader.name() == "custom_options" && 
            (xmlReader.attributes().value("version") != "1.2" && xmlReader.attributes().value("version") != "2.0"))
        {
          xmlReader.raiseError(i18n("The format of %1 is not supported.", xmlFile.fileName()));
          break;
        }
        else
        {
          if (xmlReader.name() == "options")
          {
            OptionsPtr options = OptionsPtr(new Smb4KCustomOptions());
            options->setProfile(xmlReader.attributes().value("profile").toString());
            
            //
            // Initialize the options 
            // 
            if (QString::compare(xmlReader.attributes().value("type").toString(), "host", Qt::CaseInsensitive) == 0)
            {
              options->setHost(new Smb4KHost());
            }
            else
            {
              options->setShare(new Smb4KShare());
            }

            while (!(xmlReader.isEndElement() && xmlReader.name() == "options"))
            {
              xmlReader.readNext();
  
              if (xmlReader.isStartElement())
              {
                if (xmlReader.name() == "workgroup")
                {
                  options->setWorkgroupName(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "url")
                {
                  QUrl url(xmlReader.readElementText());
                  options->setUrl(url);
                }
                else if (xmlReader.name() == "unc")
                {
                  QUrl url = QUrl::fromUserInput(xmlReader.readElementText());
                  url.setScheme("smb");
                  options->setUrl(url);
                }
                else if (xmlReader.name() == "ip")
                {
                  options->setIpAddress(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "custom")
                {
                  while (!(xmlReader.isEndElement() && xmlReader.name() == "custom"))
                  {
                    xmlReader.readNext();
                    
                    if (xmlReader.isStartElement())
                    {
                      if (xmlReader.name() == "smb_port")
                      {
                        bool ok = false;
                        
                        int portNumber = xmlReader.readElementText().toInt(&ok);
                        
                        if (ok)
                        {
                          options->setSmbPort(portNumber);
                        }
                      }
                      else if (xmlReader.name() == "use_smb_port")
                      {
                        QString useSmbPort = xmlReader.readElementText();
                        
                        if (useSmbPort == "true")
                        {
                          options->setUseSmbPort(true);
                        }
                        else
                        {
                          options->setUseSmbPort(false);
                        }
                      }
                      else if (xmlReader.name() == "kerberos")
                      {
                        QString useKerberos = xmlReader.readElementText();
                        
                        if (useKerberos == "true")
                        {
                          options->setUseKerberos(true);
                        }
                        else
                        {
                          options->setUseKerberos(false);
                        }
                      }
                      else if (xmlReader.name() == "mac_address")
                      {
                        QString macAddress = xmlReader.readElementText();
                        
                        QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
  
                        if (exp.exactMatch(macAddress))
                        {
                          options->setMACAddress(macAddress);
                        }
                      }
                      else if (xmlReader.name() == "wol_send_before_first_scan")
                      {
                        QString send = xmlReader.readElementText();
                        
                        if (send == "true")
                        {
                          options->setWOLSendBeforeNetworkScan(true);
                        }
                        else
                        {
                          options->setWOLSendBeforeNetworkScan(false);
                        }
                      }
                      else if (xmlReader.name() == "wol_send_before_mount")
                      {
                        QString send = xmlReader.readElementText();
                        
                        if (send == "true")
                        {
                          options->setWOLSendBeforeMount(true);
                        }
                        else
                        {
                          options->setWOLSendBeforeMount(false);
                        }
                      }
#if !defined(SMB4K_UNSUPPORTED_PLATFORM)
                      else if (xmlReader.name() == "remount")
                      {
                        QString remount = xmlReader.readElementText();
                        
                        if (remount == "once")
                        {
                          options->setRemount(Smb4KCustomOptions::RemountOnce);
                        }
                        else if (remount == "always")
                        {
                          options->setRemount(Smb4KCustomOptions::RemountAlways);
                        }
                        else if (remount == "never")
                        {
                          options->setRemount(Smb4KCustomOptions::RemountNever);
                        }
                        else
                        {
                          options->setRemount(Smb4KCustomOptions::UndefinedRemount);
                        }
                      }
                      else if (xmlReader.name() == "use_user")
                      {
                        QString useUser = xmlReader.readElementText();
                        
                        if (useUser == "true")
                        {
                          options->setUseUser(true);
                        }
                        else
                        {
                          options->setUseUser(false);
                        }
                      }
                      else if (xmlReader.name() == "uid")
                      {
                        KUser user((K_UID)xmlReader.readElementText().toInt());
                        
                        if (user.isValid())
                        {
                          options->setUser(user);
                        }
                      }
                      else if (xmlReader.name() == "use_group")
                      {
                        QString useGroup = xmlReader.readElementText();
                        
                        if (useGroup == "true")
                        {
                          options->setUseGroup(true);
                        }
                        else
                        {
                          options->setUseGroup(false);
                        }
                      }
                      else if (xmlReader.name() == "gid")
                      {
                        KUserGroup group((K_GID)xmlReader.readElementText().toInt());
                        
                        if (group.isValid())
                        {
                          options->setGroup(group);
                        }
                      }
                      else if (xmlReader.name() == "use_file_mode")
                      {
                        QString useFileMode = xmlReader.readElementText();
                        
                        if (useFileMode == "true")
                        {
                          options->setUseFileMode(true);
                        }
                        else
                        {
                          options->setUseFileMode(false);
                        }
                      }
                      else if (xmlReader.name() == "file_mode")
                      {
                        options->setFileMode(xmlReader.readElementText());
                      }
                      else if (xmlReader.name() == "use_directory_mode")
                      {
                        QString useDirectoryMode = xmlReader.readElementText();
                        
                        if (useDirectoryMode == "true")
                        {
                          options->setUseDirectoryMode(true);
                        }
                        else
                        {
                          options->setUseDirectoryMode(false);
                        }
                      }
                      else if (xmlReader.name() == "directory_mode")
                      {
                        options->setDirectoryMode(xmlReader.readElementText());
                      }
#endif
#if defined(Q_OS_LINUX)
                      else if (xmlReader.name() == "cifs_unix_extensions_support")
                      {
                        QString support = xmlReader.readElementText();
                        
                        if (support == "true")
                        {
                          options->setCifsUnixExtensionsSupport(true);
                        }
                        else
                        {
                          options->setCifsUnixExtensionsSupport(false);
                        }
                      }
                      else if (xmlReader.name() == "use_filesystem_port")
                      {
                        QString useFilesystemPort = xmlReader.readElementText();
                        
                        if (useFilesystemPort == "true")
                        {
                          options->setUseFileSystemPort(true);
                        }
                        else
                        {
                          options->setUseFileSystemPort(false);
                        }
                      }
                      else if (xmlReader.name() == "filesystem_port")
                      {
                        bool ok = false;
                        
                        int portNumber = xmlReader.readElementText().toInt(&ok);
                        
                        if (ok)
                        {
                          options->setFileSystemPort(portNumber);
                        }
                      }
                      else if (xmlReader.name() == "use_security_mode")
                      {
                        QString useSecurityMode = xmlReader.readElementText();
                        
                        if (useSecurityMode == "true")
                        {
                          options->setUseSecurityMode(true);
                        }
                        else
                        {
                          options->setUseSecurityMode(false);
                        }
                      }
                      else if (xmlReader.name() == "security_mode")
                      {
                        QString securityMode = xmlReader.readElementText();
                        
                        if (securityMode == "none")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::None);
                        }
                        else if (securityMode == "krb5")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Krb5);
                        }
                        else if (securityMode == "krb5i")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Krb5i);
                        }
                        else if (securityMode == "ntlm")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlm);
                        }
                        else if (securityMode == "ntlmi")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmi);
                        }
                        else if (securityMode == "ntlmv2")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmv2);
                        }
                        else if (securityMode == "ntlmv2i")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmv2i);
                        }
                        else if (securityMode == "ntlmssp")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmssp);
                        }
                        else if (securityMode == "ntlmsspi")
                        {
                          options->setSecurityMode(Smb4KMountSettings::EnumSecurityMode::Ntlmsspi);
                        }
                      }
                      else if (xmlReader.name() == "use_write_access")
                      {
                        QString useWriteAccess = xmlReader.readElementText();
                        
                        if (useWriteAccess == "true")
                        {
                          options->setUseWriteAccess(true);
                        }
                        else
                        {
                          options->setUseWriteAccess(false);
                        }
                      }
                      else if (xmlReader.name() == "write_access")
                      {
                        QString writeAccess = xmlReader.readElementText();
                        
                        if (writeAccess == "true")
                        {
                          options->setWriteAccess(Smb4KMountSettings::EnumWriteAccess::ReadWrite);
                        }
                        else if (writeAccess == "false")
                        {
                          options->setWriteAccess(Smb4KMountSettings::EnumWriteAccess::ReadWrite);
                        }
                      }
#endif
                    }
                  }
                }
  
                continue;
              }
              else
              {
                continue;
              }
            }            
              
            d->options << options;  
          }
        }
      }
    }

    xmlFile.close();

    if (xmlReader.hasError())
    {
      Smb4KNotification::readingFileFailed(xmlFile, xmlReader.errorString());
    }
  }
  else
  {
    if (xmlFile.exists())
    {
      Smb4KNotification::openingFileFailed(xmlFile);
    }
  }
}


void Smb4KCustomOptionsManager::writeCustomOptions()
{
  //
  // Set the XML file
  //
  QFile xmlFile(dataLocation()+QDir::separator()+"custom_options.xml");
  
  //
  // Write the options to the file
  //
  if (!d->options.isEmpty())
  {
    if (xmlFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
      QXmlStreamWriter xmlWriter(&xmlFile);
      xmlWriter.setAutoFormatting(true);
      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement("custom_options");
      xmlWriter.writeAttribute("version", "2.0");
      
      for (const OptionsPtr &options : d->options)
      {
#if !defined(SMB4K_UNSUPPORTED_PLATFORM)
        if (options->hasOptions() || options->remount() == Smb4KCustomOptions::RemountOnce)
#else
        if (options->hasOptions())
#endif
        {
          xmlWriter.writeStartElement("options");
          xmlWriter.writeAttribute("type", options->type() == Host ? "host" : "share");
          xmlWriter.writeAttribute("profile", options->profile());

          xmlWriter.writeTextElement("workgroup", options->workgroupName());
          xmlWriter.writeTextElement("url", options->url().toDisplayString());
          xmlWriter.writeTextElement("ip", options->ipAddress());
          
          xmlWriter.writeStartElement("custom");

          QMap<QString,QString> map = options->customOptions();
          QMapIterator<QString,QString> it(map);

          while (it.hasNext())
          {
            it.next();

            if (!it.value().isEmpty())
            {
              xmlWriter.writeTextElement(it.key(), it.value());
            }
          }

          xmlWriter.writeEndElement();
          xmlWriter.writeEndElement();
        }
      }
      
      xmlWriter.writeEndDocument();
      xmlFile.close();
    }
    else
    {
      Smb4KNotification::openingFileFailed(xmlFile);
    }      
  }
  else
  {
    xmlFile.remove();
  }
  
  // FIXME: Flush the list of custom options and reload it?
}


QList<OptionsPtr> Smb4KCustomOptionsManager::customOptions(bool optionsOnly)
{
  //
  // Options list
  //
  QList<OptionsPtr> options;

  //
  // Get this list of options
  //
  for (const OptionsPtr &o : d->options)
  {
    if (Smb4KSettings::useProfiles() && o->profile() != Smb4KProfileManager::self()->activeProfile())
    {
      continue;
    }

#if !defined(SMB4K_UNSUPPORTED_PLATFORM)
    if (o->hasOptions() || (!optionsOnly && o->remount() == Smb4KCustomOptions::RemountOnce))
#else
    if (o->hasOptions())
#endif
    {
      options << o;
    }
  }
  
  //
  // Return the list of relevant options
  //
  return options;
}


void Smb4KCustomOptionsManager::replaceCustomOptions(const QList<OptionsPtr> &optionsList)
{
  //
  // Clear the list of options. Honor profiles.
  //
  QMutableListIterator<OptionsPtr> it(d->options);
  
  while (it.hasNext())
  {
    OptionsPtr options = it.next();
    
    if (Smb4KSettings::useProfiles() && options->profile() != Smb4KProfileManager::self()->activeProfile())
    {
      continue;
    }
    
    it.remove();
  }
  
  //
  // Append the new list
  //
  if (!optionsList.isEmpty())
  {
    for (const OptionsPtr &options : optionsList)
    {
      if (Smb4KSettings::useProfiles())
      {
        options->setProfile(Smb4KProfileManager::self()->activeProfile());
      }
      
#if !defined(SMB4K_UNSUPPORTED_PLATFORM)
      if (options->hasOptions() || options->remount() == Smb4KCustomOptions::RemountOnce)
#else
      if (options->hasOptions())
#endif
      {
        d->options << options;
      }
    }
  }
  
  writeCustomOptions();
}


void Smb4KCustomOptionsManager::openCustomOptionsDialog(const NetworkItemPtr &item)
{
  if (item)
  {
    OptionsPtr options;
    
    switch (item->type())
    {
      case Host:
      {
        HostPtr host = item.staticCast<Smb4KHost>();
        
        if (host)
        {
          options = findOptions(host);
          
          if (!options)
          {
            options = OptionsPtr(new Smb4KCustomOptions(host.data()));
            options->setProfile(Smb4KProfileManager::self()->activeProfile());
          }
        }
        
        break;
      }
      case Share:
      {
        SharePtr share = item.staticCast<Smb4KShare>();
        
        if (share && !share->isPrinter())
        {
          if (share->isHomesShare())
          {
            if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true))
            {
              return;
            }
          }
          
          options = findOptions(share);
          
          if (!options)
          {
            options = OptionsPtr(new Smb4KCustomOptions(share.data()));
            options->setProfile(Smb4KProfileManager::self()->activeProfile());
            
            // Get rid of the 'homes' share
            if (share->isHomesShare())
            {
              options->setUrl(share->homeUrl());
            }
          }
          else
          {
            // In case the custom options object for the host has been 
            // returned, change its internal network item, otherwise we
            // will change the host's custom options...
            options->setShare(share.data());
          }
        }
        
        break;
      }
      default:
      {
        break;
      }
    }
    
    if (options)
    {
      QPointer<Smb4KCustomOptionsDialog> dlg = new Smb4KCustomOptionsDialog(options, QApplication::activeWindow());
        
      if (dlg->exec() == QDialog::Accepted)
      {
        if (options->hasOptions())
        {
          addCustomOptions(options);
        }
        else
        {
          removeCustomOptions(options);
        }
      }
      else
      {
        resetCustomOptions();
      }
      
      delete dlg;
    }
  }
}


void Smb4KCustomOptionsManager::addCustomOptions(const OptionsPtr &options)
{
  if (options)
  {
    //
    // Check if options for the URL already exist
    // 
    OptionsPtr knownOptions = findOptions(options->url());
    
    if (knownOptions)
    {
      //
      // Update the options
      // 
      knownOptions->update(options.data());
    }
    else
    {
      // 
      // Add the options
      //
      if (options->profile().isEmpty())
      {
        options->setProfile(Smb4KProfileManager::self()->activeProfile());
      }
        
      d->options << options;
    }
    
    //
    // In case the options are defined for a host, propagate them 
    // to the options of shares belonging to that host. Overwrite 
    // the settings
    // 
    if (options->type() == Host)
    {
      for (const OptionsPtr &o : d->options)
      {
        if (o->type() == Share && o->hostName() == options->hostName() && o->workgroupName() == options->workgroupName())
        {
          o->setIpAddress(options->ipAddress());
#if !defined(SMB4K_UNSUPPORTED_PLATFORM)
          o->setUseUser(options->useUser());
          o->setUser(options->user());
          o->setUseGroup(options->useGroup());
          o->setGroup(options->group());
          o->setUseFileMode(options->useFileMode());
          o->setFileMode(options->fileMode());
          o->setUseDirectoryMode(options->useDirectoryMode());
          o->setDirectoryMode(options->directoryMode());
#endif
#if defined(Q_OS_LINUX)
          o->setCifsUnixExtensionsSupport(options->cifsUnixExtensionsSupport());
          o->setUseFileSystemPort(options->useFileSystemPort());
          o->setFileSystemPort(options->fileSystemPort());
          o->setUseSecurityMode(options->useSecurityMode());
          o->setSecurityMode(options->securityMode());
          o->setUseWriteAccess(options->useWriteAccess());
          o->setWriteAccess(options->writeAccess());
#endif
          o->setUseSmbPort(options->useSmbPort());
          o->setSmbPort(options->smbPort());
          o->setUseKerberos(options->useKerberos());
          o->setMACAddress(options->macAddress());
          o->setWOLSendBeforeNetworkScan(options->wolSendBeforeNetworkScan());
          o->setWOLSendBeforeMount(options->wolSendBeforeMount());
        }
      }
    }
    
    //
    // Write the custom options to the file
    //
    writeCustomOptions();
  }
}


void Smb4KCustomOptionsManager::removeCustomOptions(const OptionsPtr &options)
{
  if (options)
  {
    // 
    // Find the custom options and remove them
    //
    for (int i = 0; i < d->options.size(); ++i)
    {
      if ((!Smb4KSettings::useProfiles() || Smb4KProfileManager::self()->activeProfile() == d->options.at(i)->profile()) &&
          d->options.at(i)->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::StripTrailingSlash) == 
          options->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::StripTrailingSlash))
      {
        d->options.takeAt(i).clear();
        break;
      }
    }
    
    //
    // Write the custom options to the file
    //
    writeCustomOptions();
  }
}


QList<OptionsPtr> Smb4KCustomOptionsManager::wakeOnLanEntries() const
{
  QList<OptionsPtr> list;
  
  //
  // Get the Wake-On-LAN entries
  //
  for (const OptionsPtr &options : d->options)
  {
    if (!options->macAddress().isEmpty() && (options->wolSendBeforeNetworkScan() || options->wolSendBeforeMount()))
    {
      list << options;
    }
  }
  
  // 
  // Return them
  //
  return list;
}


void Smb4KCustomOptionsManager::resetCustomOptions()
{
  readCustomOptions();
}


void Smb4KCustomOptionsManager::migrateProfile(const QString& from, const QString& to)
{
  //
  // Replace the old with the new profile
  //
  for (const OptionsPtr &options : d->options)
  {
    if (options->profile() == from)
    {
      options->setProfile(to);
    }
  }
  
  //
  // Write all custom options to the file.
  //
  writeCustomOptions();
}


void Smb4KCustomOptionsManager::removeProfile(const QString& name)
{
  //
  // Remove all entries belonging to the profile
  // 
  QMutableListIterator<OptionsPtr> it(d->options);
  
  while (it.hasNext())
  {
    OptionsPtr options = it.next();
    
    if (QString::compare(options->profile(), name, Qt::CaseSensitive) == 0)
    {
      it.remove();
    }
  }
  
  // 
  // Write all custom options to the file.
  // 
  writeCustomOptions();
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomOptionsManager::slotAboutToQuit()
{
  writeCustomOptions();
}


