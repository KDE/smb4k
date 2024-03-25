/*
    This class provides the interface for Plasma and QtQuick

    SPDX-FileCopyrightText: 2013-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kdeclarative.h"
#include "core/smb4kbasicnetworkitem.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kclient.h"
#include "core/smb4khost.h"
#include "core/smb4kmounter.h"
#include "core/smb4knotification.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4kshare.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kworkgroup.h"
#include "smb4k/smb4kcustomsettingseditor.h"
#include "smb4k/smb4khomesuserdialog.h"
#include "smb4k/smb4kpassworddialog.h"
#include "smb4k/smb4kpreviewdialog.h"
#include "smb4k/smb4kprintdialog.h"
#include "smb4k/smb4ksynchronizationdialog.h"
#include "smb4kbookmarkdialog.h"
#include "smb4kbookmarkeditor.h"
#include "smb4kbookmarkobject.h"
#include "smb4kmountdialog.h"
#include "smb4knetworkobject.h"
#include "smb4kprofileobject.h"

//
// Qt includes
#include <QDebug>
#include <QPointer>

// KDE includes
#include <KConfigDialog>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPluginMetaData>

class Smb4KDeclarativePrivate
{
public:
    QList<Smb4KNetworkObject *> workgroupObjects;
    QList<Smb4KNetworkObject *> hostObjects;
    QList<Smb4KNetworkObject *> shareObjects;
    QList<Smb4KNetworkObject *> mountedObjects;
    QList<Smb4KBookmarkObject *> bookmarkObjects;
    QList<Smb4KBookmarkObject *> bookmarkCategoryObjects;
    QList<Smb4KProfileObject *> profileObjects;
    QList<NetworkItemPtr> requestQueue;
    QPointer<Smb4KPasswordDialog> passwordDialog;
    int timerId;
};

Smb4KDeclarative::Smb4KDeclarative(QObject *parent)
    : QObject(parent)
    , d(new Smb4KDeclarativePrivate)
{
    d->passwordDialog = new Smb4KPasswordDialog();
    d->timerId = 0;

    Smb4KNotification::setComponentName(QStringLiteral("smb4k"));

    connect(Smb4KClient::self(), &Smb4KClient::workgroups, this, &Smb4KDeclarative::slotWorkgroupsListChanged);
    connect(Smb4KClient::self(), &Smb4KClient::hosts, this, &Smb4KDeclarative::slotHostsListChanged);
    connect(Smb4KClient::self(), &Smb4KClient::shares, this, &Smb4KDeclarative::slotSharesListChanged);
    connect(Smb4KClient::self(), &Smb4KClient::aboutToStart, this, &Smb4KDeclarative::busy);
    connect(Smb4KClient::self(), &Smb4KClient::finished, this, &Smb4KDeclarative::idle);
    connect(Smb4KClient::self(), &Smb4KClient::requestCredentials, this, &Smb4KDeclarative::slotCredentialsRequested);

    connect(Smb4KMounter::self(), &Smb4KMounter::mountedSharesListChanged, this, &Smb4KDeclarative::slotMountedSharesListChanged);
    connect(Smb4KMounter::self(), &Smb4KMounter::aboutToStart, this, &Smb4KDeclarative::busy);
    connect(Smb4KMounter::self(), &Smb4KMounter::finished, this, &Smb4KDeclarative::idle);
    connect(Smb4KMounter::self(), &Smb4KMounter::requestCredentials, this, &Smb4KDeclarative::slotCredentialsRequested);

    connect(Smb4KBookmarkHandler::self(), &Smb4KBookmarkHandler::updated, this, &Smb4KDeclarative::slotBookmarksListChanged);

    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profilesListChanged, this, &Smb4KDeclarative::slotProfilesListChanged);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::activeProfileChanged, this, &Smb4KDeclarative::slotActiveProfileChanged);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileUsageChanged, this, &Smb4KDeclarative::slotProfileUsageChanged);

    //
    // Do the initial loading of items
    //
    slotBookmarksListChanged();
    slotProfilesListChanged(Smb4KProfileManager::self()->profilesList());
    slotActiveProfileChanged(Smb4KProfileManager::self()->activeProfile());
    slotProfileUsageChanged(Smb4KProfileManager::self()->useProfiles());
}

Smb4KDeclarative::~Smb4KDeclarative()
{
    qDeleteAll(d->workgroupObjects);
    d->workgroupObjects.clear();

    qDeleteAll(d->hostObjects);
    d->hostObjects.clear();

    qDeleteAll(d->shareObjects);
    d->shareObjects.clear();

    qDeleteAll(d->mountedObjects);
    d->mountedObjects.clear();

    qDeleteAll(d->bookmarkObjects);
    d->bookmarkObjects.clear();

    qDeleteAll(d->bookmarkCategoryObjects);
    d->bookmarkCategoryObjects.clear();

    qDeleteAll(d->profileObjects);
    d->profileObjects.clear();
}

QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::workgroups()
{
    return QQmlListProperty<Smb4KNetworkObject>(this, &d->workgroupObjects);
}

QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::hosts()
{
    return QQmlListProperty<Smb4KNetworkObject>(this, &d->hostObjects);
}

QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::shares()
{
    return QQmlListProperty<Smb4KNetworkObject>(this, &d->shareObjects);
}

QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::mountedShares()
{
    return QQmlListProperty<Smb4KNetworkObject>(this, &d->mountedObjects);
}

QQmlListProperty<Smb4KBookmarkObject> Smb4KDeclarative::bookmarks()
{
    return QQmlListProperty<Smb4KBookmarkObject>(this, &d->bookmarkObjects);
}

QQmlListProperty<Smb4KBookmarkObject> Smb4KDeclarative::bookmarkCategories()
{
    return QQmlListProperty<Smb4KBookmarkObject>(this, &d->bookmarkCategoryObjects);
}

QQmlListProperty<Smb4KProfileObject> Smb4KDeclarative::profiles()
{
    return QQmlListProperty<Smb4KProfileObject>(this, &d->profileObjects);
}

void Smb4KDeclarative::lookup(Smb4KNetworkObject *object)
{
    if (object) {
        switch (object->type()) {
        case Smb4KNetworkObject::Network: {
            Smb4KClient::self()->lookupDomains();
            break;
        }
        case Smb4KNetworkObject::Workgroup: {
            // Check if the workgroup is known.
            WorkgroupPtr workgroup = Smb4KGlobal::findWorkgroup(object->url().host().toUpper());

            if (workgroup) {
                Smb4KClient::self()->lookupDomainMembers(workgroup);
            }

            break;
        }
        case Smb4KNetworkObject::Host: {
            // Check if the host is known.
            HostPtr host = Smb4KGlobal::findHost(object->url().host().toUpper());

            if (host) {
                Smb4KClient::self()->lookupShares(host);
            }

            break;
        }
        case Smb4KNetworkObject::Share: {
            break;
        }
        default: {
            // Shares are ignored
            break;
        }
        }
    } else {
        // If the object is 0, scan the whole network.
        Smb4KClient::self()->lookupDomains();
    }
}

Smb4KNetworkObject *Smb4KDeclarative::findNetworkItem(const QUrl &url, int type)
{
    Smb4KNetworkObject *object = nullptr;

    if (url.isValid()) {
        switch (type) {
        case Smb4KNetworkObject::Workgroup: {
            for (Smb4KNetworkObject *obj : qAsConst(d->workgroupObjects)) {
                if (url == obj->url()) {
                    object = obj;
                    break;
                } else {
                    continue;
                }
            }
            break;
        }
        case Smb4KNetworkObject::Host: {
            for (Smb4KNetworkObject *obj : qAsConst(d->hostObjects)) {
                if (url == obj->url()) {
                    object = obj;
                    break;
                } else {
                    continue;
                }
            }
            break;
        }
        case Smb4KNetworkObject::Share: {
            for (Smb4KNetworkObject *obj : qAsConst(d->shareObjects)) {
                if (url == obj->url()) {
                    object = obj;
                    break;
                } else {
                    continue;
                }
            }
            break;
        }
        default: {
            break;
        }
        }
    }

    return object;
}

void Smb4KDeclarative::openMountDialog()
{
    QPointer<Smb4KMountDialog> mountDialog = new Smb4KMountDialog();
    mountDialog->open();
}

void Smb4KDeclarative::mountShare(Smb4KNetworkObject *object)
{
    if (object && object->type() == Smb4KNetworkObject::Share) {
        SharePtr share = Smb4KGlobal::findShare(object->url(), object->workgroupName());

        if (share) {
            if (share->isHomesShare()) {
                QPointer<Smb4KHomesUserDialog> homesUserDialog = new Smb4KHomesUserDialog();
                bool proceed = false;

                if (homesUserDialog->setShare(share)) {
                    // We want to get a return value here, so we use exec()
                    if (homesUserDialog->exec() == QDialog::Accepted) {
                        proceed = true;
                    }
                }

                delete homesUserDialog;

                if (!proceed) {
                    return;
                }
            }

            Smb4KMounter::self()->mountShare(share);
        }
    }
}

void Smb4KDeclarative::mountBookmark(Smb4KBookmarkObject *object)
{
    if (object) {
        BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUrl(object->url());

        SharePtr share = SharePtr(new Smb4KShare());
        share->setUrl(object->url());
        share->setWorkgroupName(object->workgroupName());
        share->setHostIpAddress(object->hostIpAddress());

        Smb4KMounter::self()->mountShare(share);

        share.clear();
    }
}

void Smb4KDeclarative::unmount(Smb4KNetworkObject *object)
{
    if (object && object->type()) {
        if (object->mountpoint().isValid()) {
            SharePtr share = Smb4KGlobal::findShareByPath(object->mountpoint().path());

            if (share) {
                Smb4KMounter::self()->unmountShare(share);
            }
        }
    }
}

void Smb4KDeclarative::unmountAll()
{
    Smb4KMounter::self()->unmountAllShares(false);
}

bool Smb4KDeclarative::isShareMounted(const QUrl &url)
{
    QList<SharePtr> shares = Smb4KGlobal::findShareByUrl(url);

    for (const SharePtr &share : qAsConst(shares)) {
        if (!share->isForeign()) {
            return true;
        }
    }

    return false;
}

void Smb4KDeclarative::print(Smb4KNetworkObject *object)
{
    if (object && object->type() == Smb4KNetworkObject::Share) {
        SharePtr printer = Smb4KGlobal::findShare(object->url(), object->workgroupName());

        if (printer) {
            QPointer<Smb4KPrintDialog> printDialog = new Smb4KPrintDialog();

            if (printDialog->setPrinterShare(printer)) {
                printDialog->open();
            } else {
                delete printDialog;
            }
        }
    }
}

void Smb4KDeclarative::addBookmark(Smb4KNetworkObject *object)
{
    if (object) {
        QList<SharePtr> shares;

        SharePtr share = Smb4KGlobal::findShare(object->url(), object->workgroupName());

        if (share) {
            shares << share;
        } else {
            QList<SharePtr> mountedShares = Smb4KGlobal::findShareByUrl(object->url());

            if (!mountedShares.isEmpty()) {
                shares << mountedShares.first();
            }
        }

        if (!shares.isEmpty()) {
            QPointer<Smb4KBookmarkDialog> bookmarkDialog = new Smb4KBookmarkDialog();

            if (bookmarkDialog->setShares(shares)) {
                bookmarkDialog->open();
            } else {
                delete bookmarkDialog;
            }
        }
    }
}

void Smb4KDeclarative::removeBookmark(Smb4KBookmarkObject *object)
{
    if (object) {
        //
        // Find the bookmark in the list and remove it.
        //
        BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUrl(object->url());

        if (bookmark) {
            Smb4KBookmarkHandler::self()->removeBookmark(bookmark);
        }
    }
}

void Smb4KDeclarative::editBookmarks()
{
    QPointer<Smb4KBookmarkEditor> bookmarkEditor = new Smb4KBookmarkEditor();
    bookmarkEditor->open();
}

void Smb4KDeclarative::synchronize(Smb4KNetworkObject *object)
{
    if (object && object->type() == Smb4KNetworkObject::Share) {
        for (const SharePtr &share : Smb4KGlobal::mountedSharesList()) {
            if (share->url() == object->url()) {
                QPointer<Smb4KSynchronizationDialog> synchronizationDialog = new Smb4KSynchronizationDialog();
                if (synchronizationDialog->setShare(share)) {
                    synchronizationDialog->open();
                } else {
                    delete synchronizationDialog;
                }
            }
        }
    }
}

void Smb4KDeclarative::openCustomOptionsDialog(Smb4KNetworkObject *object)
{
    if (object) {
        NetworkItemPtr networkItem;

        switch (object->type()) {
        case Smb4KNetworkObject::Host: {
            for (const HostPtr &host : Smb4KGlobal::hostsList()) {
                if (host->url() == object->url()) {
                    networkItem = host;
                    break;
                }
            }
            break;
        }
        case Smb4KNetworkObject::Share: {
            for (const SharePtr &share : Smb4KGlobal::sharesList()) {
                if (share->url() == object->url()) {
                    networkItem = share;
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
        }

        if (!networkItem.isNull()) {
            QPointer<Smb4KCustomSettingsEditor> customSettingsEditor = new Smb4KCustomSettingsEditor();
            if (customSettingsEditor->setNetworkItem(networkItem)) {
                customSettingsEditor->open();
            } else {
                delete customSettingsEditor;
            }
        }
    }
}

void Smb4KDeclarative::startClient()
{
    Smb4KClient::self()->start();
}

void Smb4KDeclarative::abortClient()
{
    Smb4KClient::self()->abort();
}

void Smb4KDeclarative::startMounter()
{
    Smb4KMounter::self()->start();
}

void Smb4KDeclarative::abortMounter()
{
    Smb4KMounter::self()->abort();
}

QString Smb4KDeclarative::activeProfile() const
{
    QString activeProfile;

    for (Smb4KProfileObject *profile : qAsConst(d->profileObjects)) {
        if (profile->isActiveProfile()) {
            activeProfile = profile->profileName();
            break;
        } else {
            continue;
        }
    }

    return activeProfile;
}

void Smb4KDeclarative::setActiveProfile(const QString &profile)
{
    Smb4KProfileManager::self()->setActiveProfile(profile);
}

bool Smb4KDeclarative::profileUsage() const
{
    return Smb4KProfileManager::self()->useProfiles();
}

void Smb4KDeclarative::preview(Smb4KNetworkObject *object)
{
    if (object->type() == Smb4KNetworkObject::Share) {
        SharePtr share = Smb4KGlobal::findShare(object->url(), object->workgroupName());

        if (share) {
            QPointer<Smb4KPreviewDialog> previewDialog = new Smb4KPreviewDialog();

            if (previewDialog->setShare(share)) {
                previewDialog->open();
            } else {
                delete previewDialog;
            }
        }
    }
}

void Smb4KDeclarative::openConfigurationDialog()
{
    //
    // Check if the configuration dialog exists and try to show it.
    //
    if (KConfigDialog::exists(QStringLiteral("Smb4KConfigDialog"))) {
        KConfigDialog::showDialog(QStringLiteral("Smb4KConfigDialog"));
        return;
    }

    //
    // If the dialog does not exist, load and show it:
    //
    KPluginMetaData metaData(QStringLiteral("smb4kconfigdialog"));
    KPluginFactory::Result<KPluginFactory> result = KPluginFactory::loadFactory(metaData);

    if (result.errorReason == KPluginFactory::NO_PLUGIN_ERROR) {
        QPointer<KConfigDialog> dlg = result.plugin->create<KConfigDialog>();

        if (dlg) {
            dlg->setObjectName(QStringLiteral("Smb4KConfigDialog"));
            dlg->show();
        }
    }
}

void Smb4KDeclarative::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if (!d->requestQueue.isEmpty()) {
        if (!d->passwordDialog->isVisible()) {
            NetworkItemPtr networkItem = d->requestQueue.takeFirst();

            if (networkItem && d->passwordDialog->setNetworkItem(networkItem)) {
                d->passwordDialog->show();
            }
        }
    } else {
        killTimer(d->timerId);
        d->timerId = 0;
    }
}

void Smb4KDeclarative::slotWorkgroupsListChanged()
{
    qDeleteAll(d->workgroupObjects);
    d->workgroupObjects.clear();

    for (const WorkgroupPtr &workgroup : Smb4KGlobal::workgroupsList()) {
        d->workgroupObjects << new Smb4KNetworkObject(workgroup.data());
    }

    Q_EMIT workgroupsListChanged();
}

void Smb4KDeclarative::slotHostsListChanged()
{
    qDeleteAll(d->hostObjects);
    d->hostObjects.clear();

    for (const HostPtr &host : Smb4KGlobal::hostsList()) {
        d->hostObjects << new Smb4KNetworkObject(host.data());
    }

    Q_EMIT hostsListChanged();
}

void Smb4KDeclarative::slotSharesListChanged()
{
    qDeleteAll(d->shareObjects);
    d->shareObjects.clear();

    for (const SharePtr &share : Smb4KGlobal::sharesList()) {
        d->shareObjects << new Smb4KNetworkObject(share.data());
    }

    Q_EMIT sharesListChanged();
}

void Smb4KDeclarative::slotMountedSharesListChanged()
{
    qDeleteAll(d->mountedObjects);
    d->mountedObjects.clear();

    for (const SharePtr &mountedShare : Smb4KGlobal::mountedSharesList()) {
        d->mountedObjects << new Smb4KNetworkObject(mountedShare.data());
    }

    Q_EMIT mountedSharesListChanged();
}

void Smb4KDeclarative::slotBookmarksListChanged()
{
    qDeleteAll(d->bookmarkObjects);
    d->bookmarkObjects.clear();

    qDeleteAll(d->bookmarkCategoryObjects);
    d->bookmarkCategoryObjects.clear();

    QList<BookmarkPtr> bookmarksList = Smb4KBookmarkHandler::self()->bookmarkList();
    QStringList categoriesList = Smb4KBookmarkHandler::self()->categoryList();

    for (const BookmarkPtr &bookmark : qAsConst(bookmarksList)) {
        d->bookmarkObjects << new Smb4KBookmarkObject(bookmark.data());
    }

    for (const QString &category : qAsConst(categoriesList)) {
        d->bookmarkCategoryObjects << new Smb4KBookmarkObject(category);
    }

    Q_EMIT bookmarksListChanged();
}

void Smb4KDeclarative::slotProfilesListChanged(const QStringList &profiles)
{
    qDeleteAll(d->profileObjects);
    d->profileObjects.clear();

    for (const QString &p : profiles) {
        Smb4KProfileObject *profile = new Smb4KProfileObject();
        profile->setProfileName(p);

        if (QString::compare(p, Smb4KProfileManager::self()->activeProfile()) == 0) {
            profile->setActiveProfile(true);
        } else {
            profile->setActiveProfile(false);
        }

        d->profileObjects << profile;
    }

    Q_EMIT profilesListChanged();
}

void Smb4KDeclarative::slotActiveProfileChanged(const QString &activeProfile)
{
    for (Smb4KProfileObject *profile : qAsConst(d->profileObjects)) {
        if (QString::compare(profile->profileName(), activeProfile) == 0) {
            profile->setActiveProfile(true);
        } else {
            profile->setActiveProfile(false);
        }
    }

    Q_EMIT activeProfileChanged();
}

void Smb4KDeclarative::slotProfileUsageChanged(bool /*use*/)
{
    Q_EMIT profileUsageChanged();
}

void Smb4KDeclarative::slotCredentialsRequested(const NetworkItemPtr &networkItem)
{
    d->requestQueue.append(networkItem);

    if (d->timerId == 0) {
        d->timerId = startTimer(500);
    }
}
