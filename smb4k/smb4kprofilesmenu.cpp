/***************************************************************************
    smb4kprofilesmenu  -  The menu for the profiles
                             -------------------
    begin                : Do Aug 10 2014
    copyright            : (C) 2014-2015 by Alexander Reinholdt
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
#include "smb4kprofilesmenu.h"
#include "core/smb4ksettings.h"
#include "core/smb4kprofilemanager.h"

// Qt includes
#include <QStringList>
#include <QAction>

// KDE includes
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>

Smb4KProfilesMenu::Smb4KProfilesMenu(QObject* parent)
: KSelectAction(KDE::icon("format-list-unordered"), i18n("Profiles"), parent)
{
  QStringList profiles = Smb4KProfileManager::self()->profilesList();
  
  for (int i = 0; i < profiles.size(); ++i)
  {
    QAction *action = addAction(profiles.at(i));
    
    if (action)
    {
      action->setEnabled(Smb4KProfileManager::self()->useProfiles());
    }
    else
    {
      // Do nothing
    }
  }
  
  setCurrentAction(Smb4KProfileManager::self()->activeProfile());
  setToolBarMode(KSelectAction::MenuMode);
  
  // Connections
  connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)), 
          this, SLOT(slotActiveProfileChanged(QString)));
  connect(Smb4KProfileManager::self(), SIGNAL(profilesListChanged(QStringList)),
          this, SLOT(slotProfilesListChanged(QStringList)));
  connect(this, SIGNAL(triggered(QString)), 
          this, SLOT(slotActionTriggered(QString)));
}


Smb4KProfilesMenu::~Smb4KProfilesMenu()
{
}



void Smb4KProfilesMenu::slotActiveProfileChanged(const QString& newProfile)
{
  setCurrentAction(newProfile);
}


void Smb4KProfilesMenu::slotProfilesListChanged(const QStringList& profiles)
{
  clear();
  
  for (int i = 0; i < profiles.size(); ++i)
  {
    QAction *action = addAction(profiles.at(i));
    
    if (action)
    {
      action->setEnabled(Smb4KProfileManager::self()->useProfiles());
    }
    else
    {
      // Do nothing
    }
  }
  
  setCurrentAction(Smb4KProfileManager::self()->activeProfile());
}


void Smb4KProfilesMenu::slotProfileUsageChanged(bool use)
{
  for (int i = 0; i < actions().size(); ++i)
  {
    QAction *action = actions()[i];
    
    if (action)
    {
      action->setEnabled(use);
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KProfilesMenu::slotActionTriggered(const QString& name)
{
  Smb4KProfileManager::self()->setActiveProfile(name);
}

