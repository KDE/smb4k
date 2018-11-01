/***************************************************************************
    This class provides the interface to the libsmbclient library.
                             -------------------
    begin                : Sa Oct 20 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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
#include "smb4kclient.h"
#include "smb4kclient_p.h"
#include "smb4khardwareinterface.h"
#include "smb4ksettings.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QUdpSocket>
#include <QHostAddress>
#include <QTest>


Q_GLOBAL_STATIC(Smb4KClientStatic, p);

Smb4KClient::Smb4KClient(QObject* parent) 
: KCompositeJob(parent), d(new Smb4KClientPrivate)
{
}


Smb4KClient::~Smb4KClient()
{
}


Smb4KClient *Smb4KClient::self()
{
  return &p->instance;
}


void Smb4KClient::start()
{
  //
  // Check the network configurations
  //
  Smb4KHardwareInterface::self()->updateNetworkConfig();
 
  //
  // Connect to Smb4KHardwareInterface to be able to get the response
  // 
  connect(Smb4KHardwareInterface::self(), SIGNAL(networkConfigUpdated()), this, SLOT(slotStartJobs()));
}


bool Smb4KClient::isRunning()
{
  return hasSubjobs();
}


void Smb4KClient::abort()
{
  qDebug() << "FIXME";
}



void Smb4KClient::lookupDomains()
{
  //
  // Send Wakeup-On-LAN packages
  // 
  if (Smb4KSettings::enableWakeOnLAN())
  {
    QList<OptionsPtr> wakeOnLanEntries = Smb4KCustomOptionsManager::self()->wakeOnLanEntries();
    
    if (!wakeOnLanEntries.isEmpty())
    {
      NetworkItemPtr item = NetworkItemPtr(new Smb4KBasicNetworkItem());
//       emit aboutToStart(item, WakeUp);
      
      QUdpSocket *socket = new QUdpSocket(this);
      
      for (int i = 0; i < wakeOnLanEntries.size(); ++i)
      {
        if (wakeOnLanEntries.at(i)->wolSendBeforeNetworkScan())
        {
          QHostAddress addr;
          
          if (!wakeOnLanEntries.at(i)->ip().isEmpty())
          {
            addr.setAddress(wakeOnLanEntries.at(i)->ip());
          }
          else
          {
            addr.setAddress("255.255.255.255");
          }
          
          // Construct magic sequence
          QByteArray sequence;

          // 6 times 0xFF
          for (int j = 0; j < 6; ++j)
          {
            sequence.append(QChar(0xFF).toLatin1());
          }
          
          // 16 times the MAC address
          QStringList parts = wakeOnLanEntries.at(i)->macAddress().split(':', QString::SkipEmptyParts);
          
          for (int j = 0; j < 16; ++j)
          {
            for (int k = 0; k < parts.size(); ++k)
            {
              sequence.append(QChar(QString("0x%1").arg(parts.at(k)).toInt(0, 16)).toLatin1());
            }
          }
          
          socket->writeDatagram(sequence, addr, 9);
        }
        else
        {
          // Do nothing
        }
      }
      
      delete socket;
      
      // Wait the defined time
      int stop = 1000 * Smb4KSettings::wakeOnLANWaitingTime() / 250;
      int i = 0;
      
      while (i++ < stop)
      {
        QTest::qWait(250);
      }
      
//       emit finished(item, WakeUp);
      item.clear();
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
  
  //
  // Lookup domains
  //
  Smb4KClientJob *job = new Smb4KClientJob(this);
  job->setUrl(QUrl("smb://"));
  job->setType(Smb4KGlobal::Network);
  
  connect(job, SIGNAL(result(KJob*)), this, SLOT(slotJobFinished(KJob*)));
  
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::setOverrideCursor(Qt::BusyCursor);
  }
  else
  {
    // Do nothing
  }

  addSubjob(job);

  job->start();
}


void Smb4KClient::slotStartJobs()
{
  //
  // Disconnect from Smb4KHardwareInterface::networkConfigUpdated() signal,
  // otherwise we get unwanted periodic scanning.
  //
  disconnect(Smb4KHardwareInterface::self(), SIGNAL(networkConfigUpdated()), this, SLOT(slotStartJobs()));
  
  //
  // Lookup domains as the first step
  // 
  lookupDomains();
}


void Smb4KClient::slotJobFinished(KJob *job)
{
  removeSubjob(job);
  
  if (!hasSubjobs() && modifyCursor())
  {
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Do nothing
  }
}




