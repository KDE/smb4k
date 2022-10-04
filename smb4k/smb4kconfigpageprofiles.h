/*
    The configuration page for the profiles

    SPDX-FileCopyrightText: 2014-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
    explicit Smb4KConfigPageProfiles(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KConfigPageProfiles();

    /**
     * Apply the changes made to the list of profiles
     */
    void applyChanges();

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
