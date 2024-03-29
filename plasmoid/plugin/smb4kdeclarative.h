/*
    This class provides the interface for Plasma and QtQuick

    SPDX-FileCopyrightText: 2013-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KDECLARATIVE_H
#define SMB4KDECLARATIVE_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QObject>
#include <QQmlListProperty>
#include <QQmlListReference>
#include <QUrl>

// forward declarations
class Smb4KDeclarativePrivate;
class Smb4KNetworkObject;
class Smb4KBookmarkObject;
class Smb4KProfileObject;

/**
 * This class provides the interface for programs written in QML to the core
 * classes of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */

class Q_DECL_EXPORT Smb4KDeclarative : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<Smb4KNetworkObject> workgroups READ workgroups NOTIFY workgroupsListChanged)
    Q_PROPERTY(QQmlListProperty<Smb4KNetworkObject> hosts READ hosts NOTIFY hostsListChanged)
    Q_PROPERTY(QQmlListProperty<Smb4KNetworkObject> shares READ shares NOTIFY sharesListChanged)
    Q_PROPERTY(QQmlListProperty<Smb4KNetworkObject> mountedShares READ mountedShares NOTIFY mountedSharesListChanged)
    Q_PROPERTY(QQmlListProperty<Smb4KBookmarkObject> bookmarks READ bookmarks NOTIFY bookmarksListChanged)
    Q_PROPERTY(QQmlListProperty<Smb4KBookmarkObject> bookmarkCategories READ bookmarkCategories NOTIFY bookmarksListChanged)
    Q_PROPERTY(QQmlListProperty<Smb4KProfileObject> profiles READ profiles NOTIFY profilesListChanged)
    Q_PROPERTY(QString activeProfile READ activeProfile WRITE setActiveProfile NOTIFY activeProfileChanged)
    Q_PROPERTY(bool profileUsage READ profileUsage NOTIFY profileUsageChanged)

    friend class Smb4KDeclarativePrivate;

