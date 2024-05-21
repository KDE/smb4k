/*
    The configuration page for the profiles

    SPDX-FileCopyrightText: 2014-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEPROFILES_H
#define SMB4KCONFIGPAGEPROFILES_H

// Qt includes
#include <QCheckBox>
#include <QList>
#include <QListWidget>
#include <QPair>
#include <QPushButton>
#include <QWidget>

// KDE includes
#include <KLineEdit>

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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    /**
     * This signal is emitted when the profile settings were modified.
     */
    void profilesModified();

protected Q_SLOTS:
    void slotProfileUsageChanged(bool checked);
    void slotAddProfile(bool checked);
    void slotEditProfile(bool checked);
    void slotRemoveProfile(bool checked);
    void slotMoveProfileUp(bool checked);
    void slotMoveProfileDown(bool checked);
    void slotSetProfileActive(bool checked);
    void slotProfileChanged(QListWidgetItem *profileItem);
    void slotResetProfiles(bool checked);
    void slotEnableButtons(int row);

private:
    void loadProfiles();
    void checkProfilesChanged();
    ProfileContainer *findProfileContainer(QListWidgetItem *profileItem);
    QCheckBox *m_useProfiles;
    QWidget *m_profilesEditorWidget;
    KLineEdit *m_profilesInputLineEdit;
    QListWidget *m_profilesListWidget;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_removeButton;
    QPushButton *m_upButton;
    QPushButton *m_downButton;
    QPushButton *m_setActiveButton;
    QPushButton *m_resetButton;
    QList<ProfileContainer> m_profiles;
    bool m_profilesChanged;
    ProfileContainer *m_currentProfileContainer;
};

#endif
