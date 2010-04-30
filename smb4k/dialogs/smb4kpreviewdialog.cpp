/***************************************************************************
    smb4kpreviewdialog.cpp  -  The preview dialog of Smb4K
                             -------------------
    begin                : Fre Jul 4 2003
    copyright            : (C) 2003-2008 by Alexander Reinholdt
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
#include <QAbstractItemView>
#include <QSizePolicy>

// KDE includes
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfiggroup.h>

#include <kmimetype.h>
#include <kurl.h>

// application specific includes
#include <smb4kpreviewdialog.h>
#include <core/smb4kpreviewitem.h>
#include <core/smb4ksettings.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>
#include <core/smb4kglobal.h>
#include <core/smb4khomesshareshandler.h>
#include <core/smb4kpreviewer.h>

using namespace Smb4KGlobal;


Smb4KPreviewDialog::Smb4KPreviewDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ) /* preview item will be initialized below */
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Preview" ) );
  setButtons( Close );
  setDefaultButton( Close );

  // Set the IP address if necessary.
  if ( share->hostIP().isEmpty() )
  {
    Smb4KHost *host = findHost( share->hostName(), share->workgroupName() );
    share->setHostIP( host->ip() );
  }
  else
  {
    // Do nothing
  }

  m_item = new Smb4KPreviewItem( share );
  m_button_id = None;
  m_current_index = 0;

  setupView();

  connect( this,                   SIGNAL( closeClicked() ),
           this,                   SLOT( slotCloseClicked() ) );

  connect( Smb4KPreviewer::self(), SIGNAL( result( Smb4KPreviewItem * ) ),
           this,                   SLOT( slotReceivedData( Smb4KPreviewItem * ) ) );

  connect( Smb4KPreviewer::self(), SIGNAL( aboutToStart( Smb4KPreviewItem * ) ),
           this,                   SLOT( slotAboutToStart( Smb4KPreviewItem * ) ) );

  connect( Smb4KPreviewer::self(), SIGNAL( finished( Smb4KPreviewItem * ) ),
           this,                   SLOT( slotFinished( Smb4KPreviewItem * ) ) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "PreviewDialog" );
  restoreDialogSize( group );
}


Smb4KPreviewDialog::~Smb4KPreviewDialog()
{
  delete m_item;
}


void Smb4KPreviewDialog::setupView()
{
  // Main widget
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  m_view = new KListWidget( main_widget );
  m_view->setResizeMode( KListWidget::Adjust );
  m_view->setWrapping( true );
  m_view->setSortingEnabled( true );
  m_view->setWhatsThis( i18n( "This is the view where the contents of the share is displayed." ) );
  m_view->setToolTip( i18n( "The preview" ) );

  m_toolbar = new KToolBar( main_widget, true, false );

  m_reload  = new KAction( KIcon( "view-refresh" ), i18n( "Reload" ), m_toolbar );
  m_abort   = new KAction( KIcon( "process-stop" ), i18n( "Abort" ), m_toolbar );
  m_back    = new KAction( KIcon( "go-previous" ), i18n( "Back" ), m_toolbar );
  m_forward = new KAction( KIcon( "go-next" ), i18n( "Forward" ), m_toolbar );
  m_up      = new KAction( KIcon( "go-up" ), i18n( "Up" ), m_toolbar );

  m_combo   = new KComboBox( false, m_toolbar );
  m_combo->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  m_combo->setDuplicatesEnabled( false );
  m_combo->setWhatsThis( i18n( "The current UNC address is shown here. You can also choose one of " 
    "the previously visited locations from the drop-down menu that will then be displayed in the "
    "view above." ) );
  m_combo->setToolTip( i18n( "The current UNC address" ) );

  m_toolbar->addAction( m_reload );
  m_toolbar->addAction( m_abort );
  m_toolbar->addAction( m_back );
  m_toolbar->addAction( m_forward );
  m_toolbar->addAction( m_up );
  m_toolbar->insertSeparator( m_toolbar->addWidget( m_combo ) );

  layout->addWidget( m_view, 0, 0, 0 );
  layout->addWidget( m_toolbar, 1, 0, 0 );

  connect( m_reload,  SIGNAL( triggered( bool ) ),
           this,      SLOT( slotReloadActionTriggered( bool ) ) );

  connect( m_back,    SIGNAL( triggered( bool ) ),
           this,      SLOT( slotBackActionTriggered( bool ) ) );

  connect( m_forward, SIGNAL( triggered( bool ) ),
           this,      SLOT( slotForwardActionTriggered( bool ) ) );

  connect( m_up,      SIGNAL( triggered( bool ) ),
           this,      SLOT( slotUpActionTriggered( bool ) ) );

  connect( m_combo,   SIGNAL( activated( const QString & ) ),
           this,      SLOT( slotItemActivated( const QString & ) ) );

  connect( m_view,    SIGNAL( executed( QListWidgetItem * ) ),
           this,      SLOT( slotItemExecuted( QListWidgetItem * ) ) );
}


