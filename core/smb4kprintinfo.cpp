/***************************************************************************
    smb4kprintinfo  -  This is a container that carries information needed
    for printing.
                             -------------------
    begin                : Mo Apr 19 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
    email                : dustpuppy@mail.berlios.de
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
#include <QUrl>

// application specific includes
#include <smb4kprintinfo.h>
#include <smb4kauthinfo.h>


Smb4KPrintInfo::Smb4KPrintInfo( Smb4KShare *printer )
: m_share( *printer ), m_path( QString() ), m_copies( 1 )
{
}


Smb4KPrintInfo::Smb4KPrintInfo( const Smb4KPrintInfo &info )
: m_share( *info.printer() ), m_path( info.filePath() ), m_copies( info.copies() )
{
}


Smb4KPrintInfo::~Smb4KPrintInfo()
{
}


void Smb4KPrintInfo::setPrinter( Smb4KShare *printer )
{
  m_share = *printer;
}


void Smb4KPrintInfo::setFilePath( const QString &path )
{
  m_path = path;
}


void Smb4KPrintInfo::setCopies( int num )
{
  m_copies = num;
}


bool Smb4KPrintInfo::equals( Smb4KPrintInfo *info ) const
{
  // Minimal UNC
  if ( QString::compare( m_share.unc( QUrl::None ), info->printer()->unc( QUrl::None ) ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // Path
  if ( QString::compare( m_path, info->filePath() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // Copies
  if ( m_copies != info->copies() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  return true;
}


void Smb4KPrintInfo::setAuthInfo( Smb4KAuthInfo *authInfo )
{
  m_share.setAuthInfo( authInfo );
}

