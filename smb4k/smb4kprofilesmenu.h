/*
    smb4kprofilesmenu  -  The menu for the profiles

    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KPROFILESMENU_H
#define SMB4KPROFILESMENU_H

// Qt include
#include <QString>
#include <QStringList>

// KDE includes
#include <KWidgetsAddons/KSelectAction>

class Smb4KProfilesMenu : public KSelectAction
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KProfilesMenu(QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~Smb4KProfilesMenu();

    /**
     * Force the menu to be set up again. This should be called if
     * the settings changed and the handling of bookmarks might be
     * affected.
     */
    void refreshMenu();

protected Q_SLOTS:
    void slotActiveProfileChanged(const QString &newProfile);
    void slotProfilesListChanged(const QStringList &profiles);
    void slotProfileUsageChanged(bool use);
    void slotActionTriggered(const QString &name);
};

#endif
