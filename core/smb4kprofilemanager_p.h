/*
    Private helper class(es) for the profile manager.
    -------------------
    begin                : Mi Aug 12 2014
    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KPROFILEMANAGER_P_H
#define SMB4KPROFILEMANAGER_P_H

// Application specific includes
#include "smb4kprofilemanager.h"

// Qt includes
#include <QDialog>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QWidget>

// KDE includes
#include <KCompletion/KComboBox>

class Smb4KProfileMigrationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KProfileMigrationDialog(const QStringList &from, const QStringList &to, QWidget *parent = 0);

    /**
     * Destructor
     */
    virtual ~Smb4KProfileMigrationDialog();

    /**
     * Returns the name of the profile of which the entries
     * should be migrated.
     *
     * @returns the name of the profile.
     */
    QString from() const;

    /**
     * Returns the name of the profile where the entries should
     * be migrated to.
     *
     * @returns the name of the new profile.
     */
    QString to() const;

protected Q_SLOTS:
    void slotOkClicked();

private:
    void setupView();
    QPushButton *m_ok_button;
    QPushButton *m_cancel_button;
    QStringList m_from_list;
    QStringList m_to_list;
    KComboBox *m_from_box;
    KComboBox *m_to_box;
};

class Smb4KProfileManagerPrivate
{
public:
    QString activeProfile;
    QStringList profiles;
    bool useProfiles;
};

class Smb4KProfileManagerStatic
{
public:
    Smb4KProfileManager instance;
};

#endif
