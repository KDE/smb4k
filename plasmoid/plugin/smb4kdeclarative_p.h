/*
    This class provides helper classes for Smb4KDeclarative

    SPDX-FileCopyrightText: 2013-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KDECLARATIVE_P_H
#define SMB4KDECLARATIVE_P_H

// application specific includes
#include "smb4kbookmarkobject.h"
#include "smb4knetworkobject.h"
#include "smb4kprofileobject.h"
#include "smb4k/smb4kbookmarkeditor.h"

// Qt includes
#include <QDialog>
#include <QPointer>

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
    QPointer<Smb4KBookmarkEditor> bookmarkEditor;
};

#endif
