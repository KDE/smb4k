/***************************************************************************
    Private classes for the SMB client
                             -------------------
    begin                : So Oct 21 2018
    copyright            : (C) 2018-2019 by Alexander Reinholdt
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
#include "smb4kclient_p.h"
#include "smb4ksettings.h"
#include "smb4kwalletmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4knotification.h"

// System includes
#include <errno.h>
// #include <sys/types.h>
#include <sys/stat.h>
// #include <unistd.h>

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QWindow>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>
#include <QGroupBox>
#include <QDir>
#include <QSpinBox>
#include <QPrinter>
#include <QTemporaryDir>
#include <QTextDocument>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KConfigGui/KWindowConfig>
#include <KIconThemes/KIconLoader>
#include <KIOWidgets/KUrlComboBox>
#include <KIO/Global>
#include <KWidgetsAddons/KDualAction>
#include <KIOWidgets/KUrlRequester>
#include <KIOCore/KFileItem>

#define SMBC_DEBUG 0

using namespace Smb4KGlobal;


//
// Authentication function for libsmbclient
// 
static void get_auth_data_with_context_fn(SMBCCTX *context,
                                          const char *server, 
                                          const char *share,
                                          char *workgroup,
                                          int maxLenWorkgroup,
                                          char *username,
                                          int maxLenUsername,
                                          char *password,
                                          int maxLenPassword)
{
  if (context != nullptr)
  {
    Smb4KClientJob *job = static_cast<Smb4KClientJob *>(smbc_getOptionUserData(context));
    
    if (job)
    {
      job->get_auth_data_fn(server, share, workgroup, maxLenWorkgroup, username, maxLenUsername, password, maxLenPassword);
    }
  }
}


//
// Client job
// 
Smb4KClientJob::Smb4KClientJob(QObject* parent) 
: KJob(parent), m_process(Smb4KGlobal::NoProcess)
{
}


Smb4KClientJob::~Smb4KClientJob()
{
  //
  // Clear the list of workgroups
  //
  while (!m_workgroups.isEmpty())
  {
    m_workgroups.takeFirst().clear();
  }
  
  //
  // Clear the list of hosts
  // 
  while (!m_hosts.isEmpty())
  {
    m_hosts.takeFirst().clear();
  }
  
  //
  // Clear the list of shares
  //
  while (!m_shares.isEmpty())
  {
    m_shares.takeFirst().clear();
  }
  
  //
  // Clear the list of files and directories
  //
  while (!m_files.isEmpty())
  {
    m_files.takeFirst().clear();
  }
}


void Smb4KClientJob::start()
{
  QTimer::singleShot(50, this, SLOT(slotStartJob()));
}


void Smb4KClientJob::setNetworkItem(NetworkItemPtr item)
{
  m_item = item;
}


NetworkItemPtr Smb4KClientJob::networkItem() const
{
  return m_item;
}


void Smb4KClientJob::setProcess(Smb4KGlobal::Process process)
{
  m_process = process;
}


Smb4KGlobal::Process Smb4KClientJob::process() const
{
  return m_process;
}


void Smb4KClientJob::setPrintFileItem(const KFileItem& item)
{
  m_fileItem = item;
}


KFileItem Smb4KClientJob::printFileItem() const
{
  return m_fileItem;
}


void Smb4KClientJob::setPrintCopies(int copies)
{
  m_copies = copies;
}


int Smb4KClientJob::printCopies() const
{
  return m_copies;
}


void Smb4KClientJob::get_auth_data_fn(const char* server, const char* /*share*/, char* workgroup, int /*maxLenWorkgroup*/, char* username, int maxLenUsername, char* password, int maxLenPassword)
{
  //
  // Authentication
  // 
  switch (m_item->type())
  {
    case Network:
    {
      //
      // No authentication needed
      // 
      break;
    }
    case Workgroup:
    {
      //
      // Only request authentication data, if the master browsers require 
      // authentication data.
      // 
      if (Smb4KSettings::masterBrowsersRequireAuth())
      {
        if (QString::fromUtf8(server).toUpper() != QString::fromUtf8(workgroup).toUpper())
        {
          // 
          // This is the master browser. Create a host object for it.
          // 
          HostPtr h = HostPtr(new Smb4KHost());
          h->setWorkgroupName(QString::fromUtf8(workgroup));
          h->setHostName(QString::fromUtf8(server));
          
          // 
          // Get the authentication data
          // 
          Smb4KWalletManager::self()->readAuthInfo(h);
          
          // 
          // Copy the authentication data
          // 
          qstrncpy(username, h->login().toUtf8().data(), maxLenUsername);
          qstrncpy(password, h->password().toUtf8().data(), maxLenPassword);
        }
      }
      
      break;
    }
    case Host:
    {
      //
      // The host object
      // 
      HostPtr h = m_item.staticCast<Smb4KHost>();
      
      //
      // Get the authentication data
      //
      Smb4KWalletManager::self()->readAuthInfo(h);
        
      // 
      // Copy the authentication data
      // 
      qstrncpy(username, h->login().toUtf8().data(), maxLenUsername);
      qstrncpy(password, h->password().toUtf8().data(), maxLenPassword);
      
      break;
    }
    case Share:
    {
      //
      // The share object
      // 
      SharePtr s = m_item.staticCast<Smb4KShare>();
      
      //
      // Get the authentication data
      //
      Smb4KWalletManager::self()->readAuthInfo(s);
        
      // 
      // Copy the authentication data
      // 
      qstrncpy(username, s->login().toUtf8().data(), maxLenUsername);
      qstrncpy(password, s->password().toUtf8().data(), maxLenPassword);
      
      break;
    }
    case Directory:
    {
      //
      // The file object
      // 
      FilePtr f = m_item.staticCast<Smb4KFile>();
      
      //
      // Create a share object
      // 
      SharePtr s = SharePtr(new Smb4KShare());
      s->setWorkgroupName(f->workgroupName());
      s->setHostName(f->hostName());
      s->setShareName(f->shareName());
      s->setLogin(f->login());
      s->setPassword(f->password());
          
      //
      // Get the authentication data
      //
      Smb4KWalletManager::self()->readAuthInfo(s);
        
      // 
      // Copy the authentication data
      // 
      qstrncpy(username, s->login().toUtf8().data(), maxLenUsername);
      qstrncpy(password, s->password().toUtf8().data(), maxLenPassword);
      
      break;
    }
    default:
    {
      break;
    }
  }
}


