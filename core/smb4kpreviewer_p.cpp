/***************************************************************************
    Private helper classes for Smb4KPreviewer class.
                             -------------------
    begin                : So Dez 21 2008
    copyright            : (C) 2008-2017 by Alexander Reinholdt
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
#include "smb4kpreviewer_p.h"
#include "smb4knotification.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kglobal.h"
#include "smb4ksettings.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"

// Qt includes
#include <QTimer>
#include <QDateTime>
#include <QLatin1String>
#include <QString>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KIconThemes/KIconLoader>
#include <KIOCore/KIO/Global>
#include <KConfigGui/KWindowConfig>
#include <KCoreAddons/KShell>
#include <KI18n/KLocalizedString>
#include <KXmlGui/KToolBar>
#include <KWidgetsAddons/KGuiItem>

using namespace Smb4KGlobal;


Smb4KPreviewFileItem::Smb4KPreviewFileItem()
{
  m_name = QString();
}


Smb4KPreviewFileItem::~Smb4KPreviewFileItem()
{
}


void Smb4KPreviewFileItem::setItemName(const QString& name)
{
  m_name = name;
}


QString Smb4KPreviewFileItem::itemName() const
{
  return m_name;
}


void Smb4KPreviewFileItem::setDate(const QString& date)
{
  m_date = date;
}


QString Smb4KPreviewFileItem::date() const
{
  return m_date;
}


void Smb4KPreviewFileItem::setItemSize(const QString& size)
{
  m_size = size;
}


QString Smb4KPreviewFileItem::itemSize() const
{
  return m_size;
}


void Smb4KPreviewFileItem::setDir(bool dir)
{
  m_dir = dir;
}


bool Smb4KPreviewFileItem::isDir() const
{
  return m_dir;
}


bool Smb4KPreviewFileItem::isFile() const
{
  return !m_dir;
}


bool Smb4KPreviewFileItem::isHidden() const
{
  return (m_name.startsWith('.') && QString::compare(m_name, ".") != 0 && QString::compare(m_name, "..") != 0);
}


QIcon Smb4KPreviewFileItem::itemIcon() const
{
  QIcon icon;
  
  if (m_dir)
  {
    icon = KDE::icon("folder");
  }
  else
  {
    QUrl url(m_name);
    icon = KDE::icon(KIO::iconNameForUrl(url));
  }
  
  return icon;
}




Smb4KPreviewJob::Smb4KPreviewJob(QObject *parent) : KJob(parent),
  m_started(false), m_share(0), m_parent_widget(0), m_process(0)
{
}


Smb4KPreviewJob::~Smb4KPreviewJob()
{
}


void Smb4KPreviewJob::start()
{
  m_started = true;
  QTimer::singleShot(0, this, SLOT(slotStartPreview()));
}


void Smb4KPreviewJob::setupPreview(const SharePtr &share, const QUrl &url, QWidget *parent)
{
  Q_ASSERT(share);
  m_share = share;
  m_url   = url;
  m_parent_widget = parent;
}


bool Smb4KPreviewJob::doKill()
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


void Smb4KPreviewJob::slotStartPreview()
{
  //
  // Find shell program
  //
  QString smbclient = QStandardPaths::findExecutable("smbclient");

  if (smbclient.isEmpty())
  {
    Smb4KNotification::commandNotFound("smbclient");
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
  OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(m_share);

  //
  // The command
  //
  QStringList command;
  command << smbclient;

  // UNC
  command << m_share->unc();

  // Workgroup
  command << "-W";
  command << m_share->workgroupName();

  // Directory
  QString path = m_url.path();
  
  if (!m_share->isHomesShare())
  {
    path.remove(m_share->shareName(), Qt::CaseInsensitive); 
  }
  else
  {
    path.remove(m_share->login(), Qt::CaseInsensitive);
  }
  
  command << "-D";
  command << (path.isEmpty() ? "/" : path);

  // Command to perform (here: ls)
  command << "-c";
  command << "ls";

  // IP address
  if (m_share->hasHostIpAddress())
  {
    command << "-I";
    command << m_share->hostIpAddress();
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

  // Signing state
  switch (Smb4KSettings::signingState())
  {
    case Smb4KSettings::EnumSigningState::None:
    {
      break;
    }
    case Smb4KSettings::EnumSigningState::On:
    {
      command << "-S";
      command << "on";
      break;
    }
    case Smb4KSettings::EnumSigningState::Off:
    {
      command << "-S";
      command << "off";
      break;
    }
    case Smb4KSettings::EnumSigningState::Required:
    {
      command << "-S";
      command << "required";
      break;
    }
    default:
    {
      break;
    }
  }

  // Buffer size
  if (Smb4KSettings::bufferSize() != 65520)
  {
    command << "-b";
    command << QString("%1").arg(Smb4KSettings::bufferSize());
  }
  else
  {
    // Do nothing
  }

  // SMB Port
  if (options)
  {
    if (options->useSmbPort())
    {
      command << "-p";
      command << QString("%1").arg(options->smbPort());
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (Smb4KSettings::useRemoteSmbPort())
    {
      command << "-p";
      command << QString("%1").arg(Smb4KSettings::remoteSmbPort());
    }
    else
    {
      // Do nothing
    }
  }
  
  // Kerberos
  if (options)
  {
    if (options->useKerberos())
    {
      command << "-k";
    }
    else
    {
      // Do nothing
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

  // Resolve order
  if (!Smb4KSettings::nameResolveOrder().isEmpty() &&
      QString::compare(Smb4KSettings::nameResolveOrder(), samba_options["name resolve order"]) != 0)
  {
    command << "-R";
    command << Smb4KSettings::nameResolveOrder();
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

  // Use Winbind CCache
  if (Smb4KSettings::useWinbindCCache())
  {
    command << "-C";
  }
  else
  {
    // Do nothing
  }

  // Use encryption
  if (Smb4KSettings::encryptSMBTransport())
  {
    command << "-e";
  }
  else
  {
    // Do nothing
  }

  if (!m_share->login().isEmpty())
  {
    command << "-U";
    command << m_share->login();
  }
  else
  {
    command << "-U";
    command << "%";
  }

  //
  // The process
  //
  m_process = new Smb4KProcess(this);
  m_process->setOutputChannelMode(KProcess::SeparateChannels);
  m_process->setEnv("PASSWD", m_share->password(), true);
  m_process->setProgram(command);
  
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
  
  // Start the preview process
  emit aboutToStart(m_share, m_url);

  m_process->start();
}


void Smb4KPreviewJob::processOutput(const QString &stdOut)
{
  QStringList stdOutList = stdOut.split('\n', QString::SkipEmptyParts);
  
  if (!stdOutList.filter("NT_STATUS_ACCESS_DENIED").isEmpty() || 
      !stdOutList.filter("NT_STATUS_LOGON_FAILURE").isEmpty())
  {
    // This might happen if a directory cannot be accessed due to missing
    // read permissions.
    emit authError(this);
  }
  else
  {
    QList<Smb4KPreviewFileItem> items;

    for (const QString &line : stdOutList)
    {
      if (line.contains("blocks of size") || line.contains("Domain=["))
      {
        continue;
      }
      else
      {
        QString entry = line;
        
        QString left = entry.trimmed().section("     ", 0, -2).trimmed();
        QString right = entry.remove(left);

        QString name = left.section("  ", 0, -2).trimmed().isEmpty() ?
                       left :
                       left.section("  ", 0, -2).trimmed();

        QString dir_string = left.right(3).trimmed();
        bool is_dir = (!dir_string.isEmpty() && dir_string.contains("D"));

        QString tmp_size = right.trimmed().section("  ", 0, 0).trimmed();
        QString size;

        if (tmp_size[0].isLetter())
        {
          size = right.trimmed().section("  ", 1, 1).trimmed();
        }
        else
        {
          size = tmp_size;
        }

        QString date = QDateTime::fromString(right.section(QString(" %1 ").arg(size), 1, 1).trimmed()).toString();
        
        if (!name.isEmpty())
        {
          if (!name.startsWith('.'))
          {
            Smb4KPreviewFileItem item;
            item.setItemName(name);
            item.setDir(is_dir);
            item.setDate(date);
            item.setItemSize(size);
            items << item;
          }
          else if (name.startsWith('.') && QString::compare(name, ".") != 0 && QString::compare(name, "..") != 0)
          {
            if (Smb4KSettings::previewHiddenItems())
            {
              Smb4KPreviewFileItem item;
              item.setItemName(name);
              item.setDir(is_dir);
              item.setDate(date);
              item.setItemSize(size);
              items << item;
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
        else
        {
          // Do nothing
        }
      }
    }

    emit preview(m_url, items);
  }
}


void Smb4KPreviewJob::processErrors(const QString &stdErr)
{
  // Remove DEBUG messages, the additional information
  // that smbclient unfortunately reports to stderr and
  // error messages due to a missing smb.conf file.
  QStringList stdErrList = stdErr.split('\n', QString::SkipEmptyParts);
  QMutableStringListIterator it(stdErrList);
  bool delete_next = false;

  while (it.hasNext())
  {
    QString line = it.next();

    if (line.contains(QLatin1String("DEBUG")))
    {
      // Remove debug messages
      it.remove();
    }
    else if (line.contains(QLatin1String("WARNING")) && line.contains(QLatin1String("deprecated")))
    {
      // Remove warnings
      it.remove();
    }
    else if (line.trimmed().startsWith(QLatin1String("Domain")) || line.trimmed().startsWith(QLatin1String("OS")))
    {
      it.remove();
    }
    else if (line.trimmed().startsWith(QLatin1String("Ignoring unknown parameter")))
    {
      it.remove();
    }
    else if (line.contains(QLatin1String("smb.conf")) && line.contains(QLatin1String("Can't load")))
    {
      // smb.conf is missing.
      // Output from smbclient (1 line)
      it.remove();
    }
    else if (line.contains(QLatin1String("smb.conf")) && line.contains(QLatin1String("Unable to open configuration file")))
    {
      // smb.conf is missing.
      // Output by param.c (2 lines)
      it.remove();
      delete_next = true;
    }
    else if (delete_next && line.contains(QLatin1String("No such file or directory")))
    {
      it.remove();
      delete_next = false;
    }
    else
    {
      // Do nothing
    }
  }

  // Avoid reporting an error if the process was killed by calling the abort() function
  // or if only debug and other information was reported.
  if (!m_process->isAborted() && !stdErrList.isEmpty())
  {
    m_process->abort();

    if (!stdErrList.filter("NT_STATUS_LOGON_FAILURE").isEmpty() ||
        !stdErrList.filter("NT_STATUS_ACCESS_DENIED").isEmpty())
    {
      // Authentication error
      emit authError(this);
    }
    else
    {
      if (!stdErrList.isEmpty())
      {
        Smb4KNotification::retrievingPreviewFailed(m_share, stdErrList.join("\n"));
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


void Smb4KPreviewJob::slotProcessFinished(int /*exitCode*/, QProcess::ExitStatus status)
{
  switch (status)
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
      // Process errors
      QString stdErr = QString::fromUtf8(m_process->readAllStandardError(), -1).trimmed();
      processErrors(stdErr);
      
      // Process output
      QString stdOut = QString::fromUtf8(m_process->readAllStandardOutput(), -1).trimmed();
      processOutput(stdOut);
      break;
    }
  }

  emitResult();
  emit finished(m_share, m_url);
}



