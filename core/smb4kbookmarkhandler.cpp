/***************************************************************************
    This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2017 by Alexander Reinholdt
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

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>


using namespace Smb4KGlobal;


Q_GLOBAL_STATIC(Smb4KBookmarkHandlerStatic, p);



Smb4KBookmarkHandler::Smb4KBookmarkHandler(QObject *parent)
: QObject(parent), d(new Smb4KBookmarkHandlerPrivate)
{
  // Bookmark editor
  d->editor = 0;
  
  // First we need the directory.
  QString path = dataLocation();
  
  QDir dir;
  
  if (!dir.exists(path))
  {
    dir.mkpath(path);
  }
  else
  {
    // Do nothing
  }

  readBookmarks();
  
  // Connections
  connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)), 
          this, SLOT(slotActiveProfileChanged(QString)));
}


Smb4KBookmarkHandler::~Smb4KBookmarkHandler()
{
  while (!d->bookmarks.isEmpty())
  {
    delete d->bookmarks.takeFirst();
  }
  
  delete d->editor;
}


Smb4KBookmarkHandler *Smb4KBookmarkHandler::self()
{
  return &p->instance;
}


void Smb4KBookmarkHandler::addBookmark(const SharePtr &share, QWidget *parent)
{
  if (share)
  {
    QList<SharePtr> shares;
    shares << share;
    addBookmarks(shares, parent);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkHandler::addBookmarks(const QList<SharePtr> &list, QWidget *parent)
{
  // Prepare the list of bookmarks and show the save dialog.
  QList<Smb4KBookmark *> new_bookmarks;
  
  for (const SharePtr &share : list)
  {
    // Check if the share is a printer
    if (share->isPrinter())
    {
      Smb4KNotification::cannotBookmarkPrinter(share);
      continue;
    }
    else
    {
      // Do nothing
    }

    // Process homes shares
    if (share->isHomesShare())
    {
      // If the user provides a valid user name we set it and continue.
      // Otherwise the share will be skipped.
      if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true, parent))
      {
        continue;
      }
      else
      {
        // Do nothing
      }
    }

    Smb4KBookmark *known_bookmark = 0;
    
    if (!share->isHomesShare())
    {
      known_bookmark = findBookmarkByUNC(share->unc());
    }
    else
    {
      known_bookmark = findBookmarkByUNC(share->homeUNC());
    }

    // Skip bookmarks already present
    if (known_bookmark)
    {
      Smb4KNotification::bookmarkExists(known_bookmark);
      continue;
    }
    else
    {
      // Do nothing
    }
    
    Smb4KBookmark *bookmark = new Smb4KBookmark(share.data());
    bookmark->setProfile(Smb4KProfileManager::self()->activeProfile());
    new_bookmarks << bookmark;
  }
  
  if (!new_bookmarks.isEmpty())
  {
    QPointer<Smb4KBookmarkDialog> dlg = new Smb4KBookmarkDialog(new_bookmarks, groupsList(), parent);

    if (dlg->exec() == QDialog::Accepted)
    {
      // The bookmark dialog uses an internal list of bookmarks, 
      // so use that one instead of the temporary list created 
      // above.
      addBookmarks(dlg->bookmarks(), false);
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
  
  // Clear the list of bookmarks.
  while (!new_bookmarks.isEmpty())
  {
    delete new_bookmarks.takeFirst();
  }
}


void Smb4KBookmarkHandler::addBookmarks(const QList<Smb4KBookmark*>& list, bool replace)
{
  // Clear the internal lists if desired.
  if (replace)
  {
    while (!d->bookmarks.isEmpty())
    {
      delete d->bookmarks.takeFirst();
    }
  }
  else
  {
    // Do nothing
  }
  
  // Append the new bookmarks to the internal list and check
  // the label simultaneously.
  for (Smb4KBookmark *bookmark : list)
  {
    if (!bookmark->label().isEmpty() && findBookmarkByLabel(bookmark->label()))
    {
      Smb4KNotification::bookmarkLabelInUse(bookmark);
      Smb4KBookmark *new_bookmark = new Smb4KBookmark(*bookmark);
      new_bookmark->setLabel(QString("%1 (1)").arg(bookmark->label()));
      d->bookmarks << new_bookmark;
    }
    else
    {
      d->bookmarks << new Smb4KBookmark(*bookmark);
    }
  }
      
  // Save the bookmarks list.
  writeBookmarkList();  
  emit updated();
}


void Smb4KBookmarkHandler::removeBookmark(Smb4KBookmark* bookmark)
{
  if (bookmark)
  {
    for (int i = 0; i < d->bookmarks.size(); ++i)
    {
      if ((!Smb4KSettings::useProfiles() || Smb4KSettings::activeProfile() == d->bookmarks.at(i)->profile()) &&
          QString::compare(bookmark->unc(), d->bookmarks.at(i)->unc(), Qt::CaseInsensitive) == 0 &&
          QString::compare(bookmark->groupName(), d->bookmarks.at(i)->groupName(), Qt::CaseInsensitive) == 0)
      {
        delete d->bookmarks.takeAt(i);
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    // Write the list to the bookmarks file.
    writeBookmarkList();
    emit updated();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkHandler::removeGroup(const QString& name)
{
  QMutableListIterator<Smb4KBookmark *> it(d->bookmarks);
  
  while (it.hasNext())
  {
    Smb4KBookmark *b = it.next();
    
    if ((!Smb4KSettings::useProfiles() || Smb4KSettings::activeProfile() == b->profile()) ||
        QString::compare(b->groupName(), name, Qt::CaseInsensitive) == 0)
    {
      it.remove();
    }
    else
    {
      // Do nothing
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
      xmlWriter.writeAttribute("version", "1.1");

      for (Smb4KBookmark *bookmark : d->bookmarks)
      {
        if (!bookmark->url().isValid())
        {
          Smb4KNotification::invalidURLPassed();
          continue;
        }
        else
        {
          // Do nothing
        }

        xmlWriter.writeStartElement("bookmark");
        xmlWriter.writeAttribute("profile", bookmark->profile());
        xmlWriter.writeAttribute("group", bookmark->groupName());

        xmlWriter.writeTextElement("workgroup", bookmark->workgroupName());
        xmlWriter.writeTextElement("unc", bookmark->unc());
        xmlWriter.writeTextElement("login", bookmark->login());
        xmlWriter.writeTextElement("ip", bookmark->hostIP());
        xmlWriter.writeTextElement("type", bookmark->typeString());
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


void Smb4KBookmarkHandler::readBookmarks()
{
  // Locate the XML file.
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
             (xmlReader.attributes().value("version") != "1.0" && xmlReader.attributes().value("version") != "1.1"))
        {
          xmlReader.raiseError(i18n("The format of %1 is not supported.", xmlFile.fileName()));
          break;
        }
        else
        {
          if (xmlReader.name() == "bookmark")
          {
            QString profile = xmlReader.attributes().value("profile").toString();
            
            Smb4KBookmark *bookmark = new Smb4KBookmark();
            bookmark->setProfile(profile);
            bookmark->setGroupName(xmlReader.attributes().value("group").toString());
              
            while (!(xmlReader.isEndElement() && xmlReader.name() == "bookmark"))
            {
              xmlReader.readNext();

              if (xmlReader.isStartElement())
              {
                if (xmlReader.name() == "workgroup")
                {
                  bookmark->setWorkgroupName(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "unc")
                {
                  bookmark->setURL(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "login")
                {
                  bookmark->setLogin(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "ip")
                {
                  bookmark->setHostIP(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "type")
                {
                  bookmark->setTypeString(xmlReader.readElementText());
                }
                else if (xmlReader.name() == "label")
                {
                  bookmark->setLabel(xmlReader.readElementText());
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
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (xmlFile.exists())
    {
      Smb4KNotification::openingFileFailed(xmlFile);
    }
    else
    {
      // Do nothing
    }
  }
  
  emit updated();
}


Smb4KBookmark *Smb4KBookmarkHandler::findBookmarkByUNC(const QString &unc)
{
  // Update the bookmarks:
  update();

  // Find the bookmark:
  Smb4KBookmark *bookmark = 0;

  for (Smb4KBookmark *b : bookmarksList())
  {
    if (QString::compare(b->unc().toUpper(), unc.toUpper()) == 0)
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


Smb4KBookmark *Smb4KBookmarkHandler::findBookmarkByLabel(const QString &label)
{
  // Update the bookmarks:
  update();

  // Find the bookmark:
  Smb4KBookmark *bookmark = 0;

  for (Smb4KBookmark *b : bookmarksList())
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


QList<Smb4KBookmark *> Smb4KBookmarkHandler::bookmarksList() const
{
  // Update the bookmarks:
  update();

  // Get this list of the bookmarks
  if (Smb4KSettings::useProfiles())
  {
    QList<Smb4KBookmark *> bookmarks;
    
    for (Smb4KBookmark *b : d->bookmarks)
    {
      if (b->profile() == Smb4KSettings::activeProfile())
      {
        bookmarks << b;
      }
      else
      {
        // Do nothing
      }
    }
    
    return bookmarks;
  }
  else
  {
    // Do nothing
  }
  
  // Return the list of bookmarks:
  return d->bookmarks;
}


QList<Smb4KBookmark *> Smb4KBookmarkHandler::bookmarksList(const QString &group) const
{
  // Update bookmarks
  update();

  // Get the list of bookmarks organized in the given group
  QList<Smb4KBookmark *> bookmarks;

  for (Smb4KBookmark *bookmark : bookmarksList())
  {
    if (QString::compare(group, bookmark->groupName(), Qt::CaseInsensitive) == 0)
    {
      bookmarks << bookmark;
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
  QStringList groups;
  
  for (Smb4KBookmark *b : bookmarksList())
  {
    if (!groups.contains(b->groupName()))
    {
      groups << b->groupName();
    }
    else
    {
      // Do nothing
    }
  }
  
  return groups;
}


void Smb4KBookmarkHandler::editBookmarks(QWidget *parent)
{
  if (!d->editor)
  {
    d->editor = new Smb4KBookmarkEditor(d->bookmarks, parent);
  }
  else
  {
    d->editor->raise();
  }
  
  if (d->editor->exec() == QDialog::Accepted)
  {
    // Now replace the current list with the new one that is
    // passed by the editor. For this set the 'replace' argument
    // for addBookmarks() to TRUE.
    QList<Smb4KBookmark *> bookmarks = d->editor->editedBookmarks();
    addBookmarks(bookmarks, true);
  }
  else
  {
    // Do nothing
  }

  delete d->editor;
  d->editor = 0;
}


void Smb4KBookmarkHandler::update() const
{
  // Get new IP addresses.
  for (Smb4KBookmark *bookmark : d->bookmarks)
  {
    HostPtr host = findHost(bookmark->hostName(), bookmark->workgroupName());
    
    if (host)
    {
      if (host->hasIP() && bookmark->hostIP() != host->ip())
      {
        bookmark->setHostIP(host->ip());
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


void Smb4KBookmarkHandler::migrateProfile(const QString& from, const QString& to)
{
  // Replace the old profile name with the new one.
  for (Smb4KBookmark *bookmark : d->bookmarks)
  {
    if (QString::compare(bookmark->profile(), from, Qt::CaseSensitive) == 0)
    {
      bookmark->setProfile(to);
    }
    else
    {
      // Do nothing
    }
  }
  
  // Write the new list to the file.
  writeBookmarkList();
  
  // Profile settings changed, so invoke the slot.
  slotActiveProfileChanged(Smb4KProfileManager::self()->activeProfile());
}


void Smb4KBookmarkHandler::removeProfile(const QString& name)
{
  QMutableListIterator<Smb4KBookmark *> it(d->bookmarks);
  
  while (it.hasNext())
  {
    Smb4KBookmark *bookmark = it.next();
    
    if (QString::compare(bookmark->profile(), name, Qt::CaseSensitive) == 0)
    {
      it.remove();
    }
    else
    {
      // Do nothing
    }
  }
  
  // Write the new list to the file.
  writeBookmarkList();
  
  // Profile settings changed, so invoke the slot.
  slotActiveProfileChanged(Smb4KProfileManager::self()->activeProfile());
}


void Smb4KBookmarkHandler::slotActiveProfileChanged(const QString &/*activeProfile*/)
{
  readBookmarks();
}

