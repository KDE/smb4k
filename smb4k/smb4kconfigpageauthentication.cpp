/***************************************************************************
    The configuration page for the authentication settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2017 by Alexander Reinholdt
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
#include "smb4kconfigpageauthentication.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QMouseEvent>
#include <QCursor>
#include <QMenu>
#include <QHeaderView>
#include <QListWidgetItem>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KGuiItem>
#include <KIconThemes/KIconLoader>


Smb4KConfigPageAuthentication::Smb4KConfigPageAuthentication(QWidget *parent) : QTabWidget(parent)
{
  m_entries_displayed = false;
  m_loading_details = false;
  m_default_login = false;
  m_undo_removal = false;
  m_maybe_changed = false;
  m_auth_info = 0;
  
  //
  // General tab
  // 
  QWidget *general_tab          = new QWidget(this);

  QGridLayout *general_layout   = new QGridLayout(general_tab);
  general_layout->setSpacing(5);
  general_layout->setMargin(0);
  
  QGroupBox *password_box  = new QGroupBox(i18n("Password Storage"), general_tab);

  QGridLayout *pass_layout = new QGridLayout(password_box);
  pass_layout->setSpacing(5);

  QCheckBox *use_wallet    = new QCheckBox(Smb4KSettings::self()->useWalletItem()->label(),
                             password_box);
  use_wallet->setObjectName("kcfg_UseWallet");

  pass_layout->addWidget(use_wallet, 0, 0, 0);

  // Default login box
  QGroupBox *default_box   = new QGroupBox(i18n("Default Login"), general_tab);
  default_box->setObjectName("DefaultLoginBox");

  QGridLayout *def_layout  = new QGridLayout(default_box);
  def_layout->setSpacing(5);

  QCheckBox *default_auth  = new QCheckBox(Smb4KSettings::self()->useDefaultLoginItem()->label(),
                             default_box);
  default_auth->setObjectName("kcfg_UseDefaultLogin");

  def_layout->addWidget(default_auth, 0, 0, 1, 2, 0);

  QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

  general_layout->addWidget(password_box, 0, 0, 0);
  general_layout->addWidget(default_box, 1, 0, 0);
  general_layout->addItem(spacer, 2, 0, 1, 1, 0);

  connect(use_wallet,   SIGNAL(toggled(bool)),
           this,         SLOT(slotKWalletButtonToggled(bool)));

  connect(default_auth, SIGNAL(toggled(bool)),
           this,         SLOT(slotDefaultLoginToggled(bool)));

  slotKWalletButtonToggled(use_wallet->isChecked());
  slotDefaultLoginToggled(default_auth->isChecked());
  
  insertTab(GeneralTab, general_tab, i18n("General Settings"));
  
  //
  // Logins tab
  //
  QWidget *logins_tab        = new QWidget(this);
  
  QGridLayout *logins_layout = new QGridLayout(logins_tab);
  logins_layout->setSpacing(5);
  logins_layout->setMargin(0);
  
  // The list view that shows the wallet entriea
  m_entries_widget = new QListWidget(logins_tab);
  m_entries_widget->setDragDropMode(QListWidget::NoDragDrop);
  m_entries_widget->setSelectionMode(QListWidget::SingleSelection);
  m_entries_widget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_entries_widget->viewport()->installEventFilter(this);
  
  // Load button
  KGuiItem load_item = KGuiItem(i18n("Load"), "document-open",
                                i18n("Load the entries stored in the wallet."),
                                i18n("The login information that was stored by Smb4K will be loaded from the wallet. "
                                      "If you chose to not use the wallet, pressing this button will have no effect."));
  m_load_button = new QPushButton(logins_tab);
  KGuiItem::assign(m_load_button, load_item);
  
  // The Save button
  KGuiItem save_item = KGuiItem(i18n("Save"), "document-save-all",
                                 i18n("Save the entries to the wallet."),
                                 i18n("All modifications you applied are saved to the wallet."));
  m_save_button = new QPushButton(logins_tab);
  KGuiItem::assign(m_save_button, save_item);
  
  m_details_box = new QCheckBox(i18n("Show details"), logins_tab);
  m_details_box->setToolTip(i18n("Show the details of the selected entry."));
  m_details_box->setWhatsThis(i18n("Marking this check box will show the details of the selected login information below."));
  m_details_box->setEnabled(false);
  
  m_details_widget = new QTableWidget(logins_tab);
  m_details_widget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_details_widget->horizontalHeader()->setVisible(false);
  m_details_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_details_widget->verticalHeader()->setVisible(false);
  m_details_widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_details_widget->viewport()->installEventFilter(this);
  m_details_widget->setEnabled(false);
  
  logins_layout->addWidget(m_entries_widget, 0, 0, 4, 1, 0);
  logins_layout->addWidget(m_load_button, 0, 2, 0);
  logins_layout->addWidget(m_save_button, 1, 2, 0);
  logins_layout->addWidget(m_details_box, 2, 1, 1, 2, 0);
  logins_layout->addWidget(m_details_widget, 3, 1, 1, 2, 0);
  
  m_collection = new KActionCollection(logins_tab);  
  QAction *remove_action = new QAction(KDE::icon("edit-delete"), i18n("Remove"), this);
  QAction *clear_action = new QAction(KDE::icon("edit-clear-list"), i18n("Clear List"), this);
  QAction *edit_action = new QAction(KDE::icon("edit-rename"), i18n("Edit"), this);
  QAction *undo_details_action = new QAction(KDE::icon("edit-undo"), i18n("Undo"), this);
  QAction *undo_list_action = new QAction(KDE::icon("edit-undo"), i18n("Undo"), this);
  
  m_collection->addAction("remove_action", remove_action);
  m_collection->addAction("clear_action", clear_action);
  m_collection->addAction("edit_action", edit_action);
  m_collection->addAction("undo_details_action", undo_details_action);
  m_collection->addAction("undo_list_action", undo_list_action);

  remove_action->setEnabled(false);
  clear_action->setEnabled(false);
  edit_action->setEnabled(false);
  undo_details_action->setEnabled(false);
  undo_list_action->setEnabled(false);
  
  m_collection->addAction("remove_action", remove_action);
  m_collection->addAction("clear_action", clear_action);
  m_collection->addAction("edit_action", edit_action);
  m_collection->addAction("undo_details_action", undo_details_action);
  m_collection->addAction("undo_list_action", undo_list_action);
  
  m_entries_menu = new KActionMenu(m_entries_widget);
  m_entries_menu->addAction(remove_action);
  m_entries_menu->addAction(clear_action);
  m_entries_menu->addAction(undo_list_action);
  
  m_details_menu = new KActionMenu(m_details_widget);
  m_details_menu->addAction(edit_action);
  m_details_menu->addAction(undo_details_action);
  
  connect(m_load_button,    SIGNAL(clicked(bool)),
           this,             SIGNAL(loadWalletEntries()));
  
  connect(m_save_button,    SIGNAL(clicked(bool)),
           this,             SIGNAL(saveWalletEntries()));
           
  connect(m_save_button,    SIGNAL(clicked(bool)),
           this,             SLOT(slotSaveClicked(bool)));
  
  connect(m_details_box,    SIGNAL(clicked(bool)),
           this,             SLOT(slotDetailsClicked(bool)));
           
  connect(m_entries_widget, SIGNAL(itemSelectionChanged()),
           this,             SLOT(slotItemSelectionChanged()));
           
  connect(m_entries_widget, SIGNAL(customContextMenuRequested(QPoint)),
           this,             SLOT(slotShowListWidgetContextMenu(QPoint)));
           
  connect(m_details_widget, SIGNAL(cellChanged(int,int)),
           this,             SLOT(slotDetailsChanged(int,int)));
           
  connect(m_details_widget, SIGNAL(customContextMenuRequested(QPoint)),
           this,             SLOT(slotShowTableWidgetContextMenu(QPoint)));
           
  connect(remove_action,    SIGNAL(triggered(bool)),
           this,             SLOT(slotRemoveActionTriggered(bool)));
           
  connect(clear_action,     SIGNAL(triggered(bool)),
           this,             SLOT(slotClearActionTriggered(bool)));
           
  connect(undo_list_action, SIGNAL(triggered(bool)),
           this,             SLOT(slotUndoListActionTriggered(bool)));
           
  connect(edit_action,      SIGNAL(triggered(bool)),
           this,             SLOT(slotEditActionTriggered(bool)));
           
  connect(undo_details_action, SIGNAL(triggered(bool)),
           this,             SLOT(slotUndoDetailsActionTriggered(bool)));
           
  m_load_button->setFocus();
  
  insertTab(WalletEntriesTab, logins_tab, i18n("Wallet Entries"));
}


Smb4KConfigPageAuthentication::~Smb4KConfigPageAuthentication()
{
}


void Smb4KConfigPageAuthentication::insertWalletEntries(const QList<Smb4KAuthInfo *> &list)
{
  m_entries_list = list;
  m_maybe_changed = false;
  emit walletEntriesModified();
}


void Smb4KConfigPageAuthentication::displayWalletEntries()
{
  // Clear the list widget if necessary
  if (m_entries_widget->count() != 0)
  {
    m_entries_widget->clear();
  }
  else
  {
    // Do nothing
  }
  
  for (int i = 0; i < m_entries_list.size(); ++i)
  {
    switch (m_entries_list.at(i)->type())
    {
      case UnknownNetworkItem:
      {
        (void) new QListWidgetItem(KDE::icon("dialog-password"), i18n("Default Login"), m_entries_widget);
        break;
      }
      default:
      {
        (void) new QListWidgetItem(KDE::icon("dialog-password"), m_entries_list.at(i)->unc(), m_entries_widget);
        break;
      }
    }
  }
  
  m_entries_widget->sortItems(/* ascending */);
  
  m_entries_displayed = true;
  
  m_collection->action("clear_action")->setEnabled(!m_entries_list.isEmpty());
}


