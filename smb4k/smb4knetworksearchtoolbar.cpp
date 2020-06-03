/***************************************************************************
    This class provides the network search toolbar.
                             -------------------
    begin                : Su Dec 23 2018
    copyright            : (C) 2018-2020 by Alexander Reinholdt
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
#include "smb4knetworksearchtoolbar.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QDebug>

// KDE includes
#include <KCompletion/KComboBox>
#include <KCompletion/KLineEdit>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KDualAction>
#include <KIconThemes/KIconLoader>


Smb4KNetworkSearchToolBar::Smb4KNetworkSearchToolBar(QWidget* parent)
: QToolBar(parent), m_iterator(QStringList())
{
  //
  // Set up tool bar
  //
  // Use the settings suggested by the note provided in the 'Detailed Description'
  // section of KToolBar (https://api.kde.org/frameworks/kxmlgui/html/classKToolBar.html)
  // 
  setToolButtonStyle(Qt::ToolButtonFollowStyle);
  setProperty("otherToolbar", true);
  
  //
  // The Close action
  // 
  QAction *closeAction = new QAction(this);
  closeAction->setObjectName("CloseAction");
  closeAction->setIcon(KDE::icon("window-close"));
  closeAction->setText(i18n("Close"));
  
  connect(closeAction, SIGNAL(triggered(bool)), this, SLOT(slotCloseButtonPressed()));
  
  addAction(closeAction);
  
  //
  // The search combo box
  // 
  KComboBox *comboBox = new KComboBox(true, this);
  comboBox->setObjectName("SearchCombo");
  comboBox->lineEdit()->setPlaceholderText(i18n("Search string"));
  comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  comboBox->setCompletionMode(KCompletion::CompletionPopupAuto);
  
  connect(comboBox, SIGNAL(returnPressed()), this, SLOT(slotReturnKeyPressed()));
  // FIXME: Add a connection to the clearSearch() slot
  
  addWidget(comboBox);
  
  //
  // The search dual action
  //
  KDualAction *searchAction = new KDualAction(this);
  searchAction->setObjectName("SearchAction");
  searchAction->setInactiveIcon(KDE::icon("search"));
  searchAction->setInactiveText(i18n("Search"));
  searchAction->setActiveIcon(KDE::icon("process-stop"));
  searchAction->setActiveText(i18n("Stop"));
  searchAction->setAutoToggle(false);
  
  connect(searchAction, SIGNAL(triggered(bool)), this, SLOT(slotSearchActionTriggered()));
  
  addAction(searchAction);
  
  //
  // Go one item down action
  //
  QAction *downAction = new QAction(this);
  downAction->setObjectName("DownAction");
  downAction->setIcon(KDE::icon("go-down-search"));
  downAction->setText(i18n("Item Down"));
  downAction->setEnabled(false);
  
  connect(downAction, SIGNAL(triggered(bool)), this, SLOT(slotDownActionTriggered()));
  
  addAction(downAction);
  
  //
  // Go one item up action
  // 
  QAction *upAction = new QAction(this);
  upAction->setObjectName("UpAction");
  upAction->setIcon(KDE::icon("go-up-search"));
  upAction->setText(i18n("Item Up"));
  upAction->setEnabled(false);
  
  connect(upAction, SIGNAL(triggered(bool)), this, SLOT(slotUpActionTriggered()));
  
  addAction(upAction);
  
  /**
   * Clear the search
   */
  QAction *clearAction = new QAction(this);
  clearAction->setObjectName("ClearAction");
  clearAction->setIcon(KDE::icon("edit-clear-all"));
  clearAction->setText(i18n("Clear"));
  clearAction->setEnabled(false);
  
  connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(slotClearSearch()));
  
  addAction(clearAction);
}


Smb4KNetworkSearchToolBar::~Smb4KNetworkSearchToolBar()
{
}


void Smb4KNetworkSearchToolBar::prepareInput()
{
  //
  // Get the search combo box
  // 
  KComboBox *comboBox = findChild<KComboBox *>("SearchCombo");
  
  //
  // Set the keyboard focus to the lineedit
  //
  comboBox->lineEdit()->setFocus();
}


void Smb4KNetworkSearchToolBar::setActiveState(bool active)
{
  //
  // Get the search dual action and set the active state
  // 
  KDualAction *searchAction = findChild<KDualAction *>("SearchAction");
  searchAction->setActive(active);
  
  //
  // Get the search combo box and disable/enable it
  // 
  KComboBox *comboBox = findChild<KComboBox *>("SearchCombo");
  comboBox->setEnabled(!active);
}


