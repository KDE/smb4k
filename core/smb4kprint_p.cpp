/***************************************************************************
    smb4kprint_p  -  This file contains private helpers for the
    Smb4KPrint class
                             -------------------
    begin                : Fr Okt 31 2008
    copyright            : (C) 2008-2015 by Alexander Reinholdt
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
#include "smb4kprint_p.h"
#include "smb4kshare.h"
#include "smb4ksettings.h"
#include "smb4knotification.h"
#include "smb4kglobal.h"

// Qt includes
#include <QtCore/QTimer>
#include <QtCore/QPointer>
#include <QtCore/QLatin1String>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>
#include <QtGui/QTextDocument>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KIOCore/KFileItem>
#include <KI18n/KLocalizedString>
#include <KConfigGui/KWindowConfig>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;


Smb4KPrintJob::Smb4KPrintJob(QObject *parent) : KJob(parent),
  m_started(false), m_proc(0), m_share(0), m_parent_widget(0)
{
  setCapabilities(KJob::Killable);
}


Smb4KPrintJob::~Smb4KPrintJob()
{
}


void Smb4KPrintJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartPrinting()));
}


void Smb4KPrintJob::setupPrinting(Smb4KShare *printer, QWidget *parentWidget)
{
  Q_ASSERT(printer);
  m_share = printer;
  m_parent_widget = parentWidget;
}


bool Smb4KPrintJob::doKill()
{
  if (m_proc && (m_proc->state() == KProcess::Running || m_proc->state() == KProcess::Starting))
  {
    m_proc->abort();
  }
  else
  {
    // Do nothing
  }
  
  return KJob::doKill();
}


void Smb4KPrintJob::slotStartPrinting()
{
  // Find the smbspool program
  QString smbspool = QStandardPaths::findExecutable("smbspool");

  if (smbspool.isEmpty())
  {
    Smb4KNotification::commandNotFound("smbspool");
    emitResult();
    return;
  }
  else
  {
    // Go ahead
  }
  
  if (m_share)
  {
    // The URL and path of the document that is going to be printed
    QUrl fileURL;
    
    // Temporary directory
    m_temp_dir.setAutoRemove(false);
    
    // The printer
    QPrinter *printer = new QPrinter(QPrinter::HighResolution);
    printer->setCreator("Smb4K");
    printer->setOutputFormat(QPrinter::NativeFormat);
    printer->setOutputFileName(QString("%1/smb4k_print.ps").arg(m_temp_dir.path()));
    
    // Open the print dialog
    QPointer<Smb4KPrintDialog> dlg = new Smb4KPrintDialog(m_share, printer, m_parent_widget);
    
    if (dlg->exec() == QDialog::Accepted)
    {
      fileURL = dlg->fileURL();
      
      // Check that the file exists
      if (!QFile::exists(fileURL.path()))
      {
        Smb4KNotification::fileNotFound(fileURL.path());
        emitResult();
        return;
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // The user canceled. Kill the job.
      emitResult();
      return;
    }
    
    delete dlg;

    // Start the print process
    emit aboutToStart(m_share);

    // Get the file name.
    KFileItem file_item = KFileItem(fileURL);
    
    // Check whether we can directly print or convert the file.
    if (QString::compare(file_item.mimetype(), "application/postscript") == 0 ||
        QString::compare(file_item.mimetype(), "application/pdf") == 0 ||
        file_item.mimetype().startsWith(QLatin1String("image")))
    {
      // Nothing to do here. These mimetypes can be directly
      // printed.
    }
    else if (file_item.mimetype().startsWith(QLatin1String("text")) || 
             file_item.mimetype().startsWith(QLatin1String("message")) ||
             QString::compare(file_item.mimetype(), "application/x-shellscript") == 0)
    {
      QStringList contents;
      
      QFile file(fileURL.path());
      
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
        emitResult();
        return;
      }
      
      // Convert this file to PostScript.
      QTextDocument doc;
      if (file_item.mimetype().endsWith(QLatin1String("html")))
      {
        doc.setHtml(contents.join(" "));
      }
      else
      {
        doc.setPlainText(contents.join("\n"));
      }
      
      doc.print(printer);
      
      // Set the URL of the new file and make sure
      // that the correct protocol is used.
      fileURL.setUrl(printer->outputFileName());
      fileURL.setScheme("file");
    }
    else
    {
      Smb4KNotification::mimetypeNotSupported(file_item.mimetype());
      emitResult();
      return;
    }
    
    // Send the document to the printer.
    QStringList arguments;
    arguments << "111";                                       // job ID number; not used at the moment
    arguments << KUser(KUser::UseRealUserID).loginName();     // user name; not used at the moment
    arguments << "Smb4K print job";                           // job name
    arguments << QString("%1").arg(printer->copyCount());     // number of copies
    arguments << "";                                          // options; not used at the moment
    arguments << fileURL.path();                              // file to print
    
    delete printer;
    
    m_proc = new Smb4KProcess(this);
    m_proc->setOutputChannelMode(KProcess::SeparateChannels);
    m_proc->setEnv("DEVICE_URI", m_share->url().url(), true);
    m_proc->setEnv("PASSWD", m_share->password(), true);
    m_proc->setProgram(smbspool, arguments);

    connect(m_proc, SIGNAL(readyReadStandardOutput()), SLOT(slotReadStandardOutput()));
    connect(m_proc, SIGNAL(readyReadStandardError()),  SLOT(slotReadStandardError()));
    connect(m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

    m_proc->start();
  }
  else
  {
    emitResult();
    return;
  }
}


void Smb4KPrintJob::slotReadStandardOutput()
{
  qDebug() << m_proc->readAllStandardOutput();
}


void Smb4KPrintJob::slotReadStandardError()
{
  QString stdErr = QString::fromUtf8(m_proc->readAllStandardError(), -1).trimmed();

  // Avoid reporting an error if the process was killed by calling the abort() function.
  if (!m_proc->isAborted())
  {
    m_proc->abort();
    
    if (stdErr.contains("NT_STATUS_LOGON_FAILURE") ||
         stdErr.contains("NT_STATUS_ACCESS_DENIED"))
    {
      // Authentication error
      emit authError(this);
    }
    else
    {
      // Remove DEBUG messages.
      QStringList err_msg = stdErr.split('\n', QString::SkipEmptyParts);
      
      QMutableStringListIterator it(err_msg);
      
      while (it.hasNext())
      {
        QString line = it.next();
        
        if (line.contains("DEBUG"))
        {
          it.remove();
        }
        else if (line.trimmed().startsWith(QLatin1String("Ignoring unknown parameter")))
        {
          it.remove();
        }
        else
        {
          // Do nothing
        }
      }
      
      if (!err_msg.isEmpty())
      {
        Smb4KNotification::printingFailed(m_share, err_msg.join("\n"));
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Go ahead
  }
}


void Smb4KPrintJob::slotProcessFinished(int /*exitCode*/, QProcess::ExitStatus status)
{
  // Handle error.
  switch (status)
  {
    case QProcess::CrashExit:
    {
      if (!m_proc->isAborted())
      {
        Smb4KNotification::processError(m_proc->error());
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

  // Finish job
  m_temp_dir.remove();
  emitResult();
  emit finished(m_share);
}



Smb4KPrintDialog::Smb4KPrintDialog(Smb4KShare *share, QPrinter *printer, QWidget *parent)
: QDialog(parent), m_printer(printer)
{
  setWindowTitle(i18n("Print File"));
  
  // Set up the view.
  setupView(share);

  m_print_button->setEnabled(false);
           
  setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

  KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
}


Smb4KPrintDialog::~Smb4KPrintDialog()
{
}


void Smb4KPrintDialog::setupView(Smb4KShare *share)
{
  QVBoxLayout *main_widget_layout = new QVBoxLayout(this);

  // Printer box
  QGroupBox *printer_box = new QGroupBox(i18n("Printer"), this);
  QHBoxLayout *printer_box_layout = new QHBoxLayout(printer_box);
  
  // Icon 
  QLabel *pixmap = new QLabel(printer_box);
  QPixmap print_pix = share->icon().pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(print_pix);
  pixmap->setAlignment(Qt::AlignBottom);

  // Description
  QWidget *desc_box = new QWidget(printer_box);

  QGridLayout *desc_box_layout = new QGridLayout(desc_box);
  desc_box_layout->setSpacing(5);

  QLabel *unc_label = new QLabel(i18n("UNC Address:"), desc_box);
  QLabel *unc = new QLabel(share->unc(), desc_box);
  QLabel *ip_label = new QLabel(i18n("IP Address:"), desc_box);
  QLabel *ip = new QLabel(share->hostIP().trimmed().isEmpty() ?
                          i18n("unknown") :
                          share->hostIP(), desc_box);
  QLabel *wg_label = new QLabel(i18n("Workgroup:"), desc_box);
  QLabel *workgroup = new QLabel(share->workgroupName(), desc_box);

  desc_box_layout->addWidget(unc_label, 0, 0, 0);
  desc_box_layout->addWidget(unc, 0, 1, 0);
  desc_box_layout->addWidget(ip_label, 1, 0, 0);
  desc_box_layout->addWidget(ip, 1, 1, 0);
  desc_box_layout->addWidget(wg_label, 2, 0, 0);
  desc_box_layout->addWidget(workgroup, 2, 1, 0);
  desc_box_layout->setColumnMinimumWidth(2, 0);
  desc_box_layout->setColumnStretch(2, 1);

  desc_box->adjustSize();

  printer_box_layout->addWidget(pixmap);
  printer_box_layout->addWidget(desc_box, Qt::AlignBottom);

  // File requester box
  QGroupBox *file_box = new QGroupBox(i18n("File"), this);

  QGridLayout *file_box_layout = new QGridLayout(file_box);
  file_box_layout->setSpacing(5);

  QLabel *file_label = new QLabel(i18n("File:"), file_box);
  m_file = new KUrlRequester(file_box);
  m_file->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
  m_file->setWhatsThis(i18n("This is the file you want to print on the remote printer. " 
    "Currently only a few mimetypes are supported such as PDF, Postscript, plain text, and " 
    "images. If the file's mimetype is not supported, you need to convert it."));
//   m_file->setToolTip(i18n("The file that is to be printed"));

  file_box_layout->addWidget(file_label, 0, 0, 0);
  file_box_layout->addWidget(m_file, 0, 1, 0);

  // Options
  m_options_box = new QGroupBox(i18n("Options"), this);

  QGridLayout *options_box_layout = new QGridLayout(m_options_box);
  options_box_layout->setSpacing(5);

  QLabel *copies_label = new QLabel(i18n("Copies:"), m_options_box);
  m_copies = new QSpinBox(m_options_box);
  m_copies->setValue(1);
  m_copies->setMinimum(1);
  m_copies->setWhatsThis(i18n("This is the number of copies you want to print."));
//   m_copies->setToolTip(i18n("The number of copies"));

  options_box_layout->addWidget(copies_label, 0, 0, 0);
  options_box_layout->addWidget(m_copies, 0, 1, 0);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  m_print_button = buttonBox->addButton(i18n("Print"), QDialogButtonBox::ActionRole);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_print_button->setShortcut(Qt::CTRL|Qt::Key_P);
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_cancel_button->setDefault(true);

  main_widget_layout->addWidget(printer_box, 0);
  main_widget_layout->addWidget(file_box, 0);
  main_widget_layout->addWidget(buttonBox, 0);

//   printer_box->adjustSize();
  file_box->adjustSize();
  
  connect(m_file, SIGNAL(textChanged(QString)), this, SLOT(slotInputValueChanged(QString)));
  connect(m_print_button, SIGNAL(clicked()), this, SLOT(slotPrintClicked()));
  connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPrintDialog::slotPrintClicked()
{
  m_url = m_file->url();
  m_printer->setCopyCount(m_copies->value());
  accept();
}


void Smb4KPrintDialog::slotCancelClicked()
{
  KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  reject();
}


void Smb4KPrintDialog::slotInputValueChanged(const QString &text)
{
  m_print_button->setEnabled(!text.isEmpty());
  m_print_button->setDefault(!text.isEmpty());
  m_cancel_button->setDefault(text.isEmpty());
  m_options_box->setEnabled(!text.isEmpty());
}

