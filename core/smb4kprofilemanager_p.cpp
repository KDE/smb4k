/***************************************************************************
    Private helper class(es) for the profile manager.
                             -------------------
    begin                : Mi Aug 12 2014
    copyright            : (C) 2014-2019 by Alexander Reinholdt
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
#include "smb4kprofilemanager_p.h"
#include "smb4ksettings.h"

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPixmap>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KConfigGui/KWindowConfig>


Smb4KProfileMigrationDialog::Smb4KProfileMigrationDialog(const QStringList& from, const QStringList& to, QWidget* parent)
: QDialog(parent), m_from_list(from), m_to_list(to)
{
  setWindowTitle(i18n("Profile Migration Assistant"));
  
  setupView();
  
  setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);
  
  create();

  KConfigGroup group(Smb4KSettings::self()->config(), "ProfileMigrationDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
  resize(windowHandle()->size()); // workaround for QTBUG-40584
}


Smb4KProfileMigrationDialog::~Smb4KProfileMigrationDialog()
{
}


void Smb4KProfileMigrationDialog::setupView()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  
  // Description
  QWidget *description = new QWidget(this);

  QHBoxLayout *desc_layout = new QHBoxLayout(description);
  desc_layout->setSpacing(5);
  desc_layout->setMargin(0);

  QLabel *pixmap = new QLabel(description);
  QPixmap pix = KDE::icon("format-list-unordered").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(pix);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = new QLabel(i18n("Migrate all relevant settings of one profile to another."));
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  desc_layout->addWidget(pixmap, 0);
  desc_layout->addWidget(label, Qt::AlignBottom);
  
  QWidget *editors = new QWidget(this);
  
  QGridLayout *editors_layout = new QGridLayout(editors);
  editors_layout->setSpacing(5);
  editors_layout->setMargin(0);
  editors_layout->setColumnStretch(0, 0);
  editors_layout->setColumnStretch(1, 1);
  
  QLabel *from = new QLabel(i18n("Old Profile:"), editors);
  editors_layout->addWidget(from, 0, 0, 0);
  
  m_from_box = new KComboBox(editors);
  
  if (m_from_list.size() == 1 && m_from_list.first().isEmpty())
  {
    m_from_box->addItem(i18n("<Default Profile>"));
  }
  else
  {
    if (m_to_list.size() == 1 && m_to_list.first().isEmpty())
    {
      m_from_box->addItem(i18n("<All Profiles>"));
    }
    else
    {
      m_from_box->addItems(m_from_list);
    }
  }
  
  editors_layout->addWidget(m_from_box, 0, 1, 0);
  
  QLabel *to = new QLabel(i18n("New Profile:"), editors);
  editors_layout->addWidget(to, 1, 0, 0);
  
  m_to_box = new KComboBox(editors);
  
  if (m_to_list.size() == 1 && m_to_list.first().isEmpty())
  {
    m_to_box->addItem(i18n("<Default Profile>"));
  }
  else
  {
    m_to_box->addItems(m_to_list);
    m_to_box->setCurrentItem(Smb4KProfileManager::self()->activeProfile());
  }
  
  editors_layout->addWidget(m_to_box, 1, 1, 0);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_ok_button->setDefault(true);
  
  layout->addWidget(description, 0);
  layout->addWidget(editors, 0);
  layout->addWidget(buttonBox, 0);
  
  connect(m_ok_button, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));  
}


QString Smb4KProfileMigrationDialog::from() const
{
  return m_from_box->currentText();
}


QString Smb4KProfileMigrationDialog::to() const
{
  return m_to_box->currentText();
}


void Smb4KProfileMigrationDialog::slotOkClicked()
{
  KConfigGroup group(Smb4KSettings::self()->config(), "ProfileMigrationDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  accept();
}