QList<WorkgroupPtr> Smb4KClientJob::workgroups()
{
  return m_workgroups;
}


QList<HostPtr> Smb4KClientJob::hosts()
{
  return m_hosts;
}


QList<SharePtr> Smb4KClientJob::shares()
{
  return m_shares;
}


QList<FilePtr> Smb4KClientJob::files()
{
  return m_files;
}


QString Smb4KClientJob::workgroup()
{
  QString workgroup;
  
  switch (m_item->type())
  {
    case Network:
    {
      break;
    }
    case Workgroup:
    {
      workgroup = m_item->url().host().toUpper();
      break;
    }
    case Host:
    {
      workgroup = m_item.staticCast<Smb4KHost>()->workgroupName();
      break;
    }
    case Share:
    {
      workgroup = m_item.staticCast<Smb4KShare>()->workgroupName();
      break;
    }
    case Directory:
    case File:
    {
      workgroup = m_item.staticCast<Smb4KFile>()->workgroupName();
      break;
    }
    default:
    {
      break;
    }
  }
  
  return workgroup;
}


void Smb4KClientJob::doLookups()
{
  // 
  // Read the given URL
  // 
  int dirfd;
  struct smbc_dirent *dirp = nullptr;
  
  dirfd = smbc_opendir(m_item->url().toString().toUtf8().data());
  
  if (dirfd < 0)
  {
    int errorCode = errno;
    
    switch (errorCode)
    {
      case ENOMEM:
      {
        setError(OutOfMemoryError);
        setErrorText(i18n("Out of memory"));
        break;
      }
      case EACCES:
      {
        setError(AccessDeniedError);
        setErrorText(i18n("Permission denied"));
        break;
      }
      case EINVAL:
      {
        setError(InvalidUrlError);
        setErrorText(i18n("An invalid URL was passed"));
        break;
      }
      case ENOENT:
      {
        setError(NonExistentUrlError);
        setErrorText(i18n("The URL does not exist"));
        break;
      }
      case ENOTDIR:
      {
        setError(NoDirectoryError);
        setErrorText(i18n("Name is not a directory"));
        break;
      }
      case EPERM:
      {
        setError(NotPermittedError);
        // Is the error message correct? 
        setErrorText(i18n("Operation not permitted"));
        break;
      }
      case ENODEV:
      {
        setError(NotFoundError);
        setErrorText(i18n("The workgroup or server could not be found"));
        break;
      }
      default:
      {
        setError(UnknownError);
        setErrorText(i18n("Unknown error"));
      }
    }

    emitResult();
    return;
  }
  else
  {
    //
    // Get the entries of the "directory"
    // 
    while ((dirp = smbc_readdir(dirfd)) != nullptr)
    {
      switch (dirp->smbc_type)
      {
        case SMBC_WORKGROUP:
        {
          // 
          // Create a workgroup pointer
          // 
          WorkgroupPtr workgroup = WorkgroupPtr(new Smb4KWorkgroup());
          
          // 
          // Set the workgroup name
          // 
          QString workgroupName = QString::fromUtf8(dirp->name);
          workgroup->setWorkgroupName(workgroupName);
          
          // 
          // Set the master browser
          // 
          QString masterBrowserName = QString::fromUtf8(dirp->comment);
          workgroup->setMasterBrowserName(masterBrowserName);
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(masterBrowserName);
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip the
          // workgroup and delete the pointer.
          // 
          if (!address.isNull())
          {
            workgroup->setMasterBrowserIpAddress(address);
            m_workgroups << workgroup;
          }
          else
          {
            workgroup.clear();
          }
          
          break;
        }
        case SMBC_SERVER:
        {
          // 
          // Create a host pointer
          // 
          HostPtr host = HostPtr(new Smb4KHost());
          
          // 
          // Set the workgroup name
          // 
          host->setWorkgroupName(m_item->url().host());
          
          // 
          // Set the host name
          // 
          QString hostName = QString::fromUtf8(dirp->name);
          host->setHostName(hostName);
          
          // 
          // Set the comment
          // 
          QString comment = QString::fromUtf8(dirp->comment);
          host->setComment(comment);
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(hostName);
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            host->setIpAddress(address);
            m_hosts << host;
          }
          else
          {
            host.clear();
          } 
          
          break;
        }
        case SMBC_FILE_SHARE:
        {
          //
          // Create a share pointer
          // 
          SharePtr share = SharePtr(new Smb4KShare());
          
          //
          // Set the workgroup name
          // 
          share->setWorkgroupName(m_item.staticCast<Smb4KHost>()->workgroupName());
          
          //
          // Set the host name
          // 
          share->setHostName(m_item->url().host());
          
          //
          // Set the share name
          // 
          share->setShareName(QString::fromUtf8(dirp->name));
          
          // 
          // Set the comment
          // 
          share->setComment(QString::fromUtf8(dirp->comment));
          
          //
          // Set share type
          // 
          share->setShareType(FileShare);
          
          //
          // Set the authentication data
          // 
          share->setLogin(m_item->url().userName());
          share->setPassword(m_item->url().password());
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(m_item->url().host());
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            share->setHostIpAddress(address);
            m_shares << share;
          }
          else
          {
            share.clear();
          } 
          
          break;
        }
        case SMBC_PRINTER_SHARE:
        {
          //
          // Create a share pointer
          // 
          SharePtr share = SharePtr(new Smb4KShare());
          
          //
          // Set the workgroup name
          // 
          share->setWorkgroupName(m_item.staticCast<Smb4KHost>()->workgroupName());
          
          //
          // Set the host name
          // 
          share->setHostName(m_item->url().host());
          
          //
          // Set the share name
          // 
          share->setShareName(QString::fromUtf8(dirp->name));
          
          // 
          // Set the comment
          // 
          share->setComment(QString::fromUtf8(dirp->comment));
          
          //
          // Set share type
          // 
          share->setShareType(PrinterShare);
          
          //
          // Set the authentication data
          // 
          share->setLogin(m_item->url().userName());
          share->setPassword(m_item->url().password());
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(m_item->url().host());
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            share->setHostIpAddress(address);
            m_shares << share;
          }
          else
          {
            share.clear();
          } 
          
          break;
        }
        case SMBC_IPC_SHARE:
        {
          //
          // Create a share pointer
          // 
          SharePtr share = SharePtr(new Smb4KShare());
          
          //
          // Set the workgroup name
          // 
          share->setWorkgroupName(m_item.staticCast<Smb4KHost>()->workgroupName());
          
          //
          // Set the host name
          // 
          share->setHostName(m_item->url().host());
          
          //
          // Set the share name
          // 
          share->setShareName(QString::fromUtf8(dirp->name));
          
          // 
          // Set the comment
          // 
          share->setComment(QString::fromUtf8(dirp->comment));
          
          //
          // Set share type
          // 
          share->setShareType(IpcShare);
          
          //
          // Set the authentication data
          // 
          share->setLogin(m_item->url().userName());
          share->setPassword(m_item->url().password());
          
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(m_item->url().host());
          
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            share->setHostIpAddress(address);
            m_shares << share;
          }
          else
          {
            share.clear();
          } 
          
          break;
        }
        case SMBC_DIR:
        {
          //
          // Do not process '.' and '..' directories
          // 
          QString name = QString::fromUtf8(dirp->name);
          
          if (name != "." && name != "..")
          {
            //
            // Create the URL for the discovered item
            // 
            QUrl u = m_item->url();
            u.setPath(m_item->url().path()+QDir::separator()+QString::fromUtf8(dirp->name));
            
            //
            // We do not stat directories. Directly create the directory object
            // 
            FilePtr dir = FilePtr(new Smb4KFile(u, Directory));
            
            //
            // Set the workgroup name
            //
            dir->setWorkgroupName(m_item.staticCast<Smb4KShare>()->workgroupName());
            
            //
            // Set the authentication data
            // 
            dir->setLogin(m_item->url().userName());
            dir->setPassword(m_item->url().password());
            
            // 
            // Lookup IP address
            // 
            QHostAddress address = lookupIpAddress(m_item->url().host());
            
            // 
            // Process the IP address. 
            // If the address is null, the server most likely went offline. So, skip it
            // and delete the pointer.
            // 
            if (!address.isNull())
            {
              dir->setHostIpAddress(address);
              m_files << dir;
            }
            else
            {
              dir.clear();
            } 
          }
          
          break;
        }
        case SMBC_FILE:
        {
          //
          // Create the URL for the discovered item
          // 
          QUrl u = m_item->url();
          u.setPath(m_item->url().path()+QDir::separator()+QString::fromUtf8(dirp->name));
            
          //
          // Create the directory object
          // 
          FilePtr dir = FilePtr(new Smb4KFile(u, File));
            
          //
          // Set the workgroup name
          //
          dir->setWorkgroupName(m_item.staticCast<Smb4KShare>()->workgroupName());
          
          //
          // Stat the file
          //
          // FIXME
            
          //
          // Set the authentication data
          // 
          dir->setLogin(m_item->url().userName());
          dir->setPassword(m_item->url().password());
            
          // 
          // Lookup IP address
          // 
          QHostAddress address = lookupIpAddress(m_item->url().host());
            
          // 
          // Process the IP address. 
          // If the address is null, the server most likely went offline. So, skip it
          // and delete the pointer.
          // 
          if (!address.isNull())
          {
            dir->setHostIpAddress(address);
            m_files << dir;
          }
          else
          {
            dir.clear();
          } 
            
          break;
        }
        case SMBC_LINK:
        {
          qDebug() << "Processing links is not implemented.";
          qDebug() << dirp->name;
          qDebug() << dirp->comment;
          break;
        }
        default:
        {
          qDebug() << "Need to process network item " << dirp->name;
          break;
        }
      }
    }
    
    smbc_closedir(dirfd);
  }
}


