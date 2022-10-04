/*
    This class manages the profiles that were defined by the user.

    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KPROFILEMANAGER_H
#define SMB4KPROFILEMANAGER_H

// Qt includes
#include <QObject>
#include <QPair>
#include <QScopedPointer>
#include <QString>
#include <QStringList>

// forward declarations
class Smb4KProfileManagerPrivate;

/**
 * This class "manages" the profiles defined by the user in such a
 * degree as it sends signals when the active profile changed, a
 * profile was renamed or removed. You can also actively initiate the
 * migration of or remove a profile.
 *
 * When using profiles, please use this class instead of the KConfig XT
 * class(es).
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.2.0
 */

class Q_DECL_EXPORT Smb4KProfileManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KProfileManager(QObject *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KProfileManager();

    /**
     * Returns a static pointer to this class.
     *
     * @returns a static pointer to this class.
     */
    static Smb4KProfileManager *self();

    /**
     * Set the active profile if the use of profiles is enabled.
     * Otherwise the this function does nothing.
     *
     * @param name        Name of the active profile.
     *
     * @returns true if the active profile was changed.
     */
    void setActiveProfile(const QString &name);

    /**
     * Return the currently active profile or an empty string if
     * the use of profiles is disabled.
     *
     * @returns the active profile.
     */
    QString activeProfile() const;

    /**
     * Returns the list of profiles or an empty string list if the
     * the use of profiles is disabled.
     *
     * @returns the list of profiles.
     */
    QStringList profilesList() const;

    /**
     * Returns if profiles should be used or not. This is basically
     * a convenience function, since it just returns
     * Smb4KSettings::useProfiles().
     *
     * @returns true if profiles should be used.
     */
    bool useProfiles() const;

    /**
     * Migrate all entries of one profile to another.
     *
     * @param from        The name of the old profile.
     * @param to          The name of the new profile.
     */
    void migrateProfile(const QString &from, const QString &to);

    /**
     * Migrate all entries of a list of profiles to other profiles.
     *
     * @param list        The list of profile pairs. The first entry
     *                    is the "from" profile, the second one the
     *                    "to" profile.
     */
    void migrateProfiles(const QList<QPair<QString, QString>> &list);

    /**
     * Remove a profile with all of its entries.
     *
     * @param name        The name of the profile.
     */
    void removeProfile(const QString &name);

    /**
     * Remove a list of profiles with all of their entries.
     *
     * @param list        The list of profile names.
     */
    void removeProfiles(const QStringList &list);

Q_SIGNALS:
    /**
     * This signal is emitted when all entries of one profile was migrated
     * to another one.
     *
     * @param from        The old profile
     * @param to          The new profile
     */
    void migratedProfile(const QString &from, const QString &to);

    /**
     * This signal is emitted when a profile was removed.
     *
     * @param profile     The removed profile
     */
    void removedProfile(const QString &profile);

    /**
     * This signal is emitted when the active profile is about
     * to be changed. You should connect to this signal, if you need
     * to save settings or the like to the OLD profile.
     */
    void aboutToChangeProfile();

    /**
     * This signal is emitted when the active profile changed.
     *
     * @param newProfile  The name of the new profile
     */
    void activeProfileChanged(const QString &newProfile);

    /**
     * This signal is emitted when the list of profiles changed.
     *
     * @param profiles    The list of profiles
     */
    void profilesListChanged(const QStringList &profiles);

    /**
     * This signal is emitted when the usage of profiles is switched
     * on or off.
     *
     * @param use           TRUE if profiles are used and FALSE otherwise
     */
    void profileUsageChanged(bool use);

protected Q_SLOTS:
    /**
     * This slot is connected to the configChanged() signal of the
     * configuration object of the core.
     */
    void slotConfigChanged();

private:
    /**
     * Pointer to Smb4KBookmarkHandlerPrivate class
     */
    const QScopedPointer<Smb4KProfileManagerPrivate> d;
};

#endif
