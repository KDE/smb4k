/***************************************************************************
    smb4khomesshareshandler  -  This class handles the homes shares.
                             -------------------
    begin                : Do Aug 10 2006
    copyright            : (C) 2006-2012 by Alexander Reinholdt
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
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4khomesshareshandler.h"
#include "smb4khomesshareshandler_p.h"
#include "smb4kshare.h"
#include "smb4ksettings.h"
#include "smb4kauthinfo.h"
#include "smb4knotification.h"

// Qt includes
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>

// KDE includes
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kglobal.h>


K_GLOBAL_STATIC( Smb4KHomesSharesHandlerStatic, p );


Smb4KHomesSharesHandler::Smb4KHomesSharesHandler( QObject *parent )
: QObject( parent ), d( new Smb4KHomesSharesHandlerPrivate )
{
  // First we need the directory.
  QString dir = KGlobal::dirs()->locateLocal( "data", "smb4k", KGlobal::mainComponent() );

  if ( !KGlobal::dirs()->exists( dir ) )
  {
    KGlobal::dirs()->makeDir( dir );
  }
  
  readUserNames();
  
  connect( QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()) ); 
}


Smb4KHomesSharesHandler::~Smb4KHomesSharesHandler()
{
  while ( !d->homesUsers.isEmpty() )
  {
    delete d->homesUsers.takeFirst();
  }
}


Smb4KHomesSharesHandler *Smb4KHomesSharesHandler::self()
{
  return &p->instance;
}


bool Smb4KHomesSharesHandler::specifyUser( Smb4KShare *share, bool overwrite, QWidget *parent )
{
  Q_ASSERT( share );
  bool success = false;
  
  // Avoid that the dialog is opened although the homes
  // user name has already been defined.
  if ( share->isHomesShare() && (share->homeUNC().isEmpty() || overwrite) )
  {
    QStringList users;
    findHomesUsers( share, &users );
    
    QPointer<Smb4KHomesUserDialog> dlg = new Smb4KHomesUserDialog( parent );
    dlg->setUserNames( users );
    
    if ( dlg->exec() == KDialog::Accepted )
    {
      QString login = dlg->login();
      users = dlg->userNames();
      addHomesUsers( share, &users );
      
      if ( !login.isEmpty() )
      {
        // If the login names do not match, clear the password.
        if ( !share->login().isEmpty() && QString::compare( share->login(), login ) != 0 )
        {
          share->setPassword( QString() );
        }
        else
        {
          // Do nothing
        }
        
        // Set the login name.
        share->setLogin( login );        
        success = true;
      }
      else
      {
        // Do nothing
      }
      
      writeUserNames();
    }
    else
    {
      // Do nothing
    }
    
    delete dlg;
  }
  else
  {
    // The user name has already been set.
    success = true;
  }
  
  return success;
}


QStringList Smb4KHomesSharesHandler::homesUsers( Smb4KShare *share )
{
  Q_ASSERT( share );
  QStringList users;
  findHomesUsers( share, &users );
  return users;
}


void Smb4KHomesSharesHandler::readUserNames()
{
  // Locate the XML file.
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/homes_shares.xml", KGlobal::mainComponent() ) );

  if ( xmlFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QXmlStreamReader xmlReader( &xmlFile );

    while ( !xmlReader.atEnd() )
    {
      xmlReader.readNext();

      if ( xmlReader.isStartElement() )
      {
        if ( xmlReader.name() == "homes_shares" && xmlReader.attributes().value( "version" ) != "1.0" )
        {
          xmlReader.raiseError( i18n( "%1 is not a version 1.0 file.", xmlFile.fileName() ) );
          break;
        }
        else
        {
          if ( xmlReader.name() == "homes" )
          {
            Smb4KHomesUsers *users = new Smb4KHomesUsers();
            users->shareName = xmlReader.name().toString();

            while ( !(xmlReader.isEndElement() && xmlReader.name() == "homes") )
            {
              xmlReader.readNext();

              if ( xmlReader.isStartElement() )
              {
                if ( xmlReader.name() == "host" )
                {
                  users->hostName = xmlReader.readElementText();
                }
                else if ( xmlReader.name() == "workgroup" )
                {
                  users->workgroupName = xmlReader.readElementText();
                }
                else if ( xmlReader.name() == "ip" )
                {
                  users->hostIP = xmlReader.readElementText();
                }
                else if ( xmlReader.name() == "users" )
                {
                  while ( !(xmlReader.isEndElement() && xmlReader.name() == "users") )
                  {
                    xmlReader.readNext();

                    if ( xmlReader.isStartElement() && xmlReader.name() == "user" )
                    {
                      users->users << xmlReader.readElementText();
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

            d->homesUsers << users;
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


void Smb4KHomesSharesHandler::writeUserNames()
{
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/homes_shares.xml", KGlobal::mainComponent() ) );

  if ( !d->homesUsers.isEmpty() )
  {
    if ( xmlFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QXmlStreamWriter xmlWriter( &xmlFile );
#if QT_VERSION >= 0x040400
      xmlWriter.setAutoFormatting( true );
#endif

      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement( "homes_shares" );
      xmlWriter.writeAttribute( "version", "1.0" );

      for ( int i = 0; i < d->homesUsers.size(); ++i )
      {
        xmlWriter.writeStartElement( "homes" );
        xmlWriter.writeAttribute( "profile", "Default" );
        xmlWriter.writeTextElement( "host", d->homesUsers.at( i )->hostName );
        xmlWriter.writeTextElement( "workgroup", d->homesUsers.at( i )->workgroupName );
        xmlWriter.writeTextElement( "ip", d->homesUsers.at( i )->hostIP );
        xmlWriter.writeStartElement( "users" );

        for ( int j = 0; j < d->homesUsers.at( i )->users.size(); ++j )
        {
          xmlWriter.writeTextElement( "user", d->homesUsers.at( i )->users.at( j ) );
        }

        xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
      }

      xmlWriter.writeEndDocument();

      xmlFile.close();
    }
    else
    {
      Smb4KNotification::openingFileFailed(xmlFile);
      return;
    }
  }
  else
  {
    xmlFile.remove();
  }
}


void Smb4KHomesSharesHandler::findHomesUsers( Smb4KShare *share, QStringList *users )
{
  Q_ASSERT( share );
  Q_ASSERT( users );
 
  if ( !d->homesUsers.isEmpty() )
  {
    for ( int i = 0; i < d->homesUsers.size(); ++i )
    {
      if ( QString::compare( share->hostName(), d->homesUsers.at(i)->hostName, Qt::CaseInsensitive ) == 0 &&
           QString::compare( share->shareName(), d->homesUsers.at(i)->shareName, Qt::CaseInsensitive ) == 0 &&
           ((d->homesUsers.at( i )->workgroupName.isEmpty() || share->workgroupName().isEmpty()) ||
           QString::compare( share->workgroupName(), d->homesUsers.at(i)->workgroupName, Qt::CaseInsensitive ) == 0) )
      {
        *users = d->homesUsers.at( i )->users;
        break;
      }
      else
      {
        continue;
      }
    }
  }
}


void Smb4KHomesSharesHandler::addHomesUsers( Smb4KShare *share, QStringList *users )
{
  Q_ASSERT( share );
  Q_ASSERT( users );
  
  bool found = false;
  
  if ( !d->homesUsers.isEmpty() )
  {
    for ( int i = 0; i < d->homesUsers.size(); ++i )
    {
      if ( QString::compare( share->hostName(), d->homesUsers.at(i)->hostName, Qt::CaseInsensitive ) == 0 &&
           QString::compare( share->shareName(), d->homesUsers.at(i)->shareName, Qt::CaseInsensitive ) == 0 &&
           ((d->homesUsers.at( i )->workgroupName.isEmpty() || share->workgroupName().isEmpty()) ||
           QString::compare( share->workgroupName(), d->homesUsers.at( i )->workgroupName, Qt::CaseInsensitive ) == 0) )
      {
        d->homesUsers[i]->users = *users;
        found = true;
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
  
  if ( !found )
  {
    d->homesUsers << new Smb4KHomesUsers( *share, *users );
  }
  else
  {
    // Do nothing
  }  
}



/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KHomesSharesHandler::slotAboutToQuit()
{
  writeUserNames();
}

#include "smb4khomesshareshandler.moc"

