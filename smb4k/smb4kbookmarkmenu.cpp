/***************************************************************************
    smb4kbookmarkmenu  -  Bookmark menu
                             -------------------
    begin                : Sat Apr 02 2011
    copyright            : (C) 2011-2018 by Alexander Reinholdt
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
#include "smb4kbookmarkmenu.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kshare.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QDebug>
#include <QMenu>
#include <QLatin1String>

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>

using namespace Smb4KGlobal;


Smb4KBookmarkMenu::Smb4KBookmarkMenu(int type, QWidget *parentWidget, QObject *parent)
: KActionMenu(KDE::icon("folder-favorites"), i18n("Bookmarks"), parent), m_type(type),
  m_parent_widget(parentWidget)
{
  // The action collection
  m_action_collection = new KActionCollection(this);
  
  // Set up action group for the bookmark groups
  m_groups = new QActionGroup(m_action_collection);

  // Set up action group for the bookmarks
  m_bookmarks = new QActionGroup(m_action_collection);

  // Set up the menu
  setupMenu();

  // Connections
  connect(Smb4KBookmarkHandler::self(), SIGNAL(updated()), SLOT(slotBookmarksUpdated()));
  connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)), SLOT(slotDisableBookmark(SharePtr)));
  connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)), SLOT(slotEnableBookmark(SharePtr)));
  connect(m_groups, SIGNAL(triggered(QAction*)), SLOT(slotGroupActionTriggered(QAction*)));
  connect(m_bookmarks, SIGNAL(triggered(QAction*)), SLOT(slotBookmarkActionTriggered(QAction*)));
}


Smb4KBookmarkMenu::~Smb4KBookmarkMenu()
{
}


QAction *Smb4KBookmarkMenu::addBookmarkAction()
{
  return m_action_collection->action("add_action");
}


void Smb4KBookmarkMenu::refreshMenu()
{
  //
  // Delete all actions
  //
  while (!m_groups->actions().isEmpty())
  {
    delete m_groups->actions().takeFirst();
  }
  
  while (!m_bookmarks->actions().isEmpty())
  {
    delete m_bookmarks->actions().takeFirst();
  }
  
  menu()->clear();
  
  //
  // Set up the menu
  //
  setupMenu();
}


void Smb4KBookmarkMenu::setupMenu()
{
  //
  // Depending on the type chosen, some global actions need to be inserted
  // into the menu. These actions are always enabled.
  //
  switch (m_type)
  {
    case MainWindow:
    {
      QAction *editBookmarksAction = 0;
      
      if (!m_action_collection->action("edit_action"))
      {
        editBookmarksAction = new QAction(KDE::icon("bookmarks-organize"), i18n("&Edit Bookmarks"), this);
        m_action_collection->addAction("edit_action", editBookmarksAction);
        connect(editBookmarksAction, SIGNAL(triggered(bool)), SLOT(slotEditActionTriggered(bool)));
      }
      else
      {
        editBookmarksAction = m_action_collection->action("edit_action");
      }
      
      QAction *addBookmarkAction = 0;
      
      if (!m_action_collection->action("add_action"))
      {
        addBookmarkAction = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), this);
        addBookmarkAction->setObjectName("add_action");
        m_action_collection->addAction("add_action", addBookmarkAction);
        connect(addBookmarkAction, SIGNAL(triggered(bool)), SLOT(slotAddActionTriggered(bool)));
      }
      else
      {
        addBookmarkAction = m_action_collection->action("add_action");
      }
      
      addAction(editBookmarksAction);
      addAction(addBookmarkAction);
      
      break;
    }
    case SystemTray:
    {
      QAction *editBookmarksAction = 0;
      
      if (!m_action_collection->action("edit_action"))
      {
        editBookmarksAction = new QAction(KDE::icon("bookmarks-organize"), i18n("&Edit Bookmarks"), this);
        m_action_collection->addAction("edit_action", editBookmarksAction);
        connect(editBookmarksAction, SIGNAL(triggered(bool)), SLOT(slotEditActionTriggered(bool)));
      }
      else
      {
        editBookmarksAction = m_action_collection->action("edit_action");
      }
      
      addAction(editBookmarksAction);
      
      break;      
    }
    default:
    {
      break;
    }
  }
  
  //
  // Get the list of groups
  //
  QStringList allGroups = Smb4KBookmarkHandler::self()->groupsList();
  
  //
  // Insert a toplevel mount action, if necessary. Crucial for this is that there are 
  // no (non-empty) groups defined. Enable it if not all toplevel bookmarks are mounted.
  //
  if (allGroups.isEmpty() || (allGroups.size() == 1 && allGroups.first().isEmpty()))
  {
    QAction *toplevelMount = 0;
    
    if (!m_action_collection->action("toplevel_mount"))
    {
      toplevelMount = new QAction(KDE::icon("media-mount"), i18n("Mount All Bookmarks"), this);
      m_action_collection->addAction("toplevel_mount", toplevelMount);
      connect(toplevelMount, SIGNAL(triggered(bool)), SLOT(slotToplevelMountActionTriggered(bool)));
    }
    else
    {
      toplevelMount = m_action_collection->action("toplevel_mount");
    }
    
    addAction(toplevelMount);
    
    QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList();
    int mountedBookmarks = 0;
    
    for (const BookmarkPtr &bookmark : bookmarks)
    {
      QList<SharePtr> mountedShares = findShareByUNC(bookmark->unc());
      
      if (!mountedShares.isEmpty())
      {
        for (const SharePtr &share : mountedShares)
        {
          if (!share->isForeign())
          {
            mountedBookmarks++;
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
    }
    
    toplevelMount->setEnabled(mountedBookmarks != bookmarks.size());
  }
  else
  {
    // Do nothing
  }
  
  //
  // Add a separator
  //
  (void) addSeparator();
  
  //
  // Now add the groups and their bookmarks
  //
  for (const QString &group : allGroups)
  {
    if (!group.isEmpty())
    {
      // Group menu entry
      QAction *bookmarkGroup = new QAction(KDE::icon("folder-favorites"), group, m_groups);
      addAction(bookmarkGroup);
      m_action_collection->addAction(QString("group_%1").arg(group));
      
      // Group submenu
      KActionMenu *bookmarkGroupMenu = new KActionMenu(bookmarkGroup);
      bookmarkGroup->setMenu(bookmarkGroupMenu->menu());
      
      // Mount action for the group
      QAction *bookmarkGroupMount = new QAction(KDE::icon("media-mount"), i18n("Mount All Bookmarks"), m_groups);
      bookmarkGroupMount->setData(group);
      bookmarkGroupMount->setObjectName(QString("%1_mount_action").arg(group));
      bookmarkGroupMenu->addAction(bookmarkGroupMount);
      m_action_collection->addAction(QString("%1_mount_action").arg(group));

      // Get the list of bookmarks belonging to this group.
      // Use it to decide whether the group mount action should be enabled 
      // (only if not all bookmarks belonging to this group are mounted) and
      // to sort the bookmarks.      
      QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList(group);
      QStringList sortedBookmarks;
      int mountedBookmarks = 0;
      
      for (const BookmarkPtr &bookmark : bookmarks)
      {
        QAction *bookmarkAction = 0;
        
        if (Smb4KSettings::showCustomBookmarkLabel() && !bookmark->label().isEmpty())
        {
          bookmarkAction = new QAction(bookmark->icon(), bookmark->label(), m_bookmarks);
          QMap<QString,QVariant> info;
          info["group"] = group;
          info["unc"] = bookmark->unc();
          bookmarkAction->setData(info);
          sortedBookmarks << bookmark->label();
          m_action_collection->addAction(bookmark->unc(), bookmarkAction);
        }
        else
        {
          bookmarkAction = new QAction(bookmark->icon(), bookmark->displayString(), m_bookmarks);
          QMap<QString,QVariant> info;
          info["group"] = group;
          info["unc"] = bookmark->unc();
          bookmarkAction->setData(info);
          sortedBookmarks << bookmark->displayString();
          m_action_collection->addAction(bookmark->unc(), bookmarkAction);
        }
        
        QList<SharePtr> mountedShares = findShareByUNC(bookmark->unc());
        
        if (!mountedShares.isEmpty())
        {
          for (const SharePtr &share : mountedShares)
          {
            if (!share->isForeign())
            {
              bookmarkAction->setEnabled(false);
              mountedBookmarks++;
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
      }
      
      bookmarkGroupMount->setEnabled(mountedBookmarks != bookmarks.size());
      sortedBookmarks.sort();
      
      // Add a separator
      bookmarkGroupMenu->addSeparator();
      
      // Insert the sorted bookmarks into the group menu
      QList<QAction *> actions = m_bookmarks->actions();
      
      for (const QString &b : sortedBookmarks)
      {
        for (QAction *a : actions)
        {
          if (a->text() == b)
          {
            bookmarkGroupMenu->addAction(a);
            break;
          }
          else
          {
            continue;
          }
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Add all bookmarks that have no group
  // Sort the bookmarks before.  
  //
  QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList("");
  QStringList sortedBookmarks;
  
  for (const BookmarkPtr &bookmark : bookmarks)
  {
    QAction *bookmarkAction = 0;
        
    if (Smb4KSettings::showCustomBookmarkLabel() && !bookmark->label().isEmpty())
    {
      bookmarkAction = new QAction(bookmark->icon(), bookmark->label(), m_bookmarks);
      QMap<QString,QVariant> info;
      info["group"] = "";
      info["unc"] = bookmark->unc();
      bookmarkAction->setData(info);
      sortedBookmarks << bookmark->label();
      m_action_collection->addAction(bookmark->unc(), bookmarkAction);
    }
    else
    {
      bookmarkAction = new QAction(bookmark->icon(), bookmark->displayString(), m_bookmarks);
      QMap<QString,QVariant> info;
      info["group"] = "";
      info["unc"] = bookmark->unc();
      bookmarkAction->setData(info);
      sortedBookmarks << bookmark->displayString();
      m_action_collection->addAction(bookmark->unc(), bookmarkAction);
    }
        
    QList<SharePtr> mountedShares = findShareByUNC(bookmark->unc());
        
    if (!mountedShares.isEmpty())
    {
      for (const SharePtr &share : mountedShares)
      {
        if (!share->isForeign())
        {
          bookmarkAction->setEnabled(false);
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
  }
  
  sortedBookmarks.sort();
  
  QList<QAction *> actions = m_bookmarks->actions();
      
  for (const QString &b : sortedBookmarks)
  {
    for (QAction *a : actions)
    {
      if (a->text() == b)
      {
        addAction(a);
        break;
      }
      else
      {
        continue;
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KBookmarkMenu::slotEditActionTriggered(bool /*checked*/)
{
  //
  // Edit the bookmarks
  //
  Smb4KBookmarkHandler::self()->editBookmarks(m_parent_widget);
}


