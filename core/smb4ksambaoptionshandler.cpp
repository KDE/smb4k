/***************************************************************************
    smb4ksambaoptionshandler  -  This class handles the Samba options.
                             -------------------
    begin                : So Mai 14 2006
    copyright            : (C) 2006-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QTextCodec>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// KDE includes
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdebug.h>
#include <klocale.h>
#include <kshell.h>
#include <kglobal.h>

// system specific includes
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

// application specific includes
#include <smb4ksambaoptionshandler.h>
#include <smb4kdefs.h>
#include <smb4kcoremessage.h>
#include <smb4kglobal.h>
#include <smb4ksambaoptionsinfo.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>

using namespace Smb4KGlobal;

class Smb4KSambaOptionsHandlerPrivate
{
  public:
    /**
     * The Smb4KSambaOptionsHandler instance
     */
    Smb4KSambaOptionsHandler instance;
};


K_GLOBAL_STATIC( Smb4KSambaOptionsHandlerPrivate, m_priv );


Smb4KSambaOptionsHandler::Smb4KSambaOptionsHandler()
: QObject()
{
  // We need the directory.
  QString dir = KGlobal::dirs()->locateLocal( "data", "smb4k", KGlobal::mainComponent() );

  if ( !KGlobal::dirs()->exists( dir ) )
  {
    KGlobal::dirs()->makeDir( dir );
  }

  m_wins_server.clear();

  readCustomOptions();
}


Smb4KSambaOptionsHandler::~Smb4KSambaOptionsHandler()
{
  while ( !m_list.isEmpty() )
  {
    delete m_list.takeFirst();
  }
}


Smb4KSambaOptionsHandler *Smb4KSambaOptionsHandler::self()
{
  return &m_priv->instance;
}


QList<Smb4KSambaOptionsInfo *> Smb4KSambaOptionsHandler::sharesToRemount()
{
  QList<Smb4KSambaOptionsInfo *> remounts;

  for ( int i = 0; i < m_list.size(); ++i )
  {
    if ( m_list.at( i )->remount() == Smb4KSambaOptionsInfo::DoRemount )
    {
      remounts.append( m_list.at( i ) );

      continue;
    }
    else
    {
      continue;
    }
  }

  return remounts;
}


