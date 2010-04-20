/***************************************************************************
    smb4kpreviewer_p  -  Private helper classes for Smb4KPreviewer class.
                             -------------------
    begin                : So Dez 21 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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

#ifndef SMB4KPREVIEWER_P_H
#define SMB4KPREVIEWER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QThread>

// application specific includes
#include <smb4kpreviewer.h>
#include <smb4kpreviewitem.h>
#include <smb4kprocess.h>

// forward declarations
class Smb4KAuthInfo;

class PreviewThread : public QThread
{
  Q_OBJECT

  public:
    PreviewThread( Smb4KPreviewItem *item,
                   QObject *parent = 0 );
    ~PreviewThread();
    void preview( Smb4KAuthInfo *authInfo,
                  const QString &command );
    Smb4KProcess *process() { return m_proc; }
    Smb4KPreviewItem *previewItem() { return m_item; }
    bool authenticationError() { return m_auth_error; }

  signals:
    void result( Smb4KPreviewItem *item );

  protected slots:
    void slotProcessOutput();
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );

  private:
    Smb4KProcess *m_proc;
    Smb4KPreviewItem *m_item;
    bool m_auth_error;
};

class Smb4KPreviewerPrivate
{
  public:
    Smb4KPreviewerPrivate();
    ~Smb4KPreviewerPrivate();
    Smb4KPreviewer instance;
};

#endif
