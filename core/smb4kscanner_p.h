/***************************************************************************
    smb4kscanner_p  -  This is a private helper class for Smb4KScanner.
                             -------------------
    begin                : Do Jul 19 2007
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

#ifndef SMB4KSCANNER_P_H
#define SMB4KSCANNER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QThread>

// application specific includes
#include <smb4kscanner.h>
#include <smb4kprocess.h>
#include <smb4kbasicnetworkitem.h>
#include <smb4kauthinfo.h>

// forward declarations
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;

class BasicScanThread : public QThread
{
  Q_OBJECT

  friend class LookupDomainsThread;
  friend class LookupMembersThread;
  friend class LookupSharesThread;
  friend class LookupInfoThread;

  public:
    enum Type { LookupDomainThread,
                LookupMembersThread,
                LookupSharesThread,
                LookupInfoThread,
                UnknownThread };
    BasicScanThread( Type type,
                     Smb4KBasicNetworkItem *item,
                     QObject *parent = 0 );
    ~BasicScanThread();
    Smb4KProcess *process() { return m_proc; }
    Smb4KBasicNetworkItem *networkItem() { return m_item; }
    Type type() { return m_type; }

  signals:
    void workgroups( QList<Smb4KWorkgroup> &list );
    void hosts( Smb4KWorkgroup *workgroup,
                QList<Smb4KHost> &list );
    void shares( Smb4KHost *host,
                 QList<Smb4KShare> &list );
    void authError( Smb4KBasicNetworkItem *item );
    void info( Smb4KHost *host );

  private:
    Smb4KProcess *m_proc;
    Smb4KBasicNetworkItem *m_item;
    Type m_type;
    QList<Smb4KWorkgroup> m_workgroups;
    QList<Smb4KHost> m_hosts;
    QList<Smb4KShare> m_shares;
    Smb4KAuthInfo m_auth_info;
};


class LookupDomainsThread : public BasicScanThread
{
  Q_OBJECT

  public:
    enum Mode { LookupDomains,
                QueryMaster,
                ScanBroadcastAreas,
                Unknown };
    LookupDomainsThread( Mode mode, QObject *parent = 0 );
    ~LookupDomainsThread();
    void lookup( const QString &command );

  protected slots:
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );

  private:
    Mode m_mode;
    void processLookupDomains();
    void processQueryMaster();
    void processScanBroadcastAreas();
};


class LookupMembersThread : public BasicScanThread
{
  Q_OBJECT

  public:
    LookupMembersThread( Smb4KWorkgroup *workgroup,
                         QObject *parent = 0 );
    ~LookupMembersThread();
    void lookup( bool auth_required,
                 Smb4KAuthInfo *authInfo,
                 const QString &command );

  protected slots:
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
};


class LookupSharesThread : public BasicScanThread
{
  Q_OBJECT

  public:
    LookupSharesThread( Smb4KHost *host,
                        QObject *parent = 0 );
    ~LookupSharesThread();
    void lookup( Smb4KAuthInfo *authInfo,
                 const QString &command );

  protected slots:
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
};


class LookupInfoThread : public BasicScanThread
{
  Q_OBJECT

  public:
    LookupInfoThread( Smb4KHost *host,
                      QObject *parent = 0 );
    ~LookupInfoThread();
    void lookup( const QString &command );

  protected slots:
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
};


class Smb4KScannerPrivate
{
  public:
    Smb4KScannerPrivate();
    ~Smb4KScannerPrivate();
    Smb4KScanner instance;
};

#endif
