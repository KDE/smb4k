/*
    Private helper classes for Smb4KCustomOptionsManager class
    -------------------
    begin                : Fr 29 Apr 2011
    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCUSTOMOPTIONSMANAGER_P_H
#define SMB4KCUSTOMOPTIONSMANAGER_P_H

// application specific includes
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kglobal.h"

// Qt includes
#include <QDialog>

class Smb4KCustomOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KCustomOptionsDialog(const OptionsPtr &options, QWidget *parent = 0);

    /**
     * Destructor
     */
    ~Smb4KCustomOptionsDialog();

protected Q_SLOTS:
    void slotSetDefaultValues();
    void slotCheckValues();
    void slotOKClicked();
    void slotEnableWOLFeatures(const QString &mac);
    void slotCifsExtensionsSupport(bool support);
    void slotUseClientProtocolVersions(bool use);

private:
    void setupView();
    bool checkDefaultValues();
    void setDefaultValues();
    void saveValues();
    OptionsPtr m_options;
};

class Smb4KCustomOptionsManagerPrivate
{
public:
    QList<OptionsPtr> options;
};

class Smb4KCustomOptionsManagerStatic
{
public:
    Smb4KCustomOptionsManager instance;
};

#endif