bool Smb4KConfigPageAuthentication::eventFilter(QObject *object, QEvent *e)
{
  if (object == m_entries_widget->viewport())
  {
    // If the user clicked on the viewport of the entries view, clear 
    // the details widget and the "Details" button, if no item 
    // is under the mouse.
    if (e->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *event = static_cast<QMouseEvent *>(e);
      QPoint pos = m_entries_widget->mapFromGlobal(event->globalPos());
        
      if (!m_entries_widget->itemAt(pos))
      {
        clearDetails();
        m_entries_widget->clearSelection();
        m_collection->action("remove_action")->setEnabled(false);
      }
      else
      {
        // Do nothing. Is managed by slotItemSelectionChanged().
      }
    }
    else
    {
      // Do nothing
    }
    
    return m_entries_widget->viewport()->eventFilter(object, e);
  }
  else if (object == m_details_widget->viewport())
  {
    // Enable/disable the edit action.
    if (e->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *event = static_cast<QMouseEvent *>(e);
      QPoint pos = m_details_widget->mapFromGlobal(event->globalPos());
      
      if (m_details_widget->columnAt(pos.x()) > 0 && m_details_widget->rowAt(pos.y()) > 0)
      {
        m_collection->action("edit_action")->setEnabled(true);
      }
      else
      {
        m_collection->action("edit_action")->setEnabled(false);
      }
    }
    else
    {
      // Do nothing
    }
    
    return m_details_widget->viewport()->eventFilter(object, e);
  }
  else
  {
    // Do nothing
  }
  
  return QTabWidget::eventFilter(object, e);
}


