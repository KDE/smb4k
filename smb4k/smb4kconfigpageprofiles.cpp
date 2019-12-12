/***************************************************************************
    The configuration page for the profiles
                             -------------------
    begin                : Do Aug 07 2014
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

// application specific includes
#include "smb4kconfigpageprofiles.h"
#include "core/smb4ksettings.h"
#include "core/smb4kprofilemanager.h"

// Qt includes
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KCompletion/KLineEdit>


Smb4KConfigPageProfiles::Smb4KConfigPageProfiles(QWidget* parent)
: QWidget(parent)
{
  // Layout
  QVBoxLayout *layout = new QVBoxLayout(this);
  
  QGroupBox *settings = new QGroupBox(i18n("Settings"), this);
  QVBoxLayout *settingsLayout = new QVBoxLayout(settings);
  
  // Use profiles
  QCheckBox *useProfiles = new QCheckBox(Smb4KSettings::self()->useProfilesItem()->label(), settings);
  useProfiles->setObjectName("kcfg_UseProfiles");
  
  // Use profile migration assistant
  QCheckBox *useAssistant = new QCheckBox(Smb4KSettings::self()->useMigrationAssistantItem()->label(), settings);
  useAssistant->setObjectName("kcfg_UseMigrationAssistant");
  
  settingsLayout->addWidget(useProfiles, 0, 0);
  settingsLayout->addWidget(useAssistant, 1, 0);
  
  QGroupBox *profiles = new QGroupBox(i18n("Profiles"), this);
  
  QVBoxLayout *profilesLayout = new QVBoxLayout(profiles);
  profilesLayout->setSpacing(5);
  
  // List of profiles
  m_profiles = new KEditListWidget(profiles);
  m_profiles->setObjectName("kcfg_ProfilesList");
  m_profiles->setEnabled(Smb4KSettings::self()->useProfiles());
  
  profilesLayout->addWidget(m_profiles, 0, 0);
  
  layout->addWidget(settings, 0, 0);
  layout->addWidget(profiles, 1, 0);

  connect(useProfiles, SIGNAL(stateChanged(int)), this, SLOT(slotEnableWidget(int)));
  connect(m_profiles, SIGNAL(removed(QString)), this, SLOT(slotProfileRemoved(QString)));
  connect(m_profiles->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotProfileChanged()));
}


Smb4KConfigPageProfiles::~Smb4KConfigPageProfiles()
{
}


QList< QPair<QString,QString> > Smb4KConfigPageProfiles::renamedProfiles() const
{
  return m_renamed;
}


void Smb4KConfigPageProfiles::clearRenamedProfiles()
{
  m_renamed.clear();
}


QStringList Smb4KConfigPageProfiles::removedProfiles() const
{
  return m_removed;
}


void Smb4KConfigPageProfiles::clearRemovedProfiles()
{
  m_removed.clear();
}


void Smb4KConfigPageProfiles::slotEnableWidget(int state)
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


void Smb4KConfigPageProfiles::slotProfileRemoved(const QString& name)
{
  // If the removed profile was renamed before, remove it from 
  // the list.
  QMutableListIterator< QPair<QString,QString> > it(m_renamed);
  
  while (it.hasNext())
  {
    QPair<QString,QString> entry = it.next();
    
    if (QString::compare(entry.first, name) == 0 || QString::compare(entry.second, name) == 0)
    {
      it.remove();
    }
  }
  
  m_removed << name;
}


void Smb4KConfigPageProfiles::slotProfileChanged()
{
  QStringList savedProfiles = Smb4KProfileManager::self()->profilesList();
  QStringList currentProfiles = m_profiles->items();
    
  if (savedProfiles.size() == currentProfiles.size())
  {
    QMutableStringListIterator it(savedProfiles);
      
    while (it.hasNext())
    {
      QString entry = it.next();
      int index = currentProfiles.indexOf(entry);
          
      if (index != -1)
      {
        currentProfiles.removeAt(index);
        it.remove();
      }
    }
        
    if (!savedProfiles.isEmpty() && !currentProfiles.isEmpty())
    {
      // Take care that multiple renamings will have the correct
      // result.
      bool write = true;
      
      for (int i = 0; i < m_renamed.size(); ++i)
      {
        if (QString::compare(savedProfiles.first(), m_renamed.at(i).first, Qt::CaseSensitive) == 0)
        {
          QPair<QString,QString> pair = static_cast< QPair<QString,QString> >(m_renamed.at(i));
          pair.second = currentProfiles.first();
          write = false;
          break;
        }
      }
      
      // Write the renamed profile to the list, if necessary.
      if (write)
      {
        QPair<QString,QString> renamed(savedProfiles.first(), currentProfiles.first());
        m_renamed << renamed;
      }
    }
  }
}

