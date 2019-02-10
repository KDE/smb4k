/***************************************************************************
    smb4ksystemtray  -  This is the system tray window class of Smb4K.
                             -------------------
    begin                : Mi Jun 13 2007
    copyright            : (C) 2007-2019 by Alexander Reinholdt
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
#include "smb4ksystemtray.h"
#include "smb4kbookmarkmenu.h"
#include "smb4ksharesmenu.h"
#include "smb4kprofilesmenu.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4kshare.h"
#include "core/smb4kglobal.h"
#include "core/smb4kmounter.h"
#include "core/smb4kclient.h"

// Qt includes
#include <QMenu>
#include <QDebug>

// KDE specific includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>
#include <KConfigWidgets/KStandardAction>
#include <KConfigWidgets/KConfigDialog>
#include <KXmlGui/KActionCollection>
#include <KCoreAddons/KPluginLoader>
#include <KCoreAddons/KPluginFactory>
#include <KCoreAddons/KAboutData>
#include <KWidgetsAddons/KMessageBox>

using namespace Smb4KGlobal;


Smb4KSystemTray::Smb4KSystemTray(QWidget *parent)
: KStatusNotifierItem("smb4k_systemtray", parent)
{
  //
  // Set the icon for the system tray
  //
  QString iconName = QStringLiteral("network-workgroup");
  setIconByName(iconName);
  
  //
  // Set the tooltip text
  //
  setToolTip(iconName, i18n("Smb4K"), KAboutData::applicationData().shortDescription());
  
  //
  // Set the status of the icon. By default, it is active. It will become passive, 
  // if the scanner could not find something and no shares were mounted.
  //
  setStatus(Active);
  
  //
  // Set the category
  //
  setCategory(ApplicationStatus);
  
  //
  // Add the actions to the action collection
  //
  QAction *mountAction = new QAction(KDE::icon("view-form", QStringList("emblem-mounted")), i18n("&Open Mount Dialog"), this);
  connect(mountAction, SIGNAL(triggered(bool)), SLOT(slotMountDialog()));
    
  addAction("shares_menu", new Smb4KSharesMenu(associatedWidget()));
  addAction("bookmarks_menu", new Smb4KBookmarkMenu(Smb4KBookmarkMenu::SystemTray, associatedWidget()));
  addAction("profiles_menu", new Smb4KProfilesMenu());
  addAction("mount_action", mountAction);
  addAction("config_action", KStandardAction::preferences(this, SLOT(slotConfigDialog()), this));
  
  //
  // Set up the menu
  //  
  contextMenu()->addAction(action("shares_menu"));
  contextMenu()->addAction(action("bookmarks_menu"));
  contextMenu()->addAction(action("profiles_menu"));
  contextMenu()->addSeparator();
  contextMenu()->addAction(action("mount_action"));
  contextMenu()->addAction(action("config_action"));
  
  // 
  // Connections
  // 
  connect(Smb4KMounter::self(), SIGNAL(mountedSharesListChanged()), SLOT(slotSetStatus()));
  connect(Smb4KClient::self(), SIGNAL(workgroups()), SLOT(slotSetStatus()));
}


Smb4KSystemTray::~Smb4KSystemTray()
{
}


void Smb4KSystemTray::loadSettings()
{
  //
  // Adjust the bookmarks menu
  //
  Smb4KBookmarkMenu *bookmarkMenu = static_cast<Smb4KBookmarkMenu *>(action("bookmarks_menu"));

  if (bookmarkMenu)
  {
    bookmarkMenu->refreshMenu();
  }

  // 
  // Adjust the shares menu
  //
  Smb4KSharesMenu *sharesMenu = static_cast<Smb4KSharesMenu *>(action("shares_menu"));

  if (sharesMenu)
  {
    sharesMenu->refreshMenu();
  }
  
  //
  // Adjust the profiles menu
  //
  Smb4KProfilesMenu *profilesMenu = static_cast<Smb4KProfilesMenu *>(action("profiles_menu"));
  
  if (profilesMenu)
  {
    profilesMenu->refreshMenu();
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSystemTray::slotMountDialog()
{
  Smb4KMounter::self()->openMountDialog();
}


void Smb4KSystemTray::slotConfigDialog()
{
  //
  // Check if the configuration dialog exists and try to show it.
  //
  if (KConfigDialog::exists("ConfigDialog"))
  {
    KConfigDialog::showDialog("ConfigDialog");
    return;
  }
  
  //
  // If the dialog does not exist, load and show it:
  //
  KPluginLoader loader("smb4kconfigdialog");
  KPluginFactory *configFactory = loader.factory();

  if (configFactory)
  {
    KConfigDialog *dlg = 0;
    
    if (associatedWidget())
    {
      dlg = configFactory->create<KConfigDialog>(associatedWidget());
      dlg->setObjectName("ConfigDialog");
    }
    else
    {
      dlg = configFactory->create<KConfigDialog>(contextMenu());
      dlg->setObjectName("ConfigDialog");
    }
    
    if (dlg)
    {
      dlg->setObjectName("ConfigDialog");
      connect(dlg, SIGNAL(settingsChanged(QString)), this, SLOT(slotSettingsChanged(QString)), Qt::UniqueConnection);
      connect(dlg, SIGNAL(settingsChanged(QString)), this, SIGNAL(settingsChanged(QString)), Qt::UniqueConnection);
      dlg->show();
    }
  }
  else
  {
    KMessageBox::error(0, "<qt>"+loader.errorString()+"</qt>");
    return;
  }
}

void Smb4KSystemTray::slotSettingsChanged(const QString &)
{
  // 
  // Execute loadSettings()
  // 
  loadSettings();
}


void Smb4KSystemTray::slotSetStatus()
{
  //
  // Set the status of the system tray icon
  // 
  if (!mountedSharesList().isEmpty() || !workgroupsList().isEmpty())
  {
    setStatus(KStatusNotifierItem::Active);
  }
  else
  {
    setStatus(KStatusNotifierItem::Passive);
  }
}


