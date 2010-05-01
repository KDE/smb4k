/***************************************************************************
    smb4kmountdialog  -  This class provides a dialog for mounting shares
    manually.
                             -------------------
    begin                : Mo Nov 29 2004
    copyright            : (C) 2004-2010 by Alexander Reinholdt
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
#include <QSize>
#include <QLabel>

// KDE includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// application specific includes
#include <smb4kmountdialog.h>
#include <core/smb4kmounter.h>
#include <core/smb4kbookmarkhandler.h>
#include <core/smb4kbookmark.h>
#include <core/smb4kcore.h>
#include <core/smb4ksettings.h>
#include <core/smb4kdefs.h>
#include <core/smb4khomesshareshandler.h>

Smb4KMountDialog::Smb4KMountDialog( QWidget *parent )
: KDialog( parent )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Mount Share" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  setupView();

  connect( this,                 SIGNAL( okClicked() ),
           this,                 SLOT( slotOkClicked() ) );

  connect( this,                 SIGNAL( cancelClicked() ),
           this,                 SLOT( slotCancelClicked() ) );
           
  connect( Smb4KMounter::self(), SIGNAL( aboutToStart( Smb4KShare *, int ) ),
           this,                 SLOT( slotMounterAboutToStart( Smb4KShare *, int ) ) );

  connect( Smb4KMounter::self(), SIGNAL( finished( Smb4KShare *, int ) ),
           this,                 SLOT( slotMounterFinished( Smb4KShare *, int ) ) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  restoreDialogSize( group );
  m_share_input->completionObject()->setItems( group.readEntry( "ShareNameCompletion", QStringList() ) );
  m_ip_input->completionObject()->setItems( group.readEntry( "IPAddressCompletion", QStringList() ) );
  m_workgroup_input->completionObject()->setItems( group.readEntry( "WorkgroupCompletion", QStringList() ) );
}


Smb4KMountDialog::~Smb4KMountDialog()
{
}


void Smb4KMountDialog::setupView()
{
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *shareLabel = new QLabel( i18n( "UNC Address:" ), main_widget );
  m_share_input = new KLineEdit( main_widget );
  m_share_input->setWhatsThis( i18n( "The Uniform Naming Convention (UNC) address "
    "describes the location of the share. It has the following syntax: "
    "//[USER@]HOST/SHARE. The username is optional." ) );
//   m_share_input->setToolTip( i18n( "The UNC address of the share" ) );
  m_share_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_share_input->setClearButtonShown( true );
  m_share_input->setMinimumWidth( 200 );
  m_share_input->setFocus();
  
  QLabel *addressLabel = new QLabel( i18n( "IP Address:" ), main_widget );
  m_ip_input = new KLineEdit( main_widget);
  m_ip_input->setWhatsThis( i18n( "The Internet Protocol (IP) address identifies the "
    "host in the network and indicates where it is. It has two valid formats, the one "
    "known as IP version 4 (e.g. 192.168.2.11) and the version 6 format "
    "(e.g. 2001:0db8:85a3:08d3:1319:8a2e:0370:7334)." ) );
//   m_ip_input->setToolTip( i18n( "The IP of the host where the share is located" ) );
  m_ip_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_ip_input->setClearButtonShown( true );
  m_ip_input->setMinimumWidth( 200 );

  QLabel *workgroupLabel = new QLabel( i18n( "Workgroup:" ), main_widget );
  m_workgroup_input = new KLineEdit( main_widget );
  m_workgroup_input->setWhatsThis( i18n( "The workgroup or domain identifies the "
    "peer-to-peer computer network the host is located in." ) );
//   m_workgroup_input->setToolTip( i18n( "The workgroup where the host is located" ) );
  m_workgroup_input->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
  m_workgroup_input->setClearButtonShown( true );
  m_workgroup_input->setMinimumWidth( 200 );

  m_bookmark = new QCheckBox( i18n( "Add this share to the bookmarks" ), main_widget );
  m_bookmark->setWhatsThis( i18n( "If you tick this checkbox, the share will be bookmarked "
    "and you can access it e.g. through the \"Bookmarks\" menu entry in the main window." ) );
//   m_bookmark->setToolTip( i18n( "Add this share to the bookmarks" ) );

  layout->addWidget( shareLabel, 0, 0, 0 );
  layout->addWidget( m_share_input, 0, 1, 0 );
  layout->addWidget( addressLabel, 1, 0, 0 );
  layout->addWidget( m_ip_input, 1, 1, 0 );
  layout->addWidget( workgroupLabel, 2, 0, 0 );
  layout->addWidget( m_workgroup_input, 2, 1, 0 );
  layout->addWidget( m_bookmark, 3, 0, 1, 2, 0 );

  slotChangeInputValue( m_share_input->text() );

  // Connections
  connect( m_share_input,     SIGNAL( textChanged ( const QString & ) ) ,
           this,              SLOT( slotChangeInputValue( const QString & ) ) );
  
  connect( m_share_input,     SIGNAL( editingFinished() ), 
           this,              SLOT( slotShareNameEntered() ) );
           
  connect( m_ip_input,        SIGNAL( editingFinished() ), 
           this,              SLOT( slotIPEntered() ) );
  
  connect( m_workgroup_input, SIGNAL( editingFinished() ),
           this,              SLOT( slotWorkgroupEntered() ) );
}


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMountDialog::slotChangeInputValue( const QString& _test)
{
  enableButtonOk( !_test.isEmpty() );
}


void Smb4KMountDialog::slotOkClicked()
{
  if ( !m_share_input->text().trimmed().isEmpty() )
  {
    QUrl url( m_share_input->text().trimmed() );
    url.setScheme( "smb" );
    
    if ( url.isValid() && !url.host().isEmpty() /* no invalid host name */ && 
         url.path().length() > 1 /* share name length */ && !url.path().endsWith( "/" ) )
    {
      if ( QString::compare( url.path(), "/homes" ) == 0 )
      {
        Smb4KShare internal = Smb4KShare( url.toString( QUrl::None ) );
        
        if ( QString::compare( url.userName(), "guest" ) == 0 || url.userName().isEmpty() )
        {
          if ( Smb4KHomesSharesHandler::self()->specifyUser( &internal, this ) )
          {
            m_share = Smb4KShare( internal.homeUNC( QUrl::None ) );
          }
          else
          {
            return;
          }
        }
        else
        {
          m_share = Smb4KShare( internal.homeUNC( QUrl::None ) );
        }
      }
      else
      {
        m_share = Smb4KShare( url.toString( QUrl::None ) );
      }
      
      m_share.setWorkgroupName( m_workgroup_input->text().trimmed() );
      m_share.setHostIP( m_ip_input->text().trimmed() );

      if ( m_bookmark->isChecked() )
      {
        Smb4KBookmarkHandler::self()->addBookmark( &m_share );
      }
      else
      {
        // Do nothing
      }
      
      Smb4KMounter::self()->mountShare( &m_share );
      
      connect( Smb4KMounter::self(), SIGNAL( mounted( Smb4KShare * ) ),
               this,                 SLOT( slotShareMounted( Smb4KShare * ) ) );
    }
    else
    {
      if ( !url.isValid() || url.host().isEmpty() )
      {
        KMessageBox::error( this, i18n( "<qt>The UNC address you entered is invalid.</qt>" ) );
      }
      else
      {
        KMessageBox::error( this, i18n( "<qt>The format of the UNC address you entered is not correct. It must have the form //[USER@]HOST/SHARE." ) );
      }
    }
  }
  
  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
  group.writeEntry( "ShareNameCompletion", m_share_input->completionObject()->items() );
  group.writeEntry( "IPAddressCompletion", m_ip_input->completionObject()->items() );
  group.writeEntry( "WorkgroupCompletion", m_workgroup_input->completionObject()->items() );
}


