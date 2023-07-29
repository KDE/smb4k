/*
 *  Editor dialog for the custom settings
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KCUSTOMSETTINGSDIALOG_H
#define SMB4KCUSTOMSETTINGSDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4kcustomsettingseditorwidget.h"

// Qt includes
#include <QDialog>
#include <QPushButton>

class Q_DECL_EXPORT Smb4KCustomSettingsEditor : public QDialog
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

    /**
     * Set the network item. Ideally, this function should be used before
     * the dialog is shown.
     *
     * @param networkItem       The network item for which the custom settings
     *                          should be defined
     *
     * @returns TRUE if a network item was set and false otherwise
     */
    bool setNetworkItem(NetworkItemPtr networkItem);

protected Q_SLOTS:
    void slotRestoreDefaultsClicked();
    void slotSaveClicked();
    void slotCancelClicked();
    void slotCustomSettingsEdited(bool changed);

private:
    OptionsPtr m_customSettings;
    QLabel *m_descriptionText;
    Smb4KCustomSettingsEditorWidget *m_editorWidget;
    QPushButton *m_resetButton;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif
