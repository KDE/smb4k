/***************************************************************************
    smb4kbookmarkmenu  -  Bookmark menu
                             -------------------
    begin                : Sat Apr 02 2011
    copyright            : (C) 2011-2021 by Alexander Reinholdt
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

// application specific includes
#include "smb4kbookmarkmenu.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kglobal.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QDebug>
#include <QLatin1String>
#include <QMenu>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

Smb4KBookmarkMenu::Smb4KBookmarkMenu(int type, QWidget *parentWidget, QObject *parent)
    : KActionMenu(KDE::icon("folder-favorites"), i18n("Bookmarks"), parent)
    , m_type(type)
    , m_parent_widget(parentWidget)
{
    //
    // Set up the action group for the actions
    //
    m_actions = new QActionGroup(menu());

    //
    // Set up the action group for the bookmarks
    //
    m_bookmarks = new QActionGroup(menu());

    //
    // Set up the menu
    //
    setupMenu();

    //
    // Connections
    //
    connect(Smb4KBookmarkHandler::self(), SIGNAL(updated()), SLOT(slotBookmarksUpdated()));
    connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)), SLOT(slotEnableBookmark(SharePtr)));
    connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)), SLOT(slotEnableBookmark(SharePtr)));
    connect(m_actions, SIGNAL(triggered(QAction *)), SLOT(slotCategoryActionTriggered(QAction *)));
    connect(m_bookmarks, SIGNAL(triggered(QAction *)), SLOT(slotBookmarkActionTriggered(QAction *)));
}

Smb4KBookmarkMenu::~Smb4KBookmarkMenu()
{
}

void Smb4KBookmarkMenu::refreshMenu()
{
    //
    // Delete all entries from the menu
    //
    while (!menu()->actions().isEmpty()) {
        QAction *action = menu()->actions().takeFirst();
        removeAction(action);
        delete action;
    }

    //
    // Clear the rest of the menu
    //
    if (!menu()->isEmpty()) {
        menu()->clear();
    }

    //
    // Set up the menu
    //
    setupMenu();

    //
    // Make sure the correct menu entries are shown
    //
    menu()->update();
}

void Smb4KBookmarkMenu::setBookmarkActionEnabled(bool enable)
{
    QAction *action = menu()->findChild<QAction *>("add_action");

    if (action) {
        action->setEnabled(enable);
    }
}

void Smb4KBookmarkMenu::setupMenu()
{
    //
    // Depending on the type chosen, some global actions need to be inserted
    // into the menu. These actions are always enabled.
    //
    switch (m_type) {
    case MainWindow: {
        QAction *editBookmarksAction = new QAction(KDE::icon("bookmarks-organize"), i18n("&Edit Bookmarks"), menu());
        editBookmarksAction->setObjectName("edit_action");
        QMap<QString, QVariant> editInfo;
        editInfo["type"] = "edit";
        editBookmarksAction->setData(editInfo);
        connect(editBookmarksAction, SIGNAL(triggered(bool)), SLOT(slotEditActionTriggered(bool)));
        addAction(editBookmarksAction);
        m_actions->addAction(editBookmarksAction);

        QAction *addBookmarkAction = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), menu());
        addBookmarkAction->setObjectName("add_action");
        QMap<QString, QVariant> addInfo;
        addInfo["type"] = "add";
        addBookmarkAction->setData(addInfo);
        addBookmarkAction->setEnabled(false);
        connect(addBookmarkAction, SIGNAL(triggered(bool)), SLOT(slotAddActionTriggered(bool)));
        addAction(addBookmarkAction);
        m_actions->addAction(addBookmarkAction);

        break;
    }
    case SystemTray: {
        QAction *editBookmarksAction = new QAction(KDE::icon("bookmarks-organize"), i18n("&Edit Bookmarks"), menu());
        editBookmarksAction->setObjectName("edit_action");
        QMap<QString, QVariant> editInfo;
        editInfo["type"] = "edit";
        editBookmarksAction->setData(editInfo);
        connect(editBookmarksAction, SIGNAL(triggered(bool)), SLOT(slotEditActionTriggered(bool)));
        addAction(editBookmarksAction);
        m_actions->addAction(editBookmarksAction);

        break;
    }
    default: {
        break;
    }
    }

    //
    // Get the list of categories
    //
    QStringList allCategories = Smb4KBookmarkHandler::self()->categoryList();
    allCategories.sort();

    //
    // Insert a toplevel mount action, if necessary. Crucial for this is that there are
    // no (non-empty) categories defined. Enable it if not all toplevel bookmarks are mounted.
    //
    if (allCategories.isEmpty() || (allCategories.size() == 1 && allCategories.first().isEmpty())) {
        QAction *toplevelMount = new QAction(KDE::icon("media-mount"), i18n("Mount All Bookmarks"), menu());
        toplevelMount->setObjectName("toplevel_mount");
        QMap<QString, QVariant> mountInfo;
        mountInfo["type"] = "toplevel_mount";
        toplevelMount->setData(mountInfo);
        connect(toplevelMount, SIGNAL(triggered(bool)), SLOT(slotToplevelMountActionTriggered(bool)));
        addAction(toplevelMount);
        m_actions->addAction(toplevelMount);

        QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList();
        int mountedBookmarks = 0;

        for (const BookmarkPtr &bookmark : bookmarks) {
            QList<SharePtr> mountedShares = findShareByUrl(bookmark->url());

            if (!mountedShares.isEmpty()) {
                for (const SharePtr &share : mountedShares) {
                    if (!share->isForeign()) {
                        mountedBookmarks++;
                        break;
                    }
                }
            }
        }

        toplevelMount->setEnabled(mountedBookmarks != bookmarks.size());
    }

    //
    // Add a separator
    //
    addSeparator();

    //
    // Now add the categories and their bookmarks
    //
    for (const QString &category : allCategories) {
        if (!category.isEmpty()) {
            // Category menu entry
            KActionMenu *bookmarkCategoryMenu = new KActionMenu(category, menu());
            bookmarkCategoryMenu->setIcon(KDE::icon("folder-favorites"));
            QMap<QString, QVariant> menuInfo;
            menuInfo["type"] = "category_menu";
            menuInfo["category"] = category;
            bookmarkCategoryMenu->setData(menuInfo);
            addAction(bookmarkCategoryMenu);

            // Mount action for the category
            QAction *bookmarkCategoryMount = new QAction(KDE::icon("media-mount"), i18n("Mount All Bookmarks"), bookmarkCategoryMenu->menu());
            QMap<QString, QVariant> categoryMountInfo;
            categoryMountInfo["type"] = "category_mount";
            categoryMountInfo["category"] = category;
            bookmarkCategoryMount->setData(categoryMountInfo);
            bookmarkCategoryMenu->addAction(bookmarkCategoryMount);
            m_actions->addAction(bookmarkCategoryMount);

            // Get the list of bookmarks belonging to this category.
            // Use it to decide whether the category mount action should be enabled
            // (only if not all bookmarks belonging to this category are mounted) and
            // to sort the bookmarks.
            QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList(category);
            QStringList sortedBookmarks;
            int mountedBookmarks = 0;

            for (const BookmarkPtr &bookmark : bookmarks) {
                QAction *bookmarkAction = 0;

                if (Smb4KSettings::showCustomBookmarkLabel() && !bookmark->label().isEmpty()) {
                    bookmarkAction = new QAction(bookmark->icon(), bookmark->label(), bookmarkCategoryMenu->menu());
                    bookmarkAction->setObjectName(bookmark->url().toDisplayString());
                    QMap<QString, QVariant> bookmarkInfo;
                    bookmarkInfo["type"] = "bookmark";
                    bookmarkInfo["category"] = category;
                    bookmarkInfo["url"] = bookmark->url();
                    bookmarkInfo["text"] = bookmark->label();
                    bookmarkAction->setData(bookmarkInfo);
                    m_bookmarks->addAction(bookmarkAction);
                    sortedBookmarks << bookmark->label();
                } else {
                    bookmarkAction = new QAction(bookmark->icon(), bookmark->displayString(), bookmarkCategoryMenu->menu());
                    bookmarkAction->setObjectName(bookmark->url().toDisplayString());
                    QMap<QString, QVariant> bookmarkInfo;
                    bookmarkInfo["type"] = "bookmark";
                    bookmarkInfo["category"] = category;
                    bookmarkInfo["url"] = bookmark->url();
                    bookmarkInfo["text"] = bookmark->displayString();
                    bookmarkAction->setData(bookmarkInfo);
                    m_bookmarks->addAction(bookmarkAction);
                    sortedBookmarks << bookmark->displayString();
                }

                QList<SharePtr> mountedShares = findShareByUrl(bookmark->url());

                if (!mountedShares.isEmpty()) {
                    for (const SharePtr &share : mountedShares) {
                        if (!share->isForeign()) {
                            bookmarkAction->setEnabled(false);
                            mountedBookmarks++;
                            break;
                        }
                    }
                }
            }

            bookmarkCategoryMount->setEnabled(mountedBookmarks != bookmarks.size());
            sortedBookmarks.sort();

            // Add a separator
            bookmarkCategoryMenu->addSeparator();

            // Insert the sorted bookmarks into the category menu
            QList<QAction *> actions = m_bookmarks->actions();

            for (const QString &b : sortedBookmarks) {
                for (QAction *a : actions) {
                    if (a->text() == b) {
                        bookmarkCategoryMenu->addAction(a);
                        break;
                    }
                }
            }
        }
    }

    //
    // Add all bookmarks that have no category
    // Sort the bookmarks before.
    //
    QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList("");
    QStringList sortedBookmarks;

    for (const BookmarkPtr &bookmark : bookmarks) {
        QAction *bookmarkAction = 0;

        if (Smb4KSettings::showCustomBookmarkLabel() && !bookmark->label().isEmpty()) {
            bookmarkAction = new QAction(bookmark->icon(), bookmark->label(), menu());
            bookmarkAction->setObjectName(bookmark->url().toDisplayString());
            QMap<QString, QVariant> bookmarkInfo;
            bookmarkInfo["type"] = "bookmark";
            bookmarkInfo["category"] = "";
            bookmarkInfo["url"] = bookmark->url();
            bookmarkInfo["text"] = bookmark->label();
            bookmarkAction->setData(bookmarkInfo);
            m_bookmarks->addAction(bookmarkAction);
            sortedBookmarks << bookmark->label();
        } else {
            bookmarkAction = new QAction(bookmark->icon(), bookmark->displayString(), menu());
            bookmarkAction->setObjectName(bookmark->url().toDisplayString());
            QMap<QString, QVariant> bookmarkInfo;
            bookmarkInfo["type"] = "bookmark";
            bookmarkInfo["category"] = "";
            bookmarkInfo["url"] = bookmark->url();
            bookmarkInfo["text"] = bookmark->displayString();
            bookmarkAction->setData(bookmarkInfo);
            m_bookmarks->addAction(bookmarkAction);
            sortedBookmarks << bookmark->displayString();
        }

        QList<SharePtr> mountedShares = findShareByUrl(bookmark->url());

        if (!mountedShares.isEmpty()) {
            for (const SharePtr &share : mountedShares) {
                if (!share->isForeign()) {
                    qDebug() << "Disabling bookmark" << share->url().toDisplayString();
                    bookmarkAction->setEnabled(false);
                    break;
                }
            }
        }
    }

    sortedBookmarks.sort();

    QList<QAction *> actions = m_bookmarks->actions();

    for (const QString &b : sortedBookmarks) {
        for (QAction *a : actions) {
            if (a->data().toMap().value("text").toString() == b) {
                addAction(a);
                break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KBookmarkMenu::slotEditActionTriggered(bool /*checked*/)
{
    Smb4KBookmarkHandler::self()->editBookmarks();
}

