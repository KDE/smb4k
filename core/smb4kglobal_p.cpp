/***************************************************************************
    smb4kglobal_p  -  This is the private helper class of the Smb4KGlobal
    namespace.
                             -------------------
    begin                : Di Jul 24 2007
    copyright            : (C) 2007-2014 by Alexander Reinholdt
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
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <QtCore/QCoreApplication>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QAbstractSocket>

// system specific includes
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>


Smb4KGlobalPrivate::Smb4KGlobalPrivate()
{
  onlyForeignShares = false;
  coreInitialized = false;
  m_samba_options_read = false;
  
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
  while ( !workgroupsList.isEmpty() )
  {
    delete workgroupsList.takeFirst();
  }

  // Clear the host list.
  while ( !hostsList.isEmpty() )
  {
    delete hostsList.takeFirst();
  }

  // Clear the list of mounted shares.
  while ( !mountedSharesList.isEmpty() )
  {
    delete mountedSharesList.takeFirst();
  }

  // Clear the list of shares.
  while ( !sharesList.isEmpty() )
  {
    delete sharesList.takeFirst();
  }
}


const QMap<QString,QString> &Smb4KGlobalPrivate::globalSambaOptions( bool read )
{
  if (!m_samba_options_read || read)
  {
    // We are about to read the samba options. Set m_samba_options_read = true.
    m_samba_options_read = true;
    
    // Clear the options list before reading.
    m_samba_options.clear();

    QStringList paths;
    paths << "/etc";
    paths << "/etc/samba";
    paths << "/usr/local/etc";
    paths << "/usr/local/etc/samba";

    QFile file( "smb.conf" );
    bool file_exists = false;
    QStringList contents;

    // Locate the file and read its contents:
    for ( int i = 0; i < paths.size(); ++i )
    {
      QDir::setCurrent( paths.at( i ) );

      if ( (file_exists = file.exists()) )
      {
        if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
          QTextStream ts( &file );
          // Note: With Qt 4.3 this seems to be obsolete, we'll keep
          ts.setCodec( QTextCodec::codecForLocale() );

          while ( !ts.atEnd() )
          {
            contents.append( ts.readLine( 0 ) );
          }

          file.close();
        }
        else
        {
          Smb4KNotification::openingFileFailed(file);
          return m_samba_options;
        }

        break;
      }
      else
      {
        continue;
      }
    }
    
    // Notify the user if the configuration file could not be found
    // in the standard locations.
    if (!file_exists)
    {
      Smb4KNotification::sambaConfigFileMissing();
    }
    else
    {
      // Do nothing
    }

    if ( !contents.isEmpty() )
    {
      // Process the file contents.
      for ( int i = contents.indexOf( "[global]", 0 ); i < contents.size(); ++i )
      {
        if ( i == -1 )
        {
          // The smb.conf file does not contain a global section.
          break;
        }
        else
        {
          // Do nothing
        }
        
        if ( contents.at( i ).trimmed().startsWith( '#' ) ||
             contents.at( i ).trimmed().startsWith( ';' ) )
        {
          // This is a comment. We do not need it.
          continue;
        }
        else if ( contents.at( i ).trimmed().startsWith( QLatin1String( "include" ) ) )
        {
          // Look for the include file and put its contents into the
          // m_samba_options map.
          file.setFileName( contents.at( i ).section( '=', 1, 1 ).trimmed() );

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

                if ( buffer.startsWith( '#' ) || buffer.startsWith( ';' ) )
                {
                  continue;
                }
                else
                {
                  QString key = buffer.section( '=', 0, 0 ).trimmed().toLower();
                  m_samba_options[key] = buffer.section( '=', 1, 1 ).trimmed().toUpper();

                  continue;
                }
              }
            }
            else
            {
              Smb4KNotification::openingFileFailed(file);
              continue;
            }
          }
        }
        else if ( contents.at( i ).startsWith( '[' ) &&
                  !contents.at( i ).contains( "[global]", Qt::CaseSensitive ) )
        {
          // We reached the end of the [global] section. Stop here.
          break;
        }
        else
        {
          // Put the entries of the [global] section into the m_samba_options
          // map.
          QString key = contents.at( i ).section( '=', 0, 0 ).trimmed().toLower();
          m_samba_options[key] = contents.at( i ).section( '=', 1, 1 ).trimmed().toUpper();
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
        Smb4KNotification::systemCallFailed("gethostname()", error);
      }
      else
      {
        m_samba_options["netbios name"] = ( QString( "%1" ).arg( hostname ) ).toUpper();
      }

      delete [] hostname;
    }
  
  }
  else
  {
    // Do nothing
  }
  
  return m_samba_options;
}


void Smb4KGlobalPrivate::setDefaultSettings()
{
  // Samba options that have to be dynamically imported from smb.conf:
  QMap<QString, QString> opts = globalSambaOptions(!m_samba_options_read);

  if ( !opts["netbios name"].isEmpty() )
  {
    Smb4KSettings::self()->netBIOSNameItem()->setDefaultValue( opts["netbios name"] );

    if ( Smb4KSettings::netBIOSName().isEmpty() )
    {
      Smb4KSettings::self()->netBIOSNameItem()->setDefault();
    }
  }

  if ( !opts["workgroup"].isEmpty() )
  {
    Smb4KSettings::self()->domainNameItem()->setDefaultValue( opts["workgroup"] );

    if ( Smb4KSettings::domainName().isEmpty() )
    {
      Smb4KSettings::self()->domainNameItem()->setDefault();
    }
  }

  if ( !opts["socket options"].isEmpty() )
  {
    Smb4KSettings::self()->socketOptionsItem()->setDefaultValue( opts["socket options"] );

    if ( Smb4KSettings::socketOptions().isEmpty() )
    {
      Smb4KSettings::self()->socketOptionsItem()->setDefault();
    }
  }

  if ( !opts["netbios scope"].isEmpty() )
  {
    Smb4KSettings::self()->netBIOSScopeItem()->setDefaultValue( opts["netbios scope"] );

    if ( Smb4KSettings::netBIOSScope().isEmpty() )
    {
      Smb4KSettings::self()->netBIOSScopeItem()->setDefault();
    }
  }

  if ( !opts["name resolve order"].isEmpty() )
  {
    Smb4KSettings::self()->nameResolveOrderItem()->setDefaultValue( opts["name resolve order"] );

    if ( Smb4KSettings::nameResolveOrder().isEmpty() )
    {
      Smb4KSettings::self()->nameResolveOrderItem()->setDefault();
    }
  }

  QHostAddress address( opts["interfaces"].section( ' ', 0, 0 ) );

  if ( address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    Smb4KSettings::self()->broadcastAddressItem()->setDefaultValue( address.toString() );

    if ( Smb4KSettings::broadcastAddress().isEmpty() )
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
  Smb4KSettings::self()->writeConfig();
}


#include "smb4kglobal_p.moc"


