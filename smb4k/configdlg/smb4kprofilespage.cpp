/***************************************************************************
    smb4kprofilespage  -  The configuration page for the profiles
                             -------------------
    begin                : Do Aug 07 2014
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
#include "smb4kprofilespage.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>

// KDE includes
#include <keditlistwidget.h>
#include <klocale.h>

Smb4KProfilesPage::Smb4KProfilesPage(QWidget* parent)
: QWidget(parent)
{
  // Layout
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  layout->setMargin(0);
  
  QGroupBox *settings = new QGroupBox(i18n("Settings"), this);
  
  QVBoxLayout *settings_layout = new QVBoxLayout(settings);
  settings_layout->setSpacing(5);
//   settings_layout->setMargin(0);  
  
  // Use profiles
  QCheckBox *use_profiles = new QCheckBox(Smb4KSettings::self()->useProfilesItem()->label(), settings);
  use_profiles->setObjectName("kcfg_UseProfiles");
  
  settings_layout->addWidget(use_profiles, 0, 0);
  
  // List of profiles
  m_profiles = new KEditListWidget(this);
  m_profiles->setObjectName("kcfg_ProfilesList");
  m_profiles->setEnabled(Smb4KSettings::self()->useProfiles());
  
  layout->addWidget(settings, 0, 0);
  layout->addWidget(m_profiles, 1, 0);

  connect(use_profiles, SIGNAL(stateChanged(int)), this, SLOT(slotEnableWidget(int)));
}


Smb4KProfilesPage::~Smb4KProfilesPage()
{

}


void Smb4KProfilesPage::slotEnableWidget(int state)
{
  switch (state)
  {
    case Qt::Unchecked:
    {
      m_profiles->setEnabled(false);
      break;
    }
    case Qt::Checked:
    {
      m_profiles->setEnabled(true);
      break;
    }
    default:
    {
      break;
    }
  }
}

#include "smb4kprofilespage.moc"
