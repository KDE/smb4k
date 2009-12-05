/***************************************************************************
    smb4ksambaoptionshandler  -  This class handles the Samba options.
                             -------------------
    begin                : So Mai 14 2006
    copyright            : (C) 2006-2008 by Alexander Reinholdt
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
#include <QHostAddress>

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
  // Check if an old custom_options file is present and load the options
  // from there. Remove the file afterwards.
  QFile file( KGlobal::dirs()->locateLocal( "data", "smb4k/custom_options", KGlobal::mainComponent() ) );

  if ( file.exists() )
  {
    QStringList contents;

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
      if ( file.exists() )
      {
        Smb4KCoreMessage::error( ERROR_OPENING_FILE, file.fileName() );
      }

      return;
    }

    if ( !contents.isEmpty() )
    {
      for ( int i = 0; i < contents.size(); ++i )
      {
        if ( contents.at( i ).startsWith( "[" ) )
        {
          Smb4KSambaOptionsInfo *info = new Smb4KSambaOptionsInfo();
          info->setUNC( contents.at( i ).section( "[", 1, -1 ).section( "]", -2, 0 ) );

          for ( int j = ++i; j < contents.size(); ++j )
          {
            if ( contents.at( j ).startsWith( "workgroup=" ) )
            {
              info->setWorkgroupName( contents.at( j ).section( "=", 1, 1 ).trimmed() );

              continue;
            }
            else if ( contents.at( j ).startsWith( "ip=" ) )
            {
              info->setIP( contents.at( j ).section( "=", 1, 1 ).trimmed() );

              continue;
            }
            else if ( contents.at( j ).startsWith( "remount=" ) )
            {
              QString remount = contents.at( j ).section( "=", 1, 1 ).trimmed();

              if ( QString::compare( remount, "true", Qt::CaseInsensitive ) == 0 )
              {
                info->setRemount( Smb4KSambaOptionsInfo::DoRemount );
              }
              else if ( QString::compare( remount, "true", Qt::CaseInsensitive ) == 0 )
              {
                info->setRemount( Smb4KSambaOptionsInfo::NoRemount );
              }
              else
              {
                info->setRemount( Smb4KSambaOptionsInfo::UndefinedRemount );
              }

              continue;
            }
            else if ( contents.at( j ).startsWith( "port=" ) )
            {
              int port = contents.at( j ).section( "=", 1, 1 ).trimmed().toInt();

              info->setPort( port );

              continue;
            }
#ifndef __FreeBSD__
            else if ( contents.at( j ).startsWith( "write access=" ) )
            {
              QString write_access = contents.at( j ).section( "=", 1, 1 ).trimmed();

              if ( QString::compare( write_access, "true", Qt::CaseInsensitive ) == 0 )
              {
                info->setWriteAccess( Smb4KSambaOptionsInfo::ReadWrite );
              }
              else if ( QString::compare( write_access, "false", Qt::CaseInsensitive ) == 0 )
              {
                info->setWriteAccess( Smb4KSambaOptionsInfo::ReadOnly );
              }
              else
              {
                info->setWriteAccess( Smb4KSambaOptionsInfo::UndefinedWriteAccess );
              }

              continue;
            }
#endif
            else if ( contents.at( j ).startsWith( "protocol=" ) )
            {
              QString protocol = contents.at( j ).section( "=", 1, 1 ).trimmed();

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

              continue;
            }
            else if ( contents.at( j ).startsWith( "kerberos=" ) )
            {
              QString use_kerberos = contents.at( j ).section( "=", 1, 1 ).trimmed();

              if ( QString::compare( use_kerberos, "true", Qt::CaseInsensitive ) == 0 )
              {
                info->setUseKerberos( Smb4KSambaOptionsInfo::UseKerberos );
              }
              else if ( QString::compare( use_kerberos, "false", Qt::CaseInsensitive ) == 0 )
              {
                info->setUseKerberos( Smb4KSambaOptionsInfo::NoKerberos );
              }
              else
              {
                info->setUseKerberos( Smb4KSambaOptionsInfo::UndefinedKerberos );
              }

              continue;
            }
            else if ( contents.at( j ).startsWith( "uid=" ) )
            {
              info->setUID( (uid_t)contents.at( j ).section( "=", 1, 1 ).trimmed().toInt() );

              continue;
            }
            else if ( contents.at( j ).startsWith( "gid=" ) )
            {
              info->setGID( (gid_t)contents.at( j ).section( "=", 1, 1 ).trimmed().toInt() );

              continue;
            }
            else if ( contents.at( j ).isEmpty() || contents.at( j ).trimmed().startsWith( "[" ) )
            {
              i = j;

              break;
            }
            else
            {
              continue;
            }
          }

          has_custom_options( info );

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
      // Do nothing
    }

    file.remove();

    writeCustomOptions();

    return;
  }
  else
  {
    // Do nothing
  }

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
                        info->setPort( xmlReader.readElementText().toInt() );
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
#ifndef __FreeBSD__
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
        has_custom_options( m_list[i] );

        if ( m_list.at( i )->hasCustomOptions() ||
             m_list.at( i )->remount() == Smb4KSambaOptionsInfo::DoRemount )
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

  if ( (info = findItem( share )) )
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

  if ( (info = findItem( share )) )
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
      if ( info->hasCustomOptions() )
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


const QString Smb4KSambaOptionsHandler::smbclientOptions( Smb4KShare *share )
{
  // FIXME: Make globalOptions() return a string.

  // Get the global Samba options
  (void) globalSambaOptions();

  Smb4KSambaOptionsInfo *info = NULL;

  if ( share )
  {
    info = findItem( share );
  }
  else
  {
    // Do nothing
  }

  QString args;

  // Get the strings that are needed to put the
  // argument list together:
  QString resolve_order =  (!Smb4KSettings::nameResolveOrder().isEmpty() &&
                           QString::compare( Smb4KSettings::nameResolveOrder(),
                           m_samba_options["name resolve order"] ) != 0) ?
                           Smb4KSettings::nameResolveOrder() :
                           QString();

  QString netbios_name =   (!Smb4KSettings::netBIOSName().isEmpty() &&
                           QString::compare( Smb4KSettings::netBIOSName(),
                           m_samba_options["netbios name"] ) != 0) ?
                           Smb4KSettings::netBIOSName() :
                           QString();

  QString netbios_scope =  (!Smb4KSettings::netBIOSScope().isEmpty() &&
                           QString::compare( Smb4KSettings::netBIOSScope(),
                           m_samba_options["netbios scope"] ) != 0) ?
                           Smb4KSettings::netBIOSScope() :
                           QString();

  QString socket_options = (!Smb4KSettings::socketOptions().isEmpty() &&
                           QString::compare( Smb4KSettings::socketOptions(),
                           m_samba_options["socket options"] ) != 0) ?
                           Smb4KSettings::socketOptions() :
                           QString();

  bool kerberos = false;

  if ( info )
  {
    switch ( info->useKerberos() )
    {
      case Smb4KSambaOptionsInfo::UseKerberos:
      {
        kerberos = true;

        break;
      }
      case Smb4KSambaOptionsInfo::NoKerberos:
      {
        kerberos = false;

        break;
      }
      case Smb4KSambaOptionsInfo::UndefinedKerberos:
      {
        kerberos = Smb4KSettings::useKerberos();

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
    kerberos = Smb4KSettings::useKerberos();
  }

  int port =               info && info->port() != -1 ?
                           info->port() :
                           Smb4KSettings::remoteSMBPort();

  // Options that are not customizable:
  args.append( !resolve_order.isEmpty() ?
               QString( " -R '%1'" ).arg( resolve_order ) :
               QString() );

  args.append( !netbios_name.isEmpty() ?
               QString( " -n '%1'" ).arg( netbios_name ) :
               QString() );

  args.append( !netbios_scope.isEmpty() ?
               QString( " -i '%1'" ).arg( netbios_scope ) :
               QString() );

  args.append( !socket_options.isEmpty() ?
               QString( " -O '%1'" ).arg( socket_options ) :
               QString() );

  args.append( Smb4KSettings::bufferSize() != 65520 ?
               QString( " -b %1" ).arg( Smb4KSettings::bufferSize() ) :
               QString() );

  args.append( (Smb4KSettings::machineAccount() ?
               " -P" :
               QString()) );

  switch ( Smb4KSettings::signingState() )
  {
    case Smb4KSettings::EnumSigningState::None:
    {
      // The user does not want this setting
      // to be used.
      break;
    }
    case Smb4KSettings::EnumSigningState::On:
    {
      args.append( " -S on" );

      break;
    }
    case Smb4KSettings::EnumSigningState::Off:
    {
      args.append( " -S off" );

      break;
    }
    case Smb4KSettings::EnumSigningState::Required:
    {
      args.append( " -S required" );

      break;
    }
    default:
    {
      break;
    }
  }

  args.append( (kerberos ?
               " -k" :
               QString()) );

  args.append( QString( " -p %1" ).arg( port ) );

  return args;
}


const QString Smb4KSambaOptionsHandler::nmblookupOptions( bool with_broadcast )
{
  // Get the global Samba options
  (void) globalSambaOptions();

  QHostAddress address( Smb4KSettings::broadcastAddress() );

  QString args;

  QString netbios_name =   (!Smb4KSettings::netBIOSName().isEmpty() &&
                           QString::compare( Smb4KSettings::netBIOSName(),
                           m_samba_options["netbios name"] ) != 0) ?
                           Smb4KSettings::netBIOSName() :
                           QString();

  QString netbios_scope =  (!Smb4KSettings::netBIOSScope().isEmpty() &&
                           QString::compare( Smb4KSettings::netBIOSScope(),
                           m_samba_options["netbios scope"] ) != 0) ?
                           Smb4KSettings::netBIOSScope() :
                           QString();

  QString socket_options = (!Smb4KSettings::socketOptions().isEmpty() &&
                           QString::compare( Smb4KSettings::socketOptions(),
                           m_samba_options["socket options"] ) != 0) ?
                           Smb4KSettings::socketOptions() :
                           QString();

  QString domain =         (!Smb4KSettings::domainName().isEmpty() &&
                           QString::compare( Smb4KSettings::domainName(),
                           m_samba_options["workgroup"] ) != 0) ?
                           Smb4KSettings::domainName() :
                           QString();

  args.append( !netbios_name.isEmpty() ?
               QString( " -n '%1'" ).arg( netbios_name ) :
               QString() );

  args.append( !netbios_scope.isEmpty() ?
               QString( " -i '%1'" ).arg( netbios_scope ) :
               QString() );

  args.append( !socket_options.isEmpty() ?
               QString( " -O '%1'" ).arg( socket_options ) :
               QString() );

  args.append( !domain.isEmpty() ?
               QString( " -W '%1'" ).arg( domain ) :
               QString() );

  args.append( (!Smb4KSettings::broadcastAddress().isEmpty() &&
               with_broadcast && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol) ?
               QString( " -B %1" ).arg( Smb4KSettings::broadcastAddress() ) :
               QString() );

  args.append( Smb4KSettings::usePort137() ?
               " -r" :
               QString() );

  return args;
}


const QString Smb4KSambaOptionsHandler::netOptions( NetCommand command, Smb4KHost *host )
{
  Q_ASSERT( host );

  // Find the host in the list.
  Smb4KSambaOptionsInfo *info = findItem( host );

  // Determine the protocol that should be used.
  QString protocol_hint;

  if ( host->protocol() != Smb4KHost::Automatic )
  {
    // The protocol entry of the Smb4KHostItem overwrites
    // the default protocol hint.
    switch ( host->protocol() )
    {
      case Smb4KHost::RPC:
      {
        protocol_hint = "rpc";

        break;
      }
      case Smb4KHost::RAP:
      {
        protocol_hint = "rap";

        break;
      }
      case Smb4KHost::ADS:
      {
        protocol_hint = "ads";

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
    switch ( Smb4KSettings::protocolHint() )
    {
      case Smb4KSettings::EnumProtocolHint::RPC:
      {
        protocol_hint = "rpc";

        break;
      }
      case Smb4KSettings::EnumProtocolHint::RAP:
      {
        protocol_hint = "rap";

        break;
      }
      case Smb4KSettings::EnumProtocolHint::ADS:
      {
        protocol_hint = "ads";

        break;
      }
      default:
      {
        break;
      }
    }
  }

  QString default_protocol;

  if ( info )
  {
    switch ( info->protocol() )
    {
      case Smb4KSambaOptionsInfo::RPC:
      {
        default_protocol = "rpc";

        break;
      }
      case Smb4KSambaOptionsInfo::RAP:
      {
        default_protocol = "rap";

        break;
      }
      case Smb4KSambaOptionsInfo::ADS:
      {
        default_protocol = "ads";

        break;
      }
      default:
      {
        default_protocol = protocol_hint;
      }
    }
  }
  else
  {
    default_protocol = protocol_hint;
  }

  QString netbios_name =     (!Smb4KSettings::netBIOSName().isEmpty() &&
                             QString::compare( Smb4KSettings::netBIOSName(),
                             m_samba_options["netbios name"] ) != 0) ?
                             Smb4KSettings::netBIOSName() :
                             QString();

  QString domain =           (!Smb4KSettings::domainName().isEmpty() &&
                             QString::compare( Smb4KSettings::domainName(),
                             m_samba_options["workgroup"] ) != 0) ?
                             Smb4KSettings::domainName() :
                             QString();

  int port =                 (info && info->port() != -1) ?
                             info->port() :
                             Smb4KSettings::remoteSMBPort();

  QString args;

  switch ( command )
  {
    case Share:
    {
      // We can only use "rpc" and "rap" protocol hints here. The others
      // will be ignored.
      // Note: The protocol entry of the Smb4KHost object will always
      // overwrite the default protocol.
      switch ( host->protocol() )
      {
        case Smb4KHost::RPC:
        case Smb4KHost::RAP:
        {
          // protocol_hint carries the correct protocol:
          args.append( " "+protocol_hint );

          break;
        }
        default:
        {
          if ( QString::compare( default_protocol, "ads", Qt::CaseInsensitive ) != 0 )
          {
            args.append( " "+default_protocol /* can be empty */ );
          }

          break;
        }
      }

      args.append( " share list -l" );

      break;
    }
    case LookupHost:
    {
      // Check that the server name is present:
      if ( host->hostName().trimmed().isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_NET_COMMAND );

        return args; // still empty
      }

      // This lookup command takes no protocol:
      args.append( QString( " lookup host %1" ).arg( KShell::quoteArg( host->hostName() ) ) );

      break;
    }
    case LookupMaster:
    {
      // Check that the domain name is present:
      if ( host->workgroupName().trimmed().isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_NET_COMMAND );

        return args; // still empty
      }

      // This lookup command takes no protocol:
      args.append( QString( " lookup master %1" ).arg( KShell::quoteArg( host->workgroupName() ) ) );

      break;
    }
    default:
    {
      // Bypass the rest and return an empty string.
      return args;
    }
  }

  args.append( !domain.isEmpty() ?
               QString( " -W %1" ).arg( KShell::quoteArg( domain  ) ) :
               QString() );

  args.append( !netbios_name.isEmpty() ?
               QString( " -n %1" ).arg( KShell::quoteArg( netbios_name ) ) :
               QString() );

  args.append( Smb4KSettings::machineAccount() ?
               " -P" :
               QString() );

  args.append( QString( " -p %1" ).arg( port ) );

  return args;
}