void Smb4KNetworkSearchToolBar::clearSearch()
{
  //
  // Clear the list of search results
  // 
  m_searchResults.clear();
  
  //
  // Clear the combo box
  // 
  KComboBox *comboBox = findChild<KComboBox *>("SearchCombo");
  comboBox->clear();
  comboBox->clearEditText();
  
  //
  // Get the down action and disable it
  // 
  QAction *downAction = findChild<QAction *>("DownAction");
  downAction->setEnabled(!m_searchResults.isEmpty());
  
  //
  // Get the up action and disable it
  // 
  QAction *upAction = findChild<QAction *>("UpAction");
  upAction->setEnabled(!m_searchResults.isEmpty());
  
  //
  // Get the clear action and disable it
  // 
  QAction *clearAction = findChild<QAction *>("ClearAction");
  clearAction->setEnabled(!m_searchResults.isEmpty());
  
  //
  // Emit the clearSearchResults() signal
  // 
  emit clearSearchResults();
}


void Smb4KNetworkSearchToolBar::setSearchResults(const QList<SharePtr>& list)
{
  //
  // Set the new list
  // 
  for (const SharePtr &share : list)
  {
    m_searchResults << share->url().toString();
  }
  
  //
  // Sort the search results
  // 
  m_searchResults.sort();
  
  //
  // Set the iterator
  // 
  m_iterator = m_searchResults;
  
  //
  // Get the down action and enable it
  // 
  QAction *downAction = findChild<QAction *>("DownAction");
  downAction->setEnabled(!m_searchResults.isEmpty());
  
  //
  // Get the up action and enable it
  // 
  QAction *upAction = findChild<QAction *>("UpAction");
  upAction->setEnabled(!m_searchResults.isEmpty());
  
  //
  // Get the clear action and enable it
  // 
  QAction *clearAction = findChild<QAction *>("ClearAction");
  clearAction->setEnabled(!m_searchResults.isEmpty());
}


void Smb4KNetworkSearchToolBar::setCompletionStrings(const QStringList& strings)
{
  //
  // Get the input combo box 
  //
  KComboBox *comboBox = findChild<KComboBox *>("SearchCombo");
  
  //
  // Set the completion strings
  // 
  comboBox->completionObject()->setItems(strings);
}


QStringList Smb4KNetworkSearchToolBar::completionStrings() const
{
  //
  // Get the input combo box 
  //
  KComboBox *comboBox = findChild<KComboBox *>("SearchCombo");
  
  //
  // Return the completion strings
  // 
  return comboBox->completionObject()->items();
}


void Smb4KNetworkSearchToolBar::slotReturnKeyPressed()
{
  //
  // Get the input combo box 
  //
  KComboBox *comboBox = findChild<KComboBox *>("SearchCombo");
  
  //
  // Initialize a search if the line edit is not empty
  // 
  if (!comboBox->currentText().isEmpty())
  {
    //
    // Add the search item to the completion object
    // 
    comboBox->completionObject()->addItem(comboBox->currentText());
    
    //
    // Emit the search signal
    // 
    emit search(comboBox->currentText());
  }
}


void Smb4KNetworkSearchToolBar::slotSearchActionTriggered()
{
  //
  // Get the search dual action
  // 
  KDualAction *searchAction = findChild<KDualAction *>("SearchAction");
  
  //
  // Initialize a search if the action is in active state and abort 
  // the search if it is in inactive state
  // 
  if (!searchAction->isActive())
  {
    KComboBox *comboBox = findChild<KComboBox *>("SearchCombo");
    
    if (!comboBox->currentText().isEmpty())
    {
      //
      // Add the search item to the completion object
      // 
      comboBox->completionObject()->addItem(comboBox->currentText());
    
      //
      // Emit the search signal
      // 
      emit search(comboBox->currentText());
    }
  }
  else
  {
    //
    // Emit the abort signal
    // 
    emit abort();
  }
}


void Smb4KNetworkSearchToolBar::slotCloseButtonPressed()
{
  //
  // Clear the search toolbar
  // 
  clearSearch();
  
  //
  // Emit the abort signal
  // 
  emit abort();
  
  //
  // Emit the close signal
  // 
  emit closeSearchBar();
}


void Smb4KNetworkSearchToolBar::slotDownActionTriggered()
{
  if (m_iterator.hasNext())
  {
    //
    // Get the URL of the item
    // 
    QString url = m_iterator.next();
    
    //
    // Emit the jumpToResult() signal
    //
    emit jumpToResult(url);
  }
}


void Smb4KNetworkSearchToolBar::slotUpActionTriggered()
{
  if (m_iterator.hasPrevious())
  {
    //
    // Get the URL of the item
    // 
    QString url = m_iterator.previous();
    
    //
    // Emit the jumpToResult() signal
    //
    emit jumpToResult(url);
  }
}


void Smb4KNetworkSearchToolBar::slotClearSearch()
{
  clearSearch();
}





