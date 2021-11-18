/*
    smb4ksharesmenu  -  Shares menu

    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksharesmenu.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kglobal.h"
#include "core/smb4kmounter.h"
#include "core/smb4kshare.h"
#include "core/smb4ksynchronizer.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QMap>
#include <QMenu>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QVariant>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

Smb4KSharesMenu::Smb4KSharesMenu(QObject *parent)
    : KActionMenu(KDE::icon("folder-network", QStringList("emblem-mounted")), i18n("Mounted Shares"), parent)
{
    //
    // Set up action group for the shares menus
    //
    m_menus = new QActionGroup(menu());

    //
    // Set up action group for the shares actions
    //
    m_actions = new QActionGroup(menu());

    //
    // Add the Unmount All action
    //
    m_unmountAll = new QAction(KDE::icon("system-run"), i18n("U&nmount All"), menu());
    m_unmountAll->setEnabled(false);
    connect(m_unmountAll, SIGNAL(triggered(bool)), SLOT(slotUnmountAllShares()));
    addAction(m_unmountAll);

    //
    // Add the separator
    //
    m_separator = addSeparator();

    //
    // Connections
    //
    connect(m_actions, SIGNAL(triggered(QAction *)), SLOT(slotShareAction(QAction *)));

    connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)), SLOT(slotShareMounted(SharePtr)));
    connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)), SLOT(slotShareUnmounted(SharePtr)));
    connect(Smb4KMounter::self(), SIGNAL(mountedSharesListChanged()), SLOT(slotMountedSharesListChanged()));
}

Smb4KSharesMenu::~Smb4KSharesMenu()
{
}

void Smb4KSharesMenu::refreshMenu()
{
    //
    // Delete all shares from the menu
    //
    while (!m_menus->actions().isEmpty()) {
        QAction *shareMenu = m_menus->actions().takeFirst();
        removeAction(shareMenu);
        delete shareMenu;
    }

    //
    // Add share menus, if necessary
    //
    if (!mountedSharesList().isEmpty()) {
        for (const SharePtr &share : mountedSharesList()) {
            addShareToMenu(share);
        }
    }

    //
    // Make the separator visible, if necessary
    //
    m_separator->setVisible(!m_menus->actions().isEmpty());

    //
    // Enable or disable the Unmount All action, depending on the number of
    // mounted shares present.
    //
    m_unmountAll->setEnabled(((!onlyForeignMountedShares() || Smb4KMountSettings::unmountForeignShares()) && !m_menus->actions().isEmpty()));

    //
    // Make sure the correct menu entries are shown
    //
    menu()->update();

    //
    // Work around a display glitch were the first bookmark
    // might not be shown (see also BUG 442187)
    //
    QCoreApplication::processEvents();
}

void Smb4KSharesMenu::addShareToMenu(const SharePtr &share)
{
    //
    // For sorting purposes, get the display strings of all
    // shares that are already in the action menu.
    //
    QStringList displayNames;

    for (QAction *entry : m_menus->actions()) {
        displayNames << entry->data().toMap().value("text").toString();
    }

    //
    // Add the display string of the current share as well
    //
    displayNames << share->displayString();

    //
    // Sort the display strings
    //
    displayNames.sort();

    //
    // Create the share menu
    //
    KActionMenu *shareMenu = new KActionMenu(share->displayString(), menu());
    shareMenu->setIcon(share->icon());

    QMap<QString, QVariant> data;
    data["text"] = share->displayString();
    data["mountpoint"] = share->path();

    shareMenu->setData(data);
    m_menus->addAction(shareMenu);

    //
    // Add the unmount action to the menu
    //
    QAction *unmount = new QAction(KDE::icon("media-eject"), i18n("Unmount"), shareMenu->menu());

    QMap<QString, QVariant> unmountData;
    unmountData["type"] = "unmount";
    unmountData["mountpoint"] = share->path();

    unmount->setData(unmountData);
    unmount->setEnabled(!share->isForeign() || Smb4KMountSettings::unmountForeignShares());
    shareMenu->addAction(unmount);
    m_actions->addAction(unmount);

    //
    // Add a separator
    //
    shareMenu->addSeparator();

    //
    // Add the bookmark action to the menu
    //
    QAction *addBookmark = new QAction(KDE::icon("bookmark-new"), i18n("Add Bookmark"), shareMenu->menu());

    QMap<QString, QVariant> bookmarkData;
    bookmarkData["type"] = "bookmark";
    bookmarkData["mountpoint"] = share->path();

    addBookmark->setData(bookmarkData);
    shareMenu->addAction(addBookmark);
    m_actions->addAction(addBookmark);

    //
    // Add the synchronization action to the menu
    //
    QAction *synchronize = new QAction(KDE::icon("folder-sync"), i18n("Synchronize"), shareMenu->menu());

    QMap<QString, QVariant> syncData;
    syncData["type"] = "sync";
    syncData["mountpoint"] = share->path();

    synchronize->setData(syncData);
    synchronize->setEnabled(!QStandardPaths::findExecutable("rsync").isEmpty() && !share->isInaccessible());
    shareMenu->addAction(synchronize);
    m_actions->addAction(synchronize);

    //
    // Add a separator
    //
    shareMenu->addSeparator();

    //
    // Add the Open with Konsole action to the menu
    //
    QAction *konsole = new QAction(KDE::icon("utilities-terminal"), i18n("Open with Konsole"), shareMenu->menu());

    QMap<QString, QVariant> konsoleData;
    konsoleData["type"] = "konsole";
    konsoleData["mountpoint"] = share->path();

    konsole->setData(konsoleData);
    konsole->setEnabled(!QStandardPaths::findExecutable("konsole").isEmpty() && !share->isInaccessible());
    shareMenu->addAction(konsole);
    m_actions->addAction(konsole);

    //
    // Add the Open with Filemanager action to the menu
    //
    QAction *filemanager = new QAction(KDE::icon("system-file-manager"), i18n("Open with File Manager"), shareMenu->menu());

    QMap<QString, QVariant> fileManagerData;
    fileManagerData["type"] = "filemanager";
    fileManagerData["mountpoint"] = share->path();

    filemanager->setData(fileManagerData);
    filemanager->setEnabled(!share->isInaccessible());
    shareMenu->addAction(filemanager);
    m_actions->addAction(filemanager);

    //
    // Add the share menu to the action menu at the right place
    //
    if (displayNames.size() == 1) {
        addAction(shareMenu);
    } else {
        int index = displayNames.indexOf(share->displayString(), 0);

        if (index != displayNames.size() - 1) {
            QString displayStringBefore = displayNames.at(index + 1);

            for (QAction *action : m_menus->actions()) {
                if (action->data().toMap().value("text").toString() == displayStringBefore) {
                    insertAction(action, shareMenu);
                    break;
                }
            }
        } else {
            addAction(shareMenu);
        }
    }
}

void Smb4KSharesMenu::removeShareFromMenu(const SharePtr &share)
{
    //
    // Remove the share from the action group and the menu and delete
    // it. We do not need to take care of the actions in the menu. They
    // are delete with their parent.
    //
    for (int i = 0; i < m_menus->actions().size(); ++i) {
        if (m_menus->actions().at(i)->data().toMap().value("mountpoint").toString() == share->path()) {
            QAction *menuAction = m_menus->actions().takeAt(i);
            removeAction(menuAction);
            delete menuAction;
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSharesMenu::slotShareMounted(const SharePtr &share)
{
    addShareToMenu(share);
}

void Smb4KSharesMenu::slotShareUnmounted(const SharePtr &share)
{
    removeShareFromMenu(share);
}

void Smb4KSharesMenu::slotUnmountAllShares()
{
    //
    // Unmount all shares
    //
    Smb4KMounter::self()->unmountAllShares(false);
}

void Smb4KSharesMenu::slotMountedSharesListChanged()
{
    //
    // Enable or disable the Unmount All action
    //
    m_unmountAll->setEnabled(((!onlyForeignMountedShares() || Smb4KMountSettings::unmountForeignShares()) && !m_menus->actions().isEmpty()));

    //
    // Make the separator visible, if necessary
    //
    m_separator->setVisible(!m_menus->actions().isEmpty());

    //
    // Make sure the correct menu entries are shown
    //
    menu()->update();

    //
    // Work around a display glitch were the first bookmark
    // might not be shown (see also BUG 442187)
    //
    QCoreApplication::processEvents();
}

void Smb4KSharesMenu::slotShareAction(QAction *action)
{
    //
    // Create a share
    //
    SharePtr share;

    //
    // Check that we have a share related action
    //
    if (action->data().toMap().contains("type")) {
        share = findShareByPath(action->data().toMap().value("mountpoint").toString());
    }

    //
    // Now process the action
    //
    if (share) {
        QString type = action->data().toMap().value("type").toString();

        if (type == "unmount") {
            Smb4KMounter::self()->unmountShare(share, false);
        } else if (type == "bookmark") {
            Smb4KBookmarkHandler::self()->addBookmark(share);
        } else if (type == "sync") {
            Smb4KSynchronizer::self()->synchronize(share);
        } else if (type == "konsole") {
            openShare(share, Smb4KGlobal::Konsole);
        } else if (type == "filemanager") {
            openShare(share, Smb4KGlobal::FileManager);
        }
    }
}
