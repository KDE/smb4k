/*
    Configuration page for the custom settings

    SPDX-FileCopyrightText: 2013-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGECUSTOMSETTINGS_H
#define SMB4KCONFIGPAGECUSTOMSETTINGS_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4kcustomsettingseditorwidget.h"

// Qt includes
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

// KDE includes
#include <KMessageWidget>

/**
 * This configuration page takes care of the custom settings
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */

class Smb4KConfigPageCustomSettings : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KConfigPageCustomSettings(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KConfigPageCustomSettings();

    /**
     * Returns TRUE if there may be changed custom settings. You must check if
     * this is indeed the case in you code.
     *
     * @returns TRUE if custom settings may have changed.
     */
    bool customSettingsChanged();

public Q_SLOTS:
    /**
     * This function loads the list of custom settings.
     */
    void loadCustomSettings();

    /**
     * This function saves the list of custom settings.
     */
    void saveCustomSettings();

protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

Q_SIGNALS:
    /**
     * This signal is emitted every time the custom settings potentially were
     * modified by the user. When this signal is emitted, it does not necessarily
     * mean that any custom setting changed. It only means that the user edited
     * one option.
     */
    void customSettingsModified();

protected Q_SLOTS:
    /**
     * This slot is called when the item selection changed.
     */
    void slotItemSelectionChanged();

    /**
     * This slot is invoked when an item is double clicked. It is used
     * to edit the item the user double clicked.
     *
     * @param item            The item that was double clicked.
     */
    void slotEditCustomItem(QListWidgetItem *item);

    /**
     * This slot is called when custom settings are to be edited.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotEditButtonClicked(bool checked);

    /**
     * This slot is called when a custom option is to be removed.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotRemoveButtonClicked(bool checked);

    /**
     * This slot is called when the list of custom settings is to be cleared.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotClearButtonClicked(bool checked);

    /**
     * This slot is called when the actions performed on the custom settings
     * are to be reset.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotResetButtonClicked(bool checked);

    /**
     * Enable/disable the buttons
     */
    void slotEnableButtons();

    /**
     * Called when the selected custom setting is edited
     *
     * @param changed           TRUE if the settings are changed and FALSE if not
     */
    void slotCustomSettingsEdited(bool changed);

private:
    void setRemovalMessage(const Smb4KCustomSettings &settings);
    QListWidget *m_listWidget;
    QListWidgetItem *m_itemToEdit;
    Smb4KCustomSettingsEditorWidget *m_editorWidget;
    KMessageWidget *m_messageWidget;
    bool m_customSettingsChanged;
    bool m_savingCustomSettings;
    QPushButton *m_resetButton;
    QPushButton *m_editButton;
    QPushButton *m_removeButton;
    QPushButton *m_clearButton;
};

#endif
