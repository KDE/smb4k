/***************************************************************************
    smb4ksearchdialog_part  -  This Part encapsulates the search dialog
    of Smb4K.
                             -------------------
    begin                : Fr Jun 1 2007
    copyright            : (C) 2007-2012 by Alexander Reinholdt
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
#include "smb4knetworksearch_part.h"
#include "smb4knetworksearch.h"
#include "smb4knetworksearchitem.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"
#include "core/smb4kscanner.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksearch.h"

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
#include <kdualaction.h>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY( Smb4KNetworkSearchPartFactory, registerPlugin<Smb4KNetworkSearchPart>(); )
K_EXPORT_PLUGIN( Smb4KNetworkSearchPartFactory( "Smb4KNetworkSearchPart" ) );


Smb4KNetworkSearchPart::Smb4KNetworkSearchPart( QWidget *parentWidget, QObject *parent, const QList<QVariant> &args )
: KParts::Part( parent ), m_silent( false )
{
  // Parse arguments:
  for ( int i = 0; i < args.size(); ++i )
  {
    if ( args.at( i ).toString().startsWith( QLatin1String( "silent" ) ) )
    {
      if ( QString::compare( args.at( i ).toString().section( '=', 1, 1 ).trimmed(), "\"true\"" ) == 0 )
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
  connect( m_widget->comboBox(),   SIGNAL(returnPressed()),
           this,                   SLOT(slotReturnPressed()) );

  connect( m_widget->comboBox(),   SIGNAL(textChanged(QString)),
           this,                   SLOT(slotComboBoxTextChanged(QString)) );

  connect( m_widget->listWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           this,                   SLOT(slotItemDoubleClicked(QListWidgetItem*)) );

  connect( m_widget->listWidget(), SIGNAL(itemSelectionChanged()),
           this,                   SLOT(slotItemSelectionChanged()) );

  connect( m_widget->listWidget(), SIGNAL(customContextMenuRequested(QPoint)),
           this,                   SLOT(slotContextMenuRequested(QPoint)) );

  connect( Smb4KMounter::self(),   SIGNAL(mounted(Smb4KShare*)),
           this,                   SLOT(slotShareMounted(Smb4KShare*)) );
           
  connect( Smb4KMounter::self(),   SIGNAL(unmounted(Smb4KShare*)),
           this,                   SLOT(slotShareUnmounted(Smb4KShare*)) );

  connect( Smb4KSearch::self(),    SIGNAL(result(Smb4KShare*)),
           this,                   SLOT(slotReceivedSearchResult(Smb4KShare*)) );

  connect( Smb4KSearch::self(),    SIGNAL(aboutToStart(QString)),
           this,                   SLOT(slotSearchAboutToStart(QString)) );

  connect( Smb4KSearch::self(),    SIGNAL(finished(QString)),
           this,                   SLOT(slotSearchFinished(QString)) );
           
  connect( kapp,                   SIGNAL(aboutToQuit()),
           this,                   SLOT(slotAboutToQuit()) );
           
  connect( KGlobalSettings::self(), SIGNAL(iconChanged(int)),
           this,                    SLOT(slotIconSizeChanged(int)) );
}


Smb4KNetworkSearchPart::~Smb4KNetworkSearchPart()
{
}


void Smb4KNetworkSearchPart::setupActions()
{
  KDualAction *search_abort_action = new KDualAction( actionCollection() );
  KGuiItem search_item( i18n( "&Search" ), KIcon( "system-search" ) );
  KGuiItem abort_item( i18n( "Abort" ), KIcon( "process-stop" ) );
  search_abort_action->setActiveGuiItem( search_item );
  search_abort_action->setInactiveGuiItem( abort_item );
  search_abort_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_S ) );
  search_abort_action->setActive( true );
  search_abort_action->setAutoToggle( false );
  connect( search_abort_action, SIGNAL(triggered(bool)), this, SLOT(slotSearchAbortActionTriggered(bool)) );
  connect( search_abort_action, SIGNAL(activeChanged(bool)), this, SLOT(slotSearchAbortActionChanged(bool)) );
  
  KAction *clear_action  = new KAction( KIcon( "edit-clear-history" ), i18n( "&Clear" ),
                           actionCollection() );
  // No shortcut.
  connect( clear_action, SIGNAL(triggered(bool)), this, SLOT(slotClearActionTriggered(bool)) );

  KDualAction *mount_action = new KDualAction( actionCollection() );
  KGuiItem mount_item( i18n( "&Mount" ), KIcon( "emblem-mounted" ) );
  KGuiItem unmount_item( i18n( "&Unmount" ), KIcon( "emblem-unmounted" ) );
  mount_action->setActiveGuiItem( mount_item );
  mount_action->setInactiveGuiItem( unmount_item );
  mount_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_M ) );
  mount_action->setActive( true );
  mount_action->setAutoToggle( false );
  connect( mount_action, SIGNAL(triggered(bool)), this, SLOT(slotMountActionTriggered(bool)) );
  connect( mount_action, SIGNAL(activeChanged(bool)), this, SLOT(slotMountActionChanged(bool)) );
  
  actionCollection()->addAction( "search_abort_action", search_abort_action );
  actionCollection()->addAction( "clear_search_action", clear_action );
  actionCollection()->addAction( "mount_action", mount_action );

  // Disable all actions.
  search_abort_action->setEnabled( false );
  clear_action->setEnabled( false );
  mount_action->setEnabled( false );

  // Put the actions in the context menu.
  m_menu = new KActionMenu( this );
  m_menu_title = m_menu->menu()->addTitle( KIcon( "system-search" ), i18n( "Search Results" ) );
  m_menu->addAction( clear_action );
  m_menu->addAction( mount_action );
  
  // Put some actions in the tool bar of the search widget
  m_widget->toolBar()->addAction( search_abort_action );
}


KAboutData *Smb4KNetworkSearchPart::createAboutData()
{
  KAboutData *aboutData = new KAboutData( "smb4knetworksearchpart",
                          "smb4k",
                          ki18n( "Smb4KNetworkSearchPart" ),
                          "3.0",
                          ki18n( "The network search KPart of Smb4K" ),
                          KAboutData::License_GPL_V2,
                          ki18n( "\u00A9 2007-2011, Alexander Reinholdt" ),
                          KLocalizedString(),
                          "http://smb4k.sourceforge.net",
                          "smb4k-bugs@lists.sourceforge.net" );

  return aboutData;
}


void Smb4KNetworkSearchPart::customEvent( QEvent *e )
{
  if ( e->type() == Smb4KEvent::LoadSettings )
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
          Smb4KShare *share = new Smb4KShare( *item->shareItem() );
          share->setIsMounted( false );
          item->update( share );
          delete share;
            
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
  }
  else if ( e->type() == Smb4KEvent::SetFocus )
  {
    m_widget->comboBox()->lineEdit()->setFocus();
  }
  else if ( e->type() == Smb4KEvent::MountOrUnmountShare )
  {
    // Change the active state of the mount action. This needs
    // to be done here, because the action is not switched
    // automatically in case the part is notified from outside.
    KDualAction *mount_action = static_cast<KDualAction *>( actionCollection()->action( "mount_action" ) );
    mount_action->setActive( !mount_action->isActive() );

    // Mount or unmount the share.
    slotMountActionTriggered( false );
  }
  else
  {
    // Do nothing
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
    KDualAction *search_abort_action = static_cast<KDualAction *>( actionCollection()->action( "search_abort_action" ) );
    
    if ( search_abort_action && search_abort_action->isActive() )
    {
      slotSearchAbortActionTriggered( false );
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchPart::slotSearchAbortActionTriggered(bool /*checked*/)
{
  KDualAction *search_abort_action = static_cast<KDualAction *>( actionCollection()->action( "search_abort_action" ) );
  
  if ( search_abort_action )
  {
    if ( search_abort_action->isActive() )
    {
      // Start the search.
      m_widget->listWidget()->clear();

      QString search_item = m_widget->comboBox()->currentText();

      if ( !search_item.isEmpty() )
      {
        Smb4KSearch::self()->search( m_widget->comboBox()->currentText(), m_widget );
        KCompletion *completion = m_widget->comboBox()->completionObject();
        completion->addItem( search_item );
      }
      else
      {
        // Do nothing
      }
    }
    else
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
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkSearchPart::slotSearchAbortActionChanged(bool active)
{
  if ( active )
  {
    actionCollection()->action( "search_abort_action" )->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_S ) );
  }
  else
  {
    actionCollection()->action( "search_abort_action" )->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_A ) );
  }
}


