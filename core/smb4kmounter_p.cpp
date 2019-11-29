/***************************************************************************
    This file contains private helper classes for the Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2019 by Alexander Reinholdt
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

// application specific includes
#include "smb4kmounter_p.h"
#include "smb4ksettings.h"
#include "smb4knotification.h"
#include "smb4khomesshareshandler.h"
#include "smb4kglobal.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"

// Qt includes
#include <QUrl>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KConfigGui/KWindowConfig>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;


Smb4KMountDialog::Smb4KMountDialog(const SharePtr &share, QWidget *parent)
: QDialog(parent), m_share(share), m_valid(true)
{
  //
  // Set the title
  // 
  setWindowTitle(i18n("Mount Share"));
  
  //
  // Set up the view
  // 
  setupView();
  
  //
  // Set the dialog size
  // 
  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "MountDialog");
  QSize dialogSize;
  
  if (group.exists())
  {
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    dialogSize = windowHandle()->size();
  }
  else
  {
    dialogSize = sizeHint();
  }
  
  resize(dialogSize); // workaround for QTBUG-40584

  //
  // Fill the completion objects
  // 
  m_share_input->completionObject()->setItems(group.readEntry("ShareNameCompletion", QStringList()));
  m_ip_input->completionObject()->setItems(group.readEntry("IPAddressCompletion", QStringList()));
  m_workgroup_input->completionObject()->setItems(group.readEntry("WorkgroupCompletion", QStringList()));
}


Smb4KMountDialog::~Smb4KMountDialog()
{
}


void Smb4KMountDialog::setupView()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);

  QWidget *description = new QWidget(this);

  QHBoxLayout *desc_layout = new QHBoxLayout(description);
  desc_layout->setSpacing(5);

  QLabel *pixmap = new QLabel(description);
  QPixmap mount_pix = KDE::icon("view-form", QStringList("emblem-mounted")).pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(mount_pix);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = new QLabel(i18n("Enter the location and optionally the IP address and workgroup to mount a share."), description);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  desc_layout->addWidget(pixmap, 0);
  desc_layout->addWidget(label, Qt::AlignBottom);

  QWidget *edit_widget = new QWidget(this);

  QGridLayout *edit_layout = new QGridLayout(edit_widget);
  layout->setSpacing(5);

  QLabel *shareLabel = new QLabel(i18n("Location:"), edit_widget);
  m_share_input = new KLineEdit(edit_widget);
  m_share_input->setWhatsThis(i18n("The location of the share is provided by the Uniform Resource Locator (URL). It generally has the following syntax: "
    "[smb:]//[USER:PASSWORD@]HOST:PORT/SHARE. The username, password and port are optional. You should omit to enter the password here, because it is shown in cleartext."));
//   m_share_input->setToolTip(i18n("The URL of the share"));
  m_share_input->setCompletionMode(KCompletion::CompletionPopupAuto);
  m_share_input->setClearButtonEnabled(true);
  m_share_input->setMinimumWidth(200);
  m_share_input->setFocus();

  QLabel *addressLabel = new QLabel(i18n("IP Address:"), edit_widget);
  m_ip_input = new KLineEdit(edit_widget);
  m_ip_input->setWhatsThis(i18n("The Internet Protocol (IP) address identifies the "
    "host in the network and indicates where it is. It has two valid formats, the one "
    "known as IP version 4 (e.g. 192.168.2.11) and the version 6 format "
    "(e.g. 2001:0db8:85a3:08d3:1319:8a2e:0370:7334)."));
//   m_ip_input->setToolTip(i18n("The IP address of the host where the share is located"));
  m_ip_input->setCompletionMode(KCompletion::CompletionPopupAuto);
  m_ip_input->setClearButtonEnabled(true);
  m_ip_input->setMinimumWidth(200);

  QLabel *workgroupLabel = new QLabel(i18n("Workgroup:"), edit_widget);
  m_workgroup_input = new KLineEdit(edit_widget);
  m_workgroup_input->setWhatsThis(i18n("The workgroup or domain identifies the "
    "peer-to-peer computer network the host is located in."));
//   m_workgroup_input->setToolTip(i18n("The workgroup where the host is located"));
  m_workgroup_input->setCompletionMode(KCompletion::CompletionPopupAuto);
  m_workgroup_input->setClearButtonEnabled(true);
  m_workgroup_input->setMinimumWidth(200);

  edit_layout->addWidget(shareLabel, 0, 0, 0);
  edit_layout->addWidget(m_share_input, 0, 1, 0);
  edit_layout->addWidget(addressLabel, 1, 0, 0);
  edit_layout->addWidget(m_ip_input, 1, 1, 0);
  edit_layout->addWidget(workgroupLabel, 2, 0, 0);
  edit_layout->addWidget(m_workgroup_input, 2, 1, 0);

  m_bookmark = new QCheckBox(i18n("Add this share to the bookmarks"), this);
  m_bookmark->setWhatsThis(i18n("If you tick this checkbox, the share will be bookmarked "
    "and you can access it e.g. through the \"Bookmarks\" menu entry in the main window."));
//   m_bookmark->setToolTip(i18n("Add this share to the bookmarks"));
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);

  layout->addWidget(description, Qt::AlignBottom);
  layout->addWidget(edit_widget, 0);
  layout->addWidget(m_bookmark, 0);
  layout->addWidget(buttonBox, 0);

  slotChangeInputValue(m_share_input->text());

  // 
  // Connections
  // 
  connect(m_share_input, SIGNAL(textChanged(QString)), this, SLOT(slotChangeInputValue(QString)));
  connect(m_share_input, SIGNAL(editingFinished()), this, SLOT(slotShareNameEntered()));
  connect(m_ip_input, SIGNAL(editingFinished()), this, SLOT(slotIPEntered()));
  connect(m_workgroup_input, SIGNAL(editingFinished()), this, SLOT(slotWorkgroupEntered()));
  connect(m_ok_button, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMountDialog::slotChangeInputValue(const QString& _test)
{
  m_ok_button->setEnabled(!_test.isEmpty());
}


void Smb4KMountDialog::slotOkClicked()
{
  if (!m_share_input->text().trimmed().isEmpty())
  {
    //
    // Get the URL
    // 
    QString userInput = m_share_input->text().trimmed();
    
    //
    // Take care of a Windows-like UNC addresses
    // 
    if (userInput.startsWith(QLatin1String("\\")))
    {
      userInput.replace("\\", "/");
    }
    
    //
    // Set the URL and adjust the scheme
    // 
    QUrl smbUrl = QUrl::fromUserInput(userInput);
    smbUrl.setScheme("smb");
    
    //
    // Set the URL of the share
    // 
    if (smbUrl.isValid() && !smbUrl.host().isEmpty() && !smbUrl.path().isEmpty() && !smbUrl.path().endsWith(QLatin1Char('/')))
    {
      m_share->setUrl(smbUrl);
      m_share->setWorkgroupName(m_workgroup_input->text().trimmed());
      m_share->setHostIpAddress(m_ip_input->text().trimmed());
    }
    else
    {
      Smb4KNotification::invalidURLPassed();
      m_valid = false;
    }
  }
  
  KConfigGroup group(Smb4KSettings::self()->config(), "MountDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  group.writeEntry("ShareNameCompletion", m_share_input->completionObject()->items());
  group.writeEntry("IPAddressCompletion", m_ip_input->completionObject()->items());
  group.writeEntry("WorkgroupCompletion", m_workgroup_input->completionObject()->items());
  
  accept();
}


void Smb4KMountDialog::slotCancelClicked()
{
  Smb4KMounter::self()->abort();
  reject();
}


void Smb4KMountDialog::slotShareNameEntered()
{
  KCompletion *completion = m_share_input->completionObject();
  QUrl url(m_share_input->userText());
  url.setScheme("smb");

  if (url.isValid() && !url.isEmpty())
  {
    completion->addItem(m_share_input->userText());
  }
}


void Smb4KMountDialog::slotIPEntered()
{
  KCompletion *completion = m_ip_input->completionObject();

  if (!m_ip_input->userText().isEmpty())
  {
    completion->addItem(m_ip_input->userText());
  }
}


void Smb4KMountDialog::slotWorkgroupEntered()
{
  KCompletion *completion = m_workgroup_input->completionObject();

  if (!m_workgroup_input->userText().isEmpty())
  {
    completion->addItem(m_workgroup_input->userText());
  }
}

