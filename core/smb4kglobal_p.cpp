/***************************************************************************
    smb4kglobal_p  -  This is the private helper class of the Smb4KGlobal
    namespace.
                             -------------------
    begin                : Di Jul 24 2007
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// application specific includes
#include "smb4kglobal_p.h"


Smb4KGlobalPrivate::Smb4KGlobalPrivate()
{
}


Smb4KGlobalPrivate::~Smb4KGlobalPrivate()
{
  // Clear the workgroup list.
  while ( !workgroupsList.isEmpty() )
  {
    delete workgroupsList.takeFirst();
  }

  // Clear the host list.
  while ( !hostsList.isEmpty() )
  {
    delete hostsList.takeFirst();
  }

  // Clear the list of mounted shares.
  while ( !mountedSharesList.isEmpty() )
  {
    delete mountedSharesList.takeFirst();
  }

  // Clear the list of shares.
  while ( !sharesList.isEmpty() )
  {
    delete sharesList.takeFirst();
  }
}

