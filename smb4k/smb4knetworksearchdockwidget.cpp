/***************************************************************************
    The network search widget dock widget 
                             -------------------
    begin                : Mon Apr 30 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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
#include "smb4knetworksearchdockwidget.h"
#include "smb4knetworksearchitem.h"
#include "core/smb4ksearch.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QLineEdit>
#include <QDebug>
#include <QMenu>

// KDE includes
#include <KWidgetsAddons/KDualAction>
#include <KWidgetsAddons/KGuiItem>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>


Smb4KNetworkSearchDockWidget::Smb4KNetworkSearchDockWidget(const QString& title, QWidget* parent)
: QDockWidget(title, parent)
{
  //
  // Set the network search widget
  // 
  m_networkSearch = new Smb4KNetworkSearch(this);
  setWidget(m_networkSearch);
  
  //
  // The action collection
  // 
  m_actionCollection = new KActionCollection(this);
  
  //
  // The context menu
  // 
  m_contextMenu = new KActionMenu(this);
  
  //
  // Set up the actions
  //
  setupActions();
  
  //
  // Load the settings
  // 
  loadSettings();
  
  //
  // Connections
  // 
  connect(m_networkSearch->comboBox(), SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
  connect(m_networkSearch->comboBox(), SIGNAL(editTextChanged(QString)), this, SLOT(slotComboBoxTextChanged(QString)));
  connect(m_networkSearch->listWidget(), SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotItemActivated(QListWidgetItem*)));
  connect(m_networkSearch->listWidget(), SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
  connect(m_networkSearch->listWidget(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenuRequested(QPoint)));
  
  connect(Smb4KSearch::self(), SIGNAL(result(SharePtr)), this, SLOT(slotReceivedSearchResult(SharePtr)));
  connect(Smb4KSearch::self(), SIGNAL(aboutToStart(QString)), this, SLOT(slotSearchAboutToStart(QString)));
  connect(Smb4KSearch::self(), SIGNAL(finished(QString)), this, SLOT(slotSearchFinished(QString)));

  connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)), this, SLOT(slotShareMounted(SharePtr)));
  connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)), this, SLOT(slotShareUnmounted(SharePtr)));
  
  connect(KIconLoader::global(), SIGNAL(iconChanged(int)), this, SLOT(slotIconSizeChanged(int)));
}


Smb4KNetworkSearchDockWidget::~Smb4KNetworkSearchDockWidget()
{
}


void Smb4KNetworkSearchDockWidget::loadSettings()
{
  //
  // Load icon size
  // 
  int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
  m_networkSearch->listWidget()->setIconSize(QSize(iconSize, iconSize));
  
  //
  // Load completion strings
  // 
  KConfigGroup group(Smb4KSettings::self()->config(), "SearchDialog");
  m_networkSearch->comboBox()->completionObject()->setItems(group.readEntry("SearchItemCompletion", QStringList()));
  
  // 
  // The only changes may concern the marking of the shares
  //
  for (int i = 0; i < m_networkSearch->listWidget()->count(); ++i)
  {
    Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(m_networkSearch->listWidget()->item(i));
    
    switch (searchItem->type())
    {
      case Smb4KNetworkSearchItem::Share:
      {
        // First update the share.
        searchItem->update();
            
        // Now either mark it again or leave it unmarked.
        QList<SharePtr> list = findShareByUNC(searchItem->shareItem()->unc());
          
        for (const SharePtr &share : list)
        {
          if (share->isMounted())
          {
            slotShareMounted(share);
              
            if (!share->isForeign())
            {
              break;
            }
            else
            {
              continue;
            }
          }
          else
          {
            continue;
          }
        }
      }
      default:
      {
        break;
      }
    }        
  }
}


void Smb4KNetworkSearchDockWidget::saveSettings()
{
  KConfigGroup group(Smb4KSettings::self()->config(), "SearchDialog");
  group.writeEntry("SearchItemCompletion", m_networkSearch->comboBox()->completionObject()->items());
}


KActionCollection *Smb4KNetworkSearchDockWidget::actionCollection()
{
  return m_actionCollection;
}



void Smb4KNetworkSearchDockWidget::setupActions()
{
  //
  // Search/Abort dual action
  // 
  KDualAction *searchAbortAction = new KDualAction(this);
  KGuiItem searchItem(i18n("&Search"), KDE::icon("system-search"));
  KGuiItem abortItem(i18n("Abort"), KDE::icon("process-stop"));
  searchAbortAction->setActiveGuiItem(searchItem);
  searchAbortAction->setInactiveGuiItem(abortItem);
  searchAbortAction->setActive(true);
  searchAbortAction->setAutoToggle(false);
  connect(searchAbortAction, SIGNAL(triggered(bool)), this, SLOT(slotSearchAbortActionTriggered(bool)));
  connect(searchAbortAction, SIGNAL(activeChanged(bool)), this, SLOT(slotSearchAbortActionChanged(bool)));
  
  //
  // Clear action
  // 
  QAction *clearAction  = new QAction(KDE::icon("edit-clear-history"), i18n("&Clear"), this);
  connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(slotClearActionTriggered(bool)));
  
  //
  // The Add Bookmark action
  // 
  QAction *bookmarkAction = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), this);
  connect(bookmarkAction, SIGNAL(triggered(bool)), this, SLOT(slotBookmarkActionTriggered(bool)));

  //
  // Mount/Unmount dual action
  // 
  KDualAction *mountAction = new KDualAction(this);
  KGuiItem mountItem(i18n("&Mount"), KDE::icon("media-mount"));
  KGuiItem unmountItem(i18n("&Unmount"), KDE::icon("media-eject"));
  mountAction->setActiveGuiItem(mountItem);
  mountAction->setInactiveGuiItem(unmountItem);
  mountAction->setActive(true);
  mountAction->setAutoToggle(false);
  connect(mountAction, SIGNAL(triggered(bool)), this, SLOT(slotMountActionTriggered(bool)));
  connect(mountAction, SIGNAL(activeChanged(bool)), this, SLOT(slotMountActionChanged(bool)));
  
  //
  // One separator for the action collection
  // 
  QAction *separator1 = new QAction(this);
  separator1->setSeparator(true);

  //
  // Add actions
  // 
  m_actionCollection->addAction("search_abort_action", searchAbortAction);
  m_actionCollection->addAction("search_separator1", separator1);
  m_actionCollection->addAction("clear_search_action", clearAction);
  m_actionCollection->addAction("bookmark_action", bookmarkAction);
  m_actionCollection->addAction("mount_action", mountAction);
  
  //
  // Set shortcuts
  //
  m_actionCollection->setDefaultShortcut(searchAbortAction, QKeySequence(Qt::CTRL+Qt::Key_S));
  m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL+Qt::Key_M));
  m_actionCollection->setDefaultShortcut(bookmarkAction, QKeySequence(Qt::CTRL+Qt::Key_B));
  
  //
  // Disable all actions
  // 
  searchAbortAction->setEnabled(false);
  clearAction->setEnabled(false);
  mountAction->setEnabled(false);
  bookmarkAction->setEnabled(false);
  
  //
  // Put the actions into the context menu
  // 
  m_contextMenu->addAction(clearAction);
  m_contextMenu->addAction(bookmarkAction);
  m_contextMenu->addAction(mountAction);
  
  // 
  // Put some actions in the tool bar of the search widget
  // 
  m_networkSearch->toolBar()->addAction(searchAbortAction);
}


void Smb4KNetworkSearchDockWidget::slotReturnPressed()
{
  if (!m_networkSearch->comboBox()->currentText().isEmpty())
  {
    //
    // Get the Search/Abort dual action and change the active property
    // 
    KDualAction *searchAbortAction = static_cast<KDualAction *>(m_actionCollection->action("search_abort_action"));
    
    if (searchAbortAction && searchAbortAction->isActive())
    {
      slotSearchAbortActionTriggered(false);
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


void Smb4KNetworkSearchDockWidget::slotComboBoxTextChanged(const QString& text)
{
  //
  // Enable or disable actions
  // 
  m_actionCollection->action("search_abort_action")->setEnabled(!text.isEmpty());
  m_actionCollection->action("clear_search_action")->setEnabled(!text.isEmpty());
}


void Smb4KNetworkSearchDockWidget::slotItemActivated(QListWidgetItem* item)
{
  if (item)
  {
    //
    // Mount/unmount the share
    // 
    Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(item);

    switch (searchItem->type())
    {
      case Smb4KNetworkSearchItem::Share:
      {
        if (!searchItem->shareItem()->isMounted())
        {
          Smb4KMounter::self()->mountShare(searchItem->shareItem(), m_networkSearch);
        }
        else
        {
          Smb4KMounter::self()->unmountShare(searchItem->shareItem(), false, m_networkSearch);
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


void Smb4KNetworkSearchDockWidget::slotItemSelectionChanged()
{
  //
  // Get the selected items
  // 
  QList<QListWidgetItem *> selectedItems = m_networkSearch->listWidget()->selectedItems();
  
  //
  // Get the Mount/Unmount dual action
  // 
  KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action("mount_action"));
  
  //
  // Get the bookmark action
  // 
  QAction *bookmarkAction = m_actionCollection->action("bookmark_action");
  
  //
  // Enable/disable and/or adjust the mount and bookmark actions.
  // 
  if (selectedItems.size() == 1)
  {
    Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(selectedItems.first());
    
    if (searchItem)
    {
      switch (searchItem->type())
      {
        case Smb4KNetworkSearchItem::Share:
        {
          if (!searchItem->shareItem()->isMounted() || (searchItem->shareItem()->isMounted() && searchItem->shareItem()->isForeign()))
          {
            mountAction->setEnabled(true);
            mountAction->setActive(true);
          }
          else if (searchItem->shareItem()->isMounted() && !searchItem->shareItem()->isForeign())
          {
            mountAction->setEnabled(true);
            mountAction->setActive(false);
          }
          else
          {
            mountAction->setEnabled(false);
            mountAction->setActive(true);
          }          
          bookmarkAction->setEnabled(!searchItem->shareItem()->isPrinter());
          break;
        }
        default:
        {
          mountAction->setEnabled(false);
          mountAction->setActive(true);
          bookmarkAction->setEnabled(false);
          break;
        }
      }
    }
    else
    {
      // Do nothing
    }
  }
  else if (selectedItems.size() > 1)
  {
    //
    // For deciding which function the mount action should have, we use
    // the number of unmounted shares. If that is identical with the items.size(),
    // it will mount the items, otherwise it will unmount them.
    //
    int unmountedShares = selectedItems.size();
    
    //
    // For enabling the bookmark action, we check if the shares are printers. 
    // 
    int printerShares = 0;
    
    for (QListWidgetItem *item : selectedItems)
    {
      Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(item);
      
      if (searchItem && searchItem->shareItem()->isMounted() && !searchItem->shareItem()->isForeign())
      {
        //
        // Substract shares mounted by the user
        // 
        unmountedShares--;
      }
      else if (searchItem && searchItem->shareItem()->isPrinter())
      {
        //
        // Substract printer shares
        // 
        printerShares++;
      }
      else
      {
        // Do nothing
      }
    }
    
    mountAction->setActive(unmountedShares == selectedItems.size());
    mountAction->setEnabled(true);
    bookmarkAction->setEnabled(printerShares != selectedItems.size());
  }
  else
  {
    mountAction->setActive(true);
    mountAction->setEnabled(false);
    bookmarkAction->setEnabled(false);
  }
}


void Smb4KNetworkSearchDockWidget::slotContextMenuRequested(const QPoint& pos)
{
  m_contextMenu->menu()->popup(m_networkSearch->listWidget()->viewport()->mapToGlobal(pos));
}


void Smb4KNetworkSearchDockWidget::slotReceivedSearchResult(const SharePtr& share)
{
  if (share)
  {
    // 
    // Create a Smb4KNetworkSearchItem and add it to the first position
    // 
    (void) new Smb4KNetworkSearchItem(m_networkSearch->listWidget(), share);

    m_networkSearch->listWidget()->sortItems();

    // 
    // Enable the combo box and set the focus
    // 
    m_networkSearch->comboBox()->setEnabled(true);
    m_networkSearch->comboBox()->setFocus();

    // 
    // Now select the text, so that the user can easily
    // remove it
    // 
    m_networkSearch->comboBox()->lineEdit()->selectAll();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchDockWidget::slotSearchAboutToStart(const QString& /*string*/)
{
  //
  // Disable the combo box
  // 
  m_networkSearch->comboBox()->setEnabled(false);
  
  //
  // Get the Search/Abort dual action and change the active property
  // 
  KDualAction *searchAbortAction = static_cast<KDualAction *>(m_actionCollection->action("search_abort_action"));
  
  if (searchAbortAction)
  {
    searchAbortAction->setActive(false);
  }
  else
  {
    // Do nothing
  }
  
  //
  // Get the Clear action and disable it
  // 
  m_actionCollection->action("clear_search_action")->setEnabled(false);

  //
  // The Add action is handled elsewhere
  // 
}


