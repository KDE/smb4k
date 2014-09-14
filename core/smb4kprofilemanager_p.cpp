/***************************************************************************
    smb4kprofilemanager_p  -  Private helper class(es) for the profile 
    manager.
                             -------------------
    begin                : Mi Aug 12 2014
    copyright            : (C) 2014 by Alexander Reinholdt
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
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>

// KDE includes
#include <klocale.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kconfiggroup.h>


Smb4KProfileMigrationDialog::Smb4KProfileMigrationDialog(const QStringList& from, const QStringList& to, QWidget* parent)
: KDialog(parent), m_from_list(from), m_to_list(to)
{
  setCaption(i18n("Profile Migration Assistant"));
  setButtons(Ok|Cancel);
  setDefaultButton(Ok);
  
  setupView();
  
  setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);
  
  KConfigGroup group(Smb4KSettings::self()->config(), "ProfileMigrationDialog");
  restoreDialogSize(group);
  
  connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
}


Smb4KProfileMigrationDialog::~Smb4KProfileMigrationDialog()
{
}


void Smb4KProfileMigrationDialog::setupView()
{
  QWidget *main = new QWidget(this);
  setMainWidget(main);
  
  QVBoxLayout *layout = new QVBoxLayout(main);
  layout->setSpacing(5);
  layout->setMargin(0);
  
  // Description
  QWidget *description = new QWidget(main);

  QHBoxLayout *desc_layout = new QHBoxLayout(description);
  desc_layout->setSpacing(5);
  desc_layout->setMargin(0);

  QLabel *pixmap = new QLabel(description);
  QPixmap pix = KIcon("format-list-unordered").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(pix);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = new QLabel(i18n("Migrate all relevant settings of one profile to another."));
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  desc_layout->addWidget(pixmap, 0);
  desc_layout->addWidget(label, Qt::AlignBottom);
  
  QWidget *editors = new QWidget(main);
  
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
  
  layout->addWidget(description, 0);
  layout->addWidget(editors, 0);
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
  saveDialogSize(group, KConfigGroup::Normal);
}


#include "smb4kprofilemanager_p.moc"
