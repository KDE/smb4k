/***************************************************************************
    smb4khomesshareshandler  -  This class handles the homes shares.
                             -------------------
    begin                : Do Aug 10 2006
    copyright            : (C) 2006-2010 by Alexander Reinholdt
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
#include <QFile>
#include <QLineEdit>
#include <QTextCodec>
#include <QDesktopWidget>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

// KDE includes
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kapplication.h>
#include <kglobal.h>

// application specific includes
#include <smb4khomesshareshandler.h>
#include <smb4kdefs.h>
#include <smb4kcoremessage.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4kauthinfo.h>


class Smb4KHomesSharesHandlerPrivate
{
  public:
    /**
     * The Smb4KHomesShareHandler instance
     */
    Smb4KHomesSharesHandler instance;
};


K_GLOBAL_STATIC( Smb4KHomesSharesHandlerPrivate, m_priv );


Smb4KHomesSharesHandler::Smb4KHomesSharesHandler() : QObject()
{
  // First we need the directory.
  QString dir = KGlobal::dirs()->locateLocal( "data", "smb4k", KGlobal::mainComponent() );

  if ( !KGlobal::dirs()->exists( dir ) )
  {
    KGlobal::dirs()->makeDir( dir );
  }

  readUserNames();

  m_dlg = NULL;
}


Smb4KHomesSharesHandler::~Smb4KHomesSharesHandler()
{
  delete m_dlg;
}


Smb4KHomesSharesHandler *Smb4KHomesSharesHandler::self()
{
  return &m_priv->instance;
}


bool Smb4KHomesSharesHandler::specifyUser( Smb4KShare *share, QWidget *parent )
{
  if ( kapp )
  {
    if ( kapp->activeWindow() )
    {
      parent = kapp->activeWindow();
    }
    else
    {
      parent = kapp->desktop();
    }
  }
  else
  {
    // Do nothing
  }

  m_dlg = new KDialog( parent );
  m_dlg->setCaption( i18n( "Specify User" ) );
  m_dlg->setButtons( KDialog::User1|KDialog::Ok|KDialog::Cancel );
  m_dlg->setDefaultButton( KDialog::Ok );
  m_dlg->setButtonGuiItem( KDialog::User1, KGuiItem( i18n( "Clear List" ), "edit-clear", 0, 0 ) );
  m_dlg->enableButton( KDialog::Ok, false );
  m_dlg->enableButton( KDialog::User1, false );

  // Set up the ask pass dialog.
  QWidget *frame = new QWidget( m_dlg );
  QGridLayout *layout = new QGridLayout( frame );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  m_dlg->setMainWidget( frame );

  QLabel *pic = new QLabel( frame );
  pic->setPixmap( DesktopIcon( "user-identity" ) );
  pic->setMargin( 10 );

  QLabel *text = new QLabel( i18n( "Please specify a user name." ), frame );

  QLabel *userLabel = new QLabel( i18n( "User:" ), frame );
  KComboBox *userCombo = new KComboBox( true, frame );
  userCombo->setObjectName( "UserComboBox" );
  userCombo->setDuplicatesEnabled( false );

  QSpacerItem *spacer1 = new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred );

  layout->addWidget( pic, 0, 0, 0 );
  layout->addWidget( text, 0, 1, 1, 2, 0 );
  layout->addWidget( userLabel, 1, 0, 0 );
  layout->addWidget( userCombo, 1, 1, 1, 2, 0 );
  layout->addItem( spacer1, 0, 2 );

  connect( userCombo, SIGNAL( textChanged( const QString &) ),
           this,      SLOT( slotTextChanged( const QString & ) ) );
  connect( m_dlg,     SIGNAL( user1Clicked() ),
           this,      SLOT( slotClearClicked() ) );

  Smb4KShare *internal = findShare( share );

  if ( internal )
  {
    internal->setWorkgroupName( share->workgroupName() );
    internal->setHostIP( share->hostIP() );
  }
  else
  {
    m_list.append( *share );
    internal = &m_list.last();
  }

  if ( !internal->homesUsers().isEmpty() )
  {
    userCombo->addItems( internal->homesUsers() );
    m_dlg->enableButton( KDialog::User1, true );
  }
  else
  {
    // Do nothing
  }

  // Do the last things before showing.
  userCombo->setFocus();
  m_dlg->setFixedSize( m_dlg->sizeHint() );

  // Return value.
  bool success = false;

  if ( m_dlg->exec() == KDialog::Accepted )
  {
    QStringList users;

    // Write the new list of logins to the config file.
    if ( !userCombo->lineEdit()->text().isEmpty() )
    {
      users.append( userCombo->lineEdit()->text() );
    }
    else
    {
      // Do nothing
    }

    int index = 0;

    while ( index < userCombo->count() )
    {
      if ( users.indexOf( userCombo->itemText( index ), 0 ) == -1 )
      {
        users.append( userCombo->itemText( index ) );
      }
      else
      {
        // Do nothing
      }

      index++;
    }

    users.removeAll( QString() );
    users.sort();

    share->setHomesUsers( users );
    share->setLogin( userCombo->currentText() );

    internal->setHomesUsers( users );

    writeUserNames();

    success = !userCombo->currentText().trimmed().isEmpty();
  }
  else
  {
    // When the user cleared the list of homes users, we will clear
    // the respective lists here, too.
    if ( userCombo->count() < internal->homesUsers().size() )
    {
      share->setHomesUsers( QStringList() );
      internal->setHomesUsers( QStringList() );

      writeUserNames();
    }
    else
    {
      // Do nothing
    }

    success = false;
  }

  delete m_dlg;
  m_dlg = NULL;

  return success;
}