void Smb4KNetworkSearchDockWidget::slotSearchFinished(const QString& /*string*/)
{
  //
  // Enable the combo box
  // 
  m_networkSearch->comboBox()->setEnabled(true);
  
  //
  // Get the Search/Abort dual action and change the active property
  // 
  KDualAction *searchAbortAction = static_cast<KDualAction *>(m_actionCollection->action("search_abort_action"));
  
  if (searchAbortAction)
  {
    searchAbortAction->setActive(true);
  }
  else
  {
    // Do nothing
  }
  
  //
  // Get the Clear action and enable/disable it
  // 
  m_actionCollection->action("clear_search_action")->setEnabled(!m_networkSearch->comboBox()->currentText().isEmpty());
  
  //
  // Add an item to the list widget that tells the user 
  // that the search was not successful
  // 
  if (m_networkSearch->listWidget()->count() == 0)
  {
    new Smb4KNetworkSearchItem(m_networkSearch->listWidget());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchDockWidget::slotShareMounted(const SharePtr& share)
{
  if (share)
  {
    for (int i = 0; i < m_networkSearch->listWidget()->count(); ++i)
    {
      Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(m_networkSearch->listWidget()->item(i));
      
      switch (searchItem->type())
      {
        case Smb4KNetworkSearchItem::Share:
        {
          if (QString::compare(searchItem->shareItem()->unc(), share->unc(), Qt::CaseInsensitive) == 0)
          {
            searchItem->update();
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
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchDockWidget::slotShareUnmounted(const SharePtr& share)
{
  if (share)
  {
    for (int i = 0; i < m_networkSearch->listWidget()->count(); ++i)
    {
      Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(m_networkSearch->listWidget()->item(i));
      
      switch (searchItem->type())
      {
        case Smb4KNetworkSearchItem::Share:
        {
          if (QString::compare(searchItem->shareItem()->unc(), share->unc(), Qt::CaseInsensitive) == 0)
          {
            searchItem->update();
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
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchDockWidget::slotSearchAbortActionTriggered(bool /*checked*/)
{
  //
  // Get the Search/Abort dual action and change the active property
  // 
  KDualAction *searchAbortAction = static_cast<KDualAction *>(m_actionCollection->action("search_abort_action"));
  
  //
  // Start or abort the search
  //
  if (searchAbortAction)
  {
    if (searchAbortAction->isActive())
    {
      // 
      // Start the search
      // 
      m_networkSearch->listWidget()->clear();

      QString searchItem = m_networkSearch->comboBox()->currentText();

      if (!searchItem.isEmpty())
      {
        Smb4KSearch::self()->search(m_networkSearch->comboBox()->currentText(), m_networkSearch);
        KCompletion *completion = m_networkSearch->comboBox()->completionObject();
        completion->addItem(searchItem);
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      //
      // Stop the search
      // 
      QString searchItem = m_networkSearch->comboBox()->currentText();
    
      if (!searchItem.isEmpty())
      {
        Smb4KSearch::self()->abort(m_networkSearch->comboBox()->currentText());
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


void Smb4KNetworkSearchDockWidget::slotSearchAbortActionChanged(bool active)
{
  //
  // Get the Search/Abort dual action
  // 
  KDualAction *searchAbortAction = static_cast<KDualAction *>(m_actionCollection->action("search_abort_action"));
  
  //
  // Change the shortcuts depending on the value of the 'active' argument
  // 
  if (searchAbortAction)
  {
    if (active)
    {
      m_actionCollection->setDefaultShortcut(searchAbortAction, QKeySequence(Qt::CTRL+Qt::Key_S));
    }
    else
    {
      qDebug() << "slotSearchAbortActionChanged(): Changed shortcut from QKeySequence(Qt::CTRL+Qt::Key_A) to QKeySequence::Cancel. Problems?";
      m_actionCollection->setDefaultShortcut(searchAbortAction, QKeySequence::Cancel);
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchDockWidget::slotClearActionTriggered(bool /*checked*/)
{
  // 
  // Clear the combo box and the list widget
  // 
  m_networkSearch->comboBox()->clear();
  m_networkSearch->comboBox()->clearEditText();
  m_networkSearch->listWidget()->clear();

  // 
  // Disable the actions
  // 
  m_actionCollection->action("search_abort_action")->setEnabled(false);
  m_actionCollection->action("clear_search_action")->setEnabled(false);
  m_actionCollection->action("mount_action")->setEnabled(false);
}


void Smb4KNetworkSearchDockWidget::slotBookmarkActionTriggered(bool /*checked*/)
{
  QList<QListWidgetItem *> selectedItems = m_networkSearch->listWidget()->selectedItems();
  QList<SharePtr> shares;

  for (QListWidgetItem *selectedItem : selectedItems)
  {
    Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>(selectedItem);
    
    if (item->type() == Share)
    {
      shares << item->shareItem();
    }
    else
    {
      // Do nothing
    }
  }

  Smb4KBookmarkHandler::self()->addBookmarks(shares, m_networkSearch);
}


void Smb4KNetworkSearchDockWidget::slotMountActionTriggered(bool /*checked*/)
{
  //
  // Get the selected items
  // 
  QList<QListWidgetItem *> selectedItems = m_networkSearch->listWidget()->selectedItems();

  if (selectedItems.size() > 1)
  {
    // 
    // For deciding what the mount action is supposed to do, i.e. mount
    // the (remaining) selected unmounted shares or unmounting all selected
    // mounted shares, we use the number of unmounted shares. If that is
    // greater than 0, we mount all shares that need to be mounted, otherwise
    // we unmount all selected shares.
    // 
    QList<SharePtr> unmounted, mounted;
    
    for (QListWidgetItem *item : selectedItems)
    {
      Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(item);
      
      if (searchItem && searchItem->shareItem()->isMounted())
      {
        mounted << searchItem->shareItem();
      }
      else if (searchItem && !searchItem->shareItem()->isMounted())
      {
        unmounted << searchItem->shareItem();
      }
      else
      {
        // Do nothing
      }
    }
    
    if (!unmounted.isEmpty())
    {
      // Mount the (remaining) unmounted shares.
      Smb4KMounter::self()->mountShares(unmounted, m_networkSearch);
    }
    else
    {
      // Unmount all shares.
      Smb4KMounter::self()->unmountShares(mounted, m_networkSearch);
    }
  }
  else
  {
    Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>(selectedItems.first());
    
    switch (searchItem->type())
    {
      case Smb4KNetworkSearchItem::Share:
      {
        if (!searchItem->shareItem()->isMounted())
        {
          Smb4KMounter::self()->mountShare(searchItem->shareItem(), m_networkSearch);
        }
        else
        {
          Smb4KMounter::self()->unmountShare(searchItem->shareItem(), false, m_networkSearch);
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
}


void Smb4KNetworkSearchDockWidget::slotMountActionChanged(bool active)
{
  //
  // Get the Mount/Unmount dual action
  // 
  KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action("mount_action"));
  
  //
  // Change the shortcuts depending on the value of the 'active' argument
  // 
  if (mountAction)
  {
    if (active)
    {
      m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL+Qt::Key_M));
    }
    else
    {
      m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL+Qt::Key_U));
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchDockWidget::slotIconSizeChanged(int group)
{
  switch (group)
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
      m_networkSearch->listWidget()->setIconSize(QSize(icon_size, icon_size));
      break;
    }
    default:
    {
      break;
    }
  }
}


