/***************************************************************************
    smb4kprofilemanager_p  -  Private helper class(es) for the profile 
    manager.
                             -------------------
    begin                : Mi Aug 12 2014
    copyright            : (C) 2014 by Alexander Reinholdt
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
#include "smb4kprofilemanager_p.h"


Smb4KProfileMigrationDialog::Smb4KProfileMigrationDialog(const QString& from, const QStringList& to, QWidget* parent)
: KDialog(parent)
{
}


Smb4KProfileMigrationDialog::~Smb4KProfileMigrationDialog()
{
}


#include "smb4kprofilemanager_p.moc"
