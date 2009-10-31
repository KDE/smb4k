/***************************************************************************
    smb4kprintdialog  -  The print dialog for Smb4K
                             -------------------
    begin                : So Apr 11 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
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
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include <QFile>

// KDE includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstandardguiitem.h>
#include <kconfiggroup.h>
#include <kurl.h>
#include <kdeversion.h>

// application specific includes
#include <smb4kprintdialog.h>
#include <core/smb4kprintinfo.h>
#include <core/smb4kcore.h>
#include <core/smb4khost.h>
#include <core/smb4ksettings.h>
#include <core/smb4kglobal.h>

using namespace Smb4KGlobal;

Smb4KPrintDialog::Smb4KPrintDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Print File" ) );
  setButtons( Details|User2|User1 );
  setDefaultButton( User1 );

  setButtonGuiItem( User1, KStandardGuiItem::close() );
  setButtonGuiItem( User2, KStandardGuiItem::print() );
  setButtonText( Details, i18n( "Options" ) );

  m_info = new Smb4KPrintInfo( share );

  // Set up the view.
  setupView();

  enableButton( User2, false );
  enableButton( Details, false );

  connect( this,   SIGNAL( user1Clicked() ),
           this,   SLOT( slotUser1Clicked() ) );

  connect( this,   SIGNAL( user2Clicked() ),
           this,   SLOT( slotUser2Clicked() ) );

  connect( m_file, SIGNAL( textChanged( const QString & ) ),
           this,   SLOT( slotInputValueChanged( const QString & ) ) );

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "PrintDialog" );
  restoreDialogSize( group );
}


Smb4KPrintDialog::~ Smb4KPrintDialog()
{
  delete m_info;
}


void Smb4KPrintDialog::setupView()
{
  // Build the view:
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *main_widget_layout = new QGridLayout( main_widget );
  main_widget_layout->setSpacing( 5 );
  main_widget_layout->setMargin( 0 );

  // Printer information box
  QGroupBox *printer_box = new QGroupBox( i18n( "Printer" ), main_widget );

  QGridLayout *printer_box_layout = new QGridLayout( printer_box );
  printer_box_layout->setSpacing( 5 );

  QLabel *name_label = new QLabel( i18n( "Name:" ), printer_box );
  QLabel *name       = new QLabel( m_info->printer()->shareName()+
                                   (!m_info->printer()->comment().trimmed().isEmpty() ?
                                   " ("+m_info->printer()->comment()+")" :
                                   ""), printer_box );
  QLabel *host_label = new QLabel( i18n( "Host:" ), printer_box );
  QLabel *host       = new QLabel( m_info->printer()->hostName(), printer_box );
  QLabel *ip_label   = new QLabel( i18n( "IP Address:" ), printer_box );
  QLabel *ip         = new QLabel( m_info->printer()->hostIP().trimmed().isEmpty() ?
                                   i18n( "unknown" ) :
                                   m_info->printer()->hostIP(), printer_box );
  QLabel *wg_label   = new QLabel( i18n( "Workgroup:" ), printer_box );
  QLabel *workgroup  = new QLabel( m_info->printer()->workgroupName(), printer_box );

  printer_box_layout->addWidget( name_label, 0, 0, 0 );
  printer_box_layout->addWidget( name, 0, 1, 0 );
  printer_box_layout->addWidget( host_label, 1, 0, 0 );
  printer_box_layout->addWidget( host, 1, 1, 0 );
  printer_box_layout->addWidget( ip_label, 2, 0, 0 );
  printer_box_layout->addWidget( ip, 2, 1, 0 );
  printer_box_layout->addWidget( wg_label, 3, 0, 0 );
  printer_box_layout->addWidget( workgroup, 3, 1, 0 );

  // File requester box
  QGroupBox *file_box = new QGroupBox( i18n( "File" ), main_widget );

  QGridLayout *file_box_layout = new QGridLayout( file_box );
  file_box_layout->setSpacing( 5 );

  QLabel *file_label  = new QLabel( i18n( "File:" ), file_box );
  m_file              = new KUrlRequester( file_box );
  m_file->setMode( KFile::File | KFile::LocalOnly | KFile::ExistingOnly );

  file_box_layout->addWidget( file_label, 0, 0, 0 );
  file_box_layout->addWidget( m_file, 0, 1, 0 );

  // Details widget
  QGroupBox *options_box = new QGroupBox( i18n( "Options" ), main_widget );
  setDetailsWidget( options_box );

  QGridLayout *options_box_layout = new QGridLayout( options_box );
  options_box_layout->setSpacing( 5 );

  QLabel *copies_label = new QLabel( i18n( "Copies:" ), options_box );
  m_copies             = new KIntNumInput( 1, options_box, 10 );
  m_copies->setMinimum( 1 );

  options_box_layout->addWidget( copies_label, 0, 0, 0 );
  options_box_layout->addWidget( m_copies, 0, 1, 0 );

  main_widget_layout->addWidget( printer_box, 0, 0, 0 );
  main_widget_layout->addWidget( file_box, 1, 0, 0 );

  printer_box->adjustSize();
  file_box->adjustSize();
}


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPrintDialog::slotUser1Clicked()
{
  // Abort or exit.
  if ( Smb4KCore::print()->isRunning( m_info ) )
  {
    Smb4KCore::print()->abort( m_info );
  }
  else
  {
    KConfigGroup group( Smb4KSettings::self()->config(), "PrintDialog" );
    saveDialogSize( group, KConfigGroup::Normal );
    close();
  }
}


void Smb4KPrintDialog::slotUser2Clicked()
{
  if ( m_file->url().isValid() && QFile::exists( m_file->url().path() ) )
  {
    m_info->setFilePath( m_file->url().path().trimmed() );
    m_info->setCopies( m_copies->value() );

    connect( Smb4KCore::print(), SIGNAL( aboutToStart( Smb4KPrintInfo * ) ),
             this,               SLOT( slotAboutToStart( Smb4KPrintInfo * ) ) );

    connect( Smb4KCore::print(), SIGNAL( finished( Smb4KPrintInfo * ) ),
             this,               SLOT( slotFinished( Smb4KPrintInfo * ) ) );

    Smb4KCore::print()->print( m_info );
  }
  else
  {
    KMessageBox::error( this, i18n( "The file is invalid." ) );
  }
}


void Smb4KPrintDialog::slotInputValueChanged( const QString &text )
{
  enableButton( User2, !text.isEmpty() );
  enableButton( Details, !text.isEmpty() );
  setDefaultButton( !text.isEmpty() ? User2 : User1 );
}


void Smb4KPrintDialog::slotAboutToStart( Smb4KPrintInfo *info )
{
  if ( m_info->equals( info ) )
  {
    setButtonGuiItem( User1, KStandardGuiItem::cancel() );
    enableButton( User2, false );
    enableButton( Details, false );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPrintDialog::slotFinished( Smb4KPrintInfo *info )
{
  if ( m_info->equals( info ) )
  {
    setButtonGuiItem( User1, KStandardGuiItem::close() );
//     enableButton( User2, true );
//     enableButton( Details, true );
  }
  else
  {
    // Do nothing
  }
}


#include "smb4kprintdialog.moc"
