/***************************************************************************
    smb4knetworkbrowser_part  -  This Part encapsulates the network
    browser of Smb4K.
                             -------------------
    begin                : Fr Jan 5 2007
    copyright            : (C) 2007-2016 by Alexander Reinholdt
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
#include "smb4knetworkbrowser_part.h"
#include "smb4knetworkbrowser.h"
#include "smb4knetworkbrowseritem.h"
#include "smb4ktooltip.h"
#include "core/smb4kglobal.h"
#include "core/smb4ksettings.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kauthinfo.h"
#include "core/smb4kscanner.h"
#include "core/smb4kmounter.h"
#include "core/smb4kprint.h"
#include "core/smb4kpreviewer.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kcustomoptions.h"

// Qt includes
#include <QEvent>
#include <QDebug>
#include <QKeySequence>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QHeaderView>
#include <QApplication>
#include <QMenu>

// KDE includes
#include <KCoreAddons/KPluginFactory>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KDualAction>
#include <KWidgetsAddons/KGuiItem>
#include <KI18n/KLocalizedString>
#include <KXmlGui/KActionCollection>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY(Smb4KNetworkBrowserPartFactory, registerPlugin<Smb4KNetworkBrowserPart>();)


Smb4KNetworkBrowserPart::Smb4KNetworkBrowserPart(QWidget *parentWidget, QObject *parent, const QList<QVariant> &args)
: KParts::Part(parent), m_bookmark_shortcut(true), m_silent(false)
{
  // Parse arguments:
  for (int i = 0; i < args.size(); ++i)
  {
    if (args.at(i).toString().startsWith(QLatin1String("bookmark_shortcut")))
    {
      if (QString::compare(args.at(i).toString().section('=', 1, 1).trimmed(), "\"false\"") == 0)
      {
        m_bookmark_shortcut = false;
      }
      else
      {
        // Do nothing
      }

      continue;
    }
    else if (args.at(i).toString().startsWith(QLatin1String("silent")))
    {
      if (QString::compare(args.at(i).toString().section('=', 1, 1).trimmed(), "\"true\"") == 0)
      {
        m_silent = true;
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

  // Set the XML file:
  setXMLFile("smb4knetworkbrowser_part.rc");

  // Set the widget of this part:
  m_widget = new Smb4KNetworkBrowser(parentWidget);
  
  int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
  m_widget->setIconSize(QSize(icon_size, icon_size));
  
  setWidget(m_widget);

  // Set up the actions.
  // Do not put this before setWidget() or the shortcuts of the
  // actions will not be shown.
  setupActions();

  // Load the settings
  loadSettings();

  // Add some connections:
  connect(m_widget, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(slotContextMenuRequested(QPoint)));
  
  connect(m_widget, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotItemSelectionChanged()));
  
  connect(m_widget, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
          this, SLOT(slotItemPressed(QTreeWidgetItem*,int)));
  
  connect(m_widget, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
          this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
  
  connect(Smb4KScanner::self(), SIGNAL(workgroups(QList<Smb4KWorkgroup*>)),
          this, SLOT(slotWorkgroups(QList<Smb4KWorkgroup*>)));
  
  connect(Smb4KScanner::self(), SIGNAL(hosts(Smb4KWorkgroup*,QList<Smb4KHost*>)),
          this, SLOT(slotWorkgroupMembers(Smb4KWorkgroup*,QList<Smb4KHost*>)));
  
  connect(Smb4KScanner::self(), SIGNAL(shares(Smb4KHost*,QList<Smb4KShare*>)),
          this, SLOT(slotShares(Smb4KHost*,QList<Smb4KShare*>)));
  
  connect(Smb4KScanner::self(), SIGNAL(authError(Smb4KHost*,int)),
          this, SLOT(slotAuthError(Smb4KHost*,int)));
  
  connect(Smb4KScanner::self(), SIGNAL(aboutToStart(Smb4KBasicNetworkItem*,int)),
          this, SLOT(slotScannerAboutToStart(Smb4KBasicNetworkItem*,int)));
  
  connect(Smb4KScanner::self(), SIGNAL(finished(Smb4KBasicNetworkItem*,int)),
          this, SLOT(slotScannerFinished(Smb4KBasicNetworkItem*,int)));
  
  connect(Smb4KScanner::self(), SIGNAL(ipAddress(Smb4KHost*)),
          this, SLOT(slotAddIPAddress(Smb4KHost*)));
  
  connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)),
          this, SLOT(slotMounterAboutToStart(int)));
  
  connect(Smb4KMounter::self(), SIGNAL(finished(int)),
          this, SLOT(slotMounterFinished(int)));
  
  connect(Smb4KMounter::self(), SIGNAL(mounted(Smb4KShare*)),
          this, SLOT(slotShareMounted(Smb4KShare*)));
  
  connect(Smb4KMounter::self(), SIGNAL(unmounted(Smb4KShare*)),
          this, SLOT(slotShareUnmounted(Smb4KShare*)));
  
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
          this, SLOT(slotAboutToQuit()));
  
  connect(KIconLoader::global(), SIGNAL(iconChanged(int)),
          this, SLOT(slotIconSizeChanged(int)));
}


Smb4KNetworkBrowserPart::~Smb4KNetworkBrowserPart()
{
}


void Smb4KNetworkBrowserPart::setupActions()
{
  KDualAction *rescan_abort_action = new KDualAction(this);
  KGuiItem rescan_item(i18n("Scan Netwo&rk"), KDE::icon("view-refresh"));
  KGuiItem abort_item(i18n("&Abort"), KDE::icon("process-stop"));
  rescan_abort_action->setActiveGuiItem(rescan_item);
  rescan_abort_action->setInactiveGuiItem(abort_item);
  rescan_abort_action->setActive(true);
  rescan_abort_action->setAutoToggle(false);
  connect(rescan_abort_action, SIGNAL(triggered(bool)), this, SLOT(slotRescanAbortActionTriggered(bool)));
  
  QAction *manual_action = new QAction(KDE::icon("view-form", QStringList("emblem-mounted")), i18n("&Open Mount Dialog"), this);
  connect(manual_action, SIGNAL(triggered(bool)), this, SLOT(slotMountManually(bool)));

  QAction *auth_action = new QAction(KDE::icon("dialog-password"), i18n("Au&thentication"), this);
  connect(auth_action, SIGNAL(triggered(bool)), this, SLOT(slotAuthentication(bool)));

  QAction *custom_action = new QAction(KDE::icon("preferences-system-network"), i18n("&Custom Options"), this);
  connect(custom_action, SIGNAL(triggered(bool)), this, SLOT(slotCustomOptions(bool)));

  QAction *bookmark_action = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), this);
  connect(bookmark_action, SIGNAL(triggered(bool)), this, SLOT(slotAddBookmark(bool)));

  QAction *preview_action  = new QAction(KDE::icon("view-list-icons"), i18n("Pre&view"), this);
  connect(preview_action, SIGNAL(triggered(bool)), this, SLOT(slotPreview(bool)));

  QAction *print_action = new QAction(KDE::icon("printer"), i18n("&Print File"), this);
  connect(print_action, SIGNAL(triggered(bool)), this, SLOT(slotPrint(bool)));

  KDualAction *mount_action = new KDualAction(this);
  KGuiItem mount_item(i18n("&Mount"), KDE::icon("emblem-mounted"));
  KGuiItem unmount_item(i18n("&Unmount"), KDE::icon("emblem-unmounted"));
  mount_action->setActiveGuiItem(mount_item);
  mount_action->setInactiveGuiItem(unmount_item);
  mount_action->setActive(true);
  mount_action->setAutoToggle(false);
  connect(mount_action, SIGNAL(triggered(bool)), this, SLOT(slotMountActionTriggered(bool)));
  connect(mount_action, SIGNAL(activeChanged(bool)), this, SLOT(slotMountActionChanged(bool)));
  
  // Add the action to the action collection
  actionCollection()->addAction("rescan_abort_action", rescan_abort_action);
  actionCollection()->addAction("mount_manually_action", manual_action);
  actionCollection()->addAction("authentication_action", auth_action);
  actionCollection()->addAction("custom_action", custom_action);
  actionCollection()->addAction("bookmark_action", bookmark_action);
  actionCollection()->addAction("preview_action", preview_action);
  actionCollection()->addAction("print_action", print_action);
  actionCollection()->addAction("mount_action", mount_action);
  
  // Set the shortcuts
  actionCollection()->setDefaultShortcut(rescan_abort_action, QKeySequence::Refresh);
  actionCollection()->setDefaultShortcut(manual_action, QKeySequence(Qt::CTRL+Qt::Key_O));
  actionCollection()->setDefaultShortcut(auth_action, QKeySequence(Qt::CTRL+Qt::Key_T));
  actionCollection()->setDefaultShortcut(custom_action, QKeySequence(Qt::CTRL+Qt::Key_C));
  
  if (m_bookmark_shortcut)
  {
    actionCollection()->setDefaultShortcut(bookmark_action, QKeySequence(Qt::CTRL+Qt::Key_B));
  }
  else
  {
    // Do nothing
  }
  
  actionCollection()->setDefaultShortcut(preview_action, QKeySequence(Qt::CTRL+Qt::Key_V));
  actionCollection()->setDefaultShortcut(print_action, QKeySequence(Qt::CTRL+Qt::Key_P));
  actionCollection()->setDefaultShortcut(mount_action, QKeySequence(Qt::CTRL+Qt::Key_M));

  // Enable/disable the actions:
  rescan_abort_action->setEnabled(true);
  manual_action->setEnabled(true);
  auth_action->setEnabled(false);
  custom_action->setEnabled(false);
  bookmark_action->setEnabled(false);
  preview_action->setEnabled(false);
  print_action->setEnabled(false);
  mount_action->setEnabled(false);

  // Plug the actions into the action menu:
  m_menu = new KActionMenu(this);
  m_menu->menu()->setTitle(i18n("Network"));
  m_menu->menu()->setIcon(KDE::icon("network-workgroup"));
  m_menu->addAction(rescan_abort_action);
  m_menu->addSeparator();
  m_menu->addAction(bookmark_action);
  m_menu->addAction(manual_action);
  m_menu->addSeparator();
  m_menu->addAction(auth_action);
  m_menu->addAction(custom_action);
  m_menu->addAction(preview_action);
  m_menu->addAction(print_action);
  m_menu->addAction(mount_action);
}


void Smb4KNetworkBrowserPart::loadSettings()
{
  // Show/hide columns:
  m_widget->setColumnHidden(Smb4KNetworkBrowser::IP, !Smb4KSettings::showIPAddress());
  m_widget->setColumnHidden(Smb4KNetworkBrowser::Type, !Smb4KSettings::showType());
  m_widget->setColumnHidden(Smb4KNetworkBrowser::Comment, !Smb4KSettings::showComment());

  // Load and apply the positions of the columns.
  KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");

  QMap<int, int> map;
  map.insert(configGroup.readEntry("ColumnPositionNetwork", (int)Smb4KNetworkBrowser::Network), Smb4KNetworkBrowser::Network);
  map.insert(configGroup.readEntry("ColumnPositionType", (int)Smb4KNetworkBrowser::Type), Smb4KNetworkBrowser::Type);
  map.insert(configGroup.readEntry("ColumnPositionIP", (int)Smb4KNetworkBrowser::IP), Smb4KNetworkBrowser::IP);
  map.insert(configGroup.readEntry("ColumnPositionComment", (int)Smb4KNetworkBrowser::Comment), Smb4KNetworkBrowser::Comment);

  QMap<int, int>::const_iterator it = map.constBegin();

  while (it != map.constEnd())
  {
    if (it.key() != m_widget->header()->visualIndex(it.value()))
    {
      m_widget->header()->moveSection(m_widget->header()->visualIndex(it.value()), it.key());
    }
    else
    {
      // Do nothing
    }

    ++it;
  }

  // Does anything has to be changed with the marked shares?
  for (int i = 0; i < mountedSharesList().size(); ++i)
  {
    // We do not need to use slotShareUnmounted() here, too,
    // because slotShareMounted() will take care of everything
    // we need here.
    slotShareMounted(mountedSharesList().at(i));
  }
}


void Smb4KNetworkBrowserPart::saveSettings()
{
  // Save the position of the columns.
  KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");
  configGroup.writeEntry("ColumnPositionNetwork", m_widget->header()->visualIndex(Smb4KNetworkBrowser::Network));
  configGroup.writeEntry("ColumnPositionType", m_widget->header()->visualIndex(Smb4KNetworkBrowser::Type));
  configGroup.writeEntry("ColumnPositionIP", m_widget->header()->visualIndex(Smb4KNetworkBrowser::IP));
  configGroup.writeEntry("ColumnPositionComment", m_widget->header()->visualIndex(Smb4KNetworkBrowser::Comment));

  configGroup.sync();
}


KAboutData *Smb4KNetworkBrowserPart::createAboutData()
{
  KAboutData *aboutData = new KAboutData(QStringLiteral("smb4knetworkbrowserpart"), i18n("Smb4KNetworkBrowserPart"),
    QStringLiteral("4.0"), i18n("The network neighborhood browser KPart of Smb4K"), KAboutLicense::GPL_V2, 
    i18n("\u00A9 2007-2015, Alexander Reinholdt"), QString(), QStringLiteral("http://smb4k.sourceforge.net"), 
    QStringLiteral("smb4k-bugs@lists.sourceforge.net"));
  
  return aboutData;
}


void Smb4KNetworkBrowserPart::customEvent(QEvent *e)
{
  if (e->type() == Smb4KEvent::LoadSettings)
  {
    loadSettings();
  }
  else if (e->type() == Smb4KEvent::SetFocus)
  {
    if (m_widget->topLevelItemCount() != 0)
    {
      qDebug() << "Do we need to port the selection stuff?";
    }

    m_widget->setFocus(Qt::OtherFocusReason);
  }
  else if (e->type() == Smb4KEvent::ScanNetwork)
  {
    slotRescanAbortActionTriggered(false); // boolean is ignored
  }
  else if (e->type() == Smb4KEvent::AddBookmark)
  {
    slotAddBookmark(false);
  }
  else if (e->type() == Smb4KEvent::MountOrUnmountShare)
  {
    // Change the active state of the mount action. This needs 
    // to be done here, because the action is not switched
    // automatically in case the part is notified from outside.
    KDualAction *mount_action = static_cast<KDualAction *>(actionCollection()->action("mount_action"));
    mount_action->setActive(!mount_action->isActive());

    // Mount or unmount the share.
    slotMountActionTriggered(false);
  }
  else
  {
    // Do nothing
  }

  KParts::Part::customEvent(e);
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS (Smb4KNetworkBrowserPart)
/////////////////////////////////////////////////////////////////////////////

void Smb4KNetworkBrowserPart::slotContextMenuRequested(const QPoint &pos)
{
  QTreeWidgetItem *item = m_widget->itemAt(pos);

  if (item)
  {
    m_menu->menu()->setTitle(item->text(Smb4KNetworkBrowserItem::Network));
    m_menu->menu()->setIcon(item->icon(Smb4KNetworkBrowserItem::Network));
  }
  else
  {
    m_menu->menu()->setTitle(i18n("Network"));
    m_menu->menu()->setIcon(KDE::icon("network-workgroup"));
  }

  m_menu->menu()->popup(m_widget->viewport()->mapToGlobal(pos));
}


void Smb4KNetworkBrowserPart::slotItemSelectionChanged()
{
  // Get the selected item.
  QList<QTreeWidgetItem *> items = m_widget->selectedItems();

  if (items.size() == 1)
  {
    Smb4KNetworkBrowserItem *browser_item = static_cast<Smb4KNetworkBrowserItem *>(items.first());

    if (browser_item)
    {
      switch (browser_item->type())
      {
        case Host:
        {
          // Change the text of the rescan action:
          KGuiItem rescan_item(i18n("Scan Compute&r"), KDE::icon("view-refresh"));
          static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"))->setActiveGuiItem(rescan_item);

          // Enable/disable the actions:
          actionCollection()->action("bookmark_action")->setEnabled(false);
          actionCollection()->action("authentication_action")->setEnabled(true);
          actionCollection()->action("preview_action")->setEnabled(false);
          actionCollection()->action("mount_action")->setEnabled(false);
          static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
          actionCollection()->action("print_action")->setEnabled(false);
          actionCollection()->action("custom_action")->setEnabled(true);
          break;
        }
        case Share:
        {
          // Change the text of the rescan action:
          KGuiItem rescan_item(i18n("Scan Compute&r"), KDE::icon("view-refresh"));
          static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"))->setActiveGuiItem(rescan_item);

          // Enable/disable the actions:
          actionCollection()->action("authentication_action")->setEnabled(true);

          if (!browser_item->shareItem()->isPrinter())
          {
            actionCollection()->action("bookmark_action")->setEnabled(true);
            actionCollection()->action("preview_action")->setEnabled(true);

            if (!browser_item->shareItem()->isMounted() || (browser_item->shareItem()->isMounted() && browser_item->shareItem()->isForeign()))
            {
              actionCollection()->action("mount_action")->setEnabled(true);
              static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
            }
            else if (browser_item->shareItem()->isMounted() && !browser_item->shareItem()->isForeign())
            {
              actionCollection()->action("mount_action")->setEnabled(true);
              static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(false);
            }
            else
            {
              actionCollection()->action("mount_action")->setEnabled(false);
              static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
            }

            actionCollection()->action("print_action")->setEnabled(false);
            actionCollection()->action("custom_action")->setEnabled(true);
          }
          else
          {
            actionCollection()->action("bookmark_action")->setEnabled(false);
            actionCollection()->action("preview_action")->setEnabled(false);
            actionCollection()->action("mount_action")->setEnabled(false);
            static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
            actionCollection()->action("print_action")->setEnabled(
              !Smb4KPrint::self()->isRunning(browser_item->shareItem()));
            actionCollection()->action("custom_action")->setEnabled(false);
          }
          break;
        }
        default:
        {
          // Change the text of the rescan action:
          KGuiItem rescan_item(i18n("Scan Wo&rkgroup"), KDE::icon("view-refresh"));
          static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"))->setActiveGuiItem(rescan_item);
          actionCollection()->action("bookmark_action")->setEnabled(false);
          actionCollection()->action("authentication_action")->setEnabled(false);
          actionCollection()->action("preview_action")->setEnabled(false);
          actionCollection()->action("mount_action")->setEnabled(false);
          static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
          actionCollection()->action("print_action")->setEnabled(false);
          actionCollection()->action("custom_action")->setEnabled(false);
          break;
        }
      }
    }
    else
    {
      // Do nothing. This is managed elsewhere.
    }
  }
  else if (items.size() > 1)
  {
    // In this case there are only shares selected, because all other items
    // are automatically deselected in extended selection mode.
    
    // For deciding which function the mount action should have, we use
    // the number of unmounted shares. If that is identical with the items.size(),
    // it will mount the items, otherwise it will unmount them.
    int unmounted_shares = items.size();
    
    for (int i = 0; i < items.size(); ++i)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));
      
      if (item && item->shareItem()->isMounted())
      {
        unmounted_shares--;
      }
      else
      {
        // Do nothing
      }
    }
    
    // Adjust the actions.
    KGuiItem rescan_item(i18n("Scan Netwo&rk"), KDE::icon("view-refresh"));
    static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"))->setActiveGuiItem(rescan_item);
    actionCollection()->action("rescan_abort_action")->setEnabled(false);
    actionCollection()->action("bookmark_action")->setEnabled(true);
    actionCollection()->action("authentication_action")->setEnabled(false);
    actionCollection()->action("preview_action")->setEnabled(true);
    actionCollection()->action("mount_action")->setEnabled(true);
    static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive((unmounted_shares == items.size()));
    actionCollection()->action("print_action")->setEnabled(false);
    actionCollection()->action("custom_action")->setEnabled(false);    
  }
  else
  {
    KGuiItem rescan_item(i18n("Scan Netwo&rk"), KDE::icon("view-refresh"));
    static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"))->setActiveGuiItem(rescan_item);    
    actionCollection()->action("bookmark_action")->setEnabled(false);
    actionCollection()->action("authentication_action")->setEnabled(false);
    actionCollection()->action("preview_action")->setEnabled(false);
    actionCollection()->action("mount_action")->setEnabled(false);
    static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
    actionCollection()->action("print_action")->setEnabled(false);
    actionCollection()->action("custom_action")->setEnabled(false);
  }
}


void Smb4KNetworkBrowserPart::slotItemPressed(QTreeWidgetItem *item, int /*column*/)
{
  // FIXME: Check if this slot is still necessary...
  
  if (QApplication::keyboardModifiers() == Qt::NoModifier)
  {
    // Enable/disable the actions.
    Smb4KNetworkBrowserItem *browser_item = static_cast<Smb4KNetworkBrowserItem *>(item);
    
    if (!browser_item && m_widget->selectedItems().size() == 0)
    {
      KGuiItem rescan_item(i18n("Scan Netwo&rk"), KDE::icon("view-refresh"));
      static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"))->setActiveGuiItem(rescan_item);    
      actionCollection()->action("bookmark_action")->setEnabled(false);
      actionCollection()->action("authentication_action")->setEnabled(false);
      actionCollection()->action("preview_action")->setEnabled(false);
      actionCollection()->action("mount_action")->setEnabled(false);
      static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
      actionCollection()->action("print_action")->setEnabled(false);
      actionCollection()->action("custom_action")->setEnabled(false);
    }
    else if (browser_item)
    {
      switch (browser_item->type())
      {
        case Share:
        {
          if (browser_item->shareItem()->isPrinter())
          {
            actionCollection()->action("print_action")->setEnabled(true);
            
            actionCollection()->action("mount_action")->setEnabled(false);
            static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
          }
          else
          {
            if (!browser_item->shareItem()->isMounted() || (browser_item->shareItem()->isMounted() && browser_item->shareItem()->isForeign()))
            {
              actionCollection()->action("mount_action")->setEnabled(true);
              static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
            }
            else if (browser_item->shareItem()->isMounted() && !browser_item->shareItem()->isForeign())
            {
              actionCollection()->action("mount_action")->setEnabled(true);
              static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(false);
            }
            else
            {
              actionCollection()->action("mount_action")->setEnabled(false);
              static_cast<KDualAction *>(actionCollection()->action("mount_action"))->setActive(true);
            }
          }

          break;
        }
        default:
        {
          break;
        }
      }
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


void Smb4KNetworkBrowserPart::slotItemActivated(QTreeWidgetItem *item, int /*column*/)
{
  if (QApplication::keyboardModifiers() == Qt::NoModifier && m_widget->selectedItems().size() == 1)
  {
    Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(item);

    if (browserItem)
    {
      switch (browserItem->type())
      {
        case Workgroup:
        {
          if (browserItem->isExpanded())
          {
            Smb4KScanner::self()->lookupDomainMembers(browserItem->workgroupItem(), m_widget);
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Host:
        {
          if (browserItem->isExpanded())
          {
            Smb4KScanner::self()->lookupShares(browserItem->hostItem(), m_widget);
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Share:
        {
          if (!browserItem->shareItem()->isPrinter())
          {
            slotMountActionTriggered(false);  // boolean is ignored
          }
          else
          {
            slotPrint(false);  // boolean is ignored
          }
          break;
        }
        default:
        {
          break;
        }
      }
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


void Smb4KNetworkBrowserPart::slotWorkgroups(const QList<Smb4KWorkgroup *> &list)
{
  // Process the list.
  if (!list.isEmpty())
  {
    // Remove obsolete workgroup items from the tree widget.
    for (int i = 0; i < m_widget->topLevelItemCount(); ++i)
    {
      Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>(m_widget->topLevelItem(i));

      for (int j = 0; j < list.size(); ++j)
      {
        bool found_workgroup = false;

        if (QString::compare(list.at(j)->workgroupName(), workgroup_item->workgroupItem()->workgroupName()) == 0)
        {
          // Check if the master browser is still the same.
          if (QString::compare(list.at(j)->masterBrowserName(), workgroup_item->workgroupItem()->masterBrowserName()) != 0)
          {
            // We found the workgroup.
            bool found_new_master_browser = false;

            // Find the old and the new master browser.
            for (int k = 0; k < workgroup_item->childCount(); ++k)
            {
              Smb4KNetworkBrowserItem *host_item = static_cast<Smb4KNetworkBrowserItem *>(workgroup_item->child(k));

              if (QString::compare(workgroup_item->workgroupItem()->masterBrowserName(), host_item->hostItem()->hostName()) == 0)
              {
                // This is the old master browser. Update it.
                Smb4KHost *host = findHost(host_item->hostItem()->hostName(), host_item->hostItem()->workgroupName());

                if (host)
                {
                  // Update.
                  host_item->update(host);
                }
                else
                {
                  // The old master is not available anymore.
                  // Remove it.
                  delete host_item;
                }

                continue;
              }
              else if (QString::compare(list.at(j)->masterBrowserName(), host_item->hostItem()->hostName()) == 0)
              {
                // This is the new master browser. Update it.
                Smb4KHost *host = findHost(host_item->hostItem()->hostName(), host_item->hostItem()->workgroupName());

                if (host)
                {
                  // Update.
                  host_item->update(host);
                }
                else
                {
                  // Huh???
                }

                continue;
              }
              else
              {
                continue;
              }
            }

            if (!found_new_master_browser)
            {
              // The new master browser is not in the tree widget, so we have to put it there.
              Smb4KHost *host = findHost(workgroup_item->workgroupItem()->masterBrowserName(), workgroup_item->workgroupItem()->workgroupName());
              (void) new Smb4KNetworkBrowserItem(workgroup_item, host);
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

          // Update the workgroup item.
          workgroup_item->update(list.at(j));

          // We found the workgroup.
          found_workgroup = true;

          break;
        }
        else
        {
          continue;
        }

        // Remove the workgroup from the tree widget if it is obsolete.
        if (!found_workgroup)
        {
          delete workgroup_item;
        }
        else
        {
          // Do nothing
        }
      }
    }

    // Put the new workgroup items into the tree widget.
    for (int i = 0; i < list.size(); ++i)
    {
      QList<QTreeWidgetItem *> items = m_widget->findItems(list.at(i)->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network);

      if (items.isEmpty())
      {
        // The workgroup is not in the tree widget. Add it.
        (void) new Smb4KNetworkBrowserItem(m_widget, list.at(i));
        continue;
      }
      else
      {
        continue;
      }
    }

    // Sort the items.
    m_widget->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);
  }
  else
  {
    // Clear the tree widget.
    m_widget->clear();
  }
}


void Smb4KNetworkBrowserPart::slotWorkgroupMembers(Smb4KWorkgroup *workgroup, const QList<Smb4KHost *> &list)
{
  if (workgroup)
  {
    // Find the workgroup.
    QList<QTreeWidgetItem *> workgroups = m_widget->findItems(workgroup->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network);

    if (!list.isEmpty())
    {
      QMutableListIterator<QTreeWidgetItem *> it(workgroups);
      
      while (it.hasNext())
      {
        Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>(it.next());
        
        // Remove obsolete hosts from the workgroup.
        for (int j = 0; j < workgroup_item->childCount(); ++j)
        {
          Smb4KNetworkBrowserItem *host_item = static_cast<Smb4KNetworkBrowserItem *>(workgroup_item->child(j));
          bool found_host = false;

          for (int k = 0; k < list.size(); ++k)
          {
            if (QString::compare(list.at(k)->workgroupName(), host_item->hostItem()->workgroupName()) == 0 &&
                 QString::compare(list.at(k)->hostName(), host_item->hostItem()->hostName()) == 0)
            {
              found_host = true;
              break;
            }
            else
            {
              continue;
            }
          }

          if (!found_host)
          {
            delete host_item;
          }
          else
          {
            // Do nothing
          }
        }
        
        // Add new hosts to the workgroup and update the existing ones.
        for (int j = 0; j < list.size(); ++j)
        {
          if (QString::compare(list.at(j)->workgroupName(), workgroup_item->workgroupItem()->workgroupName()) == 0)
          {
            bool found_host = false;

            for (int k = 0; k < workgroup_item->childCount(); ++k)
            {
              Smb4KNetworkBrowserItem *host_item = static_cast<Smb4KNetworkBrowserItem *>(workgroup_item->child(k));

              if (QString::compare(list.at(j)->hostName(), host_item->hostItem()->hostName()) == 0)
              {
                host_item->update(list.at(j));
                found_host = true;
                break;
              }
              else
              {
                continue;
              }
            }

            if (!found_host)
            {
              (void) new Smb4KNetworkBrowserItem(workgroup_item, list.at(j));
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
        
        // If the workgroup item has children, expand it if necessary. 
        // Otherwise there is no use in keeping it, so remove it.
        if (workgroup_item->childCount() != 0)
        {
          if (Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded())
          {
            m_widget->expandItem(workgroup_item);
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          delete workgroup_item;
        }

        // Sort the items.
        m_widget->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);     
      }
    }
    else
    {
      // The workgroup(s) has/have no children (anymore). Remove
      // it/them.
      while (!workgroups.isEmpty())
      {
        delete workgroups.takeFirst();
      }
    }
  }
  else
  {
    // This is the list of all hosts available on the network.
    if (!list.isEmpty())
    {
      // Loop through the network neighborhood tree. Remove the obsolete host
      // and update the still available ones.
      QTreeWidgetItemIterator it(m_widget);
      
      while (*it)
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
        
        switch (item->type())
        {
          case Host:
          {
            bool found = false;
            
            for (int i = 0; i < list.size(); ++i)
            {
              if (QString::compare(list.at(i)->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive) == 0 &&
                  QString::compare(list.at(i)->unc(), item->hostItem()->unc(), Qt::CaseInsensitive) == 0)
              {
                item->update(list.at(i));
                found = true;
                break;
              }
              else
              {
                continue;
              }
            }
            
            if (!found)
            {
              delete item;
            }
            else
            {
              // Do nothing
            }            
            break;
          }
          default:
          {
            break;
          }
        }
        
        ++it;
      }
      
      // Now add the new hosts. For this we search the network neighborhood
      // tree and add the host if either there is no match or the found host(s)
      // belong(s) to another workgroup.
      for (int i = 0; i < list.size(); ++i)
      {
        QList<QTreeWidgetItem *> hosts = m_widget->findItems(list.at(i)->hostName(), 
                                                              Qt::MatchFixedString|Qt::MatchRecursive, 
                                                              Smb4KNetworkBrowser::Network);
        
        if (!hosts.isEmpty())
        {
          bool match = false;
          
          for (int j = 0; j < hosts.size(); ++j)
          {
            Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(hosts.at(i));
            
            if (item->type() == Host &&
                 QString::compare(list.at(i)->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive) == 0 &&
                 QString::compare(list.at(i)->unc(), item->hostItem()->unc(), Qt::CaseInsensitive) == 0)
            {
              match = true;
              break;
            }
            else
            {
              continue;
            }
          }
          
          if (!match)
          {
            // This host is new. Add it to the list widget and create the
            // workgroup item as well if needed.
            QList<QTreeWidgetItem *> workgroups = m_widget->findItems(list.at(i)->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network);
            
            if (!workgroups.isEmpty())
            {
              for (int j = 0; j < workgroups.size(); ++j)
              {
                Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>(workgroups.at(j));

                if (workgroup_item)
                {
                  // FIXME: Do we need to change the ·ðmaster browser here?
                  (void) new Smb4KNetworkBrowserItem(workgroup_item, list.at(i));
                  
                  if (Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded())
                  {
                    m_widget->expandItem(workgroup_item);
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
            }
            else
            {
              // We need to create the workgroup and the host item. Since the workgroup
              // was not found in the browser, we can assume, that it is also not available
              // in the global list. This may happen, if no master browser could be found
              // during an IP scan. So, we will create a temporary workgroup item now.
              Smb4KWorkgroup workgroup;
              workgroup.setWorkgroupName(list.at(i)->workgroupName());

              if (list.at(i)->isMasterBrowser())
              {
                workgroup.setMasterBrowserName(list.at(i)->hostName());
                workgroup.setMasterBrowserIP(list.at(i)->ip());
              }
              else
              {
                // Do nothing
              }

              Smb4KNetworkBrowserItem *workgroup_item = new Smb4KNetworkBrowserItem(m_widget, &workgroup);
              (void) new Smb4KNetworkBrowserItem(workgroup_item, list.at(i));
              
              if (Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded())
              {
                m_widget->expandItem(workgroup_item);
              }
              else
              {
                // Do nothing
              }
              continue;
            }
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // This host is new. Add it to the list widget and create the
          // workgroup item as well if needed.
          QList<QTreeWidgetItem *> workgroups = m_widget->findItems(list.at(i)->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network);
          
          if (!workgroups.isEmpty())
          {
            for (int j = 0; j < workgroups.size(); ++j)
            {
              Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>(workgroups.at(j));

              if (workgroup_item)
              {
                // FIXME: Do we need to change the (pseudo) master browser here?
                (void) new Smb4KNetworkBrowserItem(workgroup_item, list.at(i));
                
                if (Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded())
                {
                  m_widget->expandItem(workgroup_item);
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
          }
          else
          {
            // We need to create the workgroup and the host item. Since the workgroup
            // was not found in the browser, we can assume, that it is also not available
            // in the global list. This may happen, if no master browser could be found
            // during an IP scan. So, we will create a temporary workgroup item now.
            Smb4KWorkgroup workgroup;
            workgroup.setWorkgroupName(list.at(i)->workgroupName());

            if (list.at(i)->isMasterBrowser())
            {
              workgroup.setMasterBrowserName(list.at(i)->hostName());
              workgroup.setMasterBrowserIP(list.at(i)->ip());
            }
            else
            {
              // Do nothing
            }

            Smb4KNetworkBrowserItem *workgroup_item = new Smb4KNetworkBrowserItem(m_widget, &workgroup);
            (void) new Smb4KNetworkBrowserItem(workgroup_item, list.at(i));
            
            if (Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded())
            {
              m_widget->expandItem(workgroup_item);
            }
            else
            {
              // Do nothing
            }
            continue;
          }
        }
      }
      
      // Sort the items.
      m_widget->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);
    }
    else
    {
      // If the list of hosts is empty, we can clear the whole 
      // network neighborhood tree.
      m_widget->clear();
    }
  }
}


void Smb4KNetworkBrowserPart::slotShares(Smb4KHost *host, const QList<Smb4KShare *> &list)
{
  if (host)
  {
    QList<QTreeWidgetItem *> network_items = m_widget->findItems(host->hostName(), Qt::MatchFixedString|Qt::MatchRecursive, Smb4KNetworkBrowser::Network);
    
    // Find the host and process it.
    for (int i = 0; i < network_items.size(); ++i)
    {
      Smb4KNetworkBrowserItem *network_item = static_cast<Smb4KNetworkBrowserItem *>(network_items[i]);
      
      if (network_item && network_item->type() == Host &&
          QString::compare(network_item->hostItem()->workgroupName(), host->workgroupName(), Qt::CaseInsensitive) == 0)
      {
        QStringList selected_items;
        
        // Delete all shares of the host.
        while (network_item->childCount() != 0)
        {
          if (network_item->child(0)->isSelected())
          {
            // Add item to the list of selected items.
            selected_items << static_cast<Smb4KNetworkBrowserItem *>(network_item->child(0))->shareItem()->unc();
          }
          else
          {
            // Do nothing
          }
          delete network_item->child(0);
        }
        
        // Add the newly discovered shares to the host.
        if (!list.isEmpty())
        {
          // Expand the list of shares.
          if (Smb4KSettings::autoExpandNetworkItems() && !network_item->isExpanded())
          {
            m_widget->expandItem(network_item);
          }
          else
          {
            // Do nothing
          }
          
          // Add the shares to the host.
          for (int j = 0; j < list.size(); ++j)
          {
            Smb4KNetworkBrowserItem *item = new Smb4KNetworkBrowserItem(network_item, list.at(j));
            item->setSelected(selected_items.contains(list.at(j)->unc()));
          }
        }
        else
        {
          // Collapse the list of shares.
          m_widget->collapseItem(network_item);
        }

        // Stop the loop.
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    // Sort the items.
    m_widget->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotAddIPAddress(Smb4KHost *host)
{
  Q_ASSERT(host);
  
  // Find the host.
  Smb4KNetworkBrowserItem *hostItem = NULL;
  QTreeWidgetItemIterator it(m_widget);
  
  while(*it)
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
    
    if (item)
    {
      if (item->type() == Host)
      {
        if (QString::compare(host->unc(), item->hostItem()->unc(), Qt::CaseInsensitive) == 0 &&
             QString::compare(host->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive) == 0)
        {
          hostItem = item;
          break;
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
    else
    {
      // Do nothing
    }
    
    ++it;
  }
  
  if (hostItem)
  {
    // Update host item
    hostItem->update(host);
    
    // If the host is a master browser, set the IP address of the
    // workgroup item.
    Smb4KNetworkBrowserItem *workgroupItem = NULL;
    
    if (host->isMasterBrowser())
    {
      workgroupItem = static_cast<Smb4KNetworkBrowserItem *>(hostItem->parent());
      
      if (workgroupItem)
      {
        Smb4KWorkgroup *workgroup = findWorkgroup(host->workgroupName());
        
        if (workgroup)
        {
          workgroupItem->update(workgroup);
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
    else
    {
      // Do nothing
    }
    
    // If there are already shared resources added to the
    // host item, update their IP address entry.
    if (hostItem->childCount() != 0)
    {
      for (int i = 0; i < hostItem->childCount(); ++i)
      {
        Smb4KNetworkBrowserItem *shareItem = static_cast<Smb4KNetworkBrowserItem *>(hostItem->child(i));
        
        if (shareItem)
        {
          // We only need to update the IP address. No need to
          // use Smb4KNetworkBrowserItem::update() here.
          shareItem->shareItem()->setHostIP(host->ip());
        }
        else
        {
          // Do nothing
        }
      }
    }
    
    // Now adjust the IP address column, if it is not hidden.
    if (!m_widget->isColumnHidden(Smb4KNetworkBrowser::IP))
    {
      m_widget->resizeColumnToContents(Smb4KNetworkBrowser::IP);
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


void Smb4KNetworkBrowserPart::slotAuthError(Smb4KHost *host, int process)
{
  switch (process)
  {
    case LookupDomains:
    {
      // We queried a master browser from the list of domains and
      // workgroup. So, we can clear the whole list of domains.
      while (m_widget->topLevelItemCount() != 0)
      {
        delete m_widget->takeTopLevelItem(0);
      }
      break;
    }
    case LookupDomainMembers:
    {
      // Get the workgroup where the master browser is not accessible 
      // and clear the whole list of hosts. Then, reinsert the master 
      // browser.
      if (m_widget->topLevelItemCount() != 0)
      {
        for (int i = 0; i < m_widget->topLevelItemCount(); ++i)
        {
          Smb4KNetworkBrowserItem *workgroup = static_cast<Smb4KNetworkBrowserItem *>(m_widget->topLevelItem(i));
          
          if (workgroup && workgroup->type() == Workgroup &&
               QString::compare(host->workgroupName(), workgroup->workgroupItem()->workgroupName(), Qt::CaseInsensitive) == 0)
          {
            QList<QTreeWidgetItem *> hosts = workgroup->takeChildren();
            
            while (!hosts.isEmpty())
            {
              delete hosts.takeFirst();
            }
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
      break;
    }
    case LookupShares:
    {
      // Get the host that could not be accessed.
      QTreeWidgetItemIterator it(m_widget);
      
      while (*it)
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
        
        if (item && item->type() == Host)
        {
          if (QString::compare(host->hostName(), item->hostItem()->hostName(), Qt::CaseInsensitive) == 0 &&
               QString::compare(host->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive) == 0)
          {
            QList<QTreeWidgetItem *> shares = item->takeChildren();
            
            while (!shares.isEmpty())
            {
              delete shares.takeFirst();
            }
            break;
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
        ++it;
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkBrowserPart::slotRescanAbortActionTriggered(bool /* checked */)
{
  KDualAction *rescan_abort_action = static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"));
  
  if (rescan_abort_action)
  {
    if (rescan_abort_action->isActive())
    {
      // The mouse is inside the viewport. Let's see what we have to do.
      if (m_widget->currentItem() && m_widget->currentItem()->isSelected())
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_widget->currentItem());

        switch (item->type())
        {
          case Workgroup:
          {
            Smb4KScanner::self()->lookupDomainMembers(item->workgroupItem(), m_widget);
            break;
          }
          case Host:
          {
            Smb4KScanner::self()->lookupShares(item->hostItem(), m_widget);
            break;
          }
          case Share:
          {
            Smb4KNetworkBrowserItem *parentItem = static_cast<Smb4KNetworkBrowserItem *>(item->parent());
            Smb4KScanner::self()->lookupShares(parentItem->hostItem(), m_widget);
            break;
          }
          default:
          {
            break;
          }
        }
      }
      else
      {
        Smb4KScanner::self()->lookupDomains(m_widget);
      }
    }
    else
    {
      if (Smb4KScanner::self()->isRunning())
      {
        Smb4KScanner::self()->abortAll();
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
}


void Smb4KNetworkBrowserPart::slotMountManually(bool /*checked*/)
{
  Smb4KMounter::self()->openMountDialog(m_widget);
}


void Smb4KNetworkBrowserPart::slotAuthentication(bool /*checked*/)
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_widget->currentItem());

  if (item)
  {
    switch (item->type())
    {
      case Host:
      {
        Smb4KWalletManager::self()->showPasswordDialog(item->hostItem(), m_widget);
        break;
      }
      case Share:
      {
        Smb4KWalletManager::self()->showPasswordDialog(item->shareItem(), m_widget);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotCustomOptions(bool /*checked*/)
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_widget->currentItem());
  
  if (item)
  {
    switch (item->type())
    {
      case Host:
      {
        Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(item->hostItem(), m_widget);
        break;
      }
      case Share:
      {
        Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(item->shareItem(), m_widget);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotAddBookmark(bool /*checked*/)
{
  QList<QTreeWidgetItem *> items = m_widget->selectedItems();
  QList<Smb4KShare *> shares;

  if (!items.isEmpty())
  {
    for (int i = 0; i < items.size(); ++i)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));

      if (item && item->type() == Share && !item->shareItem()->isPrinter())
      {
        shares << item->shareItem();
        continue;
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // No selected items. Just return.
    return;
  }

  if (!shares.isEmpty())
  {
    Smb4KBookmarkHandler::self()->addBookmarks(shares, m_widget);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotPreview(bool /*checked*/)
{
  QList<QTreeWidgetItem *> items = m_widget->selectedItems();
  
  if (!items.isEmpty())
  {
    for (int i = 0; i < items.size(); ++i)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));

      if (item && item->type() == Share && !item->shareItem()->isPrinter())
      {
        Smb4KPreviewer::self()->preview(item->shareItem(), m_widget);
        continue;
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


void Smb4KNetworkBrowserPart::slotPrint(bool /*checked*/)
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_widget->currentItem());
  
  if (item && item->shareItem()->isPrinter())
  {
    Smb4KPrint::self()->print(item->shareItem(), m_widget);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotMountActionTriggered(bool /*checked*/)
{
  QList<QTreeWidgetItem *> items = m_widget->selectedItems();
  
  if (items.size() > 1)
  {
    // In the case of multiple selected network items, selectedItems() 
    // only contains shares. Thus, we do not need to test for the type.
    // For deciding what the mount action is supposed to do, i.e. mount
    // the (remaining) selected unmounted shares or unmounting all selected
    // mounted shares, we use the number of unmounted shares. If that is
    // greater than 0, we mount all shares that need to be mounted, otherwise
    // we unmount all selected shares.
    QList<Smb4KShare *> unmounted, mounted;
    
    for (int i = 0; i < items.size(); ++i)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));
      
      if (item && item->shareItem()->isMounted())
      {
        mounted << item->shareItem();
      }
      else if (item && !item->shareItem()->isMounted())
      {
        unmounted << item->shareItem();
      }
      else
      {
        // Do nothing
      }
    }
    
    if (unmounted.size() > 0)
    {
      // Mount the (remaining) unmounted shares.
      Smb4KMounter::self()->mountShares(unmounted, m_widget);
    }
    else
    {
      // Unmount all shares.
      Smb4KMounter::self()->unmountShares(mounted, m_widget);
    }
  }
  else
  {
    // If only one network item is selected, we need to test for the type
    // of the item. Only in case of a share we need to do something.
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_widget->currentItem());

    if (item)
    {
      switch (item->type())
      {
        case Share:
        {
          if (!item->shareItem()->isMounted())
          {
            Smb4KMounter::self()->mountShare(item->shareItem(), m_widget);
          }
          else
          {
            Smb4KMounter::self()->unmountShare(item->shareItem(), false, m_widget);
          }
          break;
        }
        default:
        {
          break;
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
}



void Smb4KNetworkBrowserPart::slotMountActionChanged(bool active)
{
  if (active)
  {
    QAction *mount_action = actionCollection()->action("mount_action");
    actionCollection()->setDefaultShortcut(mount_action, QKeySequence(Qt::CTRL+Qt::Key_M));
  }
  else
  {
    QAction *mount_action = actionCollection()->action("mount_action");
    actionCollection()->setDefaultShortcut(mount_action, QKeySequence(Qt::CTRL+Qt::Key_U));
  }
}


void Smb4KNetworkBrowserPart::slotScannerAboutToStart(Smb4KBasicNetworkItem *item, int process)
{
  switch (process)
  {
    case LookupDomains:
    {
      if (!m_silent)
      {
        emit setStatusBarText(i18n("Looking for workgroups and domains..."));
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case LookupDomainMembers:
    {
      if (!m_silent)
      {
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>(item);
        emit setStatusBarText(i18n("Looking for hosts in domain %1...", workgroup->workgroupName()));
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case LookupShares:
    {
      if (!m_silent)
      {
        Smb4KHost *host = static_cast<Smb4KHost *>(item);
        emit setStatusBarText(i18n("Looking for shares provided by host %1...", host->hostName()));
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case WakeUp:
    {
      if (!m_silent)
      {
        emit setStatusBarText(i18n("Waking up remote servers..."));
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }

  KDualAction *rescan_abort_action = static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"));
  
  if (rescan_abort_action)
  {
    rescan_abort_action->setActive(!rescan_abort_action->isActive());
    
    if (rescan_abort_action->isActive())
    {
      QList<QKeySequence> rescan_shortcuts;
      rescan_shortcuts += QKeySequence::Refresh;
      rescan_shortcuts += QKeySequence(Qt::CTRL+Qt::Key_R);
      actionCollection()->setDefaultShortcuts(rescan_abort_action, rescan_shortcuts);
    }
    else
    {
      QList<QKeySequence> abort_shortcuts;
      abort_shortcuts += QKeySequence(Qt::Key_Escape);
      abort_shortcuts += QKeySequence(Qt::CTRL+Qt::Key_A);
      actionCollection()->setDefaultShortcuts(rescan_abort_action, abort_shortcuts);
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotScannerFinished(Smb4KBasicNetworkItem */*item*/, int /*process*/)
{
  if (!m_silent)
  {
    emit setStatusBarText(i18n("Done."));
  }
  else
  {
    // Do nothing
  }

  KDualAction *rescan_abort_action = static_cast<KDualAction *>(actionCollection()->action("rescan_abort_action"));
  
  if (rescan_abort_action)
  {
    rescan_abort_action->setActive(!rescan_abort_action->isActive());
    
    if (rescan_abort_action->isActive())
    {
      QList<QKeySequence> rescan_shortcuts;
      rescan_shortcuts += QKeySequence::Refresh;
      rescan_shortcuts += QKeySequence(Qt::CTRL+Qt::Key_R);
      actionCollection()->setDefaultShortcuts(rescan_abort_action, rescan_shortcuts);
    }
    else
    {
      QList<QKeySequence> abort_shortcuts;
      abort_shortcuts += QKeySequence(Qt::Key_Escape);
      abort_shortcuts += QKeySequence(Qt::CTRL+Qt::Key_A);
      actionCollection()->setDefaultShortcuts(rescan_abort_action, abort_shortcuts);
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotMounterAboutToStart(int /*process*/)
{
  // Do not change the state of the rescan action here, because it has
  // nothing to do with the mounter.
//   actionCollection()->action("abort_action")->setEnabled(true);
}


void Smb4KNetworkBrowserPart::slotMounterFinished(int process)
{
  switch (process)
  {
    case MountShare:
    {
      KDualAction *mount_action = static_cast<KDualAction *>(actionCollection()->action("mount_action"));
      
      if (mount_action)
      {
        mount_action->setActive(false);
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case UnmountShare:
    {
      KDualAction *mount_action = static_cast<KDualAction *>(actionCollection()->action("mount_action"));
      
      if (mount_action)
      {
        mount_action->setActive(true);
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkBrowserPart::slotShareMounted(Smb4KShare *share)
{
  QTreeWidgetItemIterator it(m_widget);
  
  while (*it)
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
    
    if (item->type() == Share)
    {
      if (QString::compare(item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive) == 0)
      {
        item->update(share);
        break;
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
    
    ++it;
  }
}


void Smb4KNetworkBrowserPart::slotShareUnmounted(Smb4KShare *share)
{
  QTreeWidgetItemIterator it(m_widget);
  
  while (*it)
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
    
    if (item->type() == Share)
    {
      if (QString::compare(item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive) == 0)
      {
        item->update(share);
        break;
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
    
    ++it;
  }
}


void Smb4KNetworkBrowserPart::slotAboutToQuit()
{
  saveSettings();
}


void Smb4KNetworkBrowserPart::slotIconSizeChanged(int group)
{
  switch (group)
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
      m_widget->setIconSize(QSize(icon_size, icon_size));
      break;
    }
    default:
    {
      break;
    }
  }
}

// We need this for K_PLUGIN_FACTORY to work
#include "smb4knetworkbrowser_part.moc"
