/*
    The configuration page for the profiles

    SPDX-FileCopyrightText: 2014-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEPROFILES_H
#define SMB4KCONFIGPAGEPROFILES_H

// Qt includes
#include <QCheckBox>
#include <QList>
#include <QPair>
#include <QWidget>

// KDE includes
#include <KEditListWidget>

// forward declarations
struct ProfileContainer;

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

    /**
     * Returns TRUE if the list of profiles changed and FALSE otherwise.
     */
    bool profilesChanged() const;

protected Q_SLOTS:
    void slotProfileUsageChanged(bool checked);
    void slotProfileAdded(const QString &text);
    void slotProfileRemoved(const QString &text);
    void slotProfileChanged();

private:
    QCheckBox *m_useProfiles;
    KEditListWidget *m_profilesWidget;
    QList<ProfileContainer> m_profiles;
    bool m_profilesChanged;
};

#endif
