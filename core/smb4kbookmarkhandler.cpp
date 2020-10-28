/***************************************************************************
    This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2020 by Alexander Reinholdt
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
#include "smb4kprofilemanager.h"

// Qt includes
#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QPointer>
#include <QMutableListIterator>
#include <QApplication>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>


using namespace Smb4KGlobal;


Q_GLOBAL_STATIC(Smb4KBookmarkHandlerStatic, p);



Smb4KBookmarkHandler::Smb4KBookmarkHandler(QObject *parent)
: QObject(parent), d(new Smb4KBookmarkHandlerPrivate)
{
  // 
  // First we need the directory.
  // 
  QString path = dataLocation();
  
  QDir dir;
  
  if (!dir.exists(path))
  {
    dir.mkpath(path);
  }

  //
  // Read the list of bookmarks
  // 
  readBookmarkList();
  
  //
  // Init the bookmark editor
  // 
  d->editor = 0;
}


Smb4KBookmarkHandler::~Smb4KBookmarkHandler()
{
  while (!d->bookmarks.isEmpty())
  {
    d->bookmarks.takeFirst().clear();
  }
}


Smb4KBookmarkHandler *Smb4KBookmarkHandler::self()
{
  return &p->instance;
}


void Smb4KBookmarkHandler::addBookmark(const SharePtr &share)
{
  if (share)
  {
    QList<SharePtr> shares;
    shares << share;
    addBookmarks(shares);
  }
}


void Smb4KBookmarkHandler::addBookmark(const BookmarkPtr &bookmark)
{
  if (bookmark)
  {
    //
    // Create a list that will be passed to addBookmarks()
    // 
    QList<BookmarkPtr> bookmarks;
    
    //
    // Check if the share has already been bookmarked and skip it if it
    // already exists
    //
    BookmarkPtr knownBookmark = findBookmarkByUrl(bookmark->url());
    
    if (knownBookmark)
    {
      Smb4KNotification::bookmarkExists(knownBookmark.data());
      return;
    }
    
    //
    // Copy the bookmark, add the correct profile (may be empty) and
    // add it to the list.
    // 
    BookmarkPtr newBookmark = BookmarkPtr(bookmark);
    newBookmark->setProfile(Smb4KProfileManager::self()->activeProfile());
    bookmarks << newBookmark;
    
    //
    // Add the bookmark
    // 
    addBookmarks(bookmarks, false);
  }
}


void Smb4KBookmarkHandler::addBookmarks(const QList<SharePtr> &list)
{
  //
  // Prepare the list of bookmarks that should be added
  //
  QList<BookmarkPtr> newBookmarks;
  
  for (const SharePtr &share : list)
  {
    //
    // Printer shares cannot be bookmarked
    //
    if (share->isPrinter())
    {
      Smb4KNotification::cannotBookmarkPrinter(share);
      continue;
    }
    
    //
    // Process homes shares
    //
    if (share->isHomesShare() && !Smb4KHomesSharesHandler::self()->specifyUser(share, true))
    {
      continue;
    }
    
    //
    // Check if the share has already been bookmarked and skip it if it
    // already exists
    //
    BookmarkPtr knownBookmark = findBookmarkByUrl(share->isHomesShare() ? share->homeUrl() : share->url());
    
    if (knownBookmark)
    {
      Smb4KNotification::bookmarkExists(knownBookmark.data());
      continue;
    }
    
    BookmarkPtr bookmark = BookmarkPtr(new Smb4KBookmark(share.data()));
    bookmark->setProfile(Smb4KProfileManager::self()->activeProfile());
    newBookmarks << bookmark;
  }
  
  //
  // Show the bookmark dialog, if necessary
  //
  if (!newBookmarks.isEmpty())
  {
    QPointer<Smb4KBookmarkDialog> dlg = new Smb4KBookmarkDialog(newBookmarks, categoryList(), QApplication::activeWindow());
    
    if (dlg->exec() == QDialog::Accepted)
    {
      // The bookmark dialog uses an internal list of bookmarks, 
      // so use that one instead of the temporary list created 
      // above.
      addBookmarks(dlg->bookmarks(), false);
    }
    
    delete dlg;
  }
  
  //
  // Clear the temporary list of bookmarks
  //
  while (!newBookmarks.isEmpty())
  {
    newBookmarks.takeFirst().clear();
  }
}


void Smb4KBookmarkHandler::addBookmarks(const QList<BookmarkPtr> &list, bool replace)
{
  //
  // Process the incoming list.
  // In case the internal list should be replaced, clear the internal 
  // list first.
  // 
  if (replace)
  {
    QMutableListIterator<BookmarkPtr> it(d->bookmarks);
    
    while (it.hasNext())
    {
      BookmarkPtr bookmark = it.next();
      
      if (Smb4KSettings::useProfiles() && bookmark->profile() != Smb4KProfileManager::self()->activeProfile())
      {
        continue;
      }
      
      it.remove();
    }
  }
  
  //
  // Copy all bookmarks that are not in the list
  // 
  for (const BookmarkPtr &bookmark : list)
  {
    //
    // Check if the bookmark label is already in use
    // 
    if (!bookmark->label().isEmpty() && findBookmarkByLabel(bookmark->label()))
    {
      Smb4KNotification::bookmarkLabelInUse(bookmark.data());
      bookmark->setLabel(QString("%1 (1)").arg(bookmark->label()));
    }
      
    //
    // Check if we have to add the bookmark
    // 
    BookmarkPtr existingBookmark = findBookmarkByUrl(bookmark->url());
      
    if (!existingBookmark)
    {
      qDebug() << "Adding the bookmark to the internal list";
      d->bookmarks << bookmark;
    }
    else
    {
      // We do not need to update the bookmark, because we are
      // operating on a shared pointer.
    }
  }
  
  //
  // Save the bookmark list and emit the updated() signal
  //
  writeBookmarkList();
  emit updated();
}


void Smb4KBookmarkHandler::removeBookmark(const BookmarkPtr &bookmark)
{
  if (bookmark)
  {
    for (int i = 0; i < d->bookmarks.size(); ++i)
    {
      if ((!Smb4KSettings::useProfiles() || Smb4KSettings::activeProfile() == d->bookmarks.at(i)->profile()) &&
          QString::compare(d->bookmarks.at(i)->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                           bookmark->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                           Qt::CaseInsensitive) == 0 &&
          QString::compare(bookmark->categoryName(), d->bookmarks.at(i)->categoryName(), Qt::CaseInsensitive) == 0)
      {
        d->bookmarks.takeAt(i).clear();
        break;
      }
    }
    
    // Write the list to the bookmarks file.
    writeBookmarkList();
    emit updated();
  }
}


void Smb4KBookmarkHandler::removeCategory(const QString& name)
{
  QMutableListIterator<BookmarkPtr> it(d->bookmarks);
  
  while (it.hasNext())
  {
    const BookmarkPtr &b = it.next();
    
    if ((!Smb4KSettings::useProfiles() || Smb4KSettings::activeProfile() == b->profile()) ||
        QString::compare(b->categoryName(), name, Qt::CaseInsensitive) == 0)
    {
      it.remove();
    }
  }
  
  // Write the list to the bookmarks file.
  writeBookmarkList();
  emit updated();
}


void Smb4KBookmarkHandler::writeBookmarkList()
{
  QFile xmlFile(dataLocation()+QDir::separator()+"bookmarks.xml");

  if (!d->bookmarks.isEmpty())
  {
    if (xmlFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
      QXmlStreamWriter xmlWriter(&xmlFile);
      xmlWriter.setAutoFormatting(true);
      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement("bookmarks");
      xmlWriter.writeAttribute("version", "3.0");

      for (const BookmarkPtr &bookmark : d->bookmarks)
      {
        if (!bookmark->url().isValid())
        {
          Smb4KNotification::invalidURLPassed();
          continue;
        }
        
        xmlWriter.writeStartElement("bookmark");
        xmlWriter.writeAttribute("profile", bookmark->profile());
        xmlWriter.writeAttribute("category", bookmark->categoryName());

        xmlWriter.writeTextElement("workgroup", bookmark->workgroupName());
        xmlWriter.writeTextElement("url", bookmark->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort));
        xmlWriter.writeTextElement("login", bookmark->login());
        xmlWriter.writeTextElement("ip", bookmark->hostIpAddress());
        xmlWriter.writeTextElement("label", bookmark->label());

        xmlWriter.writeEndElement();
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
}


void Smb4KBookmarkHandler::readBookmarkList()
{
  //
  // Clear the list of bookmarks
  //
  while (!d->bookmarks.isEmpty())
  {
    d->bookmarks.takeFirst().clear();
  }
  
  // 
  // Locate the XML file and read the bookmarks
  // 
  QFile xmlFile(dataLocation()+QDir::separator()+"bookmarks.xml");

  if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QXmlStreamReader xmlReader(&xmlFile);

    while (!xmlReader.atEnd())
    {
      xmlReader.readNext();

      if (xmlReader.isStartElement())
      {
        if (xmlReader.name() == "bookmarks" && 
            (xmlReader.attributes().value("version") != "2.0" && xmlReader.attributes().value("version") != "3.0"))
        {
          xmlReader.raiseError(i18n("The format of %1 is not supported.", xmlFile.fileName()));
          break;
        }
        else
        {
          if (xmlReader.name() == "bookmark")
          {
            QString profile = xmlReader.attributes().value("profile").toString();
            
            BookmarkPtr bookmark = BookmarkPtr(new Smb4KBookmark());
            bookmark->setProfile(profile);
            
            if (xmlReader.attributes().hasAttribute("group"))
            {
              // For backward compatibility (since Smb4K 3.0.72)
              bookmark->setCategoryName(xmlReader.attributes().value("group").toString());
            }
            else
            {
              bookmark->setCategoryName(xmlReader.attributes().value("category").toString());
            }
              
            while (!(xmlReader.isEndElement() && xmlReader.name() == "bookmark"))
            {
              xmlReader.readNext();

              if (xmlReader.isStartElement())
              {
                if (xmlReader.name() == "workgroup")
                {
                  bookmark->setWorkgroupName(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "url")
                {
                  bookmark->setUrl(QUrl(xmlReader.readElementText()));
                }
                else if (xmlReader.name() == "login")
                {
                  bookmark->setLogin(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "ip")
                {
                  bookmark->setHostIpAddress(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "label")
                {
                  bookmark->setLabel(xmlReader.readElementText());
                }

                continue;
              }
              else
              {
                continue;
              }
            }
              
            d->bookmarks << bookmark;
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

    if (xmlReader.hasError())
    {
      Smb4KNotification::readingFileFailed(xmlFile, xmlReader.errorString());
    }
  }
  else
  {
    if (xmlFile.exists())
    {
      Smb4KNotification::openingFileFailed(xmlFile);
    }
  }
  
  emit updated();
}


BookmarkPtr Smb4KBookmarkHandler::findBookmarkByUrl(const QUrl &url)
{
  // Update the bookmarks:
  update();

  // Find the bookmark:
  BookmarkPtr bookmark;

  if (!url.isEmpty() && url.isValid() && !bookmarksList().isEmpty())
  {
    for (const BookmarkPtr &b : bookmarksList())
    {
      if (QString::compare(b->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                           url.toString(QUrl::RemoveUserInfo|QUrl::RemovePort),
                           Qt::CaseInsensitive) == 0)
      {
        bookmark = b;
        break;
      }
    }
  }

  return bookmark;
}


BookmarkPtr Smb4KBookmarkHandler::findBookmarkByLabel(const QString &label)
{
  // Update the bookmarks:
  update();

  // Find the bookmark:
  BookmarkPtr bookmark;

  for (const BookmarkPtr &b : bookmarksList())
  {
    if (QString::compare(b->label().toUpper(), label.toUpper()) == 0)
    {
      bookmark = b;
      break;
    }
    else
    {
      continue;
    }
  }

  return bookmark;
}


QList<BookmarkPtr> Smb4KBookmarkHandler::bookmarksList() const
{
  // Update the bookmarks:
  update();

  // Get this list of the bookmarks
  if (Smb4KSettings::useProfiles())
  {
    QList<BookmarkPtr> bookmarks;
    
    for (const BookmarkPtr &b : d->bookmarks)
    {
      if (b->profile() == Smb4KSettings::activeProfile())
      {
        bookmarks << b;
      }
    }
    
    return bookmarks;
  }
  
  // Return the list of bookmarks:
  return d->bookmarks;
}


QList<BookmarkPtr> Smb4KBookmarkHandler::bookmarksList(const QString &category) const
{
  // Update bookmarks
  update();

  // Get the list of bookmarks organized in the given category
  QList<BookmarkPtr> bookmarks;

  for (const BookmarkPtr &bookmark : bookmarksList())
  {
    if (QString::compare(category, bookmark->categoryName(), Qt::CaseInsensitive) == 0)
    {
      bookmarks << bookmark;
    }
  }

  return bookmarks;
}


QStringList Smb4KBookmarkHandler::categoryList() const
{
  QStringList categories;
  
  for (const BookmarkPtr &b : bookmarksList())
  {
    if (!categories.contains(b->categoryName()))
    {
      categories << b->categoryName();
    }
  }
  
  return categories;
}


void Smb4KBookmarkHandler::resetBookmarks()
{
  readBookmarkList();
}


bool Smb4KBookmarkHandler::isBookmarked(const SharePtr& share)
{
  if (findBookmarkByUrl(share->url()))
  {
    return true;
  }
  
  return false;
}


void Smb4KBookmarkHandler::editBookmarks()
{
  //
  // Only allow one instance of the bookmark editor
  // 
  if (!d->editor)
  {
    d->editor = new Smb4KBookmarkEditor(bookmarksList(), QApplication::activeWindow());
  }
  else
  {
    d->editor->raise();
  }
  
  if (d->editor->exec() == QDialog::Accepted)
  {
    addBookmarks(d->editor->editedBookmarks(), true);
  }
  else
  {
    resetBookmarks();
  }
  
  //
  // Delete the editor after use
  // 
  delete d->editor;
  d->editor = 0;
}


void Smb4KBookmarkHandler::update() const
{
  // Get new IP addresses.
  for (const BookmarkPtr &bookmark : d->bookmarks)
  {
    HostPtr host = findHost(bookmark->hostName(), bookmark->workgroupName());
    
    if (host)
    {
      if (host->hasIpAddress() && bookmark->hostIpAddress() != host->ipAddress())
      {
        bookmark->setHostIpAddress(host->ipAddress());
      }
    }
  }
}


void Smb4KBookmarkHandler::migrateProfile(const QString& from, const QString& to)
{
  // Replace the old profile name with the new one.
  for (const BookmarkPtr &bookmark : d->bookmarks)
  {
    if (QString::compare(bookmark->profile(), from, Qt::CaseSensitive) == 0)
    {
      bookmark->setProfile(to);
    }
  }
  
  // Write the new list to the file.
  writeBookmarkList();
}


void Smb4KBookmarkHandler::removeProfile(const QString& name)
{
  QMutableListIterator<BookmarkPtr> it(d->bookmarks);
  
  while (it.hasNext())
  {
    const BookmarkPtr &bookmark = it.next();
    
    if (QString::compare(bookmark->profile(), name, Qt::CaseSensitive) == 0)
    {
      it.remove();
    }
  }
  
  // Write the new list to the file.
  writeBookmarkList();
}