const QString Smb4KSambaOptionsHandler::netOptions( NetCommand command, Smb4KWorkgroup *workgroup )
{
  Q_ASSERT( workgroup );

  // Find the host in the list.
  Smb4KHost master_browser;
  master_browser.setWorkgroupName( workgroup->workgroupName() );
  master_browser.setHostName( workgroup->masterBrowserName() );
  master_browser.setIP( workgroup->masterBrowserIP() );

  Smb4KSambaOptionsInfo *info = findItem( &master_browser );

  QString netbios_name =     (!Smb4KSettings::netBIOSName().isEmpty() &&
                             QString::compare( Smb4KSettings::netBIOSName(),
                             m_samba_options["netbios name"] ) != 0) ?
                             Smb4KSettings::netBIOSName() :
                             QString();

  QString domain =           (!Smb4KSettings::domainName().isEmpty() &&
                             QString::compare( Smb4KSettings::domainName(),
                             m_samba_options["workgroup"] ) != 0) ?
                             Smb4KSettings::domainName() :
                             QString();

  int port =                 (info && info->port() != -1) ?
                             info->port() :
                             Smb4KSettings::remoteSMBPort();

  QString args;

  switch ( command )
  {
    case LookupHost:
    {
      // Check that the server name is present:
      if ( workgroup->masterBrowserName().trimmed().isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_NET_COMMAND );
        return args; // still empty
      }

      // This lookup command takes no protocol:
      args.append( QString( " lookup host %1" )
          .arg( KShell::quoteArg( workgroup->masterBrowserName() ) ) );

      break;
    }
    case LookupMaster:
    {
      // Check that the domain name is present:
      if ( workgroup->workgroupName().trimmed().isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_NET_COMMAND );
        return args; // still empty
      }

      // This lookup command takes no protocol:
      args.append( QString( " lookup master %1" ).arg( KShell::quoteArg( workgroup->workgroupName() ) ) );

      break;
    }
    default:
    {
      // Bypass the rest and return an empty string.
      return args;
    }
  }

  args.append( !domain.isEmpty() ?
               QString( " -W %1" ).arg( KShell::quoteArg( domain ) ) :
               QString() );

  args.append( !netbios_name.isEmpty() ?
               QString( " -n %1" ).arg( KShell::quoteArg( netbios_name ) ) :
               QString() );

  args.append( Smb4KSettings::machineAccount() ?
               " -P" :
               QString() );

  args.append( QString( " -p %1" ).arg( port ) );

  return args;
}


