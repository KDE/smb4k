/*
    Manage custom options

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCUSTOMOPTIONSMANAGER_H
#define SMB4KCUSTOMOPTIONSMANAGER_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QObject>
#include <QScopedPointer>

// forward declarations
class Smb4KCustomOptionsManagerPrivate;
class Smb4KProfileManager;

/**
 * This classes manages the custom options that were defined
 * for a certain share or host.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class Q_DECL_EXPORT Smb4KCustomOptionsManager : public QObject
{
    Q_OBJECT

    friend class Smb4KCustomOptionsManagerPrivate;
    friend class Smb4KProfileManager;

public:
    /**
     * Constructor
     */
    explicit Smb4KCustomOptionsManager(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KCustomOptionsManager();

    /**
     * Returns a static pointer to this class
     *
     * @returns a static pointer to this class
     */
    static Smb4KCustomOptionsManager *self();

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
     * Smb4KCustomOptions::AlwaysRemount). Apart from that, the share is only
     * removed when the option is set to Smb4KCustomOptions::DoRemount.
     *
     * @param share     The share object
     *
     * @param force     If set to TRUE, the share is removed regardless of the
     *                  remount setting.
     */
    void removeRemount(const SharePtr &share, bool force = false);

    /**
     * Removes all remounts from the list of custom options. If @p force
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
    QList<OptionsPtr> sharesToRemount();

    /**
     * Find custom options for the network item @p networkItem.
     *
     * If the network item represents a share and custom options for it are not
     * defined, but for the host that provides the share, options filled with the
     * host's options are returned. If neither is in the list, NULL is returned.
     *
     * If you set @p exactMatch to TRUE, NULL will be returned if the URL is not found.
     * Except in some special cases, you should not set @p exactMatch to true,
     * because options that are defined for all shares provided by a certain host and
     * stored in a host-type custom options object are ignored then.
     *
     * @param networkItem         The network item
     * @param exactMatch          If TRUE, only exact matches are returned
     *
     * @returns the custom options for the network item
     */
    OptionsPtr findOptions(const NetworkItemPtr &networkItem, bool exactMatch = false);

    /**
     * Find custom options for the provided @p url.
     *
     * This function searches the list of custom options and compares the host entry
     * and, if applicable, the path (i.e. the share name). If an exact match was found,
     * the corresponding custom options are returned.
     *
     * @param url                 The network item's URL
     *
     * @returns the custom options
     */
    OptionsPtr findOptions(const QUrl &url);

    /**
     * Get the list of custom options. If @p withoutRemountOnce is defined, only those
     * entries are returned that have custom options apart from the remount settings
     * defined.
     *
     * @param withoutRemountOnce  Returns the list of custom options without those that
     *                            have only the one time remount option set.
     *
     * @returns the list of custom options objects.
     */
    QList<OptionsPtr> customOptions(bool withoutRemountOnce = false) const;

    /**
     * This function opens the custom options dialog for a network item.
     *
     * @param item                The network item
     */
    void openCustomOptionsDialog(const NetworkItemPtr &item);

    /**
     * This function opens a custom options dialog for an options pointer.
     *
     * @param options           The custom options object
     *
     * @param write             Write the options to the file
     *
     * @returns TRUE if there are custom options defined, otherwise FALSE.
     */
    bool openCustomOptionsDialog(const OptionsPtr &options, bool write = true);

    /**
     * This function adds custom options for a single network item to the list
     * of options. If there already are options defined for that network item,
     * they are updated.
     *
     * Please note that this function will store a copy of @p options and not
     * the original object.
     *
     * @param options             The custom options
     *
     * @param write               Write the options to the file
     */
    void addCustomOptions(const OptionsPtr &options, bool write = false);

    /**
     * This function removes custom options for a single network item from the
     * list of options.
     *
     * @param options             The custom options
     *
     * @param write               Write the options to the file
     */
    void removeCustomOptions(const OptionsPtr &options, bool write = false);

    /**
     * This function returns a list of custom option objects that have
     * Wake-On-LAN features defined.
     *
     * @returns a list of custom options objects with WOL features defined.
     */
    QList<OptionsPtr> wakeOnLanEntries() const;

    /**
     * Reload custom options from the file.
     */
    void resetCustomOptions();

    /**
     * Save custom options to the file.
     */
    void saveCustomOptions();

protected Q_SLOTS:
    /**
     * Called when the application exits
     */
    void slotAboutToQuit();

private:
    /**
     * Read custom options
     */
    void readCustomOptions();

    /**
     * This function writes the custom options to the disk.
     */
    void writeCustomOptions();

    /**
     * Migrates one profile to another.
     *
     * This function is meant to be used by the profile manager.
     *
     * @param from        The name of the old profile.
     * @param to          The name of the new profile.
     */
    void migrateProfile(const QString &from, const QString &to);

    /**
     * Removes a profile from the list of profiles.
     *
     * This function is meant to be used by the profile manager.
     *
     * @param name        The name of the profile.
     */
    void removeProfile(const QString &name);

    /**
     * Pointer to Smb4KCustomOptionsManagerPrivate class
     */
    const QScopedPointer<Smb4KCustomOptionsManagerPrivate> d;
};

#endif
