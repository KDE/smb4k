/***************************************************************************
    smb4kpreviewitem  -   A container for previews of a remote share
                             -------------------
    begin                : Mo Mai 28 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
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

// KDE includes
#include <kdebug.h>

// application specific includes
#include <smb4kpreviewitem.h>
#include <smb4kauthinfo.h>

Smb4KPreviewItem::Smb4KPreviewItem( Smb4KShare *share, const QString &path )
{
  m_share = *share;
  m_path = path.trimmed().isEmpty() ? "/" : path;
}


Smb4KPreviewItem::~Smb4KPreviewItem()
{
}


void Smb4KPreviewItem::setShare( Smb4KShare *share )
{
  m_share = *share;
  m_path = "/";
  clearContents();
}


QString Smb4KPreviewItem::path() const
{
  return m_path;
}


void Smb4KPreviewItem::setPath( const QString &path )
{
  if ( path.trimmed().isEmpty() )
  {
    m_path = "/";
  }
  else
  {
    m_path = path+(!path.endsWith( "/" ) ? "/" : QString());
  }

  clearContents();
}


QString Smb4KPreviewItem::location( QUrl::FormattingOptions options ) const
{
  return m_share.unc( options )+m_path;
}


void Smb4KPreviewItem::addContents( const ContentsItem &item )
{
  m_contents.append( item );
}


void Smb4KPreviewItem::clearContents()
{
  m_contents.clear();
}


bool Smb4KPreviewItem::isRootDirectory()
{
  return (m_path.isEmpty() || QString::compare( m_path, "/" ) == 0);
}


bool Smb4KPreviewItem::equals( Smb4KPreviewItem *item )
{
  // We only need to compare the location string.
  return (QString::compare( location( QUrl::None ), item->location( QUrl::None ), Qt::CaseInsensitive ) == 0);
}


void Smb4KPreviewItem::setAuthInfo( Smb4KAuthInfo *authInfo )
{
  m_share.setAuthInfo( authInfo );
}