Smb4KPreviewDialog::Smb4KPreviewDialog(const SharePtr &share, QWidget *parent)
: QDialog(parent), m_share(share), m_actionUsed(false)
{
  // Set the URL
  if (!share->isHomesShare())
  {
    m_url = share->url();
  }
  else
  {
    m_url = share->homeUrl();
  }
  
  // Make the iterator operate on the history list
  m_iterator = m_history.constEnd();
  
  // Set up the dialog
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(i18n("Preview of %1", share->unc()));
  
  // Set the IP address if necessary.
  if (!share->hasHostIpAddress())
  {
    HostPtr host = findHost(share->hostName(), share->workgroupName());
    share->setHostIpAddress(host->ipAddress());
  }
  else
  {
    // Do nothing
  }

  setupView();

  setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
  resize(windowHandle()->size()); // workaround for QTBUG-40584

  QTimer::singleShot(0, this, SLOT(slotRequestPreview()));
}


Smb4KPreviewDialog::~Smb4KPreviewDialog()
{
}


void Smb4KPreviewDialog::setupView()
{
  // Main widget
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);

  m_view = new QListWidget(this);
  m_view->setResizeMode(QListWidget::Adjust);
  m_view->setWrapping(true);
  m_view->setSortingEnabled(true);
  m_view->setWhatsThis(i18n("The preview is displayed here."));
  int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
  m_view->setIconSize(QSize(icon_size, icon_size));

  KToolBar *toolbar = new KToolBar(this, true, false);
  
  m_reload_abort = new KDualAction(toolbar);
  KGuiItem reload_item(i18n("Reload"), KDE::icon("view-refresh"));
  KGuiItem abort_item(i18n("Abort"), KDE::icon("process-stop"));
  m_reload_abort->setActiveGuiItem(reload_item);
  m_reload_abort->setInactiveGuiItem(abort_item);
  m_reload_abort->setActive(true);
  m_reload_abort->setAutoToggle(false);
