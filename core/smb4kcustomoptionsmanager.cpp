/***************************************************************************
    smb4kcustomoptionsmanager - Manage custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2014 by Alexander Reinholdt
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

// Qt includes
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>

// KDE includes
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KCustomOptionsManagerStatic, p );


Smb4KCustomOptionsManager::Smb4KCustomOptionsManager( QObject *parent )
: QObject( parent ), d( new Smb4KCustomOptionsManagerPrivate )
{
  // We need the directory.
  QString dir = KGlobal::dirs()->locateLocal( "data", "smb4k", KGlobal::mainComponent() );

  if ( !KGlobal::dirs()->exists( dir ) )
  {
    KGlobal::dirs()->makeDir( dir );
  }
  
  readCustomOptions(&d->options, false);
  
  // Connections
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
  connect(Smb4KProfileManager::self(), SIGNAL(settingsChanged()), SLOT(slotProfileSettingsChanged()));
}


Smb4KCustomOptionsManager::~Smb4KCustomOptionsManager()
{
}


Smb4KCustomOptionsManager *Smb4KCustomOptionsManager::self()
{
  return &p->instance;
}


void Smb4KCustomOptionsManager::addRemount( Smb4KShare *share, bool always )
{
  Q_ASSERT( share );
  
  if ( share )
  {
    Smb4KCustomOptions *options = NULL;
    
    if ( (options = findOptions( share, true )) )
    {
      // If the options are already in the list, check if the share is
      // always to be remounted. If so, ignore the 'always' argument
      // and leave that option untouched.
      if ( options->remount() != Smb4KCustomOptions::RemountAlways )
      {
        options->setRemount( always ? Smb4KCustomOptions::RemountAlways : Smb4KCustomOptions::RemountOnce );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      options = new Smb4KCustomOptions( share );
      options->setProfile(Smb4KProfileManager::self()->activeProfile());
      options->setRemount(always ? Smb4KCustomOptions::RemountAlways : Smb4KCustomOptions::RemountOnce);
      d->options << options;
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KCustomOptionsManager::removeRemount( Smb4KShare *share, bool force )
{
  Q_ASSERT( share );
  
  if ( share )
  {
    Smb4KCustomOptions *options = NULL;
    
    if ( (options = findOptions( share, true )) )
    {
      if ( options->remount() == Smb4KCustomOptions::RemountOnce )
      {
        options->setRemount( Smb4KCustomOptions::RemountNever );
      }
      else if ( options->remount() == Smb4KCustomOptions::RemountAlways && force )
      {
        options->setRemount( Smb4KCustomOptions::RemountNever );
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


void Smb4KCustomOptionsManager::clearRemounts( bool force )
{
  for ( int i = 0; i < d->options.size(); ++i )
  {
    if ( d->options.at(i)->type() == Share )
    {
      if ( d->options.at(i)->remount() == Smb4KCustomOptions::RemountOnce )
      {
        d->options[i]->setRemount( Smb4KCustomOptions::RemountNever );
      }
      else if ( d->options.at(i)->remount() == Smb4KCustomOptions::RemountAlways && force )
      {
        d->options[i]->setRemount( Smb4KCustomOptions::RemountNever );
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


QList<Smb4KCustomOptions *> Smb4KCustomOptionsManager::sharesToRemount()
{
  QList<Smb4KCustomOptions *> remounts;
  
  for ( int i = 0; i < d->options.size(); ++i )
  {
    if ( d->options.at( i )->remount() == Smb4KCustomOptions::RemountOnce )
    {
      remounts << d->options[i];
    }
    else if ( d->options.at( i )->remount() == Smb4KCustomOptions::RemountAlways )
    {
      remounts << d->options[i];
    }
    else
    {
      // Do nothing
    }
  }
  
  return remounts;
}


Smb4KCustomOptions *Smb4KCustomOptionsManager::findOptions( Smb4KBasicNetworkItem *networkItem, bool exactMatch )
{
  Q_ASSERT( networkItem );
  
  Smb4KCustomOptions *options = NULL;

  switch ( networkItem->type() )
  {
    case Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );

      if ( host )
      {
        for ( int i = 0; i < d->options.size(); ++i )
        {
          if ( d->options.at( i )->type() == Host )
          {
            if ( QString::compare( d->options.at( i )->unc(), host->unc(), Qt::CaseInsensitive ) == 0 ||
                 QString::compare( d->options.at( i )->ip(), host->ip() ) == 0 )
            {
              options = d->options[i];
              break;
            }
            else
            {
              continue;
            }
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
      break;
    }
    case Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

      if ( share )
      {
        for ( int i = 0; i < d->options.size(); ++i )
        {
          if ( d->options.at( i )->type() == Share )
          {
            if ( QString::compare( d->options.at( i )->unc(), share->unc(), Qt::CaseInsensitive ) == 0 ||
                 QString::compare( d->options.at( i )->unc(), share->homeUNC(), Qt::CaseInsensitive ) == 0 )
            {
              options = d->options[i];
              break;
            }
            else
            {
              continue;
            }
          }
          else if ( d->options.at( i )->type() == Host && !exactMatch )
          {
            // FIXME: This might be problematic if the user uses a DHCP server.
            if ( QString::compare( d->options.at( i )->unc(), share->hostUNC(), Qt::CaseInsensitive ) == 0 ||
                 QString::compare( d->options.at( i )->ip(), share->hostIP() ) == 0 )
            {
              options = d->options[i];
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
  
  return options;
}


Smb4KCustomOptions* Smb4KCustomOptionsManager::findOptions( const KUrl &url )
{
  Smb4KCustomOptions *options = NULL;
  
  if ( url.isValid() && QString::compare( url.protocol(), "smb" ) == 0 )
  {
    for ( int i = 0; i < d->options.size(); ++i )
    {
      if ( QString::compare( d->options.at( i )->url().host(), url.host(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( d->options.at( i )->url().path(), url.path(), Qt::CaseInsensitive ) == 0 )
      {
        options = d->options[i];
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

  return options;
}


void Smb4KCustomOptionsManager::readCustomOptions(QList<Smb4KCustomOptions *> *optionsList, bool allOptions)
{
  // Locate the XML file.
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/custom_options.xml", KGlobal::mainComponent() ) );

  if ( xmlFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QXmlStreamReader xmlReader( &xmlFile );

    while ( !xmlReader.atEnd() )
    {
      xmlReader.readNext();

      if ( xmlReader.isStartElement() )
      {
        if ( xmlReader.name() == "custom_options" && 
             (xmlReader.attributes().value( "version" ) != "1.2" &&
              xmlReader.attributes().value( "version" ) != "2.0") )
        {
          xmlReader.raiseError( i18n( "The format of %1 is not supported.", xmlFile.fileName() ) );
          break;
        }
        else
        {
          if ( xmlReader.name() == "options" )
          {
            QString profile = xmlReader.attributes().value("profile").toString();
            
            if (allOptions || QString::compare(Smb4KProfileManager::self()->activeProfile(), profile, Qt::CaseSensitive) == 0)
            {
              Smb4KCustomOptions *options = new Smb4KCustomOptions();
              options->setProfile(profile);
              
              if (QString::compare(xmlReader.attributes().value("type").toString(), "host", Qt::CaseInsensitive) == 0)
              {
                options->setHost(new Smb4KHost());
              }
              else
              {
                options->setShare(new Smb4KShare());
              }
              
              while ( !(xmlReader.isEndElement() && xmlReader.name() == "options") )
              {
                xmlReader.readNext();
  
                if ( xmlReader.isStartElement() )
                {
                  if ( xmlReader.name() == "workgroup" )
                  {
                    options->setWorkgroupName( xmlReader.readElementText() );
                  }
                  else if ( xmlReader.name() == "unc" )
                  {
                    options->setURL( xmlReader.readElementText() );
                  }
                  else if ( xmlReader.name() == "ip" )
                  {
                    options->setIP( xmlReader.readElementText() );
                  }
                  else if ( xmlReader.name() == "custom" )
                  {
                    while ( !(xmlReader.isEndElement() && xmlReader.name() == "custom") )
                    {
                      xmlReader.readNext();
  
                      if ( xmlReader.isStartElement() )
                      {
                        if ( xmlReader.name() == "remount" )
                        {
                          QString remount = xmlReader.readElementText();
  
                          // FIXME: Remove the entries "true" and "false" with the next
                          // revision (i.e. Smb4K > 1.2).
                          if ( QString::compare( remount, "once" ) == 0 )
                          {
                            options->setRemount( Smb4KCustomOptions::RemountOnce );
                          }
                          else if ( QString::compare( remount, "always" ) == 0 )
                          {
                            options->setRemount( Smb4KCustomOptions::RemountAlways );
                          }
                          else if ( QString::compare( remount, "never" ) == 0 )
                          {
                            options->setRemount( Smb4KCustomOptions::RemountNever );
                          }
                          else if ( QString::compare( remount, "true" ) == 0 )
                          {
                            options->setRemount( Smb4KCustomOptions::RemountOnce );
                          }
                          else if ( QString::compare( remount, "false" ) == 0 )
                          {
                            options->setRemount( Smb4KCustomOptions::RemountNever );
                          }
                          else
                          {
                            options->setRemount( Smb4KCustomOptions::UndefinedRemount );
                          }
                        }
                        else if ( xmlReader.name() == "smb_port" )
                        {
                          options->setSMBPort( xmlReader.readElementText().toInt() );
                        }
                        else if ( xmlReader.name() == "protocol" )
                        {
                          QString protocol = xmlReader.readElementText();
  
                          if ( QString::compare( protocol, "auto" ) == 0 )
                          {
                            options->setProtocolHint( Smb4KCustomOptions::Automatic );
                          }
                          else if ( QString::compare( protocol, "rpc" ) == 0 )
                          {
                            options->setProtocolHint( Smb4KCustomOptions::RPC );
                          }
                          else if ( QString::compare( protocol, "rap" ) == 0 )
                          {
                            options->setProtocolHint( Smb4KCustomOptions::RAP );
                          }
                          else if ( QString::compare( protocol, "ads" ) == 0 )
                          {
                            options->setProtocolHint( Smb4KCustomOptions::ADS );
                          }
                          else
                          {
                            options->setProtocolHint( Smb4KCustomOptions::UndefinedProtocolHint );
                          }
                        }
#ifdef Q_OS_LINUX
                        else if ( xmlReader.name() == "filesystem_port" )
                        {
                          options->setFileSystemPort( xmlReader.readElementText().toInt() );
                        }
                        else if ( xmlReader.name() == "write_access" )
                        {
                          QString write_access = xmlReader.readElementText();
  
                          if ( QString::compare( write_access, "true" ) == 0 )
                          {
                            options->setWriteAccess( Smb4KCustomOptions::ReadWrite );
                          }
                          else if ( QString::compare( write_access, "false" ) == 0 )
                          {
                            options->setWriteAccess( Smb4KCustomOptions::ReadOnly );
                          }
                          else
                          {
                            options->setWriteAccess( Smb4KCustomOptions::UndefinedWriteAccess );
                          }
                        }
                        else if ( xmlReader.name() == "security_mode" )
                        {
                          QString security_mode = xmlReader.readElementText();
                          
                          if ( QString::compare( security_mode, "none" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::NoSecurityMode );
                          }
                          else if ( QString::compare( security_mode, "krb5" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Krb5 );
                          }
                          else if ( QString::compare( security_mode, "krb5i" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Krb5i );
                          }                        
                          else if ( QString::compare( security_mode, "ntlm" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Ntlm );
                          }
                          else if ( QString::compare( security_mode, "ntlmi" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Ntlmi );
                          }
                          else if ( QString::compare( security_mode, "ntlmv2" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Ntlmv2 );
                          }
                          else if ( QString::compare( security_mode, "ntlmv2i" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Ntlmv2i );
                          }
                          else if ( QString::compare( security_mode, "ntlmssp" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Ntlmssp );
                          }
                          else if ( QString::compare( security_mode, "ntlmsspi" ) == 0 )
                          {
                            options->setSecurityMode( Smb4KCustomOptions::Ntlmsspi );
                          }
                          else
                          {
                            options->setSecurityMode( Smb4KCustomOptions::UndefinedSecurityMode );
                          }
                        }
#endif
                        else if ( xmlReader.name() == "kerberos" )
                        {
                          QString kerberos = xmlReader.readElementText();
  
                          if ( QString::compare( kerberos, "true" ) == 0 )
                          {
                            options->setUseKerberos( Smb4KCustomOptions::UseKerberos );
                          }
                          else if ( QString::compare( kerberos, "false" ) == 0 )
                          {
                            options->setUseKerberos( Smb4KCustomOptions::NoKerberos );
                          }
                          else
                          {
                            options->setUseKerberos( Smb4KCustomOptions::UndefinedKerberos );
                          }
                        }
                        else if ( xmlReader.name() == "uid" )
                        {
                          options->setUID( (K_UID)xmlReader.readElementText().toInt() );
                        }
                        else if ( xmlReader.name() == "gid" )
                        {
                          options->setGID( (K_GID)xmlReader.readElementText().toInt() );
                        }
                        else if ( xmlReader.name() == "mac_address" )
                        {
                          options->setMACAddress( xmlReader.readElementText() );
                        }
                        else if ( xmlReader.name() == "wol_send_before_first_scan" )
                        {
                          if ( xmlReader.readElementText() == "true" )
                          {
                            options->setWOLSendBeforeNetworkScan( true );
                          }
                          else
                          {
                            options->setWOLSendBeforeNetworkScan( false );
                          }
                        }
                        else if ( xmlReader.name() == "wol_send_before_mount" )
                        {
                          if ( xmlReader.readElementText() == "true" )
                          {
                            options->setWOLSendBeforeMount( true );
                          }
                          else
                          {
                            options->setWOLSendBeforeMount( false );
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
              
              *optionsList << options;             
            }
            else
            {
              continue;
            }
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        continue;
      }
    }

    xmlFile.close();

    if ( xmlReader.hasError() )
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
    if ( xmlFile.exists() )
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
  QList<Smb4KCustomOptions *> allOptions;
  
  // First read all entries. Ignore all those that have 
  // representatives in list.
  readCustomOptions(&allOptions, true);
  
  QMutableListIterator<Smb4KCustomOptions *> it(allOptions);
  
  while (it.hasNext())
  {
    Smb4KCustomOptions *options = it.next();
    
    for (int i = 0; i < d->options.size(); ++i)
    {
      if (QString::compare(options->workgroupName(), d->options.at(i)->workgroupName()) == 0 &&
          QString::compare(options->unc(), d->options.at(i)->unc()) == 0 &&
          QString::compare(options->profile(), d->options.at(i)->profile()) == 0)
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
    }
  }
  
  for (int i = 0; i < d->options.size(); ++i)
  {
    allOptions << new Smb4KCustomOptions(*d->options[i]);
  } 
  
  QFile xmlFile(KGlobal::dirs()->locateLocal("data", "smb4k/custom_options.xml", KGlobal::mainComponent()));

  if (!allOptions.isEmpty())
  {
    if (xmlFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
      QXmlStreamWriter xmlWriter(&xmlFile);
      xmlWriter.setAutoFormatting(true);
      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement("custom_options");
      xmlWriter.writeAttribute("version", "2.0");

      for (int i = 0; i < allOptions.size(); ++i)
      {
        Smb4KCustomOptions *options = allOptions[i];
        
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
          continue;
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
  
  while (!allOptions.isEmpty())
  {
    delete allOptions.takeFirst();
  }
}


const QList<Smb4KCustomOptions *> Smb4KCustomOptionsManager::customOptions( bool optionsOnly )
{
  QList<Smb4KCustomOptions *> custom_options;
  
  for ( int i = 0; i < d->options.size(); ++i )
  {
    Smb4KCustomOptions *options = d->options[i];
    
    if ( hasCustomOptions( options ) || 
         (!optionsOnly && options->remount() == Smb4KCustomOptions::RemountOnce) )
    {
      custom_options << options;
    }
    else
    {
      // Do nothing
    }
  }
  
  return custom_options;
}


void Smb4KCustomOptionsManager::replaceCustomOptions( const QList<Smb4KCustomOptions*> &options_list )
{
  while ( !d->options.isEmpty() )
  {
    delete d->options.takeFirst();
  }
  
  if ( !options_list.isEmpty() )
  {
    for ( int i = 0; i < options_list.size(); ++i )
    {
      Smb4KCustomOptions *options = options_list[i];
      
      if (options->profile().isEmpty())
      {
        options->setProfile(Smb4KProfileManager::self()->activeProfile());
      }
      else
      {
        // Do nothing
      }
      
      if ( hasCustomOptions( options ) )
      {
        d->options << new Smb4KCustomOptions( *options );
      }
      else if ( options->remount() == Smb4KCustomOptions::RemountOnce )
      {
        d->options << new Smb4KCustomOptions( *options );
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


void Smb4KCustomOptionsManager::openCustomOptionsDialog( Smb4KBasicNetworkItem *item, QWidget *parent )
{
  Q_ASSERT( item );
  
  Smb4KCustomOptions *options = NULL;
  bool delete_options = false;
  
  switch ( item->type() )
  {
    case Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( item );
      
      if ( host )
      {
        options = findOptions( host );
      
        if ( !options )
        {
          options = new Smb4KCustomOptions( host );
          options->setProfile(Smb4KProfileManager::self()->activeProfile());
          delete_options = true;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        return;
      }
      break;
    }
    case Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>( item );
      
      if ( share )
      {
        if ( share->isPrinter() )
        {
          return;
        }
        else
        {
          // Do nothing
        }
        
        if ( share->isHomesShare() )
        {
          Smb4KHomesSharesHandler::self()->specifyUser( share, true, parent );
        }
        else
        {
          // Do nothing
        }
        
        options = findOptions( share );
        
        if ( !options )
        {
          options = new Smb4KCustomOptions( share );
          options->setProfile(Smb4KProfileManager::self()->activeProfile());
          delete_options = true;
          
          // Get rid of the 'homes' share
          if ( share->isHomesShare() )
          {
            options->setURL( share->homeURL() );
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
          options->setShare( share );
        }
      }
      else
      {
        return;
      }
      break;
    }
    default:
    {
      break;
    }
  }
  
  QPointer<Smb4KCustomOptionsDialog> dlg = new Smb4KCustomOptionsDialog( options, parent );
    
  if ( dlg->exec() == KDialog::Accepted )
  {
    if ( hasCustomOptions( options ) )
    {
      addCustomOptions( options );
    }
    else
    {
      removeCustomOptions( options );
    }
      
    writeCustomOptions();
  }
  else
  {
    // Do nothing
  }
  
  delete dlg;
  
  // Delete the options object if necessary. 
  if ( delete_options )
  {
    delete options;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KCustomOptionsManager::addCustomOptions( Smb4KCustomOptions *options )
{
  Q_ASSERT( options );
  
  // Add the custom options. Check if there already is an entry with the
  // same URL and modify it if found.
  // If the incoming options are those for a host, propagate the host specific
  // changes to its shares as well.
  
  Smb4KCustomOptions *known_options = findOptions( options->url() );
  
  if ( !known_options )
  {
    Smb4KCustomOptions *o = new Smb4KCustomOptions(*options);
    
    if (o->profile().isEmpty())
    {
      o->setProfile(Smb4KProfileManager::self()->activeProfile());
    }
    else
    {
      // Do nothing
    }
    
    d->options << o;
  }
  else
  {
    if ( known_options != options && !known_options->equals( options ) )
    {
      known_options->setSMBPort( options->smbPort() );
#ifndef Q_OS_FREEBSD
      known_options->setFileSystemPort( options->fileSystemPort() );
      known_options->setWriteAccess( options->writeAccess() );
      known_options->setSecurityMode( options->securityMode() );
#endif
      known_options->setProtocolHint( options->protocolHint() );
      known_options->setUID( options->uid() );
      known_options->setGID( options->gid() );
      known_options->setUseKerberos( options->useKerberos() );
      known_options->setMACAddress( options->macAddress() );
      known_options->setWOLSendBeforeNetworkScan( options->wolSendBeforeNetworkScan() );
      known_options->setWOLSendBeforeMount( options->wolSendBeforeMount() );
    }
    else
    {
      // Do nothing
    }
    
    if ( known_options->type() == Host )
    {
      for ( int i = 0; i < d->options.size(); ++i )
      {
        if ( d->options.at( i )->type() == Share &&
             QString::compare( d->options.at( i )->hostName() , options->hostName(), Qt::CaseInsensitive ) == 0 &&
             QString::compare( d->options.at( i )->workgroupName() , options->workgroupName(), Qt::CaseInsensitive ) == 0 )
        {
          // Propagate the options to the shared resources of the host.
          // They overwrite the ones defined for the shares.
          d->options[i]->setSMBPort( options->smbPort() );
#ifndef Q_OS_FREEBSD
          d->options[i]->setFileSystemPort( options->fileSystemPort() );
          d->options[i]->setWriteAccess( options->writeAccess() );
          d->options[i]->setSecurityMode( options->securityMode() );
#endif
          d->options[i]->setProtocolHint( options->protocolHint() );
          d->options[i]->setUID( options->uid() );
          d->options[i]->setGID( options->gid() );
          d->options[i]->setUseKerberos( options->useKerberos() );
          d->options[i]->setMACAddress( options->macAddress() );
          d->options[i]->setWOLSendBeforeNetworkScan( options->wolSendBeforeNetworkScan() );
          d->options[i]->setWOLSendBeforeMount( options->wolSendBeforeMount() );
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
}


void Smb4KCustomOptionsManager::removeCustomOptions( Smb4KCustomOptions *options )
{
  Q_ASSERT( options );
  
  Smb4KCustomOptions *known_options = findOptions( options->url() );
  
  if ( known_options )
  {
    int index = d->options.indexOf( known_options );
    
    if ( index != -1 )
    {
      delete d->options.takeAt( index );
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


bool Smb4KCustomOptionsManager::hasCustomOptions( Smb4KCustomOptions *options )
{
  Q_ASSERT( options );
  
  // Check if there are custom options defined.
  // Checks are performed against an empty and a default custom
  // options object. Default means that the values of the global
  // settings are honored.
  Smb4KCustomOptions empty_options, default_options;
  
  // Set up the default options
  default_options.setSMBPort( Smb4KSettings::remoteSMBPort() );
#ifndef Q_OS_FREEBSD
  default_options.setFileSystemPort( Smb4KSettings::remoteFileSystemPort() );
  
  switch ( Smb4KSettings::writeAccess() )
  {
    case Smb4KSettings::EnumWriteAccess::ReadWrite:
    {
      default_options.setWriteAccess( Smb4KCustomOptions::ReadWrite );
      break;
    }
    case Smb4KSettings::EnumWriteAccess::ReadOnly:
    {
      default_options.setWriteAccess( Smb4KCustomOptions::ReadOnly );
      break;
    }
    default:
    {
      default_options.setWriteAccess( Smb4KCustomOptions::UndefinedWriteAccess );
      break;
    }
  }
  
  switch ( Smb4KSettings::securityMode() )
  {
    case Smb4KSettings::EnumSecurityMode::None:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::NoSecurityMode );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Krb5 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Krb5i:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Krb5i );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlm:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Ntlm );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmi:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Ntlmi );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Ntlmv2 );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmv2i:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Ntlmv2i );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmssp:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Ntlmssp );
      break;
    }
    case Smb4KSettings::EnumSecurityMode::Ntlmsspi:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::Ntlmsspi );
      break;
    }
    default:
    {
      default_options.setSecurityMode( Smb4KCustomOptions::UndefinedSecurityMode );
      break;
    }
  }
#endif

  switch ( Smb4KSettings::protocolHint() )
  {
    case Smb4KSettings::EnumProtocolHint::Automatic:
    {
      default_options.setProtocolHint( Smb4KCustomOptions::Automatic );
      break;
    }
    case Smb4KSettings::EnumProtocolHint::RPC:
    {
      default_options.setProtocolHint( Smb4KCustomOptions::RPC );
      break;
    }
    case Smb4KSettings::EnumProtocolHint::RAP:
    {
      default_options.setProtocolHint( Smb4KCustomOptions::RAP );
      break;
    }
    case Smb4KSettings::EnumProtocolHint::ADS:
    {
      default_options.setProtocolHint( Smb4KCustomOptions::ADS );
      break;
    }
    default:
    {
      default_options.setProtocolHint( Smb4KCustomOptions::UndefinedProtocolHint );
      break;
    }
  }
  
  if ( Smb4KSettings::useKerberos() )
  {
    default_options.setUseKerberos( Smb4KCustomOptions::UseKerberos );
  }
  else
  {
    default_options.setUseKerberos( Smb4KCustomOptions::NoKerberos );
  }
  
  default_options.setUID( (K_UID)Smb4KSettings::userID().toInt() );
  default_options.setGID( (K_GID)Smb4KSettings::groupID().toInt() );
  
  // NOTE: WOL features and remounting do not have default values.
  
  //
  // Do the actual check
  //
  
  if ( options->remount() == Smb4KCustomOptions::RemountAlways )
  {
    return true;
  }
  else
  {
    // Do nothing
  }    

  // SMB port
  if ( empty_options.smbPort() != options->smbPort() && 
       default_options.smbPort() != options->smbPort() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
#ifndef Q_OS_FREEBSD
  
  // File system port (used for mounting)
  if ( empty_options.fileSystemPort() != options->fileSystemPort() && 
       default_options.fileSystemPort() != options->fileSystemPort() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }

  // Write access
  if ( empty_options.writeAccess() != options->writeAccess() &&
       default_options.writeAccess() != options->writeAccess() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // Security mode
  if ( empty_options.securityMode() != options->securityMode() &&
       default_options.securityMode() != options->securityMode() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
#endif

  // Protocol hint
  if ( empty_options.protocolHint() != options->protocolHint() &&
       default_options.protocolHint() != options->protocolHint() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // Kerberos
  if ( empty_options.useKerberos() != options->useKerberos() &&
       default_options.useKerberos() != options->useKerberos() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // UID
  if ( empty_options.uid() != options->uid() &&
       default_options.uid() != options->uid() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // GID
  if ( empty_options.gid() != options->gid() &&
       default_options.gid() != options->gid() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // MAC address
  if ( QString::compare( empty_options.macAddress(), options->macAddress() ) != 0 &&
       QString::compare( default_options.macAddress(), options->macAddress() ) != 0 )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // Send before first scan
  if ( empty_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan() &&
       default_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // Send before mount
  if ( empty_options.wolSendBeforeMount() != options->wolSendBeforeMount() &&
       default_options.wolSendBeforeMount() != options->wolSendBeforeMount() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  return false;
}


QList<Smb4KCustomOptions *> Smb4KCustomOptionsManager::wolEntries() const
{
  QList<Smb4KCustomOptions *> list;
  
  for ( int i = 0; i < d->options.size(); ++i )
  {
    if ( !d->options.at( i )->macAddress().isEmpty() && 
         (d->options.at( i )->wolSendBeforeNetworkScan() || d->options.at( i )->wolSendBeforeMount()) )
    {
      list << d->options[i];
    }
    else
    {
      // Do nothing
    }
  }
  
  return list;
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomOptionsManager::slotAboutToQuit()
{
  writeCustomOptions();
}


void Smb4KCustomOptionsManager::slotProfileSettingsChanged()
{
  // Clear the list of custom options.
  while (!d->options.isEmpty())
  {
    delete d->options.takeFirst();
  }
  
  // Reload the list.
  readCustomOptions(&d->options, false);
}

#include "smb4kcustomoptionsmanager.moc"