void Smb4KConfigPageAuthentication::showDetails(Smb4KAuthInfo *authInfo)
{
  m_loading_details = true;
  
  switch (authInfo->type())
  {
    case Host:
    case Share:
    {
      m_details_widget->setColumnCount(2);
      m_details_widget->setRowCount(4);
          
      QTableWidgetItem *entry_label = new QTableWidgetItem(i18n("Entry"));
      entry_label->setFlags(entry_label->flags() & Qt::ItemIsEditable);
      entry_label->setForeground(palette().text());
          
      QTableWidgetItem *entry = new QTableWidgetItem(authInfo->unc());
      entry->setFlags(entry->flags() & Qt::ItemIsEditable);
      entry->setForeground(palette().text());
          
      QTableWidgetItem *workgroup_label = new QTableWidgetItem(i18n("Workgroup"));
      workgroup_label->setFlags(workgroup_label->flags() & Qt::ItemIsEditable);
      workgroup_label->setForeground(palette().text());
          
      QTableWidgetItem *login_label = new QTableWidgetItem(i18n("Login"));
      login_label->setFlags(login_label->flags() & Qt::ItemIsEditable);
      login_label->setForeground(palette().text());
          
      QTableWidgetItem *password_label = new QTableWidgetItem(i18n("Password"));
      password_label->setFlags(password_label->flags() & Qt::ItemIsEditable);
      password_label->setForeground(palette().text());
          
      m_details_widget->setItem(0, 0, entry_label);
      m_details_widget->setItem(0, 1, entry);
      m_details_widget->setItem(1, 0, workgroup_label);
      m_details_widget->setItem(1, 1, new QTableWidgetItem(authInfo->workgroupName()));
      m_details_widget->setItem(2, 0, login_label);
      m_details_widget->setItem(2, 1, new QTableWidgetItem(authInfo->userName()));
      m_details_widget->setItem(3, 0, password_label);
      m_details_widget->setItem(3, 1, new QTableWidgetItem(authInfo->password()));
          
      break;
    }
    default:
    {
      m_details_widget->setColumnCount(2);
      m_details_widget->setRowCount(3);
          
      QTableWidgetItem *entry_label = new QTableWidgetItem(i18n("Entry"));
      entry_label->setFlags(entry_label->flags() & Qt::ItemIsEditable);
      entry_label->setForeground(palette().text());
          
      QTableWidgetItem *entry = new QTableWidgetItem(i18n("Default Login"));
      entry->setFlags(entry->flags() & Qt::ItemIsEditable);
      entry->setForeground(palette().text());
          
      QTableWidgetItem *login_label = new QTableWidgetItem(i18n("Login"));
      login_label->setFlags(login_label->flags() & Qt::ItemIsEditable);
      login_label->setForeground(palette().text());
          
      QTableWidgetItem *password_label = new QTableWidgetItem(i18n("Password"));
      password_label->setFlags(password_label->flags() & Qt::ItemIsEditable);
      password_label->setForeground(palette().text());
          
      m_details_widget->setItem(0, 0, entry_label);
      m_details_widget->setItem(0, 1, entry);
      m_details_widget->setItem(1, 0, login_label);
      m_details_widget->setItem(1, 1, new QTableWidgetItem(authInfo->userName()));
      m_details_widget->setItem(2, 0, password_label);
      m_details_widget->setItem(2, 1, new QTableWidgetItem(authInfo->password()));
          
      break;
    }
  }

  m_auth_info = authInfo;
  m_loading_details = false;
}