void Smb4KBookmarkMenu::slotAddActionTriggered(bool /*checked*/)
{
    emit addBookmark();
}

void Smb4KBookmarkMenu::slotToplevelMountActionTriggered(bool /*checked*/)
{
    //
    // Mount all top level bookmarks.
    // This slot will only be called if there are no categories defined.
    //
    QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList();
    QList<SharePtr> mounts;

    for (const BookmarkPtr &bookmark : bookmarks) {
        // FIXME: Check if the bookmarked share has already been mounted.
        SharePtr share = SharePtr(new Smb4KShare());
        share->setHostName(bookmark->hostName());
        share->setShareName(bookmark->shareName());
        share->setWorkgroupName(bookmark->workgroupName());
        share->setHostIpAddress(bookmark->hostIpAddress());
        share->setLogin(bookmark->login());
        mounts << share;
    }

    Smb4KMounter::self()->mountShares(mounts);

    while (!mounts.isEmpty()) {
        mounts.takeFirst().clear();
    }
}

void Smb4KBookmarkMenu::slotCategoryActionTriggered(QAction *action)
{
    if (action->data().toMap().value("type").toString() == "category_mount") {
        //
        // Mount all bookmarks of one category
        //
        QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList(action->data().toMap().value("category").toString());
        QList<SharePtr> mounts;

        for (const BookmarkPtr &bookmark : bookmarks) {
            // FIXME: Check if the bookmarked share has already been mounted.
            SharePtr share = SharePtr(new Smb4KShare());
            share->setHostName(bookmark->hostName());
            share->setShareName(bookmark->shareName());
            share->setWorkgroupName(bookmark->workgroupName());
            share->setHostIpAddress(bookmark->hostIpAddress());
            share->setLogin(bookmark->login());
            mounts << share;
        }

        Smb4KMounter::self()->mountShares(mounts);

        while (!mounts.isEmpty()) {
            mounts.takeFirst().clear();
        }
    }
}