public:
    /**
     * Constructor
     */
    explicit Smb4KDeclarative(QObject *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KDeclarative();

    /**
     * This function returns the list of workgroups. Basically, this is the
     * Smb4KGlobal::workgroupsList() list converted into a list of Smb4KNetworkItem
     * objects.
     *
     * @returns the list of discovered workgroups.
     */
    QQmlListProperty<Smb4KNetworkObject> workgroups();

    /**
     * This function returns the list of hosts. Basically, this is the
     * Smb4KGlobal::hostsList() list converted into a list of Smb4KNetworkItem
     * objects.
     *
     * @returns the list of discovered hosts.
     */
    QQmlListProperty<Smb4KNetworkObject> hosts();

    /**
     * This function returns the list of shares. Basically, this is the
     * Smb4KGlobal::sharesList() list converted into a list of Smb4KNetworkItem
     * objects.
     *
     * @returns the list of discovered shares.
     */
    QQmlListProperty<Smb4KNetworkObject> shares();

    /**
     * This function returns the list of mounted shares. Basically, this is the
     * Smb4KGlobal::mountedSharesList() list converted into a list of Smb4KNetworkItem
     * objects.
     *
     * @returns the list of the mounted shares.
     */
    QQmlListProperty<Smb4KNetworkObject> mountedShares();

    /**
     * This function returns the list of bookmarks. Basically, this is the
     * the list returned by Smb4KBookmarkHandler::bookmarksList() function
     * converted into a list of Smb4KBookmarkObject objects.
     *
     * @returns the list of bookmarks
     */
    QQmlListProperty<Smb4KBookmarkObject> bookmarks();

    /**
     * This function returns the list of bookmark categories. Basically, this is the
     * the list returned by the Smb4KBookmarkHandler::categoryList() function
     * converted into a list of Smb4KBookmarkObject objects.
     *
     * @returns the list of bookmarks
     */
    QQmlListProperty<Smb4KBookmarkObject> bookmarkCategories();

    /**
     * This function returns the list of profiles. Basically, this is the list
     * returned by the Smb4KProfileManager::profilesList() converted into a list
     * of Smb4KProfileObject objects.
     *
     * @returns the list of profiles
     */
    QQmlListProperty<Smb4KProfileObject> profiles();

    /**
     * This function takes a Smb4KNetworkObject object and initiates a network
     * scan. If you pass a NULL pointer, a network scan will be performed.
     *
     * Please note that this function only works with network objects that are
     * already known. All others will be ignored.
     *
     * @param object      The network object
     */
    Q_INVOKABLE void lookup(Smb4KNetworkObject *object = nullptr);

    /**
     * This function takes a QUrl object, looks up the respective network object
     * and returns it. If there is not such an object, NULL is returned.
     *
     * Please note that this function only works with network objects that are
     * already known. All others will be ignored.
     *
     * @param url         The URL of the network item
     *
     * @param type        The type of the network item
     *
     * @returns The network item or NULL if it was not found.
     */
    Q_INVOKABLE Smb4KNetworkObject *findNetworkItem(const QUrl &url, int type);

    /**
     * Open the mount dialog to mount a share.
     */
    Q_INVOKABLE void openMountDialog();

    /**
     * This function takes a network object and initiates the mounting of
     * the remote share.
     *
     * Please note that this function only works with network objects that
     * represent a share and that are already known, i.e. it must either be
     * a share that was already looked up during program run or one that was
     * bookmarked.
     *
     * @param object         The network object
     */
    Q_INVOKABLE void mountShare(Smb4KNetworkObject *object);

    /**
     * This function takes a bookmark object and initiates the mounting of
     * the respective remote share.
     *
     * @param object         The bookmark object
     */
    Q_INVOKABLE void mountBookmark(Smb4KBookmarkObject *object);

    /**
     * This function takes a network object and initiates the unmounting of
     * the mounted share.
     *
     * Please note that this function only works with network objects that
     * represent a share and that are already known.
     *
     * @param object         The network object
     */
    Q_INVOKABLE void unmount(Smb4KNetworkObject *object);

    /**
     * This function is a convenience function. It unmounts all currently mounted
     * shares by invoking @see unmountAllShares(0).
     */
    Q_INVOKABLE void unmountAll();

    /**
     * This function takes a QUrl object and checks if a share with this URL
     * is mounted. Returns nullptr if there is no such share.
     *
     * @param url         The URL to check
     */
    Q_INVOKABLE bool isShareMounted(const QUrl &url);

    /**
     * This function takes a network object representing a remote printer and
     * initiates the printing of a file.
     *
     * Please note that this function only works with share objects that are
     * already known. All others will be ignored.
     *
     * @param object        The network object representing a remote printer
     */
    Q_INVOKABLE void print(Smb4KNetworkObject *object);

    /**
     * This function adds a new bookmark.
     *
     * @param object        The network object that is to be bookmarked
     */
    Q_INVOKABLE void addBookmark(Smb4KNetworkObject *object);

    /**
     * This function removes a bookmark.
     *
     * @param url           The bookmark object that is to be removed
     */
    Q_INVOKABLE void removeBookmark(Smb4KBookmarkObject *object);

    /**
     * This function opens the bookmark editor.
     */
    Q_INVOKABLE void editBookmarks();

    /**
     * This function starts the synchronization of a local and a
     * remote folder.
     */
    Q_INVOKABLE void synchronize(Smb4KNetworkObject *object);

    /**
     * This function opens the custom options dialog.
     *
     * @param object              The network object
     */
    Q_INVOKABLE void openCustomOptionsDialog(Smb4KNetworkObject *object);

    /**
     * This function starts the client
     */
    Q_INVOKABLE void startClient();

    /**
     * This function stops the client
     */
    Q_INVOKABLE void abortClient();

    /**
     * This function starts the mounter.
     */
    Q_INVOKABLE void startMounter();

    /**
     * This function stops any action of the mounter.
     */
    Q_INVOKABLE void abortMounter();

    /**
     * Return the currently active profile or an empty string if
     * the use of profiles is disabled.
     *
     * @returns the active profile.
     */
    QString activeProfile() const;

    /**
     * Set the active profile.
     * @param profile       The name of the active profile
     */
    void setActiveProfile(const QString &profile);

    /**
     * Return the current setting of the profile usage.
     *
     * @returns the profile usage.
     */
    bool profileUsage() const;

    /**
     * Open the preview dialog with the contents of the passed network
     * item.
     * @param object        The network object
     */
    Q_INVOKABLE void preview(Smb4KNetworkObject *object);

    /**
     * Open the configuration dialog of the main application
     */
    Q_INVOKABLE void openConfigurationDialog();

protected:
    /**
     * Reimplemented from QObject
     */
    void timerEvent(QTimerEvent *event) override;

Q_SIGNALS:
    /**
     * This signal is emitted when the list of workgroups changed.
     */
    void workgroupsListChanged();

    /**
     * This signal is emitted when the list of hosts changed.
     */
    void hostsListChanged();

    /**
     * This signal is emitted when the list of shares changed.
     */
    void sharesListChanged();

    /**
     * This signal is emitted when the list of mounted shares changed.
     */
    void mountedSharesListChanged();

    /**
     * This signal is emitted when the list of bookmarks changed.
     */
    void bookmarksListChanged();

    /**
     * This signal is emitted when the list of profiles changed.
     */
    void profilesListChanged();

    /**
     * This signal is emitted when the active profile changed.
     */
    void activeProfileChanged();

    /**
     * This signal is emitted when the profile usage changed.
     */
    void profileUsageChanged();

    /**
     * This signal is emitted, when one of the core classes becomes
     * busy.
     */
    void busy();

    /**
     * This signal is emitted, when one of the core signals becomes
     * idle.
     */
    void idle();

protected Q_SLOTS:
    /**
     * This slot is invoked, when the list of workgroups was changed by
     * the scanner. It rebuilds the workgroups() list and emits the
     * worgroupsListChanged() signal.
     */
    void slotWorkgroupsListChanged();

    /**
     * This slot is invoked, when the list of hosts was changed by the
     * scanner. It rebuilds the hosts() list and emits the hostsListChanged()
     * signal.
     */
    void slotHostsListChanged();

    /**
     * This slot is invoked, when the list of shares was changed by the
     * scanner. It rebuilds the shares() list and emits the sharesListChanged()
     * signal.
     */
    void slotSharesListChanged();

    /**
     * This slot is invoked, when the list of mounted shares was changed
     * by the mounter. It rebuilds the mountedShares() list and emits the
     * mountedSharesListChanged() signal.
     */
    void slotMountedSharesListChanged();

    /**
     * This slot is invoked when the list of bookmarks was changed. It
     * rebuilds the bookmarks and bookmark groups lists and emits the
     * bookmarksListChanged() signal.
     */
    void slotBookmarksListChanged();

    /**
     * This slot is invoked when the list of profiles changed. It rebuils
     * the list of profiles and emits the profilesListChanged() signal.
     */
    void slotProfilesListChanged(const QStringList &profiles);

    /**
     * This slot is invoked when the active profile changed. It resets
     * the value for the active profile and emits the activeProfileChanged()
     * signal.
     */
    void slotActiveProfileChanged(const QString &activeProfile);

    /**
     * This slot is invoked when the profile usage changed. It resets
     * the value for the profile usage and emits the profileUsageChanged()
     * signal.
     */
    void slotProfileUsageChanged(bool use);

    /**
     * This slot is called when credentials are requested.
     *
     * @param networkItem         The network item
     */
    void slotCredentialsRequested(const NetworkItemPtr &networkItem);

private:
    const QScopedPointer<Smb4KDeclarativePrivate> d;
};

#endif
