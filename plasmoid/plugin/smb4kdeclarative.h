/***************************************************************************
    This class provides the interface for Plasma and QtQuick
                             -------------------
    begin                : Mo 02 Sep 2013
    copyright            : (C) 2013-2019 by Alexander Reinholdt
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

#ifndef SMB4KDECLARATIVE_H
#define SMB4KDECLARATIVE_H

// Qt includes
#include <QObject>
#include <QUrl>
#include <QQmlListProperty>
#include <QQmlListReference>

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
  Q_PROPERTY(QQmlListProperty<Smb4KBookmarkObject> bookmarkGroups READ bookmarkGroups NOTIFY bookmarksListChanged)
  Q_PROPERTY(QQmlListProperty<Smb4KProfileObject> profiles READ profiles NOTIFY profilesListChanged)
  Q_PROPERTY(QString activeProfile READ activeProfile WRITE setActiveProfile NOTIFY activeProfileChanged)
  Q_PROPERTY(bool profileUsage READ profileUsage NOTIFY profileUsageChanged);
  
  friend class Smb4KDeclarativePrivate;
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KDeclarative(QObject *parent = 0);
    
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
     * This function returns the list of bookmark groups. Basically, this is the 
     * the list returned by the Smb4KBookmarkHandler::groupsList() function 
     * converted into a list of Smb4KBookmarkObject objects.
     * 
     * @returns the list of bookmarks
     */
    QQmlListProperty<Smb4KBookmarkObject> bookmarkGroups();
    
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
    Q_INVOKABLE void lookup(Smb4KNetworkObject *object = 0);
    
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
     * This function takes a QUrl object, looks up the respective mounted share
     * and returns it. If there is not such a share, NULL is returned.
     * 
     * @param url         The URL of the mounted share
     * 
     * @param exactMatch  Determines if the function should only search for the 
     *                    exact match or if it may also except matches where the
     *                    user info and port may differ.
     * 
     * @returns The mounted share or NULL if it was not found.
     */
    Q_INVOKABLE Smb4KNetworkObject *findMountedShare(const QUrl &url, bool exactMatch = true);
    
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
    
  private:
    const QScopedPointer<Smb4KDeclarativePrivate> d;
};


#endif