void Smb4KClientJob::doPrinting()
{
  //
  // The URL that is to be printed
  // 
  QUrl fileUrl;
  
  //
  // Set the temporary directory
  //
  QTemporaryDir tempDir;
  
  //
  // Check if we can directly print the file
  // 
  if (m_fileItem.mimetype() == "application/postscript" || 
      m_fileItem.mimetype() == "application/pdf" ||
      m_fileItem.mimetype().startsWith(QLatin1String("image")))
  {
    //
    // Set the URL to the incoming file
    // 
    fileUrl = m_fileItem.url();
  } 
  else if (m_fileItem.mimetype() == "application/x-shellscript" ||
           m_fileItem.mimetype().startsWith(QLatin1String("text")) || 
           m_fileItem.mimetype().startsWith(QLatin1String("message")))
  {
    //
    // Set a printer object
    //
    QPrinter printer(QPrinter::HighResolution);
    printer.setCreator("Smb4K");
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(QString("%1/smb4k_print.pdf").arg(tempDir.path()));
    
    //
    // Open the file that is to be printed and read it
    // 
    QStringList contents;
      
    QFile file(m_fileItem.url().path());
      
    if (file.open(QFile::ReadOnly|QFile::Text))
    {
      QTextStream ts(&file);
      
      while (!ts.atEnd())
      {
        contents << ts.readLine();
      }
    }
    else
    {
      return;
    }
    
    //
    // Convert the file to PDF
    // 
    QTextDocument doc;
    
    if (m_fileItem.mimetype().endsWith(QLatin1String("html")))
    {
      doc.setHtml(contents.join(" "));
    }
    else
    {
      doc.setPlainText(contents.join("\n"));
    }
      
    doc.print(&printer);
    
    //
    // Set the URL to the converted file
    // 
    fileUrl.setUrl(printer.outputFileName());
    fileUrl.setScheme("file");
  }
  else
  {
    Smb4KNotification::mimetypeNotSupported(m_fileItem.mimetype());
    return;
  }
  
  //
  // Get the open function for the printer
  // 
  smbc_open_print_job_fn openPrinter = smbc_getFunctionOpenPrintJob(m_context);
  
  if (!openPrinter)
  {
    setError(OpenPrintJobError);
    setErrorText(i18n("The print job could not be set up (step 1)"));
    
    emitResult();
    return;
  }
  
  //
  // Open the printer for printing
  // 
  SMBCFILE *printer = openPrinter(m_context, m_item->url().toString().toUtf8().data());
  
  if (!printer)
  {
    int errorCode = errno;
    
    if (errorCode == EACCES)
    {
      setError(AccessDeniedError);
      setErrorText(i18n("Permission denied"));
    }
    else
    {
      setError(OpenPrintJobError);
      setErrorText(i18n("The print job could not be set up (step 2)"));
    }
    
    emitResult();
    return;
  }
  
  //
  // Open the file
  // 
  QFile file(fileUrl.path());
  
  if (!file.open(QFile::ReadOnly))
  {
    setError(FileAccessError);
    setErrorText(i18n("The file %1 could not be read", fileUrl.path()));
    
    emitResult();
    return;
  }
  
  //
  // Write X copies of the file to the printer
  // 
  char buffer[4096];
  qint64 bytes = 0;
  int copy = 0;
  
  while (copy < m_copies)
  {
    while ((bytes = file.read(buffer, sizeof(buffer))) > 0)
    {
      smbc_write_fn writeFile = smbc_getFunctionWrite(m_context);
      
      if (writeFile(m_context, printer, buffer, bytes) < 0)
      {
        setError(PrintFileError);
        setErrorText(i18n("The file %1 could not be printed to %2", fileUrl.path(), m_item.staticCast<Smb4KShare>()->displayString()));
        
        smbc_close_fn closePrinter = smbc_getFunctionClose(m_context);
        closePrinter(m_context, printer);
      }
    }
    
    copy++;
  }
  
  //
  // Close the printer
  // 
  smbc_close_fn closePrinter = smbc_getFunctionClose(m_context);
  closePrinter(m_context, printer);
}


