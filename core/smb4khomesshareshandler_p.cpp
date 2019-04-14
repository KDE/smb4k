/***************************************************************************
    Private helpers for the homes shares handler
                             -------------------
    begin                : Mo Apr 11 2011
    copyright            : (C) 2011-2019 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4khomesshareshandler_p.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

// Qt includes
#include <QString>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KConfigGui/KWindowConfig>

Smb4KHomesUsers::Smb4KHomesUsers(const SharePtr &s, const QStringList &u)
{
  m_workgroup_name = s->workgroupName();
  m_host_name      = s->hostName();
  m_share_name     = s->shareName();
  m_host_ip.setAddress(s->hostIpAddress());
  m_users          = u;
}


Smb4KHomesUsers::Smb4KHomesUsers(const Smb4KHomesUsers &u)
{
  m_workgroup_name = u.workgroupName();
  m_host_name      = u.hostName();
  m_share_name     = u.shareName();
  m_host_ip.setAddress(u.hostIP());
  m_users          = u.users();
  m_profile        = u.profile();
}


Smb4KHomesUsers::Smb4KHomesUsers()
{
}


Smb4KHomesUsers::~Smb4KHomesUsers()
{
}


QString Smb4KHomesUsers::workgroupName() const
{
  return m_workgroup_name;
}


void Smb4KHomesUsers::setWorkgroupName(const QString& name)
{
  m_workgroup_name = name;
}


QString Smb4KHomesUsers::hostName() const
{
  return m_host_name;
}


void Smb4KHomesUsers::setHostName(const QString& name)
{
  m_host_name = name;
}


QString Smb4KHomesUsers::shareName() const
{
  return m_share_name;
}


void Smb4KHomesUsers::setShareName(const QString& name)
{
  m_share_name = name;
}


QString Smb4KHomesUsers::hostIP() const
{
  return m_host_ip.toString();
}


void Smb4KHomesUsers::setHostIP(const QString& ip)
{
  m_host_ip.setAddress(ip);
}


QStringList Smb4KHomesUsers::users() const
{
  return m_users;
}


void Smb4KHomesUsers::setUsers(const QStringList& users)
{
  m_users = users;
}


QString Smb4KHomesUsers::profile() const
{
  return m_profile;
}


void Smb4KHomesUsers::setProfile(const QString& profile)
{
  m_profile = profile;
}



Smb4KHomesUserDialog::Smb4KHomesUserDialog(const SharePtr &share, QWidget *parent) 
: QDialog(parent), m_share(share)
{
  //
  // Set the window title
  // 
  setWindowTitle(i18n("Specify User"));

  //
  // Setup the view
  // 
  setupView();
  
  //
  // Set the dialog size
  // 
  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "HomesUserDialog");
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
  // Fill the completion object
  // 
  m_user_combo->completionObject()->setItems(group.readEntry("HomesUsersCompletion", QStringList()));
}


Smb4KHomesUserDialog::~Smb4KHomesUserDialog()
{
}


void Smb4KHomesUserDialog::setupView()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);

  QWidget *description = new QWidget(this);

  QHBoxLayout *desc_layout = new QHBoxLayout(description);
  desc_layout->setSpacing(5);
  desc_layout->setMargin(0);

  QLabel *pixmap = new QLabel(description);
  QPixmap user_pix = KDE::icon("user-identity").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(user_pix);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = new QLabel(i18n("Please specify a username for share<br><b>%1</b>.", m_share->displayString()), description);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  desc_layout->addWidget(pixmap, 0);
  desc_layout->addWidget(label, Qt::AlignBottom);
  
  QWidget *input = new QWidget(this);
  
  QGridLayout *input_layout = new QGridLayout(input);
  input_layout->setSpacing(5);
  input_layout->setMargin(0);
  input_layout->setColumnStretch(0, 0);
  input_layout->setColumnStretch(1, 1);
  
  QLabel *input_label = new QLabel(i18n("User:"), input);

  m_user_combo = new KComboBox(true, input);
  m_user_combo->setDuplicatesEnabled(false);
  m_user_combo->setEditable(true);
  
  input_layout->addWidget(input_label, 0, 0, 0);
  input_layout->addWidget(m_user_combo, 0, 1, 0);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_clear_button = buttonBox->addButton(i18n("Clear List"), QDialogButtonBox::ActionRole);
  m_clear_button->setIcon(KDE::icon("edit-clear"));
  m_clear_button->setEnabled(false);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_ok_button->setEnabled(false);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_ok_button->setDefault(true);

  layout->addWidget(description, 0);
  layout->addWidget(input, 0);
  layout->addWidget(buttonBox, 0);
  
  m_user_combo->setFocus();
  
  connect(m_user_combo, SIGNAL(currentTextChanged(QString)), SLOT(slotTextChanged(QString)));
  connect(m_user_combo->lineEdit(), SIGNAL(editingFinished()), SLOT(slotHomesUserEntered()));
  connect(m_clear_button, SIGNAL(clicked()), SLOT(slotClearClicked()));
  connect(m_ok_button, SIGNAL(clicked()), SLOT(slotOkClicked()));
  connect(m_cancel_button, SIGNAL(clicked()), SLOT(reject()));
}


void Smb4KHomesUserDialog::setUserNames(const QStringList &users)
{
  if (!users.isEmpty())
  {
    m_user_combo->addItems(users);
    m_user_combo->setCurrentItem("");
    m_clear_button->setEnabled(true);
  }
}


QStringList Smb4KHomesUserDialog::userNames()
{
  QStringList users;
  
  for (int i = 0; i < m_user_combo->count(); ++i)
  {
    users << m_user_combo->itemText(i);
  }
  
  if (!users.contains(m_user_combo->currentText()))
  {
    users << m_user_combo->currentText();
  }
  
  return users;
}


void Smb4KHomesUserDialog::slotTextChanged(const QString &text)
{
  m_ok_button->setEnabled(!text.isEmpty());
}


void Smb4KHomesUserDialog::slotClearClicked()
{
  m_user_combo->clearEditText();
  m_user_combo->clear();
  m_clear_button->setEnabled(false);
}


void Smb4KHomesUserDialog::slotOkClicked()
{
  KConfigGroup group(Smb4KSettings::self()->config(), "HomesUserDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  group.writeEntry("HomesUsersCompletion", m_user_combo->completionObject()->items());
  accept();
}


void Smb4KHomesUserDialog::slotHomesUserEntered()
{
  KCompletion *completion = m_user_combo->completionObject();

  if (!m_user_combo->currentText().isEmpty())
  {
    completion->addItem(m_user_combo->currentText());
  }
}

