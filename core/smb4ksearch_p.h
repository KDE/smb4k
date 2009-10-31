/***************************************************************************
    smb4ksearch_p  -  Private helper classes for Smb4KSearch class.
                             -------------------
    begin                : Mo Dez 22 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

#ifndef SMB4KSEARCH_P_H
#define SMB4KSEARCH_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QThread>
#include <QList>

// application specific includes
#include <smb4ksearch.h>
#include <smb4kprocess.h>

// forward declarations
class Smb4KBasicNetworkItem;
class Smb4KAuthInfo;


class SearchThread : public QThread
{
  Q_OBJECT

  public:
    SearchThread( QObject *parent = 0 );
    ~SearchThread();
    Smb4KProcess *process() { return m_proc; }
    void search( const QString &searchItem,
                 Smb4KAuthInfo *authInfo,
                 const QString &command );
    const QString &searchItem() { return m_string; }

  signals:
    void authError( const QString &searchItem );
    void result( Smb4KBasicNetworkItem *item );

  protected slots:
    void slotProcessOutput();
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus );

  private:
    Smb4KProcess *m_proc;
    QString m_string;
};


class Smb4KSearchPrivate
{
  public:
    Smb4KSearchPrivate();
    ~Smb4KSearchPrivate();
    Smb4KSearch instance;
};

#endif