void Smb4KMountDialog::slotCancelClicked()
{
  Smb4KMounter::self()->abort( &m_share );
}


void Smb4KMountDialog::slotShareMounted( Smb4KShare *share )
{
  Q_ASSERT( share );

  if ( m_share.equals( share, Smb4KShare::NetworkOnly ) )
  {
    accept();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotMounterAboutToStart( Smb4KShare *share, int /*process*/ )
{
  Q_ASSERT( share );
  
  if ( m_share.equals( share, Smb4KShare::NetworkOnly ) )
  {
    enableButtonOk( false );
    m_share_input->setEnabled( false );
    m_ip_input->setEnabled( false );
    m_workgroup_input->setEnabled( false );
    m_bookmark->setEnabled( false );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotMounterFinished( Smb4KShare *share, int /*process*/ )
{
  Q_ASSERT( share );
  
  if ( m_share.equals( share, Smb4KShare::NetworkOnly ) )
  {
    enableButtonOk( !m_share_input->text().isEmpty() );
    m_share_input->setEnabled( true );
    m_ip_input->setEnabled( true );
    m_workgroup_input->setEnabled( true );
    m_bookmark->setEnabled( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotShareNameEntered()
{
  KCompletion *completion = m_share_input->completionObject();
  
  if ( !m_share_input->userText().isEmpty() )
  {
    completion->addItem( m_share_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotIPEntered()
{
  KCompletion *completion = m_ip_input->completionObject();
  
  if ( !m_ip_input->userText().isEmpty() )
  {
    completion->addItem( m_ip_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMountDialog::slotWorkgroupEntered()
{
  KCompletion *completion = m_workgroup_input->completionObject();
  
  if ( !m_workgroup_input->userText().isEmpty() )
  {
    completion->addItem( m_workgroup_input->userText() );
  }
  else
  {
    // Do nothing
  }
}


#include "smb4kmountdialog.moc"

