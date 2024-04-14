/*
    smb4ksystemtray  -  This is the system tray window class of Smb4K.

    SPDX-FileCopyrightText: 2007-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksystemtray.h"
#include "core/smb4kclient.h"
#include "core/smb4kglobal.h"
#include "core/smb4kmounter.h"
#include "core/smb4kshare.h"
#include "core/smb4kworkgroup.h"
#include "smb4kbookmarkmenu.h"
#include "smb4kprofilesmenu.h"
#include "smb4ksharesmenu.h"

// Qt includes
#include <QDebug>
#include <QMenu>
#include <QPointer>

// KDE specific includes
#include <KAboutData>
#include <KConfigDialog>
#include <KIconLoader>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KStandardAction>

using namespace Smb4KGlobal;

Smb4KSystemTray::Smb4KSystemTray(QWidget *parent)
    : KStatusNotifierItem(QStringLiteral("org.kde.smb4k.statusnotifieritem"), parent)
{
    QString iconName;

    if (KIconLoader::global()->hasIcon(QStringLiteral("network-workgroup-symbolic"))) {
        iconName = QStringLiteral("network-workgroup-symbolic");
    } else {
        iconName = QStringLiteral("network-workgroup");
    }

    setIconByName(iconName);
    setToolTip(iconName, i18n("Smb4K"), KAboutData::applicationData().shortDescription());
    setStatus(Active);
    setCategory(ApplicationStatus);

    QAction *mountAction = new QAction(KDE::icon(QStringLiteral("view-form"), QStringList(QStringLiteral("emblem-mounted"))), i18n("&Open Mount Dialog"), this);
    connect(mountAction, &QAction::triggered, this, &Smb4KSystemTray::slotMountDialog);

    addAction(QStringLiteral("shares_menu"), new Smb4KSharesMenu(this));
    addAction(QStringLiteral("bookmarks_menu"), new Smb4KBookmarkMenu(Smb4KBookmarkMenu::SystemTray, this));
    addAction(QStringLiteral("profiles_menu"), new Smb4KProfilesMenu(this));
    addAction(QStringLiteral("mount_action"), mountAction);
    addAction(QStringLiteral("config_action"), KStandardAction::preferences(parent, SLOT(slotConfigDialog()), this));

    contextMenu()->addAction(action(QStringLiteral("shares_menu")));
    contextMenu()->addAction(action(QStringLiteral("bookmarks_menu")));
    contextMenu()->addAction(action(QStringLiteral("profiles_menu")));
    contextMenu()->addSeparator();
    contextMenu()->addAction(action(QStringLiteral("mount_action")));
    contextMenu()->addAction(action(QStringLiteral("config_action")));

    connect(Smb4KMounter::self(), &Smb4KMounter::mountedSharesListChanged, this, &Smb4KSystemTray::slotSetStatus);
    connect(Smb4KClient::self(), &Smb4KClient::workgroups, this, &Smb4KSystemTray::slotSetStatus);
}

Smb4KSystemTray::~Smb4KSystemTray()
{
}

void Smb4KSystemTray::loadSettings()
{
    Smb4KBookmarkMenu *bookmarkMenu = qobject_cast<Smb4KBookmarkMenu *>(action(QStringLiteral("bookmarks_menu")));

    if (bookmarkMenu) {
        bookmarkMenu->refreshMenu();
    }

    Smb4KSharesMenu *sharesMenu = qobject_cast<Smb4KSharesMenu *>(action(QStringLiteral("shares_menu")));

    if (sharesMenu) {
        sharesMenu->refreshMenu();
    }

    Smb4KProfilesMenu *profilesMenu = qobject_cast<Smb4KProfilesMenu *>(action(QStringLiteral("profiles_menu")));

    if (profilesMenu) {
        profilesMenu->refreshMenu();
    }
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSystemTray::slotMountDialog()
{
    QPointer<Smb4KMountDialog> mountDialog = new Smb4KMountDialog();
    mountDialog->show();
}

void Smb4KSystemTray::slotSetStatus()
{
    if (!mountedSharesList().isEmpty() || !workgroupsList().isEmpty()) {
        setStatus(KStatusNotifierItem::Active);
    } else {
        setStatus(KStatusNotifierItem::Passive);
    }
}
