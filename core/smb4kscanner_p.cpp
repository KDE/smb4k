/***************************************************************************
    Private helper classes for the scanner
                             -------------------
    begin                : So Mai 22 2011
    copyright            : (C) 2011-2016 by Alexander Reinholdt
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
#include "smb4kscanner_p.h"
#include "smb4ksettings.h"
#include "smb4knotification.h"
#include "smb4kglobal.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kwalletmanager.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QLatin1String>
#include <QStandardPaths>
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <KCoreAddons/KShell>

using namespace Smb4KGlobal;


Smb4KLookupDomainsJob::Smb4KLookupDomainsJob(QObject *parent) : KJob(parent),
  m_started(false), m_parent_widget(0), m_process1(0), m_process2(0)
{
}


Smb4KLookupDomainsJob::~Smb4KLookupDomainsJob()
{
  while (!m_workgroups_list.isEmpty())
  {
    delete m_workgroups_list.takeFirst();
  }
}


void Smb4KLookupDomainsJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartLookup()));
}


void Smb4KLookupDomainsJob::setupLookup(QWidget *parent)
{
  m_parent_widget = parent;
}


bool Smb4KLookupDomainsJob::doKill()
{
  if (m_process1 && m_process1->state() != KProcess::NotRunning)
  {
    m_process1->abort();
  }
  else
  {
    // Do nothing
  }
  
  if (m_process2 && m_process2->state() != KProcess::NotRunning)
  {
    m_process2->abort();
  }
  else
  {
    // Do nothing
  }

  return KJob::doKill();
}


void Smb4KLookupDomainsJob::startProcess1()
{
  //
  // Find shell commands
  //
  QString nmblookup = QStandardPaths::findExecutable("nmblookup");

  if (nmblookup.isEmpty())
  {
    Smb4KNotification::commandNotFound("nmblookup");
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // Global Samba options
  //
  QMap<QString,QString> sambaOptions = globalSambaOptions();
  
  //
  // The command
  //
  QStringList command;
  command << nmblookup;
  
  // Domain
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), sambaOptions["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }

  // NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), sambaOptions["netbios name"]) != 0)
  {
    command << "-n";
    command << Smb4KSettings::netBIOSName();
  }
  else
  {
    // Do nothing
  }

  // NetBIOS scope
  if (!Smb4KSettings::netBIOSScope().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSScope(), sambaOptions["netbios scope"]) != 0)
  {
    command << "-i";
    command << Smb4KSettings::netBIOSScope();
  }
  else
  {
    // Do nothing
  }

  // Socket options
  if (!Smb4KSettings::socketOptions().isEmpty() &&
      QString::compare(Smb4KSettings::socketOptions(), sambaOptions["socket options"]) != 0)
  {
    command << "-O";
    command << Smb4KSettings::socketOptions();
  }
  else
  {
    // Do nothing
  }

  // Port 137
  if (Smb4KSettings::usePort137())
  {
    command << "-r";
  }
  else
  {
    // Do nothing
  }  
  
  // Broadcast address
  QHostAddress address(Smb4KSettings::broadcastAddress());

  if (!Smb4KSettings::broadcastAddress().isEmpty() &&
      address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
  {
    command << "-B";
    command << Smb4KSettings::broadcastAddress();
  }
  else
  {
    // Do nothing
  }

  command << "-M";
  command << "--";
  command << "-";
  
  m_process1 = new Smb4KProcess(this);
  m_process1->setOutputChannelMode(KProcess::SeparateChannels);
  m_process1->setProgram(command);

  connect(m_process1, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcess1Finished(int,QProcess::ExitStatus)));

  emit aboutToStart();

  m_process1->start();
}


void Smb4KLookupDomainsJob::startProcess2(const QStringList& ipAddresses)
{
  //
  // Find shell commands
  //
  QString nmblookup = QStandardPaths::findExecutable("nmblookup");

  if (nmblookup.isEmpty())
  {
    Smb4KNotification::commandNotFound("nmblookup");
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // Global Samba options
  //
  QMap<QString,QString> sambaOptions = globalSambaOptions();

  //
  // The command
  //
  QStringList command;
  command << nmblookup;

  // Domain
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), sambaOptions["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }

  // NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), sambaOptions["netbios name"]) != 0)
  {
    command << "-n";
    command << Smb4KSettings::netBIOSName();
  }
  else
  {
    // Do nothing
  }

  // NetBIOS scope
  if (!Smb4KSettings::netBIOSScope().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSScope(), sambaOptions["netbios scope"]) != 0)
  {
    command << "-i";
    command << Smb4KSettings::netBIOSScope();
  }
  else
  {
    // Do nothing
  }

  // Socket options
  if (!Smb4KSettings::socketOptions().isEmpty() &&
      QString::compare(Smb4KSettings::socketOptions(), sambaOptions["socket options"]) != 0)
  {
    command << "-O";
    command << Smb4KSettings::socketOptions();
  }
  else
  {
    // Do nothing
  }

  // Port 137
  if (Smb4KSettings::usePort137())
  {
    command << "-r";
  }
  else
  {
    // Do nothing
  }  
  
  if (!winsServer().isEmpty())
  {
    // Query the WINS server. In this case we do not need the broadcast
    // based attempt to lookup the domains. Thus, the '-B <addr>' argument
    // is not used in this case.
    command << "-R";
    command << "-U";
    command << winsServer();
  }
  else
  {
    // Use the broadcast based attempt to lookup the domains.
    QHostAddress address(Smb4KSettings::broadcastAddress());

    if (!Smb4KSettings::broadcastAddress().isEmpty() &&
        address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      command << "-B";
      command << Smb4KSettings::broadcastAddress();
    }
    else
    {
      // Do nothing
    }
  }
  
  command << "-A";
  command << ipAddresses;
  
  m_process2 = new Smb4KProcess(this);
  m_process2->setOutputChannelMode(KProcess::SeparateChannels);
  m_process2->setProgram(command);
  
  connect(m_process2, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcess2Finished(int,QProcess::ExitStatus)));

  m_process2->start();
}


void Smb4KLookupDomainsJob::processErrors(const QString& stdErr)
{
  QString errorMessage;
  
  // Remove unimportant warnings
  if (stdErr.contains("Ignoring unknown parameter"))
  {
    QStringList list = stdErr.split('\n');
    QMutableStringListIterator it(list);

    while (it.hasNext())
    {
      QString test = it.next();

      if (test.trimmed().startsWith(QLatin1String("Ignoring unknown parameter")))
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
    }

    errorMessage = list.join("\n");
  }
  else if (stdErr.contains("smb.conf"))
  {
    QStringList list = stdErr.split('\n');
    QMutableStringListIterator it(list);
    
    while (it.hasNext())
    {
      QString test = it.next();
      
      if (test.contains(QLatin1String("smb.conf")) && test.contains(QLatin1String("Can't load")))
      {
        // smb.conf is missing.
        // Output from nmblookup (1 line)
        it.remove();
      }
      else
      {
        // Do nothing
      }
    }
    
    errorMessage = list.join('\n');
  }
  else
  {
    // Do nothing
  }
  
  if (!errorMessage.isEmpty())
  {
    Smb4KNotification::retrievingDomainsFailed(errorMessage);
  }
  else
  {
    // Do nothing
  }
}



void Smb4KLookupDomainsJob::processMasterBrowsers(const QString &stdOut)
{
  QStringList stdOutList = stdOut.split('\n', QString::SkipEmptyParts);
  QStringList ipAddresses;
  
  // Get the IP addresses of the workgroup master browsers
  if (!stdOutList.isEmpty())
  {
    Q_FOREACH(const QString &line, stdOutList)
    {
      if (line.contains("<01>"))
      {        
        ipAddresses << line.trimmed().section(" ", 0, 0).trimmed();
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }

  // Start the lookup of the workgroups/domains
  startProcess2(ipAddresses);
}


void Smb4KLookupDomainsJob::processWorkgroups(const QString &stdOut)
{
  QStringList stdOutList = stdOut.split('\n', QString::SkipEmptyParts);
  
  if (!stdOutList.isEmpty())
  {
    Smb4KWorkgroup *workgroup = new Smb4KWorkgroup();

    Q_FOREACH(const QString &line, stdOutList)
    {
      if (line.startsWith(QLatin1String("Looking up status of")))
      {
        // Get the IP address of the master browser.
        workgroup->setMasterBrowserIP(line.section("of", 1, 1).trimmed());
        continue;
      }
      else if (line.contains("MAC Address", Qt::CaseSensitive))
      {
        // Add workgroup to the list. Ignore misconfigured master browsers,
        // that do not belong to a workgroup/domain, i.e. where the workgroup
        // name is empty.
        if (!workgroup->workgroupName().isEmpty() && !workgroup->masterBrowserName().isEmpty())
        {
          m_workgroups_list << new Smb4KWorkgroup(*workgroup);
        }
        else
        {
          // Do nothing
        }

        delete workgroup;
        workgroup = new Smb4KWorkgroup();
        continue;
      }
      else if (line.contains(" <00> ", Qt::CaseSensitive))
      {
        // Set the name of the workgroup/host.
        if (line.contains(" <GROUP> ", Qt::CaseSensitive))
        {
          // Avoid setting the workgroup name twice.
          if (workgroup->workgroupName().isEmpty())
          {
            workgroup->setWorkgroupName(line.section("<00>", 0, 0).trimmed());
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Avoid setting the name of the master browser twice.
          if (workgroup->masterBrowserName().isEmpty())
          {
            workgroup->setMasterBrowserName(line.section("<00>", 0, 0).trimmed());
          }
          else
          {
            // Do nothing
          }
        }

        continue;
      }
      else if (line.contains(" <1d> ", Qt::CaseSensitive))
      {
        // Get the workgroup name.
        if (workgroup->workgroupName().isEmpty())
        {
          workgroup->setWorkgroupName(line.section("<1d>", 0, 0).trimmed());
        }
        else
        {
          // Do nothing
        }

        continue;
      }
      else
      {
        continue;
      }
    }

    delete workgroup;
  }
  else
  {
    // Do nothing
  }

  emit workgroups(m_workgroups_list);
}


void Smb4KLookupDomainsJob::slotStartLookup()
{
  // Lookup the IP addresses of the workgroup master browsers.
  startProcess1();
}


void Smb4KLookupDomainsJob::slotProcess1Finished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
    case QProcess::CrashExit:
    {
      if (!m_process1->isAborted())
      {
        Smb4KNotification::processError(m_process1->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // Process output
      QString stdOut = QString::fromUtf8(m_process1->readAllStandardOutput(), -1).trimmed();

      // Process errors
      QString stdErr = QString::fromUtf8(m_process1->readAllStandardError(), -1).trimmed();
      
      if (stdOut.trimmed().isEmpty() && !stdErr.trimmed().isEmpty())
      {
        processErrors(stdErr);
      }
      else
      {
        processMasterBrowsers(stdOut);
      }
      
      break;
    }
  }
}



void Smb4KLookupDomainsJob::slotProcess2Finished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
    case QProcess::CrashExit:
    {
      if (!m_process2->isAborted())
      {
        Smb4KNotification::processError(m_process2->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // Process output
      QString stdOut = QString::fromUtf8(m_process2->readAllStandardOutput(), -1).trimmed();

      // Process errors
      QString stdErr = QString::fromUtf8(m_process2->readAllStandardError(), -1).trimmed();
      
      if (stdOut.trimmed().isEmpty() && !stdErr.trimmed().isEmpty())
      {
        processErrors(stdErr);
      }
      else
      {
        processWorkgroups(stdOut);
      }
      
      break;
    }
  }

  emitResult();
  emit finished();
}



Smb4KQueryMasterJob::Smb4KQueryMasterJob(QObject *parent) : KJob(parent),
  m_started(false), m_parent_widget(0), m_process1(0), m_process2(0)
{
}


Smb4KQueryMasterJob::~Smb4KQueryMasterJob()
{
  while (!m_workgroups_list.isEmpty())
  {
    delete m_workgroups_list.takeFirst();
  }
}


void Smb4KQueryMasterJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartLookup()));
}


void Smb4KQueryMasterJob::setupLookup(const QString &master, QWidget *parent)
{
  m_master_browser = master;
  m_parent_widget = parent;
}


bool Smb4KQueryMasterJob::doKill()
{
  if (m_process1 && m_process1->state() != KProcess::NotRunning)
  {
    m_process1->abort();
  }
  else
  {
    // Do nothing
  }
  
  if (m_process2 && m_process2->state() != KProcess::NotRunning)
  {
    m_process2->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KQueryMasterJob::startProcess1()
{
  //
  // Find shell commands
  //
  QString net = QStandardPaths::findExecutable("net");

  if (net.isEmpty())
  {
    Smb4KNotification::commandNotFound("net");
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // The global Samba and custom options as well as the master browser
  //
  QMap<QString,QString> sambaOptions = globalSambaOptions();
  Smb4KCustomOptions *options = 0;
  Smb4KHost host;
  
  //
  // The command
  //
  QStringList command;
  command << net;
  command << "lookup";
  
  if (!m_master_browser.isEmpty())
  {
    // We do not need to set the domain here, because neither
    // Smb4KCustomOptionsMangager nor Smb4KWalletManager need 
    // the domain entry to return correct data.    
    if (QHostAddress(m_master_browser).protocol() == QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      host.setHostName(m_master_browser);
    }
    else
    {
      host.setIP(m_master_browser);
    }

    // Acquire the custom options for the master browser
    options = Smb4KCustomOptionsManager::self()->findOptions(&host);

    // Get authentication information for the host if needed
    if (Smb4KSettings::masterBrowsersRequireAuth())
    {
      Smb4KWalletManager::self()->readAuthInfo(&host);
    }
    else
    {
      // Do nothing
    }
    
    // Custom master browser lookup
    command << "host";
    command << host.hostName();
  }
  else
  {
    // Get authentication information for the host if needed
    if (Smb4KSettings::masterBrowsersRequireAuth() && Smb4KSettings::useDefaultLogin())
    {
      Smb4KWalletManager::self()->readAuthInfo(&host);
    }
    else
    {
      // Do nothing
    }
    
    // Domain master browser lookup
    command << "master";
    
    if (!Smb4KSettings::domainName().isEmpty())
    {
      command << Smb4KSettings::domainName();
    }
    else
    {
      command << sambaOptions["workgroup"];
    }
  }
  
  // User name and password if needed
  if (Smb4KSettings::masterBrowsersRequireAuth() && !host.login().isEmpty())
  {
    command << "-U";
    command << host.login();
  }
  else
  {
    // Do *not* change this to "-U guest%". This won't work.
    command << "-U";
    command << "%";
  }
  
  // The user's workgroup/domain name
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), sambaOptions["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }
  
  // The user's NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), sambaOptions["netbios name"]) != 0)
  {
    command << "-n";
    command << Smb4KSettings::netBIOSName();
  }
  else
  {
    // Do nothing
  }
  
  // Machine account
  if (Smb4KSettings::machineAccount())
  {
    command << "-P";
  }
  else
  {
    // Do nothing
  }
  
  // Encrypt SMB transport
  if (Smb4KSettings::encryptSMBTransport())
  {
    command << "-e";
  }
  else
  {
    // Do nothing
  }
  
  // Use Kerberos
  if (options)
  {
    switch (options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        command << "-k";
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        // No kerberos 
        break;
      }
      case Smb4KCustomOptions::UndefinedKerberos:
      {
        if (Smb4KSettings::useKerberos())
        {
          command << "-k";
        }
        else
        {
          // Do nothing
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    if (Smb4KSettings::useKerberos())
    {
      command << "-k";
    }
    else
    {
      // Do nothing
    }
  }
  
  // Use Winbind's ccache
  if (Smb4KSettings::useWinbindCCache())
  {
    command << "--use-ccache";
  }
  else
  {
    // Do nothing
  }
  
  // Port
  if (options && options->smbPort() != Smb4KSettings::remoteSMBPort())
  {
    command << "-p";
    command << QString("%1").arg(options->smbPort());
  }
  else
  {
    command << "-p";
    command << QString("%1").arg(Smb4KSettings::remoteSMBPort());
  }
  
  //
  // The process
  //
  m_process1 = new Smb4KProcess(this);
  m_process1->setOutputChannelMode(KProcess::SeparateChannels);
  m_process1->setProgram(command);

  if (Smb4KSettings::masterBrowsersRequireAuth() && !host.password().isEmpty())
  {
    m_process1->setEnv("PASSWD", host.password(), true);
  }
  else
  {
    m_process1->unsetEnv("PASSWD");
  }

  connect(m_process1, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcess1Finished(int,QProcess::ExitStatus)));

  emit aboutToStart();

  m_process1->start();
}


void Smb4KQueryMasterJob::startProcess2(const QString& ipAddress)
{
  //
  // Find shell commands
  //
  QString net = QStandardPaths::findExecutable("net");

  if (net.isEmpty())
  {
    Smb4KNotification::commandNotFound("net");
    emitResult();
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // Global Samba and custom options as well as the master browser to query
  //
  QMap<QString,QString> sambaOptions = globalSambaOptions();
  Smb4KCustomOptions *options = 0;
  Smb4KHost host;
  
  if (!m_master_browser.isEmpty())
  {
    // We do not need to set the domain here, because neither
    // Smb4KCustomOptionsMangager nor Smb4KWalletManager need 
    // the domain entry to return correct data.    
    if (QHostAddress(m_master_browser).protocol() == QAbstractSocket::UnknownNetworkLayerProtocol)
    {
      host.setHostName(m_master_browser);
    }
    else
    {
      host.setIP(m_master_browser);
    }

    // Acquire the custom options for the master browser
    options = Smb4KCustomOptionsManager::self()->findOptions(&host);

    // Get authentication information for the host if needed
    if (Smb4KSettings::masterBrowsersRequireAuth())
    {
      Smb4KWalletManager::self()->readAuthInfo(&host);
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Get authentication information for the host if needed
    if (Smb4KSettings::masterBrowsersRequireAuth() && Smb4KSettings::useDefaultLogin())
    {
      Smb4KWalletManager::self()->readAuthInfo(&host);
    }
    else
    {
      // Do nothing
    }
  }  
  
  //
  // The command
  //
  QStringList command;  
  command << net;
  // Protocol & command. Since the domain lookup only works 
  // with the RAP protocol, there is no point in using the 
  // 'Automatic' feature.
  command << "rap";
  command << "domain";
  
  // IP address (discovered by previous net command)
  QHostAddress hostAddress(ipAddress);
  
  if (hostAddress.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
  {
    command << "-I";
    command << hostAddress.toString();
  }
  else
  {
    // Do nothing
  }
  
  // Set debug output to very verbose.
  command << "-d3";
  
  // User name and password if needed
  if (Smb4KSettings::masterBrowsersRequireAuth() && !host.login().isEmpty())
  {
    command << "-U";
    command << host.login();
  }
  else
  {
    // Do *not* change this to "-U guest%". This won't work.
    command << "-U";
    command << "%";
  }
  
  // The user's workgroup/domain name
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), sambaOptions["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }
  
  // The user's NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), sambaOptions["netbios name"]) != 0)
  {
    command << "-n";
    command << Smb4KSettings::netBIOSName();
  }
  else
  {
    // Do nothing
  }
  
  // Machine account
  if (Smb4KSettings::machineAccount())
  {
    command << "-P";
  }
  else
  {
    // Do nothing
  }
  
  // Encrypt SMB transport
  if (Smb4KSettings::encryptSMBTransport())
  {
    command << "-e";
  }
  else
  {
    // Do nothing
  }
  
  // Use Kerberos
  if (options)
  {
    switch (options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        command << "-k";
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        // No kerberos 
        break;
      }
      case Smb4KCustomOptions::UndefinedKerberos:
      {
        if (Smb4KSettings::useKerberos())
        {
          command << "-k";
        }
        else
        {
          // Do nothing
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    if (Smb4KSettings::useKerberos())
    {
      command << "-k";
    }
    else
    {
      // Do nothing
    }
  }
  
  // Use Winbind's ccache
  if (Smb4KSettings::useWinbindCCache())
  {
    command << "--use-ccache";
  }
  else
  {
    // Do nothing
  }
  
  // Port
  if (options && options->smbPort() != Smb4KSettings::remoteSMBPort())
  {
    command << "-p";
    command << QString("%1").arg(options->smbPort());
  }
  else
  {
    command << "-p";
    command << QString("%1").arg(Smb4KSettings::remoteSMBPort());
  }

  //
  // The process
  //
  m_process2 = new Smb4KProcess(this);
  m_process2->setOutputChannelMode(KProcess::SeparateChannels);
  m_process2->setProgram(command);

  if (Smb4KSettings::masterBrowsersRequireAuth() && !host.password().isEmpty())
  {
    m_process2->setEnv("PASSWD", host.password(), true);
  }
  else
  {
    m_process2->unsetEnv("PASSWD");
  }

  connect(m_process2, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcess2Finished(int,QProcess::ExitStatus)));

  m_process2->start();
}


void Smb4KQueryMasterJob::processErrors(const QString& stdErr)
{
  QStringList stdErrList = stdErr.split('\n', QString::SkipEmptyParts);
  
  if (!stdErrList.isEmpty())
  {
    QMutableStringListIterator it(stdErrList);
    
    while (it.hasNext())
    {
      // Remove unimportant warnings and irrelevant error
      // messages.
      QString line = it.next().trimmed();
      
      if (line.startsWith(QLatin1String("Ignoring unknown parameter")))
      {
        it.remove();
      }
      else if (line.startsWith(QLatin1String("messaging_tdb_init failed:")))
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
    }
    
    // Show the error message, if necessary.
    if (!stdErrList.filter("The username or password was not correct.").isEmpty() ||
        !stdErrList.filter("NT_STATUS_ACCOUNT_DISABLED").isEmpty() /* AD error */ ||
        !stdErrList.filter("NT_STATUS_ACCESS_DENIED").isEmpty() ||
        !stdErrList.filter("NT_STATUS_LOGON_FAILURE").isEmpty())
    {
      if (m_master_browser.isEmpty())
      {
        Q_FOREACH(const QString &line, stdErrList)
        {
          if (line.contains("Connecting to host="))
          {
            m_master_browser = line.section('=', 1, 1).trimmed();
            break;
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        // Do nothing
      }

      emit authError(this);
    }
    else if (!stdErrList.filter("NT_STATUS").isEmpty())
    {
      Smb4KNotification::retrievingDomainsFailed(stdErrList.join('\n'));
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


void Smb4KQueryMasterJob::processMasterBrowser(const QString& stdOut)
{
  startProcess2(stdOut.trimmed());
}


void Smb4KQueryMasterJob::processWorkgroups(const QString &stdOut)
{
  QStringList stdOutList = stdOut.split('\n', QString::SkipEmptyParts);

  if (!stdOutList.isEmpty())
  {
    Smb4KWorkgroup *workgroup = new Smb4KWorkgroup();

    Q_FOREACH(const QString &line, stdOutList)
    {
      if (line.trimmed().startsWith(QLatin1String("Enumerating")))
      {
        continue;
      }
      else if (line.trimmed().startsWith(QLatin1String("Domain name")))
      {
        continue;
      }
      else if (line.trimmed().startsWith(QLatin1String("-------------")))
      {
        continue;
      }
      else if (line.trimmed().isEmpty())
      {
        continue;
      }
      else
      {
        // This is the workgroup and master entry. Process it.
        workgroup->setWorkgroupName(line.section("   ", 0, 0).trimmed());
        workgroup->setMasterBrowserName(line.section("   ", 1, -1).trimmed());

        m_workgroups_list << new Smb4KWorkgroup(*workgroup);

        delete workgroup;
        workgroup = new Smb4KWorkgroup();
        continue;
      }
    }

    delete workgroup;
  }
  else
  {
    // Do nothing
  }

  emit workgroups(m_workgroups_list);
}


void Smb4KQueryMasterJob::slotStartLookup()
{
  // Start to lookup the IP address of the master browser that is 
  // to be queried.
  startProcess1();
}


void Smb4KQueryMasterJob::slotProcess1Finished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
    case QProcess::CrashExit:
    {
      if (!m_process1->isAborted())
      {
        Smb4KNotification::processError(m_process1->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // Process output
      QString stdOut = QString::fromUtf8(m_process1->readAllStandardOutput(), -1).trimmed();

      // Process errors
      QString stdErr = QString::fromUtf8(m_process1->readAllStandardError(), -1).trimmed();
      
      if (stdOut.trimmed().isEmpty() && !stdErr.trimmed().isEmpty())
      {
        processErrors(stdErr);
      }
      else
      {
        processMasterBrowser(stdOut);
      }
      
      break;
    }
  }
}


void Smb4KQueryMasterJob::slotProcess2Finished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
    case QProcess::CrashExit:
    {
      if (!m_process2->isAborted())
      {
        Smb4KNotification::processError(m_process2->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // Process output
      QString stdOut = QString::fromUtf8(m_process2->readAllStandardOutput(), -1).trimmed();

      // Process errors
      QString stdErr = QString::fromUtf8(m_process2->readAllStandardError(), -1).trimmed();
      
      if (stdOut.trimmed().isEmpty() && !stdErr.trimmed().isEmpty())
      {
        processErrors(stdErr);
      }
      else
      {
        processWorkgroups(stdOut);
      }
      
      break;
    }
  }

  emitResult();
  emit finished();
}



Smb4KLookupDomainMembersJob::Smb4KLookupDomainMembersJob(QObject *parent) : KJob(parent),
  m_started(false), m_parent_widget(0), m_process(0)
{
}


Smb4KLookupDomainMembersJob::~Smb4KLookupDomainMembersJob()
{
  delete m_workgroup;

  // Do not delete m_master_browser here, because it is a pointer
  // to an entry in Smb4KGlobal::hostsList().
  
  while (!m_hosts_list.isEmpty())
  {
    delete m_hosts_list.takeFirst();
  }
}


void Smb4KLookupDomainMembersJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartLookup()));
}


