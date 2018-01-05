/***************************************************************************
    Manage custom options
                             -------------------
    begin                : Fr 29 Apr 2011
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptionsmanager_p.h"
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
#include "smb4kmountsettings_freebsd.h"
#endif

// Qt includes
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QCoreApplication>
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
  else
  {
    // Do nothing
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
      else
      {
        // Do nothing
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
  else
  {
    // Do nothing
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
      else
      {
        // Do nothing
      }      
    }
    else
    {
      // Do nothing
    }
    
    writeCustomOptions();
  }
  else
  {
    // Do nothing
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
    else
    {
      // Do nothing
    }  
  }
  
  //
  // Return relevant options
  //
  return remounts;
}


OptionsPtr Smb4KCustomOptionsManager::findOptions(const NetworkItemPtr &networkItem, bool exactMatch)
{
  //
  // The options that are to be returned
  //
  OptionsPtr options;
  
  //
  // Search the options for the network item
  //
  if (networkItem)
  {
    //
    // Get the relevant options
    //
    QList<OptionsPtr> optionsList = customOptions(false);
    
    //
    // Get the UNC and the IP address
    // 
    QString unc, hostUNC, ipAddress;
    
    switch (networkItem->type())
    {
      case Host:
      {
        HostPtr host = networkItem.staticCast<Smb4KHost>();
        
        if (host)
        {
          unc = host->unc();
          ipAddress = host->ip();
        }
        else
        {
          // Do nothing
        }
        
        break;
      }
      case Share:
      {
        SharePtr share = networkItem.staticCast<Smb4KShare>();
        
        if (share)
        {
          unc = share->isHomesShare() ? share->homeUNC() : share->unc();
          hostUNC = share->hostUNC();
          ipAddress = share->hostIP();
        }
        else
        {
          // Do nothing
        }
        
        break;
      }
      default:
      {
        break;
      }
    }
    
    //
    // Get the options
    //
    for (const OptionsPtr &o : optionsList)
    {
      if (QString::compare(unc, o->unc(), Qt::CaseInsensitive) == 0 || ipAddress == o->ip())
      {
        options = o;
        break;
      }
      else if (!exactMatch && o->type() == Host && (QString::compare(hostUNC, o->unc(), Qt::CaseInsensitive) == 0 || ipAddress == o->ip()))
      {
        options = o;
        // Do not break here. The exact match is always favored.
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  //
  // Return the options
  //
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
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
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
                else if (xmlReader.name() == "unc")
                {
                  options->setURL(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "ip")
                {
                  options->setIP(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "custom")
                {
                  while (!(xmlReader.isEndElement() && xmlReader.name() == "custom"))
                  {
                    xmlReader.readNext();
                    
                    if (xmlReader.isStartElement())
                    {
                      if (xmlReader.name() == "remount")
                      {
                        QString remount = xmlReader.readElementText();
  
                        if (QString::compare(remount, "once") == 0)
                        {
                          options->setRemount(Smb4KCustomOptions::RemountOnce);
                        }
                        else if (QString::compare(remount, "always") == 0)
                        {
                          options->setRemount(Smb4KCustomOptions::RemountAlways);
                        }
                        else if (QString::compare(remount, "never") == 0)
                        {
                          options->setRemount(Smb4KCustomOptions::RemountNever);
                        }
                        else
                        {
                          options->setRemount(Smb4KCustomOptions::UndefinedRemount);
                        }
                      }
                      else if (xmlReader.name() == "smb_port")
                      {
                        options->setSMBPort(xmlReader.readElementText().toInt());
                      }
#if defined(Q_OS_LINUX)
                      else if (xmlReader.name() == "filesystem_port")
                      {
                        options->setFileSystemPort(xmlReader.readElementText().toInt());
                      }
                      else if (xmlReader.name() == "security_mode")
                      {
                        QString security_mode = xmlReader.readElementText();
                          
                        if (QString::compare(security_mode, "none") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::NoSecurityMode);
                        }
                        else if (QString::compare(security_mode, "krb5") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Krb5);
                        }
                        else if (QString::compare(security_mode, "krb5i") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Krb5i);
                        }                        
                        else if (QString::compare(security_mode, "ntlm") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Ntlm);
                        }
                        else if (QString::compare(security_mode, "ntlmi") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Ntlmi);
                        }
                        else if (QString::compare(security_mode, "ntlmv2") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Ntlmv2);
                        }
                        else if (QString::compare(security_mode, "ntlmv2i") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Ntlmv2i);
                        }
                        else if (QString::compare(security_mode, "ntlmssp") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Ntlmssp);
                        }
                        else if (QString::compare(security_mode, "ntlmsspi") == 0)
                        {
                          options->setSecurityMode(Smb4KCustomOptions::Ntlmsspi);
                        }
                        else
                        {
                          options->setSecurityMode(Smb4KCustomOptions::UndefinedSecurityMode);
                        }
                      }
                      else if (xmlReader.name() == "write_access")
                      {
                        QString write_access = xmlReader.readElementText();
  
                        if (QString::compare(write_access, "true") == 0)
                        {
                          options->setWriteAccess(Smb4KCustomOptions::ReadWrite);
                        }
                        else if (QString::compare(write_access, "false") == 0)
                        {
                          options->setWriteAccess(Smb4KCustomOptions::ReadOnly);
                        }
                        else
                        {
                          options->setWriteAccess(Smb4KCustomOptions::UndefinedWriteAccess);
                        }
                      }
#endif
                      else if (xmlReader.name() == "kerberos")
                      {
                        QString kerberos = xmlReader.readElementText();
  
                        if (QString::compare(kerberos, "true") == 0)
                        {
                          options->setUseKerberos(Smb4KCustomOptions::UseKerberos);
                        }
                        else if (QString::compare(kerberos, "false") == 0)
                        {
                          options->setUseKerberos(Smb4KCustomOptions::NoKerberos);
                        }
                        else
                        {
                          options->setUseKerberos(Smb4KCustomOptions::UndefinedKerberos);
                        }
                      }
                      else if (xmlReader.name() == "uid")
                      {
                        options->setUser(KUser((K_UID)xmlReader.readElementText().toInt()));
                      }
                      else if (xmlReader.name() == "gid")
                      {
                        options->setGroup(KUserGroup((K_GID)xmlReader.readElementText().toInt()));
                      }
                      else if (xmlReader.name() == "mac_address")
                      {
                        options->setMACAddress(xmlReader.readElementText());
                      }
                      else if (xmlReader.name() == "wol_send_before_first_scan")
                      {
                        if (xmlReader.readElementText() == "true")
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
                        if (xmlReader.readElementText() == "true")
                        {
                          options->setWOLSendBeforeMount(true);
                        }
                        else
                        {
                          options->setWOLSendBeforeMount(false);
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
                }
                else
                {
                  // Do nothing
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
          else
          {
            // Do nothing
          }
        }
      }
      else
      {
        // Do nothing
      }
    }

    xmlFile.close();

    if (xmlReader.hasError())
    {
      Smb4KNotification::readingFileFailed(xmlFile, xmlReader.errorString());
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (xmlFile.exists())
    {
      Smb4KNotification::openingFileFailed(xmlFile);
    }
    else
    {
      // Do nothing
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
        if (hasCustomOptions(options) || options->remount() == Smb4KCustomOptions::RemountOnce)
        {
          xmlWriter.writeStartElement("options");
          xmlWriter.writeAttribute("type", options->type() == Host ? "host" : "share");
          xmlWriter.writeAttribute("profile", options->profile());

          xmlWriter.writeTextElement("workgroup", options->workgroupName());
          xmlWriter.writeTextElement("unc", options->unc());
          xmlWriter.writeTextElement("ip", options->ip());
          
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
            else
            {
              // Do nothing
            }
          }

          xmlWriter.writeEndElement();
          xmlWriter.writeEndElement();
        }
        else
        {
          // Do nothing
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
    if (hasCustomOptions(o) || (!optionsOnly && o->remount() == Smb4KCustomOptions::RemountOnce))
    {
      if (Smb4KSettings::useProfiles() && o->profile() != Smb4KProfileManager::self()->activeProfile())
      {
        continue;
      }
      else
      {
        // Do nothing
      }
      
      options << o;
    }
    else
    {
      // Do nothing
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
    else
    {
      // Do nothing
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
      else
      {
        // Do nothing
      }
      
      if (hasCustomOptions(options) || options->remount() == Smb4KCustomOptions::RemountOnce)
      {
        d->options << options;
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  writeCustomOptions();
}


void Smb4KCustomOptionsManager::openCustomOptionsDialog(const NetworkItemPtr &item, QWidget *parent)
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
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
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
            if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true, parent))
            {
              return;
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
          
          options = findOptions(share);
          
          if (!options)
          {
            options = OptionsPtr(new Smb4KCustomOptions(share.data()));
            options->setProfile(Smb4KProfileManager::self()->activeProfile());
            
            // Get rid of the 'homes' share
            if (share->isHomesShare())
            {
              options->setURL(share->homeURL());
            }
            else
            {
              // Do nothing
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
        else
        {
          // Do nothing
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
      QPointer<Smb4KCustomOptionsDialog> dlg = new Smb4KCustomOptionsDialog(options, parent);
        
      if (dlg->exec() == QDialog::Accepted)
      {
        if (hasCustomOptions(options))
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
      else
      {
        // Do nothing
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
        if (o->type() == Share && 
            QString::compare(o->hostName(), options->hostName(), Qt::CaseInsensitive) == 0 &&
            QString::compare(o->workgroupName(), options->workgroupName(), Qt::CaseInsensitive) == 0)
        {
          // Do not use Smb4KCustomOptions::update() here, because that
          // would overwrite more options than wanted.
          o->setSMBPort(options->smbPort());
#if defined(Q_OS_LINUX)
          o->setFileSystemPort(options->fileSystemPort());
          o->setSecurityMode(options->securityMode());
          o->setWriteAccess(options->writeAccess());
#endif
          o->setUser(options->user());
          o->setGroup(options->group());
          o->setUseKerberos(options->useKerberos());
          o->setMACAddress(options->macAddress());
          o->setWOLSendBeforeNetworkScan(options->wolSendBeforeNetworkScan());
          o->setWOLSendBeforeMount(options->wolSendBeforeMount());   
        }
        else
        {
          // Do nothing
        }
      }
    }
    else
    {
      // Do nothing
    }
    
    //
    // Write the custom options to the file
    //
    writeCustomOptions();
  }
  else
  {
    // Do nothing
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
      else
      {
        // Do nothing
      }
    }
    
    //
    // Write the custom options to the file
    //
    writeCustomOptions();
  }
  else
  {
    // Do nothing
  }
}


#if defined(Q_OS_LINUX)
//
// Linux
//
bool Smb4KCustomOptionsManager::hasCustomOptions(const OptionsPtr &options)
{
  if (options)
  {
    // Check if there are custom options defined.
    // Checks are performed against an empty and a default custom
    // options object. Default means that the values of the global
    // settings are honored.
    Smb4KCustomOptions empty_options, default_options;

    // Set up the default options
    default_options.setSMBPort(Smb4KSettings::remoteSMBPort());

    default_options.setFileSystemPort(Smb4KMountSettings::remoteFileSystemPort());

    switch (Smb4KMountSettings::securityMode())
    {
      case Smb4KMountSettings::EnumSecurityMode::None:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::NoSecurityMode);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Krb5);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Krb5i:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Krb5i);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlm:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Ntlm);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmi:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Ntlmi);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Ntlmv2);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Ntlmv2i);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmssp:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Ntlmssp);
        break;
      }
      case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::Ntlmsspi);
        break;
      }
      default:
      {
        default_options.setSecurityMode(Smb4KCustomOptions::UndefinedSecurityMode);
        break;
      }
    }

    switch (Smb4KMountSettings::writeAccess())
    {
      case Smb4KMountSettings::EnumWriteAccess::ReadWrite:
      {
        default_options.setWriteAccess(Smb4KCustomOptions::ReadWrite);
        break;
      }
      case Smb4KMountSettings::EnumWriteAccess::ReadOnly:
      {
        default_options.setWriteAccess(Smb4KCustomOptions::ReadOnly);
        break;
      }
      default:
      {
        default_options.setWriteAccess(Smb4KCustomOptions::UndefinedWriteAccess);
        break;
      }
    }

    if (Smb4KSettings::useKerberos())
    {
      default_options.setUseKerberos(Smb4KCustomOptions::UseKerberos);
    }
    else
    {
      default_options.setUseKerberos(Smb4KCustomOptions::NoKerberos);
    }

    default_options.setUser(KUser((K_UID)Smb4KMountSettings::userID().toInt()));
    default_options.setGroup(KUserGroup((K_GID)Smb4KMountSettings::groupID().toInt()));

    // NOTE: WOL features and remounting do not have default values.
    //
    // Do the actual checks
    //

    // Remounting
    if (options->remount() == Smb4KCustomOptions::RemountAlways)
    {
      return true;
    }
    else
    {
      // Do nothing
    }    

    // SMB port
    if (empty_options.smbPort() != options->smbPort() && 
        default_options.smbPort() != options->smbPort())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // File system port (used for mounting)
    if (empty_options.fileSystemPort() != options->fileSystemPort() && 
        default_options.fileSystemPort() != options->fileSystemPort())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Security mode
    if (empty_options.securityMode() != options->securityMode() &&
        default_options.securityMode() != options->securityMode())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Write access
    if (empty_options.writeAccess() != options->writeAccess() &&
        default_options.writeAccess() != options->writeAccess())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Kerberos
    if (empty_options.useKerberos() != options->useKerberos() &&
        default_options.useKerberos() != options->useKerberos())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // User / UID
    if (empty_options.user().userId() != options->user().userId() &&
        default_options.user().userId() != options->user().userId())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Group / GID
    if (empty_options.group().groupId() != options->group().groupId() &&
        default_options.group().groupId() != options->group().groupId())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // MAC address
    if (QString::compare(empty_options.macAddress(), options->macAddress()) != 0 &&
        QString::compare(default_options.macAddress(), options->macAddress()) != 0)
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Send before first scan
    if (empty_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan() &&
        default_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Send before mount
    if (empty_options.wolSendBeforeMount() != options->wolSendBeforeMount() &&
        default_options.wolSendBeforeMount() != options->wolSendBeforeMount())
    {
      return true;
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

  return false;
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD or NetBSD
//
bool Smb4KCustomOptionsManager::hasCustomOptions(const OptionsPtr &options)
{
  if (options)
  {
    // Check if there are custom options defined.
    // Checks are performed against an empty and a default custom
    // options object. Default means that the values of the global
    // settings are honored.
    Smb4KCustomOptions empty_options, default_options;

    // Set up the default options
    default_options.setSMBPort(Smb4KSettings::remoteSMBPort());

    if (Smb4KSettings::useKerberos())
    {
      default_options.setUseKerberos(Smb4KCustomOptions::UseKerberos);
    }
    else
    {
      default_options.setUseKerberos(Smb4KCustomOptions::NoKerberos);
    }

    default_options.setUser(KUser((K_UID)Smb4KMountSettings::userID().toInt()));
    default_options.setGroup(KUserGroup((K_GID)Smb4KMountSettings::groupID().toInt()));

    // NOTE: WOL features and remounting do not have default values.
    //
    // Do the actual checks
    //

    // Remounting
    if (options->remount() == Smb4KCustomOptions::RemountAlways)
    {
      return true;
    }
    else
    {
      // Do nothing
    }    

    // SMB port
    if (empty_options.smbPort() != options->smbPort() && 
        default_options.smbPort() != options->smbPort())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Kerberos
    if (empty_options.useKerberos() != options->useKerberos() &&
        default_options.useKerberos() != options->useKerberos())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // User / UID
    if (empty_options.user().userId() != options->user().userId() &&
        default_options.user().userId() != options->user().userId())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Group / GID
    if (empty_options.group().groupId() != options->group().groupId() &&
        default_options.group().groupId() != options->group().groupId())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // MAC address
    if (QString::compare(empty_options.macAddress(), options->macAddress()) != 0 &&
        QString::compare(default_options.macAddress(), options->macAddress()) != 0)
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Send before first scan
    if (empty_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan() &&
        default_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Send before mount
    if (empty_options.wolSendBeforeMount() != options->wolSendBeforeMount() &&
        default_options.wolSendBeforeMount() != options->wolSendBeforeMount())
    {
      return true;
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

  return false;
}
#else
//
// Generic (without mount options)
//
bool Smb4KCustomOptionsManager::hasCustomOptions(const OptionsPtr &options)
{
  if (options)
  {
    // Check if there are custom options defined.
    // Checks are performed against an empty and a default custom
    // options object. Default means that the values of the global
    // settings are honored.
    Smb4KCustomOptions empty_options, default_options;

    // Set up the default options
    default_options.setSMBPort(Smb4KSettings::remoteSMBPort());

    if (Smb4KSettings::useKerberos())
    {
      default_options.setUseKerberos(Smb4KCustomOptions::UseKerberos);
    }
    else
    {
      default_options.setUseKerberos(Smb4KCustomOptions::NoKerberos);
    }

    // NOTE: WOL features and remounting do not have default values.
    //
    // Do the actual checks
    //

    // Remounting
    if (options->remount() == Smb4KCustomOptions::RemountAlways)
    {
      return true;
    }
    else
    {
      // Do nothing
    }    

    // SMB port
    if (empty_options.smbPort() != options->smbPort() && 
        default_options.smbPort() != options->smbPort())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Kerberos
    if (empty_options.useKerberos() != options->useKerberos() &&
        default_options.useKerberos() != options->useKerberos())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // MAC address
    if (QString::compare(empty_options.macAddress(), options->macAddress()) != 0 &&
        QString::compare(default_options.macAddress(), options->macAddress()) != 0)
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Send before first scan
    if (empty_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan() &&
        default_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan())
    {
      return true;
    }
    else
    {
      // Do nothing
    }

    // Send before mount
    if (empty_options.wolSendBeforeMount() != options->wolSendBeforeMount() &&
        default_options.wolSendBeforeMount() != options->wolSendBeforeMount())
    {
      return true;
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

  return false;
}
#endif



QList<OptionsPtr> Smb4KCustomOptionsManager::wolEntries() const
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
    else
    {
      // Do nothing
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
    else
    {
      // Do nothing
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
    else
    {
      // Do nothing
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