void Smb4KPreviewDialog::getPreview()
{
  Smb4KPreviewer::self()->preview( m_item );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPreviewDialog::slotReceivedData( Smb4KPreviewItem *item )
{
  if ( !m_item->share()->equals( item->share(), Smb4KShare::NetworkOnly ) )
  {
    return;
  }
  else
  {
    // Go ahead
  }

  // Clear the icon view:
  m_view->clear();

  // The item should be equal to m_item, so we can use either of those
  // pointers.

  // Process the data:
  if ( !item->contents().isEmpty() )
  {
    // Generate the history.
    switch ( m_button_id )
    {
      case Reload: /* Really? */
      case Abort:
      case Back:
      case Forward:
      {
        // Do not insert anything into the history if
        // one of these three buttons was clicked.

        break;
      }
      default:
      {
        m_history.append( item->location() );
        m_current_index = m_history.size() - 1; /* index of the last item */

        break;
      }
    }

    // Clear the combo box, put the new history there and set the
    // current item:
    m_combo->clear();
    m_combo->insertItems( 0, m_history );
    m_combo->setCurrentItem( m_history.at( m_current_index ) );

    // Now put the contents in the icon view:
    for ( int i = 0; i < item->contents().size(); ++i )
    {
      switch ( item->contents().at( i ).first )
      {
        case Smb4KPreviewItem::File:
        {
          KUrl url( item->contents().at( i ).second );
          QListWidgetItem *listItem = new QListWidgetItem( KIcon( KMimeType::iconNameForUrl( url, 0 ) ), item->contents().at( i ).second, m_view, File );
          listItem->setData( Qt::UserRole, item->contents().at( i ).second );

          break;
        }
        case Smb4KPreviewItem::Directory:
        {
          // We do not want to show the '.' and '..' directories.
          if ( QString::compare( item->contents().at( i ).second, "." ) != 0 &&
               QString::compare( item->contents().at( i ).second, ".." ) != 0 )
          {
            QListWidgetItem *listItem = new QListWidgetItem( KIcon( "folder" ), item->contents().at( i ).second, m_view, Directory );
            listItem->setData( Qt::UserRole, item->contents().at( i ).second );
          }
          else
          {
            // Do nothing
          }

          break;
        }
        case Smb4KPreviewItem::HiddenFile:
        {
          if ( Smb4KSettings::previewHiddenItems() &&
               QString::compare( item->contents().at( i ).second, "." ) != 0 &&
               QString::compare( item->contents().at( i ).second, ".." ) != 0 )
          {
            KUrl url( item->contents().at( i ).second );
            QListWidgetItem *listItem = new QListWidgetItem( KIcon( KMimeType::iconNameForUrl( url, 0 ) ), item->contents().at( i ).second, m_view, File );
            listItem->setData( Qt::UserRole, item->contents().at( i ).second );
          }
          else
          {
            // Do nothing
          }

          break;
        }
        case Smb4KPreviewItem::HiddenDirectory:
        {
          if ( Smb4KSettings::previewHiddenItems() &&
               QString::compare( item->contents().at( i ).second, "." ) != 0 &&
               QString::compare( item->contents().at( i ).second, ".." ) != 0 )
          {
            QListWidgetItem *listItem = new QListWidgetItem( KIcon( "folder" ), item->contents().at( i ).second, m_view, Directory );
            listItem->setData( Qt::UserRole, item->contents().at( i ).second );
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

    // Now activate or deactivate the buttons:

    // Activate the 'Up' button if the current address is
    // not equal to the base address.
    m_up->setEnabled( !item->isRootDirectory() );

    // Activate/Deactivate 'Back' and 'Forward' buttons.
    m_back->setEnabled( m_current_index != 0 );
    m_forward->setEnabled( m_current_index != m_history.size() - 1 );
  }
  else
  {
    return;
  }
}


void Smb4KPreviewDialog::slotAboutToStart( Smb4KPreviewItem *item )
{
  if ( item && m_item->equals( item ) )
  {
    m_reload->setEnabled( false );
    m_abort->setEnabled( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotFinished( Smb4KPreviewItem *item )
{
  if ( item && m_item->equals( item ) )
  {
    m_reload->setEnabled( true );
    m_abort->setEnabled( false );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotItemExecuted( QListWidgetItem *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Directory:
      {
        m_button_id = None;

        if ( !Smb4KPreviewer::self()->isRunning() )
        {
          m_item->setPath( m_item->path()+item->data( Qt::UserRole ).toString() );
          Smb4KPreviewer::self()->preview( m_item );
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


void Smb4KPreviewDialog::slotReloadActionTriggered( bool /*checked*/ )
{
  m_button_id = Reload;
  m_item->clearContents();
  Smb4KPreviewer::self()->preview( m_item );
}


void Smb4KPreviewDialog::slotAbortActionTriggered( bool /*checked*/ )
{
  m_button_id = Abort;
  m_item->clearContents(); /* Is it a problem to clear the contents here? */
  Smb4KPreviewer::self()->abort( m_item );
}


void Smb4KPreviewDialog::slotBackActionTriggered( bool /*checked*/ )
{
  m_button_id = Back;
  m_item->clearContents();

  // Move one item back in the list:
  if ( m_current_index != 0 )
  {
    --m_current_index;
  }
  else
  {
    return;
  }

  // Set the path:
  QString path = m_history.at( m_current_index );

  if ( path.count( "/" ) == 3 )
  {
    m_item->setPath( "/" );
  }
  else
  {
    m_item->setPath( path.section( "/", 4, -1 ) );
  }

  Smb4KPreviewer::self()->preview( m_item );
}


void Smb4KPreviewDialog::slotForwardActionTriggered( bool /*checked*/ )
{
  m_button_id = Forward;
  m_item->clearContents();

  // Move one item forward in the list:
  if ( m_current_index != m_history.size() - 1 )
  {
    ++m_current_index;
  }
  else
  {
    return;
  }

  // Set the path:
  QString path = m_history.at( m_current_index );

  if ( path.count( "/" ) == 3 )
  {
    m_item->setPath( "/" );
  }
  else
  {
    m_item->setPath( path.section( "/", 4, -1 ) );
  }

  Smb4KPreviewer::self()->preview( m_item );
}


void Smb4KPreviewDialog::slotUpActionTriggered( bool /*checked*/ )
{
  m_button_id = Up;
  m_item->clearContents();

  if ( !m_item->isRootDirectory() )
  {
    if ( m_item->path().count( "/" ) > 1 )
    {
      m_item->setPath( m_item->path().section( "/", 0, -3 ).append( "/" ) );
    }
    else
    {
      m_item->setPath( "/" );
    }

    Smb4KPreviewer::self()->preview( m_item );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotItemActivated( const QString &item )
{
  m_button_id = Combo;

  // First we have to strip the address:
  m_item->setPath( item.section( m_item->share()->unc(), 1, 1 ).trimmed() );
  Smb4KPreviewer::self()->preview( m_item );
}


void Smb4KPreviewDialog::slotCloseClicked()
{
  KConfigGroup group( Smb4KSettings::self()->config(), "PreviewDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
}


#include "smb4kpreviewdialog.moc"

