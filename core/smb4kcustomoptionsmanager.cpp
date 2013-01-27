/***************************************************************************
    smb4kcustomoptionsmanager - Manage custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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
  
  readCustomOptions();
  
  connect( QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()) );
}


Smb4KCustomOptionsManager::~Smb4KCustomOptionsManager()
{
}


Smb4KCustomOptionsManager *Smb4KCustomOptionsManager::self()
{
  return &p->instance;
}


void Smb4KCustomOptionsManager::addRemount( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  Smb4KCustomOptions *options = NULL;
  
  if ( (options = findOptions( share, true )) )
  {
    options->setRemount( Smb4KCustomOptions::DoRemount );
  }
  else
  {
    options = new Smb4KCustomOptions( share );
    options->setRemount( Smb4KCustomOptions::DoRemount );
    d->options << options;
  }
}


void Smb4KCustomOptionsManager::removeRemount( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  Smb4KCustomOptions *options = NULL;
  
  if ( (options = findOptions( share, true )) )
  {
    options->setRemount( Smb4KCustomOptions::NoRemount );
  }
  else
  {
    // Do nothing
  }  
}


void Smb4KCustomOptionsManager::clearRemounts()
{
  for ( int i = 0; i < d->options.size(); ++i )
  {
    if ( d->options.at( i )->type() == Smb4KCustomOptions::Share &&
         d->options.at( i )->remount() == Smb4KCustomOptions::DoRemount )
    {
      d->options[i]->setRemount( Smb4KCustomOptions::NoRemount );
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
    if ( d->options.at( i )->remount() == Smb4KCustomOptions::DoRemount )
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
    case Smb4KBasicNetworkItem::Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );

      if ( host )
      {
        for ( int i = 0; i < d->options.size(); ++i )
        {
          if ( d->options.at( i )->type() == Smb4KCustomOptions::Host )
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
    case Smb4KBasicNetworkItem::Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );

      if ( share )
      {
        for ( int i = 0; i < d->options.size(); ++i )
        {
          if ( d->options.at( i )->type() == Smb4KCustomOptions::Share )
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
          else if ( d->options.at( i )->type() == Smb4KCustomOptions::Host && !exactMatch )
          {
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


void Smb4KCustomOptionsManager::readCustomOptions()
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
        // FIXME: Drop version 1.0 with the next revision
        if ( xmlReader.name() == "custom_options" && 
             (xmlReader.attributes().value( "version" ) != "1.0" && 
              xmlReader.attributes().value( "version" ) != "1.1" &&
              xmlReader.attributes().value( "version" ) != "1.2") )
        {
          xmlReader.raiseError( i18n( "The format of %1 is not supported.", xmlFile.fileName() ) );
          break;
        }
        else
        {
          if ( xmlReader.name() == "options" )
          {
            Smb4KCustomOptions *options = new Smb4KCustomOptions();
            
            options->setProfile( xmlReader.attributes().value( "profile" ).toString() );
            
            if ( QString::compare( xmlReader.attributes().value( "type" ).toString(), "host", Qt::CaseInsensitive ) == 0 )
            {
              options->setHost( new Smb4KHost() );
            }
            else
            {
              options->setShare( new Smb4KShare() );
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

                        if ( QString::compare( remount, "true" ) == 0 )
                        {
                          options->setRemount( Smb4KCustomOptions::DoRemount );
                        }
                        else if ( QString::compare( remount, "false" ) == 0 )
                        {
                          options->setRemount( Smb4KCustomOptions::NoRemount );
                        }
                        else
                        {
                          options->setRemount( Smb4KCustomOptions::UndefinedRemount );
                        }
                      }
                      else if ( xmlReader.name() == "port" )
                      {
                        // FIXME: For backward compatibility. Remove with versions >> 1.0.x.
#ifndef Q_OS_FREEBSD
                        switch ( options->type() )
                        {
                          case Smb4KCustomOptions::Host:
                          {
                            options->setSMBPort( xmlReader.readElementText().toInt() );
                            break;
                          }
                          case Smb4KCustomOptions::Share:
                          {
                            options->setFileSystemPort( xmlReader.readElementText().toInt() );
                            break;
                          }
                          default:
                          {
                            break;
                          }
                        }
#else
                        options->setSMBPort( xmlReader.readElementText().toInt() );
#endif
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
#ifndef Q_OS_FREEBSD
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

            d->options << options;
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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->readingFileFailed( xmlFile, xmlReader.errorString() );
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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->openingFileFailed( xmlFile );
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KCustomOptionsManager::writeCustomOptions()
{
  // Clean up:
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/custom_options.xml", KGlobal::mainComponent() ) );

  if ( !d->options.isEmpty() )
  {
    if ( xmlFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QXmlStreamWriter xmlWriter( &xmlFile );
      xmlWriter.setAutoFormatting( true );
      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement( "custom_options" );
      xmlWriter.writeAttribute( "version", "1.2" );

      for ( int i = 0; i < d->options.size(); ++i )
      {
        Smb4KCustomOptions *options = d->options[i];
        
        if ( hasCustomOptions( options ) || options->remount() == Smb4KCustomOptions::DoRemount )
        {
          xmlWriter.writeStartElement( "options" );
          xmlWriter.writeAttribute( "type", options->type() == Smb4KCustomOptions::Host ? "host" : "share" );
          xmlWriter.writeAttribute( "profile", options->profile() );

          xmlWriter.writeTextElement( "workgroup", options->workgroupName() );
          xmlWriter.writeTextElement( "unc", options->unc() );
          xmlWriter.writeTextElement( "ip", options->ip() );

          xmlWriter.writeStartElement( "custom" );

          QMap<QString,QString> map = options->customOptions();
          QMapIterator<QString,QString> it( map );

          while ( it.hasNext() )
          {
            it.next();

            if ( !it.value().isEmpty() )
            {
              xmlWriter.writeTextElement( it.key(), it.value() );
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
      Smb4KNotification *notification = new Smb4KNotification();
      notification->openingFileFailed( xmlFile );
      return;
    }
  }
  else
  {
    xmlFile.remove();
  }
}


const QList<Smb4KCustomOptions *> Smb4KCustomOptionsManager::customOptions( bool optionsOnly )
{
  QList<Smb4KCustomOptions *> custom_options;
  
  for ( int i = 0; i < d->options.size(); ++i )
  {
    Smb4KCustomOptions *options = d->options[i];
    
    if ( hasCustomOptions( options ) || (!optionsOnly && options->remount() == Smb4KCustomOptions::DoRemount) )
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
      
      if ( hasCustomOptions( options ) || options->remount() == Smb4KCustomOptions::DoRemount )
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
    case Smb4KBasicNetworkItem::Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( item );
      
      if ( host )
      {
        options = findOptions( host );
      
        if ( !options )
        {
          options = new Smb4KCustomOptions( host );
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
    case Smb4KBasicNetworkItem::Share:
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
  
  Smb4KCustomOptions *known_options = NULL;
  
  switch ( options->type() )
  {
    case Smb4KCustomOptions::Host:
    {
      known_options = findOptions( options->url() );
      break;
    }
    case Smb4KCustomOptions::Share:
    {
      known_options = findOptions( options->url() );
      break;
    }
    default:
    {
      break;
    }
  }
  
  if ( !known_options )
  {
    d->options << new Smb4KCustomOptions( *options );
  }
  else
  {
    if ( known_options != options && !known_options->equals( options ) )
    {
      known_options->setSMBPort( options->smbPort() );
#ifndef Q_OS_FREEBSD
      known_options->setFileSystemPort( options->fileSystemPort() );
      known_options->setWriteAccess( options->writeAccess() );
#endif
      known_options->setProtocolHint( options->protocolHint() );
      known_options->setUID( options->uid() );
      known_options->setGID( options->gid() );
      known_options->setUseKerberos( options->useKerberos() );
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
  
  // Check if there are custom options defined. We do this
  // by using an empty Smb4KCustomOptions object for comparison.
  Smb4KCustomOptions default_options;

  // SMB port
  if ( default_options.smbPort() != options->smbPort() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
#ifndef Q_OS_FREEBSD
  
  // File system port (used for mounting)
  if ( default_options.fileSystemPort() != options->fileSystemPort() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }

  // Write access
  if ( default_options.writeAccess() != options->writeAccess() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
#endif

  // Protocol hint
  if ( default_options.protocolHint() != options->protocolHint() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // Kerberos
  if ( default_options.useKerberos() != options->useKerberos() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // UID
  if ( default_options.uid() != options->uid() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // GID
  if ( default_options.gid() != options->gid() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // MAC address
  if ( QString::compare( default_options.macAddress(), options->macAddress() ) != 0 )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // Send before first scan
  if ( default_options.wolSendBeforeNetworkScan() != options->wolSendBeforeNetworkScan() )
  {
    return true;
  }
  else
  {
    // Do nothing
  }
  
  // Send before mount
  if ( default_options.wolSendBeforeMount() != options->wolSendBeforeMount() )
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



#include "smb4kcustomoptionsmanager.moc"
