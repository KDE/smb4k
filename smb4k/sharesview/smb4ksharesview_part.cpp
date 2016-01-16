/***************************************************************************
    smb4ksharesview_part  -  This Part includes the shares view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
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
#include "smb4ksharesview_part.h"
#include "smb4ksharesviewitem.h"
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
: KParts::Part(parent), m_bookmark_shortcut(true), m_silent(false)
{
  // Parse the arguments.
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
  // Set the widget of this part
  m_view = new Smb4KSharesView(m_container);
  m_layout->addWidget(m_view, 0, 0, 0);
  m_view->setVisible(true);
  m_container->setFocusProxy(m_view);
        
  // Set the icon size
  int icon_size = KIconLoader::global()->currentSize(KIconLoader::Desktop);
  m_view->setIconSize(QSize(icon_size, icon_size));

  connect(m_view, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(slotContextMenuRequested(QPoint)));
  connect(m_view, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotItemSelectionChanged()));
  connect(m_view, SIGNAL(itemPressed(QListWidgetItem*)),
          this, SLOT(slotItemPressed(QListWidgetItem*)));
  connect(m_view, SIGNAL(itemActivated(QListWidgetItem*)),
          this, SLOT(slotItemActivated(QListWidgetItem*)));
  connect(m_view, SIGNAL(acceptedDropEvent(Smb4KSharesViewItem*,QDropEvent*)),
          this, SLOT(slotIconViewDropEvent(Smb4KSharesViewItem*,QDropEvent*)));
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
  setupView();
  // The rest of the settings will be applied on the fly.
}


void Smb4KSharesViewPart::saveSettings()
{
  // Not used at the moment
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
    while (m_view->count() != 0)
    {
      delete m_view->takeItem(0);
    }
      
    for (int i = 0; i < mountedSharesList().size(); ++i)
    {
      slotShareMounted(mountedSharesList().at(i));
    }
  }
  else if (e->type() == Smb4KEvent::SetFocus)
  {
    m_view->setFocus(Qt::OtherFocusReason);
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
  QListWidgetItem *item = m_view->itemAt(pos);

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

  m_menu->menu()->popup(m_view->viewport()->mapToGlobal(pos));
}


void Smb4KSharesViewPart::slotItemSelectionChanged()
{
  QList<QListWidgetItem *> items = m_view->selectedItems();

  if (!items.isEmpty())
  {
    Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(items.first());
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
}


void Smb4KSharesViewPart::slotItemPressed(QListWidgetItem *item)
{
  Smb4KSharesViewItem *shareItem = static_cast<Smb4KSharesViewItem *>(item);

  if (!shareItem && m_view->selectedItems().isEmpty())
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


void Smb4KSharesViewPart::slotItemActivated(QListWidgetItem *item)
{
  // Do not execute the item when keyboard modifiers were pressed
  // or the mouse button is not the left one.
  if (QApplication::keyboardModifiers() == Qt::NoModifier)
  {
    // This is a precaution.
    if (item != m_view->currentItem())
    {
      m_view->setCurrentItem(item);
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


void Smb4KSharesViewPart::slotIconViewDropEvent(Smb4KSharesViewItem *item, QDropEvent *e)
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
  Q_ASSERT(share);

  (void) new Smb4KSharesViewItem(m_view, share);
  m_view->sortItems(Qt::AscendingOrder);
  actionCollection()->action("unmount_all_action")->setEnabled(
    ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && m_view->count() != 0));
}


void Smb4KSharesViewPart::slotShareUnmounted(Smb4KShare *share)
{
  Q_ASSERT(share);

  Smb4KSharesViewItem *item = 0;
      
  for (int i = 0; i < m_view->count(); ++i)
  {
    item = static_cast<Smb4KSharesViewItem *>(m_view->item(i));
        
    if (item && (QString::compare(item->shareItem()->path(), share->path()) == 0 ||
        QString::compare(item->shareItem()->canonicalPath(), share->canonicalPath()) == 0))
    {
      if (item == m_view->currentItem())
      {
        m_view->setCurrentItem(0);
      }
      else
      {
        // Do nothing
      }
          
      delete m_view->takeItem(i);
      break;
    }
    else
    {
      continue;
    }
  }
      
  actionCollection()->action("unmount_all_action")->setEnabled(
    ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && m_view->count() != 0));
}


void Smb4KSharesViewPart::slotShareUpdated(Smb4KShare *share)
{
  Smb4KSharesViewItem *item = 0;
      
  for (int i = 0; i < m_view->count(); ++i)
  {
    item = static_cast<Smb4KSharesViewItem *>(m_view->item(i));
        
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
}


void Smb4KSharesViewPart::slotUnmountShare(bool /*checked*/)
{
  QList<QListWidgetItem *> selected_items = m_view->selectedItems();
  QList<Smb4KShare *> shares;

  for (int i = 0; i < selected_items.size(); ++i)
  {
    Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selected_items.at(i));

    if (item)
    {
      shares << item->shareItem();
    }
    else
    {
      // Do nothing
    }
  }

  Smb4KMounter::self()->unmountShares(shares, false, m_view);
}


void Smb4KSharesViewPart::slotUnmountAllShares(bool /*checked*/)
{
  Smb4KMounter::self()->unmountAllShares(m_view);
}


void Smb4KSharesViewPart::slotSynchronize(bool /*checked*/)
{
  QList<QListWidgetItem *> selected_items = m_view->selectedItems();

  for (int i = 0; i < selected_items.size(); ++i)
  {
    Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selected_items.at(i));

    if (item && !item->shareItem()->isInaccessible())
    {
      Smb4KSynchronizer::self()->synchronize(item->shareItem(), m_view);
    }
    else
    {
      // Do nothing
    }
  }
}

void Smb4KSharesViewPart::slotKonsole(bool /*checked*/)
{
  QList<QListWidgetItem *> selected_items = m_view->selectedItems();

  for (int i = 0; i < selected_items.size(); ++i)
  {
    Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selected_items.at(i));

    if (item && !item->shareItem()->isInaccessible())
    {
      openShare(item->shareItem(), Konsole);
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KSharesViewPart::slotFileManager(bool /*checked*/)
{
  QList<QListWidgetItem *> selected_items = m_view->selectedItems();

  for (int i = 0; i < selected_items.size(); ++i)
  {
    Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selected_items.at(i));

    if (item && !item->shareItem()->isInaccessible())
    {
      openShare(item->shareItem(), FileManager);
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KSharesViewPart::slotAddBookmark(bool /*checked */)
{
  QList<QListWidgetItem *> selected_items = m_view->selectedItems();
  QList<Smb4KShare *> shares;

  if (!selected_items.isEmpty())
  {
    for (int i = 0; i < selected_items.size(); ++i)
    {
      Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selected_items.at(i));
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
    Smb4KBookmarkHandler::self()->addBookmarks(shares, m_view);
  }
  else
  {
    // Do nothing
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
      int icon_size = KIconLoader::global()->currentSize(KIconLoader::Desktop);
      m_view->setIconSize(QSize(icon_size, icon_size));
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
