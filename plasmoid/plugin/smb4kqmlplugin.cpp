/***************************************************************************
    smb4kqmlplugin - The QML plugin for use with Plasma/QtQuick
                             -------------------
    begin                : Di Feb 21 2012
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kqmlplugin.h"
#include "core/smb4kglobal.h"
#include "core/smb4kscanner.h"
#include "core/smb4kmounter.h"
#include "core/smb4knetworkobject.h"

// Qt includes
#include <QtDeclarative/qdeclarative.h>
#include <QIcon>

void Smb4KQMLPlugin::registerTypes( const char *uri )
{
  qmlRegisterType<Smb4KNetworkObject>( uri, 1, 0, "NetworkObject" );
  qmlRegisterType<Smb4KScanner>( uri, 1, 0, "Scanner" );
  qmlRegisterType<Smb4KMounter>( uri, 1, 0, "Mounter" );
}

Q_EXPORT_PLUGIN2( Smb4KQML, Smb4KQMLPlugin );
