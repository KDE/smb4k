/***************************************************************************
    smb4kprofileobject - This class derives from QObject and
    encapsulates a profile item/name. It is for use with QtQuick.
                             -------------------
    begin                : So 23 Nov 2014
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

#ifndef SMB4KPROFILEOBJECT_H
#define SMB4KPROFILEOBJECT_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KProfileObjectPrivate;

/**
 * This class derives from QObject and makes the name of a profile available
 * for use with QtQuick and Plasma.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.2.0
 */ 

class KDE_EXPORT Smb4KProfileObject : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString profileName READ profileName WRITE setProfileName NOTIFY changed)
  Q_PROPERTY(bool isActiveProfile READ isActiveProfile WRITE setActiveProfile NOTIFY changed)
  
  friend class Smb4KProfileObjectPrivate;
  
  public:
    /**
     * The constructor
     * @param parent        The parent of this item
     */
    explicit Smb4KProfileObject(QObject *parent = 0);
  
    /**
     * The destructor
     */
    virtual ~Smb4KProfileObject();
    
    /**
     * This function returns the name of the profile.
     * @returns the name of the profile
     */
    QString profileName() const;
    
    /**
     * This function sets the name of the profile.
     * @param profileName   The name of the profile
     */
    void setProfileName(const QString &profileName);
    
    /**
     * This function returns TRUE if this is the active 
     * profile and FALSE otherwise.
     * @returns TRUE if this is the active profile.
     */
    bool isActiveProfile() const;
    
    /**
     * With this function you can mark this profile as the
     * active one.
     * @param active        Set this as the active profile
     */
    void setActiveProfile(bool active);
    
  Q_SIGNALS:
    /**
     * This signal is emitted if anything changed.
     */
    void changed();
    
  private:
    QScopedPointer<Smb4KProfileObjectPrivate> d;
};

#endif