QHostAddress Smb4KClientJob::lookupIpAddress(const QString& name)
{
  //
  // The IP address object
  // 
  QHostAddress ipAddress;
  
  // If the IP address is not to be determined for the local machine, we can use QHostInfo to
  // determine it. Otherwise we need to use QNetworkInterface for it.
  if (name.toUpper() == QHostInfo::localHostName().toUpper() || 
      name.toUpper() == globalSambaOptions()["netbios name"].toUpper() || 
      name.toUpper() == Smb4KSettings::netBIOSName().toUpper())
  {
    // FIXME: Do we need to honor 'interfaces' here?
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    
    // Get the IP address for the host. For the time being, prefer the 
    // IPv4 address over the IPv6 address.
    for (const QHostAddress &addr : addresses)
    {
      // We only use global addresses.
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
      if (addr.isGlobal())
#else
      if (!addr.isLoopback() && !addr.isMulticast())
#endif
      {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol)
        {
          ipAddress = addr;
          break;
        }
        else if (addr.protocol() == QAbstractSocket::IPv6Protocol)
        {
          // FIXME: Use the right address here.
          ipAddress = addr;
        }
      }
    }
  }
  else
  {
    // Get the IP address
    QHostInfo hostInfo = QHostInfo::fromName(name);
            
    if (hostInfo.error() == QHostInfo::NoError)
    {
      // Get the IP address for the host. For the time being, prefer the 
      // IPv4 address over the IPv6 address.
      for (const QHostAddress &addr : hostInfo.addresses())
      {
        // We only use global addresses.
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        if (addr.isGlobal())
#else
        if (!addr.isLoopback() && !addr.isMulticast())
#endif
        {
          if (addr.protocol() == QAbstractSocket::IPv4Protocol)
          {
            ipAddress = addr;
            break;
          }
          else if (addr.protocol() == QAbstractSocket::IPv6Protocol)
          {
            // FIXME: Use the right address here.
            ipAddress = addr;
          }
        }
      }
    }
  }
  
  return ipAddress;
}


