/*
    This class derives from QObject and encapsulates a profile item/name.
    It is for use with QtQuick.

    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KPROFILEOBJECT_H
#define SMB4KPROFILEOBJECT_H

// Qt includes
#include <QObject>
#include <QScopedPointer>
#include <QString>

// forward declarations
class Smb4KProfileObjectPrivate;

/**
 * This class derives from QObject and makes the name of a profile available
 * for use with QtQuick and Plasma.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.2.0
 */

class Q_DECL_EXPORT Smb4KProfileObject : public QObject
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