const QString Smb4KSambaOptionsHandler::netOptions( NetCommand command )
{
  QString netbios_name =     (!Smb4KSettings::netBIOSName().isEmpty() &&
                             QString::compare( Smb4KSettings::netBIOSName(),
                             m_samba_options["netbios name"] ) != 0) ?
                             Smb4KSettings::netBIOSName() :
                             QString();

  QString domain =           (!Smb4KSettings::domainName().isEmpty() &&
                             QString::compare( Smb4KSettings::domainName(),
                             m_samba_options["workgroup"] ) != 0) ?
                             Smb4KSettings::domainName() :
                             QString();

  QString args;

  switch ( command )
  {
    case ServerDomain:
    {
      // This only works with the rap protocol:
      args.append( " rap server domain" );

      break;
    }
    case Domain:
    {
      // This only works with the rap protocol:
      args.append( " rap domain" );

      break;
    }
    default:
    {
      // Bypass the rest and return an
      // empty string:
      return args;
    }
  }

  args.append( !domain.isEmpty() ?
               QString( " -W %1" ).arg( KShell::quoteArg( domain ) ) :
               QString() );

  args.append( !netbios_name.isEmpty() ?
               QString( " -n %1" ).arg( KShell::quoteArg( netbios_name ) ) :
               QString() );

  args.append( Smb4KSettings::machineAccount() ?
               " -P" :
               QString() );

  args.append( QString( " -p %1" ).arg( Smb4KSettings::remoteSMBPort() ) );

  return args;
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
    else if ( m_list.at( i )->unc().startsWith( share->hostUNC(), Qt::CaseInsensitive ) )
    {
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
    else if ( QString::compare( share->hostUNC().section( "/", 2, 2 ).trimmed(),
              m_list.at( i )->unc(), Qt::CaseInsensitive ) == 0 )
    {
      // For conversion purposes only. Remove later.
      if ( !info && !exactMatch )
      {
        info = m_list.at( i );
        info->setUNC( "//"+info->unc() );
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


Smb4KSambaOptionsInfo *Smb4KSambaOptionsHandler::findItem( Smb4KHost *host, bool exactMatch )
{
  Q_ASSERT( host );

  Smb4KSambaOptionsInfo *info = NULL;

  for ( int i = 0; i < m_list.size(); ++i )
  {
    if ( QString::compare( host->unc(), m_list.at( i )->unc(), Qt::CaseInsensitive ) == 0 )
    {
      // Exact match.
      info = m_list.at( i );
      break;
    }
    else if ( QString::compare( host->hostName(), m_list.at( i )->unc(), Qt::CaseInsensitive ) == 0 )
    {
      // For conversion purposes only. Remove later.
      if ( !info )
      {
        info = m_list.at( i );
        info->setUNC( "//"+info->unc() );
      }
      else
      {
        // Do nothing
      }
      continue;
    }
    else if ( m_list.at( i )->unc().startsWith( host->unc(), Qt::CaseInsensitive ) )
    {
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


void Smb4KSambaOptionsHandler::addItem( Smb4KSambaOptionsInfo *info, bool s )
{
  Q_ASSERT( info );

  has_custom_options( info );

  if ( info->hasCustomOptions() ||
       info->remount() == Smb4KSambaOptionsInfo::DoRemount )
  {
    Smb4KSambaOptionsInfo *item = NULL;

    switch ( info->type() )
    {
      case Smb4KSambaOptionsInfo::Host:
      {
        Smb4KHost host;
        host.setWorkgroupName( info->workgroupName() );

        // For conversion purposes only. Remove later.
        if ( !info->unc().startsWith( "//" ) )
        {
          host.setHostName( info->unc() );
        }
        else
        {
          host.setUNC( info->unc() );
        }

        host.setIP( info->ip() );

        item = findItem( &host );

        break;
      }
      case Smb4KSambaOptionsInfo::Share:
      {
        Smb4KShare share;
        share.setUNC( info->unc() );
        share.setWorkgroupName( info->workgroupName() );
        share.setHostIP( info->ip() );

        item = findItem( &share );

        break;
      }
      default:
      {
        break;
      }
    }

    if ( item && QString::compare( item->unc(), info->unc(), Qt::CaseInsensitive ) == 0 )
    {
      item->update( info );

      delete info;
    }
    else
    {
      m_list.append( info );
    }

    if ( s )
    {
      sync();
    }
  }
  else
  {
    switch ( info->type() )
    {
      case Smb4KSambaOptionsInfo::Host:
      {
        Smb4KHost host;
        host.setWorkgroupName( info->workgroupName() );

        // For conversion purposes only. Remove later.
        if ( !info->unc().startsWith( "//" ) )
        {
          host.setHostName( info->unc() );
        }
        else
        {
          host.setUNC( info->unc() );
        }

        host.setIP( info->ip() );

        removeItem( &host, false );

        break;
      }
      case Smb4KSambaOptionsInfo::Share:
      {
        Smb4KShare share;
        share.setUNC( info->unc() );
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
}


void Smb4KSambaOptionsHandler::removeItem( Smb4KHost *host, bool s )
{
  Q_ASSERT( host );

  Smb4KSambaOptionsInfo *item = findItem( host, true );

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


const QString Smb4KSambaOptionsHandler::smbtreeOptions( Smb4KWorkgroup *workgroup )
{
  Smb4KSambaOptionsInfo *info = NULL;

  if ( workgroup )
  {
    Smb4KHost host;
    host.setHostName( workgroup->masterBrowserName() );
    host.setWorkgroupName( workgroup->workgroupName() );
    host.setIP( workgroup->masterBrowserIP() );

    info = findItem( &host );
  }
  else
  {
    // Do nothing
  }

  bool kerberos = false;

  if ( info )
  {
    switch ( info->useKerberos() )
    {
      case Smb4KSambaOptionsInfo::UseKerberos:
      {
        kerberos = true;

        break;
      }
      case Smb4KSambaOptionsInfo::NoKerberos:
      {
        kerberos = false;

        break;
      }
      case Smb4KSambaOptionsInfo::UndefinedKerberos:
      {
        kerberos = Smb4KSettings::useKerberos();

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
    kerberos = Smb4KSettings::useKerberos();
  }

  QString args;

  args.append( (Smb4KSettings::machineAccount() ?
               " -P" :
               QString()) );

  switch ( Smb4KSettings::signingState() )
  {
    case Smb4KSettings::EnumSigningState::None:
    {
      // The user does not want this setting
      // to be used.
      break;
    }
    case Smb4KSettings::EnumSigningState::On:
    {
      args.append( " -S on" );

      break;
    }
    case Smb4KSettings::EnumSigningState::Off:
    {
      args.append( " -S off" );

      break;
    }
    case Smb4KSettings::EnumSigningState::Required:
    {
      args.append( " -S required" );

      break;
    }
    default:
    {
      break;
    }
  }

  args.append( (Smb4KSettings::smbtreeSendBroadcasts() ?
               " -b" :
               QString()) );

  args.append( (kerberos ?
               " -k" :
               QString()) );

  return args;
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

          info = findItem( &host, true );

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


void Smb4KSambaOptionsHandler::has_custom_options( Smb4KSambaOptionsInfo *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KSambaOptionsInfo::Host:
      {
        Smb4KSambaOptionsInfo::Protocol protocol_hint;

        switch ( Smb4KSettings::protocolHint() )
        {
          case Smb4KSettings::EnumProtocolHint::Automatic:
          {
            protocol_hint = Smb4KSambaOptionsInfo::Automatic;

            break;
          }
          case Smb4KSettings::EnumProtocolHint::RPC:
          {
            protocol_hint = Smb4KSambaOptionsInfo::RPC;

            break;
          }
          case Smb4KSettings::EnumProtocolHint::RAP:
          {
            protocol_hint = Smb4KSambaOptionsInfo::RAP;

            break;
          }
          case Smb4KSettings::EnumProtocolHint::ADS:
          {
            protocol_hint = Smb4KSambaOptionsInfo::ADS;

            break;
          }
          default:
          {
            protocol_hint = Smb4KSambaOptionsInfo::UndefinedProtocol;

            break;
          }
        }

        Smb4KSambaOptionsInfo::Kerberos default_kerberos = Smb4KSambaOptionsInfo::UndefinedKerberos;

        if ( Smb4KSettings::useKerberos() )
        {
          default_kerberos = Smb4KSambaOptionsInfo::UseKerberos;
        }
        else
        {
          default_kerberos = Smb4KSambaOptionsInfo::NoKerberos;
        }

        if ( item->protocol() != Smb4KSambaOptionsInfo::UndefinedProtocol &&
             item->protocol() != protocol_hint )
        {
          item->setHasCustomOptions( true );

          break;
        }
        else if ( item->port() != -1 &&
                  item->port() != Smb4KSettings::remoteSMBPort() )
        {
          item->setHasCustomOptions( true );

          break;
        }
        else if ( item->useKerberos() != Smb4KSambaOptionsInfo::UndefinedKerberos &&
                  item->useKerberos() != default_kerberos )
        {
          item->setHasCustomOptions( true );

          break;
        }
        else
        {
          break;
        }

        break;
      }
      case Smb4KSambaOptionsInfo::Share:
      {
#ifndef __FreeBSD__
        Smb4KSambaOptionsInfo::WriteAccess write_access;

        switch( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            write_access = Smb4KSambaOptionsInfo::ReadWrite;

            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            write_access = Smb4KSambaOptionsInfo::ReadOnly;

            break;
          }
          default:
          {
            write_access = Smb4KSambaOptionsInfo::UndefinedWriteAccess;

            break;
          }
        }
#endif

        Smb4KSambaOptionsInfo::Kerberos default_kerberos = Smb4KSambaOptionsInfo::UndefinedKerberos;

        if ( Smb4KSettings::useKerberos() )
        {
          default_kerberos = Smb4KSambaOptionsInfo::UseKerberos;
        }
        else
        {
          default_kerberos = Smb4KSambaOptionsInfo::NoKerberos;
        }

#ifndef __FreeBSD__

        if ( item->port() != -1 &&
             item->port() != Smb4KSettings::remoteFileSystemPort() )
        {
          item->setHasCustomOptions( true );

          break;
        }
        else if ( item->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess &&
                  item->writeAccess() != write_access )
        {
          item->setHasCustomOptions( true );

          break;
        }
        else if ( item->useKerberos() != Smb4KSambaOptionsInfo::UndefinedKerberos &&
                  item->useKerberos() != default_kerberos )
        {
          item->setHasCustomOptions( true );

          break;
        }
#else
        if ( item->port() != -1 &&
             item->port() != Smb4KSettings::remoteSMBPort() )
        {
          item->setHasCustomOptions( true );

          break;
        }
#endif
        else if ( item->uidIsSet() && (item->uid() != (uid_t)Smb4KSettings::userID().toInt()) )
        {
          item->setHasCustomOptions( true );

          break;
        }
        else if ( item->gidIsSet() && (item->gid() != (gid_t)Smb4KSettings::groupID().toInt()) )
        {
          item->setHasCustomOptions( true );

          break;
        }
        else
        {
          break;
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
    // Nothing to do
  }
}


#include "smb4ksambaoptionshandler.moc"
