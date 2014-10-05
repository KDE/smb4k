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
#include <QtTest/QTest>

// KDE includes
#include <kglobal.h>


K_GLOBAL_STATIC(Smb4KProfileManagerStatic, p);


//
// NOTE: Do not invoke writeConfig() here, because this will/might
// trigger the configChanged() signal which can lead to unwanted
// effects.
//


Smb4KProfileManager::Smb4KProfileManager(QObject* parent)
: QObject(parent), d(new Smb4KProfileManagerPrivate)
{
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


bool Smb4KProfileManager::setActiveProfile(const QString& name, bool noSignal)
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
    
    if (!noSignal)
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
  QList< QPair<QString,QString> > list;
  list << QPair<QString,QString>(from, to);
  migrateProfiles(list);
}


void Smb4KProfileManager::migrateProfiles(const QList< QPair<QString,QString> >& list)
{
  if (d->useProfiles || (list.size() == 1 && list.first().second.isEmpty()))
  {
    for (int i = 0; i < list.size(); ++i)
    {
      QString from = list.at(i).first;
      QString to = list.at(i).second;
      
      if (!to.isEmpty()) 
      {
        // Migrate one/the default profile to another one.
        // 
        // First exchange the old profile.
        for (int j = 0; j < d->profiles.size(); ++j)
        {
          if (QString::compare(from, d->profiles.at(j), Qt::CaseSensitive) == 0)
          {
            d->profiles.replace(j, to);
            break;
          }
          else
          {
            // Do nothing
          }
        }
        
        // In case the active profile was modified, rename it according
        // the value passed.
        if (QString::compare(from, d->activeProfile, Qt::CaseSensitive) == 0)
        {
          (void)setActiveProfile(to, true);
        }
        else
        {
          // Do nothing
        }
      
        // Migrate profiles.
        Smb4KBookmarkHandler::self()->migrateProfile(from, to);
        Smb4KCustomOptionsManager::self()->migrateProfile(from, to);
        Smb4KHomesSharesHandler::self()->migrateProfile(from, to);
        emit migratedProfile(from, to);
      }
      else
      {
        // Migrate all profiles to the default one.
        for (int j = 0; j < d->profiles.size(); ++j)
        {
          Smb4KBookmarkHandler::self()->migrateProfile(d->profiles.at(j), to);
          Smb4KCustomOptionsManager::self()->migrateProfile(d->profiles.at(j), to);
          Smb4KHomesSharesHandler::self()->migrateProfile(d->profiles.at(j), to);
          emit migratedProfile(d->profiles.at(i), to);        
        }
      }
    }
    
    Smb4KSettings::setProfilesList(d->profiles);
    emit settingsChanged();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KProfileManager::removeProfile(const QString& name, QWidget* parent)
{
  QStringList list;
  list << name;
  removeProfiles(list, parent);
}


void Smb4KProfileManager::removeProfiles(const QStringList& list, QWidget* parent)
{
  if (d->useProfiles)
  {
    for (int i = 0; i < list.size(); ++i)
    {
      QString name = list.at(i);
      
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
          (void)setActiveProfile(d->profiles.first(), true);
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
    }
    
    Smb4KSettings::setProfilesList(d->profiles);
    emit settingsChanged();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KProfileManager::slotConfigChanged()
{
  bool use_changed = false;
  bool profiles_changed = false;
  bool active_changed = false;
  
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
  if (!Smb4KSettings::activeProfile().isEmpty() &&
      d->profiles.contains(Smb4KSettings::activeProfile()))
  {
    active_changed = setActiveProfile(Smb4KSettings::activeProfile(), true);
  }
  else
  {
    active_changed = setActiveProfile(d->profiles.first(), true);
  }
  
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
  
  // Emission of signal.
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