void Smb4KLookupDomainMembersJob::setupLookup(Smb4KWorkgroup *workgroup, QWidget *parent)
{
  Q_ASSERT(workgroup);
  m_workgroup = new Smb4KWorkgroup(*workgroup);
  m_parent_widget = parent;
}


bool Smb4KLookupDomainMembersJob::doKill()
{
  if (m_process && m_process->state() != KProcess::NotRunning)
  {
    m_process->abort();
  }
  else
  {
    // Do nothing
  }

  return KJob::doKill();
}


void Smb4KLookupDomainMembersJob::processErrors(const QString& stdErr)
{
  QStringList stdErrList = stdErr.split('\n', QString::SkipEmptyParts);
  
  if (!stdErrList.isEmpty())
  {
    QMutableStringListIterator it(stdErrList);
    
    while (it.hasNext())
    {
      // Remove unimportant warnings and irrelevant error
      // messages.
      QString line = it.next().trimmed();
      
      if (line.startsWith(QLatin1String("Ignoring unknown parameter")))
      {
        it.remove();
      }
      else if (line.contains(QLatin1String("tdb_transaction_recover:")))
      {
        it.remove();
      }
      else if (line.contains(QLatin1String("tdb_log")))
      {
        it.remove();
      }
      else if (line.contains(QLatin1String("packet check failed")))
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
    }
      
    if (!stdErrList.filter("The username or password was not correct.").isEmpty() ||
        !stdErrList.filter("NT_STATUS_ACCOUNT_DISABLED").isEmpty() /* AD error */ ||
        !stdErrList.filter("NT_STATUS_ACCESS_DENIED").isEmpty() ||
        !stdErrList.filter("NT_STATUS_LOGON_FAILURE").isEmpty())
    {
      emit authError(this);
    }
    else
    {
      // Notify the user that an error occurred.
      Smb4KNotification::retrievingHostsFailed(m_workgroup, stdErrList.join('\n'));
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KLookupDomainMembersJob::processHosts(const QString &stdOut)
{
  QStringList stdOutList = stdOut.split('\n', QString::SkipEmptyParts);
  
  if (!stdOutList.isEmpty())
  {
    Q_FOREACH(const QString &line, stdOutList)
    {
      if (line.trimmed().startsWith(QLatin1String("Enumerating")))
      {
        continue;
      }
      else if (line.trimmed().startsWith(QLatin1String("Server name")))
      {
        continue;
      }
      else if (line.trimmed().startsWith(QLatin1String("-------------")))
      {
        continue;
      }
      else
      {
        // Omit host names that contain spaces since QUrl cannot handle them.
        // And, they are wrong, anyway.
        if (!line.section("   ", 0, 0).trimmed().contains(" "))
        {
          Smb4KHost *host = new Smb4KHost();
          host->setHostName(line.section("   ", 0, 0).trimmed());
          host->setWorkgroupName(m_workgroup->workgroupName());
          host->setComment(line.section("   ", 1, -1).trimmed());
          
          if (QString::compare(host->hostName(), m_workgroup->masterBrowserName()) == 0)
          {
            host->setLogin(m_master_browser->login());
            host->setPassword(m_master_browser->password());
            host->setIsMasterBrowser(true);

            if (m_workgroup->hasMasterBrowserIP())
            {
              host->setIP(m_workgroup->masterBrowserIP());
            }
            else
            {
              // Do nothing
            }
          }
          else
          {
            host->setIsMasterBrowser(false);
          }
          
          m_hosts_list << host;
        }
        else
        {
          qDebug() << "This host name contains a space. I cannot handle this...";
        }
      }
    }
  }
  else
  {
    // Do nothing
  }

  emit hosts(m_workgroup, m_hosts_list);
}


void Smb4KLookupDomainMembersJob::slotStartLookup()
{
  //
  // Find shell commands
  //
  QString net = QStandardPaths::findExecutable("net");

  if (net.isEmpty())
  {
    Smb4KNotification::commandNotFound("net");
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }
  
  //
  // Get the master browser of the domain or workgroup
  //
  Smb4KHost *masterBrowser = findHost(m_workgroup->masterBrowserName(), m_workgroup->workgroupName());
  
  if (!masterBrowser)
  {
    // The master browser could not be determined. End the
    // job here and emit the signal nontheless.
    emit hosts(m_workgroup, m_hosts_list);
    emitResult();
    emit finished(m_workgroup);
    return;
  }
  else
  {
    // Do nothing
  }
  
  m_master_browser = masterBrowser;
  
  //
  // Authentication information, if needed
  //
  if (Smb4KSettings::masterBrowsersRequireAuth())
  {
    Smb4KWalletManager::self()->readAuthInfo(m_master_browser);
  }
  else
  {
    // Do nothing
  }
  
  //
  // Global Samba and custom options
  //
  QMap<QString,QString> samba_options = globalSambaOptions();
  Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions(m_master_browser);  
  
  //
  // The command
  //
  QStringList command;
  command << net;
  command << "rap";
  command << "server";
  command << "domain";
  
  // IP address or server name
  if (m_workgroup->hasMasterBrowserIP())
  {
    command << "-I";
    command << m_workgroup->masterBrowserIP();
  }
  else
  {
    // Do nothing
  }
  
  // The name of the remote master browser
  command << "-S";
  command << m_workgroup->masterBrowserName();
  
  // Workgroup of the remote master browser that is to be 
  // queried for the workgroup/domain members.
  command << "-w";
  command << m_workgroup->workgroupName();
  
  // User name and password if needed
  if (Smb4KSettings::masterBrowsersRequireAuth() && !m_master_browser->login().isEmpty())
  {
    command << "-U";
    command << m_master_browser->login();
  }
  else
  {
    // Do *not* change this to "-U guest%". This won't work.
    command << "-U";
    command << "%";
  }
  
  // The user's workgroup/domain name
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), samba_options["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }
  
  // The user's NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), samba_options["netbios name"]) != 0)
  {
    command << "-n";
    command << KShell::quoteArg(Smb4KSettings::netBIOSName());
  }
  else
  {
    // Do nothing
  }
  
  // Machine account
  if (Smb4KSettings::machineAccount())
  {
    command << "-P";
  }
  else
  {
    // Do nothing
  }
  
  // Encrypt SMB transport
  if (Smb4KSettings::encryptSMBTransport())
  {
    command << "-e";
  }
  else
  {
    // Do nothing
  }
  
  // Use Kerberos
  if (options)
  {
    switch (options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        command << "-k";
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        // No kerberos 
        break;
      }
      case Smb4KCustomOptions::UndefinedKerberos:
      {
        if (Smb4KSettings::useKerberos())
        {
          command << "-k";
        }
        else
        {
          // Do nothing
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    if (Smb4KSettings::useKerberos())
    {
      command << "-k";
    }
    else
    {
      // Do nothing
    }
  }
  
  // Use Winbind's ccache
  if (Smb4KSettings::useWinbindCCache())
  {
    command << "--use-ccache";
  }
  else
  {
    // Do nothing
  }
  
  // Port
  if (options && options->smbPort() != Smb4KSettings::remoteSMBPort())
  {
    command << "-p";
    command << QString("%1").arg(options->smbPort());
  }
  else
  {
    command << "-p";
    command << QString("%1").arg(Smb4KSettings::remoteSMBPort());
  }
  
  //
  // The process
  //
  m_process = new Smb4KProcess(this);
  m_process->setOutputChannelMode(KProcess::SeparateChannels);
  m_process->setProgram(command);

  if (Smb4KSettings::self()->masterBrowsersRequireAuth() && !m_master_browser->password().isEmpty())
  {
    m_process->setEnv("PASSWD", m_master_browser->password(), true);
  }
  else
  {
    m_process->unsetEnv("PASSWD");
  }
  
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

  emit aboutToStart(m_workgroup);
    
  m_process->start();
}


void Smb4KLookupDomainMembersJob::slotProcessFinished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
    case QProcess::CrashExit:
    {
      if (!m_process->isAborted())
      {
        Smb4KNotification::processError(m_process->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // Process output
      QString stdOut = QString::fromUtf8(m_process->readAllStandardOutput(), -1).trimmed();

      // Process errors
      QString stdErr = QString::fromUtf8(m_process->readAllStandardError(), -1).trimmed();
      
      if (stdOut.trimmed().isEmpty() && !stdErr.trimmed().isEmpty())
      {
        processErrors(stdErr);
      }
      else
      {
        processHosts(stdOut);
      }
      
      break;
    }
  }

  emitResult();
  emit finished(m_workgroup);
}



Smb4KLookupSharesJob::Smb4KLookupSharesJob(QObject *parent) : KJob(parent),
  m_started(false), m_parent_widget(0), m_process(0)
{
}


Smb4KLookupSharesJob::~Smb4KLookupSharesJob()
{
  delete m_host;

  while (!m_shares_list.isEmpty())
  {
    delete m_shares_list.takeFirst();
  }
}


void Smb4KLookupSharesJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartLookup()));
}


void Smb4KLookupSharesJob::setupLookup(Smb4KHost *host, QWidget *parent)
{
  Q_ASSERT(host);
  m_host = new Smb4KHost(*host);
  m_parent_widget = parent;
}


bool Smb4KLookupSharesJob::doKill()
{
  if (m_process && m_process->state() != KProcess::NotRunning)
  {
    m_process->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KLookupSharesJob::processErrors(const QString& stdErr)
{
  QStringList stdErrList = stdErr.split('\n', QString::SkipEmptyParts);
  
  if (!stdErrList.isEmpty())
  {
    QMutableStringListIterator it(stdErrList);
    
    while (it.hasNext())
    {
      // Remove unimportant warnings and irrelevant error
      // messages.
      QString line = it.next().trimmed();

      if (line.startsWith(QLatin1String("Ignoring unknown parameter")))
      {
        it.remove();
      }
      else if (line.contains("creating lame", Qt::CaseSensitive))
      {
        it.remove();
      }
      else if (line.contains("could not obtain sid for domain", Qt::CaseSensitive))
      {
        it.remove();
      }
      else
      {
        // Do nothing
      }
    }
    
    if (!stdErrList.filter("The username or password was not correct.").isEmpty() ||
        !stdErrList.filter("NT_STATUS_ACCOUNT_DISABLED").isEmpty() /* AD error */ ||
        !stdErrList.filter("NT_STATUS_ACCESS_DENIED").isEmpty() ||
        !stdErrList.filter("NT_STATUS_LOGON_FAILURE").isEmpty())
    {
      emit authError(this);
    }
    else
    {
      Smb4KNotification::retrievingSharesFailed(m_host, stdErr);
    }
    
  }
  else
  {
    // Do nothing
  }
}


void Smb4KLookupSharesJob::processShares(const QString &stdOut)
{
  QStringList stdOutList = stdOut.split('\n', QString::SkipEmptyParts);
  
  if (!stdOutList.isEmpty())
  {
    Q_FOREACH(const QString &line, stdOutList)
    {
      if (line.trimmed().startsWith(QLatin1String("Enumerating")))
      {
        continue;
      }
      else if (line.trimmed().startsWith(QLatin1String("Share name")))
      {
        continue;
      }
      else if (line.trimmed().startsWith(QLatin1String("----------")))
      {
        continue;
      }
      else if (line.contains(" Disk     ", Qt::CaseSensitive) /* line has comment */ ||
               (!line.contains(" Disk     ", Qt::CaseSensitive) &&
                line.trimmed().endsWith(QLatin1String(" Disk"), Qt::CaseSensitive) /* line has no comment */))
      {
        Smb4KShare *share = new Smb4KShare();
        
        if (!line.trimmed().endsWith(QLatin1String(" Disk"), Qt::CaseSensitive))
        {
          share->setShareName(line.section(" Disk     ", 0, 0).trimmed());
          share->setComment(line.section(" Disk     ", 1, 1).trimmed());
        }
        else
        {
          share->setShareName(line.section(" Disk", 0, 0).trimmed());
          share->setComment("");
        }
            
        share->setHostName(m_host->hostName());
        share->setWorkgroupName(m_host->workgroupName());
        share->setTypeString("Disk");
        share->setLogin(m_host->login());
        share->setPassword(m_host->password());

        if (m_host->hasIP())
        {
          share->setHostIP(m_host->ip());
        }
        else
        {
          // Do nothing
        }

        m_shares_list << share;
      }
      else if (line.contains(" IPC      ", Qt::CaseSensitive) /* line has comment */ ||
               (!line.contains(" IPC      ", Qt::CaseSensitive) &&
                line.trimmed().endsWith(QLatin1String(" IPC"), Qt::CaseSensitive) /* line has no comment */))
      {
        Smb4KShare *share = new Smb4KShare();
        
        if (!line.trimmed().endsWith(QLatin1String(" IPC"), Qt::CaseSensitive))
        {
          share->setShareName(line.section(" IPC      ", 0, 0).trimmed());
          share->setComment(line.section(" IPC      ", 1, 1).trimmed());
        }
        else
        {
          share->setShareName(line.section(" IPC", 0, 0).trimmed());
          share->setComment("");
        }
            
        share->setHostName(m_host->hostName());
        share->setWorkgroupName(m_host->workgroupName());
        share->setTypeString("IPC");
        share->setLogin(m_host->login());
        share->setPassword(m_host->password());

        if (m_host->hasIP())
        {
          share->setHostIP(m_host->ip());
        }
        else
        {
          // Do nothing
        }

        m_shares_list << share;
      }      
      else if (line.contains(" Print    ", Qt::CaseSensitive) /* line has comment */ ||
               (!line.contains(" Print    ", Qt::CaseSensitive) &&
                line.trimmed().endsWith(QLatin1String(" Print"), Qt::CaseSensitive) /* line has no comment */))
      {
        Smb4KShare *share = new Smb4KShare();
        
        if (!line.trimmed().endsWith(QLatin1String(" Print"), Qt::CaseSensitive))
        {
          share->setShareName(line.section(" Print    ", 0, 0).trimmed());
          share->setComment(line.section(" Print    ", 1, 1).trimmed());
        }
        else
        {
          share->setShareName(line.section(" Print", 0, 0).trimmed());
          share->setComment("");
        }
            
        share->setHostName(m_host->hostName());
        share->setWorkgroupName(m_host->workgroupName());
        share->setTypeString("Printer");
        share->setLogin(m_host->login());
        share->setPassword(m_host->password());

        if (m_host->hasIP())
        {
          share->setHostIP(m_host->ip());
        }
        else
        {
          // Do nothing
        }

        m_shares_list << share;
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }

  emit shares(m_host, m_shares_list);
}


void Smb4KLookupSharesJob::slotStartLookup()
{
  //
  // Find shell commands
  //
  QString net = QStandardPaths::findExecutable("net");

  if (net.isEmpty())
  {
    Smb4KNotification::commandNotFound("net");
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }
  
  //
  // Authentication information.
  //
  Smb4KWalletManager::self()->readAuthInfo(m_host);
  
  //
  // Global Samba and custom options
  //
  QMap<QString,QString> samba_options = globalSambaOptions();
  Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions(m_host);
  
  //
  // The command
  //
  QStringList command;
  command << net;
  command << "rpc";
  command << "share";
  command << "list";
  
  // Long output. We need this, because we want to know the type and
  // the comment, too.
  command << "-l";
  
  // Port
  // If a port was defined for the host via Smb4KHost::port(), it will 
  // overwrite the other options.
  if (m_host->port() != -1)
  {
    command << "-p";
    command << QString("%1").arg(m_host->port());
  }
  else
  {
    if (options && options->smbPort() != Smb4KSettings::remoteSMBPort())
    {
      command << "-p";
      command << QString("%1").arg(options->smbPort());
    }
    else
    {
      command << "-p";
      command << QString("%1").arg(Smb4KSettings::remoteSMBPort());
    }
  }
  
  // Remote domain/workgroup name
  command << "-w";
  command << m_host->workgroupName();
  
  // Remote host name
  command << "-S";
  command << m_host->hostName();

  // IP address
  if (m_host->hasIP())
  {
    command << "-I";
    command << m_host->ip();
  }
  else
  {
    // Do nothing
  }
  
  // Authentication data
  if (!m_host->login().isEmpty())
  {
    command << "-U";
    command << m_host->login();
  }
  else
  {
    // Under some circumstances you need under Windows the 'guest'
    // account to be able to retrieve the list of shared resources.
    command << "-U";
    command << "guest%";
  }  

  // The user's workgroup/domain name
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), samba_options["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }
  
  // The user's NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), samba_options["netbios name"]) != 0)
  {
    command << "-n";
    command << KShell::quoteArg(Smb4KSettings::netBIOSName());
  }
  else
  {
    // Do nothing
  }
  
  // Machine account
  if (Smb4KSettings::machineAccount())
  {
    command << "-P";
  }
  else
  {
    // Do nothing
  }
  
  // Encrypt SMB transport
  if (Smb4KSettings::encryptSMBTransport())
  {
    command << "-e";
  }
  else
  {
    // Do nothing
  }
  
  // Use Kerberos
  if (options)
  {
    switch (options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        command << "-k";
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        // No kerberos 
        break;
      }
      case Smb4KCustomOptions::UndefinedKerberos:
      {
        if (Smb4KSettings::useKerberos())
        {
          command << "-k";
        }
        else
        {
          // Do nothing
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    if (Smb4KSettings::useKerberos())
    {
      command << "-k";
    }
    else
    {
      // Do nothing
    }
  }
  
  // Use Winbind's ccache
  if (Smb4KSettings::useWinbindCCache())
  {
    command << "--use-ccache";
  }
  else
  {
    // Do nothing
  }
 
  m_process = new Smb4KProcess(this);
  m_process->setOutputChannelMode(KProcess::SeparateChannels);
  m_process->setProgram(command);
  
  if (!m_host->password().isEmpty())
  {
    m_process->setEnv("PASSWD", m_host->password(), true);
  }
  else
  {
    m_process->unsetEnv("PASSWD");
  }
  
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

  emit aboutToStart(m_host);

  m_process->start();
}


void Smb4KLookupSharesJob::slotProcessFinished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
    case QProcess::CrashExit:
    {
      if (!m_process->isAborted())
      {
        Smb4KNotification::processError(m_process->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      // Process output
      QString stdOut = QString::fromUtf8(m_process->readAllStandardOutput(), -1).trimmed();

      // Process errors
      QString stdErr = QString::fromUtf8(m_process->readAllStandardError(), -1).trimmed();
      
      if (stdOut.trimmed().isEmpty() && !stdErr.trimmed().isEmpty())
      {
        processErrors(stdErr);
      }
      else
      {
        processShares(stdOut);
      }
      
      break;
    }
  }

  emitResult();
  emit finished(m_host);
}



Smb4KLookupIPAddressJob::Smb4KLookupIPAddressJob(QObject *parent)
: KJob(parent), m_started(false), m_host(0), m_parent_widget(0), m_process(0)
{
}


Smb4KLookupIPAddressJob::~Smb4KLookupIPAddressJob()
{
  delete m_host;
}


void Smb4KLookupIPAddressJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartLookup()));
}


void Smb4KLookupIPAddressJob::setupLookup(Smb4KHost *host, QWidget *parent)
{
  Q_ASSERT(host);
  m_host = new Smb4KHost(*host);
  m_parent_widget = parent;
}


bool Smb4KLookupIPAddressJob::doKill()
{
  if (m_process && m_process->state() != KProcess::NotRunning)
  {
    m_process->abort();
  }
  else
  {
    // Do nothing
  }

  return KJob::doKill();
}


void Smb4KLookupIPAddressJob::useNmblookup(QStringList &command)
{
  //
  // Find shell command
  //
  QString nmblookup = QStandardPaths::findExecutable("nmblookup");

  if (nmblookup.isEmpty())
  {
    Smb4KNotification::commandNotFound("nmblookup");
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }

  //
  // Global Samba options
  //
  QMap<QString,QString> samba_options = globalSambaOptions();

  //
  // The command
  //
  command << nmblookup;

  // Domain
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), samba_options["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }

  // NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), samba_options["netbios name"]) != 0)
  {
    command << "-n";
    command << Smb4KSettings::netBIOSName();
  }
  else
  {
    // Do nothing
  }

  // NetBIOS scope
  if (!Smb4KSettings::netBIOSScope().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSScope(), samba_options["netbios scope"]) != 0)
  {
    command << "-i";
    command << Smb4KSettings::netBIOSScope();
  }
  else
  {
    // Do nothing
  }

  // Socket options
  if (!Smb4KSettings::socketOptions().isEmpty() &&
      QString::compare(Smb4KSettings::socketOptions(), samba_options["socket options"]) != 0)
  {
    command << "-O";
    command << Smb4KSettings::socketOptions();
  }
  else
  {
    // Do nothing
  }

  // Port 137
  if (Smb4KSettings::usePort137())
  {
    command << "-r";
  }
  else
  {
    // Do nothing
  }

  // Broadcast address
  QHostAddress address(Smb4KSettings::broadcastAddress());

  if (!Smb4KSettings::broadcastAddress().isEmpty() &&
      address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
  {
    command << "-B";
    command << Smb4KSettings::broadcastAddress();
  }
  else
  {
    // Do nothing
  }

  // We do not use the WINS server here, because it emerged that using
  // the WINS server is sometimes not very reliable. 
  if (!winsServer().isEmpty())
  {
    command << "-R";
    command << "-U";
    command << winsServer();
  }
  else
  {
    // Do nothing
  }

  command << "--";
  command << m_host->hostName();
}


void Smb4KLookupIPAddressJob::useNet(QStringList &command)
{
  //
  // The shell command
  //
  QString net = QStandardPaths::findExecutable("net");

  if (net.isEmpty())
  {
    Smb4KNotification::commandNotFound("net");
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }
  
  //
  // Global Samba and custom options
  // 
  QMap<QString,QString> samba_options = globalSambaOptions();
  Smb4KCustomOptions *options = Smb4KCustomOptionsManager::self()->findOptions(m_host);
  
  //
  // The command
  //
  command << net;
  command << "lookup";
  command << "host";
  command << m_host->hostName();
  
  // The user's workgroup/domain name
  if (!Smb4KSettings::domainName().isEmpty() &&
      QString::compare(Smb4KSettings::domainName(), samba_options["workgroup"]) != 0)
  {
    command << "-W";
    command << Smb4KSettings::domainName();
  }
  else
  {
    // Do nothing
  }

  // The user's NetBIOS name
  if (!Smb4KSettings::netBIOSName().isEmpty() &&
      QString::compare(Smb4KSettings::netBIOSName(), samba_options["netbios name"]) != 0)
  {
    command << "-n";
    command << Smb4KSettings::netBIOSName();
  }
  else
  {
    // Do nothing
  }

  // Machine account
  if (Smb4KSettings::machineAccount())
  {
    command << "-P";
  }
  else
  {
    // Do nothing
  }
  
  // Encrypt SMB transport
  if (Smb4KSettings::encryptSMBTransport())
  {
    command << "-e";
  }
  else
  {
    // Do nothing
  }
  
  // Use Kerberos
  if (options)
  {
    switch (options->useKerberos())
    {
      case Smb4KCustomOptions::UseKerberos:
      {
        command << "-k";
        break;
      }
      case Smb4KCustomOptions::NoKerberos:
      {
        // No kerberos 
        break;
      }
      case Smb4KCustomOptions::UndefinedKerberos:
      {
        if (Smb4KSettings::useKerberos())
        {
          command << "-k";
        }
        else
        {
          // Do nothing
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    if (Smb4KSettings::useKerberos())
    {
      command << "-k";
    }
    else
    {
      // Do nothing
    }
  }
  
  // Winbind's ccache
  if (Smb4KSettings::useWinbindCCache())
  {
    command << "--use-ccache";
  }
  else
  {
    // Do nothing
  }
  
  // Port
  // If a port was defined for the host via Smb4KHost::port(), it will 
  // overwrite the other options.
  if (m_host->port() != -1)
  {
    command << "-p";
    command << QString("%1").arg(m_host->port());
  }
  else
  {
    if (options && options->smbPort() != Smb4KSettings::remoteSMBPort())
    {
      command << "-p";
      command << QString("%1").arg(options->smbPort());
    }
    else
    {
      command << "-p";
      command << QString("%1").arg(Smb4KSettings::remoteSMBPort());
    }
  }
}


void Smb4KLookupIPAddressJob::processNmblookupOutput()
{
  // Normally, there should only be one IP address. However, there might
  // be more than one. So, split the incoming data and use the first entry
  // as IP address (it's most likely the correct one). If there is no data,
  // set the IP address to an empty string.
  QStringList output = QString::fromUtf8(m_process->readAllStandardOutput(), -1).split('\n', QString::SkipEmptyParts);

  Q_FOREACH(const QString &line, output)
  {
    if (line.contains("<00>"))
    {
      QString ip_address = line.section(' ', 0, 0).trimmed();
      m_host->setIP(ip_address);
      break;
    }
    else
    {
      continue;
    }
  }

  emit ipAddress(m_host);
}


void Smb4KLookupIPAddressJob::processNetOutput()
{
  // There is only one IP address reported:
  QString output = QString::fromUtf8(m_process->readAllStandardOutput(), -1).trimmed();
  m_host->setIP(output);
  emit ipAddress(m_host);
}


void Smb4KLookupIPAddressJob::slotStartLookup()
{
  //
  // The command
  //
  QStringList command;
  
  switch (Smb4KSettings::lookupIPs())
  {
    case Smb4KSettings::EnumLookupIPs::nmblookup:
    {
      useNmblookup(command);
      break;
    }
    case Smb4KSettings::EnumLookupIPs::net:
    {
      useNet(command);
      break;
    }
    default:
    {
      break;
    }
  }
  
  //
  // The process
  //
  if (!command.isEmpty())
  {
    m_process = new Smb4KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->setProgram(command);
    
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
    
    m_process->start();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KLookupIPAddressJob::slotProcessFinished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
  switch (exitStatus)
  {
    case QProcess::CrashExit:
    {
      if (!m_process->isAborted())
      {
        Smb4KNotification::processError(m_process->error());
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      switch (Smb4KSettings::lookupIPs())
      {
        case Smb4KSettings::EnumLookupIPs::nmblookup:
        {
          processNmblookupOutput();
          break;
        }
        case Smb4KSettings::EnumLookupIPs::net:
        {
          processNetOutput();
          break;
        }
        default:
        {
          break;
        }
      }
      break;
    }
  }

  emitResult();
}