void Smb4KClientJob::slotStartJob()
{
  // 
  // Allocate new context
  // 
  m_context = smbc_new_context();
  
  if (!m_context)
  {
    int errorCode = errno;
    
    switch (errorCode)
    {
      case ENOMEM:
      {
        setError(OutOfMemoryError);
        setErrorText(i18n("Out of memory"));
        break;
      }
      default:
      {
        setError(UnknownError);
        setErrorText(i18n("Unknown error"));
        break;
      }
    }
    
    emitResult();
    return;
  }
  
  //
  // Get the custom options
  // 
  OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(m_item);
  
  // 
  // Set debug level
  // 
  smbc_setDebug(m_context, SMBC_DEBUG);
  
  //
  // Set the NetBIOS name and the workgroup to make connections
  // 
  switch (m_item->type())
  {
    case Network:
    {
      // 
      // We do not know about the servers and the domains/workgroups
      // present. So, do not set anything and use the default.
      // 
      break;
    }
    case Workgroup:
    {
      //
      // Set the NetBIOS name of the master browser and the workgroup to be queried
      // 
      WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
      smbc_setNetbiosName(m_context, workgroup->masterBrowserName().toUtf8().data());
      smbc_setWorkgroup(m_context, workgroup->workgroupName().toUtf8().data());
      break;
    }
    case Host:
    {
      //
      // Set both the NetBIOS name of the server and the workgroup to be queried
      // 
      HostPtr host = m_item.staticCast<Smb4KHost>();
      smbc_setNetbiosName(m_context, host->hostName().toUtf8().data());
      smbc_setWorkgroup(m_context, host->workgroupName().toUtf8().data());
      break;
    }
    case Share:
    {
      //
      // Set both the NetBIOS name of the server and the workgroup to be queried
      // 
      SharePtr share = m_item.staticCast<Smb4KShare>();
      smbc_setNetbiosName(m_context, share->hostName().toUtf8().data());
      smbc_setWorkgroup(m_context, share->workgroupName().toUtf8().data());
      break;
    }
    case Directory:
    {
      //
      // Set both the NetBIOS name of the server and the workgroup to be queried
      // 
      FilePtr file = m_item.staticCast<Smb4KFile>();
      smbc_setNetbiosName(m_context, file->hostName().toUtf8().data());
      smbc_setWorkgroup(m_context, file->workgroupName().toUtf8().data());
      break;      
    }
    default:
    {
      break;
    }
  }

  //
  // Set the user for making the connection
  // 
  if (!m_item->url().userName().isEmpty())
  {
    smbc_setUser(m_context, m_item->url().userName().toUtf8().data());
  }
  
  //
  // Set the port
  // 
  if (options)
  {
    if (options->useSmbPort())
    {
      smbc_setPort(m_context, options->smbPort());
    }
    else
    {
      smbc_setPort(m_context, 0 /* use the default */);
    }
  }
  else
  {
    if (Smb4KSettings::useRemoteSmbPort())
    {
      smbc_setPort(m_context, Smb4KSettings::remoteSmbPort());
    }
    else
    {
      smbc_setPort(m_context, 0 /* use the default */);
    }
  }

  //
  // Set the user data (this class)
  // 
  smbc_setOptionUserData(m_context, this);
  
  //
  // Set number of master browsers to be used
  // 
  if (Smb4KSettings::largeNetworkNeighborhood())
  {
    smbc_setOptionBrowseMaxLmbCount(m_context, 3);
  }
  else
  {
    smbc_setOptionBrowseMaxLmbCount(m_context, 0 /* all master browsers */);
  }
  
  //
  // Set the encryption level
  // 
  if (Smb4KSettings::useEncryptionLevel())
  {
    switch (Smb4KSettings::encryptionLevel())
    {
      case Smb4KSettings::EnumEncryptionLevel::None:
      {
        smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_NONE);
        break;
      }
      case Smb4KSettings::EnumEncryptionLevel::Request:
      {
        smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_REQUEST);
        break;
      }
      case Smb4KSettings::EnumEncryptionLevel::Require:
      {
        smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_REQUIRE);
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  //
  // Set the usage of anonymous login 
  // 
  smbc_setOptionNoAutoAnonymousLogin(m_context, false);
  
  //
  // Set the usage of the winbind ccache
  // 
  smbc_setOptionUseCCache(m_context, Smb4KSettings::useWinbindCCache());
  
  //
  // Set usage of Kerberos
  // 
  if (options)
  {
    smbc_setOptionUseKerberos(m_context, options->useKerberos());
  }
  else
  {
    smbc_setOptionUseKerberos(m_context, Smb4KSettings::useKerberos());
  }
  
  smbc_setOptionFallbackAfterKerberos(m_context, 1);
  
  //
  // Set the channel for debug output
  // 
  smbc_setOptionDebugToStderr(m_context, 1);
  
  //
  // Set auth callback function
  // 
  smbc_setFunctionAuthDataWithContext(m_context, get_auth_data_with_context_fn);
  
  // 
  // Initialize context with the previously defined options
  // 
  if (!smbc_init_context(m_context))
  {
    int errorCode = errno;
    
    switch (errorCode)
    {
      case ENOMEM:
      {
        setError(OutOfMemoryError);
        setErrorText(i18n("Out of memory"));
        break;
      }
      case EBADF:
      {
        setError(NullContextError);
        setErrorText(i18n("NULL context given"));
        break;
      }
      case ENOENT:
      {
        setError(SmbConfError);
        setErrorText(i18n("The smb.conf file would not load"));
        break;
      }
      default:
      {
        setError(UnknownError);
        setErrorText(i18n("Unknown error"));
      }
    }

    smbc_free_context(m_context, 0);

    emitResult();
    return;
  }
  
  //
  // Set the new context
  // 
  (void)smbc_set_context(m_context);
  
  //
  // Process the given URL according to the passed process
  // 
  switch (m_process)
  {
    case LookupDomains:
    case LookupDomainMembers:
    case LookupShares:
    case LookupFiles:
    {
      doLookups();
      break;
    }
    case PrintFile:
    {
      doPrinting();
      break;
    }
    default:
    {
      break;
    }
  }
  
  //
  // Free the context
  // 
  smbc_free_context(m_context, 0);
  
  //
  // Emit the result
  // 
  emitResult();
}



