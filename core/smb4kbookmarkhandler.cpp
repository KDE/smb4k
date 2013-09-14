/***************************************************************************
    smb4kbookmarkhandler  -  This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2013 by Alexander Reinholdt
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
#include "smb4kbookmarkhandler.h"
#include "smb4kbookmarkhandler_p.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"
#include "smb4kbookmark.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4ksettings.h"
#include "smb4knotification.h"

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QPointer>
#include <QtCore/QMutableListIterator>

// KDE includes
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

using namespace Smb4KGlobal;


K_GLOBAL_STATIC( Smb4KBookmarkHandlerStatic, p );



Smb4KBookmarkHandler::Smb4KBookmarkHandler( QObject *parent )
: QObject( parent ), d( new Smb4KBookmarkHandlerPrivate )
{
  // Bookmark editor
  d->editor = NULL;
  
  // First we need the directory.
  QString dir = KGlobal::dirs()->locateLocal( "data", "smb4k", KGlobal::mainComponent() );

  if ( !KGlobal::dirs()->exists( dir ) )
  {
    KGlobal::dirs()->makeDir( dir );
  }

  loadBookmarks();
}


Smb4KBookmarkHandler::~Smb4KBookmarkHandler()
{
  while ( !d->bookmarks.isEmpty() )
  {
    delete d->bookmarks.takeFirst();
  }
  
  delete d->editor;
}


Smb4KBookmarkHandler *Smb4KBookmarkHandler::self()
{
  return &p->instance;
}


void Smb4KBookmarkHandler::addBookmark( Smb4KShare *share, QWidget *parent )
{
  if ( share )
  {
    QList<Smb4KShare *> shares;
    shares << share;
    addBookmarks( shares, parent );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkHandler::addBookmarks( const QList<Smb4KShare *> &list, QWidget *parent )
{
  // Prepare the list of bookmarks and show the save dialog.
  QList<Smb4KBookmark *> new_bookmarks;
  
  for ( int i = 0; i < list.size(); ++i )
  {
    // Check if the share is a printer
    if ( list.at( i )->isPrinter() )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->cannotBookmarkPrinter( list.at( i ) );
      continue;
    }
    else
    {
      // Do nothing
    }

    // Process homes shares
    if ( list.at( i )->isHomesShare() )
    {
      // If the user provides a valid user name we set it and continue.
      // Otherwise the share will be skipped.
      if ( !Smb4KHomesSharesHandler::self()->specifyUser( list.at( i ), true, parent ) )
      {
        continue;
      }
      else
      {
        // Do nothing
      }
    }

    Smb4KBookmark *known_bookmark = NULL;
    
    if ( !list.at( i )->isHomesShare() )
    {
      known_bookmark = findBookmarkByUNC( list.at( i )->unc() );
    }
    else
    {
      known_bookmark = findBookmarkByUNC( list.at( i )->homeUNC() );
    }

    // Skip bookmarks already present
    if ( known_bookmark )
    {
      Smb4KNotification *notification = new Smb4KNotification();
      notification->bookmarkExists( known_bookmark );
      continue;
    }
    else
    {
      // Do nothing
    }

    new_bookmarks << new Smb4KBookmark( list.at( i ) );
  }
  
  if ( !new_bookmarks.isEmpty() )
  {
    QPointer<Smb4KBookmarkDialog> dlg = new Smb4KBookmarkDialog( new_bookmarks, groupsList(), parent );

    if ( dlg->exec() == KDialog::Accepted )
    {
      // Check the label of the new bookmarks.
      for ( int i = 0; i < new_bookmarks.size(); ++i )
      {
        if ( !new_bookmarks.at( i )->label().isEmpty() )
        {
          Smb4KBookmark *bookmark = findBookmarkByLabel( new_bookmarks.at( i )->label() );

          if ( bookmark )
          {
            Smb4KNotification *notification = new Smb4KNotification();
            notification->bookmarkLabelInUse( new_bookmarks.at( i ) );

            new_bookmarks[i]->setLabel( QString( "%1 (1)" ).arg( new_bookmarks.at( i )->label() ) );
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
      
      addBookmarks( new_bookmarks, false );
    }
    else
    {
      // Do nothing
    }
    
    delete dlg;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkHandler::addBookmarks(const QList< Smb4KBookmark* >& list, bool replace)
{
  // Clear the internal lists if desired.
  if ( replace )
  {
    while ( !d->bookmarks.isEmpty() )
    {
      delete d->bookmarks.takeFirst();
    }
    
    d->groups.clear();
  }
  else
  {
    // Do nothing
  }
  
  // Append the new bookmarks to the internal list.
  for ( int i = 0; i < list.size(); ++i )
  {
    d->bookmarks << new Smb4KBookmark( *list.at( i ) );
  }
      
  // Append new groups to the internal list.
  for ( int i = 0; i < list.size(); ++i )
  {
    if ( !d->groups.contains( list.at( i )->groupName() ) )
    {
      d->groups << list[i]->groupName();
    }
    else
    {
      // Do nothing
    }
  }
      
  d->groups.sort();
        
  // Save the bookmarks list.
  writeBookmarkList( d->bookmarks );  
  emit updated();
}


void Smb4KBookmarkHandler::removeBookmark(Smb4KBookmark* bookmark)
{
  if ( bookmark )
  {
    // Update the bookmarks
    update();
    
    for ( int i = 0; i < d->bookmarks.size(); ++i )
    {
      if ( QString::compare( bookmark->unc(), d->bookmarks.at(i)->unc(), Qt::CaseInsensitive ) == 0 &&
          QString::compare( bookmark->groupName(), d->bookmarks.at(i)->groupName(), Qt::CaseInsensitive ) == 0 )
      {
        delete d->bookmarks.takeAt(i);
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    // Update the groups
    d->groups.clear();
    
    for ( int i = 0; i < d->bookmarks.size(); ++i )
    {
      if ( !d->groups.contains( d->bookmarks.at( i )->groupName() ) )
      {
        d->groups << d->bookmarks[i]->groupName();
      }
      else
      {
        // Do nothing
      }
    }
    
    d->groups.sort();
    
    // Write the list to the bookmarks file.
    writeBookmarkList( d->bookmarks );
    emit updated();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkHandler::removeGroup(const QString& name)
{
  update();
  
  QMutableListIterator<Smb4KBookmark *> it( d->bookmarks );
  
  while ( it.hasNext() )
  {
    Smb4KBookmark *b = it.next();
    
    if ( QString::compare( b->groupName(), name, Qt::CaseInsensitive ) == 0 )
    {
      it.remove();
    }
    else
    {
      // Do nothing
    }
  }
  
  // Update the groups list
  d->groups.clear();
  
  for ( int i = 0; i < d->bookmarks.size(); ++i )
  {
    if ( !d->groups.contains( d->bookmarks.at( i )->groupName(), Qt::CaseInsensitive ) )
    {
      d->groups << d->bookmarks[i]->groupName();
    }
    else
    {
      // Do nothing
    }
  }
  
  d->groups.sort();
  
  // Write the list to the bookmarks file.
  writeBookmarkList( d->bookmarks );
  emit updated();
}


void Smb4KBookmarkHandler::writeBookmarkList( const QList<Smb4KBookmark *> &list )
{
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/bookmarks.xml", KGlobal::mainComponent() ) );

  if ( !list.isEmpty() )
  {
    if ( xmlFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QXmlStreamWriter xmlWriter( &xmlFile );
      xmlWriter.setAutoFormatting( true );
      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement( "bookmarks" );
      xmlWriter.writeAttribute( "version", "1.1" );

      for ( int i = 0; i < list.size(); ++i )
      {
        if ( !list.at( i )->url().isValid() )
        {
          Smb4KNotification *notification = new Smb4KNotification();
          notification->invalidURLPassed();
          continue;
        }
        else
        {
          // Do nothing
        }

        xmlWriter.writeStartElement( "bookmark" );
        xmlWriter.writeAttribute( "profile", d->bookmarks.at( i )->profile() );
        xmlWriter.writeAttribute( "group", d->bookmarks.at( i )->groupName() );

        xmlWriter.writeTextElement( "workgroup", d->bookmarks.at( i )->workgroupName() );
        xmlWriter.writeTextElement( "unc", d->bookmarks.at( i )->unc() );
        xmlWriter.writeTextElement( "login", d->bookmarks.at( i )->login() );
        xmlWriter.writeTextElement( "ip", d->bookmarks.at( i )->hostIP() );
        xmlWriter.writeTextElement( "type", d->bookmarks.at( i )->typeString() );
        xmlWriter.writeTextElement( "label", d->bookmarks.at( i )->label() );

        xmlWriter.writeEndElement();
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


void Smb4KBookmarkHandler::loadBookmarks()
{
  // Locate the XML file.
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/bookmarks.xml", KGlobal::mainComponent() ) );

  if ( xmlFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QXmlStreamReader xmlReader( &xmlFile );

    while ( !xmlReader.atEnd() )
    {
      xmlReader.readNext();

      if ( xmlReader.isStartElement() )
      {
        if ( xmlReader.name() == "bookmarks" &&
             (xmlReader.attributes().value( "version" ) != "1.0" && xmlReader.attributes().value( "version" ) != "1.1") )
        {
          xmlReader.raiseError( i18n( "The format of %1 is not supported.", xmlFile.fileName() ) );
          break;
        }
        else
        {
          if ( xmlReader.name() == "bookmark" )
          {
            Smb4KBookmark *bookmark = new Smb4KBookmark();

            bookmark->setProfile( xmlReader.attributes().value( "profile" ).toString() );
            bookmark->setGroupName( xmlReader.attributes().value( "group" ).toString() );

            while ( !(xmlReader.isEndElement() && xmlReader.name() == "bookmark") )
            {
              xmlReader.readNext();

              if ( xmlReader.isStartElement() )
              {
                if ( xmlReader.name() == "workgroup" )
                {
                  bookmark->setWorkgroupName( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "unc" )
                {
                  bookmark->setURL( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "login" )
                {
                  bookmark->setLogin( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "ip" )
                {
                  bookmark->setHostIP( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "type" )
                {
                  bookmark->setTypeString( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "label" )
                {
                  bookmark->setLabel( xmlReader.readElementText() );
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

            d->bookmarks.append( bookmark );
            
            if ( !d->groups.contains( bookmark->groupName() ) )
            {
              d->groups << bookmark->groupName();
            }
            else
            {
              // Do nothing
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
  
  emit updated();
}


Smb4KBookmark *Smb4KBookmarkHandler::findBookmarkByUNC( const QString &unc )
{
  // Update the bookmarks:
  update();

  // Find the bookmark:
  Smb4KBookmark *b = NULL;

  for ( int i = 0; i < d->bookmarks.size(); ++i )
  {
    if ( QString::compare( d->bookmarks.at( i )->unc().toUpper(), unc.toUpper() ) == 0 )
    {
      b = d->bookmarks[i];
      break;
    }
    else
    {
      continue;
    }
  }

  return b;
}


Smb4KBookmark *Smb4KBookmarkHandler::findBookmarkByLabel( const QString &label )
{
  // Update the bookmarks:
  update();

  // Find the bookmark:
  Smb4KBookmark *b = NULL;

  for ( int i = 0; i < d->bookmarks.size(); ++i )
  {
    if ( QString::compare( d->bookmarks.at( i )->label().toUpper(), label.toUpper() ) == 0 )
    {
      b = d->bookmarks[i];
      break;
    }
    else
    {
      continue;
    }
  }

  return b;
}


QList<Smb4KBookmark *> Smb4KBookmarkHandler::bookmarksList() const
{
  // Update the bookmarks:
  update();

  // Return the list of bookmarks:
  return d->bookmarks;
}


QList<Smb4KBookmark *> Smb4KBookmarkHandler::bookmarksList( const QString &group ) const
{
  // Update bookmarks
  update();

  // Get the list of bookmarks organized in the given group
  QList<Smb4KBookmark *> bookmarks;

  for ( int i = 0; i < d->bookmarks.size(); ++i )
  {
    if ( QString::compare( group, d->bookmarks.at( i )->groupName(), Qt::CaseInsensitive ) == 0 )
    {
      bookmarks << d->bookmarks[i];
    }
    else
    {
      // Do nothing
    }
  }

  return bookmarks;
}


QStringList Smb4KBookmarkHandler::groupsList() const
{
  return d->groups;
}


void Smb4KBookmarkHandler::editBookmarks( QWidget *parent )
{
  if ( !d->editor )
  {
    d->editor = new Smb4KBookmarkEditor( d->bookmarks, parent );
  }
  else
  {
    d->editor->raise();
  }
  
  if ( d->editor->exec() == KDialog::Accepted )
  {
    // Now replace the current list with the new one that is
    // passed by the editor. For this set the 'replace' argument
    // for addBookmarks() to TRUE.
    QList<Smb4KBookmark *> bookmarks = d->editor->editedBookmarks();
    addBookmarks( bookmarks, true );
  }
  else
  {
    // Do nothing
  }

  delete d->editor;
  d->editor = NULL;
}


void Smb4KBookmarkHandler::update() const
{
  // Get new IP addresses.
  for ( int i = 0; i < d->bookmarks.size(); ++i )
  {
    Smb4KHost *host = findHost( d->bookmarks.at( i )->hostName(), d->bookmarks.at( i )->workgroupName() );
    
    if ( host )
    {
      if ( !host->ip().trimmed().isEmpty() &&
           QString::compare( d->bookmarks.at( i )->hostIP(), host->ip() ) != 0 )
      {
        d->bookmarks[i]->setHostIP( host->ip() );
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

#include "smb4kbookmarkhandler.moc"
