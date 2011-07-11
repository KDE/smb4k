/***************************************************************************
    smb4kbookmarkhandler  -  This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QDesktopWidget>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

// KDE includes
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

// system specific includes
#include <stdlib.h>
#include <math.h>

// application specific includes
#include <smb4khomesshareshandler.h>
#include <smb4kbookmarkhandler_p.h>
#include <smb4kbookmarkhandler.h>
#include <smb4kglobal.h>
#include <smb4kbookmark.h>
#include <smb4khost.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4knotification.h>

using namespace Smb4KGlobal;


K_GLOBAL_STATIC( Smb4KBookmarkHandlerPrivate, p );



Smb4KBookmarkHandler::Smb4KBookmarkHandler() : QObject()
{
  // Bookmark editor
  m_editor = NULL;
  
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
  while ( !m_bookmarks.isEmpty() )
  {
    delete m_bookmarks.takeFirst();
  }
}


Smb4KBookmarkHandler *Smb4KBookmarkHandler::self()
{
  return &p->instance;
}


void Smb4KBookmarkHandler::addBookmark( Smb4KShare *share, QWidget *parent )
{
  Q_ASSERT( share );
  QList<Smb4KShare *> shares;
  shares << share;
  addBookmarks( shares, parent );
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
      if ( !Smb4KHomesSharesHandler::self()->specifyUser( list.at( i ), parent ) )
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
    Smb4KBookmarkDialog dlg( new_bookmarks, groups(), parent );

    if ( dlg.exec() == KDialog::Accepted )
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

      // Append the new bookmarks to the internal list and
      // save that list to the bookmark file.
      m_bookmarks << new_bookmarks;
      writeBookmarkList( m_bookmarks );
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
        xmlWriter.writeAttribute( "profile", m_bookmarks.at( i )->profile() );
        xmlWriter.writeAttribute( "group", m_bookmarks.at( i )->group() );

        xmlWriter.writeTextElement( "workgroup", m_bookmarks.at( i )->workgroupName() );
        xmlWriter.writeTextElement( "unc", m_bookmarks.at( i )->unc() );
        xmlWriter.writeTextElement( "login", m_bookmarks.at( i )->login() );
        xmlWriter.writeTextElement( "ip", m_bookmarks.at( i )->hostIP() );
        xmlWriter.writeTextElement( "type", m_bookmarks.at( i )->typeString() );
        xmlWriter.writeTextElement( "label", m_bookmarks.at( i )->label() );

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

  emit updated();
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
          xmlReader.raiseError( i18n( "The format of %1 is not supported." ).arg( xmlFile.fileName() ) );
          break;
        }
        else
        {
          if ( xmlReader.name() == "bookmark" )
          {
            Smb4KBookmark *bookmark = new Smb4KBookmark();

            bookmark->setGroup( xmlReader.attributes().value( "profile" ).toString() );
            bookmark->setGroup(xmlReader.attributes().value( "group" ).toString() );

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
                  bookmark->setUNC( xmlReader.readElementText() );
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

            m_bookmarks.append( bookmark );
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

    return;
  }
}


Smb4KBookmark *Smb4KBookmarkHandler::findBookmarkByUNC( const QString &unc )
{
  // Update the bookmarks:
  update();

  // Find the bookmark:
  Smb4KBookmark *b = NULL;

  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    if ( QString::compare( m_bookmarks.at( i )->unc().toUpper(), unc.toUpper() ) == 0 )
    {
      b = m_bookmarks.at( i );

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

  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    if ( QString::compare( m_bookmarks.at( i )->label().toUpper(), label.toUpper() ) == 0 )
    {
      b = m_bookmarks.at( i );

      break;
    }
    else
    {
      continue;
    }
  }

  return b;
}


const QList<Smb4KBookmark *> &Smb4KBookmarkHandler::bookmarks()
{
  // Update the bookmarks:
  update();

  // Return the list of bookmarks:
  return m_bookmarks;
}


QList<Smb4KBookmark *> Smb4KBookmarkHandler::bookmarks( const QString &group )
{
  // Update bookmarks
  update();

  // Get the list of bookmarks organized in the given group
  QList<Smb4KBookmark *> bookmarks;

  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    if ( QString::compare( group, m_bookmarks.at( i )->group(), Qt::CaseInsensitive ) == 0 )
    {
      bookmarks << m_bookmarks[i];
    }
    else
    {
      // Do nothing
    }
  }

  return bookmarks;
}


QStringList Smb4KBookmarkHandler::groups()
{
  QStringList groups;

  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    if ( !groups.contains( m_bookmarks.at( i )->group() ) )
    {
      groups << m_bookmarks.at( i )->group();
    }
    else
    {
      // Do nothing
    }
  }

  groups.sort();
  
  return groups;
}


void Smb4KBookmarkHandler::editBookmarks( QWidget *parent )
{
  if ( !m_editor )
  {
    m_editor = new Smb4KBookmarkEditor( m_bookmarks, parent );

    if ( m_editor->exec() == KDialog::Accepted )
    {
      QList<Smb4KBookmark *> bookmarks = m_editor->editedBookmarks();

      // Update the list of bookmarks.
      QMutableListIterator<Smb4KBookmark *> it( m_bookmarks );
      Smb4KBookmark *bookmark = NULL;
      
      while ( it.hasNext() )
      {
        bookmark = it.next();
        bool found = false;

        for ( int i = 0; i < bookmarks.size(); ++i )
        {
          if ( QString::compare( bookmark->unc(), bookmarks.at( i )->unc() ) == 0 &&
               QString::compare( bookmark->workgroupName(), bookmarks.at( i )->workgroupName() ) == 0 )
          {
            bookmark->setLabel( bookmarks.at( i )->label() );
            bookmark->setLogin( bookmarks.at( i )->login() );
            bookmark->setHostIP( bookmarks.at( i )->hostIP() );
            bookmark->setGroup( bookmarks.at( i )->group() );
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
          it.remove();

          if ( bookmark )
          {
            delete bookmark;
            bookmark = NULL;
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
      
      // Finally write the list to the file. We do not
      // need to emit the updated() signal here, because
      // writeBookmarkList() is going to do this, anyway.
      writeBookmarkList( m_bookmarks );
    }
    else
    {
      // Do nothing
    }

    delete m_editor;
    m_editor = NULL;
  }
  else
  {
    m_editor->raise();
  }
}


void Smb4KBookmarkHandler::update()
{
  // Get new IP addresses.
  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    Smb4KHost *host = findHost( m_bookmarks.at( i )->hostName(), m_bookmarks.at( i )->workgroupName() );
    
    if ( host )
    {
      if ( !host->ip().trimmed().isEmpty() &&
           QString::compare( m_bookmarks.at( i )->hostIP(), host->ip() ) != 0 )
      {
        m_bookmarks[i]->setHostIP( host->ip() );
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
