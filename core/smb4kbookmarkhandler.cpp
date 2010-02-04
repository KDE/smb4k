/***************************************************************************
    smb4kbookmarkhandler  -  This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
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
#include <kapplication.h>
#include <klocale.h>

// system specific includes
#include <stdlib.h>
#include <math.h>

// application specific includes
#include <smb4khomesshareshandler.h>
#include <smb4kbookmarkhandler.h>
#include <smb4kdefs.h>
#include <smb4kcoremessage.h>
#include <smb4kglobal.h>
#include <smb4kbookmark.h>
#include <smb4khost.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>

using namespace Smb4KGlobal;


class Smb4KBookmarkHandlerPrivate
{
  public:
    Smb4KBookmarkHandler instance;
};

K_GLOBAL_STATIC( Smb4KBookmarkHandlerPrivate, priv );



Smb4KBookmarkHandler::Smb4KBookmarkHandler() : QObject()
{
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
  return &priv->instance;
}


void Smb4KBookmarkHandler::addBookmark( Smb4KShare *share, bool overwrite )
{
  if ( share )
  {
    if ( share->isPrinter() )
    {
      Smb4KCoreMessage::error( ERROR_BOOKMARK_PRINTER );
      return;
    }
    else
    {
      // Do nothing
    }

    // Copy the share to avoid problems with 'homes' shares.
    Smb4KShare internal_share = *share;

    kDebug() << internal_share.unc( QUrl::None ) << endl;

    if ( internal_share.isHomesShare() )
    {
      QWidget *parent = 0;

      if ( kapp )
      {
        if ( kapp->activeWindow() )
        {
          parent = kapp->activeWindow();
        }
        else
        {
          parent = kapp->desktop();
        }
      }
      else
      {
        // Do nothing
      }

      // If the user provides a valid user name we set it and continue.
      // Otherwise the function will just return.
      if ( !Smb4KHomesSharesHandler::self()->specifyUser( &internal_share, parent ) )
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

    // Search for the bookmark:
    Smb4KBookmark *result = findBookmarkByUNC( internal_share.unc() );

    if ( result )
    {
      // Update the bookmark.
      if ( overwrite &&
           QString::compare( result->workgroupName(), internal_share.workgroupName(), Qt::CaseInsensitive ) == 0 )
      {
        result->setHostIP( internal_share.hostIP() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // The bookmark is new. Append it to the list:
      m_bookmarks.append( new Smb4KBookmark( &internal_share ) );
    }

    writeBookmarkList( m_bookmarks );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkHandler::addBookmark( Smb4KBookmark *bookmark, bool overwrite )
{
  Q_ASSERT( bookmark );

  Smb4KBookmark *internal = findBookmarkByUNC( bookmark->unc() );

  if ( internal )
  {
    // Update the bookmark.
    if ( overwrite &&
         QString::compare( internal->workgroupName(), bookmark->workgroupName(), Qt::CaseInsensitive ) == 0 )
    {
      internal->setHostIP( bookmark->hostIP() );
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // The bookmark is new. Append it to the list:
    m_bookmarks.append( new Smb4KBookmark( *bookmark ) );
  }

  writeBookmarkList( m_bookmarks );
}


void Smb4KBookmarkHandler::writeBookmarkList( const QList<Smb4KBookmark *> &list )
{
  if ( list != m_bookmarks )
  {
    m_bookmarks.clear();
    m_bookmarks = list;
  }
  else
  {
    // Do nothing
  }

  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/bookmarks.xml", KGlobal::mainComponent() ) );

  if ( !m_bookmarks.isEmpty() )
  {
    if ( xmlFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QXmlStreamWriter xmlWriter( &xmlFile );
#if QT_VERSION >= 0x040400
      xmlWriter.setAutoFormatting( true );
#endif

      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement( "bookmarks" );
      xmlWriter.writeAttribute( "version", "1.0" );

      int serial_number = 0;

      for ( int i = 0; i < m_bookmarks.size(); ++i )
      {
        if ( !m_bookmarks.at( i )->label().isEmpty() )
        {
          Smb4KBookmark *result = findBookmarkByLabel( m_bookmarks.at( i )->label() );

          if ( result &&
               (QString::compare( result->unc().toUpper(),
                                  m_bookmarks.at( i )->unc().toUpper() ) != 0 ||
                QString::compare( result->workgroupName().toUpper(),
                                  m_bookmarks.at( i )->workgroupName().toUpper() ) != 0) )
          {
            Smb4KCoreMessage::information( INFO_BOOKMARK_LABEL_IN_USE,
                                           m_bookmarks.at( i )->label(),
                                           m_bookmarks.at( i )->unc() );

            m_bookmarks.at( i )->setLabel( QString( "%1 (%2)" ).arg( m_bookmarks.at( i )->label() ).arg( serial_number++ ) );
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

        xmlWriter.writeStartElement( "bookmark" );
        xmlWriter.writeAttribute( "profile", m_bookmarks.at( i )->profile() );

        xmlWriter.writeTextElement( "workgroup", m_bookmarks.at( i )->workgroupName() );
        xmlWriter.writeTextElement( "unc", m_bookmarks.at( i )->unc() );
        xmlWriter.writeTextElement( "login", m_bookmarks.at( i )->login() );
        xmlWriter.writeTextElement( "ip", m_bookmarks.at( i )->hostIP() );
        xmlWriter.writeTextElement( "type", m_bookmarks.at( i )->type() );
        xmlWriter.writeTextElement( "label", m_bookmarks.at( i )->label() );

        xmlWriter.writeEndElement();
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

  emit updated();
}


void Smb4KBookmarkHandler::loadBookmarks()
{
  // Check if an old bookmark file is present and load the bookmarks
  // from there. Remove the file afterwards.
  QFile file( KGlobal::dirs()->locateLocal( "data", "smb4k/bookmarks", KGlobal::mainComponent() ) );

  if ( file.exists() )
  {
    // Import the data from the old bookmark file.
    QStringList contents;

    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, but we'll
      // keep it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      while ( !ts.atEnd() )
      {
        contents.append( ts.readLine( 0 ) );
      }

      file.close();

      for ( int i = 0; i < contents.size();  ++i )
      {
        if ( contents.at( i ).startsWith( "#" ) || contents.at( i ).startsWith( "[" ) ||
            contents.at( i ).trimmed().isEmpty() )
        {
          continue;
        }
        else
        {
          Smb4KBookmark *bookmark = new Smb4KBookmark();
          bookmark->setWorkgroupName( contents.at( i ).section( ",", 2, 2 ).trimmed() );
          bookmark->setHostName( contents.at( i ).section( ",", 0, 0 ).trimmed() );
          bookmark->setShareName( contents.at( i ).section( ",", 1, 1 ).trimmed() );
          bookmark->setHostIP( contents.at( i ).section( ",", 3, 3 ).trimmed() );
          bookmark->setLabel( contents.at( i ).section( ",", 4, 4 ).trimmed() );

          m_bookmarks.append( bookmark );
        }
      }

      emit updated();
    }
    else
    {
      if ( file.exists() )
      {
        Smb4KCoreMessage::error( ERROR_OPENING_FILE, file.fileName() );
      }
      else
      {
        // Do nothing if the file does not exist.
      }
    }

    // Save the new bookmarks file.
    writeBookmarkList( m_bookmarks );

    // Remove the old bookmark file.
    file.remove();

    // Stop here.
    return;
  }
  else
  {
    // Do nothing
  }

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
        if ( xmlReader.name() == "bookmarks" && xmlReader.attributes().value( "version" ) != "1.0" )
        {
          xmlReader.raiseError( i18n( "%1 is not a version 1.0 file." ).arg( xmlFile.fileName() ) );

          break;
        }
        else
        {
          if ( xmlReader.name() == "bookmark" )
          {
            Smb4KBookmark *bookmark = new Smb4KBookmark();

            bookmark->setProfile( xmlReader.attributes().value( "profile" ).toString() );

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
                  bookmark->setType( xmlReader.readElementText() );
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


const QList<Smb4KBookmark *> &Smb4KBookmarkHandler::getBookmarks()
{
  // Update the bookmarks:
  update();

  // Return the list of bookmarks:
  return m_bookmarks;
}


void Smb4KBookmarkHandler::update()
{
  // Search the list of hosts for new IP addresses:
  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    for ( int j = 0; j < hostsList()->size(); ++j )
    {
      if ( QString::compare( m_bookmarks.at( i )->workgroupName().toUpper(),
                             hostsList()->at( j )->workgroupName().toUpper() ) != 0 )
      {
        // Continue, if the workgroup is not the same:
        continue;
      }
      else
      {
        if ( QString::compare( m_bookmarks.at( i )->hostName().toUpper(),
                               hostsList()->at( j )->hostName().toUpper() ) != 0 )
        {
          // Continue if the host name is not the same:
          continue;
        }
        else
        {
          // Set the IP address if it changed:
          if ( !hostsList()->at( j )->ip().trimmed().isEmpty() &&
               QString::compare( m_bookmarks.at( i )->hostIP(), hostsList()->at( j )->ip() ) != 0 )
          {
            m_bookmarks.at( i )->setHostIP( hostsList()->at( j )->ip() );
          }

          break;
        }
      }
    }
  }
}

#include "smb4kbookmarkhandler.moc"
