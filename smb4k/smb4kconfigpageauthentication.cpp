/***************************************************************************
    The configuration page for the authentication settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2019 by Alexander Reinholdt
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
#include <KIconThemes/KIconLoader>


Smb4KConfigPageAuthentication::Smb4KConfigPageAuthentication(QWidget *parent) : QWidget(parent)
{
  m_entries_displayed = false;
  m_loading_details = false;
  m_maybe_changed = false;
  
  //
  // Layout 
  // 
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  layout->setMargin(0);
  
  //
  // Settings group box
  // 
  QGroupBox *settingsBox = new QGroupBox(i18n("Settings"), this);
  
  QVBoxLayout *settingsBoxLayout = new QVBoxLayout(settingsBox);
  settingsBoxLayout->setSpacing(5);
  
  // Wallet usage
  QCheckBox *useWallet = new QCheckBox(Smb4KSettings::self()->useWalletItem()->label(), settingsBox);
  useWallet->setObjectName("kcfg_UseWallet");
  
  connect(useWallet, SIGNAL(toggled(bool)), this, SLOT(slotKWalletButtonToggled(bool)));

  settingsBoxLayout->addWidget(useWallet, 0);

  // Default login
  QCheckBox *defaultAuth  = new QCheckBox(Smb4KSettings::self()->useDefaultLoginItem()->label(), settingsBox);
  defaultAuth->setObjectName("kcfg_UseDefaultLogin");
  
  connect(defaultAuth, SIGNAL(toggled(bool)), this, SLOT(slotDefaultLoginToggled(bool)));

  settingsBoxLayout->addWidget(defaultAuth, 0);

  layout->addWidget(settingsBox, 0);

  // Adjustments
  slotKWalletButtonToggled(useWallet->isChecked());
  slotDefaultLoginToggled(defaultAuth->isChecked());
  
  //
  // Wallet Entries group box
  // 
  QGroupBox *walletEntriesBox = new QGroupBox(i18n("Wallet Entries"), this);
  
  QGridLayout *walletEntriesBoxLayout = new QGridLayout(walletEntriesBox);
  walletEntriesBoxLayout->setSpacing(5);
  
  // The list view that shows the wallet entriea
  m_entries_widget = new QListWidget(walletEntriesBox);
  m_entries_widget->setDragDropMode(QListWidget::NoDragDrop);
  m_entries_widget->setSelectionMode(QListWidget::SingleSelection);
  m_entries_widget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_entries_widget->viewport()->installEventFilter(this);
  
  // Load button
  QPushButton *loadButton = new QPushButton(walletEntriesBox);
  loadButton->setObjectName("LoadButton");
  loadButton->setText(i18n("Load"));
  loadButton->setIcon(KDE::icon("document-open"));
  loadButton->setWhatsThis(i18n("The login information that was stored by Smb4K will be loaded from the wallet. "
                                   "If you chose to not use the wallet, pressing this button will have no effect."));
  
  // The Remove button
  QPushButton *removeButton = new QPushButton(walletEntriesBox);
  removeButton->setObjectName("RemoveButton");
  removeButton->setText(i18n("Remove"));
  removeButton->setIcon(KDE::icon("edit-delete"));
  removeButton->setWhatsThis(i18n("The selected entry is removed from the wallet."));
  removeButton->setEnabled(false);
  
  // The Clear button
  QPushButton *clearButton = new QPushButton(walletEntriesBox);
  clearButton->setObjectName("ClearButton");
  clearButton->setText(i18n("Clear"));
  clearButton->setIcon(KDE::icon("edit-clear-list"));
  clearButton->setWhatsThis(i18n("All entries are removed from the wallet."));
  clearButton->setEnabled(false);
  
  // The Save button
  QPushButton *saveButton = new QPushButton(walletEntriesBox);
  saveButton->setObjectName("SaveButton");
  saveButton->setText(i18n("Save"));
  saveButton->setIcon(KDE::icon("document-save-all"));
  saveButton->setWhatsThis(i18n("All modifications you applied are saved to the wallet."));
  saveButton->setEnabled(false);
  
  m_details_box = new QCheckBox(i18n("Show details"), walletEntriesBox);
  m_details_box->setToolTip(i18n("Show the details of the selected entry."));
  m_details_box->setWhatsThis(i18n("Marking this check box will show the details of the selected login information below."));
  m_details_box->setEnabled(false);
  
  m_details_widget = new QTableWidget(walletEntriesBox);
  m_details_widget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_details_widget->horizontalHeader()->setVisible(false);
  m_details_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_details_widget->verticalHeader()->setVisible(false);
  m_details_widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_details_widget->viewport()->installEventFilter(this);
  m_details_widget->setEnabled(false);
  
  walletEntriesBoxLayout->addWidget(m_entries_widget, 0, 0, 6, 1, 0);
  walletEntriesBoxLayout->addWidget(loadButton, 0, 2, 0);
  walletEntriesBoxLayout->addWidget(removeButton, 1, 2, 0);
  walletEntriesBoxLayout->addWidget(clearButton, 2, 2, 0);
  walletEntriesBoxLayout->addWidget(saveButton, 3, 2, 0);
  walletEntriesBoxLayout->addWidget(m_details_box, 4, 2, 0);
  walletEntriesBoxLayout->addWidget(m_details_widget, 5, 2, 0);
//   walletEntriesBoxLayout->addWidget(m_details_box, 4, 1, 1, 2, 0);
//   walletEntriesBoxLayout->addWidget(m_details_widget, 5, 1, 1, 2, 0);
  
  layout->addWidget(walletEntriesBox, 0);
  
  //
  // Connections
  // 
  connect(loadButton, SIGNAL(clicked(bool)), this, SIGNAL(loadWalletEntries()));
  connect(saveButton, SIGNAL(clicked(bool)), this, SIGNAL(saveWalletEntries()));
  connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveClicked(bool)));
  connect(removeButton, SIGNAL(clicked(bool)), this, SLOT(slotRemoveActionTriggered(bool)));
  connect(clearButton, SIGNAL(clicked(bool)), this, SLOT(slotClearActionTriggered(bool)));
  connect(m_details_box, SIGNAL(clicked(bool)), this, SLOT(slotDetailsClicked(bool)));
  connect(m_entries_widget, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
  connect(m_details_widget, SIGNAL(cellChanged(int,int)), this, SLOT(slotDetailsChanged(int,int)));
           
  loadButton->setFocus();
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
  
  for (Smb4KAuthInfo *authInfo : m_entries_list)
  {
    switch (authInfo->type())
    {
      case UnknownNetworkItem:
      {
        (void) new QListWidgetItem(KDE::icon("dialog-password"), i18n("Default Login"), m_entries_widget);
        break;
      }
      default:
      {
        (void) new QListWidgetItem(KDE::icon("dialog-password"), authInfo->displayString(), m_entries_widget);
        break;
      }
    }
  }
  
  m_entries_widget->sortItems(/* ascending */);
  
  m_entries_displayed = true;
  
  //
  // Enable buttons
  // 
  findChild<QPushButton *>("SaveButton")->setEnabled(m_entries_widget->count() != 0);
  findChild<QPushButton *>("ClearButton")->setEnabled(m_entries_widget->count() != 0);
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
        findChild<QPushButton *>("RemoveButton")->setEnabled(false);
      }
    }
    
    return m_entries_widget->viewport()->eventFilter(object, e);
  }
  
  return QWidget::eventFilter(object, e);
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
          
      QTableWidgetItem *entry = new QTableWidgetItem(authInfo->displayString());
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
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KConfigPageAuthentication::slotKWalletButtonToggled(bool checked)
{
  findChild<QCheckBox *>("kcfg_UseDefaultLogin")->setEnabled(checked);
}