//   m_reload_abort->setEnabled(false);
  
  m_back    = new QAction(KDE::icon("go-previous"), i18n("Back"), toolbar);
  m_back->setEnabled(false);
  
  m_forward = new QAction(KDE::icon("go-next"), i18n("Forward"), toolbar);
  m_forward->setEnabled(false);
  
  m_up      = new QAction(KDE::icon("go-up"), i18n("Up"), toolbar);
  m_up->setEnabled(false);
  
  m_combo   = new KHistoryComboBox(true, toolbar);
  m_combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_combo->setEditable(false);
  m_combo->setWhatsThis(i18n("The current UNC address is shown here. You can also choose one of "
    "the previously visited locations from the drop-down menu that will then be displayed in the "
    "view above."));
  m_combo->setDuplicatesEnabled(false);

  toolbar->addAction(m_reload_abort);
  toolbar->addAction(m_back);
  toolbar->addAction(m_forward);
  toolbar->addAction(m_up);
  toolbar->insertSeparator(toolbar->addWidget(m_combo));
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_close_button = buttonBox->addButton(QDialogButtonBox::Close);
  
  m_close_button->setShortcut(Qt::Key_Escape);
  m_close_button->setDefault(true);

  layout->addWidget(m_view, 0);
  layout->addWidget(toolbar, 0);
  layout->addWidget(buttonBox, 0);

  connect(m_reload_abort, SIGNAL(triggered(bool)), 
          this, SLOT(slotReloadAbortActionTriggered(bool)));
  
  connect(m_back, SIGNAL(triggered(bool)),
          this, SLOT(slotBackActionTriggered(bool)));
  
  connect(m_forward, SIGNAL(triggered(bool)),
          this, SLOT(slotForwardActionTriggered(bool)));
  
  connect(m_up, SIGNAL(triggered(bool)),
          this, SLOT(slotUpActionTriggered(bool)));
  
  connect(m_combo, SIGNAL(activated(QString)),
          this, SLOT(slotItemActivated(QString)));
  
  connect(m_view, SIGNAL(itemActivated(QListWidgetItem*)),
          this, SLOT(slotItemExecuted(QListWidgetItem*)));
  
  connect(KIconLoader::global(), SIGNAL(iconChanged(int)),
          this, SLOT(slotIconSizeChanged(int)));
  
  connect(m_close_button, SIGNAL(clicked()), 
          this, SLOT(slotCloseClicked()));
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KPreviewDialog::slotReloadAbortActionTriggered(bool /*t*/)
{
  m_actionUsed = true;
  
  if (m_reload_abort->isActive())
  {
    // Request the preview
    slotRequestPreview();
  }
  else
  {
    // Emit signal to kill the preview job.
    emit abortPreview(m_share);
  }
  
  m_actionUsed = false;
}


