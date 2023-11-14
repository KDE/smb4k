/*
    smb4kbookmarkmenu  -  Bookmark menu

    SPDX-FileCopyrightText: 2011-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
#include <QMap>
#include <QMapIterator>
#include <QMenu>

// KDE includes
#include <KIconLoader>
#include <KLocalizedString>

using namespace Smb4KGlobal;

Smb4KBookmarkMenu::Smb4KBookmarkMenu(int type, QObject *parent)
    : KActionMenu(KDE::icon(QStringLiteral("folder-favorites")), i18n("Bookmarks"), parent)
{
    m_bookmarkEditor = nullptr;

    m_categories = new QActionGroup(menu());
    m_bookmarks = new QActionGroup(menu());
    m_mountActions = new QActionGroup(menu());

    m_editBookmarks = new QAction(KDE::icon(QStringLiteral("bookmarks-organize")), i18n("&Edit Bookmarks"), menu());
    m_editBookmarks->setEnabled(!Smb4KBookmarkHandler::self()->bookmarkList().isEmpty());
    connect(m_editBookmarks, SIGNAL(triggered(bool)), SLOT(slotEditActionTriggered(bool)));
    addAction(m_editBookmarks);

    if (type == MainWindow) {
        m_addBookmark = new QAction(KDE::icon(QStringLiteral("bookmark-new")), i18n("Add &Bookmark"), menu());
        m_addBookmark->setEnabled(false);
        connect(m_addBookmark, SIGNAL(triggered(bool)), SLOT(slotAddActionTriggered(bool)));
        addAction(m_addBookmark);
    } else {
        m_addBookmark = nullptr;
    }

    m_toplevelMount = new QAction(KDE::icon(QStringLiteral("media-mount")), i18n("Mount Bookmarks"), menu());
    addAction(m_toplevelMount);
    m_mountActions->addAction(m_toplevelMount);

    m_separator = addSeparator();
    m_separator->setVisible(!Smb4KBookmarkHandler::self()->bookmarkList().isEmpty());

    loadBookmarks();
    adjustMountActions();

    connect(Smb4KBookmarkHandler::self(), &Smb4KBookmarkHandler::updated, this, &Smb4KBookmarkMenu::loadBookmarks);
    connect(Smb4KMounter::self(), &Smb4KMounter::mounted, this, &Smb4KBookmarkMenu::slotEnableBookmark);
    connect(Smb4KMounter::self(), &Smb4KMounter::unmounted, this, &Smb4KBookmarkMenu::slotEnableBookmark);
    connect(m_bookmarks, &QActionGroup::triggered, this, &Smb4KBookmarkMenu::slotBookmarkActionTriggered);
    connect(m_mountActions, &QActionGroup::triggered, this, &Smb4KBookmarkMenu::slotMountActionTriggered);
}

Smb4KBookmarkMenu::~Smb4KBookmarkMenu()
{
}

void Smb4KBookmarkMenu::refreshMenu()
{
    loadBookmarks();
}

void Smb4KBookmarkMenu::loadBookmarks()
{
    while (!m_categories->actions().isEmpty()) {
        QAction *category = m_categories->actions().takeFirst();
        removeAction(category);
        delete category;
    }

    while (!m_bookmarks->actions().isEmpty()) {
        QAction *bookmark = m_bookmarks->actions().takeFirst();
        removeAction(bookmark);
        delete bookmark;
    }

    QStringList categories = Smb4KBookmarkHandler::self()->categoryList();
    categories.sort();

    KActionMenu *categoryMenu = nullptr;
    QMap<QString, QAction *> topLevelActions;

    for (const QString &category : qAsConst(categories)) {
        if (!category.isEmpty()) {
            categoryMenu = new KActionMenu(category, menu());
            categoryMenu->setIcon(KDE::icon(QStringLiteral("folder-favorites")));

            addAction(categoryMenu);
            m_categories->addAction(categoryMenu);

            topLevelActions[QStringLiteral("00_")+category] = categoryMenu;

            QAction *categoryMount = new QAction(KDE::icon(QStringLiteral("media-mount")), i18n("Mount Bookmarks"), categoryMenu->menu());
            categoryMount->setData(category);

            categoryMenu->addAction(categoryMount);
            m_mountActions->addAction(categoryMount);

            categoryMenu->addSeparator();
        } else {
            categoryMenu = this;
        }

        QMap<QString, QAction *> actionMap;
        QList<BookmarkPtr> categoryBookmarks = Smb4KBookmarkHandler::self()->bookmarkList(category);

        for (const BookmarkPtr &bookmark : qAsConst(categoryBookmarks)) {
            QAction *bookmarkAction = new QAction(categoryMenu->menu());
            bookmarkAction->setIcon(bookmark->icon());

            QString displayName;

            if (Smb4KSettings::showCustomBookmarkLabel() && !bookmark->label().isEmpty()) {
                displayName = bookmark->label();
            } else {
                displayName = bookmark->displayString();
            }

            bookmarkAction->setText(displayName);

            QVariant variant = QVariant::fromValue(*bookmark.data());
            bookmarkAction->setData(variant);

            m_bookmarks->addAction(bookmarkAction);

            QList<SharePtr> mountedShares = findShareByUrl(bookmark->url());

            if (!mountedShares.isEmpty()) {
                for (const SharePtr &share : qAsConst(mountedShares)) {
                    if (!share->isForeign()) {
                        bookmarkAction->setEnabled(false);
                        break;
                    }
                }
            }

            if (!category.isEmpty()) {
                actionMap[displayName] = bookmarkAction;
            } else {
                topLevelActions[QStringLiteral("01_")+displayName] = bookmarkAction;
            }
        }

        QMapIterator<QString, QAction *> it(actionMap);

        while (it.hasNext()) {
            it.next();
            if (!category.isEmpty()) {
                categoryMenu->addAction(it.value());
            }
        }
    }

    QMapIterator<QString, QAction *> it(topLevelActions);

    while (it.hasNext()) {
        QAction *action = it.next().value();
        addAction(action);
    }

    adjustMountActions();

    m_editBookmarks->setEnabled(!Smb4KBookmarkHandler::self()->bookmarkList().isEmpty());
    m_separator->setVisible(!Smb4KBookmarkHandler::self()->bookmarkList().isEmpty());

    menu()->update();
}

void Smb4KBookmarkMenu::setBookmarkActionEnabled(bool enable)
{
    m_addBookmark->setEnabled(enable);
}

void Smb4KBookmarkMenu::adjustMountActions()
{
    QList<BookmarkPtr> toplevelBookmarks = Smb4KBookmarkHandler::self()->bookmarkList(QStringLiteral(""));

    if (!toplevelBookmarks.isEmpty()) {
        int mountedBookmarks = 0;

        for (const BookmarkPtr &bookmark : qAsConst(toplevelBookmarks)) {
            QList<SharePtr> mountedShares = findShareByUrl(bookmark->url());

            for (const SharePtr &share : qAsConst(mountedShares)) {
                if (!share->isForeign()) {
                    mountedBookmarks++;
                    break;
                }
            }
        }

        m_toplevelMount->setVisible(true);
        m_toplevelMount->setEnabled(mountedBookmarks != toplevelBookmarks.size());
    } else {
        m_toplevelMount->setVisible(false);
        m_toplevelMount->setEnabled(false);
    }

    QList<QAction *> allMountActions = m_mountActions->actions();
    QStringList allCategories = Smb4KBookmarkHandler::self()->categoryList();
    int mountedBookmarks = 0;

    for (const QString &category : qAsConst(allCategories)) {
        QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarkList(category);

        for (const BookmarkPtr &bookmark : bookmarks) {
            QList<SharePtr> mountedShares = findShareByUrl(bookmark->url());

            if (!mountedShares.isEmpty()) {
                for (const SharePtr &share : qAsConst(mountedShares)) {
                    if (!share->isForeign()) {
                        mountedBookmarks++;
                        break;
                    }
                }
            }
        }

        for (QAction *action : allMountActions) {
            if (action->data().toString() == category) {
                action->setEnabled(bookmarks.size() != mountedBookmarks);
                break;
            }
        }

        mountedBookmarks = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KBookmarkMenu::slotEditActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    if (m_bookmarkEditor.isNull()) {
        m_bookmarkEditor = new Smb4KBookmarkEditor(menu());
        m_bookmarkEditor->open();
    } else {
        m_bookmarkEditor->raise();
    }
}

void Smb4KBookmarkMenu::slotAddActionTriggered(bool /*checked*/)
{
    Q_EMIT addBookmark();
}