void Smb4KNetworkSearchPart::slotClearActionTriggered( bool /*checked*/ )
{
  // Clear the combo box and the list widget.
  m_widget->comboBox()->clear();
  m_widget->comboBox()->clearEditText();
  m_widget->listWidget()->clear();

  // Disable the actions.
  actionCollection()->action( "search_abort_action" )->setEnabled( false );
  actionCollection()->action( "clear_search_action" )->setEnabled( false );
  actionCollection()->action( "mount_action" )->setEnabled( false );
}


void Smb4KNetworkSearchPart::slotMountActionTriggered( bool /*checked*/ )
{
  // Check if we need to add this host to the global list of hosts.
  Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->currentItem() );

  switch ( searchItem->type() )
  {
    case Smb4KNetworkSearchItem::Share:
    {
      if ( !searchItem->shareItem()->isMounted() )
      {
        Smb4KMounter::self()->mountShare( searchItem->shareItem(), m_widget );
      }
      else
      {
        Smb4KMounter::self()->unmountShare( searchItem->shareItem(), false, m_widget );
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkSearchPart::slotMountActionChanged( bool active )
{
  if ( active )
  {
    actionCollection()->action( "mount_action" )->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_M ) );
  }
  else
  {
    actionCollection()->action( "mount_action" )->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_U ) );
  }
}