Smb4KPreviewDialog::Smb4KPreviewDialog(const SharePtr& share, QWidget* parent) 
: QDialog(parent), m_share(share)
{
  // 
  // Dialog title
  // 
  setWindowTitle(i18n("Preview of %1", share->displayString()));
  
  //
  // Attributes
  // 
  setAttribute(Qt::WA_DeleteOnClose, true);
  
  //
  // Layout
  // 
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);
  
  //
  // The list widget
  // 
  QListWidget *listWidget = new QListWidget(this);
  listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotItemActivated(QListWidgetItem*)));
  
  layout->addWidget(listWidget, 0);
  
  //
  // Toolbar
  // Use QToolBar here with the settings suggested by the note provided in the 'Detailed Description'
  // section of KToolBar (https://api.kde.org/frameworks/kxmlgui/html/classKToolBar.html)
  // 
  QToolBar *toolBar = new QToolBar(this);
  toolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
  toolBar->setProperty("otherToolbar", true);
  
  //
  // Reload / cancel action
  // 
  KDualAction *reloadAction = new KDualAction(toolBar);
  reloadAction->setObjectName("reload_action");
  reloadAction->setInactiveText(i18n("Reload"));
  reloadAction->setInactiveIcon(KDE::icon("view-refresh"));
  reloadAction->setActiveText(i18n("Abort"));
  reloadAction->setActiveIcon(KDE::icon("process-stop"));
  reloadAction->setActive(false);
  reloadAction->setAutoToggle(false);
  
  connect(reloadAction, SIGNAL(toggled(bool)), this, SLOT(slotReloadActionTriggered()));
  
  toolBar->addAction(reloadAction);
  
  //
  // Up action
  // 
  QAction *upAction =toolBar->addAction(KDE::icon("go-up"), i18n("Up"), this, SLOT(slotUpActionTriggered()));
  upAction->setObjectName("up_action");
  upAction->setEnabled(false);
  
  toolBar->addSeparator();
  
  //
  // URL combo box
  // 
  KUrlComboBox *urlCombo = new KUrlComboBox(KUrlComboBox::Directories, toolBar);
  urlCombo->setEditable(false);
  toolBar->addWidget(urlCombo);
  connect(urlCombo, SIGNAL(urlActivated(QUrl)), this, SLOT(slotUrlActivated(QUrl)));
  
  layout->addWidget(toolBar, 0);
  
  //
  // Dialog button box
  // 
  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Horizontal);
  QPushButton *closeButton = buttonBox->addButton(QDialogButtonBox::Close);
  connect(closeButton, SIGNAL(clicked()), this, SLOT(slotClosingDialog()));
  
  layout->addWidget(buttonBox, 0);
  
  //
  // Set the minimum width
  // 
  setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);
  
  //
  // Set the dialog size
  // 
  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
  resize(windowHandle()->size()); // workaround for QTBUG-40584

  //
  // Start the preview
  // 
  m_currentItem = m_share;
  QTimer::singleShot(0, this, SLOT(slotInitializePreview()));
}


Smb4KPreviewDialog::~Smb4KPreviewDialog()
{
  //
  // Clear the share
  //
  m_share.clear();
  
  //
  // Clear the current item
  //
  m_currentItem.clear();
  
  //
  // Clear the listing
  // 
  while (!m_listing.isEmpty())
  {
    m_listing.takeFirst().clear();
  }
}


SharePtr Smb4KPreviewDialog::share() const
{
  return m_share;
}


void Smb4KPreviewDialog::slotClosingDialog()
{
  //
  // Save the dialog size
  // 
  KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  
  //
  // Emit the aboutToClose() signal
  // 
  emit aboutToClose(this);
  
  // 
  // Close the dialog
  // 
  accept();
}


void Smb4KPreviewDialog::slotReloadActionTriggered()
{
  KDualAction *reloadAction = findChild<KDualAction *>();
  
  if (reloadAction->isActive())
  {
    emit requestAbort();
  }
  else
  {
    emit requestPreview(m_currentItem);
  }
}


void Smb4KPreviewDialog::slotUpActionTriggered()
{
  //
  // Get the new URL
  // 
  QUrl u = KIO::upUrl(m_currentItem->url());
  
  //
  // Create a new network item object, if necessary and set the new current
  // item. Also, disable the "Up" action, if necessary.
  // 
  if (m_share->url().matches(u, QUrl::StripTrailingSlash))
  {
    findChild<QAction *>("up_action")->setEnabled(false);
    m_currentItem = m_share;
  }
  else if (m_share->url().path().length() < u.path().length())
  {
    FilePtr file = FilePtr(new Smb4KFile(u, Directory));
    file->setWorkgroupName(m_share->workgroupName());
    m_currentItem = file;
  }
  else
  {
    return;
  }
  
  //
  // Emit the requestPreview() signal
  // 
  emit requestPreview(m_currentItem);
}


void Smb4KPreviewDialog::slotUrlActivated(const QUrl &url)
{
  //
  // Get the full authentication information. This is needed, since the combo
  // box only returns sanitized URLs, i.e. without password, etc.
  // 
  QUrl u = url;
  u.setUserName(m_share->login());
  u.setPassword(m_share->password());
  
  //
  // Create a new network item object, if necessary and set the new current
  // item.
  // 
  if (m_share->url().matches(u, QUrl::StripTrailingSlash))
  {
    m_currentItem = m_share;
  }
  else
  {
    FilePtr file = FilePtr(new Smb4KFile(u, Directory));
    file->setWorkgroupName(m_share->workgroupName());
    m_currentItem = file;
  }
  
  //
  // Emit the requestPreview() signal
  // 
  emit requestPreview(m_currentItem);
}


