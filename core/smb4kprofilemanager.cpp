/***************************************************************************
    smb4kprofilemanager  -  This class manages the profiles that were
    defined by the user.
                             -------------------
    begin                : Mi Aug 06 2014
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
#include "smb4kprofilemanager.h"
#include "smb4kprofilemanager_p.h"
#include "smb4ksettings.h"

// KDE includes
#include <kglobal.h>


K_GLOBAL_STATIC(Smb4KProfileManagerStatic, p);


Smb4KProfileManager::Smb4KProfileManager(QObject* parent)
: QObject(parent), d(new Smb4KProfileManagerPrivate)
{
  d->forceSilence = false;
  d->useProfiles  = Smb4KSettings::useProfiles();
  
  if (d->useProfiles)
  {
    d->profiles = Smb4KSettings::profilesList();
    d->activeProfile = !Smb4KSettings::activeProfile().isEmpty() ? 
                       Smb4KSettings::activeProfile() : 
                       d->profiles.first();
  }
  else
  {
    d->profiles.clear();
    d->activeProfile.clear();
  }
  
  connect(Smb4KSettings::self(), SIGNAL(configChanged()), 
          this,                  SLOT(slotConfigChanged()));
}


Smb4KProfileManager::~Smb4KProfileManager()
{
}


Smb4KProfileManager* Smb4KProfileManager::self()
{
  return &p->instance;
}


bool Smb4KProfileManager::setActiveProfile(const QString& name)
{
  bool changed = false;
  
  if (d->useProfiles)
  {
    if (QString::compare(name, d->activeProfile, Qt::CaseSensitive) != 0)
    {
      d->activeProfile = name;
      changed = true;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    if (!d->activeProfile.isEmpty())
    {
      d->activeProfile.clear();
      changed = true;
    }
    else
    {
      // Do nothing
    }
  }
    
  if (changed)
  {
    Smb4KSettings::setActiveProfile(d->activeProfile);
    Smb4KSettings::self()->writeConfig();
    
    if (!d->forceSilence)
    {
      emit settingsChanged();
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
  
  return changed;
}


QString Smb4KProfileManager::activeProfile() const
{
  return d->activeProfile;
}


QStringList Smb4KProfileManager::profilesList() const
{
  return d->useProfiles ? d->profiles : QStringList();
}


bool Smb4KProfileManager::useProfiles() const
{
  return d->useProfiles;
}


void Smb4KProfileManager::slotConfigChanged()
{
  bool use_changed, profiles_changed, active_changed = false;
  
  // Use of profiles
  if (d->useProfiles != Smb4KSettings::useProfiles())
  {
    d->useProfiles = Smb4KSettings::useProfiles();
    use_changed    = true;
  }
  else
  {
    // Do nothing
  }
  
  // List of profiles
  if (d->profiles != Smb4KSettings::profilesList())
  {
    d->profiles = Smb4KSettings::profilesList();
    profiles_changed = true;
  }
  else
  {
    // Do nothing
  }

  // Active profile
  d->forceSilence = true;
  
  if (!Smb4KSettings::activeProfile().isEmpty() &&
      d->profiles.contains(Smb4KSettings::activeProfile()))
  {
    active_changed = setActiveProfile(Smb4KSettings::activeProfile());
  }
  else
  {
    active_changed = setActiveProfile(d->profiles.first());
  }
  
  d->forceSilence = false;
  
  // Emission of signal
  if (use_changed || profiles_changed || active_changed)
  {
    emit settingsChanged();
  }
  else
  {
    // Do nothing
  }
}


#include "smb4kprofilemanager.moc"
