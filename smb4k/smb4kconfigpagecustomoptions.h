/*
    The configuration page for the custom settings

    SPDX-FileCopyrightText: 2013-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGECUSTOMOPTIONS_H
#define SMB4KCONFIGPAGECUSTOMOPTIONS_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QListWidget>
#include <QWidget>

/**
 * This configuration page contains the custom options
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */

class Smb4KConfigPageCustomOptions : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KConfigPageCustomOptions(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KConfigPageCustomOptions();

    /**
     * This function loads the list of custom options.
     */
    void loadCustomOptions();

    /**
     * This function saves the list of custom options.
     */
    void saveCustomOptions();

    /**
     * This function resets the custom options.
     */
    void resetCustomOptions();

    /**
     * Returns TRUE if there may be changed custom settings. You must check if
     * this is indeed the case in you code.
     *
     * @returns TRUE if custom settings may have changed.
     */
    bool customSettingsChanged();

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
     * This slot is invoked when an item is double clicked. It is used
     * to edit the item the user double clicked.
     *
     * @param item            The item that was double clicked.
     */
    void slotEditCustomItem(QListWidgetItem *item);

    /**
     * This slot is called when a custom options is to be edited.
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
     * This slot is called when the list of custom options is to be cleared.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotClearButtonClicked(bool checked);

    /**
     * This slot is called when the actions performed on the custom options
     * are to be reset.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotResetButtonClicked(bool checked);

    /**
     * This slot is called when the reset button is to be enabled/disabled.
     */
    void slotEnableResetButton();

private:
    bool m_customSettingsChanged;
};

#endif
