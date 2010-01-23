/***************************************************************************
    smb4kbookmarkeditor  -  The bookmark editor of Smb4K
                             -------------------
    begin                : Di Okt 5 2004
    copyright            : (C) 2004-2009 by Alexander Reinholdt
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
#include <QTreeWidgetItem>
#include <QModelIndex>

// KDE includes
#include <klocale.h>
#include <kaction.h>
#include <kicon.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kactionmenu.h>
#include <kmenu.h>

// application specific includes
#include <smb4kbookmarkeditor.h>
#include <core/smb4kbookmark.h>
#include <core/smb4kcore.h>
#include <core/smb4kglobal.h>
#include <core/smb4ksettings.h>

using namespace Smb4KGlobal;


Smb4KBookmarkEditor::Smb4KBookmarkEditor( QWidget *parent )
: KDialog( parent )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Bookmark Editor" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  // Main widget
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  // Tree widget
  m_widget = new QTreeWidget( main_widget );
  m_widget->setColumnCount( 5 );
  m_widget->setSelectionMode( QTreeWidget::ExtendedSelection );
  m_widget->setRootIsDecorated( false );
  m_widget->setContextMenuPolicy( Qt::CustomContextMenu );
  m_widget->setEditTriggers( QTreeWidget::NoEditTriggers );

  // Set the headers for tree widget
  QStringList header_labels;
  header_labels.append( i18n( "Bookmark" ) );
  header_labels.append( i18n( "Workgroup" ) );
  header_labels.append( i18n( "Login" ) );
  header_labels.append( i18n( "IP Address" ) );
  header_labels.append( i18n( "Label" ) );
  m_widget->setHeaderLabels( header_labels );

  layout->addWidget( m_widget, 0, 0, 0 );

  m_collection = new KActionCollection( this, KGlobal::mainComponent() );

  // Edit action
  KAction *edit_action       = new KAction( KIcon( "edit-rename" ), i18n( "Edit" ),
                               m_collection );
  edit_action->setEnabled( false );
  connect( edit_action, SIGNAL( triggered( bool ) ), this, SLOT( slotEditActionTriggered( bool ) ) );

  // Delete action
  KAction *remove_action     = new KAction( KIcon( "edit-delete" ), i18n( "Remove" ),
                               m_collection );
  remove_action->setEnabled( false );
  connect( remove_action, SIGNAL( triggered( bool ) ), this, SLOT( slotRemoveActionTriggered( bool ) ) );
  
  // Clear action
  KAction *clear_action      = new KAction( KIcon( "edit-clear-list" ), i18n( "Clear List" ),
                               m_collection );
  clear_action->setEnabled( false );
  connect( clear_action, SIGNAL( triggered( bool ) ), this, SLOT( slotClearActionTriggered( bool ) ) );

  // Add action to collection
  m_collection->addAction( "edit_action", edit_action );
  m_collection->addAction( "remove_action", remove_action );
  m_collection->addAction( "clear_action", clear_action );

  slotLoadBookmarks();

  // Connections
  connect( m_widget,                     SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this,                         SLOT( slotContextMenuRequested( const QPoint & ) ) );

  connect( m_widget,                     SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
           this,                         SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );

  connect( this,                         SIGNAL( okClicked() ),
           this,                         SLOT( slotOkClicked() ) );

  connect( this,                         SIGNAL( cancelClicked() ),
           this,                         SLOT( slotCancelClicked() ) );

  connect( Smb4KCore::bookmarkHandler(), SIGNAL( updated() ),
           this,                         SLOT( slotLoadBookmarks() ) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkEditor" );
  restoreDialogSize( group );
}


Smb4KBookmarkEditor::~Smb4KBookmarkEditor()
{
  while ( !m_collection->actions().isEmpty() )
  {
    delete m_collection->actions().takeFirst();
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KBookmarkEditor::slotContextMenuRequested( const QPoint &pos )
{
  if ( m_widget->itemAt( pos ) )
  {
    if ( m_widget->indexAt( pos ).column() != Bookmark )
    {
      m_collection->action( "edit_action" )->setEnabled( true );
    }
    else
    {
      m_collection->action( "edit_action" )->setEnabled( false );
    }
  }
  else
  {
    m_collection->action( "edit_action" )->setEnabled( false );
  }
  
  m_collection->action( "remove_action" )->setEnabled( !m_widget->selectedItems().isEmpty() );
  m_collection->action( "clear_action" )->setEnabled( (m_widget->topLevelItemCount() != 0) );

  KActionMenu *menu = findChild<KActionMenu *>( "Smb4KBookmarkEditorMenu" );

  if ( !menu )
  {
    menu = new KActionMenu( this );
    menu->setObjectName( "Smb4KBookmarkEditorMenu" );
    menu->addAction( m_collection->action( "edit_action" ) );
    menu->addAction( m_collection->action( "remove_action" ) );
    menu->addAction( m_collection->action( "clear_action" ) );
  }
  else
  {
    // Do nothing.
  }

  menu->menu()->popup( m_widget->viewport()->mapToGlobal( pos ) );
}


void Smb4KBookmarkEditor::slotEditActionTriggered( bool /* checked */ )
{
  if ( m_widget->currentItem() && m_widget->currentIndex().column() != Bookmark )
  {
    m_widget->editItem( m_widget->currentItem(), m_widget->currentIndex().column() );
  }
  else
  {
    // Do nothing
  }

  // FIXME: Add resizing of the column that was edited.
  // Can we connect to some signal?
}