void Smb4KPreviewDialog::slotBackActionTriggered(bool /*t*/)
{
  m_actionUsed = true;
  
  // Abort if we either reached the beginning of the list or
  // if the list only contains one item.
  if (m_iterator == m_history.constBegin() || m_history.size() < 2)
  {
    m_back->setEnabled(false);
    return;
  }
  else
  {
    // Do nothing
  }
  
  // If the iterator points to the end, move it to the last item,
  // so that in the next step we can move to the next to last item.
  if (m_iterator == m_history.constEnd() && m_history.size() > 1)
  {
    // Move to the last (current) item
    --m_iterator;
  }
  else
  {
    // Do nothing
  }
  
  // Set the new URL from the history list.
  QUrl url = *(--m_iterator);
  
  if (url.isValid() && !url.isEmpty())
  {
    m_url = url;
  }
  else
  {
    // Stop here
    return;
  }
  
  // Request the preview
  slotRequestPreview();
  
  m_actionUsed = false;
}


void Smb4KPreviewDialog::slotForwardActionTriggered(bool /*t*/)
{
  m_actionUsed = true;
  
  // Abort if we either reached the end of the list or
  // if the list only contains one item.
  if (m_iterator == m_history.constBegin() || m_history.size() < 2)
  {
    m_forward->setEnabled(false);
    return;
  }
  else
  {
    // Do nothing
  }
  
  // If the iterator points to the begin, move it to the first item,
  // so that in the next step we can move to the second item.
  if (m_iterator == m_history.constBegin() && m_history.size() > 1)
  {
    // Move to the first (the oldest) item
    ++m_iterator;
  }
  else
  {
    // Do nothing
  }
  
  // Get the new location and assemble the new URL.
  QUrl url = *(++m_iterator);
  
  if (url.isValid() && !url.isEmpty())
  {
    m_url = url;
  }
  else
  {
    // Stop here
    return;
  }
  
  // Request the preview
  slotRequestPreview();
  
  m_actionUsed = false;
}