void Smb4KHomesSharesHandler::setHomesUsers( Smb4KShare *share )
{
  Smb4KShare *internal = findShare( share );

  if ( internal )
  {
    share->setHomesUsers( internal->homesUsers() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KHomesSharesHandler::setHomesUsers( Smb4KAuthInfo *authInfo )
{
  Smb4KShare *internal = findShare( authInfo );

  if ( internal )
  {
    authInfo->setHomesUsers( internal->homesUsers() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KHomesSharesHandler::readUserNames()
{
  // FIXME: Remove this as soon as possible.

  // Check if we have an old file and import its data if it exists.
  QFile file( KGlobal::dirs()->locateLocal( "data", "smb4k/homes_shares", KGlobal::mainComponent() ) );

  if ( file.exists() )
  {
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, we'll keep
      // it for now.
      ts.setCodec( QTextCodec::codecForLocale() );

      QString line;
      bool get_names = false;
      QString host;

      while ( !ts.atEnd() )
      {
        line = ts.readLine( 0 );

        if ( !get_names )
        {
          if ( line.contains( QRegExp( "\\[.*\\]" ) ) )
          {
            // Get the host name.
            host = line.section( "[", 1, 1 ).section( "]", 0, 0 ).trimmed();

            // Found the host:
            get_names = true;

            continue;
          }
          else
          {
            // No match yet...
            continue;
          }
        }
        else
        {
          if ( !line.trimmed().isEmpty() )
          {
            if ( !line.contains( QRegExp( "\\[.*\\]" ) ) )
            {
              // Write the names to the list:
              QStringList users = line.split( ",", QString::SkipEmptyParts );

              Smb4KShare share;
              share.setHostName( host );
              share.setHomesUsers( users );

              m_list.append( share );
            }
            else
            {
              // Do nothing
            }

            get_names = false;

            continue;
          }
          else
          {
            get_names = false;

            continue;
          }
        }
      }

      file.close();
    }
    else
    {
      // Do nothing
    }

    writeUserNames();

    file.remove();

    return;
  }
  else
  {
    // Do nothing
  }

  // Locate the XML file.
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/homes_shares.xml", KGlobal::mainComponent() ) );

  if ( xmlFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QXmlStreamReader xmlReader( &xmlFile );

    while ( !xmlReader.atEnd() )
    {
      xmlReader.readNext();

      if ( xmlReader.isStartElement() )
      {
        if ( xmlReader.name() == "homes_shares" && xmlReader.attributes().value( "version" ) != "1.0" )
        {
          xmlReader.raiseError( i18n( "%1 is not a version 1.0 file." ).arg( xmlFile.fileName() ) );

          break;
        }
        else
        {
          if ( xmlReader.name() == "homes" )
          {
            Smb4KShare share;
            share.setShareName( xmlReader.name().toString() );

            while ( !(xmlReader.isEndElement() && xmlReader.name() == "homes") )
            {
              xmlReader.readNext();

              if ( xmlReader.isStartElement() )
              {
                if ( xmlReader.name() == "host" )
                {
                  share.setHostName( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "workgroup" )
                {
                  share.setWorkgroupName( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "ip" )
                {
                  share.setHostIP( xmlReader.readElementText() );
                }
                else if ( xmlReader.name() == "users" )
                {
                  QStringList users;

                  while ( !(xmlReader.isEndElement() && xmlReader.name() == "users") )
                  {
                    xmlReader.readNext();

                    if ( xmlReader.isStartElement() && xmlReader.name() == "user" )
                    {
                      users.append( xmlReader.readElementText() );
                    }
                    else
                    {
                      // Do nothing
                    }
                  }

                  share.setHomesUsers( users );
                }
                else
                {
                  // Do nothing
                }

                continue;
              }
              else
              {
                continue;
              }
            }

            m_list.append( share );
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        continue;
      }
    }

    xmlFile.close();

    if ( xmlReader.hasError() )
    {
      Smb4KCoreMessage::error( ERROR_XML_ERROR, xmlFile.fileName(), xmlReader.errorString() );
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if ( xmlFile.exists() )
    {
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, xmlFile.fileName() );
    }
    else
    {
      // Do nothing
    }

    return;
  }
}


void Smb4KHomesSharesHandler::writeUserNames()
{
  QFile xmlFile( KGlobal::dirs()->locateLocal( "data", "smb4k/homes_shares.xml", KGlobal::mainComponent() ) );

  if ( !m_list.isEmpty() )
  {
    if ( xmlFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QXmlStreamWriter xmlWriter( &xmlFile );
#if QT_VERSION >= 0x040400
      xmlWriter.setAutoFormatting( true );
#endif

      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement( "homes_shares" );
      xmlWriter.writeAttribute( "version", "1.0" );

      for ( int i = 0; i < m_list.size(); ++i )
      {
        xmlWriter.writeStartElement( "homes" );
        xmlWriter.writeAttribute( "profile", "Default" );
        xmlWriter.writeTextElement( "host", m_list.at( i ).hostName() );
        xmlWriter.writeTextElement( "workgroup", m_list.at( i ).workgroupName() );
        xmlWriter.writeTextElement( "ip", m_list.at( i ).hostIP() );
        xmlWriter.writeStartElement( "users" );

        for ( int j = 0; j < m_list.at( i ).homesUsers().size(); ++j )
        {
          xmlWriter.writeTextElement( "user", m_list.at( i ).homesUsers().at( j ) );
        }

        xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
      }

      xmlWriter.writeEndDocument();

      xmlFile.close();
    }
    else
    {
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, xmlFile.fileName() );
      return;
    }
  }
  else
  {
    xmlFile.remove();
  }
}


Smb4KShare *Smb4KHomesSharesHandler::findShare( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  Smb4KShare *internal = NULL;

  for ( int i = 0; i < m_list.size(); ++i )
  {
    if ( QString::compare( m_list.at( i ).hostName(), share->hostName(), Qt::CaseInsensitive ) == 0 &&
         ((m_list.at( i ).workgroupName().isEmpty() || share->workgroupName().isEmpty()) ||
         QString::compare( m_list.at( i ).workgroupName(), share->workgroupName(), Qt::CaseSensitive) == 0) )
    {
      internal = static_cast<Smb4KShare *>( &m_list[i] );
    }
    else
    {
      continue;
    }
  }

  return internal;
}


Smb4KShare *Smb4KHomesSharesHandler::findShare( Smb4KAuthInfo *authInfo )
{
  Smb4KShare *internal = NULL;

  for ( int i = 0; i < m_list.size(); ++i )
  {
    if ( QString::compare( m_list.at( i ).hostName(), authInfo->hostName(), Qt::CaseInsensitive ) == 0 &&
         ((m_list.at( i ).workgroupName().isEmpty() || authInfo->workgroupName().isEmpty()) ||
         QString::compare( m_list.at( i ).workgroupName(), authInfo->workgroupName(), Qt::CaseSensitive) == 0) )
    {
      internal = static_cast<Smb4KShare *>( &m_list[i] );
    }
    else
    {
      continue;
    }
  }

  return internal;
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KHomesSharesHandler::slotTextChanged( const QString &text )
{
  if ( !text.isEmpty() )
  {
    m_dlg->enableButtonOk( true );
  }
  else
  {
    m_dlg->enableButtonOk( false );
  }
}


void Smb4KHomesSharesHandler::slotClearClicked()
{
  if ( m_dlg )
  {
    KComboBox *cb = m_dlg->findChild<KComboBox *>( "UserComboBox" );

    if ( cb )
    {
      cb->clearEditText();
      cb->clear();

      m_dlg->enableButton( KDialog::User1, false );
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

#include "smb4khomesshareshandler.moc"