void Smb4KNetworkSearchPart::slotComboBoxTextChanged( const QString &text )
{
  actionCollection()->action( "search_abort_action" )->setEnabled( !text.isEmpty() );
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
        if ( !searchItem->shareItem()->isMounted() )
        {
          Smb4KMounter::self()->mountShare( searchItem->shareItem(), m_widget );
        }
        else
        {
          Smb4KMounter::self()->unmountShare( searchItem->shareItem(), false, m_widget );
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


void Smb4KNetworkSearchPart::slotItemSelectionChanged()
{
  QList<QListWidgetItem *> list = m_widget->listWidget()->selectedItems();

  if ( !list.isEmpty() )
  {
    // We are in single selection mode, so there is only one
    // item in the list.
    if ( list.count() == 1 )
    {
      Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>( list.first() );

      switch ( searchItem->type() )
      {
        case Smb4KNetworkSearchItem::Share:
        {
          if ( !searchItem->shareItem()->isMounted() || (searchItem->shareItem()->isMounted() && searchItem->shareItem()->isForeign()) )
          {
            actionCollection()->action( "mount_action" )->setEnabled( true );
            static_cast<KDualAction *>( actionCollection()->action( "mount_action" ) )->setActive( true );
          }
          else if ( searchItem->shareItem()->isMounted() && !searchItem->shareItem()->isForeign() )
          {
            actionCollection()->action( "mount_action" )->setEnabled( true );
            static_cast<KDualAction *>( actionCollection()->action( "mount_action" ) )->setActive( false );
          }
          else
          {
            actionCollection()->action( "mount_action" )->setEnabled( false );
            static_cast<KDualAction *>( actionCollection()->action( "mount_action" ) )->setActive( true );
          }
          
          break;
        }
        default:
        {
          actionCollection()->action( "mount_action" )->setEnabled( false );
          static_cast<KDualAction *>( actionCollection()->action( "mount_action" ) )->setActive( true );
          break;
        }
      }
    }
    else
    {
      actionCollection()->action( "mount_action" )->setEnabled( false );
      static_cast<KDualAction *>( actionCollection()->action( "mount_action" ) )->setActive( true );
    }
  }
  else
  {
    actionCollection()->action( "mount_action" )->setEnabled( false );
    static_cast<KDualAction *>( actionCollection()->action( "mount_action" ) )->setActive( true );
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
      case Smb4KNetworkSearchItem::Share:
      {
        m_menu_title = m_menu->menu()->addTitle( item->icon(),
                                                 item->shareItem()->unc(),
                                                 actionCollection()->action( "clear_search_action" ) );
        break;
      }
      default:
      {
        m_menu_title = m_menu->menu()->addTitle( KIcon( "system-search" ),
                                                 i18n( "Search Results" ),
                                                 actionCollection()->action( "clear_search_action" ) );
        break;
      }
    }
  }
  else
  {
    m_menu_title = m_menu->menu()->addTitle( KIcon( "system-search" ),
                                             i18n( "Search Results" ),
                                             actionCollection()->action( "clear_search_action" ) );
  }

  m_menu->menu()->popup( m_widget->listWidget()->viewport()->mapToGlobal( pos ) );
}


void Smb4KNetworkSearchPart::slotReceivedSearchResult( Smb4KShare *share )
{
  Q_ASSERT( share );

  // Create a Smb4KNetworkSearchItem and add it to the first position.
  (void) new Smb4KNetworkSearchItem( m_widget->listWidget(), share );

  m_widget->listWidget()->sortItems();

  // Enable the combo box and set the focus:
  m_widget->comboBox()->setEnabled( true );
  m_widget->comboBox()->setFocus();

  // Now select the text, so that the user can easily
  // remove it.
  m_widget->comboBox()->lineEdit()->selectAll();
}


void Smb4KNetworkSearchPart::slotSearchAboutToStart( const QString &string )
{
  if ( !m_silent )
  {
    emit setStatusBarText( i18n( "Searching for \"%1\"...", string ) );
  }
  else
  {
    // Do nothing
  }

  m_widget->comboBox()->setEnabled( false );
  KDualAction *search_abort_action = static_cast<KDualAction *>( actionCollection()->action( "search_abort_action" ) );
  
  if ( search_abort_action )
  {
    search_abort_action->setActive( false );
  }
  else
  {
    // Do nothing
  }
  
  actionCollection()->action( "clear_search_action" )->setEnabled( false );
  // Add action will be disabled elsewhere.
}


void Smb4KNetworkSearchPart::slotSearchFinished( const QString &/*string*/ )
{
  if ( !m_silent )
  {
    emit setStatusBarText( i18n( "Done." ) );
  }
  else
  {
    // Do nothing
  }

  m_widget->comboBox()->setEnabled( true );
  KDualAction *search_abort_action = static_cast<KDualAction *>( actionCollection()->action( "search_abort_action" ) );
  
  if ( search_abort_action )
  {
    search_abort_action->setActive( true );
  }
  else
  {
    // Do nothing
  }
  
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


void Smb4KNetworkSearchPart::slotShareMounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  for ( int i = 0; i < m_widget->listWidget()->count(); ++i )
  {
    Smb4KNetworkSearchItem *searchItem = static_cast<Smb4KNetworkSearchItem *>( m_widget->listWidget()->item( i ) );
    
    switch ( searchItem->type() )
    {
      case Smb4KNetworkSearchItem::Share:
      {
        if ( QString::compare( searchItem->shareItem()->unc(), share->unc(), Qt::CaseInsensitive ) == 0 )
        {
          searchItem->update( share );
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
          item->update( share );
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


void Smb4KNetworkSearchPart::slotMounterFinished(Smb4KShare* /*share*/, int process)
{
  switch ( process )
  {
    case MountShare:
    {
      KDualAction *mount_action = static_cast<KDualAction *>(actionCollection()->action( "mount_action" ));
      
      if ( mount_action )
      {
        mount_action->setActive( false );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case UnmountShare:
    {
      KDualAction *mount_action = static_cast<KDualAction *>(actionCollection()->action( "mount_action" ));
      
      if ( mount_action )
      {
        mount_action->setActive( true );
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


#include "smb4knetworksearch_part.moc"
