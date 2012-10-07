/***************************************************************************
    smb4kbookmarkhandler_p  -  Private classes for the bookmark handler
                             -------------------
    begin                : Sun Mar 20 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kbookmarkhandler_p.h"
#include "smb4ksettings.h"
#include "smb4kbookmark.h"

// Qt includes
#include <QtCore/QEvent>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QHeaderView>
#include <QtGui/QDropEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>

// KDE includes
#include <klocale.h>
#include <kmenu.h>
#include <kinputdialog.h>


Smb4KBookmarkDialog::Smb4KBookmarkDialog( const QList<Smb4KBookmark *> &bookmarks, const QStringList &groups, QWidget *parent )
: KDialog( parent )
{
  setCaption( i18n( "Add Bookmarks" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );

  setupView();
  loadLists( bookmarks, groups );

  KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkDialog" );
  restoreDialogSize( group );
  m_label_edit->completionObject()->setItems( group.readEntry( "LabelCompletion", QStringList() ) );
  m_group_combo->completionObject()->setItems( group.readEntry( "GroupCompletion", m_groups ) );

  connect( this, SIGNAL(buttonClicked(KDialog::ButtonCode)), SLOT(slotUserClickedButton(KDialog::ButtonCode)) );
  connect( KGlobalSettings::self(), SIGNAL(iconChanged(int)), SLOT(slotIconSizeChanged(int)) );
}


Smb4KBookmarkDialog::~Smb4KBookmarkDialog()
{
  while ( !m_bookmarks.isEmpty() )
  {
    delete m_bookmarks.takeFirst();
  }
}


void Smb4KBookmarkDialog::setupView()
{
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QVBoxLayout *layout = new QVBoxLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QWidget *description = new QWidget( main_widget );

  QHBoxLayout *desc_layout = new QHBoxLayout( description );
  desc_layout->setSpacing( 5 );
  desc_layout->setMargin( 0 );

  QLabel *pixmap = new QLabel( description );
  QPixmap sync_pix = KIcon( "bookmark-new" ).pixmap( KIconLoader::SizeHuge );
  pixmap->setPixmap( sync_pix );
  pixmap->setAlignment( Qt::AlignBottom );

  QLabel *label = new QLabel( i18n( "All listed shares will be bookmarked. To edit the label "
                                    "or group, click the respective bookmark entry." ), description );
  label->setWordWrap( true );
  label->setAlignment( Qt::AlignBottom );

  desc_layout->addWidget( pixmap, 0 );
  desc_layout->addWidget( label, Qt::AlignBottom );

  m_widget = new KListWidget( main_widget );
  m_widget->setSortingEnabled( true );
  m_widget->setSelectionMode( QAbstractItemView::SingleSelection );
  int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
  m_widget->setIconSize( QSize( icon_size, icon_size ) );

  m_editors = new QWidget( main_widget );
  m_editors->setEnabled( false );

  QGridLayout *editors_layout = new QGridLayout( m_editors );
  editors_layout->setSpacing( 5 );
  editors_layout->setMargin( 0 );

  QLabel *l_label = new QLabel( i18n( "Label:" ), m_editors );
  m_label_edit = new KLineEdit( m_editors );
  m_label_edit->setClearButtonShown( true );

  QLabel *g_label = new QLabel( i18n( "Group:" ), m_editors );
  m_group_combo = new KComboBox( true, m_editors );

  editors_layout->addWidget( l_label, 0, 0, 0 );
  editors_layout->addWidget( m_label_edit, 0, 1, 0 );
  editors_layout->addWidget( g_label, 1, 0, 0 );
  editors_layout->addWidget( m_group_combo, 1, 1, 0 );

  layout->addWidget( description, 0 );
  layout->addWidget( m_widget, 0 );
  layout->addWidget( m_editors, 0 );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  connect( m_widget, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(slotBookmarkClicked(QListWidgetItem*)) );
  connect( m_label_edit, SIGNAL(editingFinished()), SLOT(slotLabelEdited()) );
  connect( m_group_combo->lineEdit(), SIGNAL(editingFinished()), SLOT(slotGroupEdited()) );
}


void Smb4KBookmarkDialog::loadLists( const QList<Smb4KBookmark *> &bookmarks, const QStringList &groups )
{
  // Copy the bookmarks to the internal list and add them to 
  // the list widget afterwards.
  for ( int i = 0; i < bookmarks.size(); ++i )
  {
    Smb4KBookmark *bookmark = new Smb4KBookmark( *bookmarks[i] );
    QListWidgetItem *item = new QListWidgetItem( bookmark->icon(), bookmark->unc(), m_widget );
    item->setData( Qt::UserRole, static_cast<QUrl>( bookmark->url() ) );
    
    m_bookmarks << bookmark;
  }

  m_groups = groups;
  m_group_combo->addItems( m_groups );
}


Smb4KBookmark *Smb4KBookmarkDialog::findBookmark( const KUrl &url )
{
  Smb4KBookmark *bookmark = NULL;
  
  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    if ( m_bookmarks.at( i )->url() == url )
    {
      bookmark = m_bookmarks[i];
      break;
    }
    else
    {
      continue;
    }
  }

  return bookmark;
}


void Smb4KBookmarkDialog::slotBookmarkClicked( QListWidgetItem *bookmark_item )
{
  if ( bookmark_item )
  {
    // Enable the editor widgets if necessary
    if ( !m_editors->isEnabled() )
    {
      m_editors->setEnabled( true );
    }
    else
    {
      // Do nothing
    }

    KUrl url = bookmark_item->data( Qt::UserRole ).toUrl();
    
    Smb4KBookmark *bookmark = findBookmark( url );

    if ( bookmark )
    {
      m_label_edit->setText( bookmark->label() );
      m_group_combo->setCurrentItem( bookmark->group() );
    }
    else
    {
      m_label_edit->clear();
      m_group_combo->clearEditText();
      m_editors->setEnabled( false );
    }
  }
  else
  {
    m_label_edit->clear();
    m_group_combo->clearEditText();
    m_editors->setEnabled( false );
  }
}


void Smb4KBookmarkDialog::slotLabelEdited()
{
  // Set the label
  KUrl url = m_widget->currentItem()->data( Qt::UserRole ).toUrl();

  Smb4KBookmark *bookmark = findBookmark( url );

  if ( bookmark )
  {
    bookmark->setLabel( m_label_edit->userText() );
  }
  else
  {
    // Do nothing
  }

  // Add label to completion object
  KCompletion *completion = m_label_edit->completionObject();

  if ( !m_label_edit->userText().isEmpty() )
  {
    completion->addItem( m_label_edit->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkDialog::slotGroupEdited()
{
  // Set the group
  KUrl url = m_widget->currentItem()->data( Qt::UserRole ).toUrl();

  Smb4KBookmark *bookmark = findBookmark( url );

  if ( bookmark )
  {
    bookmark->setGroup( m_group_combo->currentText() );
  }
  else
  {
    // Do nothing
  }

  // Add the group name to the combo box
  if ( m_group_combo->findText( m_group_combo->currentText() ) == -1 )
  {
    m_group_combo->addItem( m_group_combo->currentText() );
  }
  else
  {
    // Do nothing
  }

  // Add group to completion object
  KCompletion *completion = m_group_combo->completionObject();

  if ( !m_group_combo->currentText().isEmpty() )
  {
    completion->addItem( m_group_combo->currentText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkDialog::slotUserClickedButton( KDialog::ButtonCode code )
{
  switch ( code )
  {
    case KDialog::Ok:
    {
      KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkDialog" );
      saveDialogSize( group, KConfigGroup::Normal );
      group.writeEntry( "LabelCompletion", m_label_edit->completionObject()->items() );
      group.writeEntry( "GroupCompletion", m_group_combo->completionObject()->items() );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KBookmarkDialog::slotIconSizeChanged( int group )
{
  switch ( group )
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
      m_widget->setIconSize( QSize( icon_size, icon_size ) );
      break;
    }
    default:
    {
      break;
    }
  }
}



Smb4KBookmarkEditor::Smb4KBookmarkEditor( const QList<Smb4KBookmark *> &bookmarks, QWidget *parent )
: KDialog( parent )
{
  setCaption( i18n( "Edit Bookmarks" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );

  setupView();
  loadBookmarks( bookmarks );

  setMinimumWidth( sizeHint().height() > sizeHint().width() ? sizeHint().height() : sizeHint().width() );

  KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkEditor" );
  restoreDialogSize( group );
  m_label_edit->completionObject()->setItems( group.readEntry( "LabelCompletion", QStringList() ) );
  m_login_edit->completionObject()->setItems( group.readEntry( "LoginCompletion", QStringList() ) );
  m_ip_edit->completionObject()->setItems( group.readEntry( "IPCompletion", QStringList() ) );
  m_group_combo->completionObject()->setItems( group.readEntry( "GroupCompletion", m_groups ) );

  connect( this, SIGNAL(buttonClicked(KDialog::ButtonCode)), SLOT(slotUserClickedButton(KDialog::ButtonCode)) );
  connect( KGlobalSettings::self(), SIGNAL(iconChanged(int)), SLOT(slotIconSizeChanged(int)) );
}


Smb4KBookmarkEditor::~Smb4KBookmarkEditor()
{
  while ( !m_bookmarks.isEmpty() )
  {
    delete m_bookmarks.takeFirst();
  }
}


QList<Smb4KBookmark *> Smb4KBookmarkEditor::editedBookmarks() const
{
  return m_bookmarks;
}


bool Smb4KBookmarkEditor::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj == m_tree_widget->viewport() )
  {
    switch ( e->type() )
    {
      case QEvent::DragEnter:
      {
        // Only accept the drag enter event if it originates from
        // the tree widget's viewport.
        // NOTE: Do not remove the drag item from the parent (neither
        // widget nor top level item). You will crash the application
        // otherwise.
        QDragEnterEvent *ev = static_cast<QDragEnterEvent *>( e );
        QPoint pos = m_tree_widget->viewport()->mapToParent( ev->pos() );

        if ( (m_drag_item = m_tree_widget->itemAt( pos )) == NULL )
        {
          ev->ignore();
        }
        else
        {
          ev->accept();
        }
        break;
      }
      case QEvent::DragLeave:
      {
        // Do not allow a bookmark to be dragged outside the tree widget.
        QDragLeaveEvent *ev = static_cast<QDragLeaveEvent *>( e );
        ev->ignore();
        break;
      }
      case QEvent::Drop:
      {
        // Get the bookmark for the dropped item.
        Smb4KBookmark *bookmark = findBookmark( m_drag_item->data( 0, QTreeWidgetItem::UserType ).toUrl() );
        
        // Get the item where the dragged item has been dropped.
        QDropEvent *ev = static_cast<QDropEvent *>( e );
        QPoint pos = m_tree_widget->viewport()->mapToParent( ev->pos() );
        QTreeWidgetItem *neighbor = m_tree_widget->itemAt( pos );
        
        if ( neighbor )
        {
          QTreeWidgetItem *parent = NULL;
          QString group_name;
          
          if ( m_tree_widget->indexOfTopLevelItem( neighbor ) != -1 )
          {
            // This is the group item.
            parent = neighbor;
            neighbor->addChild( m_drag_item );
            group_name = neighbor->text( 0 );
          }
          else
          {
            // This is a child.
            parent = neighbor->parent();
            neighbor->parent()->addChild( m_drag_item );
            group_name = neighbor->parent()->text( 0 );
          }

          if ( bookmark )
          {
            bookmark->setGroup( group_name );
          }
          else
          {
            // Do nothing
          }

          parent->sortChildren( (m_tree_widget->columnCount() - 1), Qt::AscendingOrder );
        }
        else
        {
          // The item has been dragged to the top level. Add it as top level
          // item.
          m_tree_widget->addTopLevelItem( m_drag_item );

          if ( bookmark )
          {
            bookmark->setGroup( QString() );
          }
          else
          {
            // Do nothing
          }

          m_tree_widget->sortItems( (m_tree_widget->columnCount() - 1), Qt::AscendingOrder );
        }
        m_drag_item = NULL;
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  return KDialog::eventFilter( obj, e );
}


void Smb4KBookmarkEditor::setupView()
{
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QVBoxLayout *layout = new QVBoxLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  m_tree_widget = new QTreeWidget( main_widget );
  m_tree_widget->setColumnCount( 2 );
  m_tree_widget->hideColumn( (m_tree_widget->columnCount() - 1) ); // for sorting purposes
  m_tree_widget->headerItem()->setHidden( true );
  m_tree_widget->setRootIsDecorated( true );
  m_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );
  m_tree_widget->setContextMenuPolicy( Qt::CustomContextMenu );
  m_tree_widget->header()->setResizeMode( QHeaderView::ResizeToContents );
  m_tree_widget->setDragDropMode( QTreeWidget::InternalMove );
  int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
  m_tree_widget->setIconSize( QSize( icon_size, icon_size ) );
  m_tree_widget->viewport()->installEventFilter( this );

  m_add_group = new KAction( KIcon( "folder-bookmark" ), i18n( "Add Group" ), m_tree_widget );
  m_delete = new KAction( KIcon( "edit-delete" ), i18n( "Remove" ), m_tree_widget );
  m_clear = new KAction( KIcon( "edit-clear" ), i18n( "Clear Bookmarks" ), m_tree_widget );
  
  m_menu = new KActionMenu( m_tree_widget );
  m_menu->addAction( m_add_group );
  m_menu->addAction( m_delete );
  m_menu->addAction( m_clear );

  m_editors = new QWidget( main_widget );
  m_editors->setEnabled( false );

  QGridLayout *editors_layout = new QGridLayout( m_editors );
  editors_layout->setSpacing( 5 );
  editors_layout->setMargin( 0 );

  QLabel *l_label = new QLabel( i18n( "Label:" ), m_editors );
  m_label_edit = new KLineEdit( m_editors );
  m_label_edit->setClearButtonShown( true );

  QLabel *lg_label = new QLabel( i18n( "Login:" ), m_editors );
  m_login_edit = new KLineEdit( m_editors );
  m_login_edit->setClearButtonShown( true );

  QLabel *i_label = new QLabel( i18n( "IP Address:" ), m_editors );
  m_ip_edit = new KLineEdit( m_editors );
  m_ip_edit->setClearButtonShown( true );
  
  QLabel *g_label = new QLabel( i18n( "Group:" ), m_editors );
  m_group_combo = new KComboBox( true, m_editors );
  m_group_combo->setDuplicatesEnabled( false );

  editors_layout->addWidget( l_label, 0, 0, 0 );
  editors_layout->addWidget( m_label_edit, 0, 1, 0 );
  editors_layout->addWidget( lg_label, 1, 0, 0 );
  editors_layout->addWidget( m_login_edit, 1, 1, 0 );
  editors_layout->addWidget( i_label, 2, 0, 0 );
  editors_layout->addWidget( m_ip_edit, 2, 1, 0 );
  editors_layout->addWidget( g_label, 3, 0, 0 );
  editors_layout->addWidget( m_group_combo, 3, 1, 0 );

  layout->addWidget( m_tree_widget );
  layout->addWidget( m_editors );

  connect( m_tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(slotItemClicked(QTreeWidgetItem*,int)) );
  connect( m_tree_widget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenuRequested(QPoint)) );
  connect( m_label_edit, SIGNAL(editingFinished()), SLOT(slotLabelEdited()) );
  connect( m_ip_edit, SIGNAL(editingFinished()), SLOT(slotIPEdited()) );
  connect( m_login_edit, SIGNAL(editingFinished()), SLOT(slotLoginEdited()) );
  connect( m_group_combo->lineEdit(), SIGNAL(editingFinished()), SLOT(slotGroupEdited()) );
  connect( m_add_group, SIGNAL(triggered(bool)), SLOT(slotAddGroupTriggered(bool)) );
  connect( m_delete, SIGNAL(triggered(bool)), SLOT(slotDeleteTriggered(bool)) );
  connect( m_clear, SIGNAL(triggered(bool)), SLOT(slotClearTriggered(bool)) );
}


void Smb4KBookmarkEditor::loadBookmarks( const QList<Smb4KBookmark *> &list )
{
  for ( int i = 0; i < list.size(); ++i )
  {
    m_bookmarks << new Smb4KBookmark( *list[i] );
  }
  
  // Load the bookmarks into the tree view and compile 
  // the available groups in a list.
  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    if ( !m_bookmarks.at( i )->group().isEmpty() )
    {
      // Find the group and add the bookmark
      QList<QTreeWidgetItem *> items = m_tree_widget->findItems( m_bookmarks.at( i )->group(), Qt::MatchFixedString|Qt::MatchCaseSensitive, 0 );
      QTreeWidgetItem *group = NULL;

      if ( !items.isEmpty() )
      {
        group = items.first();
      }
      else
      {
        group = new QTreeWidgetItem( QTreeWidgetItem::UserType );
        group->setIcon( 0, KIcon( "folder-favorites" ) );
        group->setText( 0, m_bookmarks.at( i )->group() );
        group->setText( (m_tree_widget->columnCount() - 1), QString( "00_%1" ).arg( m_bookmarks.at( i )->group() ) );
        group->setFlags( Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDropEnabled ) ;
        m_tree_widget->addTopLevelItem( group );
      }

      QTreeWidgetItem *bookmark = new QTreeWidgetItem( QTreeWidgetItem::UserType );
      bookmark->setData( 0, QTreeWidgetItem::UserType, static_cast<QUrl>( m_bookmarks.at( i )->url() ) );
      bookmark->setIcon( 0, m_bookmarks.at( i )->icon() );
      bookmark->setText( 0, m_bookmarks.at( i )->unc() );
      bookmark->setText( (m_tree_widget->columnCount() - 1), QString( "01_%1" ).arg( m_bookmarks.at( i )->unc() ) );
      bookmark->setFlags( Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled ) ;
      group->addChild( bookmark );

      group->setExpanded( true );
    }
    else
    {
      QTreeWidgetItem *bookmark = new QTreeWidgetItem( QTreeWidgetItem::UserType );
      bookmark->setData( 0, QTreeWidgetItem::UserType, static_cast<QUrl>( m_bookmarks.at( i )->url() ) );
      bookmark->setIcon( 0, m_bookmarks.at( i )->icon() );
      bookmark->setText( 0, m_bookmarks.at( i )->unc() );
      bookmark->setText( (m_tree_widget->columnCount() - 1), QString( "01_%1" ).arg( m_bookmarks.at( i )->unc() ) );
      bookmark->setFlags( Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled ) ;
      m_tree_widget->addTopLevelItem( bookmark );
    }

    // Add the group to the list
    if ( !m_groups.contains( m_bookmarks.at( i )->group() ) )
    {
      m_groups << m_bookmarks.at( i )->group();
    }
    else
    {
      // Do nothing
    }
  }

  // Sort
  for ( int i = 0; i < m_tree_widget->topLevelItemCount(); ++i )
  {
    m_tree_widget->topLevelItem( i )->sortChildren( (m_tree_widget->columnCount() - 1), Qt::AscendingOrder );
  }
  
  m_tree_widget->sortItems( (m_tree_widget->columnCount() - 1), Qt::AscendingOrder );

  // Check that an empty goup entry is also present. If it is not there,
  // add it now and insert the groups to the group combo box afterwards.
  if ( !m_groups.contains( "" ) && !m_groups.contains( QString() ) )
  {
    m_groups << "";
  }
  else
  {
    // Do nothing
  }
  
  m_group_combo->addItems( m_groups );
  m_group_combo->setCurrentItem( "" );
}


Smb4KBookmark *Smb4KBookmarkEditor::findBookmark( const KUrl &url )
{
  Smb4KBookmark *bookmark = NULL;

  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    if ( m_bookmarks.at( i )->url() == url )
    {
      bookmark = m_bookmarks[i];
      break;
    }
    else
    {
      continue;
    }
  }

  return bookmark;
}


void Smb4KBookmarkEditor::slotItemClicked( QTreeWidgetItem *item, int /*col*/ )
{
  if ( item )
  {
    if ( m_tree_widget->indexOfTopLevelItem( item ) != -1 )
    {
      // This is a top-level item, i.e. it is either a bookmark without
      // group or a group entry.
      // Bookmarks have an URL stored, group folders not.
      if ( !item->data( 0, QTreeWidgetItem::UserType ).toUrl().isEmpty() )
      {
        Smb4KBookmark *bookmark = findBookmark( item->data( 0, QTreeWidgetItem::UserType ).toUrl() );

        if ( bookmark )
        {
          m_label_edit->setText( bookmark->label() );
          m_login_edit->setText( bookmark->login() );
          m_ip_edit->setText( bookmark->hostIP() );
          m_group_combo->setCurrentItem( bookmark->group() );
          m_editors->setEnabled( true );
        }
        else
        {
          m_label_edit->clear();
          m_login_edit->clear();
          m_ip_edit->clear();
          m_group_combo->clearEditText();
          m_editors->setEnabled( false );
        }
      }
      else
      {
        m_label_edit->clear();
        m_login_edit->clear();
        m_ip_edit->clear();
        m_group_combo->clearEditText();
        m_editors->setEnabled( false );
      }
    }
    else
    {
      // This can only be a bookmark.
      Smb4KBookmark *bookmark = findBookmark( item->data( 0, QTreeWidgetItem::UserType ).toUrl() );

      if ( bookmark )
      {
        m_label_edit->setText( bookmark->label() );
        m_login_edit->setText( bookmark->login() );
        m_ip_edit->setText( bookmark->hostIP() );
        m_group_combo->setCurrentItem( bookmark->group() );
        m_editors->setEnabled( true );
      }
      else
      {
        m_label_edit->clear();
        m_login_edit->clear();
        m_ip_edit->clear();
        m_group_combo->clearEditText();
        m_editors->setEnabled( false );
      }
    }
  }
  else
  {
    m_label_edit->clear();
    m_login_edit->clear();
    m_ip_edit->clear();
    m_group_combo->clearEditText();
    m_editors->setEnabled( false );
  }
}


