/***************************************************************************
    smb4kmountdialog  -  This class provides a dialog for mounting shares
    manually.
                             -------------------
    begin                : Mo Nov 29 2004
    copyright            : (C) 2004-2007 by Alexander Reinholdt
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

Smb4KMountDialog::Smb4KMountDialog( QWidget *parent )
: KDialog( parent )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Mount Share" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  setupView();

  connect( m_share_input,        SIGNAL( textChanged ( const QString & ) ) ,
           this,                 SLOT( slotChangeInputValue( const QString & ) ) );

  connect( this,                 SIGNAL( okClicked() ),
           this,                 SLOT( slotOkClicked() ) );

  connect( this,                 SIGNAL( cancelClicked() ),
           this,                 SLOT( slotCancelClicked() ) );
           
  connect( Smb4KCore::mounter(), SIGNAL( aboutToStart( Smb4KShare *, int ) ),
           this,                 SLOT( slotMounterAboutToStart( Smb4KShare *, int ) ) );

  connect( Smb4KCore::mounter(), SIGNAL( finished( Smb4KShare *, int ) ),
           this,                 SLOT( slotMounterFinished( Smb4KShare *, int ) ) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  restoreDialogSize( group );
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

  QLabel *shareLabel = new QLabel( i18n( "Share:" ), main_widget );
  m_share_input = new KLineEdit( main_widget );
  m_share_input->setMinimumWidth( 200 );
  m_share_input->setFocus();

  QLabel *addressLabel = new QLabel( i18n( "IP Address:" ), main_widget );
  m_ip_input = new KLineEdit( main_widget);
  m_ip_input->setMinimumWidth( 200 );

  QLabel *workgroupLabel = new QLabel( i18n( "Workgroup:" ), main_widget );
  m_workgroup_input = new KLineEdit( main_widget );
  m_workgroup_input->setMinimumWidth( 200 );

  m_bookmark = new QCheckBox( i18n( "Add this share to the bookmarks" ), main_widget );

  layout->addWidget( shareLabel, 0, 0, 0 );
  layout->addWidget( m_share_input, 0, 1, 0 );
  layout->addWidget( addressLabel, 1, 0, 0 );
  layout->addWidget( m_ip_input, 1, 1, 0 );
  layout->addWidget( workgroupLabel, 2, 0, 0 );
  layout->addWidget( m_workgroup_input, 2, 1, 0 );
  layout->addWidget( m_bookmark, 3, 0, 1, 2, 0 );

  slotChangeInputValue( m_share_input->text() );
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
  // FIXME: Leave the decision if the share is not formatted
  // correctly up to the mounter. Just pass the string to
  // Smb4KCore::mounter()->mountShare().

  if ( !m_share_input->text().trimmed().isEmpty() )
  {
    if ( m_share_input->text().contains( "/" ) == 3 &&
         m_share_input->text().contains( '@' ) == 0 )
    {
      m_share = Smb4KShare( m_share_input->text().trimmed() /* UNC */ );
      m_share.setWorkgroupName( m_workgroup_input->text().trimmed() );
      m_share.setHostIP( m_ip_input->text().trimmed() );

      Smb4KCore::mounter()->mountShare( &m_share );

      if ( m_bookmark->isChecked() )
      {
        Smb4KCore::bookmarkHandler()->addBookmark( &m_share );
      }
      else
      {
        // Do nothing
      }

      connect( Smb4KCore::mounter(), SIGNAL( mounted( Smb4KShare * ) ),
               this,                 SLOT( slotShareMounted( Smb4KShare * ) ) );
    }
    else
    {
      KMessageBox::error( this, i18n( "The format of the share you entered is not correct. It must have the form //HOST/SHARE." ) );
    }
  }

  KConfigGroup group( Smb4KSettings::self()->config(), "MountDialog" );
  saveDialogSize( group, KConfigGroup::Normal );
}


void Smb4KMountDialog::slotCancelClicked()
{
  Smb4KCore::mounter()->abort( &m_share );
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

#include "smb4kmountdialog.moc"

