/***************************************************************************
    smb4ksharesmenu  -  Shares menu
                             -------------------
    begin                : Mon Sep 05 2011
    copyright            : (C) 2011-2019 by Alexander Reinholdt
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
#include "smb4ksharesmenu.h"
#include "core/smb4kshare.h"
#include "core/smb4kmounter.h"
#include "core/smb4kglobal.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kbookmarkhandler.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QMap>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QMenu>

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>

using namespace Smb4KGlobal;


Smb4KSharesMenu::Smb4KSharesMenu(QWidget *parentWidget, QObject *parent)
: KActionMenu(KDE::icon("folder-network", QStringList("emblem-mounted")), i18n("Mounted Shares"), parent),
  m_parent_widget(parentWidget)
{
  // 
  // Set up action group for the shares menus
  // 
  m_menus = new QActionGroup(menu());

  // 
  // Set up action group for the shares actions
  // 
  m_actions = new QActionGroup(menu());

  // 
  // Setup the menu
  // 
  setupMenu();

  //
  // Connections
  // 
  connect(m_actions, SIGNAL(triggered(QAction*)), SLOT(slotShareAction(QAction*)));
  connect(Smb4KMounter::self(), SIGNAL(mountedSharesListChanged()), SLOT(slotMountedSharesListChanged()));
}


Smb4KSharesMenu::~Smb4KSharesMenu()
{
}


void Smb4KSharesMenu::refreshMenu()
{
  //
  // Delete all entries from the menu
  //
  while (!menu()->actions().isEmpty())
  {
    QAction *action = menu()->actions().takeFirst();
    removeAction(action);
    delete action;
  }
  
  //
  // Clear the rest of the menu
  //
  if (!menu()->isEmpty())
  {
    menu()->clear();
  }
  
  //
  // Set up the menu
  // 
  setupMenu();
  
  //
  // Make sure the correct menu entries are shown
  //
  menu()->update();
}


void Smb4KSharesMenu::setupMenu()
{
  //
  // Add the Unmount All action
  //
  QAction *unmount_all = new QAction(KDE::icon("system-run"), i18n("U&nmount All"), menu());
  unmount_all->setEnabled(false);
  connect(unmount_all, SIGNAL(triggered(bool)), SLOT(slotUnmountAllShares()));
  addAction(unmount_all);

  // 
  // Add a separator
  //
  addSeparator();

  //
  // Add the share entries
  // 
  QStringList displayNames;
  
  for (const SharePtr &share : mountedSharesList())
  {
    // Do not process null pointers
    if (!share)
    {
      continue;
    }
    
    // Add the display name to the list
    displayNames << share->displayString();
    
    // Create the share menu
    KActionMenu *shareMenu = new KActionMenu(share->displayString(), menu());
    shareMenu->setIcon(share->icon());
    
    QMap<QString,QVariant> data;
    data["text"] = share->displayString();
    
    shareMenu->setData(data);
    m_menus->addAction(shareMenu);
    
    // Add the unmount action to the menu
    QAction *unmount = new QAction(KDE::icon("media-eject"), i18n("Unmount"), shareMenu->menu());
    
    QMap<QString,QVariant> unmountData;
    unmountData["type"] = "unmount";
    unmountData["mountpoint"] = share->path();
    
    unmount->setData(unmountData);
    unmount->setEnabled(!share->isForeign() || Smb4KMountSettings::unmountForeignShares());
    shareMenu->addAction(unmount);
    m_actions->addAction(unmount);
    
    // Add a separator
    shareMenu->addSeparator();
    
    // Add the bookmark action to the menu
    QAction *addBookmark = new QAction(KDE::icon("bookmark-new"), i18n("Add Bookmark"), shareMenu->menu());
    
    QMap<QString,QVariant> bookmarkData;
    bookmarkData["type"] = "bookmark";
    bookmarkData["mountpoint"] = share->path();
    
    addBookmark->setData(bookmarkData);
    shareMenu->addAction(addBookmark);
    m_actions->addAction(addBookmark);
    
    // Add the synchronization action to the menu
    QAction *synchronize = new QAction(KDE::icon("folder-sync"), i18n("Synchronize"), shareMenu->menu());
    
    QMap<QString,QVariant> syncData;
    syncData["type"] = "sync";
    syncData["mountpoint"] = share->path();
    
    synchronize->setData(syncData);
    synchronize->setEnabled(!QStandardPaths::findExecutable("rsync").isEmpty() && !share->isInaccessible());
    shareMenu->addAction(synchronize);
    m_actions->addAction(synchronize);
    
    // Add a separator
    shareMenu->addSeparator();
    
    // Add the Open with Konsole action to the menu
    QAction *konsole = new QAction(KDE::icon("utilities-terminal"), i18n("Open with Konsole"), shareMenu->menu());
    
    QMap<QString,QVariant> konsoleData;
    konsoleData["type"] = "konsole";
    konsoleData["mountpoint"] = share->path();
    
    konsole->setData(konsoleData);
    konsole->setEnabled(!QStandardPaths::findExecutable("konsole").isEmpty() && !share->isInaccessible());
    shareMenu->addAction(konsole);
    m_actions->addAction(konsole);
    
    // Add the Open with Filemanager action to the menu
    QAction *filemanager = new QAction(KDE::icon("system-file-manager"), i18n("Open with File Manager"), shareMenu->menu());
    
    QMap<QString,QVariant> fmData;
    fmData["type"] = "filemanager";
    fmData["mountpoint"] = share->path();
    
    filemanager->setData(fmData);
    filemanager->setEnabled(!share->isInaccessible());
    shareMenu->addAction(filemanager);
    m_actions->addAction(filemanager);
  }
  
  //
  // Sort the display names and add the share menus to the menu
  // in the sorted order.
  // 
  displayNames.sort();
  
  for (const QString &name : displayNames)
  {
    for (QAction *action : m_menus->actions())
    {
      if (action->data().toMap().value("text").toString() == name)
      {
        addAction(action);
        break;
      }
    }
  }
  
  //
  // Enable or disable the Unmount All action, depending on the number of 
  // mounted shares present.
  //
  unmount_all->setEnabled(((!onlyForeignMountedShares() || Smb4KMountSettings::unmountForeignShares()) && !m_menus->actions().isEmpty()));
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSharesMenu::slotMountedSharesListChanged()
{
  //
  // Refresh the menu
  //
  refreshMenu();
}


void Smb4KSharesMenu::slotUnmountAllShares()
{
  Smb4KMounter::self()->unmountAllShares(false);
}


void Smb4KSharesMenu::slotShareAction(QAction *action)
{
  //
  // Create a share
  //
  SharePtr share;
  
  // 
  // Check that we have a share related action
  // 
  if (action->data().toMap().contains("type"))
  {
    share = findShareByPath(action->data().toMap().value("mountpoint").toString());
  }
  
  //
  // Now process the action
  //
  if (share)
  {
    QString type = action->data().toMap().value("type").toString();
    QString mountpoint = action->data().toMap().value("mountpoint").toString();
    
    if (type == "unmount")
    {
      Smb4KMounter::self()->unmountShare(share, false);
    }
    else if (type == "bookmark")
    {
      Smb4KBookmarkHandler::self()->addBookmark(share);
    }
    else if (type == "sync")
    {
      Smb4KSynchronizer::self()->synchronize(share);
    }
    else if (type == "konsole")
    {
      openShare(share, Smb4KGlobal::Konsole);
    }
    else if (type == "filemanager")
    {
      openShare(share, Smb4KGlobal::FileManager);
    }
  }
}

