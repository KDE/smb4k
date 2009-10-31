/***************************************************************************
    smb4kbookmarkdialog  -  This class provides a dialog to add a bookmark.
                             -------------------
    begin                : Di Jan 13 2009
    copyright            : (C) 2009 by Alexander Reinholdt
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
#include <QTreeWidgetItem>
#include <QHeaderView>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <kconfiggroup.h>

// application specific includes
#include <smb4kbookmarkdialog.h>
#include <core/smb4kcore.h>
#include <core/smb4ksettings.h>


Smb4KBookmarkDialog::Smb4KBookmarkDialog( QWidget *parent )
: KDialog( parent )
{
  setCaption( i18n( "Add Bookmarks" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  setupView();

  KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkDialog" );
  restoreDialogSize( group );
}


Smb4KBookmarkDialog::~Smb4KBookmarkDialog()
{
}


void Smb4KBookmarkDialog::setupView()
{
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *label = new QLabel( i18n( "<qt>All checked shares will be bookmarked. To add a descriptive label to a bookmark, double click the respective field.</qt>" ), main_widget );
  label->setWordWrap( true );
  label->setScaledContents( true );

  m_widget = new QTreeWidget( main_widget );
  m_widget->setColumnCount( 3 );
  m_widget->setSelectionMode( QTreeWidget::ExtendedSelection );
  m_widget->setRootIsDecorated( false );
  m_widget->setContextMenuPolicy( Qt::CustomContextMenu );
  m_widget->setEditTriggers( QTreeWidget::NoEditTriggers );

  QStringList header_labels;
  header_labels << QString();
  header_labels << i18n( "Share" );
  header_labels << i18n( "Label" );
  m_widget->setHeaderLabels( header_labels );

  QSpacerItem *spacer = new QSpacerItem( 10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed );

  layout->addWidget( label, 0, 0, 0 );
  layout->addItem( spacer, 1, 0 );
  layout->addWidget( m_widget, 2, 0, 0 );

  m_widget->adjustSize();

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  connect( this,     SIGNAL( buttonClicked( KDialog::ButtonCode ) ),
           this,     SLOT( slotUserClickedButton( KDialog::ButtonCode ) ) );

  connect( m_widget, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
           this,     SLOT( slotItemClicked( QTreeWidgetItem *, int ) ) );

  connect( m_widget, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
           this,     SLOT( slotItemDoubleClicked( QTreeWidgetItem *, int ) ) );
}


void Smb4KBookmarkDialog::setBookmarks( const QList<Smb4KBookmark *> &list )
{
  while ( !m_bookmarks.isEmpty() )
  {
    delete m_bookmarks.takeFirst();
  }

  m_bookmarks += list;

  for ( int i = 0; i < m_bookmarks.size(); ++i )
  {
    QStringList entries;
    entries << QString();
    entries << m_bookmarks.at( i )->unc();
    entries << m_bookmarks.at( i )->label();

    QTreeWidgetItem *item = new QTreeWidgetItem( m_widget, entries );
    item->setIcon( 1, SmallIcon( "folder-remote" ) );
    item->setFlags( Qt::ItemIsEditable | item->flags() );
    item->setCheckState( 0, Qt::Checked );
    item->setDisabled( (Smb4KCore::bookmarkHandler()->findBookmarkByUNC( m_bookmarks.at( i )->unc() ) != NULL) );
  }

  // Adjust the columns.
  for ( int col = 0; col < m_widget->columnCount(); col++ )
  {
    m_widget->resizeColumnToContents( col );
  }
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KBookmarkDialog::slotUserClickedButton( ButtonCode button_code )
{
  switch ( button_code )
  {
    case Ok:
    {
      // Add the bookmarks.
      for ( int i = 0; i < m_widget->topLevelItemCount(); ++i )
      {
        if ( m_widget->topLevelItem( i )->checkState( 0 ) == Qt::Checked &&
             !m_widget->topLevelItem( i )->isDisabled() )
        {
          for ( int j = 0; j < m_bookmarks.size(); ++j )
          {
            if ( QString::compare( m_widget->topLevelItem( i )->text( 1 ), m_bookmarks.at( j )->unc() ) == 0 )
            {
              m_bookmarks.at( j )->setLabel( m_widget->topLevelItem( i )->text( 2 ) );
              Smb4KCore::bookmarkHandler()->addBookmark( m_bookmarks.at( j ), true );
              break;
            }
            else
            {
              continue;
            }
          }

          continue;
        }
        else
        {
          continue;
        }
      }

      KConfigGroup group( Smb4KSettings::self()->config(), "BookmarkDialog" );
      saveDialogSize( group, KConfigGroup::Normal );

      break;
    }
    default:
    {
      break;
    }
  }

  // Clear the tree widget.
  m_widget->clear();

  // Clear the internal bookmark list.
  while ( !m_bookmarks.isEmpty() )
  {
    delete m_bookmarks.takeFirst();
  }
}


void Smb4KBookmarkDialog::slotItemClicked( QTreeWidgetItem *item, int col )
{
  if ( item && !item->isDisabled() && col == 0 )
  {
    // The user clicked the check box. Check what we have to do
    // with the Ok button.
    if ( item->checkState( col ) == Qt::Checked )
    {
      enableButton( Ok, true );
    }
    else
    {
      if ( m_widget->topLevelItemCount() == 1 )
      {
        enableButton( Ok, false );
      }
      else
      {
        bool have_checked_item = false;

        for ( int i = 0; i < m_widget->topLevelItemCount(); ++i )
        {
          if ( m_widget->topLevelItem( i )->checkState( col ) == Qt::Checked )
          {
            have_checked_item = true;
            break;
          }
          else
          {
            continue;
          }
        }

        enableButton( Ok, have_checked_item );
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkDialog::slotItemDoubleClicked( QTreeWidgetItem *item, int col )
{
  if ( item && !item->isDisabled() && col == 2 )
  {
    m_widget->editItem( item, col );
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


#include "smb4kbookmarkdialog.moc"
