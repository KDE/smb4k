/*
    Manage custom settings

    SPDX-FileCopyrightText: 2011-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCUSTOMOPTIONSMANAGER_H
#define SMB4KCUSTOMOPTIONSMANAGER_H

// application specific includes
#include "smb4kcore_export.h"
#include "smb4kglobal.h"

// Qt includes
#include <QObject>
#include <QScopedPointer>

// forward declarations
class Smb4KCustomSettingsManagerPrivate;

/**
 * This class manages the custom settings that were defined
 * for a certain share or host.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class SMB4KCORE_EXPORT Smb4KCustomSettingsManager : public QObject
{
    Q_OBJECT

    friend class Smb4KCustomSettingsManagerPrivate;

public:
    /**
     * Constructor
     */
    explicit Smb4KCustomSettingsManager(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KCustomSettingsManager();

    /**
     * Returns a static pointer to this class
     *
     * @returns a static pointer to this class
     */
    static Smb4KCustomSettingsManager *self();

    /**
     * Add the share to the list of shares that are to be remounted
     * either only on next program start or always Smb4K is restarted.
     *
     * @param share     The share object
     *
     * @param always    If set to TRUE the share is always mounted
     *                  when Smb4K is restarted.
     */
    void addRemount(const SharePtr &share, bool always = false);

    /**
     * Remove the share @p share from the list of shares that are to be
     * remounted. If @p force is set to TRUE, the share is removed even
     * if it should always be removed (option is set to
     * Smb4KCustomSettings::AlwaysRemount). Apart from that, the share is only
     * removed when the option is set to Smb4KCustomSettings::DoRemount.
     *
     * @param share     The share object
     *
     * @param force     If set to TRUE, the share is removed regardless of the
     *                  remount setting.
     */
    void removeRemount(const SharePtr &share, bool force = false);

    /**
     * Removes all remounts from the list of custom settings. If @p force
     * is set to TRUE, even those are removed that should always be remounted.
     *
     * @param force     If set to TRUE, even those shares are removed that should
     *                  always be remounted.
     */
    void clearRemounts(bool force);

    /**
     * Returns the list of shares that are to be remounted.
     *
     * @returns the list of shares that are to be remounted
     */
    QList<CustomSettingsPtr> sharesToRemount();

    /**
     * Find custom settings for the network item @p networkItem.
     *
     * If the network item represents a share and custom settings for it are not
     * defined, but for the host that provides the share, the custom settings of
     * the host are returned. If neither is in the list, NULL is returned.
     *
     * If you set @p exactMatch to TRUE, NULL will be returned if the URL is not found.
     * Except in some special cases, you should not set @p exactMatch to true,
     * because settings that are defined for all shares provided by a certain host and
     * stored in a host-type custom settings object are ignored then.
     *
     * @param networkItem         The network item
     * @param exactMatch          If TRUE, only exact matches are returned
     *
     * @returns the custom settings for the network item
     */
    CustomSettingsPtr findCustomSettings(const NetworkItemPtr &networkItem, bool exactMatch = false);

    /**
     * Find custom settings for the provided @p url.
     *
     * This function searches the list of custom settings and compares the host entry
     * and, if applicable, the path (i.e. the share name). If an exact match was found,
     * the corresponding custom settings are returned.
     *
     * @param url                 The network item's URL
     *
     * @returns the custom settings
     */
    CustomSettingsPtr findCustomSettings(const QUrl &url);

    /**
     * Get the list of custom settings. If @p withoutRemountOnce is defined, only those
     * entries are returned that have custom settings apart from the remount settings
     * defined.
     *
     * @param withoutRemountOnce  Returns the list of custom settings without those that
     *                            have only the one time remount option set.
     *
     * @returns the list of custom settings objects.
     */
    QList<CustomSettingsPtr> customSettings(bool withoutRemountOnce = false) const;

    /**
     * This function adds custom settings for a single network item to the list
     * of custom settings. If there already are custom settings defined for the
     * network item, they are updated.
     *
     * Please note that this function will store a copy of @p settings and not
     * the original object.
     *
     * @param settings            The custom settings
     */
    void addCustomSettings(const CustomSettingsPtr &settings);

    /**
     * This function removes custom settings for a single network item from the
     * list of options.
     *
     * @param settings            The custom settings
     */
    void removeCustomSettings(const CustomSettingsPtr &settings);

    /**
     * This function returns a list of custom settings objects that have
     * Wake-On-LAN features defined.
     *
     * @returns a list of custom settings objects with WOL features defined.
     */
    QList<CustomSettingsPtr> wakeOnLanEntries() const;

    /**
     * Save custom settings to the file.
     *
     * @param settingsList        The list of custom settings
     */
    void saveCustomSettings(const QList<CustomSettingsPtr> &settingsList);

Q_SIGNALS:
    /**
     * Emitted when the list of custom settings was updated
     */
    void updated();

protected Q_SLOTS:
    /**
     * Called when a profile was removed
     *
     * @param name          The name of the profile
     */
    void slotProfileRemoved(const QString &name);

    /**
     * Called when a profile was migrated
     *
     * @param oldName       The old profile name
     * @param newName       The new profile name
     */
    void slotProfileMigrated(const QString &oldName, const QString &newName);

    /**
     * Called when the active profile changed
     *
     * @param name          Profile name
     */
    void slotActiveProfileChanged(const QString &name);

private:
    /**
     * Add custom settings
     */
    bool add(const CustomSettingsPtr &settings);

    /**
     * Remove custom settings
     */
    bool remove(const CustomSettingsPtr &settings);

    /**
     * Read custom settings
     */
    void read();

    /**
     * Write custom settings
     */
    void write();

    /**
     * Pointer to Smb4KCustomSettingsManagerPrivate class
     */
    const QScopedPointer<Smb4KCustomSettingsManagerPrivate> d;
};

#endif
