/***************************************************************************
    smb4ksearchdialog  -  The search dialog widget of Smb4K.
                             -------------------
    begin                : Sa Jun 2 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QGridLayout>
#include <QLabel>

// KDE includes
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>

// application specific includes
#include <smb4ksearchdialog.h>
#include <smb4ksearchdialogitem.h>

Smb4KSearchDialog::Smb4KSearchDialog( QWidget *parent )
: QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setSpacing( 5 );

  // Search combo box
  QLabel *search_item = new QLabel( i18n( "Search item:" ), this );

  m_combo = new KComboBox( true, this );
  m_combo->setToolTip( i18n( "Enter the search string here." ) );
  m_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  m_combo->setCompletionMode( KGlobalSettings::CompletionPopupAuto );

  // List view
  m_list_widget = new KListWidget( this );
  m_list_widget->setResizeMode( KListWidget::Adjust );
  m_list_widget->setWrapping( true );
  m_list_widget->setContextMenuPolicy( Qt::CustomContextMenu );

  layout->addWidget( search_item, 0, 0, 0 );
  layout->addWidget( m_combo, 0, 1, 0 );
  layout->addWidget( m_list_widget, 1, 0, 1, 2, 0 );
}


Smb4KSearchDialog::~Smb4KSearchDialog()
{
}

#include "smb4ksearchdialog.moc"
