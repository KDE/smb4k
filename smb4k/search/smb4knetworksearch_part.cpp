/***************************************************************************
    smb4ksearchdialog_part  -  This Part encapsulates the search dialog
    of Smb4K.
                             -------------------
    begin                : Fr Jun 1 2007
    copyright            : (C) 2007-2011 by Alexander Reinholdt
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

// Qt includes
#include <QLineEdit>
#include <QKeySequence>

// KDE includes
#include <kaboutdata.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kaction.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <kapplication.h>

// application specific includes
#include <smb4knetworksearch_part.h>
#include <smb4knetworksearch.h>
#include <smb4knetworksearchitem.h>
#include <core/smb4kcore.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>
#include <core/smb4ksettings.h>
#include <core/smb4kglobal.h>
#include <core/smb4kscanner.h>
#include <core/smb4kmounter.h>
#include <core/smb4ksearch.h>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY( Smb4KNetworkSearchPartFactory, registerPlugin<Smb4KNetworkSearchPart>(); )
K_EXPORT_PLUGIN( Smb4KNetworkSearchPartFactory( "Smb4KNetworkSearchPart" ) );


Smb4KNetworkSearchPart::Smb4KNetworkSearchPart( QWidget *parentWidget, QObject *parent, const QList<QVariant> &args )
: KParts::Part( parent ), m_silent( false )
{
  // Parse arguments:
  for ( int i = 0; i < args.size(); ++i )
  {
    if ( args.at( i ).toString().startsWith( "silent" ) )
    {
      if ( QString::compare( args.at( i ).toString().section( "=", 1, 1 ).trimmed(), "\"true\"" ) == 0 )
      {
        m_silent = true;
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      continue;
    }
  }
      
  // Set the XML file:
  setXMLFile( "smb4knetworksearch_part.rc" );

  // Set the widget of this part:
  m_widget = new Smb4KNetworkSearch( parentWidget );
  
  int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
  m_widget->listWidget()->setIconSize( QSize( icon_size, icon_size ) );
  
  setWidget( m_widget );

  // Set up actions:
  setupActions();
  
  // Load the completion items.
  KConfigGroup group( Smb4KSettings::self()->config(), "SearchDialog" );
  m_widget->comboBox()->completionObject()->setItems( group.readEntry( "SearchItemCompletion", QStringList() ) );

  // Connections:
  connect( m_widget->comboBox(),   SIGNAL( returnPressed() ),
           this,                   SLOT( slotReturnPressed() ) );

  connect( m_widget->comboBox(),   SIGNAL( textChanged( const QString & ) ),
           this,                   SLOT( slotComboBoxTextChanged( const QString & ) ) );

  connect( m_widget->listWidget(), SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
           this,                   SLOT( slotItemDoubleClicked( QListWidgetItem * ) ) );

  connect( m_widget->listWidget(), SIGNAL( itemSelectionChanged() ),
           this,                   SLOT( slotItemSelectionChanged() ) );

  connect( m_widget->listWidget(), SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this,                   SLOT( slotContextMenuRequested( const QPoint & ) ) );

  connect( Smb4KScanner::self(),   SIGNAL( hostListChanged() ),
           this,                   SLOT( slotCheckItemIsKnown() ) );
           
  connect( Smb4KMounter::self(),   SIGNAL( mounted( Smb4KShare * ) ),
           this,                   SLOT( slotShareMounted( Smb4KShare * ) ) );
           
  connect( Smb4KMounter::self(),   SIGNAL( unmounted( Smb4KShare * ) ),
           this,                   SLOT( slotShareUnmounted( Smb4KShare * ) ) );

  connect( Smb4KSearch::self(),    SIGNAL( result( Smb4KBasicNetworkItem *, bool ) ),
           this,                   SLOT( slotReceivedSearchResult( Smb4KBasicNetworkItem *, bool ) ) );

  connect( Smb4KSearch::self(),    SIGNAL( aboutToStart( const QString & ) ),
           this,                   SLOT( slotSearchAboutToStart( const QString & ) ) );

  connect( Smb4KSearch::self(),    SIGNAL( finished( const QString & ) ),
           this,                   SLOT( slotSearchFinished( const QString & ) ) );
           
  connect( kapp,                   SIGNAL( aboutToQuit() ),
           this,                   SLOT( slotAboutToQuit() ) );
           
  connect( KGlobalSettings::self(), SIGNAL( iconChanged( int ) ),
           this,                    SLOT( slotIconSizeChanged( int ) ) );
}


Smb4KNetworkSearchPart::~Smb4KNetworkSearchPart()
{
}


void Smb4KNetworkSearchPart::setupActions()
{
  KAction *search_action = new KAction( KIcon( "system-search" ), i18n( "&Search" ),
                           actionCollection() );
  search_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_S ) );
  connect( search_action, SIGNAL( triggered( bool ) ), this, SLOT( slotSearchActionTriggered( bool ) ) );

  KAction *clear_action  = new KAction( KIcon( "edit-clear-history" ), i18n( "&Clear" ),
                           actionCollection() );
  // No shortcut.
  connect( clear_action, SIGNAL( triggered( bool ) ), this, SLOT( slotClearActionTriggered( bool ) ) );

  KAction *item_action    = new KAction( KIcon( "emblem-mounted" ), i18n( "Mount" ),
                           actionCollection() );
  item_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_D ) );
  connect( item_action, SIGNAL( triggered( bool ) ), this, SLOT( slotAddActionTriggered( bool ) ) );

  KAction *abort_action  = new KAction( KIcon( "process-stop" ), i18n( "Abort" ),
                           actionCollection() );
  abort_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_A ) );
  connect( abort_action, SIGNAL( triggered( bool ) ), this, SLOT( slotAbortActionTriggered( bool ) ) );

  actionCollection()->addAction( "search_action", search_action );
  actionCollection()->addAction( "abort_search_action", abort_action );
  actionCollection()->addAction( "clear_search_action", clear_action );
  actionCollection()->addAction( "item_action", item_action );

  // Disable all actions.
  search_action->setEnabled( false );
  clear_action->setEnabled( false );
  item_action->setEnabled( false );
  abort_action->setEnabled( false );

  // Put the actions in the context menu.
  m_menu = new KActionMenu( this );
  m_menu_title = m_menu->menu()->addTitle( KIcon( "system-search" ), i18n( "Search Results" ) );
  m_menu->addAction( abort_action );
  m_menu->addSeparator();
  m_menu->addAction( clear_action );
  m_menu->addAction( item_action );
}


KAboutData *Smb4KNetworkSearchPart::createAboutData()
{
  KAboutData *aboutData = new KAboutData( "smb4knetworksearchpart",
                          "smb4k",
                          ki18n( "Smb4KNetworkSearchPart" ),
                          "2.0",
                          ki18n( "The network search KPart of Smb4K" ),
                          KAboutData::License_GPL_V2,
                          ki18n( "\u00A9 2007-2010, Alexander Reinholdt" ),
                          KLocalizedString(),
                          "http://smb4k.berlios.de",
                          "smb4k-bugs@lists.berlios.de" );

  return aboutData;
}


void Smb4KNetworkSearchPart::customEvent( QEvent *e )
{
  switch ( e->type() )
  {
    case EVENT_LOAD_SETTINGS:
    {
      // The only changes may concern the marking of the shares.
      for ( int i = 0; i < m_widget->listWidget()->count(); ++i )
      {
        Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->item( i ) );
    
        switch ( item->type() )
        {
          case Smb4KNetworkSearchItem::Share:
          {
            // First unmark the share.
            item->setMounted( false );
            
            // Now either mark it again or leave it unmarked.
            QList<Smb4KShare *> list = findShareByUNC( item->shareItem()->unc() );
            
            for ( int j = 0; j < list.size(); ++j )
            {
              if ( list.at( j )->isMounted() )
              {
                slotShareMounted( list.at( j ) );
                
                if ( !list.at( j )->isForeign() )
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
      break;
    }
    case EVENT_SET_FOCUS:
    {
      m_widget->comboBox()->lineEdit()->setFocus();
      break;
    }
    default:
    {
      break;
    }
  }

  KParts::Part::customEvent( e );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS (Smb4KNetworkSearchPart)
/////////////////////////////////////////////////////////////////////////////

void Smb4KNetworkSearchPart::slotReturnPressed()
{
  if ( !m_widget->comboBox()->currentText().isEmpty() )
  {
    slotSearchActionTriggered( false );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchPart::slotSearchActionTriggered( bool /*checked*/ )
{
  // Start the search.
  // Note: The combo box will be disabled/enabled by slotScannerState().
  m_widget->listWidget()->clear();

  QString search_item = m_widget->comboBox()->currentText();

  if ( !search_item.isEmpty() )
  {
    Smb4KSearch::self()->search( m_widget->comboBox()->currentText() );
    
    KCompletion *completion = m_widget->comboBox()->completionObject();
    completion->addItem( search_item );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchPart::slotClearActionTriggered( bool /*checked*/ )
{
  // Clear the combo box and the list widget.
  m_widget->comboBox()->clear();
  m_widget->comboBox()->clearEditText();
  m_widget->listWidget()->clear();

  // Disable the actions.
  actionCollection()->action( "search_action" )->setEnabled( false );
  actionCollection()->action( "clear_search_action" )->setEnabled( false );
  actionCollection()->action( "item_action" )->setEnabled( false );
  actionCollection()->action( "abort_search_action" )->setEnabled( false );
}


void Smb4KNetworkSearchPart::slotAddActionTriggered( bool /*checked*/ )
{
  // Check if we need to add this host to the global list of hosts.
  Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->currentItem() );

  switch ( item->type() )
  {
    case Smb4KNetworkSearchItem::Share:
    {
      Smb4KMounter::self()->mountShare( item->shareItem() );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkSearchPart::slotAbortActionTriggered( bool /*checked*/ )
{
  QString search_item = m_widget->comboBox()->currentText();

  if ( !search_item.isEmpty() )
  {
    Smb4KSearch::self()->abort( m_widget->comboBox()->currentText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchPart::slotComboBoxTextChanged( const QString &text )
{
  actionCollection()->action( "search_action" )->setEnabled( !text.isEmpty() );
  actionCollection()->action( "clear_search_action" )->setEnabled( !text.isEmpty() );
}


void Smb4KNetworkSearchPart::slotItemDoubleClicked( QListWidgetItem *item )
{
  if ( item )
  {
    // If we have got an item, enable the "Add" button if the
    // item is regular. Otherwise disable the button.
    Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>( item );

    switch ( searchItem->type() )
    {
      case Smb4KNetworkSearchItem::Share:
      {
        Smb4KMounter::self()->mountShare( searchItem->shareItem() );
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


void Smb4KNetworkSearchPart::slotItemSelectionChanged()
{
  QList<QListWidgetItem *> list = m_widget->listWidget()->selectedItems();

  if ( !list.isEmpty() )
  {
    // We are in single selection mode, so there is only one
    // item in the list.
    if ( list.count() == 1 )
    {
      Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>( list.first() );

      switch ( item->type() )
      {
        case Smb4KNetworkSearchItem::Share:
        {
          actionCollection()->action( "item_action" )->setEnabled( true );

          break;
        }
        default:
        {
          actionCollection()->action( "item_action" )->setEnabled( false );
          break;
        }
      }
    }
    else
    {
      actionCollection()->action( "item_action" )->setEnabled( false );
    }
  }
  else
  {
    actionCollection()->action( "item_action" )->setEnabled( false );
  }
}


void Smb4KNetworkSearchPart::slotContextMenuRequested( const QPoint &pos )
{
  Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->itemAt( pos ) );

  m_menu->removeAction( m_menu_title );
  delete m_menu_title;

  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkSearchItem::Host:
      {
        m_menu_title = m_menu->menu()->addTitle( item->icon(),
                                                 item->hostItem()->hostName(),
                                                 actionCollection()->action( "abort_search_action" ) );
        break;
      }
      case Smb4KNetworkSearchItem::Share:
      {
        m_menu_title = m_menu->menu()->addTitle( item->icon(),
                                                 item->shareItem()->unc(),
                                                 actionCollection()->action( "abort_search_action" ) );
        break;
      }
      default:
      {
        m_menu_title = m_menu->menu()->addTitle( KIcon( "system-search" ),
                                                 i18n( "Search Results" ),
                                                 actionCollection()->action( "abort_search_action" ) );

        break;
      }
    }
  }
  else
  {
    m_menu_title = m_menu->menu()->addTitle( KIcon( "system-search" ),
                                             i18n( "Search Results" ),
                                             actionCollection()->action( "abort_search_action" ) );
  }

  m_menu->menu()->popup( m_widget->listWidget()->viewport()->mapToGlobal( pos ) );
}


void Smb4KNetworkSearchPart::slotReceivedSearchResult( Smb4KBasicNetworkItem *item, bool known )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KBasicNetworkItem::Host:
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( item );

        // Create a Smb4KNetworkSearchItem and add it to the first position.
        Smb4KNetworkSearchItem *item = new Smb4KNetworkSearchItem( m_widget->listWidget(), host );
        item->setKnown( known );

        m_widget->listWidget()->sortItems();

        // Enable the combo box and set the focus:
        m_widget->comboBox()->setEnabled( true );
        m_widget->comboBox()->setFocus();

        // Now select the text, so that the user can easily
        // remove it.
        m_widget->comboBox()->lineEdit()->selectAll();

        break;
      }
      case Smb4KBasicNetworkItem::Share:
      {
        Smb4KShare *share = static_cast<Smb4KShare *>( item );

        // Create a Smb4KNetworkSearchItem and add it to the first position.
        Smb4KNetworkSearchItem *item = new Smb4KNetworkSearchItem( m_widget->listWidget(), share );
        item->setMounted( known );

        m_widget->listWidget()->sortItems();

        // Enable the combo box and set the focus:
        m_widget->comboBox()->setEnabled( true );
        m_widget->comboBox()->setFocus();

        // Now select the text, so that the user can easily
        // remove it.
        m_widget->comboBox()->lineEdit()->selectAll();

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


void Smb4KNetworkSearchPart::slotSearchAboutToStart( const QString &/*string*/ )
{
//   if ( !m_silent )
//   {
//     emit setStatusBarText( i18n( "Searching for \"%1\"..." ).arg( string ) );
//   }
//   else
//   {
//     // Do nothing
//   }

  m_widget->comboBox()->setEnabled( false );
  actionCollection()->action( "abort_search_action" )->setEnabled( true );
  actionCollection()->action( "search_action" )->setEnabled( false );
  actionCollection()->action( "clear_search_action" )->setEnabled( false );
  // Add action will be disabled elsewhere.
}


void Smb4KNetworkSearchPart::slotSearchFinished( const QString &/*string*/ )
{
//   if ( !m_silent )
//   {
//     emit setStatusBarText( i18n( "Done." ) );
//   }
//   else
//   {
//     // Do nothing
//   }

  m_widget->comboBox()->setEnabled( true );
  actionCollection()->action( "abort_search_action" )->setEnabled( false );
  actionCollection()->action( "search_action" )->setEnabled( true );
  actionCollection()->action( "clear_search_action" )->setEnabled( !m_widget->comboBox()->currentText().isEmpty() );

  if ( m_widget->listWidget()->count() == 0 )
  {
    new Smb4KNetworkSearchItem( m_widget->listWidget() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchPart::slotCheckItemIsKnown()
{
  for ( int i = 0; i < m_widget->listWidget()->count(); ++i )
  {
    Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->item( i ) );

    switch ( item->type() )
    {
      case Smb4KNetworkSearchItem::Host:
      {
        Smb4KHost *host = findHost( item->hostItem()->hostName(), item->hostItem()->workgroupName() );

        if ( host )
        {
          item->setKnown( true );
        }
        else
        {
          item->setKnown( false );
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


void Smb4KNetworkSearchPart::slotShareMounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  for ( int i = 0; i < m_widget->listWidget()->count(); ++i )
  {
    Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->item( i ) );
    
    switch ( item->type() )
    {
      case Smb4KNetworkSearchItem::Share:
      {
        if ( QString::compare( item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive ) == 0 )
        {
          item->setMounted( share->isMounted() );
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


void Smb4KNetworkSearchPart::slotShareUnmounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  for ( int i = 0; i < m_widget->listWidget()->count(); ++i )
  {
    Smb4KNetworkSearchItem *item = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->item( i ) );
    
    switch ( item->type() )
    {
      case Smb4KNetworkSearchItem::Share:
      {
        if ( QString::compare( item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive ) == 0 )
        {
          item->setMounted( share->isMounted() );
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


void Smb4KNetworkSearchPart::slotAboutToQuit()
{
  KConfigGroup group( Smb4KSettings::self()->config(), "SearchDialog" );
  group.writeEntry( "SearchItemCompletion", m_widget->comboBox()->completionObject()->items() );
}


void Smb4KNetworkSearchPart::slotIconSizeChanged( int group )
{
  switch ( group )
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
      m_widget->listWidget()->setIconSize( QSize( icon_size, icon_size ) );
      break;
    }
    default:
    {
      break;
    }
  }
}


#include "smb4knetworksearch_part.moc"
