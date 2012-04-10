/***************************************************************************
    smb4knetworkitemcontainer  -  This class derives from QObject and
    encapsulates the network items. It is for use with QtQuick.
                             -------------------
    begin                : Fr Mär 02 2012
    copyright            : (C) 2012 by Alexander Reinholdt
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

// application specific includes
#include <smb4knetworkobject.h>


Smb4KNetworkObject::Smb4KNetworkObject( Smb4KWorkgroup *workgroup, QObject *parent )
: QObject( parent ), m_workgroup( *workgroup ), m_type( Workgroup )
{
}


Smb4KNetworkObject::Smb4KNetworkObject( Smb4KHost *host, QObject *parent )
: QObject( parent ), m_host( *host ), m_type( Host )
{
}


Smb4KNetworkObject::Smb4KNetworkObject( Smb4KShare *share, QObject *parent )
: QObject( parent ), m_share( *share ), m_type( Share )
{
}


Smb4KNetworkObject::Smb4KNetworkObject( QObject *parent )
: QObject( parent ), m_type( Unknown )
{
}


Smb4KNetworkObject::~Smb4KNetworkObject()
{
}


const QString Smb4KNetworkObject::workgroupName()
{
  QString name;
  
  switch ( m_type )
  {
    case Workgroup:
    {
      name = m_workgroup.workgroupName();
      break;
    }
    case Host:
    {
      name = m_host.workgroupName();
      break;
    }
    case Share:
    {
      name = m_share.workgroupName();
      break;
    }
    default:
    {
      break;
    }
  }

  return name;
}


const QString Smb4KNetworkObject::hostName()
{
  QString name;

  switch ( m_type )
  {
    case Host:
    {
      name = m_host.hostName();
      break;
    }
    case Share:
    {
      name = m_share.hostName();
      break;
    }
    default:
    {
      break;
    }
  }

  return name;
}


const QString Smb4KNetworkObject::shareName()
{
  QString name;

  switch ( m_type )
  {
    case Share:
    {
      name = m_share.shareName();
      break;
    }
    default:
    {
      break;
    }
  }

  return name;
}


const QIcon Smb4KNetworkObject::icon()
{
  QIcon icon;

  switch ( m_type )
  {
    case Workgroup:
    {
      icon = m_workgroup.icon();
      break;
    }
    case Host:
    {
      icon = m_host.icon();
      break;
    }
    case Share:
    {
      icon = m_share.icon();
      break;
    }
    default:
    {
      break;
    }
  }
  
  return icon;
}


const QString Smb4KNetworkObject::comment()
{
  QString comment;
  
  switch ( m_type )
  {
    case Host:
    {
      comment = m_host.comment();
      break;
    }
    case Share:
    {
      comment = m_share.comment();
      break;
    }
    default:
    {
      break;
    }
  }
  
  return comment;
}


const QUrl Smb4KNetworkObject::url()
{
  QUrl url;
  
  switch ( m_type )
  {
    case Workgroup:
    {
      url = m_workgroup.url();
      break;
    }
    case Host:
    {
      url = m_host.url();
      break;
    }
    case Share:
    {
      url = m_share.url();
      break;
    }
    default:
    {
      break;
    }
  }
  
  return url;
}


bool Smb4KNetworkObject::isMounted()
{
  bool mounted = false;
  
  switch ( m_type )
  {
    case Share:
    {
      mounted = m_share.isMounted();
      break;
    }
    default:
    {
      break;
    }
  }
  
  return mounted;
}


void Smb4KNetworkObject::update( Smb4KBasicNetworkItem *networkItem )
{
  if ( m_type == Workgroup && networkItem->type() == Smb4KBasicNetworkItem::Workgroup )
  {
    Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( networkItem );
    
    if ( workgroup )
    {
      // Check that we update with the correct item.
      if ( QString::compare( m_workgroup.workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive ) == 0 )
      {
        m_workgroup = *workgroup;
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
  else if ( m_type == Host && networkItem->type() == Smb4KBasicNetworkItem::Host )
  {
    Smb4KHost *host = static_cast<Smb4KHost *>( networkItem );
    
    if ( host )
    {
      // Check that we update with the correct item.
      if ( QString::compare( m_host.workgroupName(), host->workgroupName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( m_host.hostName(), host->hostName(), Qt::CaseInsensitive ) == 0 )
      {
        m_host = *host;
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
  else if ( m_type == Share && networkItem->type() == Smb4KBasicNetworkItem::Share )
  {
    Smb4KShare *share = static_cast<Smb4KShare *>( networkItem );
    
    if ( share )
    {
      // Check that we update with the correct item.
      if ( QString::compare( m_share.workgroupName(), share->workgroupName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( m_share.hostName(), share->hostName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( m_share.shareName(), share->shareName(), Qt::CaseInsensitive ) == 0 )
      {
        m_share = *share;
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
  else
  {
    m_type = Unknown;
  }
}


#include "smb4knetworkobject.moc"