void Smb4KConfigPageAuthentication::clearDetails()
{
  // Uncheck the "Show details" check box and enable/disable it.
  m_details_box->setChecked(false);
  m_details_box->setEnabled(!m_entries_widget->selectedItems().isEmpty());
  
  // Clear the table widget.
  m_details_widget->clear();
  m_details_widget->setRowCount(0);
  m_details_widget->setColumnCount(0);
  m_details_widget->setEnabled(!m_entries_widget->selectedItems().isEmpty());
  
  // Clear the auth info object and disable the "Undo" action
  m_auth_info = NULL;
  m_collection->action("undo_details_action")->setEnabled(false);
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigPageAuthentication::slotKWalletButtonToggled(bool checked)
{
  findChild<QGroupBox *>("DefaultLoginBox")->setEnabled(checked);
}


void Smb4KConfigPageAuthentication::slotDefaultLoginToggled(bool checked)
{
  if (checked && !Smb4KSettings::useDefaultLogin())
  {
    emit setDefaultLogin();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigPageAuthentication::slotDetailsClicked(bool checked)
{
  QList<QListWidgetItem *> selected_items = m_entries_widget->selectedItems();
  
  if (checked && !selected_items.isEmpty())
  {
    // Since we have single selection defined, there is definitely
    // only one entry in the list.
    for (int i = 0; i < m_entries_list.size(); ++i)
    {
      if (QString::compare(selected_items.first()->text(), m_entries_list.at(i)->unc()) == 0 ||
           (QString::compare(selected_items.first()->text(), i18n("Default Login")) == 0 &&
            m_entries_list.at(i)->type() == UnknownNetworkItem))
      {
        showDetails(m_entries_list.at(i));
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
    clearDetails();
  }
}


void Smb4KConfigPageAuthentication::slotItemSelectionChanged()
{
  // Clear details stuff
  clearDetails();
  
  // Enable the details and remove action
  m_collection->action("remove_action")->setEnabled(true);
}


void Smb4KConfigPageAuthentication::slotDetailsChanged(int row, int column)
{
  if (!m_loading_details)
  {
    for (int i = 0; i < m_entries_list.size(); ++i)
    {
      if (QString::compare(m_details_widget->item(0, 1)->text(), m_entries_list.at(i)->unc()) == 0 ||
           (QString::compare(m_details_widget->item(0, 1)->text(), i18n("Default Login")) == 0 &&
           m_entries_list.at(i)->type() == UnknownNetworkItem))
      {
        switch (m_entries_list.at(i)->type())
        {
          case Host:
          case Share:
          {
            if (column == 1)
            {
              switch (row)
              {
                case 1: // Workgroup
                {
                  m_entries_list[i]->setWorkgroupName(m_details_widget->item(row, column)->text());
                  break;
                }
                case 2: // Login
                {
                  m_entries_list[i]->setUserName(m_details_widget->item(row, column)->text());
                  break;
                }
                case 3: // Password
                {
                  m_entries_list[i]->setPassword(m_details_widget->item(row, column)->text());
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
            
            break;
          }
          default:
          {
            if (column == 1)
            {
              switch (row)
              {
                case 1: // Login
                {
                  m_entries_list[i]->setUserName(m_details_widget->item(row, column)->text());
                  break;
                }
                case 2: // Password
                {
                  m_entries_list[i]->setPassword(m_details_widget->item(row, column)->text());
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
            
            break;
          }
        }
        
        break;
      }
      else
      {
        continue;
      }
    }
    
    m_collection->action("undo_details_action")->setEnabled(true);
    m_maybe_changed = true;
    emit walletEntriesModified();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigPageAuthentication::slotShowListWidgetContextMenu(const QPoint &pos)
{
  m_entries_menu->menu()->popup(m_entries_widget->viewport()->mapToGlobal(pos));
}


void Smb4KConfigPageAuthentication::slotShowTableWidgetContextMenu(const QPoint &pos)
{
  m_details_menu->menu()->popup(m_details_widget->viewport()->mapToGlobal(pos));
}


void Smb4KConfigPageAuthentication::slotRemoveActionTriggered(bool /*checked*/)
{
  if ((m_details_widget->rowCount() != 0 && m_details_widget->columnCount() != 0) &&
       QString::compare(m_entries_widget->currentItem()->text(), m_details_widget->item(0, 1)->text()) == 0)
  {
    clearDetails();
  }
  else
  {
    // Do nothing
  }

  for (int i = 0; i < m_entries_list.size(); ++i)
  {
    if (QString::compare(m_entries_widget->currentItem()->text(), m_entries_list.at(i)->unc()) == 0 ||
         (QString::compare(m_entries_widget->currentItem()->text(), i18n("Default Login")) == 0 &&
         m_entries_list.at(i)->type() == UnknownNetworkItem))
    {
      switch (m_entries_list.at(i)->type())
      {
        case UnknownNetworkItem:
        {
          QCheckBox *default_login = findChild<QCheckBox *>("kcfg_UseDefaultLogin");
          m_default_login = default_login->isChecked();
          default_login->setChecked(false);
          break;
        }
        default:
        {
          break;
        }
      }
      
      delete m_entries_list.takeAt(i);
      break;
    }
    else
    {
      continue;
    }
  }

  delete m_entries_widget->currentItem();
  
  m_collection->action("undo_list_action")->setEnabled(true);
  m_collection->action("clear_action")->setEnabled((m_entries_widget->count() != 0));
  
  m_maybe_changed = true;
  emit walletEntriesModified();
}


void Smb4KConfigPageAuthentication::slotClearActionTriggered(bool /*checked*/)
{
  clearDetails();
  
  while (m_entries_widget->count() != 0)
  {
    delete m_entries_widget->item(0);
  }
  
  while(!m_entries_list.isEmpty())
  {
    delete m_entries_list.takeFirst();
  }
  
  m_collection->action("undo_list_action")->setEnabled(true);
  m_collection->action("clear_action")->setEnabled(false);
  
  QCheckBox *default_login = findChild<QCheckBox *>("kcfg_UseDefaultLogin");
  m_default_login = default_login->isChecked();
  default_login->setChecked(false);
  
  m_maybe_changed = true;
  emit walletEntriesModified();
}


void Smb4KConfigPageAuthentication::slotUndoListActionTriggered(bool /*checked*/)
{
  m_undo_removal = true;
  emit loadWalletEntries();
  findChild<QCheckBox *>("kcfg_UseDefaultLogin")->setChecked(m_default_login);
  m_undo_removal = false;
}


void Smb4KConfigPageAuthentication::slotEditActionTriggered(bool /*checked*/)
{
  QPoint pos = m_details_widget->mapFromGlobal(cursor().pos());
  
  if (m_details_widget->columnAt(pos.x()) > 0 && m_details_widget->rowAt(pos.y()) > 0)
  {
    m_details_widget->editItem(m_details_widget->currentItem());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigPageAuthentication::slotUndoDetailsActionTriggered(bool /*checked*/)
{
  showDetails(m_auth_info);
  
  for (int i = 0; i < m_entries_list.size(); ++i)
  {
    if (QString::compare(m_auth_info->unc(), m_entries_list.at(i)->unc()) == 0 ||
         (m_auth_info->type() == UnknownNetworkItem && m_auth_info->type() == m_entries_list.at(i)->type()))
    {
      switch (m_auth_info->type())
      {
        case Host:
        case Share:
        {
          m_entries_list[i]->setWorkgroupName(m_auth_info->workgroupName());
          m_entries_list[i]->setUserName(m_auth_info->userName());
          m_entries_list[i]->setPassword(m_auth_info->password());
          break;
        }
        default:
        {
          m_entries_list[i]->setUserName(m_auth_info->userName());
          m_entries_list[i]->setPassword(m_auth_info->password());
          break;
        }
      }
      
      break;
    }
    else
    {
      continue;
    }
  }
  
  m_collection->action("undo_details_action")->setEnabled(false);
  
  // Do not set m_dirty = false here, because we do not known whether another
  // entry in the list as not already been altered.
  
  emit walletEntriesModified();
}


void Smb4KConfigPageAuthentication::slotSaveClicked(bool /*checked*/)
{
  m_collection->action("remove_action")->setEnabled(false);
  m_collection->action("clear_action")->setEnabled((m_entries_widget->count() != 0));
  m_collection->action("undo_list_action")->setEnabled(false);
  m_collection->action("edit_action")->setEnabled(false);
  m_collection->action("undo_details_action")->setEnabled(false);
  
  m_maybe_changed = false;
  emit walletEntriesModified();
  
  m_auth_info = NULL;
}