void Smb4KSambaOptionsHandler::readCustomOptions()
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
        if ( xmlReader.name() == "custom_options" && xmlReader.attributes().value( "version" ) != "1.0" )
        {
          xmlReader.raiseError( i18n( "%1 is not a version 1.0 file." ).arg( xmlFile.fileName() ) );

          break;
        }
        else
        {
          if ( xmlReader.name() == "options" )
          {
            Smb4KSambaOptionsInfo *info = new Smb4KSambaOptionsInfo();

            info->setProfile( xmlReader.attributes().value( "profile" ).toString() );

            // The type attribute is not used at the moment.

            while ( !(xmlReader.isEndElement() && xmlReader.name() == "options") )
            {
              xmlReader.readNext();

              if ( xmlReader.isStartElement() )
              {
                if ( xmlReader.name() == "workgroup" )
                {
                  info->setWorkgroupName( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "unc" )
                {
                  info->setUNC( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "ip" )
                {
                  info->setIP( xmlReader.readElementText() );
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
                          info->setRemount( Smb4KSambaOptionsInfo::DoRemount );
                        }
                        else if ( QString::compare( remount, "false" ) == 0 )
                        {
                          info->setRemount( Smb4KSambaOptionsInfo::NoRemount );
                        }
                        else
                        {
                          info->setRemount( Smb4KSambaOptionsInfo::UndefinedRemount );
                        }
                      }
                      else if ( xmlReader.name() == "port" )
                      {
                        // FIXME: For backward compatibility. Remove with versions >> 1.0.x.
#ifndef Q_OS_FREEBSD
                        if ( info->type() == Smb4KSambaOptionsInfo::Host )
                        {
                          info->setSMBPort( xmlReader.readElementText().toInt() );
                        }
                        else if ( info->type() == Smb4KSambaOptionsInfo::Share )
                        {
                          info->setFileSystemPort( xmlReader.readElementText().toInt() );
                        }
                        else
                        {
                          // Do nothing
                        }
#else
                        info->setSMBPort( xmlReader.readElementText().toInt() );
#endif
                      }
                      else if ( xmlReader.name() == "smb_port" )
                      {
                        info->setSMBPort( xmlReader.readElementText().toInt() );
                      }
                      else if ( xmlReader.name() == "protocol" )
                      {
                        QString protocol = xmlReader.readElementText();

                        if ( QString::compare( protocol, "auto" ) == 0 )
                        {
                          info->setProtocol( Smb4KSambaOptionsInfo::Automatic );
                        }
                        else if ( QString::compare( protocol, "rpc" ) == 0 )
                        {
                          info->setProtocol( Smb4KSambaOptionsInfo::RPC );
                        }
                        else if ( QString::compare( protocol, "rap" ) == 0 )
                        {
                          info->setProtocol( Smb4KSambaOptionsInfo::RAP );
                        }
                        else if ( QString::compare( protocol, "ads" ) == 0 )
                        {
                          info->setProtocol( Smb4KSambaOptionsInfo::ADS );
                        }
                        else
                        {
                          info->setProtocol( Smb4KSambaOptionsInfo::UndefinedProtocol );
                        }
                      }
#ifndef Q_OS_FREEBSD
                      else if ( xmlReader.name() == "filesystem_port" )
                      {
                        info->setFileSystemPort( xmlReader.readElementText().toInt() );
                      }
                      else if ( xmlReader.name() == "write_access" )
                      {
                        QString write_access = xmlReader.readElementText();

                        if ( QString::compare( write_access, "true" ) == 0 )
                        {
                          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadWrite );
                        }
                        else if ( QString::compare( write_access, "false" ) == 0 )
                        {
                          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadOnly );
                        }
                        else
                        {
                          info->setWriteAccess( Smb4KSambaOptionsInfo::UndefinedWriteAccess );
                        }
                      }
#endif
                      else if ( xmlReader.name() == "kerberos" )
                      {
                        QString kerberos = xmlReader.readElementText();

                        if ( QString::compare( kerberos, "true" ) == 0 )
                        {
                          info->setUseKerberos( Smb4KSambaOptionsInfo::UseKerberos );
                        }
                        else if ( QString::compare( kerberos, "false" ) == 0 )
                        {
                          info->setUseKerberos( Smb4KSambaOptionsInfo::NoKerberos );
                        }
                        else
                        {
                          info->setUseKerberos( Smb4KSambaOptionsInfo::UndefinedKerberos );
                        }
                      }
                      else if ( xmlReader.name() == "uid" )
                      {
                        info->setUID( (uid_t)xmlReader.readElementText().toInt() );
                      }
                      else if ( xmlReader.name() == "gid" )
                      {
                        info->setGID( (gid_t)xmlReader.readElementText().toInt() );
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

            m_list.append( info );
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
      Smb4KCoreMessage::error( ERROR_XML_ERROR, xmlFile.fileName(), xmlReader.errorString() );
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
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, xmlFile.fileName() );
    }
    else
    {
      // Do nothing
    }

    return;
  }
}


