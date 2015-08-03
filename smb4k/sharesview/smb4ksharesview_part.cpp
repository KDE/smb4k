/***************************************************************************
    smb4ksharesview_part  -  This Part includes the shares view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
    copyright            : (C) 2007-2015 by Alexander Reinholdt
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
#include "smb4ksharesview_part.h"
#include "smb4kshareslistviewitem.h"
#include "smb4ksharesiconviewitem.h"
#include "../tooltips/smb4ktooltip.h"
#include "core/smb4kshare.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kbookmarkhandler.h"

// Qt includes
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtCore/QStandardPaths>
#include <QtGui/QDropEvent>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QApplication>

// KDE includes
#include <KCoreAddons/KPluginFactory>
#include <KCoreAddons/KAboutData>
#include <KCoreAddons/KJobUiDelegate>
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KActionMenu>
#include <KXmlGui/KActionCollection>
#include <KIOCore/KIO/CopyJob>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY(Smb4KSharesViewPartFactory, registerPlugin<Smb4KSharesViewPart>();)


Smb4KSharesViewPart::Smb4KSharesViewPart(QWidget *parentWidget, QObject *parent, const QList<QVariant> &args)
: KParts::Part(parent), m_mode(IconMode), m_bookmark_shortcut(true), m_silent(false)
{
  // Parse the arguments.
  for (int i = 0; i < args.size(); ++i)
  {
    if (args.at(i).toString().startsWith(QLatin1String("viewmode")))
    {
      if (QString::compare(args.at(i).toString().section('=', 1, 1).trimmed(), "list") == 0)
      {
        m_mode = ListMode;
      }
      else
      {
        // Do nothing
      }

      continue;
    }
    else if (args.at(i).toString().startsWith(QLatin1String("bookmark_shortcut")))
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

      continue;
    }
    else
    {
      continue;
    }
  }

  // Set the XML file:
  setXMLFile("smb4ksharesview_part.rc");

  // Set the container widget and its layout.
  m_container = new QWidget(parentWidget);
  m_container->setFocusPolicy(Qt::WheelFocus);

  m_layout = new QGridLayout(m_container);
  m_layout->setMargin(0);
  m_layout->setSpacing(0);

  setWidget(m_container);

  // Set up the actual view.
  m_icon_view = 0;
  m_list_view = 0;

  setupView();

  // Set up the actions.
  // Do not put this before setWidget() or the shortcuts
  // will not be shown.
  setupActions();

  // Load settings:
  loadSettings();

  // Add some connections:
  connect(Smb4KMounter::self(), SIGNAL(mounted(Smb4KShare*)),
          this, SLOT(slotShareMounted(Smb4KShare*)));
  connect(Smb4KMounter::self(), SIGNAL(unmounted(Smb4KShare*)),
          this, SLOT(slotShareUnmounted(Smb4KShare*)));
  connect(Smb4KMounter::self(), SIGNAL(updated(Smb4KShare*)),
          this, SLOT(slotShareUpdated(Smb4KShare*)));
  connect(Smb4KMounter::self(), SIGNAL(aboutToStart(Smb4KShare*,int)),
          this, SLOT(slotMounterAboutToStart(Smb4KShare*,int)));
  connect(Smb4KMounter::self(), SIGNAL(finished(Smb4KShare*,int)),
          this, SLOT(slotMounterFinished(Smb4KShare*,int)));
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
          this, SLOT(slotAboutToQuit()));
  connect(KIconLoader::global(), SIGNAL(iconChanged(int)),
          this, SLOT(slotIconSizeChanged(int)));
}


Smb4KSharesViewPart::~Smb4KSharesViewPart()
{
}


void Smb4KSharesViewPart::setupView()
{
  // Set the widget of this part:
  switch (m_mode)
  {
    case IconMode:
    {
      if (m_list_view)
      {
        disconnect(m_list_view, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(slotContextMenuRequested(QPoint)));
        disconnect(m_list_view, SIGNAL(itemSelectionChanged()),
                   this, SLOT(slotItemSelectionChanged()));
        disconnect(m_list_view, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
                   this, SLOT(slotItemPressed(QTreeWidgetItem*,int)));
        disconnect(m_list_view, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
                   this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
        disconnect(m_list_view, SIGNAL(acceptedDropEvent(Smb4KSharesListViewItem*,QDropEvent*)),
                   this, SLOT(slotListViewDropEvent(Smb4KSharesListViewItem*,QDropEvent*)));
    
        delete m_list_view;
        m_list_view = 0;
      }
      else
      {
        // Do nothing
      }

      if (!m_icon_view)
      {
        m_icon_view = new Smb4KSharesIconView(m_container);
        m_layout->addWidget(m_icon_view, 0, 0, 0);
        m_icon_view->setVisible(true);
        m_container->setFocusProxy(m_icon_view);
        
        // Set the icon size
        int icon_size = KIconLoader::global()->currentSize(KIconLoader::Desktop);
        m_icon_view->setIconSize(QSize(icon_size, icon_size));

        connect(m_icon_view, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(slotContextMenuRequested(QPoint)));
        connect(m_icon_view, SIGNAL(itemSelectionChanged()),
                this, SLOT(slotItemSelectionChanged()));
        connect(m_icon_view, SIGNAL(itemPressed(QListWidgetItem*)),
                this, SLOT(slotItemPressed(QListWidgetItem*)));
        connect(m_icon_view, SIGNAL(itemActivated(QListWidgetItem*)),
                this, SLOT(slotItemActivated(QListWidgetItem*)));
        connect(m_icon_view, SIGNAL(acceptedDropEvent(Smb4KSharesIconViewItem*,QDropEvent*)),
                this, SLOT(slotIconViewDropEvent(Smb4KSharesIconViewItem*,QDropEvent*)));
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case ListMode:
    {
      if (m_icon_view)
      {
        disconnect(m_icon_view, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(slotContextMenuRequested(QPoint)));
        disconnect(m_icon_view, SIGNAL(itemSelectionChanged()),
                   this, SLOT(slotItemSelectionChanged()));
        disconnect(m_icon_view, SIGNAL(itemPressed(QListWidgetItem*)),
                   this, SLOT(slotItemPressed(QListWidgetItem*)));
        disconnect(m_icon_view, SIGNAL(itemActivated(QListWidgetItem*)),
                   this, SLOT(slotItemActivated(QListWidgetItem*)));
        disconnect(m_icon_view, SIGNAL(acceptedDropEvent(Smb4KSharesIconViewItem*,QDropEvent*)),
                   this, SLOT(slotIconViewDropEvent(Smb4KSharesIconViewItem*,QDropEvent*)));
        
        delete m_icon_view;
        m_icon_view = 0;
      }
      else
      {
        // Do nothing
      }

      if (!m_list_view)
      {
        m_list_view = new Smb4KSharesListView(m_container);
        m_layout->addWidget(m_list_view, 0, 0, 0);
        m_list_view->setVisible(true);
        m_container->setFocusProxy(m_list_view);
        
        // Set the icon size
        int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
        m_list_view->setIconSize(QSize(icon_size, icon_size));

        connect(m_list_view, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(slotContextMenuRequested(QPoint)));
        connect(m_list_view, SIGNAL(itemSelectionChanged()),
                this, SLOT(slotItemSelectionChanged()));
        connect(m_list_view, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
                this, SLOT(slotItemPressed(QTreeWidgetItem*,int)));
        connect(m_list_view, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
                this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
        connect(m_list_view, SIGNAL(acceptedDropEvent(Smb4KSharesListViewItem*,QDropEvent*)),
                this, SLOT(slotListViewDropEvent(Smb4KSharesListViewItem*,QDropEvent*)));
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


void Smb4KSharesViewPart::setupActions()
{
  QAction *unmount_action = new QAction(KDE::icon("emblem-unmounted"), i18n("&Unmount"), this);
  connect(unmount_action, SIGNAL(triggered(bool)), this, SLOT(slotUnmountShare(bool)));

  QAction *unmount_all_action = new QAction(KDE::icon("system-run"), i18n("U&nmount All"), this);
  connect(unmount_all_action, SIGNAL(triggered(bool)), this, SLOT(slotUnmountAllShares(bool)));

  QAction *synchronize_action = new QAction(KDE::icon("folder-sync"), i18n("S&ynchronize"), this);
  connect(synchronize_action, SIGNAL(triggered(bool)), this, SLOT(slotSynchronize(bool)));
  
  KActionMenu *open_with_menu = new KActionMenu(KDE::icon("folder-open"), i18n("Open With"), this);

  QAction *konsole_action = new QAction(KDE::icon("utilities-terminal"), i18n("Open with Konso&le"), this);
  connect(konsole_action, SIGNAL(triggered(bool)), this, SLOT(slotKonsole(bool)));

  QAction *filemanager_action = new QAction(KDE::icon("system-file-manager"), i18n("Open with F&ile Manager"), this);
  connect(filemanager_action, SIGNAL(triggered(bool)), this, SLOT(slotFileManager(bool)));
  
  open_with_menu->addAction(konsole_action);
  open_with_menu->addAction(filemanager_action);

  QAction *bookmark_action = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), this);
  
  // Add the actions
  actionCollection()->addAction("unmount_action", unmount_action);
  actionCollection()->addAction("unmount_all_action", unmount_all_action);
  actionCollection()->addAction("bookmark_action", bookmark_action);
  actionCollection()->addAction("synchronize_action", synchronize_action);
  actionCollection()->addAction("open_with", open_with_menu);
  actionCollection()->addAction("konsole_action", konsole_action);
  actionCollection()->addAction("filemanager_action", filemanager_action);
  
  // Set the shortcuts
  actionCollection()->setDefaultShortcut(unmount_action, QKeySequence(Qt::CTRL+Qt::Key_U));
  actionCollection()->setDefaultShortcut(unmount_all_action, QKeySequence(Qt::CTRL+Qt::Key_N));
  actionCollection()->setDefaultShortcut(synchronize_action, QKeySequence(Qt::CTRL+Qt::Key_Y));
  actionCollection()->setDefaultShortcut(konsole_action, QKeySequence(Qt::CTRL+Qt::Key_L));
  QList<QKeySequence> shortcuts;
  shortcuts.append(QKeySequence(Qt::CTRL+Qt::Key_I));
  shortcuts.append(QKeySequence(Qt::CTRL+Qt::Key_K));  // legacy shortcut
  actionCollection()->setDefaultShortcuts(filemanager_action, shortcuts);
  
  if (m_bookmark_shortcut)
  {
    actionCollection()->setDefaultShortcut(bookmark_action, QKeySequence(Qt::CTRL+Qt::Key_B));
  }
  else
  {
    // Do nothing
  }

  // Disable all actions for now:
  unmount_action->setEnabled(false);
  unmount_all_action->setEnabled(false);
  bookmark_action->setEnabled(false);
  synchronize_action->setEnabled(false);
  open_with_menu->setEnabled(false);
  konsole_action->setEnabled(false);
  filemanager_action->setEnabled(false);

  // Insert the actions into the menu:
  m_menu = new KActionMenu(this);
  m_menu->menu()->setTitle(i18n("Shares"));
  m_menu->menu()->setIcon(KDE::icon("folder-remote"));
  m_menu->addAction(unmount_action);
  m_menu->addAction(unmount_all_action);
  m_menu->addSeparator();
  m_menu->addAction(bookmark_action);
  m_menu->addAction(synchronize_action);
  m_menu->addSeparator();
  m_menu->addAction(konsole_action);
  m_menu->addAction(filemanager_action);
  
  connect(bookmark_action, SIGNAL(triggered(bool)), this, SLOT(slotAddBookmark(bool)));
  connect(konsole_action, SIGNAL(changed()), this, SLOT(slotEnableOpenWithAction()));
  connect(filemanager_action, SIGNAL(changed()), this, SLOT(slotEnableOpenWithAction()));
}


void Smb4KSharesViewPart::loadSettings()
{
  if (Smb4KSettings::sharesIconView())
  {
    m_mode = IconMode;

    setupView();

    // Change the text of the share (first column):
    Smb4KSharesIconViewItem *item = 0;

    for (int i = 0; i < m_icon_view->count(); ++i)
    {
      item = static_cast<Smb4KSharesIconViewItem *>(m_icon_view->item(i));

      item->setShowMountPoint(Smb4KSettings::showMountPoint());
    }
  }
  else
  {
    m_mode = ListMode;

    setupView();

    m_list_view->setColumnHidden(Smb4KSharesListView::Owner, !Smb4KSettings::showOwner());
#ifdef Q_OS_LINUX
    m_list_view->setColumnHidden(Smb4KSharesListView::Login, !Smb4KSettings::showLoginName());
#endif
    m_list_view->setColumnHidden(Smb4KSharesListView::FileSystem, !Smb4KSettings::showFileSystem());
    m_list_view->setColumnHidden(Smb4KSharesListView::Free, !Smb4KSettings::showFreeDiskSpace());
    m_list_view->setColumnHidden(Smb4KSharesListView::Used, !Smb4KSettings::showUsedDiskSpace());
    m_list_view->setColumnHidden(Smb4KSharesListView::Total, !Smb4KSettings::showTotalDiskSpace());
    m_list_view->setColumnHidden(Smb4KSharesListView::Usage, !Smb4KSettings::showDiskUsage());

    // Change the text of the share (first column):
    Smb4KSharesListViewItem *item = 0;

    for (int i = 0; i < m_list_view->topLevelItemCount(); ++i)
    {
      item = static_cast<Smb4KSharesListViewItem *>(m_list_view->topLevelItem(i));

      item->setShowMountPoint(Smb4KSettings::showMountPoint());
    }

    // Load and apply the positions of the columns.
    KConfigGroup configGroup(Smb4KSettings::self()->config(), "SharesViewPart");

    QMap<int, int> map;
    map.insert(configGroup.readEntry("ColumnPositionItem", (int)Smb4KSharesListView::Item), Smb4KSharesListView::Item);
#ifdef Q_OS_LINUX
    map.insert(configGroup.readEntry("ColumnPositionLogin", (int)Smb4KSharesListView::Login), Smb4KSharesListView::Login);
#endif
    map.insert(configGroup.readEntry("ColumnPositionFileSystem", (int)Smb4KSharesListView::FileSystem), Smb4KSharesListView::FileSystem);
    map.insert(configGroup.readEntry("ColumnPositionOwner", (int)Smb4KSharesListView::Owner), Smb4KSharesListView::Owner);
    map.insert(configGroup.readEntry("ColumnPositionFree", (int)Smb4KSharesListView::Free), Smb4KSharesListView::Free);
    map.insert(configGroup.readEntry("ColumnPositionUsed", (int)Smb4KSharesListView::Used), Smb4KSharesListView::Used);
    map.insert(configGroup.readEntry("ColumnPositionTotal", (int)Smb4KSharesListView::Total), Smb4KSharesListView::Total);
    map.insert(configGroup.readEntry("ColumnPositionUsage", (int)Smb4KSharesListView::Usage), Smb4KSharesListView::Usage);

    QMap<int, int>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
      if (it.key() != m_list_view->header()->visualIndex(it.value()))
      {
        m_list_view->header()->moveSection(m_list_view->header()->visualIndex(it.value()), it.key());
      }
      else
      {
        // Do nothing
      }

      ++it;
    }
  }

  // The rest of the settings will be applied on the fly.
}


void Smb4KSharesViewPart::saveSettings()
{
  switch (m_mode)
  {
    case IconMode:
    {
      break;
    }
    case ListMode:
    {
      // Save the position of the columns.
      KConfigGroup configGroup(Smb4KSettings::self()->config(), "SharesViewPart");

      configGroup.writeEntry("ColumnPositionItem", m_list_view->header()->visualIndex(Smb4KSharesListView::Item));
#ifdef Q_OS_LINUX
      configGroup.writeEntry("ColumnPositionLogin", m_list_view->header()->visualIndex(Smb4KSharesListView::Login));
#endif
      configGroup.writeEntry("ColumnPositionFileSystem", m_list_view->header()->visualIndex(Smb4KSharesListView::FileSystem));
      configGroup.writeEntry("ColumnPositionOwner", m_list_view->header()->visualIndex(Smb4KSharesListView::Owner));
      configGroup.writeEntry("ColumnPositionFree", m_list_view->header()->visualIndex(Smb4KSharesListView::Free));
      configGroup.writeEntry("ColumnPositionUsed", m_list_view->header()->visualIndex(Smb4KSharesListView::Used));
      configGroup.writeEntry("ColumnPositionTotal", m_list_view->header()->visualIndex(Smb4KSharesListView::Total));
      configGroup.writeEntry("ColumnPositionUsage", m_list_view->header()->visualIndex(Smb4KSharesListView::Usage));

      configGroup.sync();

      break;
    }
    default:
    {
      break;
    }
  }
}


KAboutData *Smb4KSharesViewPart::createAboutData()
{
  KAboutData *aboutData = new KAboutData(QStringLiteral("smb4ksharesviewpart"), i18n("Smb4KSharesViewPart"),
    QStringLiteral("4.0"), i18n("The shares view KPart of Smb4K"), KAboutLicense::GPL_V2, 
    i18n("\u00A9 2007-2015, Alexander Reinholdt"), QString(), QStringLiteral("http://smb4k.sourceforge.net"), 
    QStringLiteral("smb4k-bugs@lists.sourceforge.net"));

  return aboutData;
}


void Smb4KSharesViewPart::customEvent(QEvent *e)
{
  if (e->type() == Smb4KEvent::LoadSettings)
  {
    // Before we reread the settings, let's save
    // widget specific things.
    saveSettings();

    // Load settings.
    loadSettings();

    // (Re-)load the list of shares.
    switch (m_mode)
    {
      case IconMode:
      {
        while (m_icon_view->count() != 0)
        {
          delete m_icon_view->takeItem(0);
        }
          
        break;
      }
      case ListMode:
      {
        while (m_list_view->topLevelItemCount() != 0)
        {
          delete m_list_view->takeTopLevelItem(0);
        }
         
        break;
      }
      default:
      {
        break;
      }
    }
      
    for (int i = 0; i < mountedSharesList().size(); ++i)
    {
      slotShareMounted(mountedSharesList().at(i));
    }
  }
  else if (e->type() == Smb4KEvent::SetFocus)
  {
    switch (m_mode)
    {
      case IconMode:
      {
        if (m_icon_view->count() != 0)
        {
          qDebug() << "Do we need to port the selection stuff?";
        }
        else
        {
          // Do nothing
        }

        m_icon_view->setFocus(Qt::OtherFocusReason);
        break;
      }
      case ListMode:
      {
        if (m_list_view->topLevelItemCount() != 0)
        {
          qDebug() << "Do we need to port the selection stuff?";
        }
        else
        {
          // Do nothing
        }

        m_list_view->setFocus(Qt::OtherFocusReason);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else if (e->type() == Smb4KEvent::AddBookmark)
  {
    slotAddBookmark(false);
  }
  else if (e->type() == Smb4KEvent::MountOrUnmountShare)
  {
    slotUnmountShare(false /*ignored*/);
  }
  else
  {
    // Do nothing
  }

  KParts::Part::customEvent(e);
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS (Smb4KSharesViewPart)
/////////////////////////////////////////////////////////////////////////////