void Smb4KBookmarkEditor::slotRemoveActionTriggered( bool /* checked */ )
{
  // Remove the selected items.
  while ( !m_widget->selectedItems().isEmpty() )
  {
    delete m_widget->selectedItems().takeFirst();
  }

  // Adjust the columns.
  for ( int col = 0; col < m_widget->columnCount(); col++ )
  {
    m_widget->resizeColumnToContents( col );
  }
}


void Smb4KBookmarkEditor::slotClearActionTriggered( bool /* checked */ )
{
  // Clear the list of bookmarks.
  m_widget->clear();
  
  // Adjust the columns.
  for ( int col = 0; col < m_widget->columnCount(); col++ )
  {
    m_widget->resizeColumnToContents( col );
  }
}


void Smb4KBookmarkEditor::slotItemDoubleClicked( QTreeWidgetItem *item, int column )
{
  if ( item )
  {
    switch( column )
    {
      case Workgroup:
      case Login:
      case IPAddress:
      case Label:
      {
        m_widget->editItem( item, column );
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

  for ( int i = 0; i < m_widget->columnCount(); ++i )
  {
    m_widget->resizeColumnToContents( i );
  }
}


void Smb4KBookmarkEditor::slotOkClicked()
{
  m_collection->clear();

  QList<Smb4KBookmark *> bookmarks;

  for ( int i = 0; i < m_widget->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *item = m_widget->topLevelItem( i );

    if ( item )
    {
      Smb4KBookmark *bookmark = new Smb4KBookmark();
      bookmark->setUNC( item->text( Bookmark ).trimmed() );
      bookmark->setWorkgroupName( item->text( Workgroup ).trimmed() );
      bookmark->setLogin( item->text( Login ).trimmed() );
      bookmark->setHostIP( item->text( IPAddress ).trimmed() );
      bookmark->setLabel( item->text( Label ).trimmed() );

      bookmarks.append( bookmark );

      continue;
    }
    else
    {
      continue;
    }
  }

  Smb4KCore::bookmarkHandler()->writeBookmarkList( bookmarks );

  KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkEditor" );
  saveDialogSize( group, KConfigGroup::Normal );
}


void Smb4KBookmarkEditor::slotCancelClicked()
{
  m_collection->clear();
}


void Smb4KBookmarkEditor::slotLoadBookmarks()
{
  m_widget->clear();

  QList<Smb4KBookmark *> bookmarks = Smb4KCore::bookmarkHandler()->getBookmarks();

  for ( int i = 0; i < bookmarks.size(); ++i )
  {
    QStringList entries;
    QTreeWidgetItem *item = new QTreeWidgetItem( m_widget, QTreeWidgetItem::Type );
    item->setText( Bookmark, bookmarks.at( i )->unc() );
    item->setIcon( Bookmark, SmallIcon( "folder-remote" ) );
    item->setText( Workgroup, bookmarks.at( i )->workgroupName() );
    item->setText( Login, bookmarks.at( i )->login() );
    item->setText( IPAddress, bookmarks.at( i )->hostIP() );
    item->setText( Label, bookmarks.at( i )->label() );
    item->setFlags( Qt::ItemIsEditable | item->flags() );
  }

  // Adjust the columns:
  for ( int col = 0; col < m_widget->columnCount(); col++ )
  {
    m_widget->resizeColumnToContents( col );
  }
}

#include "smb4kbookmarkeditor.moc"