void Smb4KBookmarkMenu::slotBookmarkActionTriggered(QAction *action)
{
    QMap<QString, QVariant> info = action->data().toMap();
    QString bookmarkCategory = info.value("category").toString();
    QUrl url = info.value("url").toUrl();

    BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUrl(url);

    if (bookmark && bookmarkCategory == bookmark->categoryName()) {
        SharePtr share = SharePtr(new Smb4KShare());
        share->setHostName(bookmark->hostName());
        share->setShareName(bookmark->shareName());
        share->setWorkgroupName(bookmark->workgroupName());
        share->setHostIpAddress(bookmark->hostIpAddress());
        share->setLogin(bookmark->login());
        Smb4KMounter::self()->mountShare(share);
        share.clear();
    }
}

void Smb4KBookmarkMenu::slotBookmarksUpdated()
{
    refreshMenu();
}

void Smb4KBookmarkMenu::slotEnableBookmark(const SharePtr &share)
{
    if (!share->isForeign() && !m_bookmarks->actions().isEmpty()) {
        //
        // Enable or disable the bookmark
        //
        QList<QAction *> actions = m_bookmarks->actions();
        QString bookmarkCategory;

        for (QAction *a : actions) {
            QUrl bookmarkUrl = a->data().toMap().value("url").toUrl();

            if (share->url().matches(bookmarkUrl, QUrl::RemoveUserInfo | QUrl::RemovePort)) {
                a->setEnabled(!share->isMounted());
                bookmarkCategory = a->data().toMap().value("category").toString();
                break;
            }
        }

        //
        // Check if all bookmarks belonging to this category are
        // mounted. Enable the respective mount action if necessary.
        //
        bool allMounted = true;

        for (QAction *a : actions) {
            if (a->data().toMap().value("category").toString() == bookmarkCategory && a->isEnabled()) {
                allMounted = false;
                break;
            }
        }

        QList<QAction *> allActions = m_actions->actions();

        for (QAction *a : allActions) {
            if (a->data().toMap().value("type").toString() == "toplevel_mount" && bookmarkCategory.isEmpty()) {
                a->setEnabled(!allMounted);
                break;
            } else if (a->data().toMap().value("type").toString() == "category_mount" && a->data().toMap().value("category").toString() == bookmarkCategory) {
                a->setEnabled(!allMounted);
                break;
            }
        }
    }
}