void Smb4KBookmarkMenu::slotAddActionTriggered(bool /*checked*/)
{
  //
  // Use addBookmarkAction() and connect the triggered() signal
  // in your application.
  //
}


void Smb4KBookmarkMenu::slotToplevelMountActionTriggered(bool /*checked*/)
{
  //
  // Mount all top level bookmarks.
  // This slot will only be called if there are no groups defined.
  //
  QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList();
  QList<SharePtr> mounts;

  for (const BookmarkPtr &bookmark : bookmarks)
  {
    // FIXME: Check if the bookmarked share has already been mounted.
    SharePtr share = SharePtr(new Smb4KShare());
    share->setHostName(bookmark->hostName());
    share->setShareName(bookmark->shareName());
    share->setWorkgroupName(bookmark->workgroupName());
    share->setHostIP(bookmark->hostIP());
    share->setLogin(bookmark->login());
    mounts << share;
  }

  Smb4KMounter::self()->mountShares(mounts);

  while (!mounts.isEmpty())
  {
    mounts.takeFirst().clear();
  }
}


void Smb4KBookmarkMenu::slotGroupActionTriggered(QAction *action)
{
  if (action->objectName().endsWith(QLatin1String("_mount_action")))
  {
    //
    // Mount all bookmarks of one group
    //
    QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList(action->data().toString());
    QList<SharePtr> mounts;

    for (const BookmarkPtr &bookmark : bookmarks)
    {
      // FIXME: Check if the bookmarked share has already been mounted.
      SharePtr share = SharePtr(new Smb4KShare());
      share->setHostName(bookmark->hostName());
      share->setShareName(bookmark->shareName());
      share->setWorkgroupName(bookmark->workgroupName());
      share->setHostIP(bookmark->hostIP());
      share->setLogin(bookmark->login());
      mounts << share;
    }

    Smb4KMounter::self()->mountShares(mounts);

    while (!mounts.isEmpty())
    {
      mounts.takeFirst().clear();
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkMenu::slotBookmarkActionTriggered(QAction *action)
{
  QMap<QString,QVariant> info = action->data().toMap();
  QString bookmarkGroup = info.value("group").toString();
  QString unc = info.value("unc").toString();
  
  BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC(unc);

  if (bookmark && bookmarkGroup == bookmark->groupName())
  {
    SharePtr share = SharePtr(new Smb4KShare());
    share->setHostName(bookmark->hostName());
    share->setShareName(bookmark->shareName());
    share->setWorkgroupName(bookmark->workgroupName());
    share->setHostIP(bookmark->hostIP());
    share->setLogin(bookmark->login());
    Smb4KMounter::self()->mountShare(share);
    share.clear();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkMenu::slotBookmarksUpdated()
{
  refreshMenu();
}


void Smb4KBookmarkMenu::slotDisableBookmark(const SharePtr &share)
{
  if (!share->isForeign() && !m_bookmarks->actions().isEmpty())
  {
    //
    // Disable the bookmark
    //
    QList<QAction *> actions = m_bookmarks->actions();
    QString bookmarkGroup;
    
    for (QAction *a : actions)
    {
      if (share->unc() == a->data().toMap().value("unc").toString())
      {
        a->setEnabled(!share->isMounted());
        bookmarkGroup = a->data().toMap().value("group").toString();
        break;
      }
      else
      {
        continue;
      }
    }
    
    //
    // Check if all bookmarks belonging to this group are 
    // mounted. Disable the respective mount action if necessary.
    //
    bool allMounted = true;
      
    for (QAction *a : actions)
    {
      if (a->data().toMap().value("group").toString() == bookmarkGroup && a->isEnabled())
      {
        allMounted = false;
        break;
      }
      else
      {
        continue;
      }
    }
      
    QList<QAction *> allActions = m_action_collection->actions();
      
    for (QAction *a : allActions)
    {
      if (a->objectName() == "toplevel_mount_action" && bookmarkGroup.isEmpty())
      {
        a->setEnabled(!allMounted);
        break;
      }
      else if (a->objectName() == QString("%1_mount_action").arg(bookmarkGroup))
      {
        a->setEnabled(!allMounted);
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
}


void Smb4KBookmarkMenu::slotEnableBookmark(const SharePtr &share)
{
  if (!share->isForeign() && !m_bookmarks->actions().isEmpty())
  {
    //
    // Disable the bookmark
    //
    QList<QAction *> actions = m_bookmarks->actions();
    QString bookmarkGroup;
    
    for (QAction *a : actions)
    {
      if (share->unc() == a->data().toMap().value("unc").toString())
      {
        a->setEnabled(!share->isMounted());
        bookmarkGroup = a->data().toMap().value("group").toString();
        break;
      }
      else
      {
        continue;
      }
    }
    
    //
    // Check if all bookmarks belonging to this group are 
    // mounted. Enable the respective mount action if necessary.
    //
    bool allMounted = true;
      
    for (QAction *a : actions)
    {
      if (a->data().toMap().value("group").toString() == bookmarkGroup && a->isEnabled())
      {
        allMounted = false;
        break;
      }
      else
      {
        continue;
      }
    }
      
    QList<QAction *> allActions = m_action_collection->actions();
      
    for (QAction *a : allActions)
    {
      if (a->objectName() == "toplevel_mount_action" && bookmarkGroup.isEmpty())
      {
        a->setEnabled(!allMounted);
        break;
      }
      else if (a->objectName() == QString("%1_mount_action").arg(bookmarkGroup))
      {
        a->setEnabled(!allMounted);
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
}