void Smb4KConfigPageAuthentication::slotDefaultLoginToggled(bool checked)
{
  if (checked && !Smb4KSettings::useDefaultLogin())
  {
    emit setDefaultLogin();
  }
}


void Smb4KConfigPageAuthentication::slotDetailsClicked(bool checked)
{
  QList<QListWidgetItem *> selectedItems = m_entries_widget->selectedItems();
  
  if (checked && !selectedItems.isEmpty())
  {
    // Since we have single selection defined, there is definitely
    // only one entry in the list.
    for (Smb4KAuthInfo *authInfo : m_entries_list)
    {
      if (QString::compare(selectedItems.first()->text(), authInfo->displayString()) == 0 ||
          (QString::compare(selectedItems.first()->text(), i18n("Default Login")) == 0 && authInfo->type() == UnknownNetworkItem))
      {
        showDetails(authInfo);
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
  findChild<QPushButton *>("RemoveButton")->setEnabled(true);
}


void Smb4KConfigPageAuthentication::slotDetailsChanged(int row, int column)
{
  if (!m_loading_details)
  {
    for (Smb4KAuthInfo *authInfo : m_entries_list)
    {
      if (QString::compare(m_details_widget->item(0, 1)->text(), authInfo->displayString()) == 0 ||
          (QString::compare(m_details_widget->item(0, 1)->text(), i18n("Default Login")) == 0 && authInfo->type() == UnknownNetworkItem))
      {
        switch (authInfo->type())
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
                  authInfo->setWorkgroupName(m_details_widget->item(row, column)->text());
                  break;
                }
                case 2: // Login
                {
                  authInfo->setUserName(m_details_widget->item(row, column)->text());
                  break;
                }
                case 3: // Password
                {
                  authInfo->setPassword(m_details_widget->item(row, column)->text());
                  break;
                }
                default:
                {
                  break;
                }
              }
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
                  authInfo->setUserName(m_details_widget->item(row, column)->text());
                  break;
                }
                case 2: // Password
                {
                  authInfo->setPassword(m_details_widget->item(row, column)->text());
                  break;
                }
                default:
                {
                  break;
                }
              }
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
    
    m_maybe_changed = true;
    emit walletEntriesModified();
  }
}


void Smb4KConfigPageAuthentication::slotRemoveActionTriggered(bool /*checked*/)
{
  if ((m_details_widget->rowCount() != 0 && m_details_widget->columnCount() != 0) &&
       QString::compare(m_entries_widget->currentItem()->text(), m_details_widget->item(0, 1)->text()) == 0)
  {
    clearDetails();
  }

  for (int i = 0; i < m_entries_list.size(); ++i)
  {
    if (QString::compare(m_entries_widget->currentItem()->text(), m_entries_list.at(i)->displayString()) == 0 ||
        (QString::compare(m_entries_widget->currentItem()->text(), i18n("Default Login")) == 0 && m_entries_list.at(i)->type() == UnknownNetworkItem))
    {
      switch (m_entries_list.at(i)->type())
      {
        case UnknownNetworkItem:
        {
          QCheckBox *default_login = findChild<QCheckBox *>("kcfg_UseDefaultLogin");
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
  
  //
  // Enabled buttons
  // 
  findChild<QPushButton *>("ClearButton")->setEnabled((m_entries_widget->count() != 0));
  
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
  
  //
  // Enabled widgets
  // 
  findChild<QPushButton *>("ClearButton")->setEnabled(false);
  
  QCheckBox *default_login = findChild<QCheckBox *>("kcfg_UseDefaultLogin");
  default_login->setChecked(false);
  
  m_maybe_changed = true;
  emit walletEntriesModified();
}


void Smb4KConfigPageAuthentication::slotEditActionTriggered(bool /*checked*/)
{
  QPoint pos = m_details_widget->mapFromGlobal(cursor().pos());
  
  if (m_details_widget->columnAt(pos.x()) > 0 && m_details_widget->rowAt(pos.y()) > 0)
  {
    m_details_widget->editItem(m_details_widget->currentItem());
  }
}


void Smb4KConfigPageAuthentication::slotSaveClicked(bool /*checked*/)
{
  //
  // Disable buttons
  // 
  findChild<QPushButton *>("RemoveButton")->setEnabled(false);
  findChild<QPushButton *>("ClearButton")->setEnabled((m_entries_widget->count() != 0));
  
  //
  // Clear the selection in the list view
  // 
  m_entries_widget->clearSelection();
  
  m_maybe_changed = false;
  emit walletEntriesModified();
}