void Smb4KSharesViewPart::slotContextMenuRequested(const QPoint &pos)
{
  switch (m_mode)
  {
    case IconMode:
    {
      QListWidgetItem *item = m_icon_view->itemAt(pos);

      if (item)
      {
        m_menu->menu()->setTitle(item->text());
        m_menu->menu()->setIcon(item->icon());
      }
      else
      {
        m_menu->menu()->setTitle(i18n("Shares"));
        m_menu->menu()->setIcon(KDE::icon("folder-remote"));
      }

      m_menu->menu()->popup(m_icon_view->viewport()->mapToGlobal(pos));

      break;
    }
    case ListMode:
    {
      QTreeWidgetItem *item = m_list_view->itemAt(pos);

      if (item)
      {
        m_menu->menu()->setTitle(item->text(Smb4KSharesListViewItem::Item));
        m_menu->menu()->setIcon(item->icon(Smb4KSharesListViewItem::Item));
      }
      else
      {
        m_menu->menu()->setTitle(i18n("Shares"));
        m_menu->menu()->setIcon(KDE::icon("folder-remote"));
      }

      m_menu->menu()->popup(m_list_view->viewport()->mapToGlobal(pos));

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotItemSelectionChanged()
{
  switch (m_mode)
  {
    case IconMode:
    {
      QList<QListWidgetItem *> items = m_icon_view->selectedItems();

      if (!items.isEmpty())
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(items.first());
        bool sync_running = Smb4KSynchronizer::self()->isRunning(item->shareItem());

        actionCollection()->action("unmount_action")->setEnabled((!item->shareItem()->isForeign() ||
                                                                    Smb4KSettings::unmountForeignShares()));
        actionCollection()->action("bookmark_action")->setEnabled(true);

        if (!item->shareItem()->isInaccessible())
        {
          actionCollection()->action("synchronize_action")->setEnabled(!QStandardPaths::findExecutable("rsync").isEmpty() && !sync_running);
          actionCollection()->action("konsole_action")->setEnabled(!QStandardPaths::findExecutable("konsole").isEmpty());
          actionCollection()->action("filemanager_action")->setEnabled(true);
        }
        else
        {
          actionCollection()->action("synchronize_action")->setEnabled(false);
          actionCollection()->action("konsole_action")->setEnabled(false);
          actionCollection()->action("filemanager_action")->setEnabled(false);
        }
      }
      else
      {
        actionCollection()->action("unmount_action")->setEnabled(false);
        actionCollection()->action("bookmark_action")->setEnabled(false);
        actionCollection()->action("synchronize_action")->setEnabled(false);
        actionCollection()->action("konsole_action")->setEnabled(false);
        actionCollection()->action("filemanager_action")->setEnabled(false);
      }

      break;
    }
    case ListMode:
    {
      // Get the selected item.
      QList<QTreeWidgetItem *> items = m_list_view->selectedItems();

      if (!items.isEmpty())
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(items.first());
        bool sync_running = Smb4KSynchronizer::self()->isRunning(item->shareItem());

        actionCollection()->action("unmount_action")->setEnabled((!item->shareItem()->isForeign() ||
                                                                    Smb4KSettings::unmountForeignShares()));
        actionCollection()->action("bookmark_action")->setEnabled(true);

        if (!item->shareItem()->isInaccessible())
        {
          actionCollection()->action("synchronize_action")->setEnabled(!QStandardPaths::findExecutable("rsync").isEmpty() && !sync_running);
          actionCollection()->action("konsole_action")->setEnabled(!QStandardPaths::findExecutable("konsole").isEmpty());
          actionCollection()->action("filemanager_action")->setEnabled(true);
        }
        else
        {
          actionCollection()->action("synchronize_action")->setEnabled(false);
          actionCollection()->action("konsole_action")->setEnabled(false);
          actionCollection()->action("filemanager_action")->setEnabled(false);
        }
      }
      else
      {
        actionCollection()->action("unmount_action")->setEnabled(false);
        actionCollection()->action("bookmark_action")->setEnabled(false);
        actionCollection()->action("synchronize_action")->setEnabled(false);
        actionCollection()->action("konsole_action")->setEnabled(false);
        actionCollection()->action("filemanager_action")->setEnabled(false);
      }

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotItemPressed(QTreeWidgetItem *item, int /*column*/)
{
  Smb4KSharesListViewItem *shareItem = static_cast<Smb4KSharesListViewItem *>(item);

  if (!shareItem && m_list_view->selectedItems().isEmpty())
  {
    actionCollection()->action("unmount_action")->setEnabled(false);
    actionCollection()->action("bookmark_action")->setEnabled(false);
    actionCollection()->action("synchronize_action")->setEnabled(false);
    actionCollection()->action("konsole_action")->setEnabled(false);
    actionCollection()->action("filemanager_action")->setEnabled(false);
  }
  else
  {
    if (shareItem)
    {
      bool sync_running = Smb4KSynchronizer::self()->isRunning(shareItem->shareItem());

      actionCollection()->action("synchronize_action")->setEnabled(!QStandardPaths::findExecutable("rsync").isEmpty() &&
                                                                   !sync_running &&
                                                                   !shareItem->shareItem()->isInaccessible());
    }
    else
    {
      // Do nothing
    }

    // The rest will be done elsewhere.
  }
}


void Smb4KSharesViewPart::slotItemPressed(QListWidgetItem *item)
{
  Smb4KSharesIconViewItem *shareItem = static_cast<Smb4KSharesIconViewItem *>(item);

  if (!shareItem && m_icon_view->selectedItems().isEmpty())
  {
    actionCollection()->action("unmount_action")->setEnabled(false);
    actionCollection()->action("bookmark_action")->setEnabled(false);
    actionCollection()->action("synchronize_action")->setEnabled(false);
    actionCollection()->action("konsole_action")->setEnabled(false);
    actionCollection()->action("filemanager_action")->setEnabled(false);
  }
  else
  {
    if (shareItem)
    {
      bool sync_running = Smb4KSynchronizer::self()->isRunning(shareItem->shareItem());

      actionCollection()->action("synchronize_action")->setEnabled(!QStandardPaths::findExecutable("rsync").isEmpty() &&
                                                                   !sync_running &&
                                                                   !shareItem->shareItem()->isInaccessible());
    }
    else
    {
      // Do nothing
    }

    // The rest will be done elsewhere.
  }
}


void Smb4KSharesViewPart::slotItemActivated(QTreeWidgetItem *item, int /*column*/)
{
  if (QApplication::keyboardModifiers() == Qt::NoModifier)
  {
    // This is a precaution.
    if (item != m_list_view->currentItem())
    {
      m_list_view->setCurrentItem(item);
    }
    else
    {
      // Do nothing
    }

    slotFileManager(false);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotItemActivated(QListWidgetItem *item)
{
  // Do not execute the item when keyboard modifiers were pressed
  // or the mouse button is not the left one.
  if (QApplication::keyboardModifiers() == Qt::NoModifier)
  {
    // This is a precaution.
    if (item != m_icon_view->currentItem())
    {
      m_icon_view->setCurrentItem(item);
    }
    else
    {
      // Do nothing
    }

    slotFileManager(false);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotListViewDropEvent(Smb4KSharesListViewItem *item, QDropEvent *e)
{
  if (item && e)
  {
    switch (e->proposedAction())
    {
      case Qt::CopyAction:
      {
        if (e->mimeData()->hasUrls())
        {
          QList<QUrl> urlList = e->mimeData()->urls();

          QUrl dest;
          dest.setPath(item->shareItem()->path());

          KIO::CopyJob *job = KIO::copy(urlList, dest, KIO::DefaultFlags);

          job->uiDelegate()->setAutoErrorHandlingEnabled(true);
          job->uiDelegate()->setAutoWarningHandlingEnabled(true);
        }
        else
        {
          // Do nothing
        }

        break;
      }
      case Qt::MoveAction:
      {
        if (e->mimeData()->hasUrls())
        {
          QList<QUrl> urlList = e->mimeData()->urls();

          QUrl dest;
          dest.setPath(item->shareItem()->path());

          KIO::CopyJob *job = KIO::move(urlList, dest, KIO::DefaultFlags);

          job->uiDelegate()->setAutoErrorHandlingEnabled(true);
          job->uiDelegate()->setAutoWarningHandlingEnabled(true);
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
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotIconViewDropEvent(Smb4KSharesIconViewItem *item, QDropEvent *e)
{
  if (item && e)
  {
    switch (e->proposedAction())
    {
      case Qt::CopyAction:
      {
        if (e->mimeData()->hasUrls())
        {
          QList<QUrl> urlList = e->mimeData()->urls();

          QUrl dest;
          dest.setPath(item->shareItem()->path());

          KIO::CopyJob *job = KIO::copy(urlList, dest, KIO::DefaultFlags);

          job->uiDelegate()->setAutoErrorHandlingEnabled(true);
          job->uiDelegate()->setAutoWarningHandlingEnabled(true);
        }
        else
        {
          // Do nothing
        }

        break;
      }
      case Qt::MoveAction:
      {
        if (e->mimeData()->hasUrls())
        {
          QList<QUrl> urlList = e->mimeData()->urls();

          QUrl dest;
          dest.setPath(item->shareItem()->path());

          KIO::CopyJob *job = KIO::move(urlList, dest, KIO::DefaultFlags);

          job->uiDelegate()->setAutoErrorHandlingEnabled(true);
          job->uiDelegate()->setAutoWarningHandlingEnabled(true);
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
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotShareMounted(Smb4KShare *share)
{
  qDebug() << "About to show share.";
  
  Q_ASSERT(share);
  
  switch (m_mode)
  {
    case IconMode:
    {
      qDebug() << "IconMode";
      (void) new Smb4KSharesIconViewItem(m_icon_view, share, Smb4KSettings::showMountPoint());
      m_icon_view->sortItems(Qt::AscendingOrder);
      actionCollection()->action("unmount_all_action")->setEnabled(
        ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && m_icon_view->count() != 0));
      break;
    }
    case ListMode:
    {
      qDebug() << "ListMode";
      (void) new Smb4KSharesListViewItem(m_list_view, share, Smb4KSettings::showMountPoint());
      m_list_view->sortItems(Smb4KSharesListView::Item, Qt::AscendingOrder);
      actionCollection()->action("unmount_all_action")->setEnabled(
        ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && m_list_view->topLevelItemCount() != 0));
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotShareUnmounted(Smb4KShare *share)
{
  Q_ASSERT(share);
  
  switch (m_mode)
  {
    case IconMode:
    {
      Smb4KSharesIconViewItem *item = 0;
      
      for (int i = 0; i < m_icon_view->count(); ++i)
      {
        item = static_cast<Smb4KSharesIconViewItem *>(m_icon_view->item(i));
        
        if (item && (QString::compare(item->shareItem()->path(), share->path()) == 0 ||
             QString::compare(item->shareItem()->canonicalPath(), share->canonicalPath()) == 0))
        {
          if (item == m_icon_view->currentItem())
          {
            m_icon_view->setCurrentItem(0);
          }
          else
          {
            // Do nothing
          }
          
          delete m_icon_view->takeItem(i);
          break;
        }
        else
        {
          continue;
        }
      }
      
      actionCollection()->action("unmount_all_action")->setEnabled(
        ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && m_icon_view->count() != 0));
      
      break;
    }
    case ListMode:
    {
      Smb4KSharesListViewItem *item = 0;
      
      for (int i = 0; i < m_list_view->topLevelItemCount(); ++i)
      {
        item = static_cast<Smb4KSharesListViewItem *>(m_list_view->topLevelItem(i));
        
        if (item && (QString::compare(item->shareItem()->path(), share->path()) == 0 ||
             QString::compare(item->shareItem()->canonicalPath(), share->canonicalPath()) == 0))
        {
          if (item == m_list_view->currentItem())
          {
            m_list_view->setCurrentItem(0);
          }
          else
          {
            // Do nothing
          }
          
          delete m_list_view->takeTopLevelItem(i);
          break;
        }
        else
        {
          continue;
        }
      }
      
      actionCollection()->action("unmount_all_action")->setEnabled(
        ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && m_list_view->topLevelItemCount() != 0));
      
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotShareUpdated(Smb4KShare *share)
{
  switch (m_mode)
  {
    case IconMode:
    {
      Smb4KSharesIconViewItem *item = 0;
      
      for (int i = 0; i < m_icon_view->count(); ++i)
      {
        item = static_cast<Smb4KSharesIconViewItem *>(m_icon_view->item(i));
        
        if (item && (QString::compare(item->shareItem()->path(), share->path()) == 0 ||
             QString::compare(item->shareItem()->canonicalPath(), share->canonicalPath()) == 0))
        {
          item->update(share);
          break;
        }
        else
        {
          continue;
        }
      }
      break;
    }
    case ListMode:
    {
      Smb4KSharesListViewItem *item = 0;
      
      for (int i = 0; i < m_list_view->topLevelItemCount(); ++i)
      {
        item = static_cast<Smb4KSharesListViewItem *>(m_list_view->topLevelItem(i));
        
        if (item && (QString::compare(item->shareItem()->path(), share->path()) == 0 ||
             QString::compare(item->shareItem()->canonicalPath(), share->canonicalPath()) == 0))
        {
          item->update(share);
          break;
        }
        else
        {
          continue;
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


void Smb4KSharesViewPart::slotUnmountShare(bool /*checked*/)
{
  switch (m_mode)
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();
      QList<Smb4KShare *> shares;

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(selected_items.at(i));

        if (item)
        {
          shares << item->shareItem();
        }
        else
        {
          // Do nothing
        }
      }

      Smb4KMounter::self()->unmountShares(shares, false, m_icon_view);

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();
      QList<Smb4KShare *> shares;

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(selected_items.at(i));

        if (item)
        {
          shares << item->shareItem();
        }
        else
        {
          // Do nothing
        }
      }

      Smb4KMounter::self()->unmountShares(shares, false, m_list_view);

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotUnmountAllShares(bool /*checked*/)
{
  switch (m_mode)
  {
    case IconMode:
    {
      Smb4KMounter::self()->unmountAllShares(m_icon_view);
      break;
    }
    case ListMode:
    {
      Smb4KMounter::self()->unmountAllShares(m_list_view);
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotSynchronize(bool /*checked*/)
{
  switch (m_mode)
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(selected_items.at(i));

        if (item && !item->shareItem()->isInaccessible())
        {
          Smb4KSynchronizer::self()->synchronize(item->shareItem(), m_icon_view);
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(selected_items.at(i));

        if (item && !item->shareItem()->isInaccessible())
        {
          Smb4KSynchronizer::self()->synchronize(item->shareItem(), m_list_view);
        }
        else
        {
          // Do nothing
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

void Smb4KSharesViewPart::slotKonsole(bool /*checked*/)
{
  switch (m_mode)
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(selected_items.at(i));

        if (item && !item->shareItem()->isInaccessible())
        {
          openShare(item->shareItem(), Konsole);
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(selected_items.at(i));

        if (item && !item->shareItem()->isInaccessible())
        {
          openShare(item->shareItem(), Konsole);
        }
        else
        {
          // Do nothing
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


void Smb4KSharesViewPart::slotFileManager(bool /*checked*/)
{
  switch (m_mode)
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(selected_items.at(i));

        if (item && !item->shareItem()->isInaccessible())
        {
          openShare(item->shareItem(), FileManager);
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for (int i = 0; i < selected_items.size(); ++i)
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(selected_items.at(i));

        if (item && !item->shareItem()->isInaccessible())
        {
          openShare(item->shareItem(), FileManager);
        }
        else
        {
          // Do nothing
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


void Smb4KSharesViewPart::slotAddBookmark(bool /*checked */)
{
  switch (m_mode)
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();
      QList<Smb4KShare *> shares;

      if (!selected_items.isEmpty())
      {
        for (int i = 0; i < selected_items.size(); ++i)
        {
          Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>(selected_items.at(i));
          shares << item->shareItem();
          continue;
        }
      }
      else
      {
        // No selected items. Just return.
        return;
      }

      if (!shares.isEmpty())
      {
        Smb4KBookmarkHandler::self()->addBookmarks(shares, m_icon_view);
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();
      QList<Smb4KShare *> shares;

      if (!selected_items.isEmpty())
      {
        for (int i = 0; i < selected_items.size(); ++i)
        {
          Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>(selected_items.at(i));
          shares << item->shareItem();
          continue;
        }
      }
      else
      {
        // No selected items. Just return.
        return;
      }

      if (!shares.isEmpty())
      {
        Smb4KBookmarkHandler::self()->addBookmarks(shares, m_list_view);
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


void Smb4KSharesViewPart::slotMounterAboutToStart(Smb4KShare *share, int process)
{
  switch (process)
  {
    case MountShare:
    {
      if (!m_silent)
      {
        emit setStatusBarText(i18n("Mounting share %1...", share->unc()));
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case UnmountShare:
    {
      if (!m_silent)
      {
        emit setStatusBarText(i18n("Unmounting share %1...", share->unc()));
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


void Smb4KSharesViewPart::slotMounterFinished(Smb4KShare */*share*/, int /*process*/)
{
  if (!m_silent)
  {
    emit setStatusBarText(i18n("Done."));
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotAboutToQuit()
{
  saveSettings();
}


void Smb4KSharesViewPart::slotIconSizeChanged(int group)
{
  switch (group)
  {
    case KIconLoader::Desktop:
    {
      if (m_icon_view)
      {
        int icon_size = KIconLoader::global()->currentSize(KIconLoader::Desktop);
        m_icon_view->setIconSize(QSize(icon_size, icon_size));
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case KIconLoader::Small:
    {
      if (m_list_view)
      {
        int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
        m_list_view->setIconSize(QSize(icon_size, icon_size));
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


void Smb4KSharesViewPart::slotEnableOpenWithAction()
{
  // Disable the Open With action menu, when all Open With actions are 
  // disabled.
  bool enable = (actionCollection()->action("konsole_action")->isEnabled() || 
                 actionCollection()->action("filemanager_action")->isEnabled());
  actionCollection()->action("open_with")->setEnabled(enable);
}

#include "smb4ksharesview_part.moc"
