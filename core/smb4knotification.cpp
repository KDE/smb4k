/***************************************************************************
    smb4knotification  -  This class provides notifications for Smb4K.
                             -------------------
    begin                : Son Jun 27 2010
    copyright            : (C) 2010 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.org
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

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <krun.h>
#include <kurl.h>

// application specific includes
#include <smb4knotification.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>


Smb4KNotification::Smb4KNotification()
{
}


Smb4KNotification::~Smb4KNotification()
{

}


void Smb4KNotification::shareMounted( Smb4KShare* share )
{
  Q_ASSERT( share );
  
  if ( Smb4KSettings::self()->showNotifications() )
  {
    m_share = *share;
  
    KNotification *notification = new KNotification( "shareMounted", KNotification::CloseOnTimeout );
    notification->setText( i18n( "The share <b>%1</b> has been mounted to <tt>%2</tt>." ).arg( share->unc() )
                          .arg( QString::fromUtf8( share->canonicalPath() ) ) );
    notification->setActions( QStringList( i18n( "Open" ) ) );
    notification->setPixmap( KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ) );
    
    connect( notification, SIGNAL( activated( unsigned int ) ), this, SLOT( slotOpenShare() ) );

    notification->sendEvent();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::shareUnmounted( Smb4KShare* share )
{
  Q_ASSERT( share );
  
  if ( Smb4KSettings::self()->showNotifications() )
  {  
    KNotification::event( "shareUnmounted", i18n( "The share <b>%1</b> has been unmounted from <tt>%2</tt>." ).arg( share->unc() )
                          .arg( QString::fromUtf8( share->path() ) ),
                          KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ) );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::sharesRemounted( int total, int actual )
{
  if ( Smb4KSettings::self()->showNotifications() )
  {
    if ( total != actual )
    {
      KNotification::event( "sharesRemounted", i18np( "%1 share out of %2 has been remounted.", "%1 shares out of %2 have been remounted.", actual, total ),
                            KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ) );
    }
    else
    {
      KNotification::event( "sharesRemounted", i18n( "All shares have been remounted." ),
                            KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-mounted" ) ) );
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNotification::allSharesUnmounted( int total, int actual )
{
  if ( Smb4KSettings::self()->showNotifications() )
  {
    if ( total != actual )
    {
      KNotification::event( "allSharesUnmounted", i18np( "%1 share out of %2 has been unmounted.", "%1 shares out of %2 have been unmounted.", actual, total ),
                            KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ) );
    }
    else
    {
      KNotification::event( "allSharesUnmounted", i18n( "All shares have been unmounted." ),
                            KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::NoGroup, 0, KIconLoader::DefaultState, QStringList( "emblem-unmounted" ) ) );
    }
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KNotification::slotOpenShare()
{
  KRun::runUrl( KUrl( m_share.canonicalPath() ), "inode/directory", 0 );
}


#include "smb4knotification.moc"