void Smb4KBookmarkEditor::slotContextMenuRequested( const QPoint &pos )
{
  QTreeWidgetItem *item = m_tree_widget->itemAt( pos );
  m_delete->setEnabled( (item) );
  m_menu->menu()->popup( m_tree_widget->viewport()->mapToGlobal( pos ) );  
}


void Smb4KBookmarkEditor::slotLabelEdited()
{
  // Set the label
  KUrl url = m_tree_widget->currentItem()->data( 0, QTreeWidgetItem::UserType ).toUrl();

  Smb4KBookmark *bookmark = findBookmark( url );

  if ( bookmark )
  {
    bookmark->setLabel( m_label_edit->userText() );
  }
  else
  {
    // Do nothing
  }

  // Add label to completion object
  KCompletion *completion = m_label_edit->completionObject();

  if ( !m_label_edit->userText().isEmpty() )
  {
    completion->addItem( m_label_edit->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotLoginEdited()
{
  // Set the login
  KUrl url = m_tree_widget->currentItem()->data( 0, QTreeWidgetItem::UserType ).toUrl();

  Smb4KBookmark *bookmark = findBookmark( url );

  if ( bookmark )
  {
    bookmark->setLogin( m_login_edit->userText() );
  }
  else
  {
    // Do nothing
  }

  // Add login to completion object
  KCompletion *completion = m_login_edit->completionObject();

  if ( !m_login_edit->userText().isEmpty() )
  {
    completion->addItem( m_login_edit->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotIPEdited()
{
  // Set the ip address
  KUrl url = m_tree_widget->currentItem()->data( 0, QTreeWidgetItem::UserType ).toUrl();

  Smb4KBookmark *bookmark = findBookmark( url );

  if ( bookmark )
  {
    bookmark->setHostIP( m_ip_edit->userText() );
  }
  else
  {
    // Do nothing
  }

  // Add login to completion object
  KCompletion *completion = m_ip_edit->completionObject();

  if ( !m_ip_edit->userText().isEmpty() )
  {
    completion->addItem( m_ip_edit->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotGroupEdited()
{
  // Do not do anything if either there is no current item
  // or the current item is not selected.
  if ( !(m_tree_widget->currentItem() && m_tree_widget->currentItem()->isSelected()) )
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  // Get the URL of the current item. We need to do this
  // here, because after the following operation there is
  // no current item anymore.
  KUrl url = m_tree_widget->currentItem()->data( 0, QTreeWidgetItem::UserType ).toUrl();

  // Return here if the item is a group
  if ( url.isEmpty() )
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  Smb4KBookmark *bookmark = findBookmark( url );

  if ( bookmark )
  {
    bookmark->setGroup( m_group_combo->currentText() );
  }
  else
  {
    // Do nothing
  }
  
  // Move the current item to the group
  QList<QTreeWidgetItem *> items = m_tree_widget->findItems( m_group_combo->currentText(), Qt::MatchFixedString|Qt::MatchCaseSensitive, 0 );

  if ( !items.isEmpty() )
  {
    // There has to be only one entry in the items list, 
    // because we let findItems() only search the top level.
    // That is the group.

    // Check if the current item is a top level item or already
    // belongs to a group.
    int index = 0;
    
    if ( (index = m_tree_widget->indexOfTopLevelItem( m_tree_widget->currentItem() )) != -1 )
    {
      // The current item is a top level item, so remove
      // it from the top level...
      QTreeWidgetItem *bookmark_item = m_tree_widget->takeTopLevelItem( index );

      // ... and add it to the group
      items.at( 0 )->addChild( bookmark_item );
    }
    else
    {
      // The current item is already in a group. First check
      // that it is not the same as the one that is to be set.
      if ( m_tree_widget->currentItem()->parent() != items.first() )
      {
        index = m_tree_widget->currentItem()->parent()->indexOfChild( m_tree_widget->currentItem() );

        // Remove the item from the current group...
        QTreeWidgetItem *bookmark_item = m_tree_widget->currentItem()->parent()->takeChild( index );

        // ... and add it to the new one
        items.at( 0 )->addChild( bookmark_item );

        // Finally expand the group
        items.at( 0 )->setExpanded( true );
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // We do not create an empty group. In this case, the item is move 
    // to the top level instead.
    if ( !m_group_combo->currentText().trimmed().isEmpty() )
    {
      // Create a new group item and add it to the widget
      QTreeWidgetItem *group = new QTreeWidgetItem( QTreeWidgetItem::UserType );
      group->setIcon( 0, KIcon( "folder-bookmark" ) );
      group->setText( 0, m_group_combo->currentText() );
      group->setText( (m_tree_widget->columnCount() - 1), QString( "00_%1" ).arg( m_group_combo->currentText() ) );
      m_tree_widget->addTopLevelItem( group );
      m_tree_widget->sortItems( (m_tree_widget->columnCount() - 1), Qt::AscendingOrder );

      // Check if the current item is a top level item or already
      // belongs to a group.
      int index = 0;

      if ( (index = m_tree_widget->indexOfTopLevelItem( m_tree_widget->currentItem() )) != -1 )
      {
        // The current item is a top level item, so remove
        // it from the top level...
        QTreeWidgetItem *bookmark_item = m_tree_widget->takeTopLevelItem( index );

        // ... and add it to the group
        group->addChild( bookmark_item );
      }
      else
      {
        // The current item is already in a group. Get its index...
        index = m_tree_widget->currentItem()->parent()->indexOfChild( m_tree_widget->currentItem() );

        // ... remove it from the current group...
        QTreeWidgetItem *bookmark_item = m_tree_widget->currentItem()->parent()->takeChild( index );

        // ... and add it to the new one
        group->addChild( bookmark_item );

        // Finally expand the group
        group->setExpanded( true );
      }

      // Add the group to the combo box
      m_group_combo->addItem( m_group_combo->currentText() );
      m_group_combo->completionObject()->addItem( m_group_combo->currentText() );
    }
    else
    {
      // Check if the current item is a top level item or belongs to a group.
      int index = 0;

      if ( (index = m_tree_widget->indexOfTopLevelItem( m_tree_widget->currentItem() )) == -1 )
      {
        // The current item is already in a group. Get its index...
        index = m_tree_widget->currentItem()->parent()->indexOfChild( m_tree_widget->currentItem() );

        // ... remove it from the current group...
        QTreeWidgetItem *bookmark_item = m_tree_widget->currentItem()->parent()->takeChild( index );

        // ... and add it to the top level
        m_tree_widget->addTopLevelItem( bookmark_item );
      }
      else
      {
        // Do nothing
      }
    }
  }

  // Clear the editor widgets if necessary and disable them
  if ( !m_tree_widget->currentItem() ||
       m_tree_widget->currentItem()->data( 0, QTreeWidgetItem::UserType ).toUrl() != url )
  {
    m_label_edit->clear();
    m_login_edit->clear();
    m_ip_edit->clear();
    m_group_combo->clearEditText();
    m_editors->setEnabled( false );
  }
  else
  {
    // Do nothing
  }

  // Add the group name to the combo box
  if ( m_group_combo->findText( m_group_combo->currentText() ) == -1 )
  {
    m_group_combo->addItem( m_group_combo->currentText() );
  }
  else
  {
    // Do nothing
  }

  // Add group to completion object
  KCompletion *completion = m_group_combo->completionObject();

  if ( !m_group_combo->currentText().isEmpty() )
  {
    completion->addItem( m_group_combo->currentText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotAddGroupTriggered( bool /*checked*/ )
{
  bool ok = false;
  
  QString group_name = KInputDialog::getText( i18n( "Add Group" ), i18n( "Group name:" ),
                                              QString(), &ok, this );

  if ( ok && !group_name.isEmpty() &&
       m_tree_widget->findItems( group_name, Qt::MatchFixedString|Qt::MatchCaseSensitive, 0 ).isEmpty() )
  {
    // Create a new group item and add it to the widget
    QTreeWidgetItem *group = new QTreeWidgetItem( QTreeWidgetItem::UserType );
    group->setIcon( 0, KIcon( "folder-bookmark" ) );
    group->setText( 0, group_name );
    group->setText( (m_tree_widget->columnCount() - 1), QString( "00_%1" ).arg( group_name ) );
    group->setFlags( Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDropEnabled ) ;
    m_tree_widget->addTopLevelItem( group );
    m_tree_widget->sortItems( (m_tree_widget->columnCount() - 1), Qt::AscendingOrder );

    // Add the group to the combo box
    m_group_combo->addItem( group_name );
    m_group_combo->completionObject()->addItem( group_name );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotDeleteTriggered( bool /*checked*/ )
{
  QList<QTreeWidgetItem *> selected = m_tree_widget->selectedItems();

  while ( !selected.isEmpty() )
  {
    delete selected.takeFirst();
  }
}


void Smb4KBookmarkEditor::slotClearTriggered( bool /*checked*/ )
{
  // Clear the tree widget. Removing bookmarks is done when
  // the dialog is closed.
  m_tree_widget->clear();
}


void Smb4KBookmarkEditor::slotUserClickedButton( KDialog::ButtonCode code )
{
  switch ( code )
  {
    case KDialog::Ok:
    {
      // Remove obsolete bookmarks.
      // We can assume that each server in the network has a unique 
      // name, so we only need to test for the UNC and are done.
      QMutableListIterator<Smb4KBookmark *> it( m_bookmarks );

      while ( it.hasNext() )
      {
        Smb4KBookmark *bookmark = it.next();
        
        QList<QTreeWidgetItem *> items = m_tree_widget->findItems( bookmark->unc(), Qt::MatchFixedString|Qt::MatchCaseSensitive|Qt::MatchRecursive, 0 );

        if ( items.isEmpty() )
        {
          it.remove();
        }
        else
        {
          // Do nothing
        }
      }

      KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkEditor" );
      saveDialogSize( group, KConfigGroup::Normal );
      group.writeEntry( "LabelCompletion", m_label_edit->completionObject()->items() );
      group.writeEntry( "LoginCompletion", m_login_edit->completionObject()->items() );
      group.writeEntry( "IPCompletion", m_ip_edit->completionObject()->items() );
      group.writeEntry( "GroupCompletion", m_group_combo->completionObject()->items() );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KBookmarkEditor::slotIconSizeChanged( int group )
{
  switch ( group )
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
      m_tree_widget->setIconSize( QSize( icon_size, icon_size ) );
      break;
    }
    default:
    {
      break;
    }
  }
}


#include "smb4kbookmarkhandler_p.moc"