void Smb4KPreviewDialog::slotUpActionTriggered(bool /*t*/)
{
  m_actionUsed = true;
  
  if (!m_share->url().matches(m_url, QUrl::StripTrailingSlash))
  {
    // Move one level up.
    m_url = KIO::upUrl(m_url);
    
    // Request the preview
    slotRequestPreview();
  }
  else
  {
    m_up->setEnabled(false);
  }
  
  m_actionUsed = false;
}


void Smb4KPreviewDialog::slotRequestPreview()
{
  // Set the current item
  QString current = m_url.toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::StripTrailingSlash)
                         .replace(m_url.host(), m_url.host().toUpper());
    
  // Set the history
  if (!m_actionUsed)
  {
    m_history << m_url;
    m_iterator = m_history.constEnd();
  }
  else
  {
    // Do nothing
  }

  m_combo->addToHistory(current);
  m_combo->setCurrentItem(current, false);

  // Clear the view
  m_view->clear();
  
  // Request the preview for the current URL
  emit requestPreview(m_share, m_url, parentWidget());
}


void Smb4KPreviewDialog::slotDisplayPreview(const QUrl &url, const QList<Smb4KPreviewFileItem> &contents)
{
  if (!m_url.matches(url, QUrl::StripTrailingSlash))
  {
    return;
  }
  else
  {
    // Do nothing
  }

  // Display the preview
  for (const Smb4KPreviewFileItem &item : contents)
  {
    QListWidgetItem *listItem = new QListWidgetItem(item.itemIcon(), item.itemName(), m_view,
                                                    (item.isDir() ? Directory : File));
    listItem->setData(Qt::UserRole, item.itemName());    
  }
  
  // Enable/disable the back action
  bool enableBack = (m_iterator != m_history.constBegin() && m_history.size() > 1);
  m_back->setEnabled(enableBack);
  
  // Enable/disable the forward action
  bool enableForward = (m_iterator != m_history.constEnd() && m_history.size() > 1);
  m_forward->setEnabled(enableForward);

  // Enable/disable the up action.
  bool enableUp = (!m_share->url().matches(m_url, QUrl::StripTrailingSlash));
  m_up->setEnabled(enableUp);
}


void Smb4KPreviewDialog::slotAboutToStart(const SharePtr &share, const QUrl &url)
{
  if (share == m_share && m_url.matches(url, QUrl::StripTrailingSlash))
  {
    m_reload_abort->setActive(false);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotFinished(const SharePtr &share, const QUrl &url)
{
  if (share == m_share && m_url.matches(url, QUrl::StripTrailingSlash))
  {
    m_reload_abort->setActive(true);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotItemExecuted(QListWidgetItem *item)
{
  if (item)
  {
    switch (item->type())
    {
      case Directory:
      {
        if (!Smb4KPreviewer::self()->isRunning(m_share))
        {
          QString old_path = m_url.path();
          m_url.setPath(QString("%1/%2").arg(old_path).arg(item->data(Qt::UserRole).toString()));
          slotRequestPreview();
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
    // Do nothing
  }
}


void Smb4KPreviewDialog::slotItemActivated(const QString &item)
{
  m_actionUsed = true;
  
  if (!Smb4KPreviewer::self()->isRunning(m_share))
  {
    QUrl u;
    u.setUrl(item, QUrl::TolerantMode);
    u.setScheme("smb");
    
    m_url.setPath(u.path());
    slotRequestPreview();
  }
  else
  {
    // Do nothing
  }
  
  m_actionUsed = false;
}


void Smb4KPreviewDialog::slotCloseClicked()
{
  KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  emit aboutToClose(this);
  accept();
}


void Smb4KPreviewDialog::slotIconSizeChanged(int group)
{
  switch (group)
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
      m_view->setIconSize(QSize(icon_size, icon_size));
      break;
    }
    default:
    {
      break;
    }
  }
}