void Smb4KPreviewDialog::slotItemActivated(QListWidgetItem *item)
{
  //
  // Only process the item if it represents a directory
  // 
  if (item && item->type() == Directory)
  {
    //
    // Find the file item, make it the current one and emit the requestPreview() 
    // signal.
    // 
    for (const FilePtr &f : m_listing)
    {
      if (item->data(Qt::UserRole).toUrl().matches(f->url(), QUrl::None))
      {
        m_currentItem = f;
        emit requestPreview(m_currentItem);
        break;
      }
    }
  }
}


void Smb4KPreviewDialog::slotInitializePreview()
{
  emit requestPreview(m_currentItem);
}


void Smb4KPreviewDialog::slotPreviewResults(const QList<FilePtr> &list)
{
  //
  // Only process data the belongs to this dialog
  // 
  if (m_share->workgroupName() == list.first()->workgroupName() && m_share->hostName() == list.first()->hostName() && 
      list.first()->url().path().startsWith(m_share->url().path()))
  {
    //
    // Clear the internal listing
    // 
    while (!m_listing.isEmpty())
    {
      m_listing.takeFirst().clear();
    }  
    
    //
    // Copy the list into the private variable
    //
    m_listing = list;
    
    //
    // Get the list widget
    // 
    QListWidget *listWidget = findChild<QListWidget *>();
    
    //
    // Clear the list widget
    //
    listWidget->clear();
    
    //
    // Insert the new listing
    // 
    if (listWidget)
    {
      for (const FilePtr &f : list)
      {
        QListWidgetItem *item = new QListWidgetItem(f->icon(), f->name(), listWidget, f->isDirectory() ? Directory : File);
        item->setData(Qt::UserRole, f->url());
      }
    }
    
    //
    // Sort the list widget
    // 
    listWidget->sortItems();
    
    //
    // Add the URL to the combo box and show it. Omit duplicates.
    // 
    KUrlComboBox *urlCombo = findChild<KUrlComboBox *>();
    QStringList urls = urlCombo->urls();
    urls << m_currentItem->url().toString();
    urlCombo->setUrls(urls);
    urlCombo->setUrl(m_currentItem->url());
    
    //
    // Enable / disable the "Up" action
    //
    findChild<QAction *>("up_action")->setEnabled(!m_share->url().matches(m_currentItem->url(), QUrl::StripTrailingSlash));
  }
}


void Smb4KPreviewDialog::slotAboutToStart(const NetworkItemPtr &item, int type)
{
  if (type == LookupFiles)
  {
    switch (item->type())
    {
      case Share:
      {
        SharePtr s = item.staticCast<Smb4KShare>();
        
        if (m_share->workgroupName() == s->workgroupName() && m_share->hostName() == s->hostName() && s->url().path().startsWith(m_share->url().path()))
        {
          KDualAction *reloadAction = findChild<KDualAction *>();
          reloadAction->setActive(true);
        }
        
        break;
      }
      case Directory:
      {
        FilePtr f = item.staticCast<Smb4KFile>();
        
        if (m_share->workgroupName() == f->workgroupName() && m_share->hostName() == f->hostName() && f->url().path().startsWith(m_share->url().path()))
        {
          KDualAction *reloadAction = findChild<KDualAction *>();
          reloadAction->setActive(true);
        }
        
        break;
      }
      default:
      {
        break;
      }
    }
  }
}


void Smb4KPreviewDialog::slotFinished(const NetworkItemPtr &item, int type)
{
  if (type == LookupFiles)
  {
    switch (item->type())
    {
      case Share:
      {
        SharePtr s = item.staticCast<Smb4KShare>();
        
        if (m_share->workgroupName() == s->workgroupName() && m_share->hostName() == s->hostName() && s->url().path().startsWith(m_share->url().path()))
        {
          KDualAction *reloadAction = findChild<KDualAction *>();
          reloadAction->setActive(false);
        }
        
        break;
      }
      case Directory:
      {
        FilePtr f = item.staticCast<Smb4KFile>();
        
        if (m_share->workgroupName() == f->workgroupName() && m_share->hostName() == f->hostName() && f->url().path().startsWith(m_share->url().path()))
        {
          KDualAction *reloadAction = findChild<KDualAction *>();
          reloadAction->setActive(false);
        }
        
        break;
      }
      default:
      {
        break;
      }
    }
  }
}


