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
#include "smb4khomesshareshandler.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kbookmarkhandler.h"

// Qt includes
#include <QtCore/QPointer>

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


void Smb4KProfileManager::migrateProfile(const QString& from, const QString& to)
{
  if (d->useProfiles || to.isEmpty())
  {
    if (!to.isEmpty()) 
    {
      // Migrate one/the defaulr profile to another one.
      // 
      // First exchange the old profile.
      for (int i = 0; i < d->profiles.size(); ++i)
      {
        if (QString::compare(from, d->profiles.at(i), Qt::CaseSensitive) == 0)
        {
          d->profiles.replace(i, to);
          break;
        }
        else
        {
          // Do nothing
        }
      }
      
      Smb4KSettings::setProfilesList(d->profiles);
      
      // In case the active profile was modified, rename it according
      // the value passed.
      if (QString::compare(from, d->activeProfile, Qt::CaseSensitive) == 0)
      {
        (void)setActiveProfile(to);
      }
      else
      {
        // Do nothing
      }
      
      // Migrate profiles.
      Smb4KBookmarkHandler::self()->migrateProfile(from, to);
      Smb4KCustomOptionsManager::self()->migrateProfile(from, to);
      Smb4KHomesSharesHandler::self()->migrateProfile(from, to);
        
      emit profileMigrated(from, to);
    }
    else
    {
      // Migrate all profiles to the default one.
      for (int i = 0; i < d->profiles.size(); ++i)
      {
        Smb4KBookmarkHandler::self()->migrateProfile(d->profiles.at(i), to);
        Smb4KCustomOptionsManager::self()->migrateProfile(d->profiles.at(i), to);
        Smb4KHomesSharesHandler::self()->migrateProfile(d->profiles.at(i), to);
          
        emit profileMigrated(d->profiles.at(i), to);        
      }
    }
    
    emit settingsChanged();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KProfileManager::removeProfile(const QString& name, QWidget* parent)
{
  if (d->useProfiles)
  {
    // First remove the profile from the list.
    QMutableStringListIterator it(d->profiles);
    
    while (it.hasNext())
    {
      QString entry = it.next();
      
      if (QString::compare(name, entry, Qt::CaseSensitive) == 0)
      {
        it.remove();
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    Smb4KSettings::setProfilesList(d->profiles);
 
    if (!d->profiles.isEmpty())
    {
      // Ask the user if he/she wants to migrate the entries 
      // of the removed profile to another one.
      if (Smb4KSettings::useMigrationAssistant())
      {
        QPointer<Smb4KProfileMigrationDialog> dlg = new Smb4KProfileMigrationDialog(QStringList(name), d->profiles, parent);
          
        if (dlg->exec() == KDialog::Accepted)
        {
          migrateProfile(dlg->from(), dlg->to());
        }
        else
        {
          // Do nothing
        }
        
        delete dlg;
      }
      else
      {
        // Do nothing
      }
      
      // Set a new active profile if the user removed the current one.
      if (QString::compare(name, d->activeProfile, Qt::CaseSensitive) == 0)
      {
        (void)setActiveProfile(d->profiles.first());
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
    
    // Remove the profile.
    Smb4KBookmarkHandler::self()->removeProfile(name);
    Smb4KCustomOptionsManager::self()->removeProfile(name);
    Smb4KHomesSharesHandler::self()->removeProfile(name);
    
    emit removedProfile(name);
    emit settingsChanged();
  }
  else
  {
    // Do nothing
  }
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
  
  // Migrate profile(s), if necessary.
  if (use_changed && Smb4KSettings::useMigrationAssistant())
  {
    QStringList from, to;
    
    if (d->useProfiles)
    {
      // Since the setting changed, the use of profiles was 
      // switched off before. So, ask the user if he/she wants
      // to migrate the default profile to any other one.
      // Therefore, from needs to get one empty entry (default
      // profile) and to has to be d->profiles.
      from << QString();
      to << d->profiles;
    }
    else
    {
      // Here it is vice versa: Ask the user if he/she wants to
      // migrate all profiles to the default profile. Therefore,
      // set from to d->profiles and to to empty.
      from << d->profiles;
      to << QString();
    }
    
    // Now, launch the migration dialog.
    QPointer<Smb4KProfileMigrationDialog> dlg = new Smb4KProfileMigrationDialog(from, to, 0);
          
    if (dlg->exec() == KDialog::Accepted)
    {
      // FIXME: Make sure that migrateProfile() does the right job.
      migrateProfile(dlg->from(), dlg->to());
    }
    else
    {
      // Do nothing
    }
        
    delete dlg;
  }
  else
  {
    // Do nothing
  }
  
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
