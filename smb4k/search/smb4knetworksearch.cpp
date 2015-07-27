/***************************************************************************
    smb4knetworksearch  -  The network search widget of Smb4K.
                             -------------------
    begin                : Sa Jun 2 2007
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
#include "smb4knetworksearch.h"
#include "smb4knetworksearchitem.h"

// Qt includes
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>

// KDE includes
#include <KI18n/KLocalizedString>


Smb4KNetworkSearch::Smb4KNetworkSearch(QWidget *parent)
: QWidget(parent)
{
  setFocusPolicy(Qt::WheelFocus);
  
  QGridLayout *layout = new QGridLayout(this);
  layout->setSpacing(5);

  // Tool bar
  m_toolbar = new KToolBar(this);
  m_toolbar->setToolBarsLocked(true);
  m_toolbar->setToolBarsEditable(false);
  
  // Search combo box
  QLabel *search_item = new QLabel(i18n("Search item:"), m_toolbar);

  m_combo = new KComboBox(true, m_toolbar);
  m_combo->setToolTip(i18n("Enter the search string here."));
  m_combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_combo->setCompletionMode(KCompletion::CompletionPopupAuto);
  
  (void) m_toolbar->addWidget(search_item);
  (void) m_toolbar->addWidget(m_combo);

  // List view
  m_list_widget = new QListWidget(this);
  m_list_widget->setResizeMode(QListWidget::Adjust);
  m_list_widget->setWrapping(true);
  m_list_widget->setContextMenuPolicy(Qt::CustomContextMenu);

  layout->addWidget(m_toolbar, 0, 0, 0);
  layout->addWidget(m_list_widget, 1, 0, 0);
}


Smb4KNetworkSearch::~Smb4KNetworkSearch()
{
}