Smb4KPrintDialog::Smb4KPrintDialog(const SharePtr& share, QWidget* parent)
: QDialog(parent), m_share(share)
{
  // 
  // Dialog title
  // 
  setWindowTitle(i18n("Print File"));
  
  //
  // Attributes
  // 
  setAttribute(Qt::WA_DeleteOnClose, true);
  
  //
  // Layout
  // 
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);
  
  //
  // Information group box
  //
  QGroupBox *informationBox = new QGroupBox(i18n("Information"), this);
  QGridLayout *informationBoxLayout = new QGridLayout(informationBox);
  
  // Printer name
  QLabel *printerNameLabel = new QLabel(i18n("Printer:"), informationBox);
  QLabel *printerName = new QLabel(share->displayString(), informationBox);
  
  informationBoxLayout->addWidget(printerNameLabel, 0, 0, 0);
  informationBoxLayout->addWidget(printerName, 0, 1, 0);
  
  // IP address
  QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), informationBox);
  QLabel *ipAddress = new QLabel(share->hostIpAddress(), informationBox);
  
  informationBoxLayout->addWidget(ipAddressLabel, 1, 0, 0);
  informationBoxLayout->addWidget(ipAddress, 1, 1, 0);
  
  // Workgroup
  QLabel *workgroupNameLabel = new QLabel(i18n("Domain:"), informationBox);
  QLabel *workgroupName = new QLabel(share->workgroupName(), informationBox);
  
  informationBoxLayout->addWidget(workgroupNameLabel, 2, 0, 0);
  informationBoxLayout->addWidget(workgroupName, 2, 1, 0);
  
  layout->addWidget(informationBox);
  
  //
  // File and settings group box
  // 
  QGroupBox *fileBox = new QGroupBox(i18n("File and Settings"), this);
  QGridLayout *fileBoxLayout = new QGridLayout(fileBox);
  
  // File
  QLabel *fileLabel = new QLabel(i18n("File:"), fileBox);
  KUrlRequester *file = new KUrlRequester(fileBox);
  file->setMode(KFile::File|KFile::LocalOnly|KFile::ExistingOnly);
  file->setUrl(QUrl::fromLocalFile(QDir::homePath()));
  file->setWhatsThis(i18n("This is the file you want to print on the remote printer. " 
    "Currently only a few mimetypes are supported such as PDF, Postscript, plain text, and " 
    "images. If the file's mimetype is not supported, you need to convert it."));
  connect(file, SIGNAL(textChanged(QString)), this, SLOT(slotUrlChanged()));
  
  fileBoxLayout->addWidget(fileLabel, 0, 0, 0);
  fileBoxLayout->addWidget(file, 0, 1, 0);
  
  // Copies
  QLabel *copiesLabel = new QLabel(i18n("Copies:"), fileBox);
  QSpinBox *copies = new QSpinBox(fileBox);
  copies->setValue(1);
  copies->setMinimum(1);
  copies->setWhatsThis(i18n("This is the number of copies you want to print."));
  
  fileBoxLayout->addWidget(copiesLabel, 1, 0, 0);
  fileBoxLayout->addWidget(copies, 1, 1, 0);
  
  layout->addWidget(fileBox);
  
  //
  // Buttons
  // 
  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  QPushButton *printButton = buttonBox->addButton(i18n("Print"), QDialogButtonBox::ActionRole);
  printButton->setObjectName("print_button");
  printButton->setShortcut(Qt::CTRL|Qt::Key_P);
  connect(printButton, SIGNAL(clicked(bool)), this, SLOT(slotPrintButtonClicked()));
  
  QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
  cancelButton->setObjectName("cancel_button");
  cancelButton->setShortcut(Qt::Key_Escape);
  cancelButton->setDefault(true);
  connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(slotCancelButtonClicked()));
  
  layout->addWidget(buttonBox);
  
  //
  // Set the minimum width
  // 
  setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);
  
  //
  // Set the dialog size
  // 
  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
  resize(windowHandle()->size()); // workaround for QTBUG-40584
  
  //
  // Set the buttons
  // 
  slotUrlChanged();
}


Smb4KPrintDialog::~Smb4KPrintDialog()
{
}


SharePtr Smb4KPrintDialog::share() const
{
  return m_share;
}


KFileItem Smb4KPrintDialog::fileItem() const
{
  return m_fileItem;
}


void Smb4KPrintDialog::slotUrlChanged()
{
  //
  // Take the focus from the URL requester and give it to the dialog's 
  // button box
  // 
  QDialogButtonBox *buttonBox = findChild<QDialogButtonBox *>();
  buttonBox->setFocus();  

  //
  // Get the URL from the URL requester
  // 
  KUrlRequester *file = findChild<KUrlRequester *>();
  KFileItem fileItem = KFileItem(file->url());
  
  //
  // Apply the settings to the buttons of the dialog's button box
  // 
  QPushButton *printButton = findChild<QPushButton *>("print_button");
  printButton->setEnabled(fileItem.url().isValid() && fileItem.isFile());
  printButton->setDefault(fileItem.url().isValid() && fileItem.isFile());
  
  QPushButton *cancelButton = findChild<QPushButton *>("cancel_button");
  cancelButton->setDefault(!fileItem.url().isValid() || !fileItem.isFile());
}


void Smb4KPrintDialog::slotPrintButtonClicked()
{
  //
  // Get the file item that is to be printed
  // 
  KUrlRequester *file = findChild<KUrlRequester *>();
  m_fileItem = KFileItem(file->url());
  
  if (m_fileItem.url().isValid())
  {
    // 
    // Check if the mime type is supported or if the file can be
    // converted into a supported mimetype
    // 
    if (m_fileItem.mimetype() != "application/postscript" &&
        m_fileItem.mimetype() != "application/pdf" &&
        m_fileItem.mimetype() != "application/x-shellscript" &&
        !m_fileItem.mimetype().startsWith(QLatin1String("text")) &&
        !m_fileItem.mimetype().startsWith(QLatin1String("message")) &&
        !m_fileItem.mimetype().startsWith(QLatin1String("image")))
    {
      Smb4KNotification::mimetypeNotSupported(m_fileItem.mimetype());
      return;
    }

    //
    // Save the window size
    // 
    KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    
    //
    // Emit the printFile() signal
    // 
    QSpinBox *copies = findChild<QSpinBox *>();
    emit printFile(m_share, m_fileItem, copies->value());
    
    //
    // Emit the aboutToClose() signal
    //
    emit aboutToClose(this);
    
    //
    // Close the print dialog
    // 
    accept();
  }
}


void Smb4KPrintDialog::slotCancelButtonClicked()
{
  //
  // Abort the printing
  // 
  Smb4KClient::self()->abort();
  
  //
  // Emit the aboutToClose() signal
  // 
  emit aboutToClose(this);
  
  //
  // Reject the dialog
  // 
  reject();
}



