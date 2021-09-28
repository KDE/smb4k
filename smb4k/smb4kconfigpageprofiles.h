/*
    The configuration page for the profiles

    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEPROFILES_H
#define SMB4KCONFIGPAGEPROFILES_H

// Qt includes
#include <QList>
#include <QPair>
#include <QWidget>

// KDE includes
#include <KWidgetsAddons/KEditListWidget>

class Smb4KConfigPageProfiles : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KConfigPageProfiles(QWidget *parent = 0);

    /**
     * Destructor
     */
    virtual ~Smb4KConfigPageProfiles();

    /**
     * This function returns a list of pairs that contains the
     * renamed profiles. The first entry of the pair is the old name
     * of the profile and the second entry is the new name.
     *
     * @returns a list of name pairs
     */
    QList<QPair<QString, QString>> renamedProfiles() const;

    /**
     * Clear the list of renamed profiles.
     */
    void clearRenamedProfiles();

    /**
     * This function returns the list of removed profiles.
     *
     * @returns the removed profiles
     */
    QStringList removedProfiles() const;

    /**
     * Clear the list of removed profiles.
     */
    void clearRemovedProfiles();

protected Q_SLOTS:
    void slotEnableWidget(int state);
    void slotProfileRemoved(const QString &name);
    void slotProfileChanged();

private:
    KEditListWidget *m_profiles;
    QList<QPair<QString, QString>> m_renamed;
    QStringList m_removed;
};

#endif
