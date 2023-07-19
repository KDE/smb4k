/*
 *  Editor dialog for the custom settings
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KCUSTOMSETTINGSDIALOG_H
#define SMB4KCUSTOMSETTINGSDIALOG_H

// Qt includes
#include <QDialog>

class Smb4KCustomSettingsEditor : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KCustomSettingsEditor(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KCustomSettingsEditor();
};

#endif