void Smb4KSambaOptionsHandler::writeCustomOptions()
{
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/custom_options.xml", KGlobal::mainComponent() ) );

  if ( !m_list.isEmpty() )
  {
    if ( xmlFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QXmlStreamWriter xmlWriter( &xmlFile );
#if QT_VERSION >= 0x040400
      xmlWriter.setAutoFormatting( true );
#endif

      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement( "custom_options" );
      xmlWriter.writeAttribute( "version", "1.0" );

      for ( int i = 0; i < m_list.size(); ++i )
      {
        Q_ASSERT( m_list.at( i ) );
        
        if ( hasCustomOptions( m_list.at( i ) ) || m_list.at( i )->remount() == Smb4KSambaOptionsInfo::DoRemount )
        {
          xmlWriter.writeStartElement( "options" );
          xmlWriter.writeAttribute( "type", m_list.at( i )->type() == Smb4KSambaOptionsInfo::Host ? "Host" : "Share" );
          xmlWriter.writeAttribute( "profile", m_list.at( i )->profile() );

          xmlWriter.writeTextElement( "workgroup", m_list.at( i )->workgroupName() );
          xmlWriter.writeTextElement( "unc", m_list.at( i )->unc() );
          xmlWriter.writeTextElement( "ip", m_list.at( i )->ip() );

          xmlWriter.writeStartElement( "custom" );

          QMap<QString,QString> map = m_list.at( i )->entries();
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
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, xmlFile.fileName() );
      return;
    }
  }
  else
  {
    xmlFile.remove();
  }
}


void Smb4KSambaOptionsHandler::addRemount( Smb4KShare *share )
{
  Q_ASSERT( share );

  Smb4KSambaOptionsInfo *info = NULL;

  if ( (info = findItem( share, true )) )
  {
    info->setRemount( Smb4KSambaOptionsInfo::DoRemount );
  }
  else
  {
    info = new Smb4KSambaOptionsInfo( share );
    info->setRemount( Smb4KSambaOptionsInfo::DoRemount );

    m_list.append( info );
  }
}


void Smb4KSambaOptionsHandler::removeRemount( Smb4KShare *share )
{
  Q_ASSERT( share );

  Smb4KSambaOptionsInfo *info = NULL;

  if ( (info = findItem( share, true )) )
  {
    info->setRemount( Smb4KSambaOptionsInfo::NoRemount );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptionsHandler::clearRemounts()
{
  // Remove all remounts from the list.
  QListIterator<Smb4KSambaOptionsInfo *> it( m_list );
  Smb4KSambaOptionsInfo *info = NULL;

  while ( it.hasNext() )
  {
    info = it.next();

    if ( info->remount() == Smb4KSambaOptionsInfo::DoRemount )
    {
      if ( hasCustomOptions( info ) )
      {
        info->setRemount( Smb4KSambaOptionsInfo::NoRemount );
      }
      else
      {
        int index = m_list.indexOf( info );
        delete m_list.takeAt( index );
      }

      continue;
    }
    else
    {
      continue;
    }
  }

  sync();
}


void Smb4KSambaOptionsHandler::sync()
{
  writeCustomOptions();
}


void Smb4KSambaOptionsHandler::read_smb_conf()
{
  // Clear the options list before reading.
  m_samba_options.clear();

  QStringList paths;
  paths << "/etc";
  paths << "/etc/samba";
  paths << "/usr/local/etc";
  paths << "/usr/local/etc/samba";

  QFile file( "smb.conf" );

  QStringList contents;

  // Locate the file and read its contents:
  for ( int i = 0; i < paths.size(); ++i )
  {
    QDir::setCurrent( paths.at( i ) );

    if ( file.exists() )
    {
      if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        QTextStream ts( &file );
        // Note: With Qt 4.3 this seems to be obsolete, we'll keep
        // it for now.
        ts.setCodec( QTextCodec::codecForLocale() );

        while ( !ts.atEnd() )
        {
          contents.append( ts.readLine( 0 ) );
        }

        file.close();
      }
      else
      {
        Smb4KCoreMessage::error( ERROR_OPENING_FILE, paths.at( i )+QDir::separator()+file.fileName() );

        // Stop here
        return;
      }

      break;
    }
    else
    {
      continue;
    }
  }

  if ( !contents.isEmpty() )
  {
    // Process the file contents.
    for ( int i = contents.indexOf( "[global]", 0 ); i < contents.size(); ++i )
    {
      if ( contents.at( i ).trimmed().startsWith( "#" ) ||
           contents.at( i ).trimmed().startsWith( ";" ) )
      {
        // This is a comment. We do not need it.
        continue;
      }
      else if ( contents.at( i ).trimmed().startsWith( "include" ) )
      {
        // Look for the include file and put its contents into the
        // m_samba_options map.
        file.setFileName( contents.at( i ).section( "=", 1, 1 ).trimmed() );

        if ( file.exists() )
        {
          if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
          {
            QTextStream ts( &file );
            // Note: With Qt 4.3 this seems to be obsolete, we'll keep
            // it for now.
            ts.setCodec( QTextCodec::codecForLocale() );

            QString buffer;

            while( !ts.atEnd() )
            {
              buffer = ts.readLine( 0 ).trimmed();

              if ( buffer.startsWith( "#" ) || buffer.startsWith( ";" ) )
              {
                continue;
              }
              else
              {
                QString key = buffer.section( "=", 0, 0 ).trimmed().toLower();
                m_samba_options[key] = buffer.section( "=", 1, 1 ).trimmed().toUpper();

                continue;
              }
            }
          }
          else
          {
            Smb4KCoreMessage::error( ERROR_OPENING_FILE, file.fileName() );

            // Continue reading the smb.conf file.
            continue;
          }
        }
      }
      else if ( contents.at( i ).startsWith( "[" ) &&
                !contents.at( i ).contains( "[global]", Qt::CaseSensitive ) )
      {
        // We reached the end of the [global] section. Stop here.
        break;
      }
      else
      {
        // Put the entries of the [global] section into the m_samba_options
        // map.
        QString key = contents.at( i ).section( "=", 0, 0 ).trimmed().toLower();
        m_samba_options[key] = contents.at( i ).section( "=", 1, 1 ).trimmed().toUpper();
      }
    }
  }
  else
  {
    // Nothing to do
  }

  // Post-processing. Some values should be entered with their defaults, if they are
  // not already present.
  if ( !m_samba_options.contains( "netbios name" ) )
  {
    size_t hostnamelen = 255;
    char *hostname = new char[hostnamelen];

    if ( gethostname( hostname, hostnamelen ) == -1 )
    {
      int error = errno;
      Smb4KCoreMessage::error( ERROR_GETTING_HOSTNAME, QString(), strerror( error ) );
    }
    else
    {
      m_samba_options["netbios name"] = ( QString( "%1" ).arg( hostname ) ).toUpper();
    }

    delete [] hostname;
  }
}


const QMap<QString,QString> &Smb4KSambaOptionsHandler::globalSambaOptions()
{
  if ( m_samba_options.isEmpty() )
  {
    read_smb_conf();
  }

  return m_samba_options;
}


const QString &Smb4KSambaOptionsHandler::winsServer()
{
  if ( m_wins_server.isEmpty() )
  {
    (void) globalSambaOptions();

    if ( !m_samba_options["wins server"].isEmpty() )
    {
      m_wins_server = m_samba_options["wins server"];
    }
    else if ( !m_samba_options["wins support"].isEmpty() &&
              (QString::compare( m_samba_options["wins support"], "yes", Qt::CaseInsensitive ) == 0 ||
              QString::compare( m_samba_options["wins support"], "true", Qt::CaseInsensitive ) == 0) )
    {
      m_wins_server = "127.0.0.1";
    }
  }

  return m_wins_server;
}


Smb4KSambaOptionsInfo *Smb4KSambaOptionsHandler::findItem( Smb4KShare *share, bool exactMatch )
{
  Q_ASSERT( share );

  Smb4KSambaOptionsInfo *info = NULL;

  for ( int i = 0; i < m_list.size(); ++i )
  {
    if ( QString::compare( share->unc(), m_list.at( i )->unc(), Qt::CaseInsensitive ) == 0 )
    {
      // Exact match.
      info = m_list.at( i );
      break;
    }
    else if ( QString::compare( share->homeUNC(), m_list.at( i )->unc(), Qt::CaseInsensitive ) == 0 )
    {
      // Exact match ('homes' share).
      info = m_list.at( i );
      break;
    }
    else if ( QString::compare( share->hostUNC(), m_list.at( i )->unc(), Qt::CaseInsensitive ) == 0 )
    {
      // The host that provides the share.
      if ( !info && !exactMatch )
      {
        info = m_list.at( i );
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

  return info;
}


Smb4KSambaOptionsInfo *Smb4KSambaOptionsHandler::findItem( Smb4KHost *host )
{
  Q_ASSERT( host );

  Smb4KSambaOptionsInfo *info = NULL;

  for ( int i = 0; i < m_list.size(); ++i )
  {
    if ( QString::compare( host->unc(), m_list.at( i )->unc(), Qt::CaseInsensitive ) == 0 )
    {
      info = m_list.at( i );
      break;
    }
    else
    {
      continue;
    }
  }

  return info;
}


void Smb4KSambaOptionsHandler::addItem( Smb4KSambaOptionsInfo *info, bool s )
{
  Q_ASSERT( info );
  
  if ( hasCustomOptions( info ) || info->remount() == Smb4KSambaOptionsInfo::DoRemount )
  {
    Smb4KSambaOptionsInfo *known_info = NULL;
    
    switch ( info->type() )
    {
      case Smb4KSambaOptionsInfo::Host:
      {
        Smb4KHost host;
        host.setUNC( info->unc( QUrl::None ) );
        host.setWorkgroupName( info->workgroupName() );
        host.setIP( info->ip() );
        
        known_info = findItem( &host );
        
        break;
      }
      case Smb4KSambaOptionsInfo::Share:
      {
        Smb4KShare share;
        share.setUNC( info->unc( QUrl::None ) );
        share.setWorkgroupName( info->workgroupName() );
        share.setHostIP( info->ip() );

        known_info = findItem( &share, true );
        
        break;
      }
      default:
      {
        break;
      }
    }
    
    if ( known_info )
    {
      known_info->update( info );
    }
    else
    {
      m_list.append( info );
    }
  }
  else
  {
    switch ( info->type() )
    {
      case Smb4KSambaOptionsInfo::Host:
      {
        Smb4KHost host;
        host.setUNC( info->unc( QUrl::None ) );
        host.setWorkgroupName( info->workgroupName() );
        host.setIP( info->ip() );
        
        removeItem( &host, false );
        
        break;
      }
      case Smb4KSambaOptionsInfo::Share:
      {
        Smb4KShare share;
        share.setUNC( info->unc( QUrl::None ) );
        share.setWorkgroupName( info->workgroupName() );
        share.setHostIP( info->ip() );

        removeItem( &share, false );
        
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  // Write the list to the hard drive.
  if ( s )
  {
    sync();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptionsHandler::removeItem( Smb4KHost *host, bool s )
{
  Q_ASSERT( host );

  Smb4KSambaOptionsInfo *item = findItem( host );

  if ( item )
  {
    int index = m_list.indexOf( item );
    delete m_list.takeAt( index );
  }

  if ( s )
  {
    sync();
  }
}


void Smb4KSambaOptionsHandler::removeItem( Smb4KShare *share, bool s )
{
  Q_ASSERT( share );

  Smb4KSambaOptionsInfo *item = findItem( share, true );

  if ( item )
  {
    int index = m_list.indexOf( item );
    delete m_list.takeAt( index );
  }

  if ( s )
  {
    sync();
  }
}


void Smb4KSambaOptionsHandler::updateCustomOptions( const QList<Smb4KSambaOptionsInfo *> &list )
{
  if ( !list.isEmpty() )
  {
    // Delete obsolete items.
    for ( int i = 0; i < m_list.size(); ++i )
    {
      bool found = false;

      for ( int j = 0; j < list.size(); ++j )
      {
        if ( QString::compare( m_list.at( i )->unc(), list.at( j )->unc(), Qt::CaseInsensitive ) == 0 )
        {
          found = true;
          break;
        }
        else
        {
          continue;
        }
      }

      if ( !found )
      {
        delete m_list.takeAt( i );
        continue;
      }
      else
      {
        continue;
      }
    }

    // Update and add items.
    for ( int i = 0; i < list.size(); ++i )
    {
      Smb4KSambaOptionsInfo *info = NULL;

      switch ( list.at( i )->type() )
      {
        case Smb4KSambaOptionsInfo::Host:
        {
          Smb4KHost host;
          host.setWorkgroupName( list.at( i )->workgroupName() );

          // For conversion purposes only. Remove later.
          if ( !list.at( i )->unc().startsWith( "//" ) )
          {
            host.setHostName( list.at( i )->unc() );
          }
          else
          {
            host.setUNC( list.at( i )->unc() );
          }

          host.setIP( list.at( i )->ip() );

          info = findItem( &host );

          break;
        }
        case Smb4KSambaOptionsInfo::Share:
        {
          Smb4KShare share;
          share.setUNC( list.at( i )->unc() );
          share.setWorkgroupName( list.at( i )->workgroupName() );
          share.setHostIP( list.at( i )->ip() );

          info = findItem( &share, true );

          break;
        }
        default:
        {
          break;
        }
      }

      if ( info )
      {
        info->update( list.at( i ) );
        continue;
      }
      else
      {
        m_list.append( list.at( i ) );
        continue;
      }
    }
  }
  else
  {
    while ( !m_list.isEmpty() )
    {
      delete m_list.takeFirst();
    }
  }

  sync();
}


bool Smb4KSambaOptionsHandler::hasCustomOptions( Smb4KSambaOptionsInfo *info )
{
  Q_ASSERT( info );
  
  switch ( info->type() )
  {
    case Smb4KSambaOptionsInfo::Host:
    {
      // Protocol.
      if ( info->protocol() != Smb4KSambaOptionsInfo::UndefinedProtocol )
      {
        switch ( Smb4KSettings::protocolHint() )
        {
          case Smb4KSettings::EnumProtocolHint::Automatic:
          {
            if ( info->protocol() != Smb4KSambaOptionsInfo::Automatic )
            {
              return true;
            }
            else
            {
              // Do nothing
            }
            
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RPC:
          {
            if ( info->protocol() != Smb4KSambaOptionsInfo::RPC )
            {
              return true;
            }
            else
            {
              // Do nothing
            }
            
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RAP:
          {
            if ( info->protocol() != Smb4KSambaOptionsInfo::RAP )
            {
              return true;
            }
            else
            {
              // Do nothing
            }
            
            break;
          }
          case Smb4KSettings::EnumProtocolHint::ADS:
          {
            if ( info->protocol() != Smb4KSambaOptionsInfo::ADS )
            {
              return true;
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
      }
      else
      {
        // Do nothing
      }
      
      // Kerberos.
      if ( info->useKerberos() != Smb4KSambaOptionsInfo::UndefinedKerberos )
      {
        if ( Smb4KSettings::useKerberos() && info->useKerberos() != Smb4KSambaOptionsInfo::UseKerberos )
        {
          return true;
        }
        else if ( !Smb4KSettings::useKerberos() && info->useKerberos() != Smb4KSambaOptionsInfo::NoKerberos )
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
      
      // Ports.
      if ( info->smbPort() != -1 )
      {
        if ( info->smbPort() != Smb4KSettings::remoteSMBPort() )
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

#ifndef Q_OS_FREEBSD
      if ( info->fileSystemPort() != -1 )
      {
        if ( info->fileSystemPort() != Smb4KSettings::remoteFileSystemPort() )
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
      
      // Write access.
      if ( info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
      {
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            if ( info->writeAccess() != Smb4KSambaOptionsInfo::ReadWrite )
            {
              return true;
            }
            else
            {
              // Do nothing
            }
            
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            if ( info->writeAccess() != Smb4KSambaOptionsInfo::ReadOnly )
            {
              return true;
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
      }
      else
      {
        // Do nothing
      }
#endif

      // User ID.
      if ( info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
      {
        return true;
      }
      else
      {
        // Do nothing
      }
      
      // Group ID.
      if ( info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
      {
        return true;
      }
      else
      {
        // Do nothing
      }
      
      break;
    }
    case Smb4KSambaOptionsInfo::Share:
    {
#ifndef Q_OS_FREEBSD
      // Write access.
      if ( info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
      {
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            if ( info->writeAccess() != Smb4KSambaOptionsInfo::ReadWrite )
            {
              return true;
            }
            else
            {
              // Do nothing
            }
            
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            if ( info->writeAccess() != Smb4KSambaOptionsInfo::ReadOnly )
            {
              return true;
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
      }
      else
      {
        // Do nothing
      }
      
      // Port.
      if ( info->fileSystemPort() != -1 )
      {
        if ( info->fileSystemPort() != Smb4KSettings::remoteFileSystemPort() )
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
#else
      // Port.
      if ( info->smbPort() != -1 )
      {
        if ( info->smbPort() != Smb4KSettings::remoteSMBPort() )
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
#endif
      
      // User ID.
      if ( info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
      {
        return true;
      }
      else
      {
        // Do nothing
      }
      
      // Group ID.
      if ( info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
      {
        return true;
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
  
  return false;
}


#include "smb4ksambaoptionshandler.moc"
