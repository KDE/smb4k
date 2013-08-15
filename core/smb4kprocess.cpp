/***************************************************************************
    smb4kprocess  -  This class executes shell processes.
                             -------------------
    begin                : Mi Mär 4 2009
    copyright            : (C) 2009-2011 by Alexander Reinholdt
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
#include <smb4kprocess.h>


class Smb4KProcessPrivate
{
  public:
    bool aborted;
};


Smb4KProcess::Smb4KProcess( QObject *parent )
: KProcess( parent ), d( new Smb4KProcessPrivate )
{
  d->aborted = false;

  // Explicitly set the language, because Samba
  // might be localized (closes SF ticket #34).
  setEnv("LANG", "C");
}


Smb4KProcess::~Smb4KProcess()
{
}


void Smb4KProcess::abort()
{
  d->aborted = true;
  kill();
  waitForFinished( -1 );
}


bool Smb4KProcess::isAborted() const
{
  return d->aborted;
}

#include "smb4kprocess.moc"