void Smb4KBookmarkMenu::slotMountActionTriggered(QAction *action)
{
    QList<BookmarkPtr> bookmarks;

    if (action == m_toplevelMount) {
        bookmarks = Smb4KBookmarkHandler::self()->bookmarkList(QStringLiteral(""));
    } else {
        bookmarks = Smb4KBookmarkHandler::self()->bookmarkList(action->data().toString());
    }

    QList<SharePtr> mounts;

    for (const BookmarkPtr &bookmark : qAsConst(bookmarks)) {
        SharePtr share = SharePtr(new Smb4KShare());
        share->setUrl(bookmark->url());
        share->setWorkgroupName(bookmark->workgroupName());
        share->setHostIpAddress(bookmark->hostIpAddress());
        mounts << share;
    }

    Smb4KMounter::self()->mountShares(mounts);

    while (!mounts.isEmpty()) {
        mounts.takeFirst().clear();
    }
}

void Smb4KBookmarkMenu::slotBookmarkActionTriggered(QAction *action)
{
    Smb4KBookmark bookmark = action->data().value<Smb4KBookmark>();

    SharePtr share = SharePtr(new Smb4KShare());
    share->setUrl(bookmark.url());
    share->setWorkgroupName(bookmark.workgroupName());
    share->setHostIpAddress(bookmark.hostIpAddress());

    Smb4KMounter::self()->mountShare(share);

    share.clear();
}

void Smb4KBookmarkMenu::slotEnableBookmark(const SharePtr &share)
{
    if (!share->isForeign() && !m_bookmarks->actions().isEmpty()) {
        QList<QAction *> actions = m_bookmarks->actions();

        for (QAction *a : qAsConst(actions)) {
            QUrl bookmarkUrl = a->data().value<Smb4KBookmark>().url();

            if (share->url().matches(bookmarkUrl, QUrl::RemoveUserInfo | QUrl::RemovePort)) {
                a->setEnabled(!share->isMounted());
                break;
            }
        }

        adjustMountActions();
    }
}
